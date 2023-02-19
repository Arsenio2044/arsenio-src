//========= Copyright Glitch Software, All rights reserved. ============//
//
// Purpose: Dialogue Panel handles Dialogue Scenes. Max 3 options per scene.
// 
//=============================================================================//

#include "cbase.h"
#include "DialoguePanel.h"
#include "vgui_controls/Frame.h"
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui/IInput.h>

#include "vgui_controls/Button.h"
#include "vgui_controls/ImagePanel.h"

#include "GameBase_Client.h"
#include "KeyValues.h"
#include "filesystem.h"

using namespace vgui;

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IGameBase_Client* GameBaseClient = NULL;

ConVar cl_dialoguepanel("cl_dialoguepanel", "0", FCVAR_CLIENTDLL | FCVAR_HIDDEN, "Opens Dialogue Panel when interacting with CRUCIAL NPC's");
ConVar cl_dialoguemode("cl_dialoguemode", "0", FCVAR_ARCHIVE, "0 = Firstperson and 1 = Thirdperson");

//-----------------------------------------------------------------------------
// Purpose: Layout Fixup
//-----------------------------------------------------------------------------
void CDialogueMenu::PerformLayout()
{
	BaseClass::PerformLayout();
}

void CDialogueMenu::PerformDefaultLayout()
{
	for (int i = 0; i <= 2; i++)
	{
		InRolloverButton[i] = false;
		m_pImgButtonOver[i]->SetVisible(true);
		m_pImgButtonOver[i]->SetImage("transparency");
		m_pButton[i]->SetVisible(false);
		m_pButton[i]->SetEnabled(false);
		m_pLabelText[i]->SetText("");
	}

	m_pImgBackground->SetVisible(true);
	m_pImgBackground->SetEnabled(true);

	PerformLayout();
}

void CDialogueMenu::OnScreenSizeChanged(int iOldWide, int iOldTall)
{
	BaseClass::OnScreenSizeChanged(iOldWide, iOldTall);

	LoadControlSettings("resource/ui/dialoguemenu.res");

	PerformDefaultLayout();
}

void CDialogueMenu::OnThink()
{
	int x, y;
	vgui::input()->GetCursorPos(x, y);

	SetSize(ScreenWidth(), ScreenHeight());
	SetPos(0, 0);

	CheckRollovers(x, y);

	BaseClass::OnThink();
}

void CDialogueMenu::CheckRollovers(int x, int y)
{
	for (int i = 0; i <= 2; i++)
	{
		if (!m_pButton[i]->IsVisible())
			continue;

		// Check and see if mouse cursor is within button bounds
		if (m_pButton[i]->IsWithin(x, y))
		{
			if (!InRolloverButton[i])
			{
				vgui::surface()->PlaySound("dialogue/button_over.wav");
				InRolloverButton[i] = true;
			}
		}
		else
		{
			if (InRolloverButton[i])
			{
				InRolloverButton[i] = false;
			}
		}

		if (InRolloverButton[i])
			m_pImgButtonOver[i]->SetImage("dialogue/button_over");
		else
			m_pImgButtonOver[i]->SetImage("transparency");
	}
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CDialogueMenu::CDialogueMenu(vgui::VPANEL parent) : BaseClass(NULL, "CDialogueMenu")
{
	SetParent(parent);

	SetKeyBoardInputEnabled(true);
	SetMouseInputEnabled(true);

	SetProportional(true);
	SetTitleBarVisible(false);
	SetMinimizeButtonVisible(false);
	SetMaximizeButtonVisible(false);
	SetCloseButtonVisible(false);
	SetSizeable(false);
	SetMoveable(false);
	SetVisible(false);

	// Initialize images
	for (int i = 0; i <= 2; i++)
	{
		m_pImgButtonOver[i] = vgui::SETUP_PANEL(new vgui::ImagePanel(this, VarArgs("Button%i", (i + 1))));
		m_pButton[i] = vgui::SETUP_PANEL(new vgui::Button(this, VarArgs("btnButton%i", (i + 1)), ""));
		m_pLabelText[i] = vgui::SETUP_PANEL(new vgui::Label(this, VarArgs("Text%i", (i + 1)), ""));

		m_pButton[i]->SetPaintBorderEnabled(false);
		m_pButton[i]->SetPaintEnabled(false);
		m_pButton[i]->SetZPos(200);
		m_pButton[i]->SetReleasedSound("dialogue/buttonclick.wav");
		m_pButton[i]->SetCommand(VarArgs("Button%i", (i + 1)));

		m_pImgButtonOver[i]->SetImage("dialogue/invis");
		m_pImgButtonOver[i]->SetZPos(150);
		m_pImgButtonOver[i]->SetShouldScaleImage(true);

		m_pLabelText[i]->SetText("");
		m_pLabelText[i]->SetVisible(true);
	}

	m_pImgBackground = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "Background"));

	// Background
	m_pImgBackground->SetVisible(true);
	m_pImgBackground->SetEnabled(true);

	m_pImgBackground->SetZPos(125);
	m_pImgBackground->SetImage("dialogue/background");
	m_pImgBackground->SetShouldScaleImage(true);

	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/ArScheme.res", "ArScheme"));

	LoadControlSettings("resource/ui/dialoguemenu.res");

	InvalidateLayout();

	PerformDefaultLayout();

	szRecentTargetEnt = NULL;
	szRecentFile = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CDialogueMenu::~CDialogueMenu()
{
	if (szRecentTargetEnt != NULL)
	{
		delete[] szRecentTargetEnt;
		szRecentTargetEnt = NULL;
	}

	if (szRecentFile != NULL)
	{
		delete[] szRecentFile;
		szRecentFile = NULL;
	}
}

void CDialogueMenu::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	for (int i = 0; i <= 2; i++)
	{
		m_pLabelText[i]->SetFont(pScheme->GetFont("Nidus Sans"));
		m_pLabelText[i]->SetFgColor(Color(90, 40, 40, 245));
	}
}

// Return loaded values of the actual file in the res-data folder:
KeyValues *CDialogueMenu::GetDialogueData(const char *szFileName)
{
	KeyValues *kvGetDialogueData = new KeyValues("DialogueSceneData");
	if (kvGetDialogueData->LoadFromFile(filesystem, VarArgs("resource/data/dialogue/%s.txt", szFileName), "MOD"))
	{
		return kvGetDialogueData;
	}
	else
		Warning("Dialogue Script %s couldn't be found!\n", szFileName);

	kvGetDialogueData->deleteThis();
	return NULL;
}

// Setup a new dialogue scene:
void CDialogueMenu::SetupDialogueScene(const char *szFile, const char *szEntity, bool bOption1, bool bOption2, bool bOption3)
{
	if (szRecentTargetEnt != NULL)
	{
		delete[] szRecentTargetEnt;
		szRecentTargetEnt = NULL;
	}

	if (szRecentFile != NULL)
	{
		delete[] szRecentFile;
		szRecentFile = NULL;
	}

	szRecentFile = new char[256];
	Q_strncpy(szRecentFile, szFile, 256);

	szRecentTargetEnt = new char[256];
	Q_strncpy(szRecentTargetEnt, szEntity, 256);

	KeyValues *kvDialogueData = GetDialogueData(szRecentFile);
	if (kvDialogueData == NULL)
		return;

	// Reset all at first:
	PerformDefaultLayout();

	// Update Pos
	SetupDialoguePositions(kvDialogueData, bOption1, bOption2, bOption3);

	if (bOption1)
	{
		KeyValues *kvOption1Field = kvDialogueData->FindKey("Option1");
		if (kvOption1Field)
		{
			bool bShouldShow = true;
#ifdef FUTURE
			const char *szItemCheck = ReadAndAllocStringValue(kvOption1Field, "HasItem");
			if (strlen(szItemCheck) > 0)
			{
				if (!GameBaseClient->PlayerHasItem(szItemCheck))
					bShouldShow = false;
			}
#endif
			if (bShouldShow)
			{
				m_pButton[0]->SetVisible(true);
				m_pButton[0]->SetEnabled(true);

				m_pLabelText[0]->SetText(ReadAndAllocStringValue(kvOption1Field, "Text"));
			}
		}
	}

	if (bOption2)
	{
		KeyValues *kvOption2Field = kvDialogueData->FindKey("Option2");
		if (kvOption2Field)
		{
			bool bShouldShow = true;
#ifdef FUTURE
			const char *szItemCheck = ReadAndAllocStringValue(kvOption2Field, "HasItem");
			if (strlen(szItemCheck) > 0)
			{
				if (!GameBaseClient->PlayerHasItem(szItemCheck))
					bShouldShow = false;
			}
#endif
			if (bShouldShow)
			{
				m_pButton[1]->SetVisible(true);
				m_pButton[1]->SetEnabled(true);

				m_pLabelText[1]->SetText(ReadAndAllocStringValue(kvOption2Field, "Text"));
			}
		}
	}

	if (bOption3)
	{
		KeyValues *kvOption3Field = kvDialogueData->FindKey("Option3");
		if (kvOption3Field)
		{
			bool bShouldShow = true;
#ifdef FUTURE
			const char *szItemCheck = ReadAndAllocStringValue(kvOption3Field, "HasItem");
			if (strlen(szItemCheck) > 0)
			{
				if (!GameBaseClient->PlayerHasItem(szItemCheck))
					bShouldShow = false;
			}
#endif

			if (bShouldShow)
			{
				m_pButton[2]->SetVisible(true);
				m_pButton[2]->SetEnabled(true);

				m_pLabelText[2]->SetText(ReadAndAllocStringValue(kvOption3Field, "Text"));
			}
		}
	}

	kvDialogueData->deleteThis();
}

// Re position buttons:
void CDialogueMenu::SetupDialoguePositions(KeyValues *pkvDialogueData, bool bOption1Once, bool bOption2Once, bool bOption3Once)
{
	bool bOption1, bOption2, bOption3;

	KeyValues *kvOption1 = pkvDialogueData->FindKey("Option1");
	KeyValues *kvOption2 = pkvDialogueData->FindKey("Option2");
	KeyValues *kvOption3 = pkvDialogueData->FindKey("Option3");

	bOption1 = (kvOption1 != NULL) ? true : false;
	bOption2 = (kvOption2 != NULL) ? true : false;
	bOption3 = (kvOption3 != NULL) ? true : false;

	int x, y;
	m_pButton[1]->GetPos(x, y);

	int yPositions[] =
	{
		307,
		325,
		342,
	};

	for (int i = 0; i <= 2; i++)
	{
		m_pButton[i]->SetPos(x, scheme()->GetProportionalScaledValue(yPositions[i]));
		m_pImgButtonOver[i]->SetPos(x, scheme()->GetProportionalScaledValue(yPositions[i]));
		m_pLabelText[i]->SetPos(x, scheme()->GetProportionalScaledValue(yPositions[i]));
	}

	if (!bOption1 || !bOption1Once)
	{
		if (bOption2 && bOption2Once)
		{
			m_pButton[1]->SetPos(x, scheme()->GetProportionalScaledValue(yPositions[0]));
			m_pImgButtonOver[1]->SetPos(x, scheme()->GetProportionalScaledValue(yPositions[0]));
			m_pLabelText[1]->SetPos(x, scheme()->GetProportionalScaledValue(yPositions[0]));

			if (bOption3 && bOption3Once)
			{
				m_pButton[2]->SetPos(x, scheme()->GetProportionalScaledValue(yPositions[1]));
				m_pImgButtonOver[2]->SetPos(x, scheme()->GetProportionalScaledValue(yPositions[1]));
				m_pLabelText[2]->SetPos(x, scheme()->GetProportionalScaledValue(yPositions[1]));
			}
		}
		else
		{
			if (bOption3 && bOption3Once)
			{
				m_pButton[2]->SetPos(x, scheme()->GetProportionalScaledValue(yPositions[0]));
				m_pImgButtonOver[2]->SetPos(x, scheme()->GetProportionalScaledValue(yPositions[0]));
				m_pLabelText[2]->SetPos(x, scheme()->GetProportionalScaledValue(yPositions[0]));
			}
		}
	}
	else
	{
		if ((!bOption2 || !bOption2Once) && bOption1 && bOption1Once && bOption3 && bOption3Once)
		{
			m_pButton[2]->SetPos(x, scheme()->GetProportionalScaledValue(yPositions[1]));
			m_pImgButtonOver[2]->SetPos(x, scheme()->GetProportionalScaledValue(yPositions[1]));
			m_pLabelText[2]->SetPos(x, scheme()->GetProportionalScaledValue(yPositions[1]));
		}
	}
}

void CDialogueMenu::OnShowPanel(bool bShow)
{
	ConVar* thirdpers_cvar = cvar->FindVar("cl_thirdperson");
	ConVar* cam_dist = cvar->FindVar("cam_idealdist");
	ConVar* cam_pos = cvar->FindVar("cam_idealdistright");

	GameBaseClient->ShowConsole(false, true, false);

	if (bShow)
	{
		engine->ClientCmd_Unrestricted("gameui_preventescapetoshow\n");

		if (cl_dialoguemode.GetBool())
		{
			cam_pos->SetValue(-15);
			cam_dist->SetValue(35);
			thirdpers_cvar->SetValue(1);
		}
		else
		{
			thirdpers_cvar->SetValue(0);
		}
	}
	else
	{
		engine->ClientCmd_Unrestricted("gameui_allowescapetoshow\n");
		cam_pos->SetValue(0);
		cam_dist->SetValue(155);
		engine->ClientCmd("firstperson\n");
		PerformDefaultLayout();
	}
}

void CDialogueMenu::OnCommand(const char* pcCommand)
{
	for (int i = 0; i <= 2; i++)
	{
		if (!Q_stricmp(pcCommand, VarArgs("Button%i", (i + 1))))
		{
			KeyValues *kvDialogueData = GetDialogueData(szRecentFile);
			if (kvDialogueData == NULL)
				return;

			KeyValues *kvOptionField = kvDialogueData->FindKey(VarArgs("Option%i", (i + 1)));
			if (kvOptionField)
			{
				const char *szEnt = ReadAndAllocStringValue(kvOptionField, "EntityToAffect");
				const char *szAction = ReadAndAllocStringValue(kvOptionField, "ActionOnEntity");
				engine->ClientCmd(VarArgs("ent_fire %s %s\n", szEnt, szAction));

				int iShouldClose = kvOptionField->GetInt("ExitOnClick");
				int iOnlyOnce = kvOptionField->GetInt("OnlyOnce");

				// If this can only be used once, tell our entity to not allow us to use this option again. ( stored on server ent after executing this line... )
				if (iOnlyOnce >= 1)
					engine->ClientCmd(VarArgs("ar_dialogue_option_pressed %i %s\n", i, szRecentTargetEnt));

				if (iShouldClose >= 1)
					GameBaseClient->OpenPanel(4); // Close
			}

			kvDialogueData->deleteThis(); // Clear memory...

			PerformDefaultLayout(); // Reset all...
		}
	}
}