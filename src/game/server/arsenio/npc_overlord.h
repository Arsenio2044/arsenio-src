//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef NPC_OVERLORD_H
#define NPC_OVERLORD_H
#ifdef _WIN32
#pragma once
#endif

#include "npc_combine.h"


//=========================================================
//	>> CNPC_OverLord
//=========================================================
class CNPC_OverLord : public CNPC_Combine
{
	DECLARE_CLASS(CNPC_OverLord, CNPC_Combine);
#if HL2_EPISODIC
	DECLARE_DATADESC();
#endif

public:
	void		Spawn(void);
	void		Precache(void);
	void		DeathSound(const CTakeDamageInfo& info);
	void		PrescheduleThink(void);
	void		BuildScheduleTestBits(void);
	int			SelectSchedule(void);
	float		GetHitgroupDamageMultiplier(int iHitGroup, const CTakeDamageInfo& info);
	void		HandleAnimEvent(animevent_t* pEvent);
	void		OnChangeActivity(Activity eNewActivity);
	void		Event_Killed(const CTakeDamageInfo& info);
	void		OnListened();
	Class_T 	Classify();
	void Shove(void);
	void Hop(void);



	CAI_Senses* m_pSenses;
	CSound* m_pLockedBestSound;

	// Get Ent Name
	const char* GetEntName(void) { return ("OverLord"); }
	int GetMaxHP(void) { return 15000; }

	void	GetGunAim(Vector* vecAim);


	void	FireCannons(void);
	void	AimGun(void);
	void	EnemyShootPosition(CBaseEntity* pEnemy, Vector* vPosition);



	void		ClearAttackConditions(void);

	bool		m_fIsBlocking;
	bool IsJumpLegal(const Vector& startPos, const Vector& apex, const Vector& endPos) const;

	Vector		m_vGunAng;
	int			m_iAmmoLoaded;
	float		m_flReloadedTime;
	mutable float	m_flJumpDist;


	bool		IsLightDamage(const CTakeDamageInfo& info);
	bool		IsHeavyDamage(const CTakeDamageInfo& info);

	virtual	bool		AllowedToIgnite(void) { return true; }

private:
	bool		ShouldHitPlayer(const Vector& targetDir, float targetDist);

#if HL2_EPISODIC
public:
	Activity	NPC_TranslateActivity(Activity eNewActivity);

protected:
	/// whether to use the more casual march anim in ep2_outland_05
	int			m_iUseMarch;
#endif

	static int m_iBonusHunters;

};

#endif // NPC_OVERLORD_H
