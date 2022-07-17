//========= TuxUI ===================//
//
// Purpose: Replace GameUI with TuxUI.
//
//
//=============================================================================//
#pragma once

#include "vgui2d/frame2d.h"
#include "vgui_controls/AnimationController.h"

#include "button_panel.h"

class Panel_QuitGame : public Frame2D
{
	DECLARE_CLASS_SIMPLE(Panel_QuitGame, Frame2D);

public:
	Panel_QuitGame(vgui::VPANEL Parent, const char* PanelName);

	virtual void OnThink();
	virtual void Paint();
	virtual void OnKillFocus();

private:
};