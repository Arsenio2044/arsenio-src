//===================  =====================//
//
// Purpose:
//			
//
//=============================================================================//


#ifndef NPC_ALIENGRUNT
#define NPC_ALIENGRUNT

#include "cbase.h"
#include "ai_basenpc.h"
#include "ai_basenpc.h"
#include "ai_behavior.h"
#include "ai_behavior_lead.h"
#include "ai_behavior_standoff.h"
#include "ai_behavior_assault.h"
#include "npc_talker.h"

#include "physics_bone_follower.h"
#include "physics_prop_ragdoll.h"

#include "vphysics/player_controller.h"
#include "vphysics_interface.h"
#include "vphysics/constraints.h"
#include "physics_saverestore.h"


extern int AE_AGRUNT_ATTACK_RIGHT;
extern int AE_AGRUNT_ATTACK_LEFT;
extern int AE_AGRUNT_STEP_LEFT;
extern int AE_AGRUNT_STEP_RIGHT;
extern int AE_AGRUNT_ATTACK_SCREAM;
extern int AE_AGRUNT_POUND;



#define ALIENGRUNT_MAX_PHYSOBJ_MASS				200
#define ALIENGRUNT_MIN_PHYSOBJ_MASS				 30
#define ALIENGRUNT_PLAYER_MAX_SWAT_DIST				1000
#define ALIENGRUNT_PHYSOBJ_SWATDIST				80
#define ALIENGRUNT_PHYSOBJ_MOVE_TO_DIST			48
#define ALIENGRUNT_SWAT_DELAY					2
#define ALIENGRUNT_FARTHEST_PHYSICS_OBJECT		40.0*12.0
#define ALIENGRUNT_PHYSICS_SEARCH_DEPTH			100


//TERO: undefine this to disable ragdoll grabbing completely
#define ALIENGRUNT_GRAB_RAGDOLL

#ifdef ALIENGRUNT_GRAB_RAGDOLL
#define SF_ALIENGRUNT_CAN_GRAB_RAGDOLL	(1 << 16)
#define SF_ALIENGRUNT_WAIT_FOR_GRAB		(1 << 17)
#define ALIENGRUNT_GRAB_RANGE 100
#endif

extern ConVar sk_aliengrunt_dmg_claw;


//=========================================================
//=========================================================
class CNPC_AlienGrunt : public CAI_PlayerAlly
{
	DECLARE_CLASS( CNPC_AlienGrunt, CAI_PlayerAlly );

public:

	void			Spawn( void );
	void			Precache( void );
	int				GetSoundInterests( void );
	Class_T			Classify( void );

	void			Claw( int iAttachment );
	void			SwatItem();
	void			UpdateOnRemove();

	int				RangeAttack1Conditions( float flDot, float flDist );
	int				MeleeAttack1Conditions( float flDot, float flDist );

	//Vector		GetShootEnemyDir( const Vector &shootOrigin, bool bNoisy = true );
	
#if 1
	void			TraceAttack(const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator);
#else
	void			TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr );
#endif
	int				OnTakeDamage_Alive( const CTakeDamageInfo &info );
	virtual float	GetReactionDelay( CBaseEntity *pEnemy ) { return 0.0; }

	virtual int		SelectSchedule ( void );
	virtual int		SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode );

	void			BuildScheduleTestBits( void );

	virtual int		TranslateSchedule( int scheduleType );
	virtual	Activity NPC_TranslateActivity( Activity baseAct );

	Activity		GetFlinchActivity( bool bHeavyDamage, bool bGesture );
	void			PlayFlinchGesture( void );

	bool			IsJumpLegal( const Vector &startPos, const Vector &apex, const Vector &endPos ) const;

	//bool			IsValidEnemy( CBaseEntity *pEnemy );
//	bool			IsLeading( void ) { return ( GetRunningBehavior() == &m_LeadBehavior && m_LeadBehavior.HasGoal() ); }


	virtual void	UpdateEfficiency( bool bInPVS )	{ SetEfficiency( ( GetSleepState() != AISS_AWAKE ) ? AIE_DORMANT : AIE_NORMAL ); SetMoveEfficiency( AIME_NORMAL ); }
