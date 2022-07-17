// Special multi-target version of the rocket launcher. Airboat rockets without the airboat.
// (EGAR - Everyone gets a rocket!)

#ifndef WEAPON_EGAR_H
#define WEAPON_EGAR_H

#ifdef _WIN32
#pragma once
#endif


#include "weapon_rpg.h"
#include "basehlcombatweapon.h"

// Constants 

#define EGAR_ROCKETS 8

#define EGAR_ROCKET_RECHARGE_TIME 1.0f
#define EGAR_ROCKET_TARGET_TIME 0.25f
#define EGAR_ROCKET_FIRE_INTVAL 0.075f

enum {
	EGAR_ROCKET_IDLE = 0,
	EGAR_ROCKET_TARGETING,
	EGAR_ROCKET_FIRING
};

// Class definition

class CWeaponEGAR : public CBaseHLCombatWeapon
{
	DECLARE_CLASS( CWeaponEGAR, CBaseHLCombatWeapon );
public:

	CWeaponEGAR();
	~CWeaponEGAR();

	DECLARE_SERVERCLASS();

	void	Precache( void );

	//virtual void	PrimaryAttack( void );

	virtual float GetFireRate( void ) { return 1; };
	void	ItemPostFrame( void );

	bool	Deploy( void );
	bool	Holster( CBaseCombatWeapon *pSwitchingTo = NULL );
	//bool	Reload( void );
	bool	WeaponShouldBeLowered( void );

	virtual void Drop( const Vector &vecVelocity );

	int		GetMinBurst() { return 1; }
	int		GetMaxBurst() { return 1; }
	float	GetMinRestTime() { return 4.0; }
	float	GetMaxRestTime() { return 4.0; }

	bool	WeaponLOSCondition( const Vector &ownerPos, const Vector &targetPos, bool bSetConditions );
	int		WeaponRangeAttack1Condition( float flDot, float flDist );

	void	Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );

	void	NotifyRocketDied( void );

	bool    UsesPrimaryAmmo( void )  { return false; }
	bool	HasAnyAmmo( void );
	bool    HasPrimaryAmmo( void ) { return true; }



	int		CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	virtual const Vector& GetBulletSpread( void )
	{
		static Vector cone = VECTOR_CONE_3DEGREES;
		return cone;
	}

	DECLARE_ACTTABLE();
	DECLARE_DATADESC();

	// STUFF BELOW NOT COPIED FROM WEAPON_RPG

	void            UpdateRocketState( const CUserCmd *ucmd, int iButtonsReleased );
	void            FireRocketAt( CBaseEntity* target );
	bool            FindRocketTarget( EHANDLE &entTarget );
	bool            IsRocketTarget( CBaseEntity* ent );
	void            QueueAllRockets();
	void            CancelTarget(int nIndex);
	void            AddTarget();
	void            AddTargetMarker( CBaseEntity* targ, bool world, const Vector& pos );


	int             m_nRocketState;
	float           m_flRocketTime; // time of last rocket aimed/fired
	float           m_flRocketGenTime; // time of last rocket regenerated

	CNetworkVar( int, m_nRocketsReady );
	CNetworkVar( int, m_nRocketsQueued );
	CNetworkArray( EHANDLE, m_hRocketTargets, EGAR_ROCKETS );



protected:

	// WEAPON_RPG STUFF

	bool				m_bInitialStateUpdate;
	



	// NOT WEAPON_RPG STUFF

};

#endif // WEAPON_RPG_H

