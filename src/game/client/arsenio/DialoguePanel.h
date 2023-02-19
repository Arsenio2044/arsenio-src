//========= Copyright Glitch Software, All rights reserved. ============//
//
// Purpose: Dialogue Panel handles Dialogue Scenes. Max 3 options per scene.
// 
//=============================================================================//

#include "cbase.h"
#include "vgui_controls/Frame.h"
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui/IInput.h>

class CDialogueMenu : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE(CDialogueMenu, vgui::Frame);

public:
	CDialogueMenu(vgui::VPANEL parent);
	~CDialogueMenu();

	void OnCommand(const char* pcCommand);

	void ApplySchemeSettings(vgui::IScheme *pScheme);

	void PaintBackground()
	{
		SetBgColor(Color(0, 0, 0, 0));
		SetPaintBackgroundType(0);
		BaseClass::PaintBackground();
	}

	void PerformLayout();
	void PerformDefaultLayout();
	void OnThink();
	void OnShowPanel(bool bShow);
	void OnScreenSizeChanged(int iOldWide, int iOldTall);
	virtual bool PlayerHasItem(const char* szItem) = 0;
	virtual void ShowConsole(bool bToggle, bool bClose, bool bClear) = 0;
	virtual void StartDialogueScene(const char* szFile, const char* szEntity, bool bOption1, bool bOption2, bool bOption3) = 0;
	virtual void OpenPanel(int iPanel) = 0; // Opens a vgui panel of the x type.

	void CheckRollovers(int x, int y);

	// Parsing, Starting Dialogue Stuff:
	void SetupDialogueScene(const char *szFile, const char *szEntity, bool bOption1, bool bOption2, bool bOption3);
	char *szRecentFile;
	char *szRecentTargetEnt;
	KeyValues *GetDialogueData(const char *szFileName);
	void SetupDialoguePositions(KeyValues *pkvDialogueData, bool bOption1Once, bool bOption2Once, bool bOption3Once);

private:

	vgui::ImagePanel *m_pImgBackground;

	vgui::Label *m_pLabelText[3];
	vgui::ImagePanel *m_pImgButtonOver[3];
	vgui::Button *m_pButton[3];

	bool InGameLayout;
	bool InRolloverButton[3];
};
