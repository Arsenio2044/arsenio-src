#include "cbase.h"
#include "loadingbg.h"
#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

IFileSystem* filesystem = NULL;

#define TFO_COLOR		    Color( 255, 255, 255, 255 )




//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CMapLoadBG::CMapLoadBG( char const *panelName ) : EditablePanel( NULL, panelName )
{
	VPANEL toolParent = enginevgui->GetPanel( PANEL_GAMEUIDLL );
	SetParent( toolParent );



	LoadControlSettings( "resource/loadingdialogbackground.res" );

	m_pBackground = FindControl<ImagePanel>( "LoadingImage", true );

	// Tips / Poems / Conclusions:
	if (m_flTipDisplayTime <= engine->Time())
	{
		m_flTipDisplayTime = engine->Time() + 1.5f;
		SetRandomLoadingTip();
	}

	// Loading Tips
	m_pTextLoadingTip = vgui::SETUP_PANEL(new vgui::Label(this, "TextTip", ""));
	m_pTextLoadingTip->SetZPos(180);
	m_pTextLoadingTip->SetFgColor(TFO_COLOR);
	m_pTextLoadingTip->SetText("");
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

	m_pTextLoadingTip->SetFont(pScheme->GetFont("Arial"));
	m_pTextLoadingTip->SetFgColor(TFO_COLOR);
}

//-----------------------------------------------------------------------------
// Purpose: Sets a new background on demand
//-----------------------------------------------------------------------------
void CMapLoadBG::SetNewBackgroundImage( char const *imageName )
{
	m_pBackground->SetImage( imageName );
}

void CMapLoadBG::SetRandomLoadingTip()
{
	KeyValues* kvLoadingTips = new KeyValues("LoadingTipData");
	if (kvLoadingTips->LoadFromFile(filesystem, "resource/data/settings/Tips.txt", "MOD"))
	{
		int iAmountTips = 0;
		for (KeyValues* sub = kvLoadingTips->GetFirstSubKey(); sub; sub = sub->GetNextKey())
			iAmountTips++;

		KeyValues* kvSelectedTip = kvLoadingTips->FindKey(VarArgs("%i", random->RandomInt(1, iAmountTips)));
		if (kvSelectedTip)
			m_pTextLoadingTip->SetText(kvSelectedTip->GetString());
	}
	else
	{
		m_pTextLoadingTip->SetText("");
	}

	kvLoadingTips->deleteThis();
}
