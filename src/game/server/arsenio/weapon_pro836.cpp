//========= Copyright Glitch Software, All rights reserved. ============//
//
// Purpose: Futuristic SMG. (It actually uses an AR2 model bu- SHUT THE FUCK UP TUCKE- I MEAN TUX.
//
//=============================================================================//

#include "cbase.h"
#include "basehlcombatweapon.h"
#include "basehlcombatweapon_shared.h"
#include "npcevent.h"
#include "basecombatcharacter.h"
#include "ai_basenpc.h"
#include "player.h"
#include "game.h"
#include "in_buttons.h"
#include "ai_memory.h"
#include "soundent.h"
#include "rumble_shared.h"
#include "gamestats.h"
//#include "..\missionchoosermodule_base.h" // TUX: We need this for the weapon armory customization.

// memdbgon must be the last include file in a .cpp file!!! TUX: FUCKING HELL VS OH FUCK I SAID A BRITISH THING!
#include "tier0/memdbgon.h"

extern ConVar sk_plr_dmg_smg1_grenade;

//-----------------------------------------------------------------------------
// CWeaponPRO836
//-----------------------------------------------------------------------------
class CWeaponPRO836 : public CHLSelectFireMachineGun
{
	DECLARE_DATADESC();

public:

	// TUX: Declar the class JAMU.
	DECLARE_CLASS( CWeaponPRO836, CHLSelectFireMachineGun );

	CWeaponPRO836();

	DECLARE_SERVERCLASS();

	void			Precache();
	void			AddViewKick();
	void			SecondaryAttack();
	void	        ToggleZoom(void);
	void	            CheckZoomToggle(void);
	bool				m_bInZoom;
	bool				m_bMustReload;

	int				GetMinBurst() { return 2; }
	int				GetMaxBurst() { return 5; }

	virtual void	Equip( CBaseCombatCharacter *pOwner );
	virtual bool	IsWeaponZoomed() { return m_bInZoom; }
	virtual void	ItemPostFrame(void);
	virtual void	ItemBusyFrame(void);
	bool			Reload();

	float			GetFireRate() { return 0.075f; } // RPS, 60sec/800 rounds = 0.075
	int				CapabilitiesGet() { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	int				WeaponRangeAttack2Condition( float flDot, float flDist );
	Activity		GetPrimaryAttackActivity();

	void	DrawHitmarker(void);

	// TUX: Values which allow our "spread" to change from button input from player
	virtual const Vector &GetBulletSpread()
	{
		// Define "spread" parameters based on the "owner" and what they are doing
		static Vector plrDuckCone = VECTOR_CONE_2DEGREES;
		static Vector plrStandCone = VECTOR_CONE_7DEGREES;
		static Vector plrMoveCone = VECTOR_CONE_20DEGREES;
		static Vector npcCone = VECTOR_CONE_5DEGREES;
		static Vector plrRunCone = VECTOR_CONE_6DEGREES;
		static Vector plrJumpCone = VECTOR_CONE_9DEGREES;

		if ( GetOwner() && GetOwner()->IsNPC() )
			return npcCone;

		//static Vector cone;

		// We must know the player "owns" the weapon before different cones may be used
		CBasePlayer *pPlayer = ToBasePlayer( GetOwnerEntity() );
		if ( pPlayer->m_nButtons & IN_DUCK )
			return plrDuckCone;
		if ( pPlayer->m_nButtons & IN_FORWARD )
			return plrMoveCone;
		if ( pPlayer->m_nButtons & IN_BACK )
			return plrMoveCone;
		if ( pPlayer->m_nButtons & IN_MOVERIGHT )
			return plrMoveCone;
		if ( pPlayer->m_nButtons & IN_MOVELEFT )
			return plrMoveCone;
		if ( pPlayer->m_nButtons & IN_JUMP )
			return plrJumpCone;
		if ( pPlayer->m_nButtons & IN_SPEED )
			return plrRunCone;
		if ( pPlayer->m_nButtons & IN_RUN )
			return plrRunCone;
		else
			return plrStandCone;
	}

	// TUX: This fucking thing causes symbol errors.
	const WeaponProficiencyInfo_t *GetProficiencyValues();

	void FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir );
	void Operator_ForceNPCFire( CBaseCombatCharacter *pOperator, bool bSecondary );
	void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );

	DECLARE_ACTTABLE();
};

IMPLEMENT_SERVERCLASS_ST( CWeaponPRO836, DT_WeaponPRO836 )
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_pro836, CWeaponPRO836 );
PRECACHE_WEAPON_REGISTER( weapon_pro836 );

BEGIN_DATADESC( CWeaponPRO836 )
DEFINE_FIELD(m_bInZoom, FIELD_BOOLEAN),
END_DATADESC()

