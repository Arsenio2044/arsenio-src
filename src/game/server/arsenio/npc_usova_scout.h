//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef NPC_SYNTHE_H
#define NPC_SYNTHE_H
#ifdef _WIN32
#pragma once
#endif

#include "npc_combine.h"

//=========================================================
//	>> CNPC_SynthE
//=========================================================
class CNPC_UScout : public CNPC_Combine
{
	DECLARE_CLASS(CNPC_UScout, CNPC_Combine );
#if HL2_EPISODIC
	DECLARE_DATADESC();
#endif

public: 
	void		Spawn( void );
	void		Precache( void );
	void		DeathSound( const CTakeDamageInfo &info );
	void		PrescheduleThink( void );
	void		BuildScheduleTestBits( void );
	int			SelectSchedule ( void );
	float		GetHitgroupDamageMultiplier( int iHitGroup, const CTakeDamageInfo &info );
	void		HandleAnimEvent( animevent_t *pEvent );
	void		OnChangeActivity( Activity eNewActivity );
	void		Event_Killed( const CTakeDamageInfo &info );
	void		OnListened();
	Class_T 	Classify();

	void		ClearAttackConditions( void );

	bool		m_fIsBlocking;

	bool		IsLightDamage( const CTakeDamageInfo &info );
	bool		IsHeavyDamage( const CTakeDamageInfo &info );

	virtual	bool		AllowedToIgnite( void ) { return true; }

private:
	bool		ShouldHitPlayer( const Vector &targetDir, float targetDist );

#if HL2_EPISODIC
public:
	Activity	NPC_TranslateActivity( Activity eNewActivity );

protected:
	/// whether to use the more casual march anim in ep2_outland_05
	int			m_iUseMarch;
#endif

	static int m_iBonusHunters;

};

#endif // NPC_SYNTHE_H
