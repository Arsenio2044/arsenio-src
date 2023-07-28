#ifndef LOADINGBG_H
#define LOADINGBG_H

#ifdef _WIN32
#pragma once
#endif

#include "vgui\ISurface.h"
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls\ImagePanel.h>
#include "ienginevgui.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CMapLoadBG : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE(CMapLoadBG, vgui::EditablePanel);

public:
	// Construction
	CMapLoadBG(char const* panelName);
	~CMapLoadBG();

	void SetNewBackgroundImage(char const* imageName);



protected:
	void ApplySchemeSettings(vgui::IScheme* pScheme);

	// Declare the OnTick function here, but don't define it
	virtual void OnTick();

private:
	vgui::ImagePanel* m_pBackground;
};

#endif	// !LOADINGBG_H