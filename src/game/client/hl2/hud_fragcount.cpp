//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "hudelement.h"
#include "hud_numericdisplay.h"
#include <vgui_controls/Panel.h>
#include "hud.h"
#include "hud_suitpower.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include <vgui_controls/AnimationController.h>
#include <vgui/ISurface.h>
#include "c_basehlplayer.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar sk_max_grenade;
static ConVar cl_fragcount_alpha("cl_fragcount_alpha", "160");

#define FRAG_ICON_CHAR 'v'

//-----------------------------------------------------------------------------
// Purpose: Shows the flashlight icon
//-----------------------------------------------------------------------------
class CHudFragCount : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudFragCount, vgui::Panel );

public:
	CHudFragCount( const char *pElementName );
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	void OnThink( void );

protected:
	virtual void Paint();
	void UpdateFragCount();

private:
	
	CPanelAnimationVar( vgui::HFont, m_hFont, "Font", "WeaponIconsSmall" );
	CPanelAnimationVarAliasType( float, m_IconX, "icon_xpos", "4", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_IconY, "icon_ypos", "4", "proportional_float" );
	
	CPanelAnimationVarAliasType( float, m_flBarInsetX, "BarInsetX", "2", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flBarInsetY, "BarInsetY", "18", "proportional_float" );

	CPanelAnimationVarAliasType( float, m_flBarWidth, "BarWidth", "28", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flBarHeight, "BarHeight", "2", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flBarChunkWidth, "BarChunkWidth", "2", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flBarChunkGap, "BarChunkGap", "2", "proportional_float" );

	int m_iFragCount;
};	

using namespace vgui;

#ifdef HL2_EPISODIC
DECLARE_HUDELEMENT( CHudFragCount );
#endif // HL2_EPISODIC

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudFragCount::CHudFragCount( const char *pElementName ) : CHudElement( pElementName ), BaseClass( NULL, "HudFragCount" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT );

	m_iFragCount = 0;
}


void CHudFragCount::OnThink( void )
{
	UpdateFragCount();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pScheme - 
//-----------------------------------------------------------------------------
void CHudFragCount::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings(pScheme);
}


void CHudFragCount::UpdateFragCount()
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
	}
}

//-----------------------------------------------------------------------------
// Purpose: draws the flashlight icon
//-----------------------------------------------------------------------------
void CHudFragCount::Paint()
{
#ifdef HL2_EPISODIC
	C_BaseHLPlayer *pPlayer = (C_BaseHLPlayer *)C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
		return;

	// Only paint if we're using the new flashlight code
	if ( m_iFragCount == 0 || pPlayer->IsInAVehicle() )
	{
		SetPaintBackgroundEnabled( false );
		return;
	}

	SetPaintBackgroundEnabled( true );

	// get bar chunks
	int chunkCount = sk_max_grenade.GetInt();
	int enabledChunks = m_iFragCount;

	Color clrFragCount;
	clrFragCount = gHUD.m_clrNormal;
	clrFragCount[A] = cl_fragcount_alpha.GetInt();


	surface()->DrawSetTextFont( m_hFont );
	surface()->DrawSetTextColor( clrFragCount );
	surface()->DrawSetTextPos( m_IconX, m_IconY );
	surface()->DrawUnicodeChar( FRAG_ICON_CHAR );

	// draw the blocks to represent frags
	surface()->DrawSetColor( clrFragCount );
	int xpos = m_flBarInsetX, ypos = m_flBarInsetY;
	for (int i = 0; i < enabledChunks; i++)
	{
		surface()->DrawFilledRect( xpos, ypos, xpos + m_flBarChunkWidth, ypos + m_flBarHeight );
		xpos += (m_flBarChunkWidth + m_flBarChunkGap);
	}
	
	// Be even more transparent than we already are
	clrFragCount[A] = clrFragCount[A] / 16;

	// draw the exhausted portion of the bar.
	surface()->DrawSetColor( clrFragCount );
	for (int i = enabledChunks; i < chunkCount; i++)
	{
		surface()->DrawFilledRect( xpos, ypos, xpos + m_flBarChunkWidth, ypos + m_flBarHeight );
		xpos += (m_flBarChunkWidth + m_flBarChunkGap);
	}
#endif // HL2_EPISODIC
}
