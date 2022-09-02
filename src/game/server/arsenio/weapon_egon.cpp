//========= Copyright Glitch Software, All rights reserved. ============//
//
// Purpose:		Egon
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "NPCEvent.h"
#include "basehlcombatweapon_shared.h"
#include "basecombatweapon_shared.h"

#include "Sprite.h"
#include "Beam_Shared.h"
#include "Takedamageinfo.h"
#include "basecombatcharacter.h"
#include "AI_BaseNPC.h"

#ifdef CLIENT_DLL
#include "c_baseplayer.h"
#include "c_hl2_player_shared.h"
#else
#include "player.h"
#include "hl2_player.h"
#endif

//#include "player.h"
#include "gamerules.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
#else
#include "soundent.h"
#include "game.h"
#endif


#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "IEffects.h"
#ifdef CLIENT_DLL
#include "c_te_effect_dispatch.h"
#else
#include "te_effect_dispatch.h"
#endif


enum EGON_FIRESTATE { FIRE_OFF, FIRE_STARTUP, FIRE_CHARGE };

#define EGON_PULSE_INTERVAL			0.1
#define EGON_DISCHARGE_INTERVAL		0.1

#define EGON_BEAM_SPRITE		"sprites/xbeam4.vmt"
//#define EGON_FLARE_SPRITE		"sprites/redglow1.vmt"
//#define EGON_FLARE_SPRITE		"sprites/XSpark1.vmt
#define EGON_FLARE_SPRITE		"sprites/glow.vmt"

extern ConVar sk_plr_dmg_egon_wide;

//-----------------------------------------------------------------------------
// CWeaponEgon
//-----------------------------------------------------------------------------

#ifdef CLIENT_DLL
#define CWeaponEgon C_WeaponEgon
#endif

class CWeaponEgon : public CBaseHLCombatWeapon
{
	DECLARE_CLASS(CWeaponEgon, CBaseHLCombatWeapon);
public:

	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	CWeaponEgon(void);

	virtual bool	Deploy(void);
	void	PrimaryAttack(void);
	virtual void    Precache(void);

	void	SecondaryAttack(void)
	{
		PrimaryAttack();
	}

	void	WeaponIdle(void);
	bool	Holster(CBaseCombatWeapon* pSwitchingTo = NULL);

	//	DECLARE_SERVERCLASS();
	//	DECLARE_DATADESC();

private:
	bool	HasAmmo(void);
	void	UseAmmo(int count);
	void	Attack(void);
	void	EndAttack(void);
	void	Fire(const Vector& vecOrigSrc, const Vector& vecDir);
	void	UpdateEffect(const Vector& startPoint, const Vector& endPoint);
	void	CreateEffect(void);
	void	DestroyEffect(void);

	EGON_FIRESTATE		m_fireState;
	float				m_flAmmoUseTime;	// since we use < 1 point of ammo per update, we subtract ammo on a timer.
	float				m_flShakeTime;
	float				m_flStartFireTime;
	float				m_flDmgTime;
	CHandle<CSprite>	m_hSprite;
	CHandle<CSprite>	m_barrelSprite;
	CHandle<CBeam>		m_hBeam;
	CHandle<CBeam>		m_hNoise;
};

