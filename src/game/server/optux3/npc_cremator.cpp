//========= Copyright Glitch Software, All rights reserved. ============//
//
// Purpose:
//				
//				
// $NoKeywords: $
//=============================================================================//


#include "cbase.h"
#include "basehlcombatweapon.h"
#include "basecombatcharacter.h"
#include "player.h"
#include "soundent.h"
#include "te_particlesystem.h"
#include "ndebugoverlay.h"
#include "in_buttons.h"
#include "ai_basenpc.h"
#include "ai_memory.h"

#include "cbase.h"
#include "beam_shared.h"
#include "basehlcombatweapon.h"
#include "basecombatcharacter.h"
#include "player.h"
#include "te_particlesystem.h"
#include "game.h"
#include "AI_Default.h"
#include "AI_Schedule.h"
#include "AI_Hull.h"
#include "AI_Route.h"
#include "AI_Squad.h"
#include "AI_Task.h"
#include "AI_Node.h"
#include "AI_Hint.h"
#include "AI_SquadSlot.h"
#include "AI_Motor.h"
#include "NPCEvent.h"
#include "gib.h"
#include "AI_Interactions.h"
#include "ndebugoverlay.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "NPC_Cremator.h"
#include "soundent.h"
#include "player.h"
#include "IEffects.h"
#include "basecombatweapon.h"
#include "soundemittersystem/isoundemittersystembase.h"

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define		CREM_AE_IMMO_SHOOT		( 4 )
#define		CREM_AE_IMMO_DONE		( 5 )

#define		CREM_BODY_GUNDRAWN		0
#define		CREM_BODY_GUNGONE			1

#define CREMATOR_INJURED_HEALTH				100 //Pretty sure this is obsolete now. I'm leaving it in just in case


ConVar	sk_cremator_health( "sk_cremator_health","200");
ConVar	sk_cremator_dmg_immo( "sk_cremator_dmg_immo","1"); //Anything higher than this is OP
ConVar	sk_cremator_max_range( "sk_cremator_max_range","950");
ConVar	sk_cremator_immolator_color_r ("sk_cremator_immolator_color_r", "0");
ConVar	sk_cremator_immolator_color_g ("sk_cremator_immolator_color_g", "255");
ConVar	sk_cremator_immolator_color_b ("sk_cremator_immolator_color_b", "0");
ConVar	sk_cremator_immolator_beamsprite ("sk_cremator_immolator_beamsprite", "sprites/physbeam.vmt");

LINK_ENTITY_TO_CLASS( monster_cremator, CNPC_Cremator );
LINK_ENTITY_TO_CLASS( npc_cremator, CNPC_Cremator );

BEGIN_DATADESC( CNPC_Cremator )
	DEFINE_FIELD( m_flBurnRadius, FIELD_FLOAT ),
	DEFINE_FIELD( m_iBravery, FIELD_INTEGER ),
	DEFINE_ARRAY( m_pBeam, FIELD_CLASSPTR, CREMATOR_MAX_BEAMS ),
	DEFINE_ARRAY( m_hNoise, FIELD_CLASSPTR, CREMATOR_MAX_NOISE ),
	DEFINE_FIELD( m_vecImmolatorTarget, FIELD_VECTOR ),
	DEFINE_FIELD( m_iBeams, FIELD_INTEGER ),
	DEFINE_FIELD( m_iNoise, FIELD_INTEGER ),
	DEFINE_FIELD( m_flNextAttack, FIELD_TIME ),
	DEFINE_FIELD( m_iVoicePitch, FIELD_INTEGER ),
	DEFINE_FIELD( m_hDead, FIELD_EHANDLE ),
	DEFINE_FIELD( m_pOwner, FIELD_CLASSPTR ),
END_DATADESC()

enum
{
	SCHED_CREMATOR_ATTACK = LAST_SHARED_SCHEDULE,
	SCHED_CREMATOR_PATROL,
	SCHED_CREMATOR_MOVE_DEPLOYED,
};

#define CREMATOR_IGNORE_PLAYER 64

int ACT_RUN_DEPLOYED;
int ACT_WALK_DEPLOYED;

