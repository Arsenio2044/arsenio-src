#include "cbase.h"
#include "loadingbg.h"
#include "vgui_controls/Label.h"
#include "arsenio/arsenio_system_gamemode.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CMapLoadBG::CMapLoadBG( char const *panelName ) : EditablePanel( NULL, panelName )
{
	VPANEL toolParent = enginevgui->GetPanel( PANEL_GAMEUIDLL );
	SetParent( toolParent );

	m_pTipLabel = new Label(this, "TipLabel", "");

	m_pTipLabel->SetText("");






	LoadControlSettings( "resource/loadingdialogbackground.res" );

	m_pBackground = FindControl<ImagePanel>( "LoadingImage", true );
}

void CMapLoadBG::LoadMapData(const char* pMapName)
{
	SetTipPanelText(pMapName);
}


//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CMapLoadBG::~CMapLoadBG()
{
	// None
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CMapLoadBG::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	int iWide, iTall;
	surface()->GetScreenSize( iWide, iTall );
	SetSize( iWide, iTall );
}

//-----------------------------------------------------------------------------
// Purpose: Sets a new background on demand
//-----------------------------------------------------------------------------
void CMapLoadBG::SetNewBackgroundImage( char const *imageName )
{
	m_pBackground->SetImage( imageName );
}

void CMapLoadBG::SetTipPanelText(const char* pMapName)
{
	const auto pFound = g_pGameModeSystem->GetGameModeFromMapName(pMapName);
	const auto eMode = pFound ? pFound->GetType() : GAMEMODE_UNKNOWN;

	m_pTipLabel->SetText(m_TipManager.GetTipForGamemode(eMode));
}