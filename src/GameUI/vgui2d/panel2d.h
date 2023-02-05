//========= GameUI ===================//
//
// Purpose: Adds new GameUI. Based off of GameUI2 by NicholasDe.
//
//
//=============================================================================//
#pragma once

#include "vgui_controls/Panel.h"

class Panel2D : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(Panel2D, vgui::Panel);

public:
	Panel2D();
	Panel2D(vgui::Panel* Parent);
	Panel2D(vgui::Panel* Parent, const char* PanelName);
	Panel2D(vgui::Panel* Parent, const char* PanelName, vgui::HScheme Scheme);

	virtual void Paint();
	virtual void PaintBlurMask();
	virtual void ApplySchemeSettings(vgui::IScheme* Scheme);

	virtual bool IsBlur() const
	{
		return bBlurEnabled;
	}

	virtual bool IsFullyVisible()
	{
		return vgui::ipanel()->IsFullyVisible(GetVPanel());
	}

	virtual bool IsSchemeLoaded() const
	{
		return bSchemeLoaded;
	}

private:
	bool bBlurEnabled;
	bool bSchemeLoaded;
};
