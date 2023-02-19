//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Client Mode for TFO. Special accessors. 
//
//=============================================================================//

#ifndef GAMEBASE_CLIENT_H
#define GAMEBASE_CLIENT_H
#ifdef _WIN32
#pragma once
#endif

#include "steam/steam_api.h"
#include <GameUI/IGameUI.h>



abstract_class IGameBase_Client
{
public:

	// VGUI
	virtual void CreateGameUIPanels(vgui::VPANEL parent) = 0; // We will override the main menu and the loading interface which is why we don't pass in a parent.
	virtual void CreateInGamePanels(vgui::VPANEL parent) = 0; // Parent them to already existing panels. vgui_drawtree 1 to see which ones we tie ourselves to.
	virtual void ResetAll(void) = 0; // Resets all VGUI panels. 
	virtual void DestroyPanels(void) = 0; // Deconstruction from vgui_int.cpp
	virtual void OpenPanel(int iPanel) = 0; // Opens a vgui panel of the x type.
	virtual void ClosePanels(int iExcluded) = 0; // Closes all VGUI Panels.

	// System
	virtual void Initialize(bool bInGame = false) = 0;
	virtual void MoveConsoleToFront(void) = 0;
	virtual void ActivateShaderEffects(void) = 0;
	virtual void DeactivateShaderEffects(void) = 0;


	virtual void SetShouldStartWithConsole(bool bValue) { m_bWantsConsole = bValue; }
	bool ShouldOpenConsole() { return m_bWantsConsole; }
	virtual void ShowConsole(bool bToggle, bool bClose, bool bClear) = 0;



#ifdef FUTURE
	// Note Panel
	virtual void ShowNote(const char *szFile) = 0;
#endif

	// Dialogue Panel
	virtual void StartDialogueScene(const char *szFile, const char *szEntity, bool bOption1, bool bOption2, bool bOption3) = 0;

protected:

	// Game UI:
	IGameUI *GameUI;

	// Misc:
	bool m_bWantsConsole;
};

extern IGameBase_Client *GameBaseClient;

#endif // GAMEBASE_CLIENT_H