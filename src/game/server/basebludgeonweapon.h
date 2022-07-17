//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		The class from which all bludgeon melee
//				weapons are derived. 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//

#include "basehlcombatweapon.h"

#ifndef BASEBLUDGEONWEAPON_H
#define BASEBLUDGEONWEAPON_H

typedef enum {
	QMELEE_NO,
	QMELEE_DOING,
	QMELEE_DONE
} QMELEE_STATE;

//=========================================================
// CBaseHLBludgeonWeapon 
//=========================================================
class CBaseHLBludgeonWeapon : public CBaseHLCombatWeapon
{
	DECLARE_CLASS( CBaseHLBludgeonWeapon, CBaseHLCombatWeapon );
public:
	CBaseHLBludgeonWeapon();

	DECLARE_SERVERCLASS();

	virtual	void	Spawn( void );
	virtual	void	Precache( void );
	
	//Attack functions
	virtual	void	PrimaryAttack( void );
	virtual	void	SecondaryAttack( void );
	virtual void    QuickAttack( void );
	virtual void    SlashAttack( void );
	virtual void    SurgeAttack( void ); // TBD
	
	virtual void    ItemBusyFrame( void );
	virtual void	ItemPostFrame( void );

	//Functions to select animation sequences 
	virtual Activity	GetPrimaryAttackActivity( void )	{	return	ACT_VM_HITCENTER;	}
	virtual Activity	GetSecondaryAttackActivity( void )	{	return	ACT_VM_HITCENTER2;	}

	virtual	float	GetFireRate( void )								{	return	0.2f;	}
	virtual float	GetRange( void )								{	return	32.0f;	}
	virtual	float	GetDamageForActivity( Activity hitActivity )	{	return	1.0f;	}

	virtual int		CapabilitiesGet( void );
	virtual	int		WeaponMeleeAttack1Condition( float flDot, float flDist );

	virtual void    SetQuickMelee( QMELEE_STATE nState );

protected:
	virtual	void	ImpactEffect( trace_t &trace );
    QMELEE_STATE    m_nQuick; // Are we doing quick attacks with the melee key
	float           m_flNextQuickAttack; // Need a cooldown that persists across deploy/switch
	float           m_flNextQuickAttackCutoff; // Interrupt the animation after a short time
	float           m_flEndSurgeTime; // Controls how long surge lasts before swing
	bool            m_nSurgeAttack;
	bool            m_bSwingOnRelease;

private:
	bool			ImpactWater( const Vector &start, const Vector &end );
	void			Swing( int bIsSecondary );
	void            TestSwing();
	void			Hit( trace_t &traceHit, Activity nHitActivity, bool bIsSecondary );
	Activity		ChooseIntersectionPointAndActivity( trace_t &hitTrace, const Vector &mins, const Vector &maxs, CBasePlayer *pOwner );

	
};

#endif