
// Client-side EGAR. Needed for HUD stuff.
#ifndef C_WEAPON_EGAR_H
#define C_WEAPON_EGAR_H

#ifdef _WIN32
#pragma once
#endif



#include "cbase.h"
#include "c_weapon__stubs.h"
#include "c_basehlcombatweapon.h"
#include "view.h"
#include "hud_crosshair.h"

#define EGAR_ROCKETS 8

class C_WeaponEGAR : public C_BaseHLCombatWeapon
{
	DECLARE_CLASS( C_WeaponEGAR, C_BaseHLCombatWeapon );
public:
	C_WeaponEGAR( void );

	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

	virtual void OnDataChanged( DataUpdateType_t updateType );

	virtual void ClientThink( void );

	virtual bool	Deploy( void );
	virtual bool	Holster( CBaseCombatWeapon *pSwitchingTo );

	virtual bool ShouldUseLargeViewModelVROverride() OVERRIDE{ return true; }

	int GetRocketsReady() { return m_nRocketsReady; }
	int GetRocketsQueued() { return m_nRocketsQueued; }
	EHANDLE* GetRocketTargets() { return m_hRocketTargets;  }

private:
	CHudTargetCircles mHud;
	int m_nRocketsReady;
	int m_nRocketsQueued;
	EHANDLE m_hRocketTargets[EGAR_ROCKETS];

};





#endif // C_WEAPON_EGAR_H