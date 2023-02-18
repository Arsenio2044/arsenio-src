#ifndef GAMEPADUI_LOADING_H
#define GAMEPADUI_LOADING_H
#ifdef _WIN32
#pragma once
#endif

#include "gamepadui_interface.h"
#include "vgui_controls/Panel.h"
#include "GameUI/IGameUI.h"
#include <vgui_controls/Label.h>
#include "filesystem.h"

namespace vgui
{
	class Panel;
}

class IVEngineClient;


class GamepadUILoading : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(GamepadUILoading, vgui::Panel);
public:
	GamepadUILoading(vgui::VPANEL parent);

	virtual void	ApplySchemeSettings(vgui::IScheme* pScheme);
	virtual void	PerformLayout();
	virtual void	Paint();
	MESSAGE_FUNC(OnActivate, "activate");
	virtual void SetRandomLoadingTip();


private:
	void	SolveEnginePanel();

private:

	int			m_iBarX;
	int			m_iBarY;
	int			m_iBarW;
	int			m_iBarH;
	bool		m_bResolved;

	vgui::Frame* m_pDialog;
	vgui::ProgressBar* m_pProgress;

	vgui::ImagePanel* m_pBGImage;
	vgui::ProgressBar* m_pProgressMirror;
	vgui::ImagePanel* m_pSpinnerImage;
	vgui::ImagePanel* m_pLogoImage;
	int m_SpinnerFrame;
	vgui::Label* m_pTextLoadingTip;


	// Tips
	float m_flTipDisplayTime;
};


extern GamepadUILoading* g_pGamepadUILoading;

#endif
