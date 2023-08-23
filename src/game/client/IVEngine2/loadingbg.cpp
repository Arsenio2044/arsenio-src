#include "cbase.h"
#include "loadingbg.h"
#include <vector> // Add this include for using std::vector
#include <string> // Add this include for using std::string
#include <vgui/IVGui.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
using namespace std; // Add this to use std namespace


//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CMapLoadBG::CMapLoadBG(char const* panelName) : EditablePanel(NULL, panelName)
{
	VPANEL toolParent = enginevgui->GetPanel(PANEL_GAMEUIDLL);
	SetParent(toolParent);

	LoadControlSettings("resource/loadingdialogbackground.res");

	m_pBackground = FindControl<ImagePanel>("LoadingImage", true);

	// Initialize the timer with a 5-second interval
	vgui::ivgui()->AddTickSignal(GetVPanel(), 5000); // 5000 milliseconds = 5 seconds
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CMapLoadBG::~CMapLoadBG()
{
	// None
}

// Add an array of loading screen tips
const char* g_LoadingScreenTips[] =
{
	"Tip 1: This is loading tip 1.",
	"Tip 2: This is loading tip 2.",
	"Tip 3: This is loading tip 3.",
	// Add more tips as needed
};

//-----------------------------------------------------------------------------
// Purpose: Sets a new background on demand
//-----------------------------------------------------------------------------
void CMapLoadBG::SetNewBackgroundImage(char const* imageName)
{
	m_pBackground->SetImage(imageName);

	// Set a random tip text from the array
	int numTips = sizeof(g_LoadingScreenTips) / sizeof(g_LoadingScreenTips[0]);
	int randomIndex = rand() % numTips;
	const char* randomTip = g_LoadingScreenTips[randomIndex];

	// Set the tip text in the resource file with the control name "LoadingTipLabel"
	SetDialogVariable("LoadingTipLabel", randomTip);
}


//-----------------------------------------------------------------------------
// Purpose: Tick signal handler
//-----------------------------------------------------------------------------
void CMapLoadBG::OnTick()
{
	// Change the background and tip every 5 seconds
	SetNewBackgroundImage("path/to/your/new/background/image");
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CMapLoadBG::ApplySchemeSettings(IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	int iWide, iTall;
	surface()->GetScreenSize(iWide, iTall);
	SetSize(iWide, iTall);
}