acttable_t	CWeaponEgon::m_acttable[] =
{
	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_AR2,			true },
	{ ACT_RELOAD,					ACT_RELOAD_SMG1,				true },		// FIXME: hook to AR2 unique
	{ ACT_IDLE,						ACT_IDLE_SMG1,					true },		// FIXME: hook to AR2 unique
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_SMG1,			true },		// FIXME: hook to AR2 unique

	{ ACT_WALK,						ACT_WALK_RIFLE,					true },

	// Readiness activities (not aiming)
		{ ACT_IDLE_RELAXED,				ACT_IDLE_SMG1_RELAXED,			false },//never aims
		{ ACT_IDLE_STIMULATED,			ACT_IDLE_SMG1_STIMULATED,		false },
		{ ACT_IDLE_AGITATED,			ACT_IDLE_ANGRY_SMG1,			false },//always aims

		{ ACT_WALK_RELAXED,				ACT_WALK_RIFLE_RELAXED,			false },//never aims
		{ ACT_WALK_STIMULATED,			ACT_WALK_RIFLE_STIMULATED,		false },
		{ ACT_WALK_AGITATED,			ACT_WALK_AIM_RIFLE,				false },//always aims

		{ ACT_RUN_RELAXED,				ACT_RUN_RIFLE_RELAXED,			false },//never aims
		{ ACT_RUN_STIMULATED,			ACT_RUN_RIFLE_STIMULATED,		false },
		{ ACT_RUN_AGITATED,				ACT_RUN_AIM_RIFLE,				false },//always aims

	// Readiness activities (aiming)
		{ ACT_IDLE_AIM_RELAXED,			ACT_IDLE_SMG1_RELAXED,			false },//never aims	
		{ ACT_IDLE_AIM_STIMULATED,		ACT_IDLE_AIM_RIFLE_STIMULATED,	false },
		{ ACT_IDLE_AIM_AGITATED,		ACT_IDLE_ANGRY_SMG1,			false },//always aims

		{ ACT_WALK_AIM_RELAXED,			ACT_WALK_RIFLE_RELAXED,			false },//never aims
		{ ACT_WALK_AIM_STIMULATED,		ACT_WALK_AIM_RIFLE_STIMULATED,	false },
		{ ACT_WALK_AIM_AGITATED,		ACT_WALK_AIM_RIFLE,				false },//always aims

		{ ACT_RUN_AIM_RELAXED,			ACT_RUN_RIFLE_RELAXED,			false },//never aims
		{ ACT_RUN_AIM_STIMULATED,		ACT_RUN_AIM_RIFLE_STIMULATED,	false },
		{ ACT_RUN_AIM_AGITATED,			ACT_RUN_AIM_RIFLE,				false },//always aims
	//End readiness activities

		{ ACT_WALK_AIM,					ACT_WALK_AIM_RIFLE,				true },
		{ ACT_WALK_CROUCH,				ACT_WALK_CROUCH_RIFLE,			true },
		{ ACT_WALK_CROUCH_AIM,			ACT_WALK_CROUCH_AIM_RIFLE,		true },
		{ ACT_RUN,						ACT_RUN_RIFLE,					true },
		{ ACT_RUN_AIM,					ACT_RUN_AIM_RIFLE,				true },
		{ ACT_RUN_CROUCH,				ACT_RUN_CROUCH_RIFLE,			true },
		{ ACT_RUN_CROUCH_AIM,			ACT_RUN_CROUCH_AIM_RIFLE,		true },
		{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_AR2,	false },
		{ ACT_COVER_LOW,				ACT_COVER_SMG1_LOW,				false },		// FIXME: hook to AR2 unique
		{ ACT_RANGE_AIM_LOW,			ACT_RANGE_AIM_AR2_LOW,			false },
		{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_SMG1_LOW,		true },		// FIXME: hook to AR2 unique
		{ ACT_RELOAD_LOW,				ACT_RELOAD_SMG1_LOW,			false },
		{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_SMG1,		true },
		//	{ ACT_RANGE_ATTACK2, ACT_RANGE_ATTACK_AR2_GRENADE, true },
};

IMPLEMENT_ACTTABLE(CWeaponEgon);

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponEgon, DT_WeaponEgon);

BEGIN_NETWORK_TABLE(CWeaponEgon, DT_WeaponEgon)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponEgon)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_egon, CWeaponEgon);
PRECACHE_WEAPON_REGISTER(weapon_egon);

/*
IMPLEMENT_SERVERCLASS_ST( CWeaponEgon, DT_WeaponEgon )
END_SEND_TABLE()
*/

