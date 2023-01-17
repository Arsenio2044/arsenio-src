//========= Copyright Glitch Software, All rights reserved. ============//
//
// Purpose: The MP5K "Kurz" is a small automatic submachine gun used by the player and various npcs.
// It is slow and has high recoil, but makes up for it in its accuracy and damage.
//=============================================================================//

#include "cbase.h"
#include "basehlcombatweapon.h"
#include "npcevent.h"
#include "basecombatcharacter.h"
#include "ai_basenpc.h"
#include "player.h"
#include "game.h"
#include "in_buttons.h"
#include "grenade_ar2.h"
#include "ai_memory.h"
#include "soundent.h"
#include "rumble_shared.h"
#include "gamestats.h"
#include "particle_parse.h" // BREADMAN - particle muzzle

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define MIN_SPREAD_COMPONENT arsenio_mp5k_min_spread.GetFloat()
#define MAX_SPREAD_COMPONENT arsenio_mp5k_max_spread.GetFloat()

ConVar arsenio_mp5k_altfire_enabled( "arsenio_mp5k_altfire_enabled", "1", FCVAR_NONE, "Allows weapon_mp5k to fire full auto if the player holds down the secondary attack button." );
ConVar arsenio_mp5k_altfire_ammo_modifier( "arsenio_mp5k_altfire_ammo_modifier", "1", FCVAR_NONE, "Multiply the number of bullets per shot by this amount for altfire" );
ConVar arsenio_mp5k_altfire_spread_divisor( "arsenio_mp5k_altfire_spread_divisor", "3", FCVAR_NONE, "How much to divide the spread component for altfire (higher numbers = better sustained accuracy)" );
ConVar arsenio_mp5k_altfire_rate( "arsenio_mp5k_altfire_rate", "0.06", FCVAR_NONE, "weapon_mp5k's full-auto fire rate." );
ConVar arsenio_mp5k_min_spread( "arsenio_mp5k_min_spread", "0.015", FCVAR_NONE, "MP5K minimum fire cone vector component" );
ConVar arsenio_mp5k_max_spread( "arsenio_mp5k_max_spread", "0.075", FCVAR_NONE, "MP5K maximum fire cone vector component" );
ConVar arsenio_mp5k_burst_cycle_rate( "arsenio_mp5k_burst_cycle_rate", "0.2", FCVAR_NONE, "MP5K maximum fire cone vector component" );
ConVar arsenio_mp5k_debug( "arsenio_mp5k_debug", "0", FCVAR_NONE, "Log messages to console about the MP5K spread" );

class CWeaponMP5K : public CHLSelectFireMachineGun
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS( CWeaponMP5K, CHLSelectFireMachineGun );

	CWeaponMP5K();

	DECLARE_SERVERCLASS();
	
	void	AddViewKick( void );
	void	PrimaryAttack(void);

	virtual Vector	GetBulletSpread( WeaponProficiency_t proficiency );

	Vector CalculateBurstAttackSpread();
	void	BurstAttack(int burstSize, float cycleRate, int spentAmmoModifier = 1, float spreadComponentDivisor = 1.0f);
	
	void	SecondaryAttack( void );
	void	ItemPostFrame(void);

	float	GetBurstCycleRate(void) { return arsenio_mp5k_burst_cycle_rate.GetFloat(); };
	int		GetMinBurst() { return 3; }
	int		GetMaxBurst() { return 3; }
	int		GetBurstSize(void) { return 3; };

	bool	Reload( void );

	float	GetFullAutoFireRate( void ) { return arsenio_mp5k_altfire_rate.GetFloat(); }
	int		CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	Activity	GetPrimaryAttackActivity( void );

	const WeaponProficiencyInfo_t *GetProficiencyValues();

	void FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir );
	void Operator_ForceNPCFire( CBaseCombatCharacter  *pOperator, bool bSecondary );
	void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );

	DECLARE_ACTTABLE();

protected:
	float	m_flLastPrimaryAttack;
	float   m_flSpreadComponent;
};

IMPLEMENT_SERVERCLASS_ST(CWeaponMP5K, DT_WeaponMP5K)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_mp5k, CWeaponMP5K );
PRECACHE_WEAPON_REGISTER(weapon_mp5k);

BEGIN_DATADESC( CWeaponMP5K )

	DEFINE_FIELD(m_flSpreadComponent, FIELD_FLOAT ),
	DEFINE_FIELD(m_flLastPrimaryAttack, FIELD_TIME ),

END_DATADESC()

