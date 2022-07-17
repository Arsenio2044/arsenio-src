//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#include "cbase.h"
#include "basehlcombatweapon.h"
#include "player.h"
#include "gamerules.h"
#include "ammodef.h"
#include "mathlib/mathlib.h"
#include "in_buttons.h"
#include "soundent.h"
#include "animation.h"
#include "ai_condition.h"
#include "basebludgeonweapon.h"
#include "ndebugoverlay.h"
#include "te_effect_dispatch.h"
#include "rumble_shared.h"
#include "gamestats.h"

#include "doors.h" // for surge attack magic
#include "props.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_SERVERCLASS_ST( CBaseHLBludgeonWeapon, DT_BaseHLBludgeonWeapon )
END_SEND_TABLE()

#define BLUDGEON_HULL_DIM		16

static const Vector g_bludgeonMins(-BLUDGEON_HULL_DIM,-BLUDGEON_HULL_DIM,-BLUDGEON_HULL_DIM);
static const Vector g_bludgeonMaxs(BLUDGEON_HULL_DIM,BLUDGEON_HULL_DIM,BLUDGEON_HULL_DIM);

static ConVar quick_melee_cutoff( "quick_melee_cutoff", "0.25" ); // minimum time before switching back to gun
static ConVar slash_attack_range( "slash_attack_range", "66" );

static ConVar surge_attack_max_speed( "surge_attack_max_speed", "525" );
static ConVar surge_attack_slide_speed( "surge_attack_slide_speed", "420" );
static ConVar surge_attack_time( "surge_attack_time", "0.5" );

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CBaseHLBludgeonWeapon::CBaseHLBludgeonWeapon()
{
	m_bFiresUnderwater = true;
}

//-----------------------------------------------------------------------------
// Purpose: Spawn the weapon
//-----------------------------------------------------------------------------
void CBaseHLBludgeonWeapon::Spawn( void )
{
	m_fMinRange1	= 0;
	m_fMinRange2	= 0;
	m_fMaxRange1	= 64;
	m_fMaxRange2	= 64;
	//Call base class first
	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: Precache the weapon
//-----------------------------------------------------------------------------
void CBaseHLBludgeonWeapon::Precache( void )
{
	//Call base class first
	BaseClass::Precache();
	PrecacheScriptSound( "Weapon_Crowbar.Double" );
}

int CBaseHLBludgeonWeapon::CapabilitiesGet()
{ 
	return bits_CAP_WEAPON_MELEE_ATTACK1; 
}

int CBaseHLBludgeonWeapon::WeaponMeleeAttack1Condition( float flDot, float flDist )
{
	if (flDist > 64)
	{
		return COND_TOO_FAR_TO_ATTACK;
	}
	else if (flDot < 0.7)
	{
		return COND_NOT_FACING_ATTACK;
	}

	return COND_CAN_MELEE_ATTACK1;
}

//------------------------------------------------------------------------------
// Purpose : Update weapon
//------------------------------------------------------------------------------
void CBaseHLBludgeonWeapon::ItemPostFrame( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return;

	if (m_flNextSecondaryAttack <= gpGlobals->curtime) {
		pOwner->m_nMeleeState =
			(m_nQuick == QMELEE_DONE) ? MELEE_DONE : MELEE_NO;

	}

	if (m_nQuick == QMELEE_DONE || m_nQuick == QMELEE_NO)
	{
		m_nSurgeAttack = false;
	}

	if ( (pOwner->m_nButtons & IN_ATTACK) && (m_flNextPrimaryAttack <= gpGlobals->curtime) )
	{
		if (m_nQuick == QMELEE_NO || m_nQuick == QMELEE_DONE)
		{
			PrimaryAttack(); 
		}
		else
		{
			SurgeAttack();
		}
		m_bSwingOnRelease = false; // already did
	} 
	else if ((pOwner->m_nButtons & IN_ATTACK2) && (m_flNextSecondaryAttack <= gpGlobals->curtime))
	{
		SecondaryAttack(); 
		m_bSwingOnRelease = false;
	}
	else if ( m_nQuick == QMELEE_DONE && m_flNextQuickAttackCutoff < gpGlobals->curtime )
	{
		pOwner->Weapon_Switch( pOwner->Weapon_GetLast() );
		SetQuickMelee( QMELEE_NO );
		
	} 
	else 
	{
		WeaponIdle();
		return;
	}
}

//------------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CBaseHLBludgeonWeapon::PrimaryAttack()
{
	Swing( false );
}

//------------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CBaseHLBludgeonWeapon::SecondaryAttack()
{
	SlashAttack();
}


//------------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CBaseHLBludgeonWeapon::QuickAttack()
{
	Swing( false );
}

//------------------------------------------------------------------------------
// Purpose: Implement impact function
//------------------------------------------------------------------------------
void CBaseHLBludgeonWeapon::Hit( trace_t &traceHit, Activity nHitActivity, bool bIsSecondary )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	
	//Do view kick
	AddViewKick();

	//Make sound for the AI
	CSoundEnt::InsertSound( SOUND_BULLET_IMPACT, traceHit.endpos, 400, 0.2f, pPlayer );

	// This isn't great, but it's something for when the crowbar hits.
	pPlayer->RumbleEffect( RUMBLE_AR2, 0, RUMBLE_FLAG_RESTART );

	CBaseEntity	*pHitEntity = traceHit.m_pEnt;

	//Apply damage to a hit target
	if ( pHitEntity != NULL )
	{
		Vector hitDirection;
		pPlayer->EyeVectors( &hitDirection, NULL, NULL );
		VectorNormalize( hitDirection );

		float dmg = GetDamageForActivity( nHitActivity );
		CTakeDamageInfo info( GetOwner(), GetOwner(), dmg, DMG_CLUB );

		if( pPlayer && pHitEntity->IsNPC() )
		{
			// If bonking an NPC, adjust damage.
			info.AdjustPlayerDamageInflictedForSkillLevel();
		}
		float forceScale = bIsSecondary ? 3.0f : 1.0f;
		CalculateMeleeDamageForce( &info, hitDirection, traceHit.endpos, forceScale );

		pHitEntity->DispatchTraceAttack( info, hitDirection, &traceHit ); 
		ApplyMultiDamage();

		// Now hit all triggers along the ray that... 
		TraceAttackToTriggers( info, traceHit.startpos, traceHit.endpos, hitDirection );

		if ( ToBaseCombatCharacter( pHitEntity ) )
		{
			gamestats->Event_WeaponHit( pPlayer, !bIsSecondary, GetClassname(), info );
		}

		// Special impact sound, loud and clear
		if (IsPlayer() && traceHit.m_pEnt &&
			traceHit.m_pEnt->IsCombatCharacter())
		{
			float flSoundDur;
			if (traceHit.m_pEnt->BloodColor() != DONT_BLEED &&
				traceHit.m_pEnt->BloodColor() != BLOOD_COLOR_MECH)
			{
				EmitSound( "Flesh.BulletImpact", 0.0, &flSoundDur );
			}
			else if (traceHit.m_pEnt->ClassMatches( "npc_hunter" ) ||
				traceHit.m_pEnt->ClassMatches( "npc_strider" ))
			{
				EmitSound( "Biomech.BulletImpact", 0.0, &flSoundDur );
			}
			else
			{
				EmitSound( "Mech.BulletImpact", 0.0, &flSoundDur );
			}
		}
	}

	// Apply an impact effect
	ImpactEffect( traceHit );
}