//	virtual void		GatherEnemyConditions( CBaseEntity *pEnemy );

	void			StartTask( const Task_t *pTask );
	void			RunTask( const Task_t *pTask );

	// Chase the enemy, updating the target position as the player moves
	void StartTaskChaseEnemyContinuously( const Task_t *pTask );
	void RunTaskChaseEnemyContinuously( const Task_t *pTask );

	float			MaxYawSpeed( void );

	bool			FVisible( CBaseEntity *pEntity, int traceMask = MASK_OPAQUE, CBaseEntity **ppBlocker = NULL );

	bool			ShouldGoToIdleState( void );

	void			HandleAnimEvent( animevent_t *pEvent );

	void			DeathSound( const CTakeDamageInfo &info ) ;
	void			PainSound( const CTakeDamageInfo &info );
	void			AlertSound( void );
	void			AttackSound( void );
	void			IdleSound( void );
	void			HideSound( void );
	void			ChargeSound( void );

	void			PrescheduleThink(void);

	inline bool		InAttackSequence( void );

	bool			CreateBehaviors( void );

	void			InputAssault( inputdata_t &inputdata );

	virtual bool	ShouldMoveAndShoot( void ) { return true; }
	virtual bool	IsCurTaskContinuousMove();

	//Vector			FindNodePositionForBee(Vector startPos, Vector enemyPos);


	// Swatting physics objects
	int				GetSwatActivity( void );
	bool			FindNearestPhysicsObject( int iMaxMass );
	float			DistToPhysicsEnt( void );

	bool			OnInsufficientStopDist( AILocalMoveGoal_t *pMoveGoal, float distClear, AIMoveResult_t *pResult );

	void			GatherConditions( void );

	// Charging (from Antlion Guard)
	float			ChargeSteer( void );
	bool			ShouldCharge( const Vector &startPos, const Vector &endPos, bool useTime, bool bCheckForCancel );
	void			ImpactShock( const Vector &origin, float radius, float magnitude, CBaseEntity *pIgnored = NULL );
	void			ChargeLookAhead( void );
	bool			HandleChargeImpact( Vector vecImpact, CBaseEntity *pEntity );
	bool			EnemyIsRightInFrontOfMe( CBaseEntity **pEntity );
	void			ChargeDamage( CBaseEntity *pTarget );
	bool			OverrideMove( float flInterval );
	bool			OverrideMoveFacing( const AILocalMoveGoal_t &move, float flInterval );

	void			InputAttackItem( inputdata_t &inputdata );
	void			OpenPropDoorNow( CBasePropDoor *pDoor );

	// Grabbing ragdolls
#ifdef ALIENGRUNT_GRAB_RAGDOLL
	void			GrabRagdoll( void );
	void			ThrowRagdoll( void );
	void			InputDisableRagdollGrab( inputdata_t &inputdata );
	void			InputEnableRagdollGrab( inputdata_t &inputdata );
	void			InputDisableWaitRagdollGrab( inputdata_t &inputdata );

	void			NPCThink( void );

	void			Event_Killed( const CTakeDamageInfo &info );
	//int				MeleeAttack2Conditions( float flDot, float flDist );
#endif

private: 


#ifdef ALIENGRUNT_GRAB_RAGDOLL

	//TERO: ragdoll grabbing
	//CBoneFollowerManager m_BoneFollowerManager;
	EHANDLE				m_hRagdoll;
	IPhysicsConstraint*	m_hPlayerConstrain;
	float				m_flNextRagdollGrabTime;

	bool				DestroyPlayerConstrain(bool bThrow = false);
#endif

	//Item Swatting, stolen from the BaseZombie
	EHANDLE			m_hSetTarget;
	EHANDLE			m_hDoor;
	EHANDLE			m_hPhysicsEnt;
	EHANDLE			m_hObstructor;
	float			m_flNextSwat;
	float			m_flNextSwatScan;

	float			m_flChargeCancelTime;

	void			UpdateBeehiveGunAim();
	bool			m_bShouldAim;

	Activity		TranslateAimingActivity(Activity iActivity);

	void			UpdateHead();

	enum
	{
		SCHED_ALIENGRUNT_STAND = BaseClass::NEXT_SCHEDULE,
		SCHED_ALIENGRUNT_SHOOTBEES,
		SCHED_ALIENGRUNT_MELEE_ATTACK,
		SCHED_ALIENGRUNT_MOVE_SWATITEM,
		SCHED_ALIENGRUNT_SWATITEM,
		SCHED_ALIENGRUNT_ATTACKITEM,
		SCHED_ALIENGRUNT_GRAB_RAGDOLL,
		SCHED_ALIENGRUNT_ALERT,
		SCHED_ALIENGRUNT_PRESS_ATTACK,
		SCHED_ALIENGRUNT_CHARGE,

	};

	//=========================================================
	// ALIENGRUNT Tasks 
	//=========================================================
	enum 
	{
		TASK_ALIENGRUNT_GET_PATH_TO_PHYSOBJ = BaseClass::NEXT_TASK,
		TASK_ALIENGRUNT_SWAT_ITEM,
		TASK_ALIENGRUNT_SHOOT_BEES,
		TASK_ALIENGRUNT_CHARGE,
		TASK_ALIENGRUNT_CHASE_ENEMY_CONTINUOUSLY,
	};

	enum 
	{
		COND_ALIENGRUNT_CAN_SWAT_ATTACK = BaseClass::NEXT_CONDITION,
		COND_ALIENGRUNT_LOCAL_MELEE_OBSTRUCTION,
		COND_ALIENGRUNT_CAN_ATTACK_DOOR,
		COND_ALIENGRUNT_CAN_GRAB_RAGDOLL,
		COND_ALIENGRUNT_CANNOT_GRAB_RAGDOLL,
		COND_ALIENGRUNT_SHOULD_CHARGE,
		COND_ALIENGRUNT_BEES,
		COND_ALIENGRUNT_SHOULD_PROTECT_SQUADMATE,
	};

	void			ShootBees();

	int				m_iLeftHandAttachment;
	int				m_iRightHandAttachment;

	float			m_flNextChargeTime;
	bool			m_bDecidedNotToStop;

	float			m_HideSoundTime;

	CAI_AssaultBehavior	 m_AssaultBehavior;
	//CAI_LeadBehavior	 m_LeadBehavior;

	int				m_iBees;

#define ALIENGRUNT_BEE_BURST_SIZE_MIN 3
#define ALIENGRUNT_BEE_BURST_SIZE_MAX 4

private:

	float	m_flBurnDamage;				// Keeps track of how much burn damage we've incurred in the last few seconds.
	float	m_flBurnDamageResetTime;	// Time at which we reset the burn damage.

public:
	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;


};



//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CTraceFilterChargeAlienGrunt : public CTraceFilterEntitiesOnly
{
public:
	// It does have a base, but we'll never network anything below here..
	DECLARE_CLASS_NOBASE( CTraceFilterChargeAlienGrunt );
	
	CTraceFilterChargeAlienGrunt( const IHandleEntity *passentity, int collisionGroup, CNPC_AlienGrunt *pAttacker )
		: m_pPassEnt(passentity), m_collisionGroup(collisionGroup), m_pAttacker(pAttacker)
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

			if ( pEntity->m_takedamage == DAMAGE_NO )
				return false;

			// Translate the vehicle into its driver for damage
			/*if ( pEntity->GetServerVehicle() != NULL )	//TERO: disabled for now
			{
				CBaseEntity *pDriver = pEntity->GetServerVehicle()->GetPassenger();

				if ( pDriver != NULL )
				{
					pEntity = pDriver;
				}
			}*/
	
			Vector	attackDir = pEntity->WorldSpaceCenter() - m_pAttacker->WorldSpaceCenter();
			VectorNormalize( attackDir );

			float	flDamage = (pEntity->Classify() == CLASS_PLAYER_ALLY_VITAL) ? (sk_aliengrunt_dmg_claw.GetFloat() * 0.75f) : (sk_aliengrunt_dmg_claw.GetFloat());

			CTakeDamageInfo info( m_pAttacker, m_pAttacker, flDamage, DMG_CRUSH );
			CalculateMeleeDamageForce( &info, attackDir, info.GetAttacker()->WorldSpaceCenter(), 4.0f );

			CBaseCombatCharacter *pVictimBCC = pEntity->MyCombatCharacterPointer();

			// Only do these comparisons between NPCs
			if ( pVictimBCC )
			{
				// Can only damage other NPCs that we hate
				if ( m_pAttacker->IRelationType( pEntity ) == D_HT )
				{
					pEntity->TakeDamage( info );
					return true;
				}
			}
			else
			{
				// Otherwise just damage passive objects in our way
				pEntity->TakeDamage( info );
				Pickup_ForcePlayerToDropThisObject( pEntity );
			}
		}

		return false;
	}

public:
	const IHandleEntity *m_pPassEnt;
	int					m_collisionGroup;
	CNPC_AlienGrunt		*m_pAttacker;
};

#endif // NPC_ALIENGRUNT