//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef HUD_RADAR_PANEL_H
#define HUD_RADAR_PANEL_H
#ifdef _WIN32
#pragma once
#endif

#include "hudelement.h"
#include <vgui_controls/Panel.h>
#include <vgui_controls/Label.h>

class C_BasePlayer;
void Radar_FlashPlayer(int iPlayer);

using namespace vgui;

class CPlayerRadarFlash
{
public:
	CPlayerRadarFlash()
	{
		m_flNextRadarFlashTime = 0.0f;
		m_iNumRadarFlashes = 0;
		m_bRadarFlash = false;
	}
	float m_flNextRadarFlashTime;	// When to next toggle the flash on the radar
	int	m_iNumRadarFlashes;			// How many flashes more to do
	bool m_bRadarFlash;				// Flash or do not, there is no try
};
//-----------------------------------------------------------------------------
// Purpose: Shows the radar panel
//-----------------------------------------------------------------------------
class CHudRadarPanel : public CHudElement, public Panel
{
	DECLARE_CLASS_SIMPLE( CHudRadarPanel, Panel );

public:
	CHudRadarPanel( const char *name );
	~CHudRadarPanel();
	
	virtual void Paint();
	virtual void Init();
	virtual void LevelInit();
	virtual bool ShouldDraw();
	virtual void SetVisible(bool state);
	virtual void Reset();

	void DrawRadar(void) { m_bHideRadar = false; }
	void HideRadar(void) { m_bHideRadar = true; }
	void MsgFunc_UpdateRadar(bf_read &msg);

private:
	void WorldToRadar(const Vector location, const Vector origin, const QAngle angles, float &x, float &y, float &z_delta);

	void DrawPlayerOnRadar(int iPlayer, C_BasePlayer *pLocalPlayer);
	void DrawEntityOnRadar(CBaseEntity *pEnt, C_BasePlayer *pLocalPlayer, int flags, int r, int g, int b, int a);

	void FillRect(int x, int y, int w, int h);
	void DrawRadarDot(int x, int y, float z_diff, int iBaseDotSize, int flags, int r, int g, int b, int a);

	CHudTexture *m_pBackground;
	CHudTexture *m_pBackgroundTrans;

	float m_flNextBombFlashTime;
	bool m_bBombFlash;

	float m_flNextHostageFlashTime;
	bool m_bHostageFlash;
	bool m_bHideRadar;
};

#endif // HUD_RADAR_PANEL_H