acttable_t	CWeaponMP5K::m_acttable[] = 
{
	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_AR2,			true },
	{ ACT_RELOAD,					ACT_RELOAD_SMG1,				true },
	{ ACT_IDLE,						ACT_IDLE_SMG1,					true },
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_SMG1,			true },

	{ ACT_WALK,						ACT_WALK_RIFLE,					true },
	{ ACT_WALK_AIM,					ACT_WALK_AIM_RIFLE,				true  },
	
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
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_SMG1,	true },
	{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_SMG1_LOW,		true },
	{ ACT_COVER_LOW,				ACT_COVER_SMG1_LOW,				false },
	{ ACT_RANGE_AIM_LOW,			ACT_RANGE_AIM_SMG1_LOW,			false },
	{ ACT_RELOAD_LOW,				ACT_RELOAD_SMG1_LOW,			false },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_SMG1,		true },
};

IMPLEMENT_ACTTABLE(CWeaponMP5K);

//=========================================================
CWeaponMP5K::CWeaponMP5K( )
{
	m_fMinRange1		= 0;// No minimum range. 
	m_fMaxRange1		= 1400;
	m_iBurstSize = 0;
	m_iSecondaryAmmoType = -1;
	m_iClip2 = -1;
	m_flSpreadComponent = MIN_SPREAD_COMPONENT;

	m_bAltFiresUnderwater = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMP5K::FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir )
{
	// FIXME: use the returned number of bullets to account for >10hz firerate
	WeaponSoundRealtime( SINGLE_NPC );

	CSoundEnt::InsertSound( SOUND_COMBAT|SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy() );
	pOperator->FireBullets( 1, vecShootOrigin, vecShootDir, pOperator->GetAttackSpread( this ), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2, entindex(), 0 );
	pOperator->DoMuzzleFlash(); // Changing the shots doesn't help - just blows us up !
	m_flSpreadComponent += (MAX_SPREAD_COMPONENT - MIN_SPREAD_COMPONENT) * (1.0f / (float) GetBurstSize() ); // 1/3 The difference between max and min

	m_iClip1 = m_iClip1 - 1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMP5K::Operator_ForceNPCFire( CBaseCombatCharacter *pOperator, bool bSecondary )
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
void CWeaponMP5K::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	switch( pEvent->event )
	{
	case EVENT_WEAPON_AR2:
	case EVENT_WEAPON_SMG2:
	case EVENT_WEAPON_SMG1:
		{
			Vector vecShootOrigin, vecShootDir;
			QAngle angDiscard;

			// Support old style attachment point firing
			if ((pEvent->options == NULL) || (pEvent->options[0] == '\0') || (!pOperator->GetAttachment(pEvent->options, vecShootOrigin, angDiscard)))
			{
				vecShootOrigin = pOperator->Weapon_ShootPosition();
			}

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
Activity CWeaponMP5K::GetPrimaryAttackActivity( void )
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
//-----------------------------------------------------------------------------
bool CWeaponMP5K::Reload( void )
{
	bool fRet;

	fRet = DefaultReload( GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD );
	if ( fRet )
	{
		// Undo whatever the reload process has done to our secondary
		// attack timer. We allow you to interrupt reloading to fire
		// a grenade.
		m_flNextSecondaryAttack = GetOwner()->m_flNextAttack = fCacheTime;

		WeaponSound( RELOAD );
	}

	return fRet;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMP5K::AddViewKick( void )
{
	#define	EASY_DAMPEN			2.5f	// BREADMAN
	#define	MAX_VERTICAL_KICK	22.0f	//Degrees - was 1.0
	#define	SLIDE_LIMIT			4.0f	//Seconds - was 2.0
	
	//Get the view kick
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if ( pPlayer == NULL )
		return;

	DoMachineGunKick( pPlayer, EASY_DAMPEN, MAX_VERTICAL_KICK, m_fFireDuration, SLIDE_LIMIT );
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CWeaponMP5K::PrimaryAttack(void)
{
	BurstAttack( GetBurstSize(), GetBurstCycleRate(), 1, (float)GetBurstSize() );
}

Vector CWeaponMP5K::GetBulletSpread( WeaponProficiency_t proficiency )
{
	Vector baseSpread = CalculateBurstAttackSpread();

	const WeaponProficiencyInfo_t *pProficiencyValues = GetProficiencyValues();
	float flModifier = (pProficiencyValues)[proficiency].spreadscale;
	return (baseSpread * flModifier);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMP5K::SecondaryAttack(void)
{
	if ( arsenio_mp5k_altfire_enabled.GetBool() )
	{
		BurstAttack( 1, GetFullAutoFireRate(), arsenio_mp5k_altfire_ammo_modifier.GetInt(), arsenio_mp5k_altfire_spread_divisor.GetFloat() );
	}
}

Vector CWeaponMP5K::CalculateBurstAttackSpread()
{
	float l_flSpreadRegen = ((gpGlobals->curtime - m_flLastPrimaryAttack) / GetBurstCycleRate()) * (MAX_SPREAD_COMPONENT - MIN_SPREAD_COMPONENT) / 2;
	m_flSpreadComponent -= l_flSpreadRegen;

	// Minimum spread
	if (m_flSpreadComponent < MIN_SPREAD_COMPONENT)
		m_flSpreadComponent = MIN_SPREAD_COMPONENT;

	// Maximum spread
	if (m_flSpreadComponent > MAX_SPREAD_COMPONENT)
		m_flSpreadComponent = MAX_SPREAD_COMPONENT;

	if (arsenio_mp5k_debug.GetBool())
	{
		DevMsg( "CWeaponMP5K::BurstAttack(): MP5K spread component: %f\n", m_flSpreadComponent );
	}

	return Vector( m_flSpreadComponent, m_flSpreadComponent, m_flSpreadComponent );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMP5K::BurstAttack( int burstSize, float cycleRate, int spentAmmoModifier, float spreadComponentDivisor )
{
	// Bursts always use the weapon's fire rate
	float fireRate =  GetFireRate();

	if ( m_bFireOnEmpty )
	{
		return;
	}

	if (!GetOwner())
	{
		Msg( "Error: MP5K has NULL owner!" );
		return;
	}

	// Only the player fires this way so we can cast
	CBasePlayer * pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;

	m_nShotsFired++;

	// Send the animation early to properly handle recoil animation
	SendWeaponAnim( GetPrimaryAttackActivity() );

	pPlayer->DoMuzzleFlash();

	// To make the firing framerate independent, we may have to fire more than one bullet here on low-framerate systems, 
	// especially if the weapon we're firing has a really fast rate of fire.
	int iBulletsToFire = 0;

	// If the last time fired was longer ago than the cycle rate, reset the burst count
	if (gpGlobals->curtime - m_flLastPrimaryAttack >= cycleRate)
	{
		m_iBurstSize = 0;
	}

	// MUST call sound before removing a round from the clip of a CHLMachineGun
	while (m_flNextPrimaryAttack <= gpGlobals->curtime)
	{
		WeaponSound(SINGLE, m_flNextPrimaryAttack);

		// Add the bullets to fire to the burst count
		m_iBurstSize++;

		// Increase weapon spread;
		m_flSpreadComponent += (MAX_SPREAD_COMPONENT - MIN_SPREAD_COMPONENT) / spreadComponentDivisor; // 1/3 The difference between max and min

		// If the burst count is greater than the burst size, wait for the cycle rate and adjust
		if (m_iBurstSize >= burstSize) {
			m_iBurstSize = 0;
			m_nShotsFired = burstSize > 1 ? 0 : m_nShotsFired; // Reset the shots fired counter so the correct activity plays
			m_flNextPrimaryAttack = m_flNextPrimaryAttack + cycleRate;
			m_flNextSecondaryAttack = m_flNextPrimaryAttack + cycleRate; // MP5K shares primary attack between primary and secondary
		}
		else {
			m_flNextPrimaryAttack = m_flNextPrimaryAttack + fireRate;
			m_flNextSecondaryAttack = m_flNextPrimaryAttack + fireRate; // MP5K shares primary attack between primary and secondary
		}

		iBulletsToFire++;
	}

	// Make sure we don't fire more than the amount in the clip, if this weapon uses clips
	if (UsesClipsForAmmo1())
	{
		if (iBulletsToFire > m_iClip1)
			iBulletsToFire = m_iClip1;
		m_iClip1 -= MIN(iBulletsToFire * spentAmmoModifier, m_iClip1);
	}

	m_iPrimaryAttacks++;
	gamestats->Event_WeaponFired(pPlayer, true, GetClassname());

	// Fire the bullets
	FireBulletsInfo_t info;
	info.m_iShots = iBulletsToFire;
	info.m_vecSrc = pPlayer->Weapon_ShootPosition();
	info.m_vecDirShooting = pPlayer->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT);
	info.m_vecSpread = GetOwner()->GetAttackSpread( this );
	info.m_flDistance = MAX_TRACE_LENGTH;
	info.m_iAmmoType = m_iPrimaryAmmoType;
	info.m_iTracerFreq = 1;
	FireBullets(info);

	// Update last attack time - need to do this after calculating weapon spread
	m_flLastPrimaryAttack = gpGlobals->curtime;

	//Factor in the view kick
	AddViewKick();

	CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, pPlayer);

	if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}

	pPlayer->SetAnimation(PLAYER_ATTACK1);

	// Register a muzzleflash for the AI
	pPlayer->SetMuzzleFlashTime(gpGlobals->curtime + 0.5);
	SetWeaponIdleTime(gpGlobals->curtime + 3.0f);

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner)
	{
		m_iPrimaryAttacks++;
		gamestats->Event_WeaponFired(pOwner, true, GetClassname());
	}
}

//-----------------------------------------------------------------------------
// Purpose: MP5K overrides ItemPostFrame because the primary and secondary
// fire modes are the same but with slightly different parameters.
//-----------------------------------------------------------------------------
void CWeaponMP5K::ItemPostFrame(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	UpdateAutoFire();

	//Track the duration of the fire
	//FIXME: Check for IN_ATTACK2 as well?
	//FIXME: What if we're calling ItemBusyFrame?
	m_fFireDuration = (pOwner->m_nButtons & IN_ATTACK) || (pOwner->m_nButtons & IN_ATTACK2) ? (m_fFireDuration + gpGlobals->frametime) : 0.0f;
	CheckReload();


	if (((pOwner->m_nButtons & IN_ATTACK) || (pOwner->m_nButtons & IN_ATTACK2)) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
	{
		// Clip empty? Or out of ammo on a no-clip weapon?
		if ((UsesClipsForAmmo1() && m_iClip1 <= 0) || (!UsesClipsForAmmo1() && pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0))
		{
			HandleFireOnEmpty();
		}
		else if (pOwner->GetWaterLevel() == 3 && m_bFiresUnderwater == false)
		{
			// This weapon doesn't fire underwater
			WeaponSound(EMPTY);
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
			return;
		}
		else
		{
			//NOTENOTE: There is a bug with this code with regards to the way machine guns catch the leading edge trigger
			//			on the player hitting the attack key.  It relies on the gun catching that case in the same frame.
			//			However, because the player can also be doing a secondary attack, the edge trigger may be missed.
			//			We really need to hold onto the edge trigger and only clear the condition when the gun has fired its
			//			first shot.  Right now that's too much of an architecture change -- jdw

			// If the firing button was just pressed, or the alt-fire just released, reset the firing time
			//if ((pOwner->m_afButtonPressed & IN_ATTACK) || (pOwner->m_afButtonReleased & IN_ATTACK2))
			{
				m_flNextPrimaryAttack = gpGlobals->curtime;
			}
			if ((pOwner->m_nButtons & IN_ATTACK)) 
			{
				PrimaryAttack();
			}
			else {
				SecondaryAttack();
			}
		}
	}

	// -----------------------
	//  Reload pressed / Clip Empty
	// -----------------------
	if ((pOwner->m_nButtons & IN_RELOAD) && UsesClipsForAmmo1() && !m_bInReload)
	{
		// reload when reload is pressed, or if no buttons are down and weapon is empty.
		Reload();
		m_fFireDuration = 0.0f;
	}

	// -----------------------
	//  No buttons down
	// -----------------------
	if (!((pOwner->m_nButtons & IN_ATTACK) || (pOwner->m_nButtons & IN_ATTACK2) || (CanReload() && pOwner->m_nButtons & IN_RELOAD)))
	{
		// no fire buttons down or reloading
		if (!ReloadOrSwitchWeapons() && (m_bInReload == false))
		{
			WeaponIdle();
		}
	}

	// Debounce the recoiling counter
	if ((pOwner->m_nButtons & IN_ATTACK) == false && (pOwner->m_nButtons & IN_ATTACK2) == false)
	{
		m_nShotsFired = 0;
	}
}

//-----------------------------------------------------------------------------
const WeaponProficiencyInfo_t *CWeaponMP5K::GetProficiencyValues()
{
	static WeaponProficiencyInfo_t proficiencyTable[] =
	{
		{ 7.0,		0.75 },
	{ 5.00,		0.75 },
	{ 10.0/3.0, 0.75 },
	{ 5.0/3.0,	0.75 },
	{ 1.00,		1.0 },
	};

	COMPILE_TIME_ASSERT( ARRAYSIZE( proficiencyTable ) == WEAPON_PROFICIENCY_PERFECT + 1 );

	return proficiencyTable;
}