acttable_t CWeaponPRO836::m_acttable[] =
{
	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_SMG1,			true },
	{ ACT_RELOAD,					ACT_RELOAD_SMG1,				true },
	{ ACT_IDLE,						ACT_IDLE_SMG1,					true },
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_SMG1,			true },

	{ ACT_WALK,						ACT_WALK_RIFLE,					true },
	{ ACT_WALK_AIM,					ACT_WALK_AIM_RIFLE,				true  },

	// Readiness activities (not aiming)
	{ ACT_IDLE_RELAXED,				ACT_IDLE_SMG1_RELAXED,			false }, // Never aims
	{ ACT_IDLE_STIMULATED,			ACT_IDLE_SMG1_STIMULATED,		false },
	{ ACT_IDLE_AGITATED,			ACT_IDLE_ANGRY_SMG1,			false }, // Always aims

	{ ACT_WALK_RELAXED,				ACT_WALK_RIFLE_RELAXED,			false }, // Never aims
	{ ACT_WALK_STIMULATED,			ACT_WALK_RIFLE_STIMULATED,		false },
	{ ACT_WALK_AGITATED,			ACT_WALK_AIM_RIFLE,				false }, // Always aims

	{ ACT_RUN_RELAXED,				ACT_RUN_RIFLE_RELAXED,			false }, // Never aims
	{ ACT_RUN_STIMULATED,			ACT_RUN_RIFLE_STIMULATED,		false },
	{ ACT_RUN_AGITATED,				ACT_RUN_AIM_RIFLE,				false }, // Always aims

	// Readiness activities (aiming)
	{ ACT_IDLE_AIM_RELAXED,			ACT_IDLE_SMG1_RELAXED,			false }, // Never aims
	{ ACT_IDLE_AIM_STIMULATED,		ACT_IDLE_AIM_RIFLE_STIMULATED,	false },
	{ ACT_IDLE_AIM_AGITATED,		ACT_IDLE_ANGRY_SMG1,			false }, // Always aims

	{ ACT_WALK_AIM_RELAXED,			ACT_WALK_RIFLE_RELAXED,			false }, // Never aims
	{ ACT_WALK_AIM_STIMULATED,		ACT_WALK_AIM_RIFLE_STIMULATED,	false },
	{ ACT_WALK_AIM_AGITATED,		ACT_WALK_AIM_RIFLE,				false }, // Always aims

	{ ACT_RUN_AIM_RELAXED,			ACT_RUN_RIFLE_RELAXED,			false }, // Never aims
	{ ACT_RUN_AIM_STIMULATED,		ACT_RUN_AIM_RIFLE_STIMULATED,	false },
	{ ACT_RUN_AIM_AGITATED,			ACT_RUN_AIM_RIFLE,				false }, // Always aims
	// End readiness activities

	{ ACT_WALK_AIM,					ACT_WALK_AIM_RIFLE,				true },
	{ ACT_WALK_CROUCH,				ACT_WALK_CROUCH_RIFLE,			true },
	{ ACT_WALK_CROUCH_AIM,			ACT_WALK_CROUCH_AIM_RIFLE,		true },
	{ ACT_RUN,						ACT_RUN_RIFLE,					true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_RIFLE,				true },
	{ ACT_RUN_CROUCH,				ACT_RUN_CROUCH_RIFLE,			true },
	{ ACT_RUN_CROUCH_AIM,			ACT_RUN_CROUCH_AIM_RIFLE,		true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_SMG1,	true },
	{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_SMG1_LOW,		true },
	{ ACT_COVER_LOW,				ACT_COVER_SMG1_LOW,				false },
	{ ACT_RANGE_AIM_LOW,			ACT_RANGE_AIM_SMG1_LOW,			false },
	{ ACT_RELOAD_LOW,				ACT_RELOAD_SMG1_LOW,			false },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_SMG1,		true },
};