//=========================================================
// Spawn
//=========================================================
void CNPC_Cremator::Spawn()
{
	Precache( );

	SetModel( "models/cremator_npc.mdl" );
	
	SetRenderColor( 255, 255, 255, 255 );

	SetHullType(HULL_HUMAN);
	SetHullSizeNormal();
	
	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_NOT_STANDABLE );
	SetMoveType( MOVETYPE_STEP );
	m_bloodColor = BLOOD_COLOR_GREEN; //NOTE TO DL'ers: You really, really, really want to change this
	ClearEffects();
    m_iHealth			= sk_cremator_health.GetFloat();
	m_flFieldOfView		= 0.65;
	m_NPCState			= NPC_STATE_NONE;

	m_iVoicePitch		= random->RandomInt( 85, 110 );

	CapabilitiesClear(); //Still can't open doors on his own for some reason... WTF
	CapabilitiesAdd( bits_CAP_MOVE_GROUND | bits_CAP_OPEN_DOORS | bits_CAP_AUTO_DOORS | bits_CAP_USE | bits_CAP_DOORS_GROUP);
	CapabilitiesAdd( bits_CAP_INNATE_RANGE_ATTACK1 | bits_CAP_MOVE_SHOOT | bits_CAP_TURN_HEAD | bits_CAP_ANIMATEDFACE );
	CapabilitiesAdd( bits_CAP_SQUAD | bits_CAP_NO_HIT_SQUADMATES | bits_CAP_FRIENDLY_DMG_IMMUNE  );

	m_iBravery = 0;

	NPCInit();

	BaseClass::Spawn();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CNPC_Cremator::Precache()
{
	BaseClass::Precache();

	PrecacheModel( "models/gibs/cremator_arm1.mdl" );
	PrecacheModel( "models/gibs/cremator_arm2.mdl" );
	PrecacheModel( "models/gibs/cremator_lfoot.mdl" );
	PrecacheModel( "models/gibs/cremator_rfoot.mdl" );
	PrecacheModel( "models/gibs/cremator_head.mdl" );
	PrecacheModel( "models/gibs/cremator_midsection.mdl" );

	PrecacheModel("models/cremator_npc.mdl");
	PrecacheModel("sprites/lgtning.vmt");

	PrecacheScriptSound( "Crem.Pain" );
	PrecacheScriptSound( "Crem.Die" );
	PrecacheScriptSound( "Crem.ImmoShoot" );
	PrecacheScriptSound( "Crem.Footstep" );
	PrecacheScriptSound( "Crem.Swish" );
}


//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
Class_T	CNPC_Cremator::Classify ( void )
{
	return	CLASS_COMBINE;
}

void CNPC_Cremator::CallForHelp( char *szClassname, float flDist, CBaseEntity * pEnemy, Vector &vecLocation )
{
	// ALERT( at_aiconsole, "help " );

	// skip ones not on my netname
	if ( !m_pSquad )
		 return;

	DevMsg( "Cremator: CALLING FOR BACKUP!\n" );

	AISquadIter_t iter;
	for (CAI_BaseNPC *pSquadMember = m_pSquad->GetFirstMember( &iter ); pSquadMember; pSquadMember = m_pSquad->GetNextMember( &iter ) )
	{
		float d = ( GetAbsOrigin() - pSquadMember->GetAbsOrigin() ).Length();

		if ( d < flDist )
		{
			pSquadMember->Remember( bits_MEMORY_PROVOKED );
			pSquadMember->UpdateEnemyMemory( pEnemy, vecLocation );
		}
	}
}


//=========================================================
// ALert Sound
//=========================================================
void CNPC_Cremator::AlertSound( void )
{
	if ( GetEnemy() != NULL )
	{
		SENTENCEG_PlayRndSz( edict(), "CREM_ALERT", 1.00, SNDLVL_NORM, 0, m_iVoicePitch );

	/*	Vector vecTmp = GetEnemy()->GetAbsOrigin();
		CallForHelp( "npc_cremator", 512, GetEnemy(), vecTmp );
		CallForHelp( "npc_metropolice", 512, GetEnemy(), vecTmp );
		CallForHelp( "npc_combine_s", 512, GetEnemy(), vecTmp );
		CallForHelp( "npc_combineguard", 512, GetEnemy(), vecTmp ); */

	}
}

