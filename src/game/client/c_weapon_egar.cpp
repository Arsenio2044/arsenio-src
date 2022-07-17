//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Client-side weapon_egar. Does the rocket HUD stuff.
//
//=============================================================================//

#include "cbase.h"
#include "c_weapon__stubs.h"
#include "c_basehlcombatweapon.h"
#include "view.h"
#include "view_scene.h"
#include "vgui_controls/Controls.h"
#include "vgui/ISurface.h"
#include "hud_crosshair.h"
#include "c_weapon_egar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


STUB_WEAPON_CLASS_IMPLEMENT( weapon_egar, C_WeaponEGAR );

IMPLEMENT_CLIENTCLASS_DT( C_WeaponEGAR, DT_WeaponEGAR, CWeaponEGAR )
RecvPropInt( RECVINFO( m_nRocketsReady ) ),
RecvPropInt( RECVINFO( m_nRocketsQueued ) ),
RecvPropArray3( RECVINFO_ARRAY( m_hRocketTargets ), RecvPropEHandle( RECVINFO( m_hRocketTargets[0] ) ) )
END_RECV_TABLE()



//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
C_WeaponEGAR::C_WeaponEGAR( void ) :
    mHud( "HudTargetCircles" )
{
	mHud.SetTargets( m_hRocketTargets );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void C_WeaponEGAR::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );
	SetNextClientThink( CLIENT_THINK_ALWAYS );
}

bool C_WeaponEGAR::Deploy( void )
{	
	mHud.ShouldDraw( true );
	return BaseClass::Deploy();
}

bool C_WeaponEGAR::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	mHud.ShouldDraw( false );
    return BaseClass::Holster( pSwitchingTo );
}

void C_WeaponEGAR::ClientThink()
{
	BaseClass::ClientThink();
}