IMPLEMENT_ACTTABLE( CWeaponPRO836 );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CWeaponPRO836::CWeaponPRO836()
{
	m_fMinRange1 = 0; // No minimum range
	m_fMaxRange1 = 1400;
	m_bInZoom = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPRO836::ToggleZoom(void)
{
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer == NULL)
		return;

	if (m_bInZoom)
	{
		if (pPlayer->SetFOV(this, 0, 0.2f))
		{
			m_bInZoom = false;
		}
	}
	else
	{
		if (pPlayer->SetFOV(this, 20, 0.1f))
		{
			m_bInZoom = true;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPRO836::CheckZoomToggle(void)
{
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer->m_afButtonPressed & IN_ATTACK2)
	{
		ToggleZoom();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPRO836::ItemBusyFrame(void)
{
	// Allow zoom toggling even when we're reloading
	CheckZoomToggle();
}

void CWeaponPRO836::DrawHitmarker(void)
{
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer == NULL)
		return;

#ifndef CLIENT_DLL
	CSingleUserRecipientFilter filter(pPlayer);
	UserMessageBegin(filter, "ShowHitmarker");
	WRITE_BYTE(1);
	MessageEnd();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPRO836::ItemPostFrame(void)
{
	// Allow zoom toggling
	CheckZoomToggle();

	if (m_bMustReload && HasWeaponIdleTimeElapsed())
	{
		Reload();
	}

	BaseClass::ItemPostFrame();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPRO836::Precache()
{
	BaseClass::Precache();  // shit
}

//-----------------------------------------------------------------------------
// Purpose: Give this weapon longer range when wielded by an ally NPC.
//-----------------------------------------------------------------------------
void CWeaponPRO836::Equip( CBaseCombatCharacter *pOwner )
{
	if ( pOwner->Classify() == CLASS_PLAYER_ALLY )
		m_fMaxRange1 = 3000;
	else
		m_fMaxRange1 = 1400;

	BaseClass::Equip( pOwner );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPRO836::FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir )
{
	// FIXME: Use the returned number of bullets to account for >10hz firerate
	WeaponSoundRealtime( SINGLE_NPC );

	CSoundEnt::InsertSound( SOUND_COMBAT|SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy() );
	pOperator->FireBullets( 1, vecShootOrigin, vecShootDir, VECTOR_CONE_PRECALCULATED, MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2, entindex(), 0 );

	pOperator->DoMuzzleFlash();
	m_iClip1--;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPRO836::Operator_ForceNPCFire( CBaseCombatCharacter *pOperator, bool bSecondary )
{
	// Ensure we have enough rounds in the magazine
	m_iClip1++;

	Vector vecShootOrigin, vecShootDir;
	QAngle angShootDir;
	GetAttachment( LookupAttachment("muzzle"), vecShootOrigin, angShootDir );
	AngleVectors( angShootDir, &vecShootDir );
	FireNPCPrimaryAttack( pOperator, vecShootOrigin, vecShootDir );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPRO836::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	switch ( pEvent->event )
	{
	case EVENT_WEAPON_SMG1:
		{
			Vector vecShootOrigin, vecShootDir;
			QAngle angDiscard;

			// Support old style attachment point firing
			if ( (pEvent->options == NULL) || (pEvent->options[0] == '\0') || (!pOperator->GetAttachment(pEvent->options, vecShootOrigin, angDiscard)) )
				vecShootOrigin = pOperator->Weapon_ShootPosition();

			CAI_BaseNPC *npc = pOperator->MyNPCPointer();
			ASSERT( npc != NULL );
			vecShootDir = npc->GetActualShootTrajectory( vecShootOrigin );

			FireNPCPrimaryAttack( pOperator, vecShootOrigin, vecShootDir );
		}
		break;

	default:
		BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Activity
//-----------------------------------------------------------------------------
Activity CWeaponPRO836::GetPrimaryAttackActivity()
{
	if ( m_nShotsFired < 2 )
		return ACT_VM_PRIMARYATTACK;

	if ( m_nShotsFired < 3 )
		return ACT_VM_RECOIL1;

	if ( m_nShotsFired < 4 )
		return ACT_VM_RECOIL2;

	return ACT_VM_RECOIL3;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPRO836::AddViewKick(void)
{
#define	EASY_DAMPEN			2.5f	// BREADMAN
#define	MAX_VERTICAL_KICK	30.0f	//Degrees - was 1.0
#define	SLIDE_LIMIT			1.0f	//Seconds - was 2.0

	//Get the view kick
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer == NULL)
		return;

	DoMachineGunKick(pPlayer, EASY_DAMPEN, MAX_VERTICAL_KICK, m_fFireDuration, SLIDE_LIMIT);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponPRO836::Reload()
{
	bool fRet = DefaultReload( GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD );
	if ( fRet )
		WeaponSound( RELOAD );

	return fRet;
}
/* ; old broken code
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPRO836::AddViewKick()
{
	#define EASY_DAMPEN			2.3f
	#define MAX_VERTICAL_KICK	17.0f // Degrees
	#define SLIDE_LIMIT			3.11f // Seconds

	// Get the view kick
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if ( pPlayer == NULL )
		return;

	DoMachineGunKick( pPlayer, EASY_DAMPEN, MAX_VERTICAL_KICK, m_fFireDuration, SLIDE_LIMIT );
}
*/
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPRO836::SecondaryAttack()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : flDot -
//          flDist -
// Output : int
//-----------------------------------------------------------------------------
int CWeaponPRO836::WeaponRangeAttack2Condition( float flDot, float flDist )
{
	return COND_NONE;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const WeaponProficiencyInfo_t *CWeaponPRO836::GetProficiencyValues()
{
	static WeaponProficiencyInfo_t proficiencyTable[] =
	{
		{ 7.0,		0.75	},
		{ 5.00,		0.75	},
		{ 10.0/3.0, 0.75	},
		{ 5.0/3.0,	0.75	},
		{ 1.00,		1.0		},
	};

	COMPILE_TIME_ASSERT( ARRAYSIZE(proficiencyTable) == WEAPON_PROFICIENCY_PERFECT + 1);

	return proficiencyTable;
}