Activity CBaseHLBludgeonWeapon::ChooseIntersectionPointAndActivity( trace_t &hitTrace, const Vector &mins, const Vector &maxs, CBasePlayer *pOwner )
{
	int			i, j, k;
	float		distance;
	const float	*minmaxs[2] = {mins.Base(), maxs.Base()};
	trace_t		tmpTrace;
	Vector		vecHullEnd = hitTrace.endpos;
	Vector		vecEnd;

	distance = 1e6f;
	Vector vecSrc = hitTrace.startpos;

	vecHullEnd = vecSrc + ((vecHullEnd - vecSrc)*2);
	UTIL_TraceLine( vecSrc, vecHullEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &tmpTrace );
	if ( tmpTrace.fraction == 1.0 )
	{
		for ( i = 0; i < 2; i++ )
		{
			for ( j = 0; j < 2; j++ )
			{
				for ( k = 0; k < 2; k++ )
				{
					vecEnd.x = vecHullEnd.x + minmaxs[i][0];
					vecEnd.y = vecHullEnd.y + minmaxs[j][1];
					vecEnd.z = vecHullEnd.z + minmaxs[k][2];

					UTIL_TraceLine( vecSrc, vecEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &tmpTrace );
					if ( tmpTrace.fraction < 1.0 )
					{
						float thisDistance = (tmpTrace.endpos - vecSrc).Length();
						if ( thisDistance < distance )
						{
							hitTrace = tmpTrace;
							distance = thisDistance;
						}
					}
				}
			}
		}
	}
	else
	{
		hitTrace = tmpTrace;
	}


	return ACT_VM_HITCENTER;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &traceHit - 
//-----------------------------------------------------------------------------
bool CBaseHLBludgeonWeapon::ImpactWater( const Vector &start, const Vector &end )
{
	//FIXME: This doesn't handle the case of trying to splash while being underwater, but that's not going to look good
	//		 right now anyway...
	
	// We must start outside the water
	if ( UTIL_PointContents( start ) & (CONTENTS_WATER|CONTENTS_SLIME))
		return false;

	// We must end inside of water
	if ( !(UTIL_PointContents( end ) & (CONTENTS_WATER|CONTENTS_SLIME)))
		return false;

	trace_t	waterTrace;

	UTIL_TraceLine( start, end, (CONTENTS_WATER|CONTENTS_SLIME), GetOwner(), COLLISION_GROUP_NONE, &waterTrace );

	if ( waterTrace.fraction < 1.0f )
	{
		CEffectData	data;

		data.m_fFlags  = 0;
		data.m_vOrigin = waterTrace.endpos;
		data.m_vNormal = waterTrace.plane.normal;
		data.m_flScale = 8.0f;

		// See if we hit slime
		if ( waterTrace.contents & CONTENTS_SLIME )
		{
			data.m_fFlags |= FX_WATER_IN_SLIME;
		}

		DispatchEffect( "watersplash", data );			
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseHLBludgeonWeapon::ImpactEffect( trace_t &traceHit )
{
	// See if we hit water (we don't do the other impact effects in this case)
	if ( ImpactWater( traceHit.startpos, traceHit.endpos ) )
		return;

	//FIXME: need new decals
	UTIL_ImpactTrace( &traceHit, DMG_CLUB );
}


//------------------------------------------------------------------------------
// Purpose : Starts the swing of the weapon and determines the animation
// Input   : bIsSecondary - is this a secondary attack?
//------------------------------------------------------------------------------
void CBaseHLBludgeonWeapon::Swing( int bIsSecondary )
{
	trace_t traceHit;

	// Try a ray
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return;

	pOwner->RumbleEffect( RUMBLE_CROWBAR_SWING, 0, RUMBLE_FLAG_RESTART );

	Vector swingStart = pOwner->Weapon_ShootPosition( );
	Vector forward;

	forward = pOwner->GetAutoaimVector( AUTOAIM_SCALE_DEFAULT, GetRange() );

	Vector swingEnd = swingStart + forward * GetRange();
	UTIL_TraceLine( swingStart, swingEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit );
	Activity nHitActivity = ACT_VM_HITCENTER;

	// Like bullets, bludgeon traces have to trace against triggers.
	CTakeDamageInfo triggerInfo( GetOwner(), GetOwner(), GetDamageForActivity( nHitActivity ), DMG_CLUB );
	triggerInfo.SetDamagePosition( traceHit.startpos );
	triggerInfo.SetDamageForce( forward );
	TraceAttackToTriggers( triggerInfo, traceHit.startpos, traceHit.endpos, forward );

	if ( traceHit.fraction == 1.0 )
	{
		float bludgeonHullRadius = 1.732f * BLUDGEON_HULL_DIM;  // hull is +/- 16, so use cuberoot of 2 to determine how big the hull is from center to the corner point

		// Back off by hull "radius"
		swingEnd -= forward * bludgeonHullRadius;

		UTIL_TraceHull( swingStart, swingEnd, g_bludgeonMins, g_bludgeonMaxs, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit );
		if ( traceHit.fraction < 1.0 && traceHit.m_pEnt )
		{
			Vector vecToTarget = traceHit.m_pEnt->GetAbsOrigin() - swingStart;
			VectorNormalize( vecToTarget );

			float dot = vecToTarget.Dot( forward );

			// YWB:  Make sure they are sort of facing the guy at least...
			if ( dot < 0.70721f )
			{
				// Force amiss
				traceHit.fraction = 1.0f;
			}
			else
			{
				nHitActivity = ChooseIntersectionPointAndActivity( traceHit, g_bludgeonMins, g_bludgeonMaxs, pOwner );
			}
		}
	}

	if ( !bIsSecondary )
	{
		m_iPrimaryAttacks++;
	} 
	else 
	{
		m_iSecondaryAttacks++;
	}

	gamestats->Event_WeaponFired( pOwner, !bIsSecondary, GetClassname() );

	// -------------------------
	//	Miss
	// -------------------------
	if ( traceHit.fraction == 1.0f )
	{
		nHitActivity = bIsSecondary ? ACT_VM_MISSCENTER2 : ACT_VM_MISSCENTER;

		// We want to test the first swing again
		Vector testEnd = swingStart + forward * GetRange();
		
		// See if we happened to hit water
		ImpactWater( swingStart, testEnd );
	}
	else
	{
		Hit( traceHit, nHitActivity, bIsSecondary ? true : false );
	}

	// Send the anim
	SendWeaponAnim( nHitActivity );

	//Setup our next attack times
	m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
	m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();

	if (m_nQuick > QMELEE_NO)
	{
		m_flNextQuickAttackCutoff = gpGlobals->curtime + quick_melee_cutoff.GetFloat();
	}

	//Play swing sound
	WeaponSound( SINGLE );
}


//------------------------------------------------------------------------------
// Purpose : Basically swing but if they don't hit anything nothing happens
//------------------------------------------------------------------------------
void CBaseHLBludgeonWeapon::TestSwing( void )
{
	trace_t traceHit;

	// Try a ray
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if (!pOwner)
		return;

	Vector swingStart = pOwner->Weapon_ShootPosition();
	Vector forward;

	forward = pOwner->GetAutoaimVector( AUTOAIM_SCALE_DEFAULT, GetRange() );

	Vector swingEnd = swingStart + forward * GetRange();
	UTIL_TraceLine( swingStart, swingEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit );
	Activity nHitActivity = ACT_VM_HITCENTER;

	// Like bullets, bludgeon traces have to trace against triggers.
	CTakeDamageInfo triggerInfo( GetOwner(), GetOwner(), GetDamageForActivity( nHitActivity ), DMG_CLUB );
	triggerInfo.SetDamagePosition( traceHit.startpos );
	triggerInfo.SetDamageForce( forward );
	TraceAttackToTriggers( triggerInfo, traceHit.startpos, traceHit.endpos, forward );

	if (traceHit.fraction == 1.0)
	{
		float bludgeonHullRadius = 1.732f * BLUDGEON_HULL_DIM;  // hull is +/- 16, so use cuberoot of 2 to determine how big the hull is from center to the corner point

		// Back off by hull "radius"
		swingEnd -= forward * bludgeonHullRadius;

		UTIL_TraceHull( swingStart, swingEnd, g_bludgeonMins, g_bludgeonMaxs, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit );
		if (traceHit.fraction < 1.0 && traceHit.m_pEnt)
		{
			Vector vecToTarget = traceHit.m_pEnt->GetAbsOrigin() - swingStart;
			VectorNormalize( vecToTarget );

			float dot = vecToTarget.Dot( forward );

			// YWB:  Make sure they are sort of facing the guy at least...
			if (dot < 0.70721f)
			{
				// Force a miss
				traceHit.fraction = 1.0f;
			}
			else
			{
				nHitActivity = ChooseIntersectionPointAndActivity( traceHit, g_bludgeonMins, g_bludgeonMaxs, pOwner );
			}
		}
	}

	// -------------------------
	//	Miss
	// -------------------------
	if (traceHit.fraction == 1.0f)
	{
		return;
	}
	else
	{
		pOwner->m_nMeleeState = MELEE_SURGE_HIT;
		Hit( traceHit, nHitActivity, true );
		pOwner->m_nMeleeState = MELEE_DONE;
	}

	// Send the anim
	SendWeaponAnim( nHitActivity );

	//Setup our next attack times
	m_flNextPrimaryAttack = gpGlobals->curtime + (1.5f * GetFireRate());
	m_flNextSecondaryAttack = gpGlobals->curtime + (1.25f * SequenceDuration());

	if (m_nQuick > QMELEE_NO)
	{
		m_flNextQuickAttackCutoff = gpGlobals->curtime + quick_melee_cutoff.GetFloat();
	}

	//Play swing sound
	WeaponSound( SINGLE );
}


// Powerful horizontal slash that hits everything in front of us
void CBaseHLBludgeonWeapon::SlashAttack( void )
{
	
	CBasePlayer* pPlayer = ToBasePlayer( GetOwner() );
	if (!pPlayer)
		return;

	pPlayer->RumbleEffect( RUMBLE_CROWBAR_SWING, 0, RUMBLE_FLAG_RESTART );

	Vector slashStart = pPlayer->Weapon_ShootPosition();

	// There is bound to be a more efficient way to do this, but it's a one-off (per frame, anyway)
	CBaseEntity *pSearch[32];
	int nNumEnemies = UTIL_EntitiesInSphere( pSearch, ARRAYSIZE( pSearch ), 
		slashStart, slash_attack_range.GetFloat(), 0 );

	Vector forward = pPlayer->GetAutoaimVector( AUTOAIM_SCALE_DEFAULT, GetRange() );
	Activity nHitActivity = ACT_VM_HITCENTER;
	for (int i = 0; i < nNumEnemies; i++)
	{
		// We only care about solids
		if (pSearch[i] == NULL || !pSearch[i]->IsSolid())
			continue;

		Vector impactDir = pSearch[i]->WorldSpaceCenter() - slashStart;

		float dot = impactDir.Dot( forward );

		// Have to be in front, even if only slightly
		if (dot < 0.1f)
			continue;

		trace_t traceHit;

		UTIL_TraceLine( slashStart, pSearch[i]->WorldSpaceCenter(),
			MASK_SHOT_HULL, pPlayer, COLLISION_GROUP_NONE, &traceHit );

		
		if (traceHit.fraction < 1.0 && traceHit.m_pEnt)
		{
			Hit( traceHit, nHitActivity, true );
		}
	}

	// tell the player class we're in a slash attack
	// (this triggers rotating the viewmodel)
	pPlayer->m_nMeleeState = MELEE_SLASH;
	// Send the anim
	SendWeaponAnim( ACT_VM_MISSCENTER );

	//Setup our next attack times
	m_flNextPrimaryAttack = gpGlobals->curtime + ( 1.5f * GetFireRate() );
	m_flNextSecondaryAttack = gpGlobals->curtime + (1.25f * SequenceDuration());

	//Play swing sound 
	float duration;
	EmitSound( "Weapon_Crowbar.Double", 0.0, &duration );

	if (m_nQuick > QMELEE_NO)
	{
		m_flNextQuickAttackCutoff = gpGlobals->curtime + quick_melee_cutoff.GetFloat();
	}

}


void CBaseHLBludgeonWeapon::SetQuickMelee( QMELEE_STATE nQuick )
{
	if (m_nQuick == QMELEE_NO && nQuick == QMELEE_DOING)
		m_bSwingOnRelease = true;

	if (nQuick == QMELEE_DONE &&
		(m_flNextQuickAttack <= gpGlobals->curtime) && m_bSwingOnRelease)
	{
		PrimaryAttack();
		m_flNextQuickAttack = gpGlobals->curtime + GetFireRate();
	}

	m_nQuick = nQuick;
	
}

// Cut off the attack animation after the swing is done -
// we don't want to return to the idle position before 
// switching back to a gun
void  CBaseHLBludgeonWeapon::ItemBusyFrame( void )
{
	/*
	if ( m_flNextQuickAttackCutoff <= gpGlobals->curtime )
	{
		// No more time for this animation.
		SetActivity( ACT_VM_IDLE );

		if ( m_nQuick == QMELEE_DONE )
		{
			CBasePlayer* pOwner = ToBasePlayer( GetOwner() );
			ASSERT( pOwner );
			pOwner->Weapon_Switch( pOwner->Weapon_GetLast() );
			pOwner->m_nMeleeState = MELEE_DONE;
			SetQuickMelee( QMELEE_NO );
		}
	}*/
}

// Charge forward and attack
void CBaseHLBludgeonWeapon::SurgeAttack( void )
{
	CBasePlayer* pOwner = ToBasePlayer( GetOwner() );
	if (!pOwner)
		return;

	if ( !m_nSurgeAttack && m_flEndSurgeTime <= gpGlobals->curtime )
	{
		m_nSurgeAttack = true;

		// First time - set timer, play sound etc.
		pOwner->m_nMeleeState = MELEE_SURGE_MOVE;

		
		Vector pos = pOwner->GetAbsOrigin();

		// see how much move we can add
		Vector forward;
		QAngle angle = pOwner->GetAbsAngles();
		AngleVectors( angle, &forward );

		// Limit upward velocity - we do allow some gliding, but not full flying
		forward.z = clamp( forward.z, -1, -0.1f );
		VectorNormalize( forward );

		float speedboost = surge_attack_max_speed.GetFloat();

		if ((pOwner->m_Local.m_bDucked || pOwner->m_Local.m_bDucking) &&
			pOwner->GetGroundEntity() != NULL)
		{
			// crouched on ground, so less speedboost. Triggers a powerslide, 
			// which adds some speed anyway.
			speedboost = surge_attack_slide_speed.GetFloat();
		}

		// Cancel out any lateral movement
		Vector curVel = pOwner->GetAbsVelocity();
		float flVelocity = clamp( curVel.Dot( forward ), -50.0f, 100.0f );

		pOwner->SetLocalVelocity( forward * flVelocity );
		pOwner->VelocityPunch( speedboost * forward );

		m_flEndSurgeTime = gpGlobals->curtime + surge_attack_time.GetFloat();

		pOwner->PlayAirjumpSound( pos );

		return;
	} 

	// In motion - check if time up yet
	if (m_flEndSurgeTime <= gpGlobals->curtime)
	{
		pOwner->m_nMeleeState = MELEE_NO;
		Swing( false );
		m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
		m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
		m_nSurgeAttack = false;
		m_flEndSurgeTime = gpGlobals->curtime + 1.25f;
		return;
	}

	// No change. Keep surging, swing if anything is in range
	TestSwing();
}