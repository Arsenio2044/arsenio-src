//========= Copyright Glitch Software, All rights reserved. ============//
//
// Purpose: A turret.
//
//=============================================================================//

// for ai
#include "cbase.h"
#include "npc_mobile_turret.h"
#include "ai_baseactor.h"
#include "ai_hull.h"
#include "ammodef.h"
#include "gamerules.h"
#include "IEffects.h"
#include "engine/IEngineSound.h"
#include "ai_behavior.h"
#include "ai_behavior_assault.h"
#include "ai_behavior_lead.h"
#include "ai_behavior_follow.h"
#include "npcevent.h"
#include "ai_playerally.h"
#include "ai_senses.h"
#include "soundent.h"

// for turret
#include "ai_default.h"
#include "ai_task.h"
#include "ai_schedule.h"
#include "ai_memory.h"
#include "game.h"
#include "vstdlib/random.h"
#include "beam_shared.h"
#include "explode.h"
#include "te_effect_dispatch.h"

// for eye sprite
#include "Sprite.h"

ConVar mobileturret_headshot_freq( "mobileturret_headshot_freq", "2" );
ConVar ai_newmobileturret ( "ai_newmobileturret", "0" );
ConVar sk_mobileturret_health( "sk_mobileturret_health","0");

#define MOBILETURRET_BEAM_SPRITE "materials/effects/bluelaser2.vmt"
#define MOBILETURRET_GLOW_SPRITE "sprites/glow1.vmt" //

#define MOBILETURRET_VIEWCONE		60.0f // (degrees)
#define MOBILETURRET_RETIRE_TIME	7.0f

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Activities.
//-----------------------------------------------------------------------------
int ACT_MOBILE_TURRET_GUN_IDLE;

LINK_ENTITY_TO_CLASS( npc_mobile_turret, CNPC_Mobile_Turret );

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CNPC_Mobile_Turret )
//	first 4 from monk class
	DEFINE_FIELD( m_iNumZombies, FIELD_INTEGER ),
	DEFINE_FIELD( m_iDangerousZombies, FIELD_INTEGER ),
	DEFINE_FIELD( m_bPerfectAccuracy, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bMournedPlayer, FIELD_BOOLEAN ),
	
	DEFINE_FIELD( m_iAmmoType,		FIELD_INTEGER ),
	DEFINE_FIELD( m_pSmoke,			FIELD_CLASSPTR ),
	DEFINE_FIELD( m_vecSpread,		FIELD_VECTOR ),
	DEFINE_FIELD( m_bEnabled,		FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flTimeNextShoot, FIELD_TIME ),
	DEFINE_FIELD( m_flTimeLastSawEnemy, FIELD_TIME ),
	DEFINE_FIELD( m_iDeathSparks,	FIELD_INTEGER ),
	DEFINE_FIELD( m_bHasExploded,	FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flSensingDist,	FIELD_FLOAT ),
	DEFINE_FIELD( m_flTimeNextPing, FIELD_TIME ),
	DEFINE_FIELD( m_bSeeEnemy,		FIELD_BOOLEAN ),
	DEFINE_FIELD( m_vecClosedPos,	FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_vecLightOffset,	FIELD_POSITION_VECTOR ),

	DEFINE_FIELD( m_iEyeAttachment,	FIELD_INTEGER ), //
	DEFINE_FIELD( m_iEyeState,		FIELD_INTEGER ), //
	DEFINE_FIELD( m_hEyeGlow,		FIELD_EHANDLE ), //
	DEFINE_FIELD( m_bBlinkState,	FIELD_BOOLEAN ), //

	DEFINE_FIELD( m_bIsBleeding, FIELD_BOOLEAN ), // smoke
	DEFINE_FIELD( m_vecShootPitch,	FIELD_FLOAT ), // shoot pitch

	DEFINE_THINKFUNC( DeathEffects ),

	DEFINE_OUTPUT( m_OnAreaClear, "OnAreaClear" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),

	// Inputs from monk class
	DEFINE_INPUTFUNC( FIELD_VOID, "PerfectAccuracyOn", InputPerfectAccuracyOn ),
	DEFINE_INPUTFUNC( FIELD_VOID, "PerfectAccuracyOff", InputPerfectAccuracyOff ),

END_DATADESC()

//
CNPC_Mobile_Turret::CNPC_Mobile_Turret( void) : 
	m_hEyeGlow( NULL ), 
	m_bBlinkState( false )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Mobile_Turret::Precache()
{
	PrecacheModel( MOBILETURRET_BEAM_SPRITE );
	PrecacheModel( MOBILETURRET_GLOW_SPRITE );

	PrecacheModel( "models/MobileTurret/mobile_turret.mdl" );
	
	PrecacheScriptSound( "NPC_Citizen.FootstepLeft" );
	PrecacheScriptSound( "NPC_Citizen.FootstepRight" );

	PrecacheScriptSound( "NPC_CeilingTurret.Deploy" );
	m_ShotSounds = PrecacheScriptSound( "NPC_FloorTurret.ShotSounds" );
	PrecacheScriptSound( "NPC_FloorTurret.Die" );
	PrecacheScriptSound( "NPC_FloorTurret.Ping" );
	PrecacheScriptSound( "DoSpark" );

	BaseClass::Precache();
}
 

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Mobile_Turret::Spawn()
{
	Precache();

	BaseClass::Spawn();

	SetModel( "models/MobileTurret/mobile_turret.mdl" );

	SetHullType(HULL_TINY);
	SetSolid( SOLID_BBOX );
	//AddSolidFlags( FSOLID_NOT_STANDABLE );
	//m_takedamage = DAMAGE_YES;
	SetCollisionGroup( COLLISION_GROUP_NPC );
	SetMoveType( MOVETYPE_STEP );
	SetBloodColor( BLOOD_COLOR_MECH ); //DONT_BLEED

	m_iHealth			= sk_mobileturret_health.GetFloat();

	m_flFieldOfView		= cos( ((MOBILETURRET_VIEWCONE / 2.0f) * M_PI / 180.0f) ); // from ground turret
	m_NPCState			= NPC_STATE_NONE;

	m_vecSpread.x = 0.07;
	m_vecSpread.y = 0.07;
	m_vecSpread.z = 0.07;

	CapabilitiesAdd( bits_CAP_MOVE_GROUND );
	CapabilitiesAdd( bits_CAP_SIMPLE_RADIUS_DAMAGE );
	//CapabilitiesAdd( bits_CAP_FRIENDLY_DMG_IMMUNE ); We can kill it!
	CapabilitiesAdd( bits_CAP_MOVE_SHOOT );

	// We don't mind penguins so much.
	//AddClassRelationship( CLASS_PENGUIN, D_NU, 0 );

	NPCInit();

	// rest from FLOOR TURRET
	m_iAmmoType = GetAmmoDef()->Index( "AR2" ); //was "PISTOL" but it was very harsh, SMG1 too soft

	m_pSmoke = NULL;

	m_bHasExploded = false;
	m_bEnabled = true;

	if( ai_newmobileturret.GetBool() )
	{
		m_flSensingDist = 384;
		SetDistLook( m_flSensingDist );
	}
	else
	{
		m_flSensingDist = 2048;
	}

	m_flTimeNextShoot = gpGlobals->curtime;
	m_flTimeNextPing = gpGlobals->curtime;

	m_vecClosedPos = GetAbsOrigin();

	StudioFrameAdvance();

	Vector vecPos;

	GetAttachment( "eyes", vecPos );
	SetViewOffset( vecPos - GetAbsOrigin() );

	GetAttachment( "light", vecPos );
	m_vecLightOffset = vecPos - GetAbsOrigin();

	m_iEyeAttachment = LookupAttachment( "light" );
	SetEyeState( MTURRET_EYE_DORMANT );

	m_nPoseFaceVert = LookupPoseParameter( "turnhead_vert" );
	m_nPoseFaceHoriz = LookupPoseParameter( "turnhead" );

	CreateVPhysics();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CNPC_Mobile_Turret::CreateBehaviors()
{
	AddBehavior( &m_LeadBehavior );
	AddBehavior( &m_AssaultBehavior );
	AddBehavior( &m_FollowBehavior );
	
	return BaseClass::CreateBehaviors();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int CNPC_Mobile_Turret::GetSoundInterests()
{
	return	SOUND_WORLD		|
			SOUND_COMBAT	|
			SOUND_DANGER;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Mobile_Turret::BuildScheduleTestBits( void )
{
	// FIXME: we need a way to make scenes non-interruptible
#if 0
	if ( IsCurSchedule( SCHED_RANGE_ATTACK1 ) || IsCurSchedule( SCHED_SCENE_GENERIC ) )
	{
		ClearCustomInterruptCondition( COND_LIGHT_DAMAGE );
		ClearCustomInterruptCondition( COND_HEAVY_DAMAGE );
		ClearCustomInterruptCondition( COND_NEW_ENEMY );
		ClearCustomInterruptCondition( COND_HEAR_DANGER );
	}
#endif

	// Don't interrupt while shooting the gun
	const Task_t* pTask = GetTask();
	if ( pTask && (pTask->iTask == TASK_RANGE_ATTACK1) )
	{
		ClearCustomInterruptCondition( COND_HEAVY_DAMAGE );
		ClearCustomInterruptCondition( COND_ENEMY_OCCLUDED );
		ClearCustomInterruptCondition( COND_HEAR_DANGER );
		ClearCustomInterruptCondition( COND_WEAPON_BLOCKED_BY_FRIEND );
		ClearCustomInterruptCondition( COND_WEAPON_SIGHT_OCCLUDED );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Mobile_Turret::Activate( void )
{
	BaseClass::Activate();

	// Force the eye state to the current state so that our glows are recreated after transitions
	SetEyeState( m_iEyeState );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
Class_T	CNPC_Mobile_Turret::Classify( void )
{
	return CLASS_PLAYER_ALLY_VITAL;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CNPC_Mobile_Turret::CreateVPhysics( void )
{
	//Spawn our physics hull
	if ( !VPhysicsInitStatic() )
	{
		DevMsg( "npc_mobile_turret unable to spawn physics object!\n" );
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
Activity CNPC_Mobile_Turret::NPC_TranslateActivity( Activity eNewActivity )
{
	return ACT_IDLE; //
}


void CNPC_Mobile_Turret::StayOnGround( void )
{
	trace_t tr;
	UTIL_TraceHull( GetAbsOrigin(), GetAbsOrigin() + Vector( 0, 0, -512 ), NAI_Hull::Mins( GetHullType() ), NAI_Hull::Maxs( GetHullType() ), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr );
	SetAbsOrigin( tr.endpos );
}

//-----------------------------------------------------------------------------
// Purpose: Handles all flight movement because we don't ever build paths when
//			when we are flying.
// Input  : flInterval - Seconds to simulate.
//-----------------------------------------------------------------------------
bool CNPC_Mobile_Turret::OverrideMove( float flInterval )
{
	// Update what we're looking at
	UpdateHead();

	// Update our local angles
	trace_t tr;
	AI_TraceLine( GetAbsOrigin(), GetAbsOrigin()-Vector(0,0,24), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr );
	if ( tr.fraction < 1.0f && tr.DidHitWorld())
	{
		Vector vecUp;
		Vector vForward, vecDeflect;
		QAngle newAngle;

		AngleVectors( GetAbsAngles(), &vForward );

		CrossProduct( vForward, tr.plane.normal, vecUp );
		CrossProduct( tr.plane.normal, vecUp, vecDeflect ); //tr.plane.normal
		VectorNormalize( vecDeflect );

		VectorAngles( vecDeflect, newAngle );
		SetLocalAngles( newAngle );
	}

	// Lower ourself on slopes
	StayOnGround();

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : flInterval - 
//-----------------------------------------------------------------------------
void CNPC_Mobile_Turret::UpdateHead( void )
{
	float yaw = GetPoseParameter( m_nPoseFaceHoriz );
	float pitch = GetPoseParameter( m_nPoseFaceVert );

	// If we should be watching our enemy, turn our head
	if ( ( GetEnemy() != NULL ) && HasCondition( COND_IN_PVS ) )
	{
		Vector	enemyDir = GetEnemy()->WorldSpaceCenter() - WorldSpaceCenter();
		VectorNormalize( enemyDir );
		
		float angle = VecToYaw( BodyDirection3D() );
		float angleDiff = VecToYaw( enemyDir );
		angleDiff = UTIL_AngleDiff( angleDiff, angle + yaw );

		SetPoseParameter( m_nPoseFaceHoriz, UTIL_Approach( yaw + angleDiff, yaw, 50 ) );

		angle = UTIL_VecToPitch( BodyDirection3D() );
		angleDiff = UTIL_VecToPitch( enemyDir );
		angleDiff = UTIL_AngleDiff( angleDiff, angle + pitch );

		m_vecShootPitch = enemyDir; // store the shooting pitch for shoot code

		SetPoseParameter( m_nPoseFaceVert, UTIL_Approach( pitch + angleDiff, pitch, 50 ) );
	}
	else
	{
		SetPoseParameter( m_nPoseFaceHoriz,	UTIL_Approach( 0, yaw, 10 ) );
		SetPoseParameter( m_nPoseFaceVert, UTIL_Approach( 0, pitch, 10 ) );
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_Mobile_Turret::Shoot()
{
	FireBulletsInfo_t info;

	Vector vecSrc = EyePosition();
	Vector vecDir;

	Vector vecMuzzle, vecMuzzleDir;
	QAngle vecMuzzleAng;
	GetAttachment( "eyes", vecMuzzle, vecMuzzleAng );
	AngleVectors( vecMuzzleAng, &vecMuzzleDir );

	//float yaw = GetPoseParameter( m_nPoseFaceHoriz );

	//GetVectors( &vecDir, NULL, NULL );

	//vecDir = m_vecShootPitch; // shoot pitch, from head rotation

	for( int i = 0 ; i < 1 ; i++ )
	{
		info.m_vecSrc = vecSrc;
		
		
		if ( GetEnemy() != NULL )
		{
			Vector vecDir = GetActualShootTrajectory( vecSrc );

			info.m_vecSrc = vecSrc;
			info.m_vecDirShooting = vecDir;
			info.m_iTracerFreq = 1;
			info.m_iShots = 1;
			info.m_pAttacker = this;
			info.m_vecSpread = VECTOR_CONE_PRECALCULATED;
			info.m_flDistance = MAX_COORD_RANGE;
			info.m_iAmmoType = m_iAmmoType; 
		}
		else
		{
			// Just shoot where you're facing!
			//Vector vecMuzzle, vecMuzzleDir;
			//QAngle vecMuzzleAng;
		
			info.m_vecSrc = vecSrc;
			info.m_vecDirShooting = vecMuzzleDir;
			info.m_iTracerFreq = 1;
			info.m_iShots = 1;
			info.m_pAttacker = this;
			info.m_vecSpread = GetAttackSpread( NULL, NULL );
			info.m_flDistance = MAX_COORD_RANGE;
			info.m_iAmmoType = m_iAmmoType;
		}
		

		FireBullets( info );
	}

	// Do the AR2 muzzle flash
	CEffectData data;
	data.m_nEntIndex = entindex();
	data.m_nAttachmentIndex = LookupAttachment( "eyes" );
	data.m_flScale = 1.0f;
	data.m_fFlags = MUZZLEFLASH_COMBINE;
	if ( GetEnemy() != NULL )
	{
		AngleVectors(data.m_vAngles, &vecDir); //test
	}
	else
		data.m_vAngles = vecMuzzleAng;
	DispatchEffect( "MuzzleFlash", data );

	EmitSound( "NPC_FloorTurret.ShotSounds", m_ShotSounds );

	if( IsX360() )
	{
		m_flTimeNextShoot = gpGlobals->curtime + 0.2;
	}
	else
	{
		m_flTimeNextShoot = gpGlobals->curtime + 0.09;
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_Mobile_Turret::Scan()
{
	if( m_bSeeEnemy )
	{
		// Using a bool for this check because the condition gets wiped out by changing schedules.
		return;
	}

	if( IsOpeningOrClosing() )
	{
		// Moving.
		return;
	}

	if( !UTIL_FindClientInPVS(edict()) )
	{
		return;
	}

	if( gpGlobals->curtime >= m_flTimeNextPing )
	{
		SetEyeState( MTURRET_EYE_SEEKING_TARGET );
		EmitSound( "NPC_FloorTurret.Ping" );
		m_flTimeNextPing = gpGlobals->curtime + 1.0f;
	}

	QAngle	scanAngle;
	Vector	forward;
	Vector	vecEye = GetAbsOrigin(); //Vector	vecEye = GetAbsOrigin() + m_vecLightOffset;

	// Draw the outer extents
	scanAngle = GetAbsAngles();
	scanAngle.y += (MOBILETURRET_VIEWCONE / 2.0f);
	AngleVectors( scanAngle, &forward, NULL, NULL );
	ProjectBeam( vecEye, forward, 1, 30, 0.1 );

	scanAngle = GetAbsAngles();
	scanAngle.y -= (MOBILETURRET_VIEWCONE / 2.0f);
	AngleVectors( scanAngle, &forward, NULL, NULL );
	ProjectBeam( vecEye, forward, 1, 30, 0.1 );

	// Draw a sweeping beam
	scanAngle = GetAbsAngles();
	scanAngle.y += (MOBILETURRET_VIEWCONE / 2.0f) * sin( gpGlobals->curtime * 3.0f );
	
	AngleVectors( scanAngle, &forward, NULL, NULL );
	ProjectBeam( vecEye, forward, 1, 30, 0.3 );
}

//-----------------------------------------------------------------------------
// Purpose: Sets the state of the glowing eye attached to the turret
// Input  : state - state the eye should be in
//-----------------------------------------------------------------------------
void CNPC_Mobile_Turret::SetEyeState( mt_eyeState_t state )
{
	// Must have a valid eye to affect
	if ( !m_hEyeGlow )
	{
		// Create our eye sprite
		m_hEyeGlow = CSprite::SpriteCreate( MOBILETURRET_GLOW_SPRITE, GetLocalOrigin(), false );
		if ( !m_hEyeGlow )
			return;

		m_hEyeGlow->SetTransparency( kRenderWorldGlow, 255, 0, 0, 128, kRenderFxNoDissipation );
		m_hEyeGlow->SetAttachment( this, m_iEyeAttachment );
	}

	m_iEyeState = state;

	//Set the state
	switch( state )
	{
	default:
	case MTURRET_EYE_SEE_TARGET: //Fade in and scale up
		m_hEyeGlow->SetColor( 255, 0, 0 );
		m_hEyeGlow->SetBrightness( 164, 0.1f );
		m_hEyeGlow->SetScale( 0.4f, 0.1f );
		break;

	case MTURRET_EYE_SEEKING_TARGET: //Ping-pongs
		
		//Toggle our state
		m_bBlinkState = !m_bBlinkState;
		m_hEyeGlow->SetColor( 255, 128, 0 );

		if ( m_bBlinkState )
		{
			//Fade up and scale up
			m_hEyeGlow->SetScale( 0.25f, 0.1f );
			m_hEyeGlow->SetBrightness( 164, 0.1f );
		}
		else
		{
			//Fade down and scale down
			m_hEyeGlow->SetScale( 0.2f, 0.1f );
			m_hEyeGlow->SetBrightness( 64, 0.1f );
		}

		break;

	case MTURRET_EYE_DORMANT: //Fade out and scale down
		m_hEyeGlow->SetColor( 0, 255, 0 );
		m_hEyeGlow->SetScale( 0.1f, 0.5f );
		m_hEyeGlow->SetBrightness( 64, 0.5f );
		break;

	case MTURRET_EYE_DEAD: //Fade out slowly
		m_hEyeGlow->SetColor( 255, 0, 0 );
		m_hEyeGlow->SetScale( 0.1f, 3.0f );
		m_hEyeGlow->SetBrightness( 0, 3.0f );
		break;

	case MTURRET_EYE_DISABLED:
		m_hEyeGlow->SetColor( 0, 255, 0 );
		m_hEyeGlow->SetScale( 0.1f, 1.0f );
		m_hEyeGlow->SetBrightness( 0, 1.0f );
		break;
	
	case MTURRET_EYE_ALARM:
		{
			//Toggle our state
			m_bBlinkState = !m_bBlinkState;
			m_hEyeGlow->SetColor( 255, 0, 0 );

			if ( m_bBlinkState )
			{
				//Fade up and scale up
				m_hEyeGlow->SetScale( 0.75f, 0.05f );
				m_hEyeGlow->SetBrightness( 192, 0.05f );
			}
			else
			{
				//Fade down and scale down
				m_hEyeGlow->SetScale( 0.25f, 0.25f );
				m_hEyeGlow->SetBrightness( 64, 0.25f );
			}
		}
		break;
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_Mobile_Turret::InputEnable( inputdata_t &inputdata )
{
	m_bEnabled = true;

	// Because the turret might not ever ACQUIRE an enemy, we need to arrange to 
	// retire after a few seconds.
	m_flTimeLastSawEnemy = gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_Mobile_Turret::InputDisable( inputdata_t &inputdata )
{
	m_bEnabled = false;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_Mobile_Turret::ProjectBeam( const Vector &vecStart, const Vector &vecDir, int width, int brightness, float duration )
{
	CBeam *pBeam;
	pBeam = CBeam::BeamCreate( MOBILETURRET_BEAM_SPRITE, width );
	if ( !pBeam )
		return;

	trace_t tr;
	AI_TraceLine( vecStart, vecStart + vecDir * m_flSensingDist, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );
	
	pBeam->SetStartPos( tr.endpos );
	pBeam->SetEndPos( tr.startpos );
	pBeam->SetWidth( width );
	pBeam->SetEndWidth( 0.1 );
	pBeam->SetFadeLength( 16 );

	pBeam->SetBrightness( brightness );
	pBeam->SetColor( 0, 145+random->RandomInt( -16, 16 ), 255 );
	pBeam->RelinkBeam();
	pBeam->LiveForTime( duration );
}

//---------------------------------------------------------
//---------------------------------------------------------
void CNPC_Mobile_Turret::PostNPCInit()
{
	BaseClass::PostNPCInit();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void CNPC_Mobile_Turret::PainSound( const CTakeDamageInfo &info )
{
	SpeakIfAllowed( TLK_WOUND );
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void CNPC_Mobile_Turret::DeathSound( const CTakeDamageInfo &info )
{
	// Sentences don't play on dead NPCs
	SentenceStop();

	Speak( TLK_DEATH );
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
WeaponProficiency_t CNPC_Mobile_Turret::CalcWeaponProficiency( CBaseCombatWeapon *pWeapon )
{
	return WEAPON_PROFICIENCY_PERFECT;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
Vector CNPC_Mobile_Turret::GetActualShootPosition( const Vector &shootOrigin )
{
	return BaseClass::GetActualShootPosition( shootOrigin );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
Vector CNPC_Mobile_Turret::GetActualShootTrajectory( const Vector &shootOrigin )
{
	if( GetEnemy() && GetEnemy()->Classify() == CLASS_ZOMBIE )
	{
		Vector vecShootDir;

		if( m_bPerfectAccuracy || random->RandomInt( 1, mobileturret_headshot_freq.GetInt() ) == 1 )
		{
			vecShootDir = GetEnemy()->HeadTarget( shootOrigin ) - shootOrigin;
		}
		else
		{
			vecShootDir = GetEnemy()->BodyTarget( shootOrigin ) - shootOrigin;
		}

		VectorNormalize( vecShootDir );
		return vecShootDir;
	}

	return BaseClass::GetActualShootTrajectory( shootOrigin );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pEvent - 
//-----------------------------------------------------------------------------
void CNPC_Mobile_Turret::HandleAnimEvent( animevent_t *pEvent )
{
	switch( pEvent->event )
	{
		case NPC_EVENT_LEFTFOOT:
			{
				EmitSound( "NPC_Citizen.FootstepLeft", pEvent->eventtime );
			}
			break;
		case NPC_EVENT_RIGHTFOOT:
			{
				EmitSound( "NPC_Citizen.FootstepRight", pEvent->eventtime );
			}
			break;

		default:
			BaseClass::HandleAnimEvent( pEvent );
			break;
	}
}

//-------------------------------------
// Grigori tries to stand his ground until
// enemies are very close.
//-------------------------------------
#define MOBILETURRET_STAND_GROUND_HEIGHT	24.0
bool CNPC_Mobile_Turret::ShouldBackAway()
{
	if( !GetEnemy() )
		return false;

	if( GetAbsOrigin().z - GetEnemy()->GetAbsOrigin().z >= MOBILETURRET_STAND_GROUND_HEIGHT )
	{
		// This is a fairly special case. Grigori looks better fighting from his assault points in the
		// elevated places of the Graveyard, so we prevent his back away behavior anytime he has a height
		// advantage on his enemy.
		return false;
	}

	float flDist;
	flDist = ( GetAbsOrigin() - GetEnemy()->GetAbsOrigin() ).Length();

	if( flDist <= 180 )
		return true;

	return false;
}

//---------------------------------------------------------
//---------------------------------------------------------
int CNPC_Mobile_Turret::OnTakeDamage_Alive( const CTakeDamageInfo &info )
{
	
	CTakeDamageInfo myInfo = info;

	int nRet = BaseClass::OnTakeDamage_Alive( myInfo );
	
	// start smoke at 30% health.
	if ( !IsBleeding() && ( GetHealth() <= sk_mobileturret_health.GetInt() * 0.3 ) )
	{
		StartBleeding();
	}
	
	return nRet;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_Mobile_Turret::OnRestore()
{
	BaseClass::OnRestore();
	CreateVPhysics();
	
	if ( IsBleeding() )
	{
		StartBleeding();
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_Mobile_Turret::StartBleeding()
{
	// Do this even if we're already bleeding (see OnRestore).
	m_bIsBleeding = true;

	// Smoke
	if ( m_pSmoke != NULL )
		return;

	m_pSmoke = SmokeTrail::CreateSmokeTrail();
	
	if ( m_pSmoke )
	{
		m_pSmoke->m_SpawnRate			= 18;
		m_pSmoke->m_ParticleLifetime	= 3.0;
		m_pSmoke->m_StartSize			= 8;
		m_pSmoke->m_EndSize				= 32;
		m_pSmoke->m_SpawnRadius			= 16;
		m_pSmoke->m_MinSpeed			= 8;
		m_pSmoke->m_MaxSpeed			= 32;
		m_pSmoke->m_Opacity 			= 0.6;
		
		m_pSmoke->m_StartColor.Init( 0.25f, 0.25f, 0.25f );
		m_pSmoke->m_EndColor.Init( 0, 0, 0 );
		m_pSmoke->SetLifetime( 30.0f );
		m_pSmoke->FollowEntity( this );
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
void CNPC_Mobile_Turret::Event_Killed( const CTakeDamageInfo &info )
{
	BaseClass::Event_Killed( info );

	m_iDeathSparks = random->RandomInt( 6, 12 );

	SetThink( &CNPC_Mobile_Turret::DeathEffects );
	SetNextThink( gpGlobals->curtime + 1.5f );

	if ( m_pSmoke != NULL )
		return;

	m_pSmoke = SmokeTrail::CreateSmokeTrail();
	
	if ( m_pSmoke )
	{
		m_pSmoke->m_SpawnRate			= 18;
		m_pSmoke->m_ParticleLifetime	= 3.0;
		m_pSmoke->m_StartSize			= 8;
		m_pSmoke->m_EndSize				= 32;
		m_pSmoke->m_SpawnRadius			= 16;
		m_pSmoke->m_MinSpeed			= 8;
		m_pSmoke->m_MaxSpeed			= 32;
		m_pSmoke->m_Opacity 			= 0.6;
		
		m_pSmoke->m_StartColor.Init( 0.25f, 0.25f, 0.25f );
		m_pSmoke->m_EndColor.Init( 0, 0, 0 );
		m_pSmoke->SetLifetime( 30.0f );
		m_pSmoke->FollowEntity( this );
	}

	//m_iDeathSparks = random->RandomInt( 6, 12 );

	//SetThink( &CNPC_Mobile_Turret::DeathEffects );
	//SetNextThink( gpGlobals->curtime + 1.5f );
}

//---------------------------------------------------------
//---------------------------------------------------------
void CNPC_Mobile_Turret::DeathEffects()
{
	if( !m_bHasExploded )
	{
		ExplosionCreate( GetAbsOrigin(), QAngle( 0, 0, 1 ), this, 150, 150, true ); // added the explosion
		CTakeDamageInfo info;
		DeathSound( info );
		m_bHasExploded = true;
		SetEyeState( MTURRET_EYE_DEAD );
		SetNextThink( gpGlobals->curtime + 0.5 );
	}
	else
	{
		// Sparks
		EmitSound( "DoSpark" );
		m_iDeathSparks--;

		if( m_iDeathSparks == 0 )
		{
			SetThink(NULL);
			return;
		}

		SetNextThink( gpGlobals->curtime + random->RandomFloat( 0.5, 2.5 ) );
	}
}

//-------------------------------------

int CNPC_Mobile_Turret::TranslateSchedule( int scheduleType ) 
{
	switch( scheduleType )
	{

	case SCHED_IDLE_STAND:
		// Our first method of backing away failed. Try another.
		return SCHED_FOLLOW_PLAYER;
		break;

	case SCHED_RANGE_ATTACK1:
		{
			return SCHED_FOLLOW_ENEMY;
		}
		break;
		
	}

	return BaseClass::TranslateSchedule( scheduleType );
}


//-------------------------------------

void CNPC_Mobile_Turret::PrescheduleThink()
{
	//Vector vecOrigin;
	//QAngle vecAngles;
	//MatrixAngles( GetAbsOrigin, vecAngles, vecOrigin );
	//SetAbsOrigin( vecOrigin );
	//SetAbsAngles( vecAngles );

	if( UTIL_FindClientInPVS(edict()) )
	{
		SetNextThink( gpGlobals->curtime + 0.03f );
	}
	else
	{
		SetNextThink( gpGlobals->curtime + 0.1f );
	}
	BaseClass::PrescheduleThink();
}	

//-------------------------------------

int CNPC_Mobile_Turret::SelectSchedule()
{
	if( !UTIL_FindClientInPVS(edict()) )
	{
		// players getting away, follow him! ignore enemies!
		if(m_bSeeEnemy)
			m_bSeeEnemy = false;
		SetEyeState( MTURRET_EYE_DORMANT );
		return SCHED_FOLLOW_PLAYER;
	}

	// Follow, Ivestigate
	if(!m_bSeeEnemy)
	{
		if( HasCondition( COND_HEAR_DANGER ) || HasCondition( COND_HEAR_COMBAT ) )
		{
			SetEyeState( MTURRET_EYE_SEEKING_TARGET );
			return SCHED_IVESTIGATE_SOUND;
		}
		if ( HasCondition ( COND_LOST_PLAYER ) )
		{
			SetEyeState( MTURRET_EYE_DORMANT );
			return SCHED_FOLLOW_PLAYER;
		}
		if ( HasCondition ( COND_SEE_ENEMY ) )
		{
			SetEyeState( MTURRET_EYE_SEE_TARGET );
			return SCHED_FOLLOW_ENEMY;
		}
		if ( HasCondition ( COND_SCHEDULE_DONE ) )
		{
			SetEyeState( MTURRET_EYE_DORMANT );
			return SCHED_FOLLOW_PLAYER;
		}
	}

	// see enemy and attack it
	if ( HasCondition ( COND_SEE_ENEMY ) )
	{
		SetEyeState( MTURRET_EYE_SEE_TARGET );
		return SCHED_FOLLOW_ENEMY;
	}
	if ( HasCondition ( COND_ENEMY_UNREACHABLE ) )
	{
		SetEyeState( MTURRET_EYE_SEE_TARGET );
		return SCHED_FIRE_ON_ENEMY;
	}
	
	// when enemy dies, or its lost, go back to follow and investigate
	if ( HasCondition ( COND_ENEMY_DEAD ) )
	{
		SetEyeState( MTURRET_EYE_DORMANT );
		return SCHED_FOLLOW_PLAYER;
	}
	if ( HasCondition ( COND_ENEMY_OCCLUDED ) )
	{
		SetEyeState( MTURRET_EYE_DORMANT );
		return SCHED_LOOK_AROUND;
	}

	// default behaviour
	if( !BehaviorSelectSchedule() )
	{
		SetEyeState( MTURRET_EYE_DORMANT );
		return SCHED_FOLLOW_PLAYER;
	}

	return BaseClass::SelectSchedule();
}

//-------------------------------------

void CNPC_Mobile_Turret::StartTask( const Task_t *pTask )
{
	BaseClass::StartTask( pTask );
}


void CNPC_Mobile_Turret::RunTask( const Task_t *pTask )
{
	if( !m_bSeeEnemy ) //&& GetAbsVelocity() == vec3_origin
	{
		Scan();
	}
	BaseClass::RunTask( pTask );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_Mobile_Turret::GatherConditions()
{
	if( !IsEnabled() )
	{
		return;
	}

	if( !IsOpen() && !UTIL_FindClientInPVS( edict() ) )
	{
		return;
	}

	// Throw away old enemies so the turret can retire
	AIEnemiesIter_t iter;

	for( AI_EnemyInfo_t *pEMemory = GetEnemies()->GetFirst(&iter); pEMemory != NULL; pEMemory = GetEnemies()->GetNext(&iter) )
	{
		if( pEMemory->timeLastSeen < gpGlobals->curtime - MOBILETURRET_RETIRE_TIME )
		{
			pEMemory->hEnemy = NULL;
		}
	}

	BaseClass::GatherConditions();

	if( GetEnemy() && HasCondition(COND_SEE_ENEMY) )
	{
		m_flTimeLastSawEnemy = gpGlobals->curtime;
	}
	else
	{
		if( gpGlobals->curtime - m_flTimeLastSawEnemy >= MOBILETURRET_RETIRE_TIME )
		{
			m_OnAreaClear.FireOutput(this, this);
			m_flTimeLastSawEnemy = FLT_MAX;
			return;
		}
	}

	if( HasCondition( COND_SEE_ENEMY ) )
	{
		m_bSeeEnemy = true;
	}
	else
	{
		m_bSeeEnemy = false;
	}

	if( GetEnemy() && m_bSeeEnemy && IsEnabled() )
	{
		if( m_flTimeNextShoot < gpGlobals->curtime )
		{
			Shoot();
		}
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
Vector CNPC_Mobile_Turret::EyePosition()
{
	return GetAbsOrigin() + Vector( 0, 0, 15 ); //return GetAbsOrigin() + Vector( 0, 0, 6 );
}

//---------------------------------------------------------
//---------------------------------------------------------
bool CNPC_Mobile_Turret::FVisible( CBaseEntity *pEntity, int traceMask, CBaseEntity **ppBlocker )
{
	if ( BaseClass::FVisible( pEntity, traceMask, ppBlocker ) )
		return true;
	if ( ( pEntity->GetAbsOrigin().AsVector2D() - GetAbsOrigin().AsVector2D() ).LengthSqr() < Square(10*12) &&
		 FInViewCone( pEntity->GetAbsOrigin() ) &&
		 BaseClass::FVisible( pEntity->GetAbsOrigin() + Vector( 0, 0, 1 ), traceMask, ppBlocker ) )
		return true;
	return false;
}

//---------------------------------------------------------
//---------------------------------------------------------
bool CNPC_Mobile_Turret::QuerySeeEntity( CBaseEntity *pEntity, bool bOnlyHateOrFearIfNPC)
{
	float flDist;

	flDist = (pEntity->GetAbsOrigin() - EyePosition()).Length2DSqr();

	if( flDist <= m_flSensingDist * m_flSensingDist )
	{
		return BaseClass::QuerySeeEntity(pEntity, bOnlyHateOrFearIfNPC);
	}

	return false;
}

//---------------------------------------------------------
//---------------------------------------------------------
bool CNPC_Mobile_Turret::IsEnabled()
{
	if( ai_newmobileturret.GetBool() )
	{
		return true;
	}

	return m_bEnabled;
}

//---------------------------------------------------------
//---------------------------------------------------------
bool CNPC_Mobile_Turret::IsOpen()
{
	// The method is hacky but in the end, this does actually give
	// us a pretty good idea if the turret is open or closed.
	return( fabs(GetAbsOrigin().z - m_vecClosedPos.z ) > 1.0f );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CNPC_Mobile_Turret::PassesDamageFilter( const CTakeDamageInfo &info )
{
	if ( info.GetAttacker()->ClassMatches( "npc_headcrab_black" ) || info.GetAttacker()->ClassMatches( "npc_headcrab_poison" ) )
		return false;

	CTakeDamageInfo inputInfo = info;

	if ( info.GetAttacker()->ClassMatches( "npc_voloxelican" ) )
	{
		int newDMG = info.GetDamage() * .2;
		inputInfo.SetDamage(newDMG);
	}

	return BaseClass::PassesDamageFilter( inputInfo );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_Mobile_Turret::OnKilledNPC( CBaseCombatCharacter *pKilled )
{

}

//---------------------------------------------------------
//---------------------------------------------------------
void CNPC_Mobile_Turret::MakeTracer( const Vector &vecTracerSrc, const trace_t &tr, int iTracerType )
{
#if 1
	//BaseClass::MakeTracer( vecTracerSrc, tr, iTracerType );
	UTIL_Tracer( vecTracerSrc, tr.endpos, 0, TRACER_DONT_USE_ATTACHMENT, 5000, true, "AR2Tracer" );
#else
	CBeam *pBeam;
	int	width = 2;

	pBeam = CBeam::BeamCreate( GROUNDTURRET_BEAM_SPRITE, width );
	if ( !pBeam )
		return;
	
	pBeam->SetStartPos( vecTracerSrc );
	pBeam->SetEndPos( tr.endpos );
	pBeam->SetWidth( width );
	pBeam->SetEndWidth( width / 4.0f );

	pBeam->SetBrightness( 100 );
	pBeam->SetColor( 0, 145+random->RandomInt( -16, 16 ), 255 );
	pBeam->RelinkBeam();
	pBeam->LiveForTime( random->RandomFloat( 0.2f, 0.5f ) );
#endif
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int CNPC_Mobile_Turret::SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode )
{
	return BaseClass::SelectFailSchedule( failedSchedule, failedTask, taskFailCode );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CNPC_Mobile_Turret::IsJumpLegal( const Vector &startPos, const Vector &apex, const Vector &endPos ) const
{
	if ( startPos.z - endPos.z < 0 )
		return false;
	return BaseClass::IsJumpLegal( startPos, apex, endPos );
}

//-----------------------------------------------------------------------------
// Every shot's a headshot. Useful for scripted Grigoris
//-----------------------------------------------------------------------------
void CNPC_Mobile_Turret::InputPerfectAccuracyOn( inputdata_t &inputdata )
{
	m_bPerfectAccuracy = true;
}

//-----------------------------------------------------------------------------
// Turn off perfect accuracy.
//-----------------------------------------------------------------------------
void CNPC_Mobile_Turret::InputPerfectAccuracyOff( inputdata_t &inputdata )
{
	m_bPerfectAccuracy = false;
}

void CNPC_Mobile_Turret::UpdateOnRemove( void )
{
	if ( m_hEyeGlow != NULL )
	{
		UTIL_Remove( m_hEyeGlow );
		m_hEyeGlow = NULL;
	}

	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
//
// CNPC_Monk Schedules
//
//-----------------------------------------------------------------------------
AI_BEGIN_CUSTOM_NPC( npc_mobile_turret, CNPC_Mobile_Turret )

	DECLARE_ACTIVITY( ACT_MOBILE_TURRET_GUN_IDLE )
	DECLARE_TASK( FIRE_AT_ENEMY );


	DEFINE_SCHEDULE
	(
		SCHED_FOLLOW_PLAYER,

		"	Tasks"
		"		TASK_TARGET_PLAYER					0"
		"		TASK_GET_PATH_TO_TARGET				0"
		"		TASK_MOVE_TO_TARGET_RANGE			350"
		"		TASK_SET_SCHEDULE					SCHEDULE:SCHED_LOOK_AROUND"
		""
		"	Interrupts"
		"		COND_SCHEDULE_DONE"
		"		COND_SEE_ENEMY"
		"		COND_HEAR_DANGER"
		//"		COND_SEE_PLAYER"
	)

	DEFINE_SCHEDULE
	(
		SCHED_IVESTIGATE_SOUND,

		"	Tasks"
		"		TASK_STOP_MOVING			0"
		"		TASK_STORE_BESTSOUND_REACTORIGIN_IN_SAVEPOSITION		0"
		"		TASK_STOP_MOVING			0"
		"		TASK_FACE_SAVEPOSITION		0"
		"		TASK_MOVE_TO_TARGET_RANGE			350"
		"		TASK_STOP_MOVING			0"
		""
		"	Interrupts"
		"		COND_SEE_ENEMY"
		"		COND_LOST_PLAYER"
		"		COND_SCHEDULE_DONE"
	)

	DEFINE_SCHEDULE
	(
		SCHED_FOLLOW_ENEMY,

		"	Tasks"
		"		TASK_FACE_ENEMY						0"
		"		TASK_GET_PATH_TO_ENEMY				0"
		"		TASK_MOVE_TO_TARGET_RANGE			80"
		"		TASK_FACE_ENEMY						0"
		"		TASK_STOP_MOVING			0"
		""
		"	Interrupts"
		"		COND_ENEMY_UNREACHABLE"
		"		COND_ENEMY_DEAD"
		"		COND_ENEMY_OCCLUDED"
		"		COND_LOST_PLAYER"
	)

	DEFINE_SCHEDULE
	(
		SCHED_LOOK_AROUND,

		"	Tasks"
		"		TASK_WANDER							1"
		"		TASK_TURN_LEFT						0"
		//"		TASK_TURN_LEFT						0"
		//"		TASK_TURN_LEFT						0"
		"		TASK_WAIT_RANDOM					1"
		"		TASK_TURN_RIGHT						0"
		//"		TASK_TURN_RIGHT						0"
		//"		TASK_TURN_RIGHT						0"
		"		TASK_WAIT_RANDOM					1"
		"		TASK_TURN_LEFT						0"
		//"		TASK_TURN_LEFT						0"
		//"		TASK_TURN_LEFT						0"
		"		TASK_WAIT_RANDOM					1"
		"		TASK_FACE_PLAYER					0"
		//"		TASK_TURN_RIGHT						0"
		//"		TASK_TURN_RIGHT						0"
		""
		"	Interrupts"
		"		COND_SCHEDULE_DONE"
		"		COND_SEE_ENEMY"
		"		COND_ENEMY_DEAD"
		"		COND_ENEMY_OCCLUDED"
		"		COND_LOST_PLAYER"
	)

	DEFINE_SCHEDULE
	(
		SCHED_FIRE_ON_ENEMY,

		"	Tasks"
		"		TASK_FACE_ENEMY						1"
		""
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_ENEMY_DEAD"
		"		COND_ENEMY_OCCLUDED"
		"		COND_LOST_PLAYER"
	)

AI_END_CUSTOM_NPC()