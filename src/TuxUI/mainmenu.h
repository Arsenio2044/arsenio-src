//========= TuxUI ===================//
//
// Purpose: Replace GameUI with TuxUI.
//
//
//=============================================================================//
#pragma once

#include "vgui2d/frame2d.h"

#include "button_mainmenu.h"

class MainMenu : public Frame2D
{
	DECLARE_CLASS_SIMPLE(MainMenu, Frame2D);

public:
	MainMenu(vgui::Panel* Parent);

	virtual void ApplySchemeSettings(vgui::IScheme* Scheme);
	virtual void CreateMenu(const char* MenuScript);
	virtual void OnThink();
	virtual void DrawMainMenu();
	virtual void DrawLogo();
	virtual void Paint();
	virtual void OnCommand(char const* Command);
	virtual void OnSetFocus();
	virtual void OnKillFocus();
	virtual void UpdateMenu();
	virtual bool IsVisible();

private:
	CUtlVector<Button_MainMenu*> ActiveButtons;
	CUtlVector<Button_MainMenu*> ButtonsInGame;
	CUtlVector<Button_MainMenu*> ButtonsInGameSP;
	CUtlVector<Button_MainMenu*> ButtonsInGameMP;
	CUtlVector<Button_MainMenu*> ButtonsBackground;
	CUtlVector<Button_MainMenu*> ButtonsShared;

	bool bIsVisible;

	wchar_t* LogoLeftText;
	wchar_t* LogoRightText;

	float ButtonsSpace;

	float ButtonsOffsetX;
	float ButtonsOffsetY;

	float LogoOffsetX;
	float LogoOffsetY;

	bool bLogoAttachToMenu;
	bool bLogoAlignToLeft;

	Color LogoLeftColor;
	Color LogoRightColor;

	vgui::HFont LogoFont;
};

class MainMenuHelper : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(MainMenuHelper, vgui::Panel);

public:
	MainMenuHelper(MainMenu* Menu, vgui::Panel* Parent);
};