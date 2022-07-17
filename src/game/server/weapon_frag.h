//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "basehlcombatweapon.h"
#include "player.h"
#include "gamerules.h"
#include "grenade_frag.h"
#include "npcevent.h"
#include "engine/IEngineSound.h"
#include "items.h"
#include "in_buttons.h"
#include "soundent.h"
#include "gamestats.h"

//-----------------------------------------------------------------------------
// Fragmentation grenades
//-----------------------------------------------------------------------------
class CWeaponFrag : public CBaseHLCombatWeapon
{
	DECLARE_CLASS( CWeaponFrag, CBaseHLCombatWeapon );
public:
	DECLARE_SERVERCLASS();

public:
	CWeaponFrag();

	void	Precache( void );
	void	Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
	void	PrimaryAttack( void );
	void	SecondaryAttack( void );
	void	DecrementAmmo( CBaseCombatCharacter *pOwner );
	void	ItemPostFrame( void );

	bool	Deploy( void );
	bool	Holster( CBaseCombatWeapon *pSwitchingTo = NULL );

	int		CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	bool	Reload( void );

	bool	ShouldDisplayHUDHint() { return true; }

	void    ChuckGrenade( CBasePlayer* pPlayer, float flCookTime = 0.0 );

private:
	void	ThrowGrenade( CBasePlayer *pPlayer, float flCookTime = 0.0 );
	void	RollGrenade( CBasePlayer *pPlayer, float flCookTime = 0.0 );
	void	LobGrenade( CBasePlayer *pPlayer, float flCookTime = 0.0 );
	// check a throw from vecSrc.  If not valid, move the position back along the line to vecEye
	void	CheckThrowPosition( CBasePlayer *pPlayer, const Vector &vecEye, Vector &vecSrc );

	bool	m_bRedraw;	//Draw the weapon again after throwing a grenade

	int		m_AttackPaused;
	bool	m_fDrawbackFinished;
	bool    m_bTeleport; // teleport the player to the grenade's position after it explodes

	DECLARE_ACTTABLE();

	DECLARE_DATADESC();
};