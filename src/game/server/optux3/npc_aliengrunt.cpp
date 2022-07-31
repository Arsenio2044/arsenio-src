//=================== Copyright Glitch Software =====================//
//
// Purpose:	Alien Grunt from Half-Life 1.
//			
//
//=============================================================================//

#include "cbase.h"
#include "beam_shared.h"
#include "game.h"				// For skill levels
#include "globalstate.h"
#include "npc_talker.h"
#include "ai_motor.h"
#include "ai_schedule.h"
#include "scripted.h"
#include "basecombatweapon.h"
#include "soundent.h"
#include "npcevent.h"
#include "ai_hull.h"
#include "animation.h"
#include "ammodef.h"				// For DMG_CLUB
#include "Sprite.h"
#include "npc_aliengrunt.h"
#include "activitylist.h"
#include "player.h"
#include "items.h"
#include "vehicle_base.h"
#include "ai_interactions.h"
#include "IEffects.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "globals.h"
#include "effect_dispatch_data.h"
#include "te_effect_dispatch.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"
#include "physics_prop_ragdoll.h"
#include "RagdollBoogie.h"

#include "physics_bone_follower.h"
#include "physics_prop_ragdoll.h"
#include "bone_setup.h"

#include "weapon_physcannon.h"

// Cut content

//#include "weapon_bee.h"



#include "ai_hint.h"
#include "ai_network.h"
#include "ai_pathfinder.h"
#include "ai_navigator.h"
#include "ai_senses.h"

#include "ai_squadslot.h"
#include "ai_squad.h"

#include "ai_moveprobe.h"

#include "basepropdoor.h"
#include "doors.h"

#include "vphysics_interface.h"
#include "vphysics/constraints.h"
#include "physics_saverestore.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


inline void TraceHull_SkipPhysics( const Vector &vecAbsStart, const Vector &vecAbsEnd, const Vector &hullMin, 
					 const Vector &hullMax,	unsigned int mask, const CBaseEntity *ignore, 
					 int collisionGroup, trace_t *ptr, float minMass );

#define	ALIENGRUNT_LEFT_HAND				"lefthand"
#define	ALIENGRUNT_RIGHT_HAND				"righthand"

static const char *AGRUNT_ATTACK = "AGRUNT_ATTACK";
static const char *AGRUNT_ALERT  = "AGRUNT_ALERT";
static const char *AGRUNT_HIDE   = "AGRUNT_HIDE";
static const char *AGRUNT_CHARGE = "AGRUNT_CHARGE";


//=========================================================
// ALIENGRUNT General Attacking & Health
//=========================================================
ConVar	sk_aliengrunt_health( "sk_aliengrunt_health","0");
ConVar	sk_aliengrunt_dmg_claw( "sk_aliengrunt_dmg_claw","0");

ConVar	aliengrunt_ragdoll_grab_delay("hlss_aliengrunt_ragdoll_grab_delay", "4");
ConVar	aliengrunt_shoot_bees_delay("hlss_aliengrunt_shoot_bees_delay", "1");

#define ALIENGRUNT_NEXT_ATTACK_DELAY_SMALL 0.55f
#define ALIENGRUNT_NEXT_ATTACK_DELAY				random->RandomFloat( aliengrunt_shoot_bees_delay.GetFloat(), aliengrunt_shoot_bees_delay.GetFloat() + 1.0f )
//#define ALIENGRUNT_NEXT_ATTACK_DELAY_WHILE_CHARGE	random->RandomFloat( aliengrunt_shoot_bees_delay.GetFloat() + 1.0f, aliengrunt_shoot_bees_delay.GetFloat() + 3.0f )
#define ALIENGRUNT_TOO_CLOSE_FOR_BEES 70


#define ALIENGRUNT_FACE_ENEMY_DISTANCE 512.0f
#define ALIENGRUNT_FACE_PHYSICS_ENT_DISTANCE 128.0f

//=========================================================
// ALIENGRUNT Ragdoll Grabbing
//=========================================================

#define ALIENGRUNT_ALWAYS_RAGDOLL_GRAB
#ifdef ALIENGRUNT_ALWAYS_RAGDOLL_GRAB
static ConVar aliengrunt_always_ragdoll_grab("hlss_aliengrunt_always_ragdoll_grab","0");
#endif

//=========================================================
// ALIENGRUNT Charge
//=========================================================

ConVar	aliengrunt_charge_delay_max("hlss_aliengrunt_charge_delay_max", "6");
ConVar	aliengrunt_charge_delay_min("hlss_aliengrunt_charge_delay", "2");

#define	ALIENGRUNT_CHARGE_MIN			128		//Antlionguard has 256
#define	ALIENGRUNT_CHARGE_MAX			768	//Antlionguard has 2048, I originally had 512 there because Alien Grunt is woulnerable during the charge
#define ALIENGRUNT_CHARGE_CANCEL_TIME	random->RandomFloat( 2.0, 3.0 )

//=========================================================
// ALIENGRUNT activities
//=========================================================
Activity ACT_ALIENGRUNT_TO_ACTION;
Activity ACT_ALIENGRUNT_TO_IDLE;

Activity ACT_ALIENGRUNT_CHARGE_START;
Activity ACT_ALIENGRUNT_CHARGE_LOOP;
Activity ACT_ALIENGRUNT_CHARGE_HIT;
Activity ACT_ALIENGRUNT_CHARGE_CANCEL;


#ifdef ALIENGRUNT_GRAB_RAGDOLL
Activity ACT_ALIENGRUNT_GRAB_RAGDOLL;
#endif

//=========================================================
// ALIENGRUNT Anim Events 
//=========================================================
int AE_ALIENGRUNT_HAND_LEFT;
int AE_ALIENGRUNT_HAND_RIGHT;
int AE_ALIENGRUNT_SHOOTBEES;
//int AE_ALIENGRUNT_SHOOTBEES_DONE;
int AE_ALIENGRUNT_SWING_SOUND;

#ifdef ALIENGRUNT_GRAB_RAGDOLL
int AE_ALIENGRUNT_GRAB_RAGDOLL;
int AE_ALIENGRUNT_THROW_RAGDOLL;
#endif

int AE_ALIENGRUNT_CHARGE_HIT;

#define ALIENGRUNT_CUSTOM_FOOT_AE
#ifdef ALIENGRUNT_CUSTOM_FOOT_AE
int AE_ALIENGRUNT_FOOT_LEFT;
int AE_ALIENGRUNT_FOOT_RIGHT;
#endif

enum
{	
	SQUAD_SLOT_ALIENGRUNT_CHARGE = LAST_SHARED_SQUADSLOT,
};

//-----------------------------------------------------------------------------
// Interactions
//-----------------------------------------------------------------------------
//int	g_interactionALIENGRUNTStomp		= 0;
//int	g_interactionALIENGRUNTStompFail	= 0;
//int	g_interactionALIENGRUNTStompHit		= 0;
//int	g_interactionALIENGRUNTKick			= 0;
//int	g_interactionALIENGRUNTClaw			= 0;


//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CNPC_AlienGrunt )

	DEFINE_FIELD( m_HideSoundTime,			FIELD_TIME),

	//DEFINE_FIELD( m_nextLineFireTime,		FIELD_TIME),
	DEFINE_FIELD( m_iLeftHandAttachment,	FIELD_INTEGER ),
	DEFINE_FIELD( m_iRightHandAttachment,	FIELD_INTEGER ),
	DEFINE_FIELD( m_bShouldAim,				FIELD_BOOLEAN ),

	DEFINE_FIELD( m_flNextSwat,				FIELD_TIME ),
	DEFINE_FIELD( m_flNextSwatScan,			FIELD_TIME ),
	DEFINE_FIELD( m_hPhysicsEnt,			FIELD_EHANDLE ),
	DEFINE_FIELD( m_hObstructor,			FIELD_EHANDLE ),
	DEFINE_FIELD( m_hSetTarget,				FIELD_EHANDLE ),
	DEFINE_FIELD( m_hDoor,					FIELD_EHANDLE ),

	DEFINE_FIELD( m_iBees,					FIELD_INTEGER ),

	DEFINE_FIELD( m_flNextChargeTime, FIELD_TIME ),
	DEFINE_FIELD( m_bDecidedNotToStop, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flChargeCancelTime, FIELD_TIME ),

	DEFINE_FIELD( m_flBurnDamage, FIELD_FLOAT ),
	DEFINE_FIELD( m_flBurnDamageResetTime, FIELD_TIME ),

#ifdef ALIENGRUNT_GRAB_RAGDOLL
	DEFINE_FIELD( m_hRagdoll,				FIELD_EHANDLE ),
	DEFINE_FIELD( m_flNextRagdollGrabTime,	FIELD_TIME ),
	DEFINE_PHYSPTR( m_hPlayerConstrain ),	

	DEFINE_INPUTFUNC( FIELD_VOID,	"EnableRagdollGrab",		InputEnableRagdollGrab ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"DisableRagdollGrab",		InputDisableRagdollGrab ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"DisableWaitRagdollGrab",	InputDisableWaitRagdollGrab ),
	DEFINE_INPUTFUNC( FIELD_STRING, "AttackItem",				InputAttackItem ),
#endif

END_DATADESC()
#ifdef HL1_NPC
LINK_ENTITY_TO_CLASS(monster_aliengrunt, CNPC_AlienGrunt );
#endif

#ifdef ALIENGRUNT_GRAB_RAGDOLL

const char *pFollowerBoneNames[] = { "ValveBiped.Bip01_R_Forearm", "ValveBiped.Bip01_Head1" };

void CNPC_AlienGrunt::InputEnableRagdollGrab( inputdata_t &inputdata )
{
	AddSpawnFlags( SF_ALIENGRUNT_CAN_GRAB_RAGDOLL );
}

void CNPC_AlienGrunt::InputDisableRagdollGrab( inputdata_t &inputdata )
{
	RemoveSpawnFlags( SF_ALIENGRUNT_CAN_GRAB_RAGDOLL );
}

void CNPC_AlienGrunt::InputDisableWaitRagdollGrab( inputdata_t &inputdata )
{
	RemoveSpawnFlags( SF_ALIENGRUNT_WAIT_FOR_GRAB );
}

void CNPC_AlienGrunt::Event_Killed( const CTakeDamageInfo &info )
{
//	m_BoneFollowerManager.DestroyBoneFollowers();

	if (!DestroyPlayerConstrain() && m_hRagdoll)
	{
		CBaseEntity *pRagdoll = m_hRagdoll;
		if ( pRagdoll )
		{
			DetachAttachedRagdoll( pRagdoll );
		}
		m_hRagdoll = NULL;
	}

	BaseClass::Event_Killed( info );
}

//---------------------------------------------------------
//---------------------------------------------------------
/*static Vector GetAttachmentPositionInSpaceOfBone( CStudioHdr *pStudioHdr, const char *pAttachmentName, int outputBoneIndex )
{
	int attachment = Studio_FindAttachment( pStudioHdr, pAttachmentName );

	Vector localAttach;
	const mstudioattachment_t &pAttachment = pStudioHdr->pAttachment(attachment);
	int iBone = pStudioHdr->GetAttachmentBone( attachment );
	MatrixGetColumn( pAttachment.local, 3, localAttach );

	matrix3x4_t inputToOutputBone;
	Studio_CalcBoneToBoneTransform( pStudioHdr, iBone, outputBoneIndex, inputToOutputBone );
	
	Vector out;
	VectorTransform( localAttach, inputToOutputBone, out );

	return out;
}*/

//-------------------------------------


