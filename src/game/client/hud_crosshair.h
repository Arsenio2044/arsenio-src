//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef HUD_CROSSHAIR_H
#define HUD_CROSSHAIR_H
#ifdef _WIN32
#pragma once
#endif

#include "hudelement.h"
#include <vgui_controls/Panel.h>
#include "basecombatweapon_shared.h"

#define BULLET_CHAR L'\u2022'
#define HOLLOW_BULLET_CHAR L'\u25E6'

namespace vgui
{
	class IScheme;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CHudCrosshair : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudCrosshair, vgui::Panel );
public:
	CHudCrosshair( const char *pElementName );
	virtual ~CHudCrosshair();

	virtual void	SetCrosshairAngle( const QAngle& angle );
	virtual void	SetCrosshair( CHudTexture *texture, const Color& clr );
	virtual void	ResetCrosshair();
	virtual void	DrawCrosshair( void ) {}
  	virtual bool	HasCrosshair( void ) { return ( m_pCrosshair != NULL ); }
	virtual bool	ShouldDraw();

	// any UI element that wants to be at the aim point can use this to figure out where to draw
	static void	GetDrawPosition ( float *pX, float *pY, bool *pbBehindCamera, QAngle angleCrosshairOffset = vec3_angle );
protected:
	virtual void	ApplySchemeSettings( vgui::IScheme *scheme );
	virtual void	Paint();
	virtual void    PaintHitMarks( const int crosshair_w, const int crosshair_h );
	virtual void    PaintHorizon( const int crosshair_w, const int crosshair_h );

	Vector m_vecPelletMarks[ MAX_PELLET_MARKS ];
	
	// Crosshair sprite and colors
	CHudTexture		*m_pCrosshair;
	CHudTexture		*m_pDefaultCrosshair;
	Color			m_clrCrosshair;
	QAngle			m_vecCrossHairOffsetAngle;

	CPanelAnimationVar( bool, m_bHideCrosshair, "never_draw", "false" );
	CPanelAnimationVar( vgui::HFont, m_Font1, "TitleFont", "Trebuchet8" );
	CPanelAnimationVar( vgui::HFont, m_Font2, "TitleFont", "Trebuchet10" );
	CPanelAnimationVar( vgui::HFont, m_Font3, "TitleFont", "Trebuchet12" );
	CPanelAnimationVar( vgui::HFont, m_Font4, "TitleFont", "Trebuchet14" );
	CPanelAnimationVar( vgui::HFont, m_Font5, "TitleFont", "Trebuchet18" );
	CPanelAnimationVar( vgui::HFont, m_Font6, "TitleFont", "Trebuchet24" );

	vgui::HFont GetTextFont( int i ) {
		switch (i) {
		case 1: return m_Font1; break;
		case 2: return m_Font2; break;
		case 3: return m_Font3; break;
		case 4: return m_Font4; break;
		case 5: return m_Font5; break;
		case 6: //fallthrough
		default: return m_Font6; break;

		}
	}
};



#define TARGET_CIRCLE_SCALE 0.0125f
// keep this consistent with weapon_egar and the airboat
#define EGAR_ROCKETS 8 

class CHudTargetCircles : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudTargetCircles, vgui::Panel );

public:
	CHudTargetCircles( const char *pElementName );
	virtual ~CHudTargetCircles();

	virtual void    ApplySchemeSettings( vgui::IScheme *scheme );
	virtual void    SetTargets( EHANDLE* hTargets ) { m_hTargets = hTargets; }
	virtual bool	ShouldDraw();
	virtual void    ShouldDraw( bool bDraw ) { m_bDraw = bDraw; }
	virtual void	Paint();
private:

	EHANDLE* m_hTargets;
	bool m_bDraw = false;
	CPanelAnimationVar( vgui::HFont, m_FontHuge, "TitleFont", "TrebuchetHuge" );
};

// Enable/disable crosshair rendering.
extern ConVar crosshair;


#endif // HUD_CROSSHAIR_H