//=========================================================
// IdleSound
//=========================================================
void CNPC_Cremator::IdleSound( void )
{
	if ( random->RandomInt( 0, 2 ) == 0)
	  	 SENTENCEG_PlayRndSz( edict(), "CREM_IDLE", 1.0, SNDLVL_NORM, 0, 100);
}

//=========================================================
// PainSound
//=========================================================
void CNPC_Cremator::PainSound( const CTakeDamageInfo &info )
{
	if ( random->RandomInt( 0, 2 ) == 0)
	{
		CPASAttenuationFilter filter( this );

		CSoundParameters params;
		if ( GetParametersForSound( "Crem.Pain", params, NULL ) )
		{
			EmitSound_t ep( params );
			params.pitch = m_iVoicePitch;

			EmitSound( filter, entindex(), ep );
		}
	}
}

//=========================================================
// DieSound
//=========================================================

void CNPC_Cremator::DeathSound( const CTakeDamageInfo &info )
{
	CPASAttenuationFilter filter( this );
	CSoundParameters params;
	if ( GetParametersForSound( "Crem.Die", params, NULL ) )
	{
		EmitSound_t ep( params );
		params.pitch = m_iVoicePitch;

		EmitSound( filter, entindex(), ep );
	}
}

int CNPC_Cremator::GetSoundInterests ( void )
{
	return	SOUND_WORLD	|
			SOUND_COMBAT	|
			SOUND_DANGER	|
			SOUND_PLAYER;
}

