//========= TuxUI ===================//
//
// Purpose: Replace GameUI with TuxUI.
//
//
//=============================================================================//
#pragma once

#include "vgui_controls/Frame.h"

class Frame2D : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE(Frame2D, vgui::Frame);

public:
	Frame2D(vgui::Panel* Parent, const char* PanelName, bool bShowTaskbarIcon = true, bool bPopUp = true);

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