void CNPC_AlienGrunt::InputAttackItem( inputdata_t &inputdata )
{
	CBaseEntity *pTarget = gEntList.FindEntityByName( NULL, inputdata.value.String(), NULL, inputdata.pActivator, inputdata.pCaller );
	
	if (!pTarget)
	{
		Warning("npc_aliengrunt: InputAttackItem, %s could not be found\n");
		return;
	}

	if (FClassnameIs(pTarget, "prop_physics") ||
		FClassnameIs(pTarget, "func_physics") ||
		FClassnameIs(pTarget, "prop_door_rotating"))
	{
		m_hSetTarget = pTarget;
	}
	else
	{
		Warning("npc_aliengrunt: InputAttackItem, %s is not correct type of entity\n");
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
void CNPC_AlienGrunt::UpdateOnRemove()
{
	DestroyPlayerConstrain();

	BaseClass::UpdateOnRemove();
}

bool CNPC_AlienGrunt::DestroyPlayerConstrain(bool bThrow)
{
	if (m_hPlayerConstrain != NULL)
	{
		if (m_hPlayerConstrain->GetAttachedObject() && m_hPlayerConstrain->GetAttachedObject()->GetGameData())
		{
			CBasePlayer *pPlayer = static_cast<CBasePlayer *>( m_hPlayerConstrain->GetAttachedObject()->GetGameData() );
			if (pPlayer)
			{
				//DevMsg("removing PFLAG_VPHYSICS_MOTIONCONTROLLER\n");
				pPlayer->SetPhysicsFlag( PFLAG_VPHYSICS_MOTIONCONTROLLER, false );

				if (bThrow)
				{
					Vector hitPosition;	

					//TERO: take slightly more damage than regular melee
					CTakeDamageInfo info( this, this, sk_aliengrunt_dmg_claw.GetFloat() * 1.3f, DMG_BLAST );
					pPlayer->VPhysicsTakeDamage( info );

					hitPosition = ( pPlayer->WorldSpaceCenter() - WorldSpaceCenter());
					hitPosition *= 12.0f;
					//GetVectors( &forward, NULL, NULL );

					pPlayer->ViewPunch( QAngle(5,0,-18) );
					pPlayer->VelocityPunch( hitPosition );
			
					// Shake the screen
					UTIL_ScreenShake( pPlayer->GetAbsOrigin(), 100.0, 1.5, 1.0, 2, SHAKE_START );

					// Red damage indicator
					color32 red = { 128, 0, 0, 128 };
					UTIL_ScreenFade( pPlayer, red, 1.0f, 0.1f, FFADE_IN );
				}
			}
		}

		PhysEnableEntityCollisions( m_hPlayerConstrain->GetReferenceObject(),	m_hPlayerConstrain->GetAttachedObject() );
		PhysEnableEntityCollisions( VPhysicsGetObject(),						m_hPlayerConstrain->GetAttachedObject() );

		physenv->DestroyConstraint( m_hPlayerConstrain );
		m_hPlayerConstrain = NULL;

		if (m_hRagdoll)
		{
			UTIL_Remove( m_hRagdoll );
			m_hRagdoll = NULL;
		}

		return true;
	}

	return false;
}

void CNPC_AlienGrunt::GrabRagdoll()
{
	if (GetEnemy()->Classify() == CLASS_PLAYER)
	{
		CBasePlayer *pPlayer = ToBasePlayer( GetEnemy() );
		if (pPlayer && !pPlayer->GetVehicle())
		{
			IPhysicsObject *pPhysObject = pPlayer->VPhysicsGetObject();

			Assert( pPhysObject );

			Vector hitPosition = pPlayer->EyePosition();	
			int iAttachment = LookupAttachment( "righthand" );
			//GetAttachment( iAttachment, hitPosition );

			pPlayer->SetPhysicsFlag( PFLAG_VPHYSICS_MOTIONCONTROLLER, true );

			//Create the thing where we constrain our player
			CBaseEntity *pBolt = CBaseEntity::CreateNoSpawn( "prop_dynamic", hitPosition, GetAbsAngles(), this );
			pBolt->SetModelName( MAKE_STRING( "models/weapons/bee.mdl"  ) );
			pBolt->SetModel( "models/weapons/bee.mdl" );
			DispatchSpawn( pBolt );
			pBolt->AddEffects( EF_NODRAW );
			pBolt->AddSolidFlags( FSOLID_NOT_SOLID );
			pBolt->SetParent( this, iAttachment );
			m_hRagdoll = pBolt;

			//Vector vecRight;
			//GetVectors(NULL, &vecRight, NULL );

			IPhysicsObject *pPhys = pBolt->VPhysicsGetObject();

			constraint_hingeparams_t hingeParams;
			hingeParams.Defaults();
			hingeParams.worldAxisDirection = Vector(0,0,1); //vecRight;
			hingeParams.worldPosition = hitPosition;

			m_hPlayerConstrain = physenv->CreateHingeConstraint( pPhys, pPhysObject, NULL, hingeParams );

			//TERO: disable collisions between the "bolt" and the player
			PhysDisableEntityCollisions( pPhysObject, pPhys );
			PhysDisableEntityCollisions( VPhysicsGetObject(), pPhys );

			m_flNextRagdollGrabTime = gpGlobals->curtime + aliengrunt_ragdoll_grab_delay.GetInt();
			RemoveSpawnFlags( SF_ALIENGRUNT_WAIT_FOR_GRAB );
		}
	}
	else
	{
		CStudioHdr *pStudioHdr = GetModelPtr();

		int boneIndex = Studio_BoneIndexByName( pStudioHdr, pFollowerBoneNames[0] );

		CBaseEntity *pEnemy = GetEnemy();
		CAI_BaseNPC *pNPC = pEnemy ? pEnemy->MyNPCPointer() : NULL;
		bool bIsValidTarget = pNPC && pNPC->GetModelPtr();

		if (!bIsValidTarget)
			return;

		if (pNPC->Classify() != CLASS_COMBINE && pNPC->Classify() != CLASS_METROPOLICE && pNPC->Classify() != CLASS_PLAYER_ALLY_VITAL )
		{
			return;
		}

		pStudioHdr = pNPC->GetModelPtr();

		int parentBoneIndex = Studio_BoneIndexByName( pStudioHdr, pFollowerBoneNames[1] );

		Vector hitPosition;
		GetAttachment( "righthand", hitPosition );

		Vector localHit = Vector(26,0,0); //TERO: in the model it's 25

		Vector delta;
		VectorSubtract( pEnemy->GetAbsOrigin(), hitPosition, delta );
		if ( delta.LengthSqr() > (ALIENGRUNT_GRAB_RANGE * ALIENGRUNT_GRAB_RANGE) )
			return;

		Vector vecStabPos = ( pEnemy->WorldSpaceCenter() + Vector(0,0,35)  + pEnemy->EyePosition() ) * 0.5f;

		CTakeDamageInfo damageInfo( this, this, 100, DMG_CRUSH );
		Vector forward;
		pEnemy->GetVectors( &forward, NULL, NULL );
		damageInfo.SetDamagePosition( hitPosition );
		damageInfo.SetDamageForce( Vector(0,0,0) ); //-50 * 300 * forward );
		pEnemy->TakeDamage( damageInfo );

		if ( !pNPC || pNPC->IsAlive() )
			return;

		Vector vecBloodDelta = hitPosition - vecStabPos;
		vecBloodDelta.z = 0; // effect looks better 
		VectorNormalize( vecBloodDelta );
		UTIL_BloodSpray( vecStabPos + vecBloodDelta * 4, vecBloodDelta, BLOOD_COLOR_RED, 8, FX_BLOODSPRAY_ALL );
		UTIL_BloodSpray( vecStabPos + vecBloodDelta * 4, vecBloodDelta, BLOOD_COLOR_RED, 11, FX_BLOODSPRAY_DROPS );

		CBaseEntity *pRagdoll = CreateServerRagdollAttached( pNPC, vec3_origin, -1, COLLISION_GROUP_DEBRIS, VPhysicsGetObject(), this, boneIndex, vecStabPos, parentBoneIndex, localHit );
		if ( pRagdoll )
		{
			CRagdollProp *pPropRagdoll = dynamic_cast<CRagdollProp*>( pRagdoll);
			if (pPropRagdoll)
			{
				ragdoll_t *pRagdollPhys = pPropRagdoll->GetRagdoll( );

				Vector vSpeed(0,0,0);
				AngularImpulse aImpulse(0,0,0);
				float flSpeed = 6.0f;

				int j;
				for ( j = 0; j < pRagdollPhys->listCount; ++j )
				{
					pRagdollPhys->list[j].pObject->SetDamping(&flSpeed, &flSpeed);
					pRagdollPhys->list[j].pObject->SetVelocityInstantaneous(&vSpeed,&aImpulse);
				}
			}

			m_hRagdoll = pRagdoll;
			m_flNextRagdollGrabTime = gpGlobals->curtime + aliengrunt_ragdoll_grab_delay.GetInt();
			RemoveSpawnFlags( SF_ALIENGRUNT_WAIT_FOR_GRAB );
			UTIL_Remove( pNPC );
		}
	}
}

void CNPC_AlienGrunt::ThrowRagdoll()
{
	if (!DestroyPlayerConstrain(true))
	{
		CBaseEntity *pEntity = m_hRagdoll;
		CRagdollProp *pRagdoll = dynamic_cast<CRagdollProp*>( pEntity);
		if ( pRagdoll )
		{
			//CPASAttenuationFilter filter( pRagdoll, "NPC_Strider.RagdollDetach" );
			//EmitSound( filter, pRagdoll->entindex(), "NPC_Strider.RagdollDetach" );
			DetachAttachedRagdoll( pRagdoll );
			PhysDisableEntityCollisions( pEntity, this );

			Vector vVel;
			QAngle aVel;
			AngularImpulse aImp(0,0,0);

			//GetVectors(&vVel, &vRig, NULL);
			GetAttachment( "righthand", vVel, aVel );
			AngleVectors(aVel, &vVel );
			//vVel = ( 0.75 * vVel) + (0.25 * vRig);
			aImp.x = -aVel.x * 10;
			aImp.y = -aVel.y * 10;
			vVel.z = 1;
			vVel *= 800; //250

			ragdoll_t *pRagdollPhys = pRagdoll->GetRagdoll( );

			float flSpeed = 1.0f;

			pRagdoll->SetAbsVelocity(vVel);

			int j;
			for ( j = 0; j < pRagdollPhys->listCount; ++j )
			{
				pRagdollPhys->list[j].pObject->SetDamping(&flSpeed, &flSpeed);
				pRagdollPhys->list[j].pObject->SetVelocity( &vVel, &aImp ); 
			}
		}
	}

	ClearCondition( COND_ALIENGRUNT_CAN_GRAB_RAGDOLL );
	m_hRagdoll = NULL;
}

/*int CNPC_AlienGrunt::MeleeAttack2Conditions ( float flDot, float flDist )
{
	DevMsg("Grab ragdoll Conditions\n");

	if (!HasSpawnFlags( SF_ALIENGRUNT_CAN_GRAB_RAGDOLL))
		return COND_NONE;

	if (GetEnemy() && GetEnemy()->Classify() != CLASS_COMBINE && GetEnemy()->Classify() != CLASS_METROPOLICE)
	{
		DevMsg("Not right enemy\n");
		return COND_NONE;
	}

	if (flDist > 50 )
	{
		DevMsg("Too far\n");
		return COND_TOO_FAR_TO_ATTACK;
	}

	if (flDot < 0.7)
	{
		DevMsg("Not facing enemy\n");
		ClearCondition(COND_TOO_FAR_TO_ATTACK);
		return COND_NOT_FACING_ATTACK;
	}

	DevMsg("COND_CAN_GRAB_RAGDOLL\n");
	// Move around some more
	return COND_ALIENGRUNT_CAN_GRAB_RAGDOLL;
}*/

void CNPC_AlienGrunt::NPCThink( void )
{
	/*if (HasSpawnFlags( SF_ALIENGRUNT_CAN_GRAB_RAGDOLL ))
	{
		m_BoneFollowerManager.UpdateBoneFollowers(this);
	}*/

	BaseClass::NPCThink();
}

#endif

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEntity - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_AlienGrunt::FVisible( CBaseEntity *pEntity, int traceMask, CBaseEntity **ppBlocker )
{
	CBaseEntity	*pHitEntity = NULL;
	if ( BaseClass::FVisible( pEntity, traceMask, &pHitEntity ) )
		return true;

	// If we hit something that's okay to hit anyway, still fire
	if ( pHitEntity && pHitEntity->GetCollisionGroup()  == COLLISION_GROUP_BREAKABLE_GLASS )
	{
		if ( pHitEntity->m_takedamage == DAMAGE_YES )
			return true;
	}

	if (ppBlocker)
	{
		*ppBlocker = pHitEntity;
	}

	return false;
}

Activity CNPC_AlienGrunt::TranslateAimingActivity(Activity iActivity)
{
	m_bShouldAim = false;

	if (GetEnemy() && HasCondition( COND_SEE_ENEMY) )
	{
		if (iActivity == ACT_RUN)
		{
			return ACT_RUN_AIM;
		}
		else if (iActivity == ACT_WALK)
		{
			return ACT_WALK_AIM;
		}
		else if (iActivity == ACT_IDLE)
		{
			return ACT_IDLE_AIM_AGITATED;
		}

		if ( iActivity == ACT_RUN_AIM || iActivity == ACT_WALK_AIM || iActivity == ACT_IDLE_AIM_AGITATED )
		{
			m_bShouldAim = true;
		}
	}
	else
	{
		if (iActivity == ACT_RUN_AIM)
		{
			return ACT_RUN;
		}
		else if (iActivity == ACT_WALK_AIM)
		{
			return ACT_WALK;
		}
		else if (iActivity == ACT_IDLE_AIM_AGITATED)
		{
			return ACT_IDLE;
		}
	}

	return iActivity;
}

void CNPC_AlienGrunt::PrescheduleThink(void)
{
	BaseClass::PrescheduleThink();

	//TERO: moved here
	if (!HasCondition( COND_CAN_MELEE_ATTACK1 ) &&
		!HasCondition( COND_CAN_MELEE_ATTACK2 ) &&
		!HasCondition( COND_ALIENGRUNT_SHOULD_PROTECT_SQUADMATE ) &&
		(HasCondition( COND_CAN_RANGE_ATTACK1 ) || HasCondition( COND_ALIENGRUNT_BEES ))&& 
		!IsPlayingGesture( ACT_GESTURE_RANGE_ATTACK1 ) )
	{
		//DevMsg("ACT_GESTURE_RANGE_ATTACK1\n");
		ShootBees();

		RestartGesture( ACT_GESTURE_RANGE_ATTACK1, true );
		ClearCondition( COND_CAN_RANGE_ATTACK1 );
		ClearCondition( COND_ALIENGRUNT_BEES );
	}
	
	if ( m_hRagdoll == NULL && m_hPlayerConstrain == NULL &&
		(IsCurSchedule( SCHED_ALIENGRUNT_GRAB_RAGDOLL ) || HasCondition( COND_ALIENGRUNT_CAN_GRAB_RAGDOLL ) ))
	{
		bool bFailed = false;

		if (HasCondition( COND_ENEMY_DEAD ) ||
			HasCondition( COND_ENEMY_OCCLUDED ) ||
			HasCondition( COND_NEW_ENEMY ))
		{	
			bFailed = true;
		}
		else if (GetEnemy())
		{
			if (GetEnemy()->MyCombatCharacterPointer() && GetEnemy()->MyCombatCharacterPointer()->IsInAVehicle())
			{
				bFailed = true;
			}
			else
			{
				Vector vecEnemy, vecForward;
				GetVectors(&vecForward, NULL, NULL);

				vecEnemy = GetEnemy()->GetAbsOrigin() - GetAbsOrigin();
				vecEnemy.z = 0;

				float flDist = VectorNormalize(vecEnemy);
				float flDot  = DotProduct(vecEnemy, vecForward);

				if (flDist > 60 || flDot < 0.6)
				{
					//DevMsg("Cancelled ragdoll grabbing, dist %f, dot product %f \n", flDist, flDot);

					//NDebugOverlay::Line( GetAbsOrigin(), GetAbsOrigin() + vecForward, 255, 0, 0, true, 0.1f );
					//NDebugOverlay::Line( GetEnemy()GetAbsOrigin(), GetAbsOrigin() + vecForward, 255, 0, 0, true, 0.1f );

					bFailed = true;
				}
			}
		}
		else 
		{	
			bFailed = true;
		}

		if (bFailed)
		{
			SetCondition( COND_ALIENGRUNT_CANNOT_GRAB_RAGDOLL );
			ClearCondition( COND_ALIENGRUNT_CAN_GRAB_RAGDOLL );
			m_flNextRagdollGrabTime = gpGlobals->curtime + 1.0f + aliengrunt_ragdoll_grab_delay.GetInt();
		}
	}

	int iActivity = GetActivity();
	int iNewActivity = TranslateAimingActivity( (Activity) iActivity );
	if ( iActivity != iNewActivity )
	{
		SetActivity( (Activity) iNewActivity );
		GetNavigator()->SetMovementActivity( (Activity) iNewActivity );
	}

	UpdateHead();
	UpdateBeehiveGunAim();

	if ( ( m_flBurnDamageResetTime ) && ( gpGlobals->curtime >= m_flBurnDamageResetTime ) )
	{
		m_flBurnDamage = 0;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
inline bool CNPC_AlienGrunt::InAttackSequence( void )
{
	if ( m_MoveAndShootOverlay.IsMovingAndShooting() )
		return true;
	
	if ( GetActivity() == ACT_RANGE_ATTACK1 )
		return true;

	if ( IsPlayingGesture( ACT_GESTURE_RANGE_ATTACK1 ) )
		return true;

	return false;
}


//TERO: YOU COULD OPTIMIZE BY PUTTING THESE TWO IN A SAME FUNCTION
void CNPC_AlienGrunt::UpdateBeehiveGunAim(void)
{
	float yaw = GetPoseParameter( "aim_yaw" );
	float pitch = GetPoseParameter( "aim_pitch" );

	// If we should be watching our enemy, turn our head
	if ( m_bShouldAim && ( GetEnemy() != NULL ) )
	{
		Vector vecEnemy;
		GetEnemy()->BodyTarget(vecEnemy);

		Vector	enemyDir = vecEnemy - WorldSpaceCenter();
		VectorNormalize( enemyDir );
		
		float angle = VecToYaw( BodyDirection3D() );
		float angleDiff = VecToYaw( enemyDir );
		angleDiff = UTIL_AngleDiff( angleDiff, angle + yaw );

		SetPoseParameter( "aim_yaw", UTIL_Approach( yaw + angleDiff, yaw, 50 ) );

		angle = UTIL_VecToPitch( BodyDirection3D() );
		angleDiff = UTIL_VecToPitch( enemyDir );
		angleDiff = UTIL_AngleDiff( angleDiff, angle + pitch );

		SetPoseParameter( "aim_pitch", UTIL_Approach( pitch + angleDiff, pitch, 50 ) );
	}
	else
	{
		// Otherwise turn the head back to its normal position
		SetPoseParameter( "aim_yaw",	UTIL_Approach( 0, yaw, 10 ) );
		SetPoseParameter( "aim_pitch", UTIL_Approach( 0, pitch, 10 ) );
	}

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_AlienGrunt::UpdateHead( void )
{
	float yaw = GetPoseParameter( "head_yaw" );
	float pitch = GetPoseParameter( "head_pitch" );

	int nActivity = GetActivity();

	// If we should be watching our enemy, turn our head
	if ( ( GetEnemy() != NULL ) && 
		 ( nActivity != ACT_ALIENGRUNT_CHARGE_START ) &&
		 ( nActivity != ACT_ALIENGRUNT_CHARGE_HIT ) && 
		 ( nActivity != ACT_ALIENGRUNT_CHARGE_CANCEL ) &&
		 ( nActivity != ACT_ALIENGRUNT_CHARGE_LOOP ) )
	{
		Vector	enemyDir = GetEnemy()->WorldSpaceCenter() - WorldSpaceCenter();
		VectorNormalize( enemyDir );
		
		float angle = VecToYaw( BodyDirection3D() );
		float angleDiff = VecToYaw( enemyDir );
		angleDiff = UTIL_AngleDiff( angleDiff, angle + yaw );

		SetPoseParameter( "head_yaw", UTIL_Approach( yaw + angleDiff, yaw, 50 ) );

		if (GetActivity() == ACT_ALIENGRUNT_CHARGE_LOOP)
		{
			SetPoseParameter( "head_pitch", UTIL_Approach( 43.6, pitch, 50 ) );
		}
		else
		{
			angle = UTIL_VecToPitch( BodyDirection3D() );
			angleDiff = UTIL_VecToPitch( enemyDir );
			angleDiff = UTIL_AngleDiff( angleDiff, angle + pitch );

			SetPoseParameter( "head_pitch", UTIL_Approach( pitch + angleDiff, pitch, 50 ) );
		}
	}
	else
	{
		// Otherwise turn the head back to its normal position
		SetPoseParameter( "head_yaw",	UTIL_Approach( 0, yaw, 10 ) );
		SetPoseParameter( "head_pitch", UTIL_Approach( 0, pitch, 10 ) );
	}
}

bool CNPC_AlienGrunt::CreateBehaviors()
{
	AddBehavior( &m_AssaultBehavior );

	//AddBehavior( &m_LeadBehavior );
	//AddBehavior( &m_FollowBehavior );
	
	return BaseClass::CreateBehaviors();
}

float CNPC_AlienGrunt::MaxYawSpeed( void )
{
	Activity eActivity = GetActivity();

	if ( (eActivity == ACT_ALIENGRUNT_TO_ACTION) || (eActivity == ACT_ALIENGRUNT_TO_IDLE) )
		return 0.0f;

	//TERO: note that these have to be checked outside of switch/case
	if ( eActivity == ACT_ALIENGRUNT_CHARGE_START )
		return 4.0f;

	if ( eActivity == ACT_ALIENGRUNT_CHARGE_LOOP )
	{
		if ( !GetEnemy()  )
			return 2.0f;

		float dist = UTIL_DistApprox2D( GetEnemy()->WorldSpaceCenter(), WorldSpaceCenter() );

		if ( dist > 128 )
			return 24.0f;

		//FIXME: Alter by skill level
		float yawSpeed = RemapValClamped( dist, 0, 128, 1.0f, 2.0f );
		//yawSpeed = clamp( yawSpeed, 2.0f, 4.0f );

		return (yawSpeed * RemapValClamped( m_iHealth, 20, m_iMaxHealth, 12.0f, 6.0f));
	}

	if ( eActivity == ACT_ALIENGRUNT_CHARGE_CANCEL )
		return 8.0f;

	switch( eActivity )
	{
		// Stay still
	case ACT_RANGE_ATTACK1:
	case ACT_MELEE_ATTACK1:
		return 0.0f;
		break;

	case ACT_IDLE:
		return 30.0f;
		break;

	case ACT_TURN_LEFT:
	case ACT_TURN_RIGHT:
		return 40.0f;
		break;
	case ACT_RUN:
	default:
		return 30.0f;
		break;
	}
}


void CNPC_AlienGrunt::StartTask( const Task_t *pTask )
{
	switch ( pTask->iTask)
	{
	case TASK_ALIENGRUNT_CHASE_ENEMY_CONTINUOUSLY:
		StartTaskChaseEnemyContinuously( pTask );
		break;

	case TASK_ALIENGRUNT_SHOOT_BEES:
		{
			SetIdealActivity( ACT_RANGE_ATTACK1 );
		}
		break;

	case TASK_ALIENGRUNT_GET_PATH_TO_PHYSOBJ:
		{
			CBaseEntity *pPhysicsEntity = m_hPhysicsEnt;
			if( !pPhysicsEntity )
			{
				TaskFail("Physics ent NULL");
				break;
			}

			Vector vecGoalPos;
			Vector vecDir;

			vecDir = GetLocalOrigin() - m_hPhysicsEnt->GetLocalOrigin();

			VectorNormalize(vecDir);
			vecDir.z = 0;

			AI_NavGoal_t goal( m_hPhysicsEnt->WorldSpaceCenter() );
			goal.pTarget = m_hPhysicsEnt;
			GetNavigator()->SetGoal( goal );

			TaskComplete();
		}
		break;

	case TASK_ALIENGRUNT_SWAT_ITEM:
		{
			if( m_hPhysicsEnt == NULL )
			{
				// Physics Object is gone! Probably was an explosive 
				// or something else broke it.
				TaskFail("Physics ent NULL");
			}
			else if ( DistToPhysicsEnt() > ALIENGRUNT_PHYSOBJ_SWATDIST )
			{
				// Physics ent is no longer in range! Probably another zombie swatted it or it moved
				// for some other reason.
				TaskFail( "Physics swat item has moved" );
			}
			else
			{
				SetIdealActivity( (Activity)GetSwatActivity() );
			}
			break;
		}
		break;

	case TASK_MELEE_ATTACK2:
		{
			AttackSound();
			ResetIdealActivity( ACT_ALIENGRUNT_GRAB_RAGDOLL );
		}
		break;

	case TASK_ALIENGRUNT_CHARGE:
		{
			// HACK: Because the guard stops running his normal blended movement 
			//		 here, he also needs to remove his blended movement layers!
			GetMotor()->MoveStop();

			SetActivity( ACT_ALIENGRUNT_CHARGE_START );
			m_bDecidedNotToStop = false;

			ChargeSound();

			m_flChargeCancelTime = gpGlobals->curtime + ALIENGRUNT_CHARGE_CANCEL_TIME;
		}
		break;

	case TASK_RANGE_ATTACK1:
	case TASK_MELEE_ATTACK1:
		AttackSound();
	default:
		{
			BaseClass::StartTask( pTask );	
			break;
		}
  }
}

void CNPC_AlienGrunt::RunTask( const Task_t *pTask )
{
	/*if (GetCurSchedule())
		DevMsg("npc_aliengrunt: running schedule with name: %s, with task id %d\n", GetCurSchedule()->GetName(), pTask->iTask );
	else
		DevMsg("npc_aliengrunt: running schedule with task: %d\n", pTask->iTask );*/

	switch ( pTask->iTask )
	{
	case TASK_ALIENGRUNT_CHASE_ENEMY_CONTINUOUSLY:
		RunTaskChaseEnemyContinuously( pTask );
		break;

	case TASK_ALIENGRUNT_SHOOT_BEES:
		{
			if ( IsWaitFinished() )
			{
				//m_bShouldAim = false;
				TaskComplete();
			}
		}
		break;

	case TASK_ALIENGRUNT_SWAT_ITEM:
		if( IsActivityFinished() )
		{
			TaskComplete();
		}
		break;

	case TASK_RANGE_ATTACK1:
		{
			if (GetEnemy() != NULL)
			{
				if (GetEnemy()->IsPlayer())
				{
					m_flPlaybackRate = 1.5;
				}
				if (!GetEnemy()->IsAlive())
				{
					if( IsActivityFinished() )
					{
						TaskComplete();
						break;
					}
				}
				// This is along attack sequence so if the enemy
				// becomes occluded bail
				/*if (HasCondition( COND_ENEMY_OCCLUDED ))
				{
					TaskComplete();
					break;
				}*/
			}
			BaseClass::RunTask( pTask );
			break;
		}
	case TASK_MELEE_ATTACK1:
		{
			if (GetEnemy() == NULL)
			{
				if ( IsActivityFinished() )
				{
					TaskComplete();
				}
			}
			else
			{
				BaseClass::RunTask( pTask );
			}
			break;	
		}
	case TASK_MELEE_ATTACK2:
		{
			if ( IsActivityFinished() )
			{
				TaskComplete();
			}
			break;
		}
	case TASK_ALIENGRUNT_CHARGE:
		{
			Activity eActivity = GetActivity();

			//DevMsg("npc_aliengrunt: running activity: %s\n", GetActivityName( eActivity ));

			//TERO: I am not sure what causes this to happen
			if ( eActivity == ACT_IDLE )
			{
				SetActivity( ACT_ALIENGRUNT_CHARGE_START );
			}

			// See if we're trying to stop after hitting/missing our target
			if ( eActivity ==  ACT_ALIENGRUNT_CHARGE_CANCEL || eActivity ==  ACT_ALIENGRUNT_CHARGE_HIT ) //ACT_ANTLIONGUARD_CHARGE_STOP || eActivity == ACT_ANTLIONGUARD_CHARGE_CRASH ) 
			{
				if ( IsActivityFinished() )
				{
					//DevMsg("npc_aliengrunt: ACT_ALIENGRUNT_CHARGE_CANCEL finished\n");
					TaskComplete();
					return;
				}

				// Still in the process of slowing down. Run movement until it's done.
				AutoMovement();
				return;
			}

			// Check for manual transition
			if ( ( eActivity == ACT_ALIENGRUNT_CHARGE_START ) && ( IsActivityFinished() ) ) // ACT_ANTLIONGUARD_CHARGE_START
			{
				SetActivity( ACT_ALIENGRUNT_CHARGE_LOOP );
			}

			// See if we're still running
			if ( eActivity == ACT_ALIENGRUNT_CHARGE_LOOP || eActivity == ACT_ALIENGRUNT_CHARGE_START ) 
			{
				if ( HasCondition( COND_CAN_RANGE_ATTACK1 ) && !IsPlayingGesture(ACT_GESTURE_RANGE_ATTACK1) )
				{
					RestartGesture( ACT_GESTURE_RANGE_ATTACK1 );
					ShootBees();
					ClearCondition( COND_CAN_RANGE_ATTACK1 ); //TERO: not sure if this is supposed to be done here
				}

				if ( HasCondition( COND_NEW_ENEMY ) || HasCondition( COND_LOST_ENEMY ) || HasCondition( COND_ENEMY_DEAD ) )
				{
					SetActivity( ACT_ALIENGRUNT_CHARGE_CANCEL );
					return;
				}
				else 
				{
					if ( GetEnemy() != NULL )
					{
						Vector	goalDir = ( GetEnemy()->GetAbsOrigin() - GetAbsOrigin() );
						VectorNormalize( goalDir );

						//TERO: the Charge Cancel time is added by me to avoid scenes where the Alien Grunt runs after the player in front
						//		of him for long periods, which looks silly and makes the Alien Grunt vulnerable
						if ( DotProduct( BodyDirection2D(), goalDir ) < 0.25f || m_flChargeCancelTime < gpGlobals->curtime )
						{
							if ( !m_bDecidedNotToStop )
							{
								// We've missed the target. Randomly decide not to stop, which will cause
								// the guard to just try and swing around for another pass.
								m_bDecidedNotToStop = true;
								if ( RandomFloat(0,1) > 0.3 )
								{
									//m_iChargeMisses++;
									SetActivity( ACT_ALIENGRUNT_CHARGE_CANCEL );
								}

								m_flChargeCancelTime = gpGlobals->curtime + ALIENGRUNT_CHARGE_CANCEL_TIME;

								ChargeSound();
							}
						}
						else
						{
							m_bDecidedNotToStop = false;
						}
					}
				}
			}

			// Steer towards our target
			float idealYaw;
			if ( GetEnemy() == NULL )
			{
				idealYaw = GetMotor()->GetIdealYaw();
			}
			else
			{
				idealYaw = CalcIdealYaw( GetEnemy()->GetAbsOrigin() );
			}

			// Add in our steering offset
			idealYaw += ChargeSteer();
			
			// Turn to face
			GetMotor()->SetIdealYawAndUpdate( idealYaw );

			// See if we're going to run into anything soon
			ChargeLookAhead();

			// Let our animations simply move us forward. Keep the result
			// of the movement so we know whether we've hit our target.
			AIMoveTrace_t moveTrace;
			if ( AutoMovement( GetEnemy(), &moveTrace ) == false )
			{
				// Only stop if we hit the world
				if ( HandleChargeImpact( moveTrace.vEndPosition, moveTrace.pObstruction ) )
				{
					// If we're starting up, this is an error
					if ( eActivity == ACT_ALIENGRUNT_CHARGE_START )
					{
						TaskFail( "Unable to make initial movement of charge\n" );
						return;
					}

					// Crash unless we're trying to stop already
					if ( eActivity != ACT_ALIENGRUNT_CHARGE_CANCEL )
					{
						if ( moveTrace.fStatus == AIMR_BLOCKED_WORLD && moveTrace.vHitNormal == vec3_origin )
						{
							SetActivity( ACT_ALIENGRUNT_CHARGE_CANCEL );
						}
						else
						{
							SetActivity( ACT_ALIENGRUNT_CHARGE_HIT );
						}
					}
				}
				else if ( moveTrace.pObstruction )
				{

					// If we hit an antlion, don't stop, but kill it
					/*if ( moveTrace.pObstruction->Classify() == CLASS_ALIENGRUNT )
					{
						if ( FClassnameIs( moveTrace.pObstruction, "npc_aliengrunt" ) )
						{
							// Crash unless we're trying to stop already
							if ( eActivity != ACT_ALIENGRUNT_CHARGE_CANCEL )
							{
								SetActivity( ACT_ALIENGRUNT_CHARGE_CANCEL );
							}
						}
					}*/

					////int disposition = CBaseCombatCharacter::GetDefaultRelationshipDispositionBetweenClasses( CLASS_ANTLION, moveTrace.pObstruction->Classify() );
					//if ( disposition == D_HT )
					//{
					//	// Crash unless we're trying to stop already
					//	if ( eActivity == ACT_ALIENGRUNT_CHARGE_LOOP )
					//	{
					//		SetActivity( ACT_ALIENGRUNT_CHARGE_HIT );
					//		//ChargeDamage( pEntity );
		
					//		ChargeDamage( moveTrace.pObstruction );
					//		moveTrace.pObstruction->ApplyAbsVelocityImpulse( ( BodyDirection2D() * 400 ) + Vector( 0, 0, 50 ) );
					//	}
					//}
					//else if ( disposition == D_LI )
					//{
					//	// Crash unless we're trying to stop already
					//	if ( eActivity == ACT_ALIENGRUNT_CHARGE_LOOP )
					//	{
					//		SetActivity( ACT_ALIENGRUNT_CHARGE_CANCEL );
					//	}
					//}
					//else
					//{
					//	//TERO: this reduces the cancel time so that if we have had impact shock recently
					//	m_flChargeCancelTime = m_flChargeCancelTime - 1.0f;

					//	//DevMsg("Cancel in %f\n", m_flChargeCancelTime - gpGlobals->curtime);
					//}
				}
			}
		}
		break;

	default:
		{
			BaseClass::RunTask( pTask );
			break;
		}
	}
}

int CNPC_AlienGrunt::GetSoundInterests ( void) 
{
	return	SOUND_WORLD	|
			SOUND_COMBAT	|
			SOUND_CARCASS	|
			SOUND_MEAT		|
			SOUND_GARBAGE	|
			SOUND_DANGER	|
			SOUND_PLAYER;
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
Class_T	CNPC_AlienGrunt::Classify ( void )
{
	return	CLASS_ANTLION;
}

int CNPC_AlienGrunt::RangeAttack1Conditions( float flDot, float flDist )
{
	if (GetEnemy() == NULL)
	{
		return( COND_NONE );
	}

	if ( gpGlobals->curtime < m_flNextAttack || HasSpawnFlags(SF_ALIENGRUNT_WAIT_FOR_GRAB) )
	{
		return( COND_NONE );
	}

	// Range attack is ineffective on manhack so never use it
	// Melee attack looks a lot better anyway
	/*if (GetEnemy()->Classify() == CLASS_MANHACK)
	{
		return( COND_NONE );
	}*/

	/*if ( flDist <= ALIENGRUNT_TOO_CLOSE_FOR_BEES )
	{
		DevMsg("AlienGrunt: too close for bees\n");
		return( COND_TOO_CLOSE_TO_ATTACK );
	}*/
	if ( flDist > 1500 * 12 )	// 1500ft max
	{
		//DevMsg("npc_aliengrunt: too far %f\n", flDist);
		return( COND_TOO_FAR_TO_ATTACK );
	}
	else if ( flDot < 0.65 )
	{
		return( COND_NOT_FACING_ATTACK );
	}

	return COND_ALIENGRUNT_BEES; //( COND_CAN_RANGE_ATTACK1 );
}


//-----------------------------------------------------------------------------
// Purpose: For innate melee attack
// Input  :
// Output :
//-----------------------------------------------------------------------------
int CNPC_AlienGrunt::MeleeAttack1Conditions ( float flDot, float flDist )
{
	if (flDist > 50 )
	{
		// Translate a hit vehicle into its passenger if found
		if ( GetEnemy() != NULL )
		{
			//CBaseCombatCharacter *pCCEnemy = GetEnemy()->MyCombatCharacterPointer();
			//if ( pCCEnemy != NULL && pCCEnemy->IsInAVehicle() )
			//	return MeleeAttack1ConditionsVsEnemyInVehicle( pCCEnemy, flDot );

			// If the player is holding an object, knock it down.
			if( GetEnemy()->IsPlayer() )
			{
				CBasePlayer *pPlayer = ToBasePlayer( GetEnemy() );

				Assert( pPlayer != NULL );

				// Is the player carrying something?
				CBaseEntity *pObject = GetPlayerHeldEntity(pPlayer);

				if( !pObject )
				{
					pObject = PhysCannonGetHeldEntity( pPlayer->GetActiveWeapon() );
				}

				if( pObject )
				{
					float flDist = pObject->WorldSpaceCenter().DistTo( WorldSpaceCenter() );

					if( flDist <= 50 )
						return COND_CAN_MELEE_ATTACK1;
				}
			}
		}
		return COND_TOO_FAR_TO_ATTACK;
	}

	if (flDot < 0.7)
	{
		//ClearCondition(COND_TOO_FAR_TO_ATTACK);
		return COND_NOT_FACING_ATTACK;
	}

	if ( ( HasSpawnFlags( SF_ALIENGRUNT_CAN_GRAB_RAGDOLL ) ) && ( m_flNextRagdollGrabTime < gpGlobals->curtime ) && GetEnemy() )
	{
		switch (GetEnemy()->Classify())
		{
		case CLASS_PLAYER_ALLY_VITAL:
			{
				if (GetEnemy()->m_iHealth > 10)
				{
					break;
				}
			}
		case CLASS_COMBINE:
		case CLASS_METROPOLICE:
			{
				if (FClassnameIs(GetEnemy(), "npc_combine_s") ||
					FClassnameIs(GetEnemy(), "npc_metropolice" ) ||
					FClassnameIs(GetEnemy(), "npc_noah" ) ||
					FClassnameIs(GetEnemy(), "npc_eloise" ) ||
					FClassnameIs(GetEnemy(), "npc_larson" ))
				{		
					return COND_ALIENGRUNT_CAN_GRAB_RAGDOLL;
				}
			}
			break;
		case CLASS_PLAYER:
			{
				CBasePlayer *pPlayer = ToBasePlayer( GetEnemy() );
				if (pPlayer && !pPlayer->GetVehicle())
				{
					return COND_ALIENGRUNT_CAN_GRAB_RAGDOLL;
				}
			}
			break;
		}
	}

	// Build a cube-shaped hull, the same hull that ClawAttack() is going to use.
	Vector vecMins = GetHullMins();
	Vector vecMaxs = GetHullMaxs();
	vecMins.z = vecMins.x;
	vecMaxs.z = vecMaxs.x;

	Vector forward;
	GetVectors( &forward, NULL, NULL );

	trace_t	tr;
	CTraceFilterNav traceFilter( this, false, this, COLLISION_GROUP_NONE );
	AI_TraceHull( WorldSpaceCenter(), WorldSpaceCenter() + forward * 50, vecMins, vecMaxs, MASK_NPCSOLID, &traceFilter, &tr );

	if( tr.fraction == 1.0 || !tr.m_pEnt )
	{
		// This attack would miss completely. Trick the zombie into moving around some more.
		return COND_TOO_FAR_TO_ATTACK;
	}

	if( tr.m_pEnt == GetEnemy() || tr.m_pEnt->IsNPC() || (tr.m_pEnt->m_takedamage == DAMAGE_YES && (dynamic_cast<CBreakableProp*>(tr.m_pEnt))) )
	{
		// -Let the zombie swipe at his enemy if he's going to hit them.
		// -Also let him swipe at NPC's that happen to be between the zombie and the enemy. 
		//  This makes mobs of zombies seem more rowdy since it doesn't leave guys in the back row standing around.
		// -Also let him swipe at things that takedamage, under the assumptions that they can be broken.
		return COND_CAN_MELEE_ATTACK1;
	}

	if( tr.m_pEnt->IsBSPModel() )
	{
		// The trace hit something solid, but it's not the enemy. If this item is closer to the zombie than
		// the enemy is, treat this as an obstruction.
		Vector vecToEnemy = GetEnemy()->WorldSpaceCenter() - WorldSpaceCenter();
		Vector vecTrace = tr.endpos - tr.startpos;

		if( vecTrace.Length2DSqr() < vecToEnemy.Length2DSqr() )
		{
			return COND_ALIENGRUNT_LOCAL_MELEE_OBSTRUCTION;
		}
	}

	if ( !tr.m_pEnt->IsWorld() && GetEnemy() && GetEnemy()->GetGroundEntity() == tr.m_pEnt )
	{
		//Try to swat whatever the player is standing on instead of acting like a dill.
		return COND_CAN_MELEE_ATTACK1;
	}

	// Move around some more
	return COND_TOO_FAR_TO_ATTACK;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static bool IsMovablePhysicsObject( CBaseEntity *pEntity )
{
	return pEntity && pEntity->GetMoveType() == MOVETYPE_VPHYSICS && pEntity->VPhysicsGetObject() && pEntity->VPhysicsGetObject()->IsMoveable();
}



void CNPC_AlienGrunt::Claw( int iAttachment)
{
	CBaseEntity *pHurt = CheckTraceHullAttack( 50, Vector(-10,-10,-10), Vector(10,10,10),sk_aliengrunt_dmg_claw.GetFloat(), DMG_SLASH );
	if ( pHurt )
	{
		// Play a random attack hit sound
		EmitSound( "NPC_Vortigaunt.Claw" );

		CBasePlayer *pPlayer = ToBasePlayer( pHurt );

		if ( pPlayer != NULL && !(pPlayer->GetFlags() & FL_GODMODE ) )
		{
			Vector vecOrigin; //, forward;
			QAngle angles;
			GetAttachment( iAttachment, vecOrigin, angles );

			vecOrigin = ( pPlayer->WorldSpaceCenter() - vecOrigin);
			vecOrigin *= 2.0f;

			//GetVectors( &forward, NULL, NULL );

			pPlayer->ViewPunch( QAngle(5,0,-18) );
			pPlayer->VelocityPunch( vecOrigin );
			
			// Shake the screen
			UTIL_ScreenShake( pPlayer->GetAbsOrigin(), 100.0, 1.5, 1.0, 2, SHAKE_START );

			// Red damage indicator
			color32 red = { 128, 0, 0, 128 };
			UTIL_ScreenFade( pPlayer, red, 1.0f, 0.1f, FFADE_IN );
		}
		else if ( !pPlayer )
		{
			if ( IsMovablePhysicsObject( pHurt ) )
			{
				// If it's a vphysics object that's too heavy, crash into it too.
				IPhysicsObject *pPhysics = pHurt->VPhysicsGetObject();
				if ( pPhysics )
				{
					// If the object is being held by the player, break it or make them drop it.
					if ( pPhysics->GetGameFlags() & FVPHYSICS_PLAYER_HELD )
					{
						// If it's breakable, break it.
						if ( pHurt->m_takedamage == DAMAGE_YES )
						{
							CBreakableProp *pBreak = dynamic_cast<CBreakableProp*>(pHurt);
							if ( pBreak )
							{
								CTakeDamageInfo info( this, this, 20, DMG_SLASH );
								pBreak->Break( this, info );
							}
						}
					}
				}
			}
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: Called when we are trying to open a prop_door and it's time to start
//			the door moving. This is called either in response to an anim event
//			or as a fallback when we don't have an appropriate open activity.
//-----------------------------------------------------------------------------
void CNPC_AlienGrunt::OpenPropDoorNow( CBasePropDoor *pDoor )
{
	// we accept the door if we are not going for a physics entity or we already have a door
	//if ( (m_NPCState == NPC_STATE_COMBAT || m_NPCState == NPC_STATE_ALERT )&& pDoor->HasSpawnFlags(SF_BREAKABLE_BY_AGRUNTS))
	//{
	//	m_hDoor = pDoor;
	//	//DevMsg("npc_aliengrunt: a breakable door during OpenPropDoornow(), grunt smash!\n");
	//	return;
	//}

	// Start the door moving.
	pDoor->NPCOpenDoor(this);

	// Wait for the door to finish opening before trying to move through the doorway.
	m_flMoveWaitFinished = gpGlobals->curtime + pDoor->GetOpenInterval();
}


//TERO: The swat AE_ copied from the Base Zombie
void CNPC_AlienGrunt::SwatItem()
{
	Vector v;
	CBaseEntity *pPhysicsEntity = m_hPhysicsEnt;
	if( !pPhysicsEntity )
	{
		DevWarning( "**Alie Grunt: Missing my physics ent!!" );
		m_hPhysicsEnt = NULL;
		return;
	}

	CBasePropDoor *pDoor = dynamic_cast<CBasePropDoor*>((CBaseEntity*)m_hPhysicsEnt);
	if (pDoor)
	{
		// add some spin so the object doesn't appear to just fly in a straight line
		// Also this spin will move the object slightly as it will press on whatever the object
		// is resting on.
		AngularImpulse angVelocity( random->RandomFloat(-180, 180), 20, random->RandomFloat(-360, 360) );

		ClearCondition( COND_ALIENGRUNT_CAN_ATTACK_DOOR ); 
		m_hDoor = NULL;

		//pDoor->BreakDoors(WorldSpaceCenter(), angVelocity);
		//EmitSound("d1_trainstation_03.breakin_doorkick");
		//return;
	}
			
	IPhysicsObject *pPhysObj = pPhysicsEntity->VPhysicsGetObject();

	if( !pPhysObj )
	{
		DevWarning( "**Alien Grunt: No Physics Object for physics Ent!" );
		return;
	}

	CBaseEntity *pEnemy = GetEnemy();

	if (m_hSetTarget == m_hPhysicsEnt)
	{
		pEnemy			= m_hSetTarget;
		m_hSetTarget	= NULL;
	}

	if ( pEnemy )
	{

		EmitSound( "NPC_Vortigaunt.Claw" );
		PhysicsImpactSound( pEnemy, pPhysObj, CHAN_BODY, pPhysObj->GetMaterialIndex(), physprops->GetSurfaceIndex("flesh"), 0.5, 800 );

		Vector physicsCenter = pPhysicsEntity->WorldSpaceCenter();
		v = pEnemy->WorldSpaceCenter() - physicsCenter;
		VectorNormalize(v);

		// Send the object at 800 in/sec toward the enemy.  Add 200 in/sec up velocity to keep it
		// in the air for a second or so.
		v = v * 1000;
		v.z += 200;

		// add some spin so the object doesn't appear to just fly in a straight line
		// Also this spin will move the object slightly as it will press on whatever the object
		// is resting on.
		AngularImpulse angVelocity( random->RandomFloat(-180, 180), 20, random->RandomFloat(-360, 360) );

		pPhysObj->AddVelocity( &v, &angVelocity );

		// If we don't put the object scan time well into the future, the zombie
		// will re-select the object he just hit as it is flying away from him.
		// It will likely always be the nearest object because the zombie moved
		// close enough to it to hit it.
		m_hPhysicsEnt = NULL;

		m_flNextSwatScan = gpGlobals->curtime + ALIENGRUNT_SWAT_DELAY;

		return;
	}

}

void CNPC_AlienGrunt::HandleAnimEvent( animevent_t *pEvent )
{
	if ( pEvent->event == AE_ALIENGRUNT_HAND_LEFT )
	{
		if (IsCurSchedule( SCHED_ALIENGRUNT_SWATITEM) || IsCurSchedule(SCHED_ALIENGRUNT_MOVE_SWATITEM) || IsCurSchedule(SCHED_ALIENGRUNT_ATTACKITEM) )
			SwatItem();
		else 
			Claw( m_iLeftHandAttachment );

		return;
	}

	if ( pEvent->event == AE_ALIENGRUNT_HAND_RIGHT )
	{
		if (IsCurSchedule( SCHED_ALIENGRUNT_SWATITEM) || IsCurSchedule(SCHED_ALIENGRUNT_MOVE_SWATITEM) || IsCurSchedule(SCHED_ALIENGRUNT_ATTACKITEM) )
			SwatItem();
		else 
			Claw( m_iRightHandAttachment );

		return;
	}

	if ( pEvent->event == AE_ALIENGRUNT_SHOOTBEES )
	{
		ShootBees();

		/*CSoundEnt::InsertSound( SOUND_COMBAT|SOUND_CONTEXT_GUNFIRE, GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, this, SOUNDENT_CHANNEL_WEAPON, GetEnemy() );
		EmitSound( "NPC_AlienGrunt.Shoot" );

		m_iBees--;

		if (m_iBees <= 0)
		{
			m_flNextAttack = gpGlobals->curtime + ALIENGRUNT_NEXT_ATTACK_DELAY;

		
			if (m_iHealth < 25)
			{
				m_iBees = ALIENGRUNT_BEE_BURST_SIZE_MAX;
			}
			else
			{
				m_iBees = ALIENGRUNT_BEE_BURST_SIZE_MIN;
			}
		}
		else
		{
			m_flNextAttack = gpGlobals->curtime + ALIENGRUNT_NEXT_ATTACK_DELAY_SMALL;
		}*/

		return;
	}

#ifdef ALIENGRUNT_CUSTOM_FOOT_AE
	if ( pEvent->event == AE_ALIENGRUNT_FOOT_LEFT ) 
#else
	if ( pEvent->event == AE_NPC_LEFTFOOT )
#endif
	{
		if (GetWaterLevel() != WL_NotInWater)
		{
			EmitSound("Water.StepLeft", pEvent->eventtime );
		}
		else
		{
			EmitSound( "NPC_Vortigaunt.FootstepLeft", pEvent->eventtime );
		}
		return;
	}

#ifdef ALIENGRUNT_CUSTOM_FOOT_AE
	if ( pEvent->event == AE_ALIENGRUNT_FOOT_RIGHT ) 
#else
	if ( pEvent->event == AE_NPC_RIGHTFOOT )
#endif
	{
		if (GetWaterLevel() != WL_NotInWater)
		{
			EmitSound("Water.StepRight", pEvent->eventtime );
		}
		else
		{
			EmitSound( "NPC_Vortigaunt.FootstepRight", pEvent->eventtime );
		}
		return;
	}
	if ( pEvent->event == AE_ALIENGRUNT_SWING_SOUND )
	{
		EmitSound( "NPC_Vortigaunt.Swing" );	
		return;
	}

	if ( pEvent->event == AE_ALIENGRUNT_GRAB_RAGDOLL )
	{
		//DevMsg("Grabbing a ragdoll\n");
		GrabRagdoll();
		return;
	}

	if ( pEvent->event == AE_ALIENGRUNT_THROW_RAGDOLL)
	{
		//DevMsg("Throwing a ragdoll\n");
		ThrowRagdoll();
		return;
	}

	if ( pEvent->event == AE_ALIENGRUNT_CHARGE_HIT )
	{
		UTIL_ScreenShake( GetAbsOrigin(), 32.0f, 4.0f, 1.0f, 512, SHAKE_START );
		EmitSound( "NPC_Vortigaunt.Claw" );

		Vector	startPos = GetAbsOrigin();
		float	checkSize = ( CollisionProp()->BoundingRadius() + 8.0f );
		Vector	endPos = startPos + ( BodyDirection3D() * checkSize );

		CTraceFilterChargeAlienGrunt traceFilter( this, COLLISION_GROUP_NONE, this );

		Ray_t ray;
		ray.Init( startPos, endPos, GetHullMins(), GetHullMaxs() );

		trace_t tr;
		enginetrace->TraceRay( ray, MASK_SHOT, &traceFilter, &tr );

		// Cause a shock wave from this point which will distrupt nearby physics objects
		ImpactShock( tr.endpos, 100, 30 );
		return;
	}

	
	BaseClass::HandleAnimEvent( pEvent );
}

Activity CNPC_AlienGrunt::NPC_TranslateActivity( Activity eNewActivity )
{
	/*if (eNewActivity == ACT_ALIENGRUNT_TO_ACTION || eNewActivity == ACT_RANGE_ATTACK1 )
	{
		m_bShouldAim = true;
	} 
	else
	{
		m_bShouldAim = false;
	}*/

	//TERO: If the Alien Grunt is close enough to his enemy, he will walk to him instead of running
	/*if (eNewActivity == ACT_RUN)  //Commented out because it doesn't look that great
	{
		if (GetEnemy()!=NULL)
		{
			float flDist=UTIL_DistApprox( GetAbsOrigin(), GetEnemy()->GetAbsOrigin());
			if (flDist<ALIENGRUNT_SHOULD_WALK_DIST)
				eNewActivity = ACT_WALK;
		}
	}*/

	eNewActivity = TranslateAimingActivity( (Activity) eNewActivity );

	if ((eNewActivity == ACT_SIGNAL3)									&& 
		(SelectWeightedSequence ( ACT_SIGNAL3 ) == ACTIVITY_NOT_AVAILABLE)	)
	{
		eNewActivity = ACT_IDLE;
	}

	if (eNewActivity == ACT_MELEE_ATTACK1)
	{
		// If enemy is low pick ATTACK2 (kick)
		if (GetEnemy() != NULL && (GetEnemy()->EyePosition().z - GetLocalOrigin().z) < 20)
		{
			return ACT_MELEE_ATTACK2;
		}
	}
	return BaseClass::NPC_TranslateActivity( eNewActivity );
}

//------------------------------------------------------------------------------
// Purpose : If I've been in alert for a while and nothing's happened, 
//			 go back to idle
// Input   :
// Output  :
//------------------------------------------------------------------------------
bool CNPC_AlienGrunt::ShouldGoToIdleState( void )
{
	if (m_flLastStateChangeTime + 10 < gpGlobals->curtime)
	{
		return true;
	}
	return false;
}

void CNPC_AlienGrunt::Spawn()
{
	Precache();
	SetModel("models/AlienGrunt.mdl");

	BaseClass::Spawn();

	SetHullType(HULL_WIDE_HUMAN);
	SetHullSizeNormal();

	SetNavType( NAV_GROUND );

	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_NOT_STANDABLE );
	SetMoveType( MOVETYPE_STEP );
	m_bloodColor		= BLOOD_COLOR_GREEN;
	m_iHealth 			= sk_aliengrunt_health.GetFloat();
	m_iMaxHealth		= m_iHealth;
	m_flFieldOfView		= VIEW_FIELD_NARROW; // NOTE: we need a wide field of view so npc will notice player and say hello
	m_NPCState			= NPC_STATE_NONE;

	//TERO: we can't dissolve with full healths
	AddEFlags( EFL_NO_DISSOLVE );

	GetExpresser()->SetVoicePitch( random->RandomInt( 85, 110 ) );

	CapabilitiesClear();
	CapabilitiesAdd( bits_CAP_SQUAD );

	CapabilitiesAdd ( bits_CAP_ANIMATEDFACE | bits_CAP_TURN_HEAD | bits_CAP_MOVE_GROUND );
	CapabilitiesAdd ( bits_CAP_INNATE_RANGE_ATTACK1 );
	CapabilitiesAdd	( bits_CAP_MOVE_SHOOT );
	CapabilitiesAdd ( bits_CAP_AIM_GUN );
	CapabilitiesAdd	( bits_CAP_INNATE_MELEE_ATTACK1 );
	CapabilitiesAdd	( bits_CAP_DOORS_GROUP );
	CapabilitiesAdd ( bits_CAP_NO_HIT_SQUADMATES | bits_CAP_FRIENDLY_DMG_IMMUNE );

	//CapabilitiesAdd ( bits_CAP_MOVE_FLY ); //TERO: this may have to be here for the bee's capabilities check
	
	m_flEyeIntegRate		= 0.6;		// Got a big eyeball so turn it slower

	//m_bStopLoopingSounds	= false;

	m_bShouldAim = false;

	m_iLeftHandAttachment = LookupAttachment( ALIENGRUNT_LEFT_HAND );
	m_iRightHandAttachment = LookupAttachment( ALIENGRUNT_RIGHT_HAND );

	NPCInit();

	m_HideSoundTime	  = 0;

	#ifdef ALIENGRUNT_ALWAYS_RAGDOLL_GRAB
	if (aliengrunt_always_ragdoll_grab.GetBool())
	{
		AddSpawnFlags( SF_ALIENGRUNT_CAN_GRAB_RAGDOLL );
	}
	#endif

	m_flBurnDamageResetTime = 0;
	m_flBurnDamage = 0;

	m_hRagdoll				= NULL;
	m_flNextRagdollGrabTime = 0;
	m_hPlayerConstrain		= NULL;

	m_hPhysicsEnt = NULL;
	m_hObstructor = NULL;

	m_hSetTarget  = NULL;
	m_hDoor		  = NULL;

	m_flNextChargeTime = gpGlobals->curtime; // + aliengrunt_charge_delay_max.GetFloat();
	m_flChargeCancelTime = 0;

	m_iBees = ALIENGRUNT_BEE_BURST_SIZE_MIN;
	
	// Don't allow us to skip animation setup because our attachments are critical to us!
	SetBoneCacheFlags( BCF_NO_ANIMATION_SKIP );
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CNPC_AlienGrunt::Precache()
{
	PrecacheModel( "models/AlienGrunt.mdl" );

	UTIL_PrecacheOther( "bee_missile" ); 

	TalkInit();

	PrecacheScriptSound( "NPC_AlienGrunt.Idle" );
	PrecacheScriptSound( "NPC_AlienGrunt.Hide1" );
	PrecacheScriptSound( "NPC_AlienGrunt.Hide2" );
	PrecacheScriptSound( "NPC_AlienGrunt.Hide3" );
	PrecacheScriptSound( "NPC_AlienGrunt.Hide4" );
	PrecacheScriptSound( "NPC_AlienGrunt.Alert" );
	PrecacheScriptSound( "NPC_AlienGrunt.Combine" );
	PrecacheScriptSound( "NPC_AlienGrunt.CombineHere" );
	PrecacheScriptSound( "NPC_AlienGrunt.GoAway" );
	PrecacheScriptSound( "NPC_AlienGrunt.GetAway" );
	PrecacheScriptSound( "NPC_AlienGrunt.Attack" );


	PrecacheScriptSound( "NPC_AlienGrunt.Pain" );
	PrecacheScriptSound( "NPC_AlienGrunt.Die" );
	PrecacheScriptSound( "NPC_AlienGrunt.Shoot" );
	PrecacheScriptSound( "NPC_Vortigaunt.FootstepLeft" );
	PrecacheScriptSound( "NPC_Vortigaunt.FootstepRight" );
	PrecacheScriptSound( "NPC_Vortigaunt.Claw" );
	PrecacheScriptSound( "NPC_Vortigaunt.Swing" );

	PrecacheScriptSound( "d1_trainstation_03.breakin_doorkick");

	PrecacheScriptSound( "NPC_Hunter.ChargeHitEnemy" );

	PrecacheScriptSound( "Water.StepLeft" );
	PrecacheScriptSound( "Water.StepRight" );

	BaseClass::Precache();
}	

int	CNPC_AlienGrunt::OnTakeDamage_Alive( const CTakeDamageInfo &info )
{
	// make sure friends talk about it if player hurts talkmonsters...
	/*int ret = BaseClass::OnTakeDamage_Alive( info );
	if (!IsAlive())
	{
		return ret;
	}*/

	if ( !IsOnFire() && info.GetDamageType() & DMG_BURN )
	{
		//
		// If we take more than ten percent of our health in burn damage within a five
		// second interval, we should catch on fire.
		//
		m_flBurnDamage += info.GetDamage();
		m_flBurnDamageResetTime = gpGlobals->curtime + 5;

		if ( m_flBurnDamage >= m_iMaxHealth * 0.1 )
		{
			Ignite( 100.0f );
		}
	}

	if ( IsEFlagSet( EFL_NO_DISSOLVE ) && m_iHealth < (m_iMaxHealth / 2) )
	{
		RemoveEFlags( EFL_NO_DISSOLVE );
	}

	//PainSound();

	return  BaseClass::OnTakeDamage_Alive( info ); //ret;
}

//-----------------------------------------------------------------------------
// Purpose: Returns true if a reasonable jumping distance
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool CNPC_AlienGrunt::IsJumpLegal( const Vector &startPos, const Vector &apex, const Vector &endPos ) const
{
	const float MAX_JUMP_RISE		= 40;
	const float MAX_JUMP_DROP		= 1024;
	const float MAX_JUMP_DISTANCE	= 250;

	return BaseClass::IsJumpLegal( startPos, apex, endPos, MAX_JUMP_RISE, MAX_JUMP_DROP, MAX_JUMP_DISTANCE );
}


void CNPC_AlienGrunt::PainSound ( const CTakeDamageInfo &info )
{

	EmitSound( "NPC_AlienGrunt.Pain" );
}

void CNPC_AlienGrunt::DeathSound( const CTakeDamageInfo &info ) 
{
	EmitSound( "NPC_AlienGrunt.Die" );
}

void CNPC_AlienGrunt::IdleSound( void )
{

	EmitSound( "NPC_AlienGrunt.Idle" );
}

void CNPC_AlienGrunt::HideSound( void )
{
	if ( GetEnemy() != NULL && IsOkToCombatSpeak() )
	{
		if (gpGlobals->curtime < m_HideSoundTime)
			return;
	
		m_HideSoundTime = gpGlobals->curtime + random->RandomFloat(2.5, 6.75);

		Speak( AGRUNT_HIDE );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Play a random attack sound.
//-----------------------------------------------------------------------------
void CNPC_AlienGrunt::AttackSound( void )
{
	if ( GetEnemy() != NULL && IsOkToCombatSpeak() )
	{
		Speak( AGRUNT_ATTACK );
	}
}

void CNPC_AlienGrunt::ChargeSound( void )
{
	if ( GetEnemy() != NULL && IsOkToCombatSpeak() )
	{
		Speak ( AGRUNT_CHARGE );
	}
}

void CNPC_AlienGrunt::AlertSound( void )
{
	if ( GetEnemy() != NULL && IsOkToCombatSpeak() )
	{
		Speak( AGRUNT_ALERT );
	}
}

#if 1
void CNPC_AlienGrunt::TraceAttack(const CTakeDamageInfo &inputInfo, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator)
#else
void CNPC_AlienGrunt::TraceAttack( const CTakeDamageInfo &inputInfo, const Vector &vecDir, trace_t *ptr )
#endif
{
#if 1
	BaseClass::TraceAttack( inputInfo, vecDir, ptr, pAccumulator );
#else
	BaseClass::TraceAttack( inputInfo, vecDir, ptr );
#endif
}

int CNPC_AlienGrunt::TranslateSchedule( int scheduleType )
{
	int baseType;

	switch( scheduleType )
	{
	/*case SCHED_CHASE_ENEMY:
		if ( HasCondition( COND_ALIENGRUNT_LOCAL_MELEE_OBSTRUCTION ) && !HasCondition(COND_TASK_FAILED) && IsCurSchedule( SCHED_CHASE_ENEMY, false ) )
		{
			return SCHED_COMBAT_PATROL;
		}
		return SCHED_CHASE_ENEMY;
		break;*/
	case SCHED_RUN_RANDOM:
		{
			if (GetEnemy())
			{
				//TERO: run_random seems to make the Grunt ignore the enemy until the task is done
				return SCHED_ALIENGRUNT_STAND;
			}
			return BaseClass::TranslateSchedule( scheduleType );
		}
		break;
	case SCHED_ALIENGRUNT_SWATITEM:
		// If the object is far away, move and swat it. If it's close, just swat it.
		if( DistToPhysicsEnt() > ALIENGRUNT_PHYSOBJ_SWATDIST )
		{
			return SCHED_ALIENGRUNT_MOVE_SWATITEM;
		}
		else
		{
			return SCHED_ALIENGRUNT_SWATITEM;
		}
		break;

	case SCHED_RANGE_ATTACK1:
		{
		return SCHED_ALIENGRUNT_SHOOTBEES;
		break;
		}
	case SCHED_MELEE_ATTACK1:
		{
		return SCHED_ALIENGRUNT_MELEE_ATTACK;
		break;
		}
	case SCHED_MELEE_ATTACK2:
		{
		return SCHED_ALIENGRUNT_GRAB_RAGDOLL;
		break;
		}
	case SCHED_IDLE_STAND:
		{
			// call base class default so that scientist will talk
			// when standing during idle
			baseType = BaseClass::TranslateSchedule(scheduleType);

			if (baseType == SCHED_IDLE_STAND)
			{

			/*	if (HasCondition( COND_TOO_FAR_TO_ATTACK ))
					return SCHED_CHASE_ENEMY; */

				// just look straight ahead
				return SCHED_ALIENGRUNT_STAND;
			}
			else
				return baseType;	
			break;

		}
	case SCHED_FAIL_ESTABLISH_LINE_OF_FIRE:
		{
			// Fail schedule doesn't go through SelectSchedule()
			// So we have to clear beams and glow here
			//ClearBeams();
			//EndHandGlow();

			return SCHED_COMBAT_FACE;
			break;
		}
	case SCHED_CHASE_ENEMY_FAILED:
		{
			baseType = BaseClass::TranslateSchedule(scheduleType);
			if ( baseType != SCHED_CHASE_ENEMY_FAILED )
				return baseType;

			if (HasMemory(bits_MEMORY_INCOVER))
			{
				// Occasionally run somewhere so I don't just
				// stand around forever if my enemy is stationary
				if (random->RandomInt(0,5) == 5)
				{
					return SCHED_PATROL_RUN;
				}
				else
				{
					return SCHED_ALIENGRUNT_STAND;
				}
			}
			break;
		}
	case SCHED_FAIL_TAKE_COVER:
		{
			// Fail schedule doesn't go through SelectSchedule()
			// So we have to clear beams and glow here
			//ClearBeams();
			//EndHandGlow();

			return SCHED_RUN_RANDOM;
			break;
		}
	}
	
	return BaseClass::TranslateSchedule( scheduleType );
}

//---------------------------------------------------------
//---------------------------------------------------------
//int	CNPC_AlienGrunt::SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode )
//{
//	if ( IsPathTaskFailure( taskFailCode ) &&
//		 //m_hObstructor != NULL && FClassnameIs( m_hObstructor, "prop_door_rotating") && m_hObstructor->HasSpawnFlags(SF_BREAKABLE_BY_AGRUNTS))
//	//{
//	//	m_hPhysicsEnt = m_hObstructor;
//	//	m_hObstructor = NULL;
//	//	//DevMsg("npc_aliengrunt: a breakable door in our way during SelectFailSchedule(), grunt smash!\n");
//	//	return SCHED_ALIENGRUNT_ATTACKITEM;
//	//}
//
//	//if ( IsPathTaskFailure( taskFailCode ) && 
//	//	m_hObstructor != NULL && m_hObstructor->VPhysicsGetObject() && 
//	//	 m_hObstructor->VPhysicsGetObject()->GetMass() < 100 )
//	//{
//	//	if ( !FClassnameIs( m_hObstructor, "physics_prop_ragdoll" ) && !FClassnameIs( m_hObstructor, "prop_ragdoll" ) )
//	//	{
//	//		m_hPhysicsEnt = m_hObstructor;
//	//		m_hObstructor = NULL;
//	//		return SCHED_ALIENGRUNT_ATTACKITEM;
//	//	}
//	//	//else
//	//	//	DevMsg("AlienGrunt: yikes! we almost took a ragdoll as m_hPhysicsEnt\n");
//	//}
//
//	//m_hObstructor = NULL;
//
//	return BaseClass::SelectFailSchedule( failedSchedule, failedTask, taskFailCode );
//}

//-----------------------------------------------------------------------------
// Determines the best type of flinch anim to play.
//-----------------------------------------------------------------------------
Activity CNPC_AlienGrunt::GetFlinchActivity( bool bHeavyDamage, bool bGesture )
{

	return BaseClass::GetFlinchActivity( bHeavyDamage, true );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_AlienGrunt::PlayFlinchGesture( void )
{
	BaseClass::PlayFlinchGesture();

	// To ensure old playtested difficulty stays the same, stop cops shooting for a bit after gesture flinches
	if (m_flNextAttack + 0.5 < gpGlobals->curtime )
		m_flNextAttack = gpGlobals->curtime + ALIENGRUNT_NEXT_ATTACK_DELAY;
}


int CNPC_AlienGrunt::SelectSchedule( void )
{
	if (HasCondition( COND_ENEMY_OCCLUDED ))
	{
		HideSound();
	}

	/*int nSched = SelectFlinchSchedule();
	if ( nSched != SCHED_NONE )
		return nSched;*/

	if ( BehaviorSelectSchedule() )
		return BaseClass::SelectSchedule();

	//TERO: moved here in case we need to open doors during alert or idle
	if ( HasCondition( COND_ALIENGRUNT_CAN_SWAT_ATTACK ) )
	{
		return SCHED_ALIENGRUNT_SWATITEM;
	}

	switch( m_NPCState )
	{
	case NPC_STATE_COMBAT:
		{
		
			// dead enemy
			if ( HasCondition( COND_ENEMY_DEAD ) )
			{
				// call base class, all code to handle dead enemies is centralized there.
				return BaseClass::SelectSchedule();
			}

			if ( HasCondition( COND_ALIENGRUNT_CAN_GRAB_RAGDOLL ) )
			{
				ClearCondition( COND_ALIENGRUNT_CANNOT_GRAB_RAGDOLL );
				return SCHED_ALIENGRUNT_GRAB_RAGDOLL;
			}

			if ( HasSpawnFlags( SF_ALIENGRUNT_WAIT_FOR_GRAB ) && !HasCondition( COND_CAN_MELEE_ATTACK1 ) )
			{
				return SCHED_CHASE_ENEMY;
			}


			if ( HasCondition( COND_ALIENGRUNT_SHOULD_CHARGE ) )
			{
				//DevMsg("npc_aliengrunt: SCHED_ALIENGRUNT_CHARGE\n"); 
				OccupyStrategySlot( SQUAD_SLOT_ALIENGRUNT_CHARGE );
				return SCHED_ALIENGRUNT_CHARGE;
			}

			if (HasCondition(COND_ENEMY_OCCLUDED))
			{
				// stand up, just in case
				/*Stand();
				DesireStand();*/

				if( GetEnemy() )
				{
					if (!(GetEnemy()->GetFlags() & FL_NOTARGET) && OccupyStrategySlotRange( SQUAD_SLOT_ATTACK1, SQUAD_SLOT_ATTACK2 ))
					{
						// Charge in and break the enemy's cover!
						return SCHED_ESTABLISH_LINE_OF_FIRE;
					}

					// If I'm a long, long way away, establish a LOF anyway. Once I get there I'll
					// start respecting the squad slots again.
					float flDistSq = GetEnemy()->WorldSpaceCenter().DistToSqr( WorldSpaceCenter() );
					if ( flDistSq > Square(3000) )
						return SCHED_ESTABLISH_LINE_OF_FIRE;
				}
			}


			if ( HasCondition( COND_SEE_ENEMY ) && 
				!HasCondition( COND_CAN_MELEE_ATTACK1 ) && 
				!HasCondition( COND_CAN_MELEE_ATTACK2 ) && 
				!HasCondition(COND_NOT_FACING_ATTACK) ) //TERO: added most recently to avoid crowbar kicking
			{
				if ( HasCondition( COND_ALIENGRUNT_SHOULD_PROTECT_SQUADMATE ) )
				{
					if ( HasCondition( COND_ALIENGRUNT_BEES ) || HasCondition( COND_CAN_RANGE_ATTACK1 ) )
					{
						return SCHED_ALIENGRUNT_SHOOTBEES;
					}
					else if ( gpGlobals->curtime < m_flNextAttack )
					{
						//TERO: we have just finished shooting few minutes ago, we probably will have another chance to shoot soon
						//		let's wait and see

						return SCHED_ALIENGRUNT_STAND;
					}
				}
				//HasCondition( COND_TOO_FAR_TO_ATTACK )  && 
				else if ( OccupyStrategySlotRange( SQUAD_SLOT_ATTACK1, SQUAD_SLOT_ATTACK2 ))
					//	  !HasCondition( COND_HEAR_DANGER ) &&  !HasCondition( COND_HEAR_MOVE_AWAY ))
				{
					return SCHED_ALIENGRUNT_PRESS_ATTACK;
				}
			}

			//TERO: if enemy goes behind a corner we want to shoot some bees after him
			//We need a better schedule here, something that shoots towards the last position we saw him
		}
		break;

	case NPC_STATE_ALERT:	
	case NPC_STATE_IDLE:

		if ( HasCondition( COND_HEAR_DANGER ) || HasCondition( COND_HEAR_MOVE_AWAY ))
		{
			return SCHED_TAKE_COVER_FROM_BEST_SOUND;
		}

		/*if ( HasCondition( COND_ENEMY_DEAD ) && IsOkToCombatEmitSound() )
		{
			EmitSound( AGRUNT_KILL );
		}*/
		break;
	}	
	
	return BaseClass::SelectSchedule();
}

/*bool CNPC_AlienGrunt::CanBeAnEnemyOf( CBaseEntity *pEnemy )
{
	if ( pEnemy->Classify() == CLASS_MANHACK &&
		(pEnemy->GetAbsOrigin() - GetAbsOrigin()).Length() > 128.0f)
	{
		return false;
	}

	return BaseClass::CanBeAnEnemyOf( CBaseEntity *pEnemy );
}*/

void CNPC_AlienGrunt::BuildScheduleTestBits( void )
{
	BaseClass::BuildScheduleTestBits();


	/*if ( ( ConditionInterruptsCurSchedule( COND_GIVE_WAY ) || 
		   IsCurSchedule(SCHED_HIDE_AND_RELOAD ) || 
		   IsCurSchedule(SCHED_RELOAD ) || 
		   IsCurSchedule(SCHED_STANDOFF ) || 
		   IsCurSchedule(SCHED_TAKE_COVER_FROM_ENEMY ) || 
		   IsCurSchedule(SCHED_COMBAT_FACE ) || 
		   IsCurSchedule(SCHED_ALERT_FACE )  ||
		   IsCurSchedule(SCHED_COMBAT_STAND ) || 
		   IsCurSchedule(SCHED_ALERT_FACE_BESTSOUND) ||
		   IsCurSchedule(SCHED_ALERT_STAND) ) )
	{
		SetCustomInterruptCondition( COND_PLAYER_PUSHING );
	}*/

	if (IsCurSchedule(SCHED_FAIL_ESTABLISH_LINE_OF_FIRE ) || 
		IsCurSchedule(SCHED_CHASE_ENEMY))
	{
		SetCustomInterruptCondition( COND_ALIENGRUNT_CAN_ATTACK_DOOR );
	}
}

void CNPC_AlienGrunt::ShootBees() 
{
		CSoundEnt::InsertSound( SOUND_COMBAT|SOUND_CONTEXT_GUNFIRE, GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, this, SOUNDENT_CHANNEL_WEAPON, GetEnemy() );
		EmitSound( "NPC_AlienGrunt.Shoot" );

		m_iBees--;

		if (m_iBees <= 0)
		{
			m_flNextAttack = gpGlobals->curtime + ALIENGRUNT_NEXT_ATTACK_DELAY;

		
			if (m_iHealth < 25)
			{
				m_iBees = ALIENGRUNT_BEE_BURST_SIZE_MAX;
			}
			else
			{
				m_iBees = ALIENGRUNT_BEE_BURST_SIZE_MIN;
			}
		}
		else
		{
			m_flNextAttack = gpGlobals->curtime + ALIENGRUNT_NEXT_ATTACK_DELAY_SMALL;
		}

		Vector vecOrigin;
		QAngle vecAngles;

		int handAttachment = LookupAttachment( "Hivehand_mouth" );
		GetAttachment( handAttachment, vecOrigin, vecAngles );

	//	Vector forward;
		//GetVectors( NULL, NULL, forward );

		Vector vecDir;

		AngleVectors(vecAngles, &vecDir);

		VectorNormalize( vecDir );

		Vector vecVelocity;
		VectorMultiply( vecDir, 50, vecVelocity );

		//QAngle angles;
		//VectorAngles( vecDir, angles );

//		CBee *pBee = CBee::Create( vecOrigin, vecAngles, this );
//		
//		if (pBee)
//		{
//			pBee->m_hOwner = this;
//			pBee->SetGracePeriod( 2 );
////			pBee->CreateNavigator();
//			pBee->SetEnemy( GetEnemy() );
//			//pBee->m_hTarget = GetEnemy();
//		}

		//Lets create a "muzzle flash"

		UTIL_BloodDrips( vecOrigin, vecVelocity, BLOOD_COLOR_YELLOW, 1 );

		/*CEffectData data;
		data.m_nEntIndex = entindex();
		data.m_nAttachmentIndex = handAttachment;
		data.m_flScale = 1.0f;
		DispatchEffect( "ChopperMuzzleFlash", data );*/

}

/*Vector CNPC_AlienGrunt::FindNodePositionForBee(Vector startPos, Vector enemyPos)
{
	Vector vTargetDir = enemyPos - startPos; //just saving in case nodes are not found

	SetNavType( NAV_FLY);
	
	//we can't get there straight, lets try to find an air node
	int	iNearestNode = GetNavigator()->GetNetwork()->NearestNodeToPoint( enemyPos, true);
		
		// GetPathfinder()->NearestNodeToPoint( enemyPos );

	if ( iNearestNode != NO_NODE )
	{

		// Get the node and move to it
		CAI_Node *pNode = GetNavigator()->GetNetwork()->GetNode( iNearestNode );
		if ( pNode )
		{

			Vector vecNodePos = pNode->GetPosition( HULL_TINY ) + Vector(0,0,35);
			vTargetDir = vecNodePos - startPos;

			NDebugOverlay::Box( vecNodePos, Vector(10,10,10), Vector(-10,-10,-10), 255, 0, 0, 0, 5 );
			DevMsg("Aliengrunt: giving node position to bee\n");
		}
	} 
	else
		DevMsg("Aliengrunt: no air node\n");

	SetNavType( NAV_GROUND );

	return vTargetDir;
}*/

void CNPC_AlienGrunt::InputAssault( inputdata_t &inputdata )
{
	m_AssaultBehavior.SetParameters( AllocPooledString(inputdata.value.String()), CUE_DONT_WAIT, RALLY_POINT_SELECT_DEFAULT );
}


bool CNPC_AlienGrunt::OnInsufficientStopDist( AILocalMoveGoal_t *pMoveGoal, float distClear, AIMoveResult_t *pResult )
{
	if ( pMoveGoal->directTrace.fStatus == AIMR_BLOCKED_ENTITY && gpGlobals->curtime >= m_flNextSwat )
	{
		m_hObstructor = pMoveGoal->directTrace.pObstruction;
	}
	
	return BaseClass::OnInsufficientStopDist( pMoveGoal, distClear, pResult );
}

int CNPC_AlienGrunt::GetSwatActivity( void )
{
	Vector vecMyCenter;
	Vector vecObjCenter;

	vecMyCenter = WorldSpaceCenter();
	vecObjCenter = m_hPhysicsEnt->WorldSpaceCenter();
	float flZDiff;

	flZDiff = vecMyCenter.z - vecObjCenter.z;

		
	if( flZDiff < 0 )
	{
			return ACT_MELEE_ATTACK1;
	}
	return ACT_MELEE_ATTACK2;
}

bool CNPC_AlienGrunt::FindNearestPhysicsObject( int iMaxMass )
{
	CBaseEntity		*pList[ ALIENGRUNT_PHYSICS_SEARCH_DEPTH ];
	CBaseEntity		*pNearest = NULL;
	float			flDist;
	IPhysicsObject	*pPhysObj;
	int				i;
	Vector			vecDirToEnemy;
	Vector			vecDirToObject;

	//TERO: commented out because we want swats even in assaults when the Grunt may have not seen the player yet
	if ( !GetEnemy() )
	{
		// Can't swat, or no enemy, so no swat.
		m_hPhysicsEnt = NULL;
		return false;
	}

	vecDirToEnemy = GetEnemy()->GetAbsOrigin() - GetAbsOrigin();
	float dist = VectorNormalize(vecDirToEnemy);
	vecDirToEnemy.z = 0;

	if( dist > ALIENGRUNT_PLAYER_MAX_SWAT_DIST )
	{
		// Player is too far away. Don't bother 
		// trying to swat anything at them until
		// they are closer.
		return false;
	}

	float flNearestDist = min( dist, ALIENGRUNT_FARTHEST_PHYSICS_OBJECT * 0.5 );
	Vector vecDelta( flNearestDist, flNearestDist, GetHullHeight() * 2.0 );

	class CAlienGruntSwatEntitiesEnum : public CFlaggedEntitiesEnum
	{
	public:
		CAlienGruntSwatEntitiesEnum( CBaseEntity **pList, int listMax, int iMaxMass )
		 :	CFlaggedEntitiesEnum( pList, listMax, 0 ),
			m_iMaxMass( iMaxMass )
		{
		}

		virtual IterationRetval_t EnumElement( IHandleEntity *pHandleEntity )
		{
			CBaseEntity *pEntity = gEntList.GetBaseEntity( pHandleEntity->GetRefEHandle() );
			if ( pEntity && 
				 pEntity->VPhysicsGetObject() && 
				 pEntity->VPhysicsGetObject()->GetMass() <= m_iMaxMass && 
				 pEntity->VPhysicsGetObject()->GetMass() >= ALIENGRUNT_MIN_PHYSOBJ_MASS &&
				 pEntity->VPhysicsGetObject()->IsAsleep() && 
				 pEntity->VPhysicsGetObject()->IsMoveable() )
			{
				return CFlaggedEntitiesEnum::EnumElement( pHandleEntity );
			}
			return ITERATION_CONTINUE;
		}

		int m_iMaxMass;
	};

	CAlienGruntSwatEntitiesEnum swatEnum( pList, ALIENGRUNT_PHYSICS_SEARCH_DEPTH, iMaxMass );

	int count = UTIL_EntitiesInBox( GetAbsOrigin() - vecDelta, GetAbsOrigin() + vecDelta, &swatEnum );

	// magically know where they are
	Vector vecAlienGruntKnees;
	CollisionProp()->NormalizedToWorldSpace( Vector( 0.5f, 0.5f, 0.25f ), &vecAlienGruntKnees );

	for( i = 0 ; i < count ; i++ )
	{
		pPhysObj = pList[ i ]->VPhysicsGetObject();

		Assert( !( !pPhysObj || pPhysObj->GetMass() > iMaxMass || !pPhysObj->IsAsleep() ) );

		Vector center = pList[ i ]->WorldSpaceCenter();
		flDist = UTIL_DistApprox2D( GetAbsOrigin(), center );

		if( flDist >= flNearestDist )
			continue;

		// This object is closer... but is it between the player and the zombie?
		vecDirToObject = pList[ i ]->WorldSpaceCenter() - GetAbsOrigin();
		VectorNormalize(vecDirToObject);
		vecDirToObject.z = 0;

		if( DotProduct( vecDirToEnemy, vecDirToObject ) < 0.8 )
			continue;

		if( flDist >= UTIL_DistApprox2D( center, GetEnemy()->GetAbsOrigin() ) )
			continue;

		// don't swat things where the highest point is under my knees
		// NOTE: This is a rough test; a more exact test is going to occur below
		if ( (center.z + pList[i]->BoundingRadius()) < vecAlienGruntKnees.z )
			continue;

		// don't swat things that are over my head.
		if( center.z > EyePosition().z )
			continue;

		vcollide_t *pCollide = modelinfo->GetVCollide( pList[i]->GetModelIndex() );
		
		Vector objMins, objMaxs;
		physcollision->CollideGetAABB( &objMins, &objMaxs, pCollide->solids[0], pList[i]->GetAbsOrigin(), pList[i]->GetAbsAngles() );

		if ( objMaxs.z < vecAlienGruntKnees.z )
			continue;

		if ( !FVisible( pList[i] ) )
			continue;

		if ( hl2_episodic.GetBool() )
		{
			// Skip things that the enemy can't see. Do we want this as a general thing? 
			// The case for this feature is that zombies who are pursuing the player will
			// stop along the way to swat objects at the player who is around the corner or 
			// otherwise not in a place that the object has a hope of hitting. This diversion
			// makes the zombies very late (in a random fashion) getting where they are going. (sjb 1/2/06)
			if( !GetEnemy()->FVisible( pList[i] ) )
				continue;
		}

		// Make this the last check, since it makes a string.
		// Don't swat server ragdolls! -TERO: why the hell not? -because it makes the game crash, dumbass! -oh... sorry...
		if ( FClassnameIs( pList[ i ], "physics_prop_ragdoll" ) )
			continue;
			
		if ( FClassnameIs( pList[ i ], "prop_ragdoll" ) )
			continue;

		// The object must also be closer to the zombie than it is to the enemy
		pNearest = pList[ i ];
		flNearestDist = flDist;
	}

	m_hPhysicsEnt = pNearest;

	if( m_hPhysicsEnt == NULL )
	{
		return false;
	}
	else
	{
		return true;
	}
}

float CNPC_AlienGrunt::DistToPhysicsEnt( void )
{
	//return ( GetLocalOrigin() - m_hPhysicsEnt->GetLocalOrigin() ).Length();
	if ( m_hPhysicsEnt != NULL )
		return UTIL_DistApprox2D( GetAbsOrigin(), m_hPhysicsEnt->WorldSpaceCenter() );
	return ALIENGRUNT_PHYSOBJ_SWATDIST + 1;
}

/*void CNPC_AlienGrunt::GatherEnemyConditions( CBaseEntity *pEnemy )
{
	DevMsg("Gathering enemy conditions\n");
	if (FCanCheckAttacks())
	{
		DevMsg("FCanCheckAttacks()\n");
	}

	BaseClass::GatherEnemyConditions( pEnemy );
}*/

//---------------------------------------------------------
//---------------------------------------------------------
void CNPC_AlienGrunt::GatherConditions( void )
{
	ClearCondition( COND_ALIENGRUNT_LOCAL_MELEE_OBSTRUCTION );

	//DevMsg("Gathering conditions\n");

	BaseClass::GatherConditions();

	/*if (HasCondition( COND_CAN_RANGE_ATTACK1 ))
	{
		DevMsg("yay\n");
	}*/

	if( m_NPCState == NPC_STATE_COMBAT && m_hSetTarget == NULL && m_hDoor == NULL)
	{
		// This check for !m_pPhysicsEnt prevents a crashing bug, but also
		// eliminates the zombie picking a better physics object if one happens to fall
		// between him and the object he's heading for already. 
		if( gpGlobals->curtime >= m_flNextSwatScan && (m_hPhysicsEnt == NULL) )
		{
			FindNearestPhysicsObject( ALIENGRUNT_MAX_PHYSOBJ_MASS );
			m_flNextSwatScan = gpGlobals->curtime + 2.0;
		}
	}

	if ( m_hDoor != NULL )
	{
		m_hPhysicsEnt = m_hDoor;
		SetCondition( COND_ALIENGRUNT_CAN_ATTACK_DOOR );
		SetCondition( COND_ALIENGRUNT_CAN_SWAT_ATTACK );
	}
	else if ( m_hSetTarget != NULL )
	{
		m_hPhysicsEnt = m_hSetTarget;
		SetCondition( COND_ALIENGRUNT_CAN_SWAT_ATTACK );
	}
	else if(((m_hPhysicsEnt != NULL) && gpGlobals->curtime >= m_flNextSwat) && 
			( HasCondition( COND_SEE_ENEMY ) || FClassnameIs( m_hPhysicsEnt, "prop_door_rotating") ))
	{
		SetCondition( COND_ALIENGRUNT_CAN_SWAT_ATTACK );
	}
	else
	{
		ClearCondition( COND_ALIENGRUNT_CAN_SWAT_ATTACK );
	}

	bool bFound = false;

	if ( m_pSquad )
	{	
		AISquadIter_t iter;
		CAI_BaseNPC *pSquadmate = m_pSquad->GetFirstMember( &iter );

		while ( pSquadmate && !bFound)
		{
			if ( pSquadmate != this )
			{
				if ( pSquadmate->Classify() == CLASS_ANTLION && 
					 pSquadmate->IsCurSchedule( SCHED_ALIENGRUNT_CHARGE ) )
				{
					bFound = true;
				}

				if ( pSquadmate->Classify() == CLASS_VORTIGAUNT && 
					 pSquadmate->HasCondition( COND_CAN_RANGE_ATTACK1 ) )
				{
					bFound = true;
				}

				/*if ( pSquadmate->Classify() == CLASS_ALIENCONTROLLER &&
					 pSquadmate->HasCondition( COND_ALIENCONTROLLER_CAN_PHYS_ATTACK ))
				{
					bFound = true;
				}*/

				if ( pSquadmate->HasCondition( COND_CAN_MELEE_ATTACK1 ) || 
					 pSquadmate->HasCondition(COND_CAN_MELEE_ATTACK2 ) )
				{
					bFound = true; 
				}
			}

			pSquadmate = m_pSquad->GetNextMember( &iter );
		}
	}

	if (bFound)
	{
		SetCondition( COND_ALIENGRUNT_SHOULD_PROTECT_SQUADMATE );
	}
	else
	{
		ClearCondition( COND_ALIENGRUNT_SHOULD_PROTECT_SQUADMATE );
	}

	// See if we can charge the target
	if ( GetEnemy() )
	{
		if ( ShouldCharge( GetAbsOrigin(), GetEnemy()->GetAbsOrigin(), true, false ) )
		{
			SetCondition( COND_ALIENGRUNT_SHOULD_CHARGE );
		}
		else
		{
			ClearCondition( COND_ALIENGRUNT_SHOULD_CHARGE );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Calculate & apply damage & force for a charge to a target.
//			Done outside of the guard because we need to do this inside a trace filter.
//-----------------------------------------------------------------------------
void ApplyChargeDamageAlienGrunt( CBaseEntity *pAlienGrunt, CBaseEntity *pTarget, float flDamage )
{
	Vector attackDir = ( pTarget->WorldSpaceCenter() - pAlienGrunt->WorldSpaceCenter() );
	VectorNormalize( attackDir );
	Vector offset = RandomVector( -32, 32 ) + pTarget->WorldSpaceCenter();

	// Generate enough force to make a 75kg guy move away at 700 in/sec
	Vector vecForce = attackDir * ImpulseScale( 75, 700 );

	// Deal the damage
	CTakeDamageInfo	info( pAlienGrunt, pAlienGrunt, vecForce, offset, flDamage, DMG_CLUB );
	pTarget->TakeDamage( info );
}

//-----------------------------------------------------------------------------
// Purpose: While charging, look ahead and see if we're going to run into anything.
//			If we are, start the gesture so it looks like we're anticipating the hit.
//-----------------------------------------------------------------------------
void CNPC_AlienGrunt::ChargeLookAhead( void )
{
	trace_t	tr;
	Vector vecForward;
	GetVectors( &vecForward, NULL, NULL );
	Vector vecTestPos = GetAbsOrigin() + ( vecForward * m_flGroundSpeed * 0.75 );
	Vector testHullMins = GetHullMins();
	testHullMins.z += (StepHeight() * 2);
	TraceHull_SkipPhysics( GetAbsOrigin(), vecTestPos, testHullMins, GetHullMaxs(), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr, VPhysicsGetObject()->GetMass() * 0.5 );

	//NDebugOverlay::Box( tr.startpos, testHullMins, GetHullMaxs(), 0, 255, 0, true, 0.1f );
	//NDebugOverlay::Box( vecTestPos, testHullMins, GetHullMaxs(), 255, 0, 0, true, 0.1f );

	/*if ( tr.fraction != 1.0 )
	{
		// Start playing the hit animation
		AddGesture( ACT_ANTLIONGUARD_CHARGE_ANTICIPATION );
	}*/
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &origin - 
//			radius - 
//			magnitude - 
//-----------------------------------------------------------------------------
void CNPC_AlienGrunt::ImpactShock( const Vector &origin, float radius, float magnitude, CBaseEntity *pIgnored )
{
	// Also do a local physics explosion to push objects away
	float	adjustedDamage, flDist;
	Vector	vecSpot;
	float	falloff = 1.0f / 2.5f;

	CBaseEntity *pEntity = NULL;

	// Find anything within our radius
	while ( ( pEntity = gEntList.FindEntityInSphere( pEntity, origin, radius ) ) != NULL )
	{
		// Don't affect the ignored target
		if ( pEntity == pIgnored )
			continue;
		if ( pEntity == this )
			continue;

		CBasePropDoor *pDoor = dynamic_cast<CBasePropDoor*>(pEntity);
		if (pDoor)
		{
			//	if (pDoor->HasSpawnFlags(SF_BREAKABLE_BY_AGRUNTS) && (WorldSpaceCenter() - pDoor->WorldSpaceCenter()).Length() < radius )
			{
				//		// add some spin so the object doesn't appear to just fly in a straight line
				//		// Also this spin will move the object slightly as it will press on whatever the object
				//		// is resting on.
				//		AngularImpulse angVelocity( random->RandomFloat(-180, 180), 20, random->RandomFloat(-360, 360) );

				//		pDoor->BreakDoors(WorldSpaceCenter(), angVelocity);
				//		EmitSound("d1_trainstation_03.breakin_doorkick");
			}
		}
		
		// UNDONE: Ask the object if it should get force if it's not MOVETYPE_VPHYSICS?
		else if ( pEntity->GetMoveType() == MOVETYPE_VPHYSICS || ( pEntity->VPhysicsGetObject() && pEntity->IsPlayer() == false ) )
		{
			vecSpot = pEntity->BodyTarget( WorldSpaceCenter() );
			
			// decrease damage for an ent that's farther from the bomb.
			flDist = ( WorldSpaceCenter() - vecSpot ).Length();

			if ( flDist <= radius ) //radius == 0 || flDist <= radius )
			{
				/*adjustedDamage = flDist * falloff;
				adjustedDamage = magnitude - adjustedDamage;
		
				if ( adjustedDamage < 1 )
				{
					adjustedDamage = 1;
				}*/

				//DevMsg("dist %f, rad %f, ", flDist, radius );
				adjustedDamage = radius - flDist;
				//DevMsg("rad - dist %f, ", adjustedDamage);
				adjustedDamage *= falloff;
				//DevMsg("times fallof %f, ", adjustedDamage );
		
				adjustedDamage = clamp(adjustedDamage, 1, magnitude );

				//DevMsg("%s, damage/magnitude %f/%f, dist/radius %f/%f\n", pEntity->GetDebugName(), adjustedDamage, magnitude, flDist, radius );


				CTakeDamageInfo info( this, this, adjustedDamage, DMG_BLAST );

				Vector vecDir = (vecSpot - GetAbsOrigin());

				CalculateExplosiveDamageForce( &info, vecDir, GetAbsOrigin() );

				pEntity->VPhysicsTakeDamage( info );
			}
		}
	}

	//TERO: lets add some more time
	m_flChargeCancelTime = m_flChargeCancelTime + 1.0f;
}

//-----------------------------------------------------------------------------
// Purpose: Return true if our charge target is right in front of the guard
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_AlienGrunt::EnemyIsRightInFrontOfMe( CBaseEntity **pEntity )
{
	if ( !GetEnemy() )
		return false;

	if ( (GetEnemy()->WorldSpaceCenter() - WorldSpaceCenter()).LengthSqr() < (156*156) )
	{
		Vector vecLOS = ( GetEnemy()->GetAbsOrigin() - GetAbsOrigin() );
		vecLOS.z = 0;
		VectorNormalize( vecLOS );
		Vector vBodyDir = BodyDirection2D();
		if ( DotProduct( vecLOS, vBodyDir ) > 0.8 )
		{
			// He's in front of me, and close. Make sure he's not behind a wall.
			trace_t tr;
			UTIL_TraceLine( WorldSpaceCenter(), GetEnemy()->EyePosition(), MASK_SOLID, this, COLLISION_GROUP_NONE, &tr );
			if ( tr.m_pEnt == GetEnemy() )
			{
				*pEntity = tr.m_pEnt;
				return true;
			}
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pTarget - 
//-----------------------------------------------------------------------------
void CNPC_AlienGrunt::ChargeDamage( CBaseEntity *pTarget )
{
	if ( pTarget == NULL )
		return;

	CBasePlayer *pPlayer = ToBasePlayer( pTarget );

	if ( pPlayer != NULL )
	{
		//Kick the player angles
		pPlayer->ViewPunch( QAngle( 20, 20, -30 ) );	

		Vector	dir = pPlayer->WorldSpaceCenter() - WorldSpaceCenter();
		VectorNormalize( dir );
		dir.z = 0.0f;
		
		Vector vecNewVelocity = dir * 180.0f;
		vecNewVelocity[2] += 128.0f;
		pPlayer->SetAbsVelocity( vecNewVelocity );

		color32 red = {128,0,0,128};
		UTIL_ScreenFade( pPlayer, red, 1.0f, 0.1f, FFADE_IN );
	}
	
	// Player takes less damage
	float flDamage = (pTarget->Classify() == CLASS_PLAYER_ALLY_VITAL) ? (sk_aliengrunt_dmg_claw.GetFloat() * 0.75f) : (sk_aliengrunt_dmg_claw.GetFloat());
	
	// If it's being held by the player, break that bond
	Pickup_ForcePlayerToDropThisObject( pTarget );

	// Calculate the physics force
	ApplyChargeDamageAlienGrunt( this, pTarget, flDamage );
}

//-----------------------------------------------------------------------------
// Purpose: Handles the guard charging into something. Returns true if it hit the world.
//-----------------------------------------------------------------------------
bool CNPC_AlienGrunt::HandleChargeImpact( Vector vecImpact, CBaseEntity *pEntity )
{
	// Cause a shock wave from this point which will disrupt nearby physics objects
	ImpactShock( vecImpact, 100, 30 );	//AntlionGuard has magnitude of 350

	// Did we hit anything interesting?
	if ( !pEntity || pEntity->IsWorld() )
	{
		// Robin: Due to some of the finicky details in the motor, the guard will hit
		//		  the world when it is blocked by our enemy when trying to step up 
		//		  during a moveprobe. To get around this, we see if the enemy's within
		//		  a volume in front of the guard when we hit the world, and if he is,
		//		  we hit him anyway.
		EnemyIsRightInFrontOfMe( &pEntity );

		// Did we manage to find him? If not, increment our charge miss count and abort.
		if ( pEntity->IsWorld() )
		{
			//m_iChargeMisses++;
			return true;
		}
	}

	// Hit anything we don't like
	if ( IRelationType( pEntity ) == D_HT && ( GetNextAttack() < gpGlobals->curtime ) )
	{
		EmitSound( "NPC_Hunter.ChargeHitEnemy" );
		EmitSound( "NPC_AlienGrunt.Pain", 0.5f );

		if ( GetActivity() != ACT_ALIENGRUNT_CHARGE_HIT )	//!IsPlayingGesture( ACT_ALIENGRUNT )
		{
			SetActivity( ACT_ALIENGRUNT_CHARGE_HIT );
		}
		
		ChargeDamage( pEntity );
		
		pEntity->ApplyAbsVelocityImpulse( ( BodyDirection2D() * 400 ) + Vector( 0, 0, 10 ) );

		if ( !pEntity->IsAlive() && GetEnemy() == pEntity )
		{
			SetEnemy( NULL );
		}

		SetNextAttack( gpGlobals->curtime + 2.0f );
		//SetActivity( ACT_ALIENGRUNT_CHARGE_CANCEL );

		// We've hit something, so clear our miss count
//		m_iChargeMisses = 0;
		return false;
	}

	// Hit something we don't hate. If it's not moveable, crash into it.
	if ( pEntity->GetMoveType() == MOVETYPE_NONE || pEntity->GetMoveType() == MOVETYPE_PUSH )
		return true;

	// If it's a vphysics object that's too heavy, crash into it too.
	if ( pEntity->GetMoveType() == MOVETYPE_VPHYSICS )
	{
		IPhysicsObject *pPhysics = pEntity->VPhysicsGetObject();
		if ( pPhysics )
		{
			// If the object is being held by the player, knock it out of his hands
			if ( pPhysics->GetGameFlags() & FVPHYSICS_PLAYER_HELD )
			{
				Pickup_ForcePlayerToDropThisObject( pEntity );
				return false;
			}

			if ( (!pPhysics->IsMoveable() || pPhysics->GetMass() > VPhysicsGetObject()->GetMass() * 0.5f ) )
				return true;
		}
	}

	return false;

}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
float CNPC_AlienGrunt::ChargeSteer( void )
{
	trace_t	tr;
	Vector	testPos, steer, forward, right;
	QAngle	angles;
	const float	testLength = m_flGroundSpeed * 0.15f;

	//Get our facing
	GetVectors( &forward, &right, NULL );

	steer = forward;

	const float faceYaw	= UTIL_VecToYaw( forward );

	//Offset right
	VectorAngles( forward, angles );
	angles[YAW] += 45.0f;
	AngleVectors( angles, &forward );

	//Probe out
	testPos = GetAbsOrigin() + ( forward * testLength );

	//Offset by step height
	Vector testHullMins = GetHullMins();
	testHullMins.z += (StepHeight() * 2);

	//Probe
	TraceHull_SkipPhysics( GetAbsOrigin(), testPos, testHullMins, GetHullMaxs(), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr, VPhysicsGetObject()->GetMass() * 0.5f );

	//Debug info
	/*if ( g_debug_antlionguard.GetInt() == 1 )
	{
		if ( tr.fraction == 1.0f )
		{
  			NDebugOverlay::BoxDirection( GetAbsOrigin(), testHullMins, GetHullMaxs() + Vector(testLength,0,0), forward, 0, 255, 0, 8, 0.1f );
   		}
   		else
   		{
  			NDebugOverlay::BoxDirection( GetAbsOrigin(), testHullMins, GetHullMaxs() + Vector(testLength,0,0), forward, 255, 0, 0, 8, 0.1f );
		}
	}*/

	//Add in this component
	steer += ( right * 0.5f ) * ( 1.0f - tr.fraction );

	//Offset left
	angles[YAW] -= 90.0f;
	AngleVectors( angles, &forward );

	//Probe out
	testPos = GetAbsOrigin() + ( forward * testLength );

	// Probe
	TraceHull_SkipPhysics( GetAbsOrigin(), testPos, testHullMins, GetHullMaxs(), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr, VPhysicsGetObject()->GetMass() * 0.5f );

	//Debug
/*	if ( g_debug_antlionguard.GetInt() == 1 )
	{
		if ( tr.fraction == 1.0f )
		{
			NDebugOverlay::BoxDirection( GetAbsOrigin(), testHullMins, GetHullMaxs() + Vector(testLength,0,0), forward, 0, 255, 0, 8, 0.1f );
		}
		else
		{
			NDebugOverlay::BoxDirection( GetAbsOrigin(), testHullMins, GetHullMaxs() + Vector(testLength,0,0), forward, 255, 0, 0, 8, 0.1f );
		}
	}*/

	//Add in this component
	steer -= ( right * 0.5f ) * ( 1.0f - tr.fraction );

	//Debug
	/*if ( g_debug_antlionguard.GetInt() == 1 )
	{
		NDebugOverlay::Line( GetAbsOrigin(), GetAbsOrigin() + ( steer * 512.0f ), 255, 255, 0, true, 0.1f );
		NDebugOverlay::Cross3D( GetAbsOrigin() + ( steer * 512.0f ), Vector(2,2,2), -Vector(2,2,2), 255, 255, 0, true, 0.1f );

		NDebugOverlay::Line( GetAbsOrigin(), GetAbsOrigin() + ( BodyDirection3D() * 256.0f ), 255, 0, 255, true, 0.1f );
		NDebugOverlay::Cross3D( GetAbsOrigin() + ( BodyDirection3D() * 256.0f ), Vector(2,2,2), -Vector(2,2,2), 255, 0, 255, true, 0.1f );
	}*/

	return UTIL_AngleDiff( UTIL_VecToYaw( steer ), faceYaw );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_AlienGrunt::ShouldCharge( const Vector &startPos, const Vector &endPos, bool useTime, bool bCheckForCancel )
{
	// Must have a target
	if ( !GetEnemy() )
		return false;

	// No one else in the squad can be charging already
	if ( IsStrategySlotRangeOccupied( SQUAD_SLOT_ALIENGRUNT_CHARGE, SQUAD_SLOT_ALIENGRUNT_CHARGE ) )
		return false;

	// Don't check the distance once we start charging
	if ( !bCheckForCancel )
	{
		//TERO: IsOnFire added by me
		if (useTime && !IsOnFire())
		{
			float flTime = RemapValClamped( (float)m_iHealth, 20, m_iMaxHealth, aliengrunt_charge_delay_min.GetFloat(), aliengrunt_charge_delay_max.GetFloat());
			//DevMsg("Charge time, health left %d/%d, next charge in %f\n", m_iHealth, m_iMaxHealth, flTime);

			// Don't allow use to charge again if it's been too soon
			if ( (m_flNextChargeTime + flTime) > gpGlobals->curtime )
				return false;
		}

		float distance = UTIL_DistApprox2D( startPos, endPos );

		// Must be within our tolerance range
		if ( ( distance < ALIENGRUNT_CHARGE_MIN ) || ( distance > ALIENGRUNT_CHARGE_MAX ) )
			return false;
	}

	if ( GetSquad() )
	{
		// If someone in our squad is closer to the enemy, then don't charge (we end up hitting them more often than not!)
		float flOurDistToEnemySqr = ( GetAbsOrigin() - GetEnemy()->GetAbsOrigin() ).LengthSqr();
		AISquadIter_t iter;
		for ( CAI_BaseNPC *pSquadMember = GetSquad()->GetFirstMember( &iter ); pSquadMember; pSquadMember = GetSquad()->GetNextMember( &iter ) )
		{
			if ( pSquadMember->IsAlive() == false || pSquadMember == this )
				continue;

			if ( ( pSquadMember->GetAbsOrigin() - GetEnemy()->GetAbsOrigin() ).LengthSqr() < flOurDistToEnemySqr )
				return false;
		}
	}

	//FIXME: We'd like to exclude small physics objects from this check!

	// We only need to hit the endpos with the edge of our bounding box
	Vector vecDir = endPos - startPos;
	VectorNormalize( vecDir );
	float flWidth = WorldAlignSize().x * 0.5;
	Vector vecTargetPos = endPos - (vecDir * flWidth);

	// See if we can directly move there
	AIMoveTrace_t moveTrace;
	GetMoveProbe()->MoveLimit( NAV_GROUND, startPos, vecTargetPos, MASK_NPCSOLID_BRUSHONLY, GetEnemy(), &moveTrace );
	
	// Draw the probe
	/*if ( g_debug_antlionguard.GetInt() == 1 )
	{
		Vector	enemyDir	= (vecTargetPos - startPos);
		float	enemyDist	= VectorNormalize( enemyDir );

		NDebugOverlay::BoxDirection( startPos, GetHullMins(), GetHullMaxs() + Vector(enemyDist,0,0), enemyDir, 0, 255, 0, 8, 1.0f );
	}*/

	// If we're not blocked, charge
	if ( IsMoveBlocked( moveTrace ) )
	{
		// Don't allow it if it's too close to us
		if ( UTIL_DistApprox( WorldSpaceCenter(), moveTrace.vEndPosition ) < ALIENGRUNT_CHARGE_MIN )
			return false;

		// Allow some special cases to not block us
		if ( moveTrace.pObstruction != NULL )
		{
			// If we've hit the world, see if it's a cliff
			if ( moveTrace.pObstruction == GetContainingEntity( INDEXENT(0) ) )
			{	
				// Can't be too far above/below the target
				if ( fabs( moveTrace.vEndPosition.z - vecTargetPos.z ) > StepHeight() )
					return false;

				// Allow it if we got pretty close
				if ( UTIL_DistApprox( moveTrace.vEndPosition, vecTargetPos ) < 64 )
					return true;
			}

			// Hit things that will take damage
			if ( moveTrace.pObstruction->m_takedamage != DAMAGE_NO )
				return true;

			// Hit things that will move
			if ( moveTrace.pObstruction->GetMoveType() == MOVETYPE_VPHYSICS )
				return true;
		}

		return false;
	}

	// Only update this if we've requested it
	if ( useTime )
	{
	

		m_flNextChargeTime = gpGlobals->curtime; // + flTime;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : flInterval - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_AlienGrunt::OverrideMove( float flInterval )
{
	// If the guard's charging, we're handling the movement
	if ( IsCurSchedule( SCHED_ALIENGRUNT_CHARGE ) )
		return true;

	return BaseClass::OverrideMove( flInterval );
}

//-----------------------------------------------------------------------------
// Purpose: turn in the direction of movement
// Output :
//-----------------------------------------------------------------------------
bool CNPC_AlienGrunt::OverrideMoveFacing( const AILocalMoveGoal_t &move, float flInterval )
{

	//TERO: we have to check how far away the enemy is, and if we have object to smash
	if ( m_hPhysicsEnt && (m_hPhysicsEnt->WorldSpaceCenter() - WorldSpaceCenter()).Length() < ALIENGRUNT_FACE_PHYSICS_ENT_DISTANCE && 
		(IsCurSchedule( SCHED_ALIENGRUNT_MOVE_SWATITEM, false ) || 
		 IsCurSchedule( SCHED_ALIENGRUNT_SWATITEM, false ) ||
		 IsCurSchedule( SCHED_ALIENGRUNT_ATTACKITEM, false ) ))
	{
		AddFacingTarget( m_hPhysicsEnt, m_hPhysicsEnt->WorldSpaceCenter(), 3.0f, 0.2f );
		return BaseClass::OverrideMoveFacing( move, flInterval );
	}

	if ( GetEnemy() && (GetEnemy()->WorldSpaceCenter() - WorldSpaceCenter()).Length() < ALIENGRUNT_FACE_ENEMY_DISTANCE )
	{
		AddFacingTarget( GetEnemy(), GetEnemy()->WorldSpaceCenter(), 1.0f, 0.2f );
		return BaseClass::OverrideMoveFacing( move, flInterval );
	}

	return BaseClass::OverrideMoveFacing( move, flInterval );
}

//-----------------------------------------------------------------------------
// Continuous movement tasks
//-----------------------------------------------------------------------------
bool CNPC_AlienGrunt::IsCurTaskContinuousMove()
{
	const Task_t* pTask = GetTask();
	if ( pTask && (pTask->iTask == TASK_ALIENGRUNT_CHASE_ENEMY_CONTINUOUSLY) )
		return true;

	return BaseClass::IsCurTaskContinuousMove();
}

//-----------------------------------------------------------------------------
// Chase the enemy, updating the target position as the player moves
//-----------------------------------------------------------------------------
void CNPC_AlienGrunt::StartTaskChaseEnemyContinuously( const Task_t *pTask )
{
	//DevMsg("StartTaskChaseEnemyContinuously( pTask );\n");

	CBaseEntity *pEnemy = GetEnemy();
	if ( !pEnemy )
	{
		TaskFail( FAIL_NO_ENEMY );
		return;
	}

	// We're done once we get close enough
	/*if ( WorldSpaceCenter().DistToSqr( pEnemy->WorldSpaceCenter() ) <= pTask->flTaskData * pTask->flTaskData )
	{
		TaskComplete();
		return;
	}*/

	// TASK_GET_PATH_TO_ENEMY
	if ( IsUnreachable( pEnemy ) )
	{
		TaskFail(FAIL_NO_ROUTE);
		return;
	}

	if ( !GetNavigator()->SetGoal( GOALTYPE_ENEMY, AIN_NO_PATH_TASK_FAIL ) )
	{
		// no way to get there =( 
		DevWarning( 2, "GetPathToEnemy failed!!\n" );
		RememberUnreachable( pEnemy );
		TaskFail(FAIL_NO_ROUTE);
		return;
	}

	// NOTE: This is TaskRunPath here.
	if ( TranslateActivity( ACT_RUN ) != ACT_INVALID )
	{
		GetNavigator()->SetMovementActivity( ACT_RUN );
	}
	else
	{
		GetNavigator()->SetMovementActivity(ACT_WALK);
	}

	// Cover is void once I move
	Forget( bits_MEMORY_INCOVER );

	if (GetNavigator()->GetGoalType() == GOALTYPE_NONE)
	{
		TaskComplete();
		GetNavigator()->ClearGoal();		// Clear residual state
		return;
	}

	if (!GetNavigator()->IsGoalActive())
	{
		SetIdealActivity( GetStoppedActivity() );
	}
	else
	{
		// Check validity of goal type
		ValidateNavGoal();
	}

	// set that we're probably going to stop before the goal
	GetNavigator()->SetArrivalDistance( 64.0f ); //pTask->flTaskData );
	m_vSavePosition = GetEnemy()->WorldSpaceCenter();
}

void CNPC_AlienGrunt::RunTaskChaseEnemyContinuously( const Task_t *pTask )
{
	//DevMsg("RunTaskChaseEnemyContinuously( pTask );\n");

	if (!GetNavigator()->IsGoalActive())
	{
		SetIdealActivity( GetStoppedActivity() );
	}
	else
	{
		// Check validity of goal type
		ValidateNavGoal();
	}

	CBaseEntity *pEnemy = GetEnemy();
	if ( !pEnemy )
	{
		TaskFail( FAIL_NO_ENEMY );
		return;
	}

	// We're done once we get close enough
	/*if ( WorldSpaceCenter().DistToSqr( pEnemy->WorldSpaceCenter() ) <= pTask->flTaskData * pTask->flTaskData )
	{
		GetNavigator()->StopMoving();
		TaskComplete();
		return;
	}*/

	// Recompute path if the enemy has moved too much
	if ( m_vSavePosition.DistToSqr( pEnemy->WorldSpaceCenter() ) < (pTask->flTaskData * pTask->flTaskData) )
		return;

	if ( IsUnreachable( pEnemy ) )
	{
		TaskFail(FAIL_NO_ROUTE);
		return;
	}

	if ( !GetNavigator()->RefindPathToGoal() )
	{
		TaskFail(FAIL_NO_ROUTE);
		return;
	}

	m_vSavePosition = pEnemy->WorldSpaceCenter();
}


//-----------------------------------------------------------------------------
// Purpose: A simple trace filter class to skip small moveable physics objects
//-----------------------------------------------------------------------------
class CTraceFilterSkipPhysics : public CTraceFilter
{
public:
	// It does have a base, but we'll never network anything below here..
	DECLARE_CLASS_NOBASE( CTraceFilterSkipPhysics );
	
	CTraceFilterSkipPhysics( const IHandleEntity *passentity, int collisionGroup, float minMass )
		: m_pPassEnt(passentity), m_collisionGroup(collisionGroup), m_minMass(minMass)
	{
	}
	virtual bool ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask )
	{
		if ( !StandardFilterRules( pHandleEntity, contentsMask ) )
			return false;

		if ( !PassServerEntityFilter( pHandleEntity, m_pPassEnt ) )
			return false;

		// Don't test if the game code tells us we should ignore this collision...
		CBaseEntity *pEntity = EntityFromEntityHandle( pHandleEntity );
		if ( pEntity )
		{
			if ( !pEntity->ShouldCollide( m_collisionGroup, contentsMask ) )
				return false;
			
			if ( !g_pGameRules->ShouldCollide( m_collisionGroup, pEntity->GetCollisionGroup() ) )
				return false;

			// don't test small moveable physics objects (unless it's an NPC)
			if ( !pEntity->IsNPC() && pEntity->GetMoveType() == MOVETYPE_VPHYSICS )
			{
				IPhysicsObject *pPhysics = pEntity->VPhysicsGetObject();
				Assert(pPhysics);
				if ( pPhysics->IsMoveable() && pPhysics->GetMass() < m_minMass )
					return false;
			}

			// If we hit an antlion, don't stop, but kill it
			if ( pEntity->Classify() == CLASS_CITIZEN_REBEL )
			{
				CBaseEntity *pGuard = (CBaseEntity*)EntityFromEntityHandle( m_pPassEnt );
				ApplyChargeDamageAlienGrunt( pGuard, pEntity, pEntity->GetHealth() );
				return false;
			}
		}

		return true;
	}



private:
	const IHandleEntity *m_pPassEnt;
	int m_collisionGroup;
	float m_minMass;
};

inline void TraceHull_SkipPhysics( const Vector &vecAbsStart, const Vector &vecAbsEnd, const Vector &hullMin, 
					 const Vector &hullMax,	unsigned int mask, const CBaseEntity *ignore, 
					 int collisionGroup, trace_t *ptr, float minMass )
{
	Ray_t ray;
	ray.Init( vecAbsStart, vecAbsEnd, hullMin, hullMax );
	CTraceFilterSkipPhysics traceFilter( ignore, collisionGroup, minMass );
	enginetrace->TraceRay( ray, mask, &traceFilter, ptr );
}

//------------------------------------------------------------------------------
//
// Schedules
//
//------------------------------------------------------------------------------

AI_BEGIN_CUSTOM_NPC( monster_aliengrunt, CNPC_AlienGrunt )

	DECLARE_USES_SCHEDULE_PROVIDER( CAI_AssaultBehavior )

	DECLARE_TASK( TASK_ALIENGRUNT_SWAT_ITEM )
	DECLARE_TASK( TASK_ALIENGRUNT_GET_PATH_TO_PHYSOBJ )
	DECLARE_TASK( TASK_ALIENGRUNT_SHOOT_BEES )

	DECLARE_TASK( TASK_ALIENGRUNT_CHARGE )
	DECLARE_TASK( TASK_ALIENGRUNT_CHASE_ENEMY_CONTINUOUSLY )

	DECLARE_SQUADSLOT( SQUAD_SLOT_ALIENGRUNT_CHARGE )

	DECLARE_ACTIVITY( ACT_ALIENGRUNT_TO_ACTION )
	DECLARE_ACTIVITY( ACT_ALIENGRUNT_TO_IDLE)

	DECLARE_ACTIVITY( ACT_ALIENGRUNT_CHARGE_LOOP )
	DECLARE_ACTIVITY( ACT_ALIENGRUNT_CHARGE_HIT )
	DECLARE_ACTIVITY( ACT_ALIENGRUNT_CHARGE_CANCEL )
	DECLARE_ACTIVITY( ACT_ALIENGRUNT_CHARGE_START )

	DECLARE_ACTIVITY( ACT_ALIENGRUNT_GRAB_RAGDOLL )

	DECLARE_ANIMEVENT( AE_ALIENGRUNT_GRAB_RAGDOLL )
	DECLARE_ANIMEVENT( AE_ALIENGRUNT_THROW_RAGDOLL )

	DECLARE_ANIMEVENT( AE_ALIENGRUNT_HAND_LEFT )
	DECLARE_ANIMEVENT( AE_ALIENGRUNT_HAND_RIGHT )
	DECLARE_ANIMEVENT( AE_ALIENGRUNT_SHOOTBEES )
	DECLARE_ANIMEVENT( AE_ALIENGRUNT_SWING_SOUND )
	DECLARE_ANIMEVENT( AE_ALIENGRUNT_CHARGE_HIT )

#ifdef ALIENGRUNT_CUSTOM_FOOT_AE
	DECLARE_ANIMEVENT( AE_ALIENGRUNT_FOOT_LEFT )
	DECLARE_ANIMEVENT( AE_ALIENGRUNT_FOOT_RIGHT )
#endif

	DECLARE_CONDITION( COND_ALIENGRUNT_CAN_SWAT_ATTACK )
	DECLARE_CONDITION( COND_ALIENGRUNT_LOCAL_MELEE_OBSTRUCTION )
	DECLARE_CONDITION( COND_ALIENGRUNT_CAN_GRAB_RAGDOLL )
	DECLARE_CONDITION( COND_ALIENGRUNT_CANNOT_GRAB_RAGDOLL )
	DECLARE_CONDITION( COND_ALIENGRUNT_SHOULD_CHARGE )

	DECLARE_CONDITION( COND_ALIENGRUNT_CAN_ATTACK_DOOR )
	DECLARE_CONDITION( COND_ALIENGRUNT_BEES )
	DECLARE_CONDITION( COND_ALIENGRUNT_SHOULD_PROTECT_SQUADMATE )

	//=========================================================
	// > SCHED_ALIENGRUNT_RANGE_ATTACK
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_ALIENGRUNT_SHOOTBEES,

		"	Tasks"
		"		TASK_STOP_MOVING				0" // commented out so that we can choose our anim
		"		TASK_FACE_IDEAL					0"	//TASK_ADD_GESTURE_WAIT
		"		TASK_RANGE_ATTACK1				0" //TASK_RANGE_ATTACK1 TASK_ALIENGRUNT_SHOOT_BEES
		"		TASK_RANGE_ATTACK1				0"
		"		TASK_RANGE_ATTACK1 				0"
	//	"		TASK_SET_SCHEDULE				SCHEDULE:SCHED_CHASE_ENEMY"
		//"		TASK_WAIT						0.1" // Wait a sec before firing again
		""
		"	Interrupts"
		"		COND_CAN_MELEE_ATTACK1"
		"		COND_ALIENGRUNT_CAN_GRAB_RAGDOLL"
		"		COND_ALIENGRUNT_SHOULD_CHARGE"
		"		COND_HEAVY_DAMAGE"
	);

	//=========================================================
	// > SCHED_ALIENGRUNT_MELEE_ATTACK
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_ALIENGRUNT_MELEE_ATTACK,

		"	Tasks"
		"		TASK_STOP_MOVING					0"
		"		TASK_SET_ACTIVITY					ACTIVITY:ACT_IDLE"
		"		TASK_FACE_ENEMY						0"
		"		TASK_MELEE_ATTACK1					ACTIVITY:ACT_MELEE_ATTACK1"
		""
		"	Interrupts"
		"		COND_ENEMY_DEAD"
		"		COND_ENEMY_OCCLUDED"
		"		COND_ALIENGRUNT_CAN_GRAB_RAGDOLL"
//		"		COND_ALIENGRUNT_SHOULD_CHARGE"		//TERO: I am not sure about this, if he's close enough to the player, no need to charge
	);

	//=========================================================
	// > SCHED_ALIENGRUNT_STAND
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_ALIENGRUNT_STAND,

		"	Tasks"
		"		TASK_STOP_MOVING					0"
		"		TASK_SET_ACTIVITY					ACTIVITY:ACT_IDLE"
		"		TASK_WAIT							2"					// repick IDLESTAND every two seconds."
		""
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
		"		COND_SMELL"
		"		COND_PROVOKED"
		"		COND_HEAR_COMBAT"
		"		COND_HEAR_DANGER"
		"		COND_ALIENGRUNT_SHOULD_CHARGE"
		"		COND_ALIENGRUNT_CAN_SWAT_ATTACK"
		"		COND_ALIENGRUNT_CAN_GRAB_RAGDOLL"
		"		COND_ALIENGRUNT_CAN_ATTACK_DOOR"
	);

	//=========================================================
	// > SCHED_ALIENGRUNT_MOVE_SWATITEM
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_ALIENGRUNT_MOVE_SWATITEM,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_CHASE_ENEMY"
		"		TASK_ALIENGRUNT_GET_PATH_TO_PHYSOBJ	0"
		"		TASK_RUN_PATH						0"
		"		TASK_WAIT_FOR_MOVEMENT				0"
		"		TASK_FACE_ENEMY						0"
		"		TASK_ALIENGRUNT_SWAT_ITEM			0"
		""
		"	Interrupts"
		"		COND_ENEMY_DEAD"
		"		COND_NEW_ENEMY"
	)

	//=========================================================
	// SwatItem
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_ALIENGRUNT_SWATITEM,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_CHASE_ENEMY"
		"		TASK_FACE_ENEMY					0"
		"		TASK_ALIENGRUNT_SWAT_ITEM		0"
		""
		"	Interrupts"
		"		COND_ENEMY_DEAD"
		"		COND_NEW_ENEMY"
	);

	//=========================================================
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_ALIENGRUNT_ATTACKITEM,

		"	Tasks"
		"		TASK_FACE_ENEMY					0"
		"		TASK_MELEE_ATTACK1				0"
		""
		"	Interrupts"
		"		COND_ENEMY_DEAD"
		"		COND_NEW_ENEMY"
	);

	//=========================================================
	// > SCHED_ALIENGRUNT_GRAB_RAGDOLL
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_ALIENGRUNT_GRAB_RAGDOLL,

		"	Tasks"
		"		TASK_STOP_MOVING					0"
		"		TASK_SET_ACTIVITY					ACTIVITY:ACT_IDLE"
		"		TASK_FACE_ENEMY						0"
		"		TASK_MELEE_ATTACK2					ACTIVITY:ACT_ALIENGRUNT_GRAB_RAGDOLL"
		""
		"	Interrupts"
		"		COND_ALIENGRUNT_CANNOT_GRAB_RAGDOLL"
	);

	//=========================================================
	// SCHED_COMBINE_PRESS_ATTACK
	//=========================================================
	DEFINE_SCHEDULE 
	(
		SCHED_ALIENGRUNT_PRESS_ATTACK,

		"	Tasks "
		"		TASK_STOP_MOVING					0"
		"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_CHASE_ENEMY_FAILED"
		"		TASK_ALIENGRUNT_CHASE_ENEMY_CONTINUOUSLY		192"
		"		TASK_FACE_ENEMY						0"
		""
		"	Interrupts "
		"		COND_NEW_ENEMY"
		"		COND_ENEMY_DEAD"
		"		COND_ENEMY_UNREACHABLE"
		"		COND_TOO_CLOSE_TO_ATTACK"
		"		COND_CAN_MELEE_ATTACK1"
		"		COND_CAN_RANGE_ATTACK1"
		"		COND_CAN_MELEE_ATTACK2"
		"		COND_ALIENGRUNT_CAN_GRAB_RAGDOLL"
		"		COND_ALIENGRUNT_SHOULD_CHARGE"
		"		COND_ALIENGRUNT_CAN_ATTACK_DOOR"
		"		COND_ALIENGRUNT_SHOULD_PROTECT_SQUADMATE"
	);

	//==================================================
	// SCHED_ALIENGRUNT_CHARGE
	//==================================================

	DEFINE_SCHEDULE
	(
		SCHED_ALIENGRUNT_CHARGE,

		"	Tasks"
		"		TASK_STOP_MOVING					0"
		"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_ALIENGRUNT_PRESS_ATTACK"
		"		TASK_FACE_ENEMY						0"
		"		TASK_ALIENGRUNT_CHARGE				0"
		""
		"	Interrupts"
		"		COND_TASK_FAILED"
		"		COND_HEAVY_DAMAGE"
	);

AI_END_CUSTOM_NPC()