/*
BEGIN_DATADESC( CWeaponEgon )
DEFINE_FIELD( m_fireState, FIELD_INTEGER ),
DEFINE_FIELD( m_flAmmoUseTime, FIELD_TIME ),
DEFINE_FIELD( m_flShakeTime, FIELD_TIME ),
DEFINE_FIELD( m_flStartFireTime, FIELD_TIME ),
DEFINE_FIELD( m_flDmgTime, FIELD_TIME ),
DEFINE_FIELD( m_hSprite, FIELD_EHANDLE ),
DEFINE_FIELD( m_hBeam, FIELD_EHANDLE ),
DEFINE_FIELD( m_hNoise, FIELD_EHANDLE ),
END_DATADESC()
*/

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponEgon::CWeaponEgon(void)
{
	m_bReloadsSingly = false;
	m_bFiresUnderwater = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponEgon::Precache(void)
{
	PrecacheScriptSound("Weapon_Gluon.Start");
	PrecacheScriptSound("Weapon_Gluon.Run");
	PrecacheScriptSound("Weapon_Gluon.Off");

	PrecacheModel(EGON_BEAM_SPRITE);
	PrecacheModel(EGON_FLARE_SPRITE);

	BaseClass::Precache();
}

bool CWeaponEgon::Deploy(void)
{
	m_fireState = FIRE_OFF;

	return BaseClass::Deploy();
}

bool CWeaponEgon::HasAmmo(void)
{
	//CHL1_Player *pPlayer = ToHL1Player(GetOwner());
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
	{
		return false;
	}

	if (pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
		return false;

	return true;
}

void CWeaponEgon::UseAmmo(int count)
{
	//CHL1_Player *pPlayer = ToHL1Player(GetOwner());
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
	{
		return;
	}

	if (pPlayer->GetAmmoCount(m_iPrimaryAmmoType) >= count)
		pPlayer->RemoveAmmo(count, m_iPrimaryAmmoType);
	else
		pPlayer->RemoveAmmo(pPlayer->GetAmmoCount(m_iPrimaryAmmoType), m_iPrimaryAmmoType);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponEgon::PrimaryAttack(void)
{
	Msg("Primary attack called\n");
	//CHL1_Player *pPlayer = ToHL1Player(GetOwner());
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
	{
		Msg("We bad\n");
		return;
	}

	// don't fire underwater
	if (pPlayer->GetWaterLevel() == 3)
	{
		Msg("We in some water?\n");
		if (m_fireState != FIRE_OFF || m_hBeam)
		{
			EndAttack();
		}
		else
		{
			WeaponSound(EMPTY);
		}

		m_flNextPrimaryAttack = gpGlobals->curtime + 0.5;
		m_flNextSecondaryAttack = gpGlobals->curtime + 0.5;
		return;
	}

	Vector vecAiming = pPlayer->GetAutoaimVector(0);
	Vector vecSrc = pPlayer->Weapon_ShootPosition();

	switch (m_fireState)
	{
		Msg("Switch firestate?\n");
	case FIRE_OFF:
	{
		Msg("Case Fire Off?\n");
		if (!HasAmmo())
		{
			Msg("Has no ammo\n");
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.25;
			m_flNextSecondaryAttack = gpGlobals->curtime + 0.25;
			WeaponSound(EMPTY);
			return;
		}

		m_flAmmoUseTime = gpGlobals->curtime;// start using ammo ASAP.

		EmitSound("Weapon_Gluon.Start");

		SendWeaponAnim(ACT_VM_PRIMARYATTACK);

		m_flShakeTime = 0;
		m_flStartFireTime = gpGlobals->curtime;

		SetWeaponIdleTime(gpGlobals->curtime + 0.1);

		m_flDmgTime = gpGlobals->curtime + EGON_PULSE_INTERVAL;
		m_fireState = FIRE_STARTUP;
	}
	break;

	case FIRE_STARTUP:
	{
		Msg("Fire startup?\n");
		Fire(vecSrc, vecAiming);

		if (gpGlobals->curtime >= (m_flStartFireTime + 0.1))
		{
			EmitSound("Weapon_Gluon.Run");

			m_fireState = FIRE_CHARGE;
		}

		if (!HasAmmo())
		{
			EndAttack();
			m_flNextPrimaryAttack = gpGlobals->curtime + 1.0;
			m_flNextSecondaryAttack = gpGlobals->curtime + 1.0;
		}
	}
	case FIRE_CHARGE:
	{
		Msg("Fire Charge???\n");
		Fire(vecSrc, vecAiming);

		if (!HasAmmo())
		{
			EndAttack();
			m_flNextPrimaryAttack = gpGlobals->curtime + 1.0;
			m_flNextSecondaryAttack = gpGlobals->curtime + 1.0;
		}
	}
	Msg("Break\n");
	break;
	}
}

void CWeaponEgon::Fire(const Vector& vecOrigSrc, const Vector& vecDir)
{
	Msg("Fire called\n");
	//CHL1_Player *pPlayer = ToHL1Player(GetOwner());
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
	{
		return;
	}

	//CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), 450, 0.1 );
	WeaponSound(SINGLE);

	Vector vecDest = vecOrigSrc + (vecDir * MAX_TRACE_LENGTH);

	trace_t	tr;
	UTIL_TraceLine(vecOrigSrc, vecDest, MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr);

	if (tr.allsolid)
		return;

	CBaseEntity* pEntity = tr.m_pEnt;
	if (pEntity == NULL)
		return;

	if (g_pGameRules->IsMultiplayer())
	{
		if (m_hSprite)
		{
			if (pEntity->m_takedamage != DAMAGE_NO)
			{
				m_hSprite->TurnOn();
			}
			else
			{
				//m_hSprite->TurnOff();
				m_hSprite->TurnOn();
			}
		}

		if (m_barrelSprite)
		{
			if (pEntity->m_takedamage != DAMAGE_NO)
			{
				m_barrelSprite->TurnOn();
			}
			else
			{
				//m_hSprite->TurnOff();
				m_barrelSprite->TurnOn();
			}
		}
	}

	if (m_flDmgTime < gpGlobals->curtime)
	{
		// wide mode does damage to the ent, and radius damage
		if (pEntity->m_takedamage != DAMAGE_NO)
		{
			ClearMultiDamage();
			CTakeDamageInfo info(this, pPlayer, sk_plr_dmg_egon_wide.GetFloat() * g_pGameRules->GetDamageMultiplier(), DMG_ENERGYBEAM | DMG_ALWAYSGIB);
			CalculateMeleeDamageForce(&info, vecDir, tr.endpos);
			pEntity->DispatchTraceAttack(info, vecDir, &tr);
			ApplyMultiDamage();
		}

		if (g_pGameRules->IsMultiplayer())
		{
			// radius damage a little more potent in multiplayer.
#ifndef CLIENT_DLL
			RadiusDamage(CTakeDamageInfo(this, pPlayer, sk_plr_dmg_egon_wide.GetFloat() * g_pGameRules->GetDamageMultiplier() / 4, DMG_ENERGYBEAM | DMG_BLAST | DMG_ALWAYSGIB), tr.endpos, 128, CLASS_NONE, NULL);
#endif
		}

		if (!pPlayer->IsAlive())
			return;

		if (g_pGameRules->IsMultiplayer())
		{
			//multiplayer uses 5 ammo/second
			if (gpGlobals->curtime >= m_flAmmoUseTime)
			{
				UseAmmo(1);
				m_flAmmoUseTime = gpGlobals->curtime + 0.2;
			}
		}
		else
		{
			// Wide mode uses 10 charges per second in single player
			if (gpGlobals->curtime >= m_flAmmoUseTime)
			{
				UseAmmo(1);
				m_flAmmoUseTime = gpGlobals->curtime + 0.1;
			}
		}

		m_flDmgTime = gpGlobals->curtime + EGON_DISCHARGE_INTERVAL;
		if (m_flShakeTime < gpGlobals->curtime)
		{
#ifndef CLIENT_DLL
			UTIL_ScreenShake(tr.endpos, 5.0, 150.0, 0.75, 250.0, SHAKE_START);
#endif
			m_flShakeTime = gpGlobals->curtime + 1.5;
		}
	}

	Vector vecUp, vecRight;
	QAngle angDir;

	VectorAngles(vecDir, angDir);
	AngleVectors(angDir, NULL, &vecRight, &vecUp);

	Vector tmpSrc = vecOrigSrc + (vecUp * -8) + (vecRight * 3);
	UpdateEffect(tmpSrc, tr.endpos);
}

void CWeaponEgon::UpdateEffect(const Vector& startPoint, const Vector& endPoint)
{
	if (!m_hBeam)
	{
		CreateEffect();
	}

	if (m_hBeam)
	{
		m_hBeam->SetStartPos(endPoint);
	}

	if (m_hSprite)
	{
		m_hSprite->SetAbsOrigin(endPoint);

		m_hSprite->m_flFrame += 32 * gpGlobals->frametime;
		if (m_hSprite->m_flFrame > m_hSprite->Frames())
			m_hSprite->m_flFrame = 0;
	}

	if (m_barrelSprite)
	{
		m_barrelSprite->SetAbsOrigin(startPoint);

		//m_barrelSprite->m_flFrame += 8 * gpGlobals->frametime;
		m_barrelSprite->m_flFrame += 32 * gpGlobals->frametime;
		if (m_barrelSprite->m_flFrame > m_barrelSprite->Frames())
			m_barrelSprite->m_flFrame = 0;
	}

	if (m_hNoise)
	{
		m_hNoise->SetStartPos(endPoint);
	}
}

void CWeaponEgon::CreateEffect(void)
{
#ifndef CLIENT_DLL    
	//CHL1_Player *pPlayer = ToHL1Player(GetOwner());
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
	{
		return;
	}

	DestroyEffect();

	m_hBeam = CBeam::BeamCreate(EGON_BEAM_SPRITE, 3.5);
	m_hBeam->PointEntInit(GetAbsOrigin(), this);
	m_hBeam->SetBeamFlags(FBEAM_SINENOISE);
	m_hBeam->SetEndAttachment(1);
	m_hBeam->AddSpawnFlags(SF_BEAM_TEMPORARY);	// Flag these to be destroyed on save/restore or level transition
	m_hBeam->SetOwnerEntity(pPlayer);
	m_hBeam->SetScrollRate(10);
	m_hBeam->SetBrightness(200);
	m_hBeam->SetColor(0, 240, 240);
	m_hBeam->SetNoise(0.2);

	m_hNoise = CBeam::BeamCreate(EGON_BEAM_SPRITE, 5.0);
	m_hNoise->PointEntInit(GetAbsOrigin(), this);
	//m_hBeam->SetBeamFlags(FBEAM_FADEIN);
	//FBEAM_FADEIN
	m_hNoise->SetEndAttachment(1);
	m_hNoise->AddSpawnFlags(SF_BEAM_TEMPORARY);
	m_hNoise->SetOwnerEntity(pPlayer);
	m_hNoise->SetScrollRate(25);
	m_hNoise->SetBrightness(200);
	m_hNoise->SetColor(0, 240, 240);
	m_hNoise->SetNoise(0.8);

	m_hSprite = CSprite::SpriteCreate(EGON_FLARE_SPRITE, GetAbsOrigin(), false);
	m_hSprite->SetScale(1.0);
	/*m_hSprite->SetTransparency(kRenderGlow, 0, 255, 255, 255, kRenderFxNoDissipation);*/
	m_hSprite->SetTransparency(kRenderGlow, 0, 255, 255, 255, kRenderFxDistort);
	m_hSprite->AddSpawnFlags(SF_SPRITE_TEMPORARY);
	m_hSprite->SetOwnerEntity(pPlayer);

	m_barrelSprite = CSprite::SpriteCreate(EGON_FLARE_SPRITE, GetAbsOrigin(), false);
	m_barrelSprite->SetScale(2);
	//m_barrelSprite->SetEndAttachment(1);
	//m_barrelSprite->SetTransparency(kRenderGlow, 0, 255, 255, 255, kRenderFxNoDissipation); kRenderFxDistort
	m_barrelSprite->SetTransparency(kRenderGlow, 0, 255, 255, 255, kRenderFxDistort);
	m_barrelSprite->AddSpawnFlags(SF_SPRITE_TEMPORARY);
	m_barrelSprite->SetOwnerEntity(pPlayer);
#endif    
}


void CWeaponEgon::DestroyEffect(void)
{
#ifndef CLIENT_DLL    
	if (m_hBeam)
	{
		UTIL_Remove(m_hBeam);
		m_hBeam = NULL;
	}
	if (m_hNoise)
	{
		UTIL_Remove(m_hNoise);
		m_hNoise = NULL;
	}
	if (m_hSprite)
	{
		m_hSprite->Expand(10, 500);
		m_hSprite = NULL;
	}
	if (m_barrelSprite)
	{
		m_barrelSprite->Expand(10, 500);
		m_barrelSprite = NULL;
	}
#endif
}

void CWeaponEgon::EndAttack(void)
{
	StopSound("Weapon_Gluon.Run");

	if (m_fireState != FIRE_OFF)
	{
		EmitSound("Weapon_Gluon.Off");
	}

	SetWeaponIdleTime(gpGlobals->curtime + 2.0);
	m_flNextPrimaryAttack = gpGlobals->curtime + 0.5;
	m_flNextSecondaryAttack = gpGlobals->curtime + 0.5;

	m_fireState = FIRE_OFF;

	DestroyEffect();
}

bool CWeaponEgon::Holster(CBaseCombatWeapon* pSwitchingTo)
{
	EndAttack();

	return BaseClass::Holster(pSwitchingTo);
}

void CWeaponEgon::WeaponIdle(void)
{
	if (!HasWeaponIdleTimeElapsed())
		return;

	if (m_fireState != FIRE_OFF)
		EndAttack();

	int iAnim;

	float flRand = random->RandomFloat(0, 1);
	float flIdleTime;
	if (flRand <= 0.5)
	{
		iAnim = ACT_VM_IDLE;
		flIdleTime = gpGlobals->curtime + random->RandomFloat(10, 15);
	}
	else
	{
		iAnim = ACT_VM_FIDGET;
		flIdleTime = gpGlobals->curtime + 3.0;
	}

	SendWeaponAnim(iAnim);

	SetWeaponIdleTime(flIdleTime);
}