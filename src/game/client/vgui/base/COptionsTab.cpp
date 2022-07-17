
#include "COptionsFrame.h"
#include "COptionsTab.h"
#include "COptionVarItem.h"

#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Button.h>

#include "L4D360UI/L4DBasePanel.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

#define COLOR_NONE Color(0, 0, 0, 0)

#ifdef JSON_ENABLE
static CUtlVectorFixed<Json::Value, 7> _maxjson;

CON_COMMAND( cl_optionsui_json_reload, "Reload the json files for Options UI" )
{
	for ( int i = 0 ; i < 7; i++ )
		_maxjson[i].clear();
	Msg( "Options UI json files has been reloaded.\n" );
}

#endif

COptionsTab::COptionsTab( Panel* parent, const char* szName, const char* szFile, int tabval ) :
	BaseClass( parent, "OptionsTab" )
{
	m_Active = false;
	m_szName = szName;
	m_Label = new vgui::Label(this, "label", szName);
	m_ButttonHidden = new COptionHiddenButton(this, "butt", "", this, "SetActive");
	m_iTabVal = tabval;
#ifdef JSON_ENABLE
	if ( !_maxjson[m_iTabVal].empty() ) return;
	Json::CharReaderBuilder rbuilder;
	std::string strErr;
	std::string strFile = "contagion/cfg/gameoptions/" + std::string( szFile ) + ".json";
	std::ifstream JsonFile( strFile, std::ifstream::binary );
	if ( !Json::parseFromStream( rbuilder, JsonFile, &_maxjson[m_iTabVal], &strErr ) )
		Warning( "Error:\n\t%s failed to load!\n\t%s\n", strFile.c_str(), strErr.c_str() );
#endif
}

COptionsTab::~COptionsTab()
{
}

#ifdef JSON_ENABLE
Json::Value COptionsTab::Get()
{
	return _maxjson[m_iTabVal];
}
#endif

void COptionsTab::SetLabelSize(int w, int t)
{
	m_Label->SetContentAlignment( vgui::Label::Alignment::a_center );
	m_Label->SetSize( w, t );
	m_ButttonHidden->SetSize( w, t );
}

void COptionsTab::Paint()
{
	BaseClass::Paint();
	int w, t;
	GetSize(w, t);

	// Paint background
	DrawBackground(w, t);

	// Draw a nice border at the bottom
	DrawBorder(w, t);
}

void COptionsTab::DrawBackground(int w, int t)
{
	if (m_Active)
		vgui::surface()->DrawSetColor( 150, 150, 150, 160 );
	else
		if ( IsCursorOver() )
			vgui::surface()->DrawSetColor( 100, 100, 100, 160 );
		else
			vgui::surface()->DrawSetColor( 50, 50, 50, 160 );
	vgui::surface()->DrawFilledRect( 0, 0, 0 + w, 0 + t );
}

void COptionsTab::DrawBorder(int w, int t)
{
	// Set border size
	int border_size = 1;
	// Mouse is over, change size
	if ( IsCursorOver() )
		border_size = 2;
	// override if active
	if ( m_Active )
		border_size = 4;

	int border_pos = t - border_size;
	vgui::surface()->DrawSetColor( 150, 25, 25, 220 );
	vgui::surface()->DrawFilledRect( 0, border_pos, 0 + w, border_pos + border_size );
}

void COptionsTab::OnMousePressed(vgui::MouseCode code)
{
	BaseClass::OnMousePressed(code);
	if (code == MOUSE_LEFT)
	{
		COptionsFrame* pFrame = dynamic_cast<COptionsFrame*>(GetParent());
		if (!pFrame) return;
		pFrame->LoadOptionsTab(this, true);
		CONTUI::CL4DBasePanel::GetSingleton().PlayUISound( CONTUI::UISOUND_CLICK );
	}
}

void COptionsTab::ApplySchemeSettings(IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	m_Font = pScheme->GetFont( "GameOptionsTab" );
	m_Label->SetFont( m_Font );
}

void COptionsTab::OnCommand(const char* command)
{
	if ( Q_stricmp( "SetActive", command ) == 0 )
		OnMousePressed(MOUSE_LEFT);
	else
		BaseClass::OnCommand(command);
}

COptionHiddenButton::COptionHiddenButton(Panel* parent, const char* panelName, const char* text, Panel* pActionSignalTarget, const char* pCmd, const char* pCmd2) :
	Button( parent, panelName, text, pActionSignalTarget, pCmd)
{
	m_varitem = false;
	szCmd = pCmd ? pCmd : "";
	szCmd2 = pCmd2 ? pCmd2 : "";
}

COptionHiddenButton::COptionHiddenButton(Panel* parent, const char* panelName, const wchar_t* text, Panel* pActionSignalTarget, const char* pCmd, const char* pCmd2) :
	Button(parent, panelName, text, pActionSignalTarget, pCmd)
{
	m_varitem = false;
	szCmd = pCmd ? pCmd : "";
	szCmd2 = pCmd2 ? pCmd2 : "";
}

void COptionHiddenButton::OnMousePressed(vgui::MouseCode code)
{
	if (m_varitem)
	{
		if (code == MOUSE_LEFT)
		{
			COptionVarItem* pVarItem = dynamic_cast<COptionVarItem*>(GetParent());
			if (!pVarItem) return;
			pVarItem->OnCommand(szCmd.c_str());
		}
		if (code == MOUSE_RIGHT)
		{
			COptionVarItem* pVarItem = dynamic_cast<COptionVarItem*>(GetParent());
			if (!pVarItem) return;
			pVarItem->OnCommand(szCmd2.c_str());
		}
	}
	else
		BaseClass::OnMousePressed(code);
}

void COptionHiddenButton::OnCursorEntered()
{
	BaseClass::OnCursorEntered();
	CONTUI::CL4DBasePanel::GetSingleton().PlayUISound( CONTUI::UISOUND_FOCUS );
}

void COptionHiddenButton::ApplySchemeSettings(vgui::IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	SetAlpha(0);
	SetBgColor(COLOR_NONE);
	SetFgColor(COLOR_NONE);
	SetBorder(nullptr);
}
