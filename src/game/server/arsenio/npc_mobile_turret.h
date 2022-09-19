//========= Copyright Glitch Software, All rights reserved. ============//
//
// Purpose: A turret.
//
//=============================================================================//

#ifndef NPC_MOBILE_TURRET_H
#define NPC_MOBILE_TURRET_H
#ifdef _WIN32
#pragma once
#endif


#include "ai_behavior_assault.h"
#include "ai_behavior_lead.h"
#include "ai_behavior_follow.h"

#include "smoke_trail.h"
#include "ai_playerally.h"

#define COORD_FRACTIONAL_BITSII		5
#define COORD_DENOMINATORII			(1<<(COORD_FRACTIONAL_BITSII))
#define COORD_RESOLUTIONII			(1.0/(COORD_DENOMINATORII))

//Eye states
enum mt_eyeState_t
{
	MTURRET_EYE_SEE_TARGET,			//Sees the target, bright and big
	MTURRET_EYE_SEEKING_TARGET,		//Looking for a target, blinking (bright)
	MTURRET_EYE_DORMANT,				//Not active
	MTURRET_EYE_DEAD,				//Completely invisible
	MTURRET_EYE_DISABLED,			//Turned off, must be reactivated before it'll deploy again (completely invisible)
	MTURRET_EYE_ALARM,				// On side, but warning player to pick it back up
};

class CSprite; // for eye sprite

//-----------------------------------------------------------------------------
// Activities.
//-----------------------------------------------------------------------------
//int ACT_MONK_GUN_IDLE;  MOVED INTO THE PROTECTED SECTION !! JASON

class CNPC_Mobile_Turret : public CAI_PlayerAlly
{
	DECLARE_CLASS( CNPC_Mobile_Turret, CAI_PlayerAlly );
	DECLARE_DATADESC(); //

public:

	CNPC_Mobile_Turret( void );
	void Spawn();
	void Precache();
	void PostNPCInit(); //
	void OnRestore(); // use to reload the smoking when low healthed

	bool CreateBehaviors();
	int GetSoundInterests();
	void BuildScheduleTestBits( void );
	Class_T	Classify( void );
	bool	CreateVPhysics( void ); //

	bool ShouldBackAway();

	//bool IsValidEnemy( CBaseEntity *pEnemy ); REMOVED BY JASON, was saying enemies wernt valid, maybe we can define a max length

	// Damage & Death
	virtual int OnTakeDamage_Alive( const CTakeDamageInfo &info ); //
	void Event_Killed( const CTakeDamageInfo &info ); //
	void DeathEffects(); //
	bool CanBecomeRagdoll( void ) { return false; } // 
	void PainSound( const CTakeDamageInfo &info );
	void DeathSound( const CTakeDamageInfo &info );

	bool PassesDamageFilter( const CTakeDamageInfo &info );
	void OnKilledNPC( CBaseCombatCharacter *pKilled );

	void StartBleeding();
	inline bool		IsBleeding() { return m_bIsBleeding; }

	// Combat
	void MakeTracer( const Vector &vecTracerSrc, const trace_t &tr, int iTracerType ); //
	Vector GetAttackSpread( CBaseCombatWeapon *pWeapon, CBaseEntity *pTarget ) { return VECTOR_CONE_5DEGREES; } //
	WeaponProficiency_t CalcWeaponProficiency( CBaseCombatWeapon *pWeapon );
	Vector GetActualShootPosition( const Vector &shootOrigin );
	Vector GetActualShootTrajectory( const Vector &shootOrigin );

	// Sensing 

	void GatherConditions(); //
	Vector EyePosition(); //
	bool FVisible( CBaseEntity *pEntity, int traceMask, CBaseEntity **ppBlocker ); //
	bool QuerySeeEntity( CBaseEntity *pEntity, bool bOnlyHateOrFearIfNPC = false ); //

	bool IsOpeningOrClosing() { return (GetAbsVelocity().z != 0.0f); } //
	bool IsEnabled(); //
	bool IsOpen(); //

	// Tasks & Schedules
	void PrescheduleThink();
	int	TranslateSchedule( int scheduleType );
	int	SelectSchedule ();
	void StartTask( const Task_t *pTask );
	void RunTask( const Task_t *pTask );
	int SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode );

	bool IsJumpLegal( const Vector &startPos, const Vector &apex, const Vector &endPos ) const;

	// Updating body and head rotation
	void UpdateHead();
	bool OverrideMove( float flInterval );
	void StayOnGround( void );

	// Activities & Animation
	void HandleAnimEvent( animevent_t *pEvent );
	Activity			NPC_TranslateActivity( Activity eNewActivity );
	virtual void		Shoot();
	virtual void		Scan();
	void				ProjectBeam( const Vector &vecStart, const Vector &vecDir, int width, int brightness, float duration );//

	// Eye Laser
	void SetEyeState( mt_eyeState_t state );
	void Activate( void ); // from base but needed for an update
	void UpdateOnRemove( void ); // needed to clear sprites

	// Local
	void SetActive( bool bActive ) {}

	// Inputs
	void InputEnable( inputdata_t &inputdata );
	void InputDisable( inputdata_t &inputdata );

	// Outputs
	COutputEvent	m_OnAreaClear;

	DEFINE_CUSTOM_AI;

protected:
	// Bleeding (smoking)
	bool				m_bIsBleeding;
	// Pose parameters
	Vector				m_vecShootPitch; // from head pitch to shoot pitch - JASON
	int					m_nPoseFaceVert;
	int					m_nPoseFaceHoriz;
private:
	//-----------------------------------------------------
	// Conditions, Schedules, Tasks
	//-----------------------------------------------------
	enum
	{
		SCHED_FOLLOW_PLAYER = BaseClass::NEXT_SCHEDULE,
		SCHED_IVESTIGATE_SOUND,
		SCHED_FOLLOW_ENEMY,
		SCHED_LOOK_AROUND,
		SCHED_FIRE_ON_ENEMY,

		FIRE_AT_ENEMY = BaseClass::NEXT_TASK,

	};

	// Inputs
	void	InputPerfectAccuracyOn( inputdata_t &inputdata );
	void	InputPerfectAccuracyOff( inputdata_t &inputdata );
	
	CAI_AssaultBehavior		m_AssaultBehavior;
	CAI_LeadBehavior		m_LeadBehavior;
	CAI_FollowBehavior		m_FollowBehavior; // <--
	int						m_iNumZombies;
	int						m_iDangerousZombies;
	bool					m_bPerfectAccuracy;
	bool					m_bMournedPlayer;

	// v from Ground Turret v
	int			m_iAmmoType;
	SmokeTrail	*m_pSmoke;

	bool		m_bEnabled;

	float		m_flTimeNextShoot;
	float		m_flTimeLastSawEnemy;
	Vector		m_vecSpread;
	bool		m_bHasExploded;
	int			m_iDeathSparks;
	float		m_flSensingDist;
	bool		m_bSeeEnemy;
	float		m_flTimeNextPing;

	Vector		m_vecClosedPos;
	Vector		m_vecLightOffset;

	// for the eye sprite
	int						m_iEyeAttachment;
	bool					m_bBlinkState;
	mt_eyeState_t			m_iEyeState;
	CHandle<CSprite>		m_hEyeGlow;

	HSOUNDSCRIPTHANDLE 	m_ShotSounds;

};

#endif //#ifndef NPC_MOBILE_TURRET_H