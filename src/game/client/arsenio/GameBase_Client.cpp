//========= Copyright Glitch Software, All rights reserved. ============//
//
// Purpose: 
// 
//=============================================================================//
#include "cbase.h"
#include "GameBase_Client.h"
#include "ienginevgui.h"
#include "c_baseplayer.h"
#include "c_baseentity.h"
#include "viewrender.h"

// ADD INCLUDES FOR OTHER MENUS: (NON-BASEVIEWPORT/INTERFACE)
#ifdef FUTURE
#include "NotePanel.h"
#endif
#include "DialoguePanel.h"
#include "steam/steam_api.h"
#include "gameconsoledialog.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static void EnableFilmGrain(IConVar *pConVar, char const *pOldString, float flOldValue)
{
	ConVarRef var(pConVar);
	if (var.GetBool())
	{
		IMaterial *pMaterial = materials->FindMaterial("effects/filmgrain", TEXTURE_GROUP_OTHER, true);
		if (pMaterial && view)
			view->SetScreenOverlayMaterial(pMaterial);
	}
	else
	{
		if (view)
			view->SetScreenOverlayMaterial(NULL);
	}
}

static ConVar tfo_fx_filmgrain("tfo_fx_filmgrain", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Enable or Disable film grain.", true, 0, true, 1, EnableFilmGrain);
static ConVar tfo_fx_filmgrain_strength("tfo_fx_filmgrain_strength", "2", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Set film grain strength.", true, 0.75f, true, 2.0f);

// GameUI
static CDllDemandLoader g_GameUIDLL("GameUI");

class CGameBase_Client : public IGameBase_Client
{
private:

	CGameConsoleDialog *m_pGameConsole;
#ifdef FUTURE
	CNotePanel *NotePanel;
#endif
	CDialogueMenu *DialoguePanel;

public:

	CGameBase_Client(void)
	{
		m_pGameConsole = NULL;

#ifdef FUTURE
		NotePanel = NULL;
#endif
		DialoguePanel = NULL;
	}

	// System
	void Initialize(bool bInGame = false);
	bool CanLoadMainMenu(void);
	void MoveConsoleToFront(void);
	void ActivateShaderEffects(void);
	void DeactivateShaderEffects(void);

	// VGUI Inits
	void CreateInGamePanels(vgui::VPANEL parent);

	// Cleanup on game exit
	void DestroyPanels(void);

	// Misc
	void ShowConsole(bool bToggle, bool bClose, bool bClear);

	// VGUI
	void ResetAll(void);
	void OpenPanel(int iPanel);
	void ClosePanels(int iExcluded);
#ifdef FUTURE
	void ShowNote(const char *szFile);
#endif
	void StartDialogueScene(const char *szFile, const char *szEntity, bool bOption1, bool bOption2, bool bOption3);
};





// Create in-game panels such as the inventory panel, note panel, etc...
void CGameBase_Client::CreateInGamePanels(vgui::VPANEL parent)
{
#ifdef FUTURE
	NotePanel = new CNotePanel(parent);
#endif
	DialoguePanel = new CDialogueMenu(parent);

}

// Destroy all instances of vgui panels/frames.
void CGameBase_Client::DestroyPanels(void)
{


	g_GameUIDLL.Unload();
	GameUI = NULL;


	if (m_pGameConsole)
	{
		m_pGameConsole->SetParent((vgui::Panel *)NULL);
		delete m_pGameConsole;
	}
#ifdef FUTURE
	if (NotePanel)
	{
		NotePanel->SetParent((vgui::Panel *)NULL);
		delete NotePanel;
	}
#endif


	if (DialoguePanel)
	{
		DialoguePanel->SetParent((vgui::Panel *)NULL);
		delete DialoguePanel;
	}


}





// Always force the console to be in front of everything else.
void CGameBase_Client::MoveConsoleToFront(void)
{
	if (m_pGameConsole && m_pGameConsole->IsVisible())
		m_pGameConsole->MoveToFront();
}



void CGameBase_Client::ActivateShaderEffects(void)
{
	if (tfo_fx_filmgrain.GetBool())
	{
		engine->ClientCmd_Unrestricted("tfo_fx_filmgrain 1\n");
		return;
	}

	if (view)
		view->SetScreenOverlayMaterial(NULL);
}

void CGameBase_Client::DeactivateShaderEffects(void)
{
	if (view)
		view->SetScreenOverlayMaterial(NULL);
}

// Initialize the main menu (background map load if possible)
// Called on game startup after bg load or if bg doesn't exist. Will be called again on bg loading manually.
// bool bInGame = true will auto fix key input issues related to the main menu and keyboard option menu.
void CGameBase_Client::Initialize(bool bInGame)
{




	// Did we want to launch with the console?
	if (ShouldOpenConsole())
		ShowConsole(true, false, false);

	m_bWantsConsole = false;

	// Set the language to english
	ConVarRef cc_lang("cc_lang");
	cc_lang.SetValue("english");

	// Disable Stuff:
	ConVar *hud_draw = cvar->FindVar("cl_drawhud");
	if (hud_draw)
		hud_draw->SetValue(0);

	// Make sure we hide the HUD:
	ConVar *pTFOHUD = cvar->FindVar("tfo_drawhud");
	if (pTFOHUD)
		pTFOHUD->SetValue(0);


}



#ifdef FUTURE
// Show Notes... Called by event...
void CGameBase_Client::ShowNote(const char *szFile)
{
	if (!szFile)
	{
		Warning("Note has faulty filename!\n");
		return;
	}

	if (strlen(szFile) <= 0)
	{
		Warning("Note has faulty filename!\n");
		return;
	}

	// Parse it in our note view then show it!
	if (NotePanel)
		NotePanel->ParseScriptFile(szFile);
}
#endif

// Start a dialogue scene.
void CGameBase_Client::StartDialogueScene(const char *szFile, const char *szEntity, bool bOption1, bool bOption2, bool bOption3)
{
	if (DialoguePanel != NULL)
	{
		if (!DialoguePanel->IsVisible())
			OpenPanel(4);

		DialoguePanel->SetupDialogueScene(szFile, szEntity, bOption1, bOption2, bOption3);
	}
}

// Reset vgui layouts @ all panels.
void CGameBase_Client::ResetAll()
{
	// Make sure fullbright never gets forced on.
	ConVar *fullBright = cvar->FindVar("mat_fullbright");
	if (fullBright)
		fullBright->SetValue(0);

#ifdef FUTURE
	if (NotePanel != NULL)
		NotePanel->PerformDefaultLayout();

#endif

	if (DialoguePanel != NULL)
	{
		if (DialoguePanel->IsVisible())
			OpenPanel(4);

		DialoguePanel->PerformDefaultLayout();
	}



	if (m_pGameConsole)
		ShowConsole(false, true, false);



	ClosePanels(0);
}

// Show the console, clear it or close it.
void CGameBase_Client::ShowConsole(bool bToggle, bool bClose, bool bClear)
{
	if (m_pGameConsole)
	{
		if (bToggle)
			m_pGameConsole->ToggleConsole(!m_pGameConsole->IsVisible());

		if (bClose && m_pGameConsole->IsVisible())
			m_pGameConsole->ToggleConsole(false, true);

		if (bClear)
			m_pGameConsole->Clear();
	}
}



// Close all open in-game panels.
void CGameBase_Client::ClosePanels(int iExcluded)
{

#ifdef FUTURE
	if (NotePanel && NotePanel->IsVisible() && (iExcluded != 1))
	{
		NotePanel->SetVisible(false);
		NotePanel->OnShowPanel(false);
	}
#endif


	if (DialoguePanel && DialoguePanel->IsVisible() && (iExcluded != 4))
	{
		ConVar* dialogue_menu = cvar->FindVar("cl_dialoguepanel");

		DialoguePanel->SetVisible(false);
		DialoguePanel->OnShowPanel(false);

		if (dialogue_menu)
			dialogue_menu->SetValue(0);
	}


}

// Open a GameUI panel for arse.
void CGameBase_Client::OpenPanel(int iPanel)
{
	ShowConsole(false, true, false);
	ClosePanels(iPanel);

	switch (iPanel)
	{
#ifdef FUTURE
	case 1:
	{
		if (NotePanel != NULL)
		{
			if (NotePanel->IsVisible())
			{
				NotePanel->Close();
				NotePanel->OnShowPanel(false);
			}
			else
			{
				NotePanel->Activate();
				NotePanel->OnShowPanel(true);
			}
		}
		break;
	}
#endif
#ifdef FUTURE
	case 2:
#else
	case 1:
#endif:
	{
		if (DialoguePanel != NULL)
		{
			// Use this convar to check on the server for enabling zoom fx. ( if bugs then change this to a clientCMD )...
			ConVar* dialogue_menu = cvar->FindVar("cl_dialoguepanel");

			if (DialoguePanel->IsVisible())
			{
				DialoguePanel->SetVisible(false);
				DialoguePanel->OnShowPanel(false);

				if (dialogue_menu)
					dialogue_menu->SetValue(0);
			}
			else
			{
				DialoguePanel->SetVisible(true);
				DialoguePanel->Activate();
				DialoguePanel->OnShowPanel(true);

				if (dialogue_menu)
					dialogue_menu->SetValue(1);
			}
		}
		break;
	}
	default:
		Warning("Invalid GameUI command!\n");
		break;
	}
}


// Console Commands
CON_COMMAND(OpenGameConsole, "Toggle the Console ON or OFF...")
{
	GameBaseClient->ShowConsole(true, false, false);
};

CON_COMMAND(CloseGameConsole, "Force the Console to close!")
{
	GameBaseClient->ShowConsole(false, true, false);
};

CON_COMMAND(ClearGameConsole, "Reset Console/Clear all history text.")
{
	GameBaseClient->ShowConsole(false, false, true);
};

CON_COMMAND(tfo_gameui_command, "Open or Close gameui panels for tfo.")
{
	if (args.ArgC() != 2)
	{
		Warning("This command requires 2 arguments!\n");
		return;
	}

	const char *szPanel = args[1];

	// Which panel do we want to open or close?
#ifdef FUTURE
	if (!strcmp(szPanel, "OpenNotePanel"))
		GameBaseClient->OpenPanel(1);
#endif
#ifdef FUTURE
	else if (!strcmp(szPanel, "OpenDialogue"))
		GameBaseClient->OpenPanel(4);
#else
	if (!strcmp(szPanel, "OpenDialogue"))
		GameBaseClient->OpenPanel(4);
#endif
	else
		Warning("Invalid panel!\n");
};

