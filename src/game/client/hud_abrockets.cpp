//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "c_basehlplayer.h"
#include "c_vehicle_airboat.h"
#include "c_weapon_egar.h"
#include "vgui_controls/Panel.h"
#include "vgui_controls/AnimationController.h"
#include "vgui/ISurface.h"
#include <vgui/ILocalize.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"




//-----------------------------------------------------------------------------
// Purpose: Shows the airboat rocket display. Show a rocket icon
//          for each rocket that is ready. If the rocket is queued, draw in red
//          otherwise yellow.
//-----------------------------------------------------------------------------
class CHudABRockets : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudABRockets, vgui::Panel );

public:
	CHudABRockets( const char *pElementName );
	virtual void Init( void );
	virtual void Reset( void );
	virtual void OnThink( void );
	bool ShouldDraw();

	C_PropAirboat* GetAirboat();
	void UpdateRocketCounts();

protected:
	virtual void Paint();

private:
	CPanelAnimationVar( vgui::HFont, m_hTextFont, "TextFont", "Default" );
	CPanelAnimationVarAliasType( float, text_xpos, "text_xpos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, text_ypos, "text_ypos", "300", "proportional_float" );

	CPanelAnimationVar( vgui::HFont, m_hIconFont, "IconFont", "HudNumbers" );
	CPanelAnimationVarAliasType( float, m_flIconInsetX, "IconInsetX", "1", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flIconInsetY, "IconInsetY", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flIconGap, "IconGap", "10", "proportional_float" );

	CPanelAnimationVar( Color, m_RocketIconColor, "RocketIconColor", "255 220 0 128" );
	CPanelAnimationVar( Color, m_QueuedRocketColor, "RocketQueuedColor", "255 80 0 128" );

	CPanelAnimationVar( Color, m_Special, "SpecialColor", "255 255 255 160" );

	int m_iRocketsReady;
	int m_iRocketsQueued;
	EHANDLE* m_hRocketTargets;
	bool m_bAnyRocketsReady;
	bool m_bRocketAdded;
	bool m_bRocketLaunched;
	bool m_bRocketQueued;

};


DECLARE_HUDELEMENT( CHudABRockets );

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudABRockets::CHudABRockets( const char *pElementName ) : CHudElement( pElementName ), BaseClass( NULL, "HudABRockets" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudABRockets::Init( void )
{
	m_iRocketsReady = 0;
	m_iRocketsQueued = 0;
	m_bAnyRocketsReady = false;

	m_bRocketAdded = false;
	m_bRocketLaunched = true;
	m_bRocketQueued = false;
	SetAlpha( 0 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudABRockets::Reset( void )
{
	Init();
}

//-----------------------------------------------------------------------------
// Purpose: Get the current vehicle
// Output : IClientVehicle
//-----------------------------------------------------------------------------
C_PropAirboat *CHudABRockets::GetAirboat()
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer || !pPlayer->IsInAVehicle())
	{
		return NULL;
	}

	return dynamic_cast<C_PropAirboat*>(pPlayer->GetVehicle());
}


void CHudABRockets::UpdateRocketCounts()
{
	bool msgs = false;

	C_PropAirboat* pAirboat = GetAirboat();
	if (pAirboat) {
		m_iRocketsReady = pAirboat->GetRocketsReady();
		m_iRocketsQueued = pAirboat->GetRocketsQueued();
		m_hRocketTargets = pAirboat->GetRocketTargets();
	}
	else
	{
		if (msgs)
			Msg( "Not in airboat, maybe launcher?\n" );
		C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
		if (!pPlayer)
			return;

		C_BaseCombatWeapon* pWeapon = pPlayer->GetActiveWeapon();
		if (!pWeapon)
			return;

		C_WeaponEGAR* pLauncher = dynamic_cast<C_WeaponEGAR*>(pWeapon);
		if (pLauncher)
		{
			if (msgs)
				Msg( "Have launcher, getting rocket counts!\n" );

			m_iRocketsReady = pLauncher->GetRocketsReady();
			m_iRocketsQueued = pLauncher->GetRocketsQueued();
			m_hRocketTargets = pLauncher->GetRocketTargets();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Save CPU cycles by letting the HUD system early cull
// costly traversal.  Called per frame, return true if thinking and 
// painting need to occur.
//-----------------------------------------------------------------------------
bool CHudABRockets::ShouldDraw( void )
{
	bool msgs = false;

	C_BaseHLPlayer *pPlayer = (C_BaseHLPlayer *)C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return false;

	C_PropVehicleDriveable* pVehicle =
		dynamic_cast<C_PropVehicleDriveable*>(pPlayer->GetVehicle());

	if (pVehicle)
	{

		C_PropAirboat* pAirboat =
			dynamic_cast<C_PropAirboat*>(pVehicle);

		if (pAirboat)
			return CHudElement::ShouldDraw();
	}
	else 
	{
		if (msgs)
		    Msg( "Not in vehicle - does have launcher?\n" );
		// If player has weapon_egar equipped
		C_BaseCombatWeapon* pWeapon = pPlayer->GetActiveWeapon();

		if (!pWeapon)
			return false;
		
		C_WeaponEGAR* pLauncher = dynamic_cast<C_WeaponEGAR*>(pWeapon);
		if (!pLauncher)
		{
			if (msgs)
				Msg( "No launcher, no hud\n" );
			return false;
		}
		if (msgs)
		Msg( "Drawing abrocket hud because player has launcher\n" );
		return CHudElement::ShouldDraw();
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: updates hud icons
//-----------------------------------------------------------------------------
void CHudABRockets::OnThink( void )
{
	UpdateRocketCounts();
	// show the display
	g_pClientMode->GetViewportAnimationController()->RunAnimationCommand( this, "alpha", 255.0f, 0.0f, 0.4f, AnimationController::INTERPOLATOR_LINEAR );
}

//-----------------------------------------------------------------------------
// Purpose: draws the rocket display
//-----------------------------------------------------------------------------
void CHudABRockets::Paint()
{
	C_BaseHLPlayer *pPlayer = (C_BaseHLPlayer *)C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	// draw the rocket display

	surface()->DrawSetTextColor( m_Special );
	surface()->DrawSetTextFont( m_hIconFont );
	int xpos = m_flIconInsetX, ypos = m_flIconInsetY;

	// Draw a special rocket to show where we start
	//surface()->DrawSetTextPos( xpos, ypos );
	//surface()->DrawUnicodeChar( 'x' );

	// start at the bottom and move up
	ypos += (AIRBOAT_ROCKETS - 2) * m_flIconGap;

	for (int i = 0; i < m_iRocketsReady; i++)
	{
		if (i < m_iRocketsQueued)
		{
			// draw targeted rockets redder
			surface()->DrawSetTextColor( m_QueuedRocketColor );
		}
		else
		{
			surface()->DrawSetTextColor( m_RocketIconColor );
		}

		surface()->DrawSetTextPos( xpos, ypos );

		surface()->DrawUnicodeChar( 'x' );

		ypos -= m_flIconGap;
	}

}


