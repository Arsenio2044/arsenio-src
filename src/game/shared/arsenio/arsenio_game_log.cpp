//========= Copyright Glitch Software, All rights reserved. ============//
//
// Purpose: New logging system for Arsenio.
//=============================================================================//

#include "cbase.h"

#include "tier0/icommandline.h"
#include "igamesystem.h"
#include "filesystem.h"
#include "utlbuffer.h"
#ifdef CLIENT_DLL
#else
#include "ammodef.h"
#include "ai_basenpc.h"
#include "ai_squad.h"
#include "fmtstr.h"
#include "GameEventListener.h"
#include "saverestore_utlvector.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef GAME_DLL
// ------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------

class CArsenioGameLogger : public CLogicalEntity, public CGameEventListener
{
public:
	DECLARE_DATADESC();
	DECLARE_CLASS( CArsenioGameLogger, CLogicalEntity );

	CArsenioGameLogger()
	{
		pGameLoggerEnt = this;
	}

	void Activate()
	{
		BaseClass::Activate();

		ListenForGameEvent("skill_changed");
	}

	void FireGameEvent( IGameEvent *event )
	{
		if (FStrEq(event->GetName(), "skill_changed"))
		{
			m_ListSkillChanged.AddToTail(event->GetInt("skill_level"));
			m_ListSkillChangedTime.AddToTail(gpGlobals->curtime);
		}
	}

	float m_flLastLogTime;
	int m_iSaveID;

	CUtlVector<int> m_ListSkillChanged;
	CUtlVector<float> m_ListSkillChangedTime;

	static CArsenioGameLogger *GetGameLoggerEnt()
	{
		if (!pGameLoggerEnt)
			pGameLoggerEnt = static_cast<CArsenioGameLogger*>(CBaseEntity::Create("arsenio_game_logger", vec3_origin, vec3_angle));

		return pGameLoggerEnt;
	}

private:
	static CHandle<CArsenioGameLogger> pGameLoggerEnt;
};

LINK_ENTITY_TO_CLASS( arsenio_game_logger, CArsenioGameLogger );

BEGIN_DATADESC( CArsenioGameLogger )

	DEFINE_FIELD( m_flLastLogTime, FIELD_TIME ),
	DEFINE_FIELD( m_iSaveID, FIELD_INTEGER ),

	DEFINE_UTLVECTOR( m_ListSkillChanged, FIELD_INTEGER ),
	DEFINE_UTLVECTOR( m_ListSkillChangedTime, FIELD_TIME ),

END_DATADESC()

CHandle<CArsenioGameLogger> CArsenioGameLogger::pGameLoggerEnt;

void ArsenioGameLog_CVarToggle( IConVar *var, const char *pOldString, float flOldValue );
ConVar arsenio_game_log_on_autosave( "arsenio_game_log_on_autosave", "1", FCVAR_NONE, "Logs information to %mapname%_log_%number%.txt on each autosave", ArsenioGameLog_CVarToggle );

void ArsenioGameLog_Init()
{
	if (arsenio_game_log_on_autosave.GetBool())
	{
		// Create the game logger ent
		CArsenioGameLogger::GetGameLoggerEnt();
	}
}

