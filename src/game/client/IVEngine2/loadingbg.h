#ifndef LOADINGBG_H
#define LOADINGBG_H

#ifdef _WIN32
#pragma once
#endif

#include "vgui\ISurface.h"
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls\ImagePanel.h>
#include "ienginevgui.h"
#include <vgui_controls/Label.h>




//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CMapLoadBG : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CMapLoadBG, vgui::EditablePanel);

public:
	// Construction
	CMapLoadBG( char const *panelName );
	~CMapLoadBG();

	void SetNewBackgroundImage( char const *imageName );

	void SetRandomLoadingTip();


protected:
	void ApplySchemeSettings( vgui::IScheme *pScheme );

private:
	vgui::ImagePanel *m_pBackground;

	vgui::Label* m_pTextLoadingTip;

	// Tips
	float m_flTipDisplayTime;
};

#endif	// !LOADINGBG_H