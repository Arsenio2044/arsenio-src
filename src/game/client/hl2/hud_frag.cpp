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
#include "vgui_controls/Panel.h"
#include "vgui_controls/AnimationController.h"
#include "vgui/ISurface.h"
#include <vgui/ILocalize.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar cl_frag_hud( "cl_frag_hud", "1" );
static ConVar cl_frag_hud_bg( "cl_frag_hud_bg", "0" );

#define FRAG_ICON_CHAR 'v'
#define BOX_SPACING 1.4

//-----------------------------------------------------------------------------
// Purpose: Shows the airboat rocket display. Show a rocket icon, aimed upwards
//          for each rocket that is ready. If the rocket is queued, draw in red
//          otherwise yellow.
//-----------------------------------------------------------------------------
class CHudFrags : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudFrags, vgui::Panel );

public:
	CHudFrags( const char *pElementName );
	virtual void Init( void );
	virtual void Reset( void );
	virtual void OnThink( void );
	bool ShouldDraw();
	void UpdateFragCount();

protected:
	virtual void Paint();
	virtual void PaintFuse( int ypos, int startx, int deltax, Color& color );

private:
	CPanelAnimationVar( vgui::HFont, m_hTextFont, "TextFont", "Default" );
	CPanelAnimationVarAliasType( float, m_flCookIconInsetX, "CookIconInsetX", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flCookIconInsetY, "CookIconInsetY", "300", "proportional_float" );

	CPanelAnimationVar( vgui::HFont, m_hIconFont, "IconFont", "WeaponIconsSmall" );
	CPanelAnimationVar( vgui::HFont, m_hIconFontBig, "IconFont", "WeaponIcons" );

	CPanelAnimationVarAliasType( float, m_flFragIconInsetX, "FragIconInsetX", "1", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flFragIconInsetY, "FragIconInsetY", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flFragIconGap, "FragIconGap", "15", "proportional_float" );

	CPanelAnimationVarAliasType( float, m_flCookIconWidth, "CookIconWidth", "32", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flBoxInsetY, "BoxInsetY", "45", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flBoxSize, "BoxSize", "5", "proportional_float" );
	CPanelAnimationVar( Color, m_FragIconColor, "FragIconColor", "255 220 0 60" );
	int m_iFragCount;
	float m_flCookTime;
	Color m_hInvis;
	int m_fcount;
};


DECLARE_HUDELEMENT( CHudFrags );

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudFrags::CHudFrags( const char *pElementName ) : CHudElement( pElementName ), 
                                                   BaseClass( NULL, "HudFrags" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudFrags::Init( void )
{
	m_iFragCount = 0;
	SetAlpha( 0 );
	m_hInvis.SetColor(0,0,0,0);
	if (!cl_frag_hud_bg.GetBool())
	    SetBgColor( m_hInvis );
	//Msg( "CHudFrags::Init" );
	m_fcount = 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudFrags::Reset( void )
{
	Init();
}


void CHudFrags::UpdateFragCount()
{
	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();

	if (pPlayer) {
		if (pPlayer->Weapon_OwnsThisType( "weapon_frag" )) {
			int newFragCount = pPlayer->GetAmmoCount( "grenade" );
			m_iFragCount = newFragCount;
		}
		else {
			m_iFragCount = 0;
		}
	    

		
		if (pPlayer->m_flFragCookStartTime > 0.0) {
			m_flCookTime = MIN(2.75, gpGlobals->curtime - pPlayer->m_flFragCookStartTime);
		}
		else {
			m_flCookTime = 0.0; // Not cooking a frag
		}
		/*if (++m_fcount % 30 == 0)
			Msg( "PCST %f CT %f\n", pPlayer->m_flFragCookStartTime, m_flCookTime );*/
	}
}

//-----------------------------------------------------------------------------
// Purpose: Save CPU cycles by letting the HUD system early cull
// costly traversal.  Called per frame, return true if thinking and 
// painting need to occur.
//-----------------------------------------------------------------------------
bool CHudFrags::ShouldDraw( void )
{
	if (!cl_frag_hud.GetBool())
		return false;

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if (!pPlayer)
		return false;

	return CHudElement::ShouldDraw();
}

//-----------------------------------------------------------------------------
// Purpose: updates hud icons
//-----------------------------------------------------------------------------
void CHudFrags::OnThink( void )
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	if (pPlayer->IsInAVehicle())
		SetVisible( false );

	UpdateFragCount();
	if (m_iFragCount < 1) {
        SetVisible( false );
		
		m_flCookTime = 0.0;
	}
		

	// show the display
	g_pClientMode->GetViewportAnimationController()->RunAnimationCommand( 
		this, "alpha", 255.0f, 0.0f, 0.4f, AnimationController::INTERPOLATOR_LINEAR );
}

//-----------------------------------------------------------------------------
// Purpose: draws the grenades display
//-----------------------------------------------------------------------------
void CHudFrags::Paint()
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	if (cl_frag_hud.GetInt() > 1 && cl_frag_hud.GetInt() < 256) {
		// use as alpha value
		m_FragIconColor[A] = cl_frag_hud.GetInt();
	}
	if (!cl_frag_hud_bg.GetBool())
	    SetBgColor( m_hInvis );
	
	int icon_count = m_iFragCount;
	Color cook_color( m_FragIconColor );

	// positioning
	float flCenteringAmt;
	int xpos;
	int ypos = scheme()->GetProportionalScaledValue( 1 );
	int panelx = scheme()->GetProportionalScaledValue( 200 );
	//int panely = scheme()->GetProportionalScaledValue( 64 );

	//int small_icon_y = scheme()->GetProportionalScaledValue( 5 );

	// If we're cooking a grenade, draw the top icon first
	if (m_flCookTime > 0.0)
	{
		icon_count--;
		// fade to red by lowering the green value (cook time increases from 0-2.75 sec)
		cook_color[G] -= MIN( 220, m_flCookTime * (m_FragIconColor[G] / 2.75f) );

		m_flCookIconWidth = surface()->GetCharacterWidth( m_hIconFontBig, (int)FRAG_ICON_CHAR ) * 0.90;
		float flCookIconHeight = surface()->GetFontTall( m_hIconFontBig );
		float flFuse_ypos = m_flBoxInsetY + ((flCookIconHeight) / 2);
		float flBoxSpace = (m_flBoxSize * BOX_SPACING) - m_flBoxSize;
		flCenteringAmt = ( panelx - m_flCookIconWidth ) / 2;
		xpos = flCenteringAmt;

		surface()->DrawSetTextColor( cook_color );
		ypos = m_flCookIconInsetY;
		surface()->DrawSetTextPos( xpos, ypos );
		surface()->DrawSetTextFont( m_hIconFontBig );

		surface()->DrawUnicodeChar( FRAG_ICON_CHAR );
		surface()->DrawSetColor( cook_color );
		PaintFuse( flFuse_ypos, xpos + (m_flCookIconWidth)+ flBoxSpace, 1, cook_color );
		PaintFuse( flFuse_ypos, xpos - (m_flBoxSize * BOX_SPACING) - scheme()->GetProportionalScaledValue( 1 ), -1, cook_color );
	}
	/*
	 * Have another hud element to display the number of frags now
	 *
	surface()->DrawSetTextFont( m_hIconFont );


	surface()->DrawSetTextColor( m_FragIconColor );
	
	m_flFragIconGap = surface()->GetCharacterWidth( m_hIconFont, (int)FRAG_ICON_CHAR ) * 0.95;
	flCenteringAmt = (panelx - (icon_count * m_flFragIconGap)) / 2;
	xpos = m_flFragIconInsetX + flCenteringAmt;
	ypos = panely - (m_flFragIconInsetY + small_icon_y);
	
	for (int i = 0; i < icon_count; i++)
	{
		surface()->DrawSetTextPos( xpos, ypos );
		
		surface()->DrawUnicodeChar( FRAG_ICON_CHAR );

		xpos += m_flFragIconGap;
	}

	*/

    

}


// Draw some rectangles on either side of the cooking frag to 
// indicate the remaining fuse time
void
CHudFrags::PaintFuse( int ypos, int startx, int deltax, Color& color )
{
	float interval = 0.25;
	int opac = color[A];

	for (float fuse_time = 3.0 - m_flCookTime; fuse_time > 0; fuse_time -= interval)
	{
		surface()->DrawSetTextPos( startx, ypos );

		// Make the final slash fade away
		if (fuse_time < interval) {
			color[A] = int( fuse_time / interval * opac );

		}
		surface()->DrawSetColor( color );
		surface()->DrawFilledRect( startx, ypos, startx + m_flBoxSize, ypos + m_flBoxSize );

		startx += deltax * m_flBoxSize * BOX_SPACING;
	}

	color[A] = opac;
}


