//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//


#ifndef IVENGINE2_BASEWEAPON_H
#define IVENGINE2_BASEWEAPON_H
#ifdef _WIN32
#pragma once
#endif

#if defined( CLIENT_DLL )
#define CBaseIVEngine2Weapon C_BaseIVEngine2Weapon
#endif

class CBaseIVEngine2Weapon : public CBaseIVEngine2Weapon
{
#if !defined( CLIENT_DLL )
#ifndef _XBOX
#else
protected:
	DECLARE_DATADESC();
private:
#endif
#endif

	DECLARE_CLASS(CBaseIVEngine2Weapon, CBaseIVEngine2Weapon);
	DECLARE_DATADESC();
public:
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	virtual bool	WeaponShouldBeLowered( void );

			bool	CanLower();

			bool	CanSprint();
			bool	CanWalkBob();
	virtual bool	Ready( void );
	virtual bool	Lower( void );
	virtual bool	Deploy( void );
	virtual bool	Holster( CBaseCombatWeapon *pSwitchingTo );
	virtual void	WeaponIdle( void );


	virtual void	AddViewmodelBob(CBaseViewModel *viewmodel, Vector &origin, QAngle &angles);
	virtual	float	CalcViewmodelBob(void);


	virtual Vector	GetBulletSpread( WeaponProficiency_t proficiency );
	virtual float	GetSpreadBias( WeaponProficiency_t proficiency );

	virtual const	WeaponProficiencyInfo_t *GetProficiencyValues();
	static const	WeaponProficiencyInfo_t *GetDefaultProficiencyValues();

	virtual void	ItemHolsterFrame( void );

	int				m_iPrimaryAttacks;		// # of primary attacks performed with this weapon
	int				m_iSecondaryAttacks;	// # of secondary attacks performed with this weapon

protected:

	bool			m_bLowered;			// Whether the viewmodel is raised or lowered
	float			m_flRaiseTime;		// If lowered, the time we should raise the viewmodel
	float			m_flHolsterTime;	// When the weapon was holstered

	float           m_flBobKickZ; // auto-decaying bob to dip after jumping and landing
	bool            m_bTgtBobKickZ; // kick push to max first, then set this to false to decay
	float           m_flPrevPlayerVelZ; // remember the player's previous z velocity, and use the delta
	                                    // to adjust the BobKick

	float           m_flRollAdj; // crowbar slash uses roll adjustment

};

#endif // IVENGINE2_BASEWEAPON_H