void ArsenioGameLog_Record(const char* szContext)
{
	CArsenioGameLogger* pGameLoggerEnt = CArsenioGameLogger::GetGameLoggerEnt();
	if (!pGameLoggerEnt)
	{
		Warning("Failed to get game logger ent\n");
		return;
	}

	KeyValues* pKV = new KeyValues("Log");

	// Logging Information
	KeyValues* pKVLogInfo = pKV->FindKey("logging_info", true);
	if (pKVLogInfo)
	{
		pKVLogInfo->SetString("context", szContext);
		pKVLogInfo->SetFloat("last_log", pGameLoggerEnt->m_flLastLogTime > 0.0f ? gpGlobals->curtime - pGameLoggerEnt->m_flLastLogTime : -1.0f);
	}

	// Game Information
	KeyValues* pKVGameInfo = pKV->FindKey("game_info", true);
	if (pKVGameInfo)
	{
		pKVGameInfo->SetInt("skill", g_pGameRules->GetSkillLevel());

		if (pGameLoggerEnt->m_ListSkillChanged.Count() > 0)
		{
			KeyValues* pKVSkill = pKVGameInfo->FindKey("skill_changes", true);
			for (int i = 0; i < pGameLoggerEnt->m_ListSkillChanged.Count(); i++)
			{
				float flTime = pGameLoggerEnt->m_ListSkillChangedTime[i];
				switch (pGameLoggerEnt->m_ListSkillChanged[i])
				{
				case SKILL_EASY:    pKVSkill->SetString(CNumStr(flTime), "easy"); break;
				case SKILL_MEDIUM:  pKVSkill->SetString(CNumStr(flTime), "normal"); break;
				case SKILL_HARD:    pKVSkill->SetString(CNumStr(flTime), "hard"); break;
				}
			}
		}
	}

	// Player Information
	KeyValues* pKVPlayer = pKV->FindKey("player", true);
	if (pKVPlayer)
	{
		CBasePlayer* pPlayer = UTIL_GetLocalPlayer();

		if (pPlayer)
		{
			pKVPlayer->SetInt("health", pPlayer->GetHealth());
			pKVPlayer->SetInt("armor", pPlayer->ArmorValue());

			pKVPlayer->SetString("position", CFmtStrN<128>("[%f %f %f]", pPlayer->GetAbsOrigin().x, pPlayer->GetAbsOrigin().y, pPlayer->GetAbsOrigin().z));
			pKVPlayer->SetString("angles", CFmtStrN<128>("[%f %f %f]", pPlayer->EyeAngles().x, pPlayer->EyeAngles().y, pPlayer->EyeAngles().z));

			KeyValues* pKVWeapons = pKVPlayer->FindKey("weapons", true);
			if (pKVWeapons)
			{
				// Cycle through all of the player's weapons
				for (int i = 0; i < pPlayer->WeaponCount(); i++)
				{
					CBaseCombatWeapon* pWeapon = pPlayer->GetWeapon(i);
					if (!pWeapon)
						continue;

					KeyValues* pKVWeapon = pKVWeapons->FindKey(pWeapon->GetClassname(), true);
					if (pKVWeapon)
					{
						pKVWeapon->SetString("ammo", CFmtStrN<32>("%i; %i", pWeapon->m_iClip1.Get(), pWeapon->m_iClip2.Get()));
						pKVWeapon->SetBool("active", pPlayer->GetActiveWeapon() == pWeapon);
					}
				}
			}

			KeyValues* pKVAmmo = pKVPlayer->FindKey("ammo", true);
			if (pKVAmmo)
			{
				// Cycle through all of the player's ammo
				for (int i = 0; i < GetAmmoDef()->m_nAmmoIndex; i++)
				{
					int iAmmo = pPlayer->GetAmmoCount(i);
					if (iAmmo > 0)
						pKVAmmo->SetInt(GetAmmoDef()->m_AmmoType[i].pName, iAmmo);
				}
			}
		}
	}

	// AI Information
	CFmtStrN<128> npcPositionFmt("[%f %f %f]");
	KeyValues* pKVAIs = pKV->FindKey("ai_characters", true);
	if (pKVAIs)
	{
		CAI_BaseNPC** ppAIs = g_AI_Manager.AccessAIs();
		int nAIs = g_AI_Manager.NumAIs();
		for (int i = 0; i < nAIs; i++)
		{
			CAI_BaseNPC* pNPC = ppAIs[i];

			if (!pNPC->IsAlive() || pNPC->GetSleepState() != AISS_AWAKE)
				continue;

			KeyValues* pKVNPC = pKVAIs->FindKey(CNumStr(pNPC->entindex()), true);
			if (pKVNPC)
			{
				pKVNPC->SetString("classname", pNPC->GetClassname());
				pKVNPC->SetString("name", STRING(pNPC->GetEntityName()));
				pKVNPC->SetString("position", CFmtStrN<128>(npcPositionFmt, pNPC->GetAbsOrigin().x, pNPC->GetAbsOrigin().y, pNPC->GetAbsOrigin().z));
				pKVNPC->SetInt("health", pNPC->GetHealth());
				pKVNPC->SetString("squad", pNPC->GetSquad() ? pNPC->GetSquad()->GetName() : "None");
				pKVNPC->SetString("weapon", pNPC->GetActiveWeapon() ? pNPC->GetActiveWeapon()->GetClassname() : "None");
			}
		}
	}
}

static void CC_Arsenio_GameLogRecord( const CCommand& args )
{
	ArsenioGameLog_Record( "command" );
}

static ConCommand arsenio_game_log_record("arsenio_game_log_record", CC_Arsenio_GameLogRecord, "Records game data to %mapname%_log_%number%." );

void ArsenioGameLog_CVarToggle( IConVar *var, const char *pOldString, float flOldValue )
{
	if (arsenio_game_log_on_autosave.GetBool())
	{
		// Create the game logger ent
		CArsenioGameLogger::GetGameLoggerEnt();
	}
}
#endif