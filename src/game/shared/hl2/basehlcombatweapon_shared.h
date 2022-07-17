//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "basecombatweapon_shared.h"

#ifndef BASEHLCOMBATWEAPON_SHARED_H
#define BASEHLCOMBATWEAPON_SHARED_H
#ifdef _WIN32
#pragma once
#endif

#if defined( CLIENT_DLL )
#define CBaseHLCombatWeapon C_BaseHLCombatWeapon
#endif

class CBaseHLCombatWeapon : public CBaseCombatWeapon
{
#if !defined( CLIENT_DLL )
#ifndef _XBOX
	DECLARE_DATADESC();
#else
protected:
	DECLARE_DATADESC();
private:
#endif
#endif

	DECLARE_CLASS(CBaseHLCombatWeapon, CBaseCombatWeapon);
public:
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	virtual bool	WeaponShouldBeLowered(void);

	bool	CanLower();
	bool	CanWalkBob();
	bool 	CanSprint();


	virtual bool	Ready(void);
	virtual bool	Lower(void);
	virtual bool	Deploy(void);
	virtual bool	Holster(CBaseCombatWeapon *pSwitchingTo);
	virtual void	WeaponIdle(void);

	virtual Activity	GetIdleLoweredActivity() { return ACT_VM_IDLE_LOWERED; };
	virtual Activity	GetIdleActivity() { return ACT_VM_IDLE; };
	virtual Activity	GetWalkActivity() { return ACT_VM_WALK; };
	virtual Activity	GetSprintActivity() { return ACT_VM_SPRINT; };


	virtual void	AddViewmodelBob(CBaseViewModel *viewmodel, Vector &origin, QAngle &angles);
	virtual	float	CalcViewmodelBob(void);

	virtual Vector	GetBulletSpread(WeaponProficiency_t proficiency);
	virtual float	GetSpreadBias(WeaponProficiency_t proficiency);

	virtual const	WeaponProficiencyInfo_t *GetProficiencyValues();
	static const	WeaponProficiencyInfo_t *GetDefaultProficiencyValues();

	virtual void	ItemHolsterFrame(void);

	int				m_iPrimaryAttacks;		// # of primary attacks performed with this weapon
	int				m_iSecondaryAttacks;	// # of secondary attacks performed with this weapon



protected:

	bool			m_bLowered;			// Whether the viewmodel is raised or lowered
	float			m_flRaiseTime;		// If lowered, the time we should raise the viewmodel
	float			m_flHolsterTime;	// When the weapon was holstered


	void					StartWalking();
	void					StopWalking();

	float           m_flBobKickZ; // auto-decaying bob to dip after jumping and landing
	bool            m_bTgtBobKickZ; // kick push to max first, then set this to false to decay
	float           m_flPrevPlayerVelZ; // remember the player's previous z velocity, and use the delta
	// to adjust the BobKick

	float           m_flRollAdj; // crowbar slash uses roll adjustment

private:

	float				m_flNextFidgetTime;



};

#endif // BASEHLCOMBATWEAPON_SHARED_H