void CNPC_Cremator::Event_Killed( const CTakeDamageInfo &info )
{

	if ( m_nBody < CREM_BODY_GUNGONE )
	{
		// drop the gun!
		Vector vecGunPos;
		QAngle angGunAngles;
		//CBaseEntity *pGun = NULL;

		SetBodygroup( 1, CREM_BODY_GUNGONE);

		GetAttachment( "0", vecGunPos, angGunAngles );
		
		angGunAngles.y += 180;
	//	pGun = DropItem( "weapon_immolator", vecGunPos, angGunAngles );
	}

	ClearBeams( );
	BaseClass::Event_Killed( info );
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
float CNPC_Cremator::MaxYawSpeed ( void )
{
	float flYS;

	switch ( GetActivity() )
	{
	case ACT_WALK:		
		flYS = 50;	
		break;
	case ACT_RUN:		
		flYS = 70;
		break;
	case ACT_IDLE:		
		flYS = 50;
		break;
	case ACT_RANGE_ATTACK1:
		flYS = 90;
		break;
	default:
		flYS = 90;
		break;
	}

	return flYS;
}

//=========================================================
// HandleAnimEvent 
//=========================================================
void CNPC_Cremator::HandleAnimEvent( animevent_t *pEvent )
{
	switch( pEvent->event )
	{

		case NPC_EVENT_LEFTFOOT:
			{
				EmitSound( "Crem.Footstep", pEvent->eventtime );
			}
			break;
		case NPC_EVENT_RIGHTFOOT:
			{
				EmitSound( "Crem.Footstep", pEvent->eventtime );
			}
			break;	


		case CREM_AE_IMMO_SHOOT:
		{
			ClearBeams( );

			if ( m_hDead != NULL )
			{
				Vector vecDest = m_hDead->GetAbsOrigin() + Vector( 0, 0, 38 );
				trace_t trace;
				UTIL_TraceHull( vecDest, vecDest, GetHullMins(), GetHullMaxs(),MASK_SOLID, m_hDead, COLLISION_GROUP_NONE, &trace );
			}
			
			ClearMultiDamage();

			ImmoBeam( -1 );
			ImmoBeam( 1 );

			CPASAttenuationFilter filter4( this );
			EmitSound( filter4, entindex(), "Crem.ImmoShoot" );
			ApplyMultiDamage();

			CBroadcastRecipientFilter filter;
			te->DynamicLight( filter, 0.0, &GetAbsOrigin(), sk_cremator_immolator_color_r.GetFloat(), sk_cremator_immolator_color_g.GetFloat(), sk_cremator_immolator_color_b.GetFloat(), 2, 256, 0.2 / m_flPlaybackRate, 0 );

			m_flNextAttack = gpGlobals->curtime + random->RandomFloat( 0.5, 4.0 );
		}
		break;
		
		case CREM_AE_IMMO_DONE:
		{
			ClearBeams();
		}
		break;

		default:
			BaseClass::HandleAnimEvent( pEvent );
			break;
	}
}

//------------------------------------------------------------------------------
// Purpose : For innate range attack
// Input   :
// Output  :
//------------------------------------------------------------------------------
int CNPC_Cremator::RangeAttack1Conditions( float flDot, float flDist )
{
	if ( GetEnemy() == NULL )
		 return( COND_LOST_ENEMY );

	if ( flDist > sk_cremator_max_range.GetFloat() ) //1024 by default
	{
		return( COND_TOO_FAR_TO_ATTACK );
	}
	
	if ( gpGlobals->curtime < m_flNextAttack )
		 return COND_NONE;

	if ( HasCondition( COND_CAN_MELEE_ATTACK1 ) )
		 return COND_NONE;
	
	if ( HasCondition( COND_TOO_FAR_TO_ATTACK ) )
		 return COND_NONE;

	return COND_CAN_RANGE_ATTACK1;
}


void CNPC_Cremator::StartTask( const Task_t *pTask )
{
	ClearBeams();
	BaseClass::StartTask( pTask );
}

//=========================================================
// TakeDamage - get provoked when injured
//=========================================================

int CNPC_Cremator::OnTakeDamage_Alive( const CTakeDamageInfo &info )
{
	CTakeDamageInfo newInfo = info;
	if( newInfo.GetDamageType() & DMG_DISSOLVE)
	{
		newInfo.ScaleDamage( 0 );
		DevMsg( "Cremator: Immolator damage, no actual damage is taken\n" );
	}

	if( newInfo.GetDamageType() & DMG_BURN)
	{
		newInfo.ScaleDamage( 0 );
		DevMsg( "Cremator: Immolator damage, no actual damage is taken\n" );
	}

	if( newInfo.GetDamageType() & DMG_PLASMA)
	{
		newInfo.ScaleDamage( 0 );
		DevMsg( "Cremator: Immolator damage, no actual damage is taken\n" );
	}

	int nDamageTaken = BaseClass::OnTakeDamage_Alive( newInfo );


	return nDamageTaken;
}

void CNPC_Cremator::TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator )
{
	if ( info.GetDamageType() & DMG_SHOCK )
	 return;

	BaseClass::TraceAttack( info, vecDir, ptr, pAccumulator );
}

//=========================================================
//=========================================================
int CNPC_Cremator::SelectSchedule( void )
{
	ClearBeams();

	if ( HasCondition( COND_ENEMY_DEAD ))
	{
		SENTENCEG_PlayRndSz( edict(), "CREM_KILL", 1.0, SNDLVL_NORM, 0, m_iVoicePitch );
	}

	switch( m_NPCState )
	{

	case NPC_STATE_ALERT:
		{
			if ( HasCondition ( COND_HEAR_DANGER ) || HasCondition ( COND_HEAR_COMBAT ) ) 
			{
				if ( HasCondition ( COND_HEAR_DANGER ) )
					 return SCHED_TAKE_COVER_FROM_BEST_SOUND;
				
				else
				 	 return SCHED_INVESTIGATE_SOUND;
			}
			
		}
		break;


	case NPC_STATE_COMBAT:
		{
			// dead enemy
			if ( HasCondition( COND_ENEMY_DEAD ) )
				 return BaseClass::SelectSchedule(); // call base class, all code to handle dead enemies is centralized there.
		
			// always act surprized with a new enemy
			if ( HasCondition( COND_NEW_ENEMY ) && HasCondition( COND_LIGHT_DAMAGE) )
				return SCHED_SMALL_FLINCH;
				
			if ( HasCondition( COND_HEAVY_DAMAGE ) )
				 return SCHED_TAKE_COVER_FROM_ENEMY;
			if( HasCondition( COND_TOO_FAR_TO_ATTACK ) )
			{
				ClearBeams();
				return SCHED_CHASE_ENEMY;
			}

			if ( !HasCondition(COND_SEE_ENEMY) )
			{
				// we can't see the enemy
				if ( !HasCondition(COND_ENEMY_OCCLUDED) )
				{
					// enemy is unseen, but not occluded!
					// turn to face enemy
					return SCHED_COMBAT_FACE;
				}
				else
				{
					return SCHED_CHASE_ENEMY;
				}

		if ( HasCondition( COND_CAN_RANGE_ATTACK1 ) )
			 return SCHED_RANGE_ATTACK1;

		if ( m_iHealth < 20 || m_iBravery < 0)
			{
			if ( !HasCondition( COND_CAN_MELEE_ATTACK1 ) )
					{
					SetDefaultFailSchedule( SCHED_CHASE_ENEMY );
					if ( HasCondition( COND_LIGHT_DAMAGE ) || HasCondition( COND_HEAVY_DAMAGE ) )
						SENTENCEG_PlayRndSz( edict(), "CREM_DISTRESS", 1.00, SNDLVL_NORM, 0, m_iVoicePitch );
						 return SCHED_TAKE_COVER_FROM_ENEMY;
						if ( HasCondition ( COND_SEE_ENEMY ) && HasCondition ( COND_ENEMY_FACING_ME ) )
						 return SCHED_TAKE_COVER_FROM_ENEMY;

					SENTENCEG_PlayRndSz( edict(), "CREM_DISTRESS", 1.00, SNDLVL_NORM, 0, m_iVoicePitch );

					Vector vecTmp = GetEnemy()->GetAbsOrigin();
					CallForHelp( "monster_cremator", 512, GetEnemy(), vecTmp );
					CallForHelp( "npc_cremator", 512, GetEnemy(), vecTmp );
					CallForHelp( "npc_metropolice", 512, GetEnemy(), vecTmp );
					CallForHelp( "npc_combine_s", 512, GetEnemy(), vecTmp );
					CallForHelp( "npc_combineguard", 512, GetEnemy(), vecTmp );
					}
			}


		}
		break;
	}

	case NPC_STATE_IDLE:

		{
				return SCHED_CREMATOR_PATROL;
		}

		if ( HasCondition( COND_LIGHT_DAMAGE ) || HasCondition( COND_HEAVY_DAMAGE ) ) 
		{
			// flinch if hurt
			return SCHED_SMALL_FLINCH;
		}

		if ( GetEnemy() == NULL)
		{
			{
				return SCHED_CREMATOR_PATROL;
			}
		}

		break;
	}
	
	return BaseClass::SelectSchedule();
}

Activity CNPC_Cremator::NPC_TranslateActivity(Activity eNewActivity)
{
		if (m_iHealth <= sk_cremator_health.GetFloat() / 2 && eNewActivity == ACT_RUN)
		{
			// limp!
			return ACT_WALK_HURT;
		}
	/*	if ( GetEnemy() && eNewActivity == ACT_RUN)
		{
			return (Activity)ACT_RUN_DEPLOYED;
		} //Animation is selected ingame but does not work
		*/

		if (m_iHealth <= sk_cremator_health.GetFloat() / 2 && eNewActivity == ACT_WALK )
		{
			// limp!
			return ACT_WALK_HURT;
		}
	/*	if ( GetEnemy() && eNewActivity == ACT_WALK )
		{
			return (Activity)ACT_WALK_DEPLOYED;
		}*/
		
	
	return eNewActivity;
}


int CNPC_Cremator::TranslateSchedule( int scheduleType )
{
	//Oops can't get to my enemy.
	if ( scheduleType == SCHED_CHASE_ENEMY_FAILED )
	{
		
		return SCHED_ESTABLISH_LINE_OF_FIRE;
	}

	switch	( scheduleType )
	{
	case SCHED_FAIL:

		if ( HasCondition( COND_CAN_MELEE_ATTACK1 ) )
		{
	  		 return ( SCHED_MELEE_ATTACK1 );
		}

		break;

	case SCHED_RANGE_ATTACK1:
		{
			if ( HasCondition( COND_CAN_MELEE_ATTACK1 ) )
			{
	  		 return ( SCHED_MELEE_ATTACK1 );
			}
		
			return SCHED_CREMATOR_ATTACK;
		}

		break;
	}

	return BaseClass::TranslateSchedule( scheduleType );
}

void CNPC_Cremator::RunTask( const Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_RANGE_ATTACK1:
				if (HasCondition( COND_ENEMY_OCCLUDED )) //Enemy hiding from us? Don't be retarded, get a line of sight
				{
					ClearBeams();
					TaskComplete();
					break;
				}
				if (HasCondition( COND_ENEMY_DEAD )) //Killed our enemy? No reason to keep immolating
				{
					ClearBeams();
					TaskComplete();
					break;
				}
			BaseClass::RunTask( pTask );
			break;
		default:
		{
			BaseClass::RunTask( pTask );
			break;
		}
	}
}

//=========================================================
// BeamGlow - brighten all beams
//=========================================================
void CNPC_Cremator::BeamGlow( )
{
	int b = m_iBeams * 32;
	
	if ( b > 255 )
		 b = 255;

	for ( int i = 0; i < m_iBeams; i++ )
	{
		if ( m_pBeam[i] != NULL )
		{
			if ( m_pBeam[i]->GetBrightness() != 255 ) 
				m_pBeam[i]->SetBrightness( b );
		}
	}
}

//=========================================================
// ImmoBeam - heavy damage directly forward
//=========================================================
void CNPC_Cremator::ImmoBeam( int side )
{
	Vector vecSrc, vecAim;
	trace_t tr;
	CBaseEntity *pEntity;

	if ( m_iBeams >= CREMATOR_MAX_BEAMS )
		 return;
	if (m_iNoise >= CREMATOR_MAX_NOISE )
		return;

	Vector forward, right, up;
	AngleVectors( GetAbsAngles(), &forward, &right, &up );

	vecSrc = GetAbsOrigin() + up * 36;
	vecAim = GetShootEnemyDir( vecSrc );
	float deflection = 0;
	vecAim = vecAim + side * right * random->RandomFloat( 0, deflection ) + up * random->RandomFloat( -deflection, deflection );
	UTIL_TraceLine ( vecSrc, vecSrc + vecAim * sk_cremator_max_range.GetFloat(), MASK_SOLID, this, COLLISION_GROUP_NONE, &tr); //range is 1024 by default
	
	m_pBeam[m_iBeams] = CBeam::BeamCreate( sk_cremator_immolator_beamsprite.GetString(), 5.0f ); //physbeam default
	if ( m_pBeam[m_iBeams] == NULL )
		 return;


	m_pBeam[m_iBeams]->PointEntInit( tr.endpos, this );
	m_pBeam[m_iBeams]->SetEndAttachment( side < 0 ? 2 : 1 );
	m_pBeam[m_iBeams]->SetColor( sk_cremator_immolator_color_r.GetFloat(), sk_cremator_immolator_color_g.GetFloat(), sk_cremator_immolator_color_b.GetFloat() ); // 0 255 0 by default
	m_pBeam[m_iBeams]->SetBrightness( 255 );
	m_pBeam[m_iBeams]->SetNoise( 0.8f );
	m_pBeam[m_iBeams]->AddSpawnFlags( SF_BEAM_TEMPORARY );
	m_iBeams++;





	for( m_iNoise = 0 ; m_iNoise < CREMATOR_MAX_NOISE ; m_iNoise++ ) 
	{
		Vector vecDest;

			// Random unit vector
		vecDest.x = random->RandomFloat( -1, 1 );
		vecDest.y = random->RandomFloat( -1, 1 );
		vecDest.z = random->RandomFloat( -1, 1 );

		// Range for noise beams is 128 world units
		vecDest = tr.endpos + vecDest * 128;

		//spawn immolator-like radius beam doo-dads
		m_hNoise[m_iNoise] = CBeam::BeamCreate(  sk_cremator_immolator_beamsprite.GetString(), 3.00f ); //physbeam default
		m_hNoise[m_iNoise]->PointsInit( tr.endpos, vecDest);
		m_hNoise[m_iNoise]->AddSpawnFlags( SF_BEAM_TEMPORARY );
		m_hNoise[m_iNoise]->SetScrollRate( 60 );
		m_hNoise[m_iNoise]->SetBrightness( 255 );
		m_hNoise[m_iNoise]->SetColor( sk_cremator_immolator_color_r.GetFloat(), sk_cremator_immolator_color_g.GetFloat(), sk_cremator_immolator_color_b.GetFloat() ); //0 255 0
		m_hNoise[m_iNoise]->SetNoise( 15 );
		m_hNoise[m_iNoise]->SetFadeLength( 0.75 );
		UTIL_DecalTrace( &tr, "FadingScorch" );
	}

	//m_iNoise++;


	trace_t noisetr;




	pEntity = tr.m_pEnt;

	if ( pEntity != NULL && m_takedamage )
	{
		CTakeDamageInfo info( this, this, sk_cremator_dmg_immo.GetFloat(), DMG_DISSOLVE ); //dmg_dissolve and dmg_burn by default
		Ignite(100);
		CalculateMeleeDamageForce( &info, vecAim, tr.endpos );
		RadiusDamage( CTakeDamageInfo( this, this, sk_cremator_dmg_immo.GetFloat(), DMG_BURN ), tr.endpos, 128,  CLASS_NONE, NULL ); //changed from 256 to 128 to correspond with noisebeams
		pEntity->DispatchTraceAttack( info, vecAim, &tr );
	}

}

//=========================================================
// ClearBeams - remove all beams
//=========================================================
void CNPC_Cremator::ClearBeams( )
{
	for (int i = 0; i < CREMATOR_MAX_BEAMS; i++)
	{
		if (m_pBeam[i])
		{
			UTIL_Remove( m_pBeam[i] );
			m_pBeam[i] = NULL;
		}
	}

	for (int i =0; i < CREMATOR_MAX_NOISE; i++)
	{
		if (m_hNoise[i])
		{
			UTIL_Remove( m_hNoise[i] );
			m_hNoise[i] = NULL;
		}
		if (m_iNoise > 0)
		{
			UTIL_Remove( m_hNoise[i] );
			m_iNoise = NULL;
		}
	}

	m_iBeams = 0;
	m_iNoise = 0;
	m_nSkin = 0;
}


//------------------------------------------------------------------------------
//
// Schedules
//
//------------------------------------------------------------------------------

AI_BEGIN_CUSTOM_NPC( npc_cremator, CNPC_Cremator )

	DECLARE_ACTIVITY(ACT_WALK_DEPLOYED);
	DECLARE_ACTIVITY(ACT_RUN_DEPLOYED);

	//=========================================================
	// > SCHED_CREMATOR_ATTACK
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_CREMATOR_ATTACK,

		"	Tasks"
		"		TASK_STOP_MOVING			0"
		"		TASK_FACE_IDEAL				0"
		"		TASK_RANGE_ATTACK1			0"
		"	"
		"	Interrupts"
		"		COND_CAN_MELEE_ATTACK1"
		"		COND_HEAVY_DAMAGE"
		"		COND_HEAR_DANGER"
	)


	DEFINE_SCHEDULE
	(
	SCHED_CREMATOR_PATROL,

		 "	Tasks"
		 "		TASK_STOP_MOVING				0"
		 "		TASK_WANDER						900540" 
		 "		TASK_WALK_PATH					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		"		TASK_STOP_MOVING				0"
		"		TASK_FACE_REASONABLE			0"
		"		TASK_WAIT						3"
		"		TASK_WAIT_RANDOM				3"
		"		TASK_SET_SCHEDULE				SCHEDULE:SCHED_CREMATOR_PATROL" // keep doing it
		""
		 "	Interrupts"
		 "		COND_ENEMY_DEAD"
		 "		COND_LIGHT_DAMAGE"
		 "		COND_HEAVY_DAMAGE"
		 "		COND_HEAR_DANGER"
		"		COND_HEAR_MOVE_AWAY"
		 "		COND_NEW_ENEMY"
		 "		COND_SEE_ENEMY"
		"		COND_CAN_RANGE_ATTACK1"
		"		COND_CAN_RANGE_ATTACK2"
	)



AI_END_CUSTOM_NPC()
