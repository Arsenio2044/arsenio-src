#include "cbase.h"
#include "loadingbg.h"

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

	LoadControlSettings( "resource/loadingdialogbackground.res" );

	m_pBackground = FindControl<ImagePanel>( "LoadingImage", true );
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