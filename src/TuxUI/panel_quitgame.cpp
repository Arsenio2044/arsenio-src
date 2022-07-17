//========= Copyright Glitch Software, All rights reserved. ===================//
//
// Purpose: Replace GameUI with TuxUI.
//
//
//=============================================================================//
#include "gameui2_interface.h"
#include "panel_quitgame.h"

#include "ienginevgui.h"
#include "vgui/ILocalize.h"
#include "vgui/ISurface.h"
#include "vgui/IVGui.h"

#include <string>
#include <algorithm>
#include <functional>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

Panel_QuitGame::Panel_QuitGame(vgui::VPANEL Parent, const char* PanelName) : BaseClass(nullptr, PanelName)
{
	vgui::HScheme Scheme = vgui::scheme()->LoadSchemeFromFile("TuxUI/schemepanel.res", "SchemePanel");
	SetScheme(Scheme);
	
	SetProportional(false);
	SetPaintBorderEnabled(false);
	SetPaintBackgroundEnabled(false);
	SetDeleteSelfOnClose(false);
	SetTitleBarVisible(false);
	SetSizeable(false);
	SetMoveable(false);
	SetEnabled(true);
	SetVisible(false);
	SetParent(Parent);

	Activate();
}

void Panel_QuitGame::OnKillFocus()
{
	Close();
}

void Panel_QuitGame::OnThink()
{
	BaseClass::OnThink();

	SetBounds(0, 0, GetGameUI2().GetViewport().x, GetGameUI2().GetViewport().y);
}

void Panel_QuitGame::Paint()
{
	BaseClass::Paint();

	vgui::surface()->DrawSetColor(Color(0, 0, 0, 255));
	vgui::surface()->DrawFilledRect(0, GetTall() / 2 - 150, GetWide(), 300 + (GetTall() / 2 - 150));
}

CON_COMMAND(gameui2_openquitgamedialog, "")
{
	new Panel_QuitGame(GetGameUI2().GetVPanel(), "");
}