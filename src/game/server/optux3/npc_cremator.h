//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#ifndef NPC_CrematorHL1_H
#define NPC_CrematorHL1_H

#define	CREMATOR_MAX_BEAMS	1
#define	CREMATOR_MAX_NOISE	12
#define MAX_BURN_RADIUS		256
#define RADIUS_GROW_RATE	50.0	// units/sec 
#define IMMOLATOR_TARGET_INVALID Vector( FLT_MAX, FLT_MAX, FLT_MAX )

#include "ai_basenpc.h"
//=========================================================
//=========================================================
class CNPC_Cremator : public CAI_BaseNPC //CHL1BaseNPC by Default
{
	DECLARE_CLASS( CNPC_Cremator, CAI_BaseNPC );
public:

	float m_flBurnRadius;
	float m_flNextPrimaryAttack;
	float m_flTimeLastUpdatedRadius;
	Vector  m_vecImmolatorTarget;

	void Spawn( void );
	void Precache( void );
	Class_T	Classify ( void );

	void AlertSound( void );
	void IdleSound( void );
	void PainSound( const CTakeDamageInfo &info );
	void DeathSound( const CTakeDamageInfo &info );
	void RunTask ( const Task_t *pTask );
	
	int	 GetSoundInterests ( void );
	
	float MaxYawSpeed ( void );

	void Event_Killed( const CTakeDamageInfo &info );
	void CallForHelp( char *szClassname, float flDist, CBaseEntity * pEnemy, Vector &vecLocation );

	Activity NPC_TranslateActivity( Activity eNewActivity );

	int  RangeAttack1Conditions( float flDot, float flDist );

	int  OnTakeDamage_Alive( const CTakeDamageInfo &inputInfo );

	void TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator );
	
	void StartTask( const Task_t *pTask );

	int  SelectSchedule( void );
	int  TranslateSchedule( int scheduleType );

	void NoiseBeam( void );
	void BeamGlow( void );
	void ImmoBeam( int side );
	void ClearBeams( void );

	int	m_beamIndex;

	CBaseEntity*	m_pOwner;

	void HandleAnimEvent( animevent_t *pEvent );


	DEFINE_CUSTOM_AI;
	DECLARE_DATADESC();

private:
	int m_iVoicePitch;
	int	  m_iBeams;
	int	  m_iNoise;

	int m_iBravery;

	CBeam *m_pBeam[CREMATOR_MAX_BEAMS];
	CBeam *m_hNoise[CREMATOR_MAX_NOISE];

	float m_flNextAttack;

	EHANDLE m_hDead;
};


#endif //NPC_CrematorHL1_