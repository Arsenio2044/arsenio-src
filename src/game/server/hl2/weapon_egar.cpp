
#include "cbase.h"
#include "basehlcombatweapon.h"
#include "basecombatcharacter.h"
#include "movie_explosion.h"
#include "soundent.h"
#include "player.h"
#include "rope.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "explode.h"
#include "util.h"
#include "in_buttons.h"
#include "weapon_EGAR.h"
#include "shake.h"
#include "ai_basenpc.h"
#include "ai_squad.h"
#include "te_effect_dispatch.h"
#include "triggers.h"
#include "smoke_trail.h"
#include "collisionutils.h"
#include "hl2_shareddefs.h"
#include "rumble_shared.h"
#include "gamestats.h"
#include "weapon_egar.h"
// for airboat rockets
#include "vehicle_base.h"

//=============================================================================
// EGAR
//=============================================================================

BEGIN_DATADESC( CWeaponEGAR )
DEFINE_FIELD( m_nRocketState, FIELD_INTEGER ),
DEFINE_FIELD( m_flRocketTime, FIELD_FLOAT ),
DEFINE_FIELD( m_flRocketGenTime, FIELD_FLOAT ),
DEFINE_FIELD( m_nRocketsQueued, FIELD_INTEGER ),
DEFINE_FIELD( m_nRocketsReady, FIELD_INTEGER ),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CWeaponEGAR, DT_WeaponEGAR)
SendPropInt( SENDINFO( m_nRocketsReady ) ),
SendPropInt( SENDINFO( m_nRocketsQueued ) ),
SendPropArray3( SENDINFO_ARRAY3( m_hRocketTargets ), SendPropEHandle( SENDINFO_ARRAY( m_hRocketTargets ) ) )
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_egar, CWeaponEGAR );

PRECACHE_WEAPON_REGISTER( weapon_egar );


acttable_t	CWeaponEGAR::m_acttable[] =
{
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_RPG, true },

	{ ACT_IDLE_RELAXED, ACT_IDLE_RPG_RELAXED, true },
	{ ACT_IDLE_STIMULATED, ACT_IDLE_ANGRY_RPG, true },
	{ ACT_IDLE_AGITATED, ACT_IDLE_ANGRY_RPG, true },

	{ ACT_IDLE, ACT_IDLE_RPG, true },
	{ ACT_IDLE_ANGRY, ACT_IDLE_ANGRY_RPG, true },
	{ ACT_WALK, ACT_WALK_RPG, true },
	{ ACT_WALK_CROUCH, ACT_WALK_CROUCH_RPG, true },
	{ ACT_RUN, ACT_RUN_RPG, true },
	{ ACT_RUN_CROUCH, ACT_RUN_CROUCH_RPG, true },
	{ ACT_COVER_LOW, ACT_COVER_LOW_RPG, true },
};

IMPLEMENT_ACTTABLE( CWeaponEGAR );




static ConVar egar_lpos_f( "egar_lpos_f", "22.0" );
static ConVar egar_lpos_r( "egar_lpos_r", "8.0" );
static ConVar egar_lpos_u( "egar_lpos_u", "0.0" );


static ConVar sv_egar_rocket_range( "sv_egar_rocket_range", "4000" );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CWeaponEGAR::CWeaponEGAR()
{
	// nothing to do
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CWeaponEGAR::~CWeaponEGAR()
{
    // nothing to do
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponEGAR::Precache( void )
{
	BaseClass::Precache();

	PrecacheScriptSound( "Missile.Ignite" );
	PrecacheScriptSound( "Missile.Accelerate" );
	PrecacheScriptSound( "Buttons.snd15_softer" );
	UTIL_PrecacheOther( "homing_missile" );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEvent - 
//			*pOperator - 
//-----------------------------------------------------------------------------
void CWeaponEGAR::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	
    BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponEGAR::HasAnyAmmo( void )
{
	return true; // regenerates ammo, so is never out of ammo
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponEGAR::WeaponShouldBeLowered( void )
{
	return BaseClass::WeaponShouldBeLowered();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
/*void CWeaponEGAR::PrimaryAttack( void )
{
	
}*/

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponEGAR::ItemPostFrame( void )
{
	//BaseClass::ItemPostFrame();

	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if (pPlayer == NULL)
		return;

	const CUserCmd* pCmd = pPlayer->GetCurrentUserCommand();
	if (pCmd == NULL)
		return;

	UpdateRocketState( pCmd, pPlayer->m_afButtonReleased );

}


//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponEGAR::Deploy( void )
{
	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponEGAR::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	// Clear the rocket targets 
	for (int i = 0; i < EGAR_ROCKETS; i++) {

		// If this was an info_target we added, remove it
		if (m_hRocketTargets[i] &&
			Q_strcmp( m_hRocketTargets[i]->GetEntityName().ToCStr(), "egar_target" ) == 0)
		{
			inputdata_t input;
			input.pActivator = this;
			input.pCaller = this;
			m_hRocketTargets[i]->InputKill( input );
		}

		m_hRocketTargets.Set( i, NULL );
		m_nRocketsQueued = 0;
	}

	return BaseClass::Holster( pSwitchingTo );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponEGAR::Drop( const Vector &vecVelocity )
{
	BaseClass::Drop( vecVelocity );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
/*bool CWeaponEGAR::Reload( void )
{
	return true;
}*/

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CWeaponEGAR::WeaponLOSCondition( const Vector &ownerPos, const Vector &targetPos, bool bSetConditions )
{
	bool bResult = BaseClass::WeaponLOSCondition( ownerPos, targetPos, bSetConditions );

	if (bResult)
	{
		CAI_BaseNPC* npcOwner = GetOwner()->MyNPCPointer();

		if (npcOwner)
		{
			trace_t tr;

			Vector vecRelativeShootPosition;
			VectorSubtract( npcOwner->Weapon_ShootPosition(), npcOwner->GetAbsOrigin(), vecRelativeShootPosition );
			Vector vecMuzzle = ownerPos + vecRelativeShootPosition;
			Vector vecShootDir = npcOwner->GetActualShootTrajectory( vecMuzzle );

			// Make sure I have a good 10 feet of wide clearance in front, or I'll blow my teeth out.
			AI_TraceHull( vecMuzzle, vecMuzzle + vecShootDir * (10.0f*12.0f), Vector( -24, -24, -24 ), Vector( 24, 24, 24 ), MASK_NPCSOLID, NULL, &tr );

			if (tr.fraction != 1.0f)
				bResult = false;
		}
	}

	return bResult;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : flDot - 
//			flDist - 
// Output : int
//-----------------------------------------------------------------------------
int CWeaponEGAR::WeaponRangeAttack1Condition( float flDot, float flDist )
{

	// Ignore vertical distance when doing our EGAR distance calculations
	CAI_BaseNPC *pNPC = GetOwner()->MyNPCPointer();
	if (pNPC)
	{
		CBaseEntity *pEnemy = pNPC->GetEnemy();
		Vector vecToTarget = (pEnemy->GetAbsOrigin() - pNPC->GetAbsOrigin());
		vecToTarget.z = 0;
		flDist = vecToTarget.Length();
	}

	if (flDist < MIN( m_fMinRange1, m_fMinRange2 ))
		return COND_TOO_CLOSE_TO_ATTACK;

	if (m_flNextPrimaryAttack > gpGlobals->curtime)
		return 0;

	// See if there's anyone in the way!
	CAI_BaseNPC *pOwner = GetOwner()->MyNPCPointer();
	ASSERT( pOwner != NULL );

	if (pOwner)
	{
		// Make sure I don't shoot the world!
		trace_t tr;

		Vector vecMuzzle = pOwner->Weapon_ShootPosition();
		Vector vecShootDir = pOwner->GetActualShootTrajectory( vecMuzzle );

		// Make sure I have a good 10 feet of wide clearance in front, or I'll blow my teeth out.
		AI_TraceHull( vecMuzzle, vecMuzzle + vecShootDir * (10.0f*12.0f), Vector( -24, -24, -24 ), Vector( 24, 24, 24 ), MASK_NPCSOLID, NULL, &tr );

		if (tr.fraction != 1.0)
		{
			return COND_WEAPON_SIGHT_OCCLUDED;
		}
	}

	return COND_CAN_RANGE_ATTACK1;
}


bool CWeaponEGAR::IsRocketTarget( CBaseEntity* ent )
{
	for (int i = 0; i < EGAR_ROCKETS; i++) {

		// If we are directly targeting the entity already, return true
		if (m_hRocketTargets[i].Get() == ent)
			return true;

		// If we are indirectly targeting the entity via an egar_target, return true
		if (m_hRocketTargets[i].Get() &&
			Q_strcmp( STRING( m_hRocketTargets[i].Get()->GetEntityName() ), "egar_target" ) == 0 &&
			m_hRocketTargets[i].Get()->GetParent() == ent)
			return true;

	}
	return false;
}

void CWeaponEGAR::CancelTarget( int nIndex )
{
	bool debug_cancel_target = false;
	if (debug_cancel_target)
		Msg( "Cancelling target %d\n", nIndex );
	

	// If this was an info_target we added, reduce the number of rockets targeting it and
	// remove it entirely if this was the last rocket
	if (m_hRocketTargets[nIndex] &&
		Q_strcmp( m_hRocketTargets[nIndex]->GetEntityName().ToCStr(), "egar_target" ) == 0)
	{
        // Using a probably dumb hack of using the info_target's local YAW to encode the number of 
		// rockets targeting it.
		QAngle ang = m_hRocketTargets[nIndex]->GetLocalAngles();
		int rockets = int( ang[YAW] );

		if (debug_cancel_target)
			Msg( "%d Rocket(s) targeting this egar_target\n", rockets );

		if (rockets > 0)
		{
			ang[YAW] -= 1;
			m_hRocketTargets[nIndex]->SetLocalAngles( ang );
		}
		else {
			inputdata_t input;
			input.pActivator = this;
			input.pCaller = this;
			m_hRocketTargets[nIndex]->InputKill( input );
		}
	}
	m_hRocketTargets.Set( nIndex, NULL );
}

void CWeaponEGAR::QueueAllRockets()
{
	bool debug_queue = false;
	int lastQueued = m_nRocketsQueued - 1;
	if (m_hRocketTargets[lastQueued]) {
		for (; m_nRocketsQueued < m_nRocketsReady; m_nRocketsQueued++) {
			// if the target is an egar_target (info_target named egar_target)
			// increment the YAW to indicate number of rockets using it. This
			// HACK prevents the egar_target from being removed until the last
			// rocket is done with it.
			if (Q_strcmp( STRING( m_hRocketTargets[lastQueued]->GetEntityName() ),
				"egar_target" ) == 0)
			{
				QAngle ang = m_hRocketTargets[lastQueued]->GetLocalAngles();
				ang[YAW]++;
				if (debug_queue)
					Msg( "Rocket target %d has %d rockets targeting it\n", lastQueued, int( ang[YAW] ) );
				m_hRocketTargets[lastQueued]->SetLocalAngles( ang );
			}
			m_hRocketTargets.Set( m_nRocketsQueued, m_hRocketTargets[lastQueued] );
		}
	}
}

bool CWeaponEGAR::FindRocketTarget( EHANDLE &Target )
{
	// Need to search the whole list so we 
	// can exclude multiple entities already targeted
	const CEntInfo *pInfo = g_pEntityList->FirstEntInfo();
	const bool debug = false;
	for (; pInfo; pInfo = pInfo->m_pNext)
	{
		CBaseEntity* ent = dynamic_cast<CBaseEntity*>(pInfo->m_pEntity);

		// Looking for an entity that:
		//  * is within cone of player vision (finite range)
		//  * is an npc (classname matches npc_*), or chopper bomb or 
		//    apc or apc missile
		//  * is not the player or a player ally
		//  * is not already targeted
		// Do the fastest checks first so we only do the slower checks if we have to

		// if already targeted

		if (debug)
			Msg( "\nConsidering a %s,", ent->GetClassname() );

		// if not the kind of entity we want to target
		CBaseCombatCharacter* tgt = ToBaseCombatCharacter( ent );
		bool bcc = false;
		if (tgt)
			bcc = true;

		if (bcc && debug)
			Msg( "It's a basecombatcharacter alright...\n" );
		if (!(bcc ||
			ent->ClassMatches( "prop_vehicle_apc" ) ||
			ent->ClassMatches( "grenade_helicopter" ) ||
			ent->ClassMatches( "npc_gunship" ) ||
			ent->ClassMatches( "npc_cscanner" ) ||
			ent->Classify() == CLASS_COMBINE))
		{
			continue;
		}


		if (debug)
			Msg( "\nSofarsogood\n" );

		// special exclusions
		if (ent->ClassMatches( "player" ) ||
			ent->ClassMatches( "npc_heli_*" ) ||
			ent->ClassMatches( "npc_bullseye*" ) ||
			ent->ClassMatches( "bullseye_strider_focus" ) ||
			ent->ClassMatches( "npc_helicoptersensor" ) ||
			ent->ClassMatches( "helicopter_chunk" ))
		{
			if (debug)
			    Msg( "Matches a special exclusion\n" );
			continue;
		}

		// if friendly
		if (!UTIL_IsPlayerEnemy( ent ))
		{
			continue;
		}

		if (debug)
			Msg( "not ally,\n" );

		if (!ent->ClassMatches( "prop_vehicle_apc" ) &&
			!ent->IsAlive())
		{
			continue;
		}

		if (debug)
			Msg( "either alive or APC," );

		// Don't target anything the rocket can't collide with (sniper is not solid)
		if (!ent->IsSolid() && !ent->ClassMatches("npc_sniper"))
		{
			if (debug)
			    Msg( "Not Solid\n" );
			continue;
		}

		// if already targeted
		if (IsRocketTarget( ent ))
			continue;

		// Okay, this is the kind of thing we want to target. 
		// Is it in the targeting cone?
		CBasePlayer* player = ToBasePlayer( GetOwner() );
		if (!player || !player->FInViewCone( ent ))
		{
			continue;
		}

		if (debug)
			Msg( "in view,\n" );

		// Distance check
		Vector a = ent->GetAbsOrigin();
		Vector b = GetAbsOrigin();
		if (a.IsValid() && b.IsValid() &&
			a.DistTo( b ) > sv_egar_rocket_range.GetFloat())
			continue;


		// Finally check if we have line of sight.
		// This always returns false for APC missiles for some reason.
		CBaseEntity** pBlocker = 0;
		if (!player->FVisible( ent, MASK_VISIBLE, pBlocker ))
		{
			continue;
		}

		if (debug)
			Msg( "visible\n" );
		// Looks like we have a target

		if (FClassnameIs( ent, "npc_sniper" ))
		{
			AddTargetMarker( ent, true, ent->WorldSpaceCenter() );
		}
		else if (FClassnameIs( ent, "npc_strider" ))
		{
			AddTargetMarker( ent, true, ent->BodyTarget(GetAbsOrigin(), false) );
		}
		else {
			Target = ent;
		}
		float flDuration;
		EmitSound( "Buttons.snd15_softer", 0.0, &flDuration );
		return true;
	}
	return false;
}

void CWeaponEGAR::FireRocketAt( CBaseEntity* target )
{
	Vector vecOrigin;
	Vector vecForward;

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if (pOwner == NULL)
		return;

	Vector	vForward, vRight, vUp;

	pOwner->EyeVectors( &vForward, &vRight, &vUp );

	Vector	muzzlePoint = pOwner->Weapon_ShootPosition() +
		                  vForward * egar_lpos_f.GetFloat() +
		                  vRight * egar_lpos_r.GetFloat() + 
						  vUp * egar_lpos_u.GetFloat();

	QAngle launch_angle = GetAbsAngles();


	CHomingMissile* missile =
		CHomingMissile::Create( muzzlePoint, launch_angle, this, target, m_nRocketsQueued - 1 );

	if (target)
	{
		//Msg( "Aiming rocket at %s", target->GetClassname() );
		missile->IgniteThink();
		missile->SeekThink();
	}
}

void CWeaponEGAR::UpdateRocketState( const CUserCmd *ucmd, int iButtonsReleased )
{
	// button-down, start targeting
	if (ucmd->buttons & IN_ATTACK2)
	{
		if (m_nRocketState == EGAR_ROCKET_IDLE)
		{
			m_nRocketState = EGAR_ROCKET_TARGETING;
			// set time so it takes time to acquire the first target
			m_flRocketTime = gpGlobals->curtime;
			//Msg( "Targeting Rockets...\n" );
		}
	} // button-up, start firing
	else if (iButtonsReleased & IN_ATTACK2) {
		if (m_nRocketState == EGAR_ROCKET_TARGETING) {
			m_nRocketState = EGAR_ROCKET_FIRING;
			// set time so the rockets start firing instantly
			m_flRocketTime = 0;
			//Msg( "Firing Rockets...\n" );
		}
	}
	// regenerate rockets 
	if (m_nRocketsReady < EGAR_ROCKETS &&
		m_flRocketGenTime + EGAR_ROCKET_RECHARGE_TIME < gpGlobals->curtime)
	{
		m_nRocketsReady++;
		m_flRocketGenTime = gpGlobals->curtime;
		//Msg( "New rocket regenerated (%d now)\n", m_nRocketsReady );
	}

	switch (m_nRocketState) {
	case (EGAR_ROCKET_TARGETING) :
		// check timer, try to acquire target if it's time
		if (gpGlobals->curtime > m_flRocketTime + EGAR_ROCKET_TARGET_TIME &&
			m_nRocketsQueued < m_nRocketsReady &&
			m_nRocketsReady > 0) {

			// Try to acquire next target
			EHANDLE hTarget;

			if (FindRocketTarget( hTarget ))
			{
				CBaseEntity* target = hTarget.Get();
				if (target)
				{
					//Msg( "\nValidTarget\n" );
					EHANDLE hTarget = target;
					m_hRocketTargets.Set( m_nRocketsQueued, hTarget );
					m_nRocketsQueued++;
					//Msg( "Rocket Target (%s) Acquired...(%d now)\n",
					//	target->GetClassname(), m_nRocketsQueued );
					m_flRocketTime = gpGlobals->curtime;
				}
			}
		}

		break;
	case (EGAR_ROCKET_FIRING) :
		// check timer, fire rocket if it's time and targets in queue
		if (m_nRocketsQueued > 0) {
			if (gpGlobals->curtime > m_flRocketTime + EGAR_ROCKET_FIRE_INTVAL) {
				// Time to fire a rocket
				EHANDLE hTarget = m_hRocketTargets[m_nRocketsQueued - 1];
				CBaseEntity* target = hTarget.Get();

				if (target) {
					//Msg( "Firing Rocket at %s...\n", target->GetClassname() );
					FireRocketAt( target );
					m_nRocketsReady--;
				}

				// if no target, skip
				m_nRocketsQueued--;

				m_flRocketTime = gpGlobals->curtime;
				if (m_nRocketsQueued <= 0) {
					m_nRocketState = EGAR_ROCKET_IDLE;

					//Msg( "All Queued Rockets Fired.\n" );
				}
				// Stop regenerating rockets while firing
				m_flRocketGenTime = gpGlobals->curtime;
			}
		}
		else {
			m_nRocketState = EGAR_ROCKET_IDLE;
			//Msg( "No Targets.\n" );
		}
		break;
	default: break;
	}

	// Manual targeting
	if (m_nRocketState != EGAR_ROCKET_FIRING &&
		ucmd->buttons & IN_ATTACK &&
		m_flNextPrimaryAttack < gpGlobals->curtime &&
		m_nRocketsReady > m_nRocketsQueued)
	{

		m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;

		CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

		if (pOwner == NULL)
			return;

		Vector	vForward, vRight, vUp;

		pOwner->EyeVectors( &vForward, &vRight, &vUp );

		trace_t	tr;
		Vector vecEye = pOwner->EyePosition();
		UTIL_TraceLine( vecEye, vecEye + vForward * sv_egar_rocket_range.GetFloat(),
			MASK_SHOT_HULL, this, COLLISION_GROUP_PROJECTILE, &tr );

		if (tr.fraction < 1.0)
		{
			AddTargetMarker( tr.m_pEnt, tr.DidHitNonWorldEntity(), tr.endpos );

			float flDuration;
			EmitSound( "Buttons.snd15_softer", 0.0, &flDuration );
		}
	}

	// Support queueing all remaining rockets with the reload button
	if (ucmd->buttons & IN_RELOAD && 
		m_nRocketsQueued > 0 &&
		m_nRocketsReady > m_nRocketsQueued) {

		QueueAllRockets();
	}


}

void CWeaponEGAR::AddTargetMarker( CBaseEntity* targ, bool nonworld, const Vector& pos )
{
	// Hit something - create a marker and target it
	CBaseEntity* pMarker = CreateEntityByName( "info_target" );

	pMarker->AddSpawnFlags( 0x01 );
	pMarker->Spawn();
	pMarker->Activate();

	if (nonworld)
	{
		pMarker->FollowEntity( targ, false );
	}

	pMarker->SetName( MAKE_STRING( "egar_target" ) );
	pMarker->SetAbsOrigin( pos );

	EHANDLE hTarget = pMarker;
	m_hRocketTargets.Set( m_nRocketsQueued, hTarget );
	m_nRocketsQueued++;
}