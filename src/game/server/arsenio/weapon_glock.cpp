//========= Copyright Glitch Software, All rights reserved. ============//
//
// Purpose: idfk
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "basehlcombatweapon.h"
#include "basecombatcharacter.h"
#include "ai_basenpc.h"
#include "player.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "soundent.h"
#include "game.h"
#include "vstdlib/random.h"
#include "gamestats.h"
#include "particle_parse.h"
#include "effect_dispatch_data.h"
#include "te_effect_dispatch.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	GLOCK_FASTEST_REFIRE_TIME		0.1f
#define	GLOCK_FASTEST_DRY_REFIRE_TIME	0.2f

//#define	GLOCK_ACCURACY_SHOT_PENALTY_TIME		0.2f	// Applied amount of time each shot adds to the time we must recover from
//#define	GLOCK_ACCURACY_MAXIMUM_PENALTY_TIME	1.5f	// Maximum penalty to deal out

//-----------------------------------------------------------------------------
// CWeaponGlock
//-----------------------------------------------------------------------------

class CWeaponGlock : public CBaseHLCombatWeapon
{
	DECLARE_DATADESC();

public:
	DECLARE_CLASS( CWeaponGlock, CBaseHLCombatWeapon );

	CWeaponGlock(void);

	DECLARE_SERVERCLASS();

	void	Precache( void );
	void	ItemPostFrame( void );
	void	ItemPreFrame( void );
	void	ItemBusyFrame( void );
	void	PrimaryAttack( void );
	void	SecondaryAttack(void);
//	void	DrawHitmarker( void ); // ADAHAHA
	void	AddViewKick( void );
	void	DryFire( void );
	void	Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
	void	FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir );
	void	Operator_ForceNPCFire( CBaseCombatCharacter  *pOperator, bool bSecondary );

	void	UpdatePenaltyTime( void );

	int		CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	Activity	GetPrimaryAttackActivity( void );

	Vector CWeaponGlock::m_vDefaultAccuracy = VECTOR_CONE_MAX_INACCURACY;


	virtual bool Reload( void );

	const float CWeaponGlock::GLOCK_ACCURACY_SHOT_PENALTY_TIME = 0.2f;
	const float CWeaponGlock::GLOCK_ACCURACY_MAXIMUM_PENALTY_TIME = 1.5f;
	const Vector CWeaponGlock::VECTOR_CONE_PERFECT_ACCURACY = Vector(0.0f, 0.0f, 0.0f);
	const Vector CWeaponGlock::VECTOR_CONE_MAX_INACCURACY = Vector(0.06f, 0.06f, 0.0f);

	const Vector& CWeaponGlock::GetBulletSpread(void)
	{
		CBasePlayer* pOwner = ToBasePlayer(GetOwner());
		if (!pOwner)
			return m_vDefaultAccuracy;

		if (m_nNumShotsFired == 0)
			return VECTOR_CONE_PERFECT_ACCURACY; // First shot is perfectly accurate



		// Calculate the time since the last shot was fired
		float timeSinceLastShot = gpGlobals->curtime - m_flLastAttackTime;

		// Calculate the time we have been firing continuously
		float continuousFireTime = gpGlobals->curtime - m_flSoonestPrimaryAttack;

		// Calculate the penalty time based on the time since last shot and the maximum penalty time
		float penaltyTime = GLOCK_ACCURACY_SHOT_PENALTY_TIME * (timeSinceLastShot / GLOCK_FASTEST_REFIRE_TIME);
		penaltyTime = clamp(penaltyTime, 0.0f, GLOCK_ACCURACY_MAXIMUM_PENALTY_TIME);

		// Calculate the penalty lerp factor based on the time firing continuously
		float penaltyLerp = RemapValClamped(continuousFireTime, 0.0f, GLOCK_ACCURACY_MAXIMUM_PENALTY_TIME, 0.0f, 1.0f);

		// Lerp between perfect accuracy and maximum inaccuracy based on the penalty lerp factor
		VectorLerp(VECTOR_CONE_PERFECT_ACCURACY, VECTOR_CONE_MAX_INACCURACY, penaltyLerp, m_vDefaultAccuracy);
		return m_vDefaultAccuracy;
	}
	
	virtual int	GetMinBurst() 
	{ 
		return 1; 
	}

	virtual int	GetMaxBurst() 
	{ 
		return 3; 
	}

	virtual float GetFireRate( void ) 
	{
		return 0.5f; 
	}

	DECLARE_ACTTABLE();

private:
	float	m_flSoonestPrimaryAttack;
	float	m_flLastAttackTime;
	float	m_flAccuracyPenalty;
	int		m_nNumShotsFired;
};


IMPLEMENT_SERVERCLASS_ST(CWeaponGlock, DT_WeaponGlock)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_glock, CWeaponGlock );
PRECACHE_WEAPON_REGISTER( weapon_glock );

BEGIN_DATADESC( CWeaponGlock )

	DEFINE_FIELD( m_flSoonestPrimaryAttack, FIELD_TIME ),
	DEFINE_FIELD( m_flLastAttackTime,		FIELD_TIME ),
	DEFINE_FIELD( m_flAccuracyPenalty,		FIELD_FLOAT ), //NOTENOTE: This is NOT tracking game time
	DEFINE_FIELD( m_nNumShotsFired,			FIELD_INTEGER ),

END_DATADESC()

acttable_t	CWeaponGlock::m_acttable[] = 
{
	{ ACT_IDLE,						ACT_IDLE_PISTOL,				true },
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_PISTOL,			true },
	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_PISTOL,		true },
	{ ACT_RELOAD,					ACT_RELOAD_PISTOL,				true },
	{ ACT_WALK_AIM,					ACT_WALK_AIM_PISTOL,			true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_PISTOL,				true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_PISTOL,true },
	{ ACT_RELOAD_LOW,				ACT_RELOAD_PISTOL_LOW,			false },
	{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_PISTOL_LOW,	false },
	{ ACT_COVER_LOW,				ACT_COVER_PISTOL_LOW,			false },
	{ ACT_RANGE_AIM_LOW,			ACT_RANGE_AIM_PISTOL_LOW,		false },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_PISTOL,		false },
	{ ACT_WALK,						ACT_WALK_PISTOL,				false },
	{ ACT_RUN,						ACT_RUN_PISTOL,					false },
};


IMPLEMENT_ACTTABLE( CWeaponGlock );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponGlock::CWeaponGlock( void )
{
	m_flSoonestPrimaryAttack = gpGlobals->curtime;
	m_flAccuracyPenalty = 0.0f;

	m_fMinRange1		= 24;
	m_fMaxRange1		= 1500;
	m_fMinRange2		= 24;
	m_fMaxRange2		= 200;

	m_bFiresUnderwater	= true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponGlock::Precache( void )
{
	PrecacheParticleSystem( "muzzle_smoke" );
	PrecacheParticleSystem( "weapon_glock_muzzleflash" );
	PrecacheParticleSystem( "hl2mmod_muzzleflash_npc_pistol" );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeaponGlock::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	switch( pEvent->event )
	{
		case EVENT_WEAPON_PISTOL_FIRE:
		{
			Vector vecShootOrigin, vecShootDir;
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

void CWeaponGlock::FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir )
{
	CSoundEnt::InsertSound( SOUND_COMBAT|SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_PISTOL, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy() );

	WeaponSound( SINGLE_NPC );
	pOperator->FireBullets( 1, vecShootOrigin, vecShootDir, VECTOR_CONE_PRECALCULATED, MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2 );
	//pOperator->DoMuzzleFlash();
	m_iClip1 = m_iClip1 - 1;

	Vector vecShootOrigin2;  //The origin of the shot 
	QAngle	angShootDir2;    //The angle of the shot

	//We need to figure out where to place the particle effect, so look up where the muzzle is
	GetAttachment( LookupAttachment( "muzzle" ), vecShootOrigin2, angShootDir2 );

	//pOperator->DoMuzzleFlash();
	DispatchParticleEffect( "hl2mmod_muzzleflash_npc_pistol", vecShootOrigin2, angShootDir2);
	m_iClip1 = m_iClip1 - 1;

	// LMAO WHY
}

//-----------------------------------------------------------------------------
// Purpose: Some things need this. (e.g. the new Force(X)Fire inputs or blindfire actbusy)
//-----------------------------------------------------------------------------
void CWeaponGlock::Operator_ForceNPCFire( CBaseCombatCharacter *pOperator, bool bSecondary )
{
	// Ensure we have enough rounds in the clip
	m_iClip1++;

	Vector vecShootOrigin, vecShootDir;
	QAngle	angShootDir;
	GetAttachment( LookupAttachment( "muzzle" ), vecShootOrigin, angShootDir );
	AngleVectors( angShootDir, &vecShootDir );
	FireNPCPrimaryAttack( pOperator, vecShootOrigin, vecShootDir );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponGlock::DryFire( void )
{
	WeaponSound( EMPTY );
	SendWeaponAnim( ACT_VM_DRYFIRE );
	
	m_flSoonestPrimaryAttack	= gpGlobals->curtime + GLOCK_FASTEST_DRY_REFIRE_TIME;
	m_flNextPrimaryAttack		= gpGlobals->curtime + SequenceDuration();
}

//void CWeaponGlock::DrawHitmarker( void )
//{
//	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
//
//	if ( pPlayer == NULL )
//		return;
//
//#ifndef CLIENT_DLL
//	CSingleUserRecipientFilter filter( pPlayer );
//	UserMessageBegin( filter, "ShowHitmarker" );
//	WRITE_BYTE( 1 );
//	MessageEnd();
//#endif
//}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponGlock::PrimaryAttack( void )
{
	if ( ( gpGlobals->curtime - m_flLastAttackTime ) > 0.5f )
	{
		m_nNumShotsFired = 0;
	}
	else
	{
		m_nNumShotsFired++;
	}

	m_flLastAttackTime = gpGlobals->curtime;
	m_flSoonestPrimaryAttack = gpGlobals->curtime + GLOCK_FASTEST_REFIRE_TIME;
	CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), SOUNDENT_VOLUME_PISTOL, 0.2, GetOwner() );

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if( pOwner )
	{
		// Each time the player fires the pistol, reset the view punch. This prevents
		// the aim from 'drifting off' when the player fires very quickly. This may
		// not be the ideal way to achieve this, but it's cheap and it works, which is
		// great for a feature we're evaluating. (sjb)
		pOwner->ViewPunchReset();
	}

	BaseClass::PrimaryAttack();

	// Draw effect
	DispatchParticleEffect( "weapon_glock_muzzleflash", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), "muzzle", true);

	// Add an accuracy penalty which can move past our maximum penalty time if we're really spastic
	m_flAccuracyPenalty += GLOCK_ACCURACY_SHOT_PENALTY_TIME;

	m_iPrimaryAttacks++;
	gamestats->Event_WeaponFired( pOwner, true, GetClassname() );

	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if ( !pPlayer )
		return;

	// Set up the vectors and traceline
	trace_t tr;
	Vector vecStart, vecStop, vecDir;

	// Get the angles
	AngleVectors( pPlayer->EyeAngles(), &vecDir );

	// Get the vectors
	vecStart = pPlayer->Weapon_ShootPosition();
	vecStop = vecStart + vecDir * MAX_TRACE_LENGTH;

	// Do the TraceLine
	UTIL_TraceLine( vecStart, vecStop, MASK_ALL, pPlayer, COLLISION_GROUP_NONE, &tr );

	// Check to see if we hit an NPC
	if (tr.m_pEnt)
	{
		if (tr.m_pEnt->IsNPC())
		{
#ifndef CLIENT_DLL
			// Light Kill: Draw ONLY if we hit an enemy NPC
			if (pPlayer->GetDefaultRelationshipDisposition(tr.m_pEnt->Classify()) != D_HT)
			{
				//DevMsg( "Neutral NPC!\n" );
			}
			else
			{
				//DrawHitmarker();
			}
#endif
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponGlock::UpdatePenaltyTime( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return;

	// Check our penalty time decay
	if ( ( ( pOwner->m_nButtons & IN_ATTACK ) == false ) && ( m_flSoonestPrimaryAttack < gpGlobals->curtime ) )
	{
		m_flAccuracyPenalty -= gpGlobals->frametime;
		m_flAccuracyPenalty = clamp( m_flAccuracyPenalty, 0.0f, GLOCK_ACCURACY_MAXIMUM_PENALTY_TIME );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponGlock::ItemPreFrame( void )
{
	UpdatePenaltyTime();

	BaseClass::ItemPreFrame();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponGlock::ItemBusyFrame( void )
{
	UpdatePenaltyTime();

	BaseClass::ItemBusyFrame();
}

void CWeaponGlock::ItemPostFrame(void)
{
	BaseClass::ItemPostFrame();

	CBasePlayer* pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	// Allow a refire as fast as the player can click
	if (!(m_bInReload))
	{
		// Check for secondary fire
		bool bSecondaryAttack = ((pOwner->m_nButtons & IN_ATTACK2) != 0);

		if (bSecondaryAttack && (m_flNextPrimaryAttack < gpGlobals->curtime))
		{
			SecondaryAttack();
		}
		else if ((pOwner->m_nButtons & IN_ATTACK) && (m_flNextPrimaryAttack < gpGlobals->curtime) && (m_iClip1 <= 0))
		{
			DryFire();
		}
	}

	// This new code going to add dynamic smoke out from the muzzle when ammo = 0
	if (!(pOwner->GetWaterLevel() == 3))
	{
		if (m_iClip1 <= 0)
		{
			if (m_bInReload)
			{
				DispatchParticleEffect("muzzle_smoke", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), "muzzle", true);
			}
		}
	}
}

void CWeaponGlock::SecondaryAttack(void)
{
	//// Check if we can fire
	//if (!CanAttack())
	//	return;

	// Set next secondary attack time (1 second delay)
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + 0.2f;

	// Fire the weapon
	PrimaryAttack();

	// Apply accuracy penalty for rapid fire
	m_flAccuracyPenalty += GLOCK_ACCURACY_SHOT_PENALTY_TIME * 0.5f; // Adjust as needed

	// Play weapon sound for secondary fire (rapid fire)
	WeaponSound(SINGLE_NPC);

	// Add view kick for secondary fire
	AddViewKick();

	// Increment the number of shots fired
	m_nNumShotsFired++;

	// Update game stats
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer)
	{
		gamestats->Event_WeaponFired(pPlayer, true, GetClassname());
	}
}



//-----------------------------------------------------------------------------
// Purpose: 
// Output : int
//-----------------------------------------------------------------------------
Activity CWeaponGlock::GetPrimaryAttackActivity( void )
{
	return ACT_VM_PRIMARYATTACK;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CWeaponGlock::Reload( void )
{
	bool fRet = DefaultReload( GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD );
	if ( fRet )
	{
		WeaponSound( RELOAD );
		m_flAccuracyPenalty = 0.0f;
	}
	return fRet;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponGlock::AddViewKick(void)
{
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer == NULL)
		return;

	QAngle	viewPunch;

	viewPunch.x = random->RandomFloat(0.25f, 0.5f);
	viewPunch.y = random->RandomFloat(-.6f, .6f);
	viewPunch.z = 0.0f;

	//Add it to the view punch
	pPlayer->ViewPunch(QAngle(random->RandomFloat(-6.0f, -3.0f), random->RandomFloat(-2.0f, 2.0f), 0.0f));
}
