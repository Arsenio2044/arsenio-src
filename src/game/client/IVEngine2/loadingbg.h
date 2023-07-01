#ifndef LOADINGBG_H
#define LOADINGBG_H

#ifdef _WIN32
#pragma once
#endif

#include "vgui\ISurface.h"
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls\ImagePanel.h>
#include "ienginevgui.h"
#include "TipManager.h"


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

	void SetTipPanelText(const char* pMapName);

	void LoadMapData(const char* pMapName);


	vgui::Label* m_pTipLabel;


	CTipManager m_TipManager;



protected:
	void ApplySchemeSettings( vgui::IScheme *pScheme );

private:


	vgui::ImagePanel *m_pBackground;


};

#endif	// !LOADINGBG_H