
#include "COptionsFrame.h"
#include "COptionsTab.h"
#include "COptionVarList.h"
#include "COptionVarItem.h"
#include "COptionVarHelp.h"

#include <vgui/ISurface.h>
#include <vgui_controls/Image.h>
#include <vgui_controls/TextEntry.h>
#include <vgui_controls/CheckButton.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/Button.h>

#include "L4D360UI/NewGameOptions.h"

#include "videocfg/videocfg.h"
#include "materialsystem/materialsystem_config.h"
#include "EngineInterface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

extern int g_m_AspectRatio;
int GetScreenAspectMode( int width, int height );

COptionsFrame::COptionsFrame( Panel* parent, const char* panelName ) :
	BaseClass( parent, panelName )
{
	m_iItemSelected = -1;
	m_TabChoice = nullptr;
	m_nTabWide = 100;
	m_nTabTall = 24;
	frame_info = new COptionVarHelp(this, "VarHelp");
	BuildOptionsTabs();
}

COptionsFrame::~COptionsFrame()
{
	ClearVectors();
}

void COptionsFrame::BuildOptionsTabs()
{
	for ( int i = 0; i < MAX_TABS; i++ )
	{
		frame_val.AddToTail(new COptionVarList(this, VarArgs( "VarList%i", i )));
		frame_val[i]->SetVisible(false);
	}

	CreateTab("#CONTUI_NewGameOptions_Tab_Audio", "audio", 0);
	CreateTab("#CONTUI_NewGameOptions_Tab_Video", "video", 1);
	CreateTab("#CONTUI_NewGameOptions_Tab_Mouse", "mouse", 2);
	CreateTab("#CONTUI_NewGameOptions_Tab_Keyboard", "keyboard", 3);
	CreateTab("#CONTUI_NewGameOptions_Tab_Controller", "controller", 4);
	CreateTab("#CONTUI_NewGameOptions_Tab_Splitscreen", "splitscreen", 5);
	CreateTab("#CONTUI_NewGameOptions_Tab_Multiplayer", "multiplayer", 6);

	// Load default tab
	LoadOptionsTab(tabs[0], true);
}

void COptionsFrame::CreateTab(const char* name, const char* szFile, int tab_val)
{
	COptionsTab *tab = new COptionsTab(this, name, szFile, tab_val);
	int val = tabs.AddToTail(tab);

	if ( tab->Get().empty() ) return;
	std::vector<std::string> membergroups = tab->Get().getMemberNames();

	Panel *objParent = frame_val[val];
	COptionVarItem* pItem;

	for ( auto groupname : membergroups )
	{
		if ( groupname == "" ) return;
		const Json::Value data = tab->Get()[ groupname ];

		const std::string strID = data.get( "id", "0" ).asString();
		const std::string strLabel = data.get( "string", "" ).asString();
		const std::string strConvar = data.get( "convar", "" ).asString();
		const std::string strDefault = data.get( "default", "" ).asString();
		const std::string strVideo = data.get( "video", "" ).asString();
		const std::string strHelp = data.get( "help", "" ).asString();
		const std::string strHelpImg = data.get( "help_img", "" ).asString();
		const std::string strHelpVid = data.get( "help_vid", "" ).asString();	// If found, and valid, it will replace the image
		const std::string strCommand = data.get( "command", "" ).asString();

		const std::string strTitle = data.get( "title", "0" ).asString();
		const std::string strSS = data.get( "splitscreen", "0" ).asString();
		const std::string strRestart = data.get( "restart", "0" ).asString();
		const std::string strKeyBind = data.get( "bind", "" ).asString();
		const std::string strControllerBind = data.get( "controller", "" ).asString();
		const std::string strSpecial = data.get( "special", "" ).asString();

		// Sliders
		const std::string strSliderMin = data.get("minValue", "").asString();
		const std::string strSliderMax = data.get("maxValue", "").asString();
		const std::string strSliderStep = data.get("stepSize", "").asString();
		const std::string strSliderFrmt1 = data.get("DisplayValue", "0").asString();
		const std::string strSliderFrmt2 = data.get("DisplayInFloat", "0").asString();
		const std::string strSliderFrmt2b = data.get("inverseFill", "0").asString();
		const std::string strSliderFrmt3 = data.get("FloatFormat", "0").asString();

		// Custom selector size
		const std::string strDivider = data.get("divide", "0").asString();

		COptionVarData *pCtrl = new COptionVarData( objParent, strID.c_str() );

		auto cvar_ref = g_pCVar->FindVar( strConvar.c_str() );

		// Set our video
		pCtrl->m_video = strVideo;

		// Set our help text
		pCtrl->m_cvar = strConvar;
		pCtrl->m_help = strHelp;
		pCtrl->m_help_img = strHelpImg;
		pCtrl->m_help_vid = strHelpVid;

		pItem = new COptionVarItem( pCtrl, strID.c_str(), strLabel.c_str(), GetParent(), strCommand.c_str() );

		Json::Value jVal = data[ "values" ];
		if ( !jVal.empty() )
		{
			std::vector<std::string> valuesgroups = jVal.getMemberNames();
			for ( auto value : valuesgroups )
			{
				if ( value == "" ) return;
				pItem->AddItem( jVal.get( value, "" ).asCString(), value );
			}
		}
		// Set our current value!
		pItem->SetValueText( strDefault.c_str() );
		pItem->SetValue( cvar_ref ? cvar_ref->GetInt() : 0 );	// Overrides default value, if it exists
		pItem->CheckConvar( cvar_ref );
		pItem->SetCvar( cvar_ref ? strConvar.c_str() : "" );

		pItem->SetSpecial( strSpecial, strConvar );
		pItem->SetController( strControllerBind );
		pItem->SetKeyBind( strKeyBind );
		pItem->SetTitle( std::stoi( strTitle ) == 1, std::stoi( strSS ) == 1 );
		pItem->SetRestart( std::stoi( strRestart ) == 1 );
		pItem->SetSlider( strSliderMin, strSliderMax, strSliderStep, strConvar, std::stoi( strSliderFrmt1 ) == 1, std::stoi( strSliderFrmt2 ) == 1, std::stoi( strSliderFrmt2b ) == 1, std::stoi( strSliderFrmt3 ), std::stoi( strRestart ) == 1 );
		pItem->SetCustomDivide( std::stoi( strDivider ) );

		pCtrl->pControl = (Panel *)pItem;

		pCtrl->SetSize( GetWide(), vgui::scheme()->GetProportionalScaledValue( 25 ) );
		frame_val[val]->AddItem( pCtrl );
	}
}

void COptionsFrame::Paint()
{
	BaseClass::Paint();
	int w, t;
	GetSize( w, t );
	// Paint background
	vgui::surface()->DrawSetColor( 15, 15, 15, 160 );
	vgui::surface()->DrawFilledRect( 0, 0, 0 + w, 0 + t );

	// Paint tabs
	int xPos = 0;
	for each (COptionsTab* tab in tabs)
	{
		tab->SetSize(m_nTabWide, m_nTabTall);
		tab->SetLabelSize(m_nTabWide, m_nTabTall);
		tab->SetPos(xPos, 0);
		xPos += m_nTabWide;
	}

	// Set the frames
	int wide = w / 2;
	int tall = t - m_nTabTall - 5;
	frame_val[m_TabChoice->GetTabValue()]->SetPos(0, m_nTabTall + 5);
	frame_val[m_TabChoice->GetTabValue()]->SetSize(wide, tall);

	frame_info->SetPos(wide, m_nTabTall + 5);
	frame_info->SetSize(wide, tall);
}

void COptionsFrame::LoadOptionsTab( COptionsTab* tab, bool bIsTabClick )
{
	m_TabChoice = tab;

	// Deactivate all
	for each (COptionsTab * item in tabs)
	{
		item->SetActive(false);
		HideAllItems(item);
	}

	// Set our aspect ratio, for our little button
	const MaterialSystem_Config_t &config = materials->GetCurrentConfigForVideoCard();
	g_m_AspectRatio = GetScreenAspectMode( config.m_VideoMode.m_Width, config.m_VideoMode.m_Height );

	frame_val[m_TabChoice->GetTabValue()]->SetVisible(true);	// Make sure we are visible
	frame_val[m_TabChoice->GetTabValue()]->MoveToFront();		// Make sure this is in front
	tab->SetActive(true); // Activate this

	m_iItemSelected = -1;

	// Reload our vars
	for (int i = 0; i < frame_val[m_TabChoice->GetTabValue()]->GetItemCount(); i++)
	{
		COptionVarData *mp = dynamic_cast<COptionVarData*>(frame_val[m_TabChoice->GetTabValue()]->GetItem(i));
		if ( !mp ) continue;
		COptionVarItem *pValue = dynamic_cast<COptionVarItem*>(mp->pControl);
		if (!pValue) continue;
		// Default our selected state
		pValue->SetSelected(COptionVarItem::SelectedState_t::STATE_NONE);
		// Before we draw this, destroy our previous label, and create a new one.
		pValue->RebuildLabel();
		// Recheck our binds, if we applied new ones
		pValue->GrabButtonBind();
	}

	// New tab!
	if ( bIsTabClick )
	{
		// Make sure we scroll all the way up
		frame_val[m_TabChoice->GetTabValue()]->ResetScrollBarPos();
		// Reset this
		UpdateHelpBox( "" );
	}
}

void COptionsFrame::SetTabSizes(int w, int t)
{
	m_nTabWide = w;
	m_nTabTall = t;
}

bool COptionsFrame::HasChangedValues()
{
	CONTUI::NewGameOptions* self = static_cast< CONTUI::NewGameOptions * >( CONTUI::CL4DBasePanel::GetSingleton().GetWindow( CONTUI::WT_OPTIONS_NEW ) );
	if ( self )
		return self->IsValueChanged();
	return false;
}

bool COptionsFrame::RequiresRestart()
{
	CONTUI::NewGameOptions* self = static_cast< CONTUI::NewGameOptions * >( CONTUI::CL4DBasePanel::GetSingleton().GetWindow( CONTUI::WT_OPTIONS_NEW ) );
	if ( self )
		return self->RequiresRestart();
	return false;
}

void COptionsFrame::ApplyChanges()
{
	// We changed nothing don't apply anything
	if ( !HasChangedValues() ) return;

	if ( RequiresRestart() && HasChangedValues() )
	{
		// If connected to a server, disconnect.
		INetChannelInfo *nci = engine->GetNetChannelInfo();
		if ( nci )
		{
			// Only retry if we're not running the server
			const char *pAddr = nci->GetAddress();
			if ( pAddr )
				engine->ClientCmd_Unrestricted( "disconnect\n" );
		}
	}

	int wide = 0;
	int tall = 0;
	int displaymode = GetDisplayMode();
	bool bWindowed = displaymode >= 1;
	bool bNoBorder = displaymode == 2;

	for (int i = 0; i < frame_val[m_TabChoice->GetTabValue()]->GetItemCount(); i++)
	{
		COptionVarData *mp = dynamic_cast<COptionVarData*>(frame_val[m_TabChoice->GetTabValue()]->GetItem(i));
		if ( !mp ) continue;
		COptionVarItem *pValue = dynamic_cast<COptionVarItem*>(mp->pControl);
		if (!pValue) continue;
		pValue->ApplyChanges(mp->m_cvar.c_str());
		if (!pValue->IsSpecial("video_res")) continue;
		pValue->GetResSize(wide, tall);
	}

	// Specials
	if ( wide > 0 && tall > 0 )
	{
		const MaterialSystem_Config_t &config = materials->GetCurrentConfigForVideoCard();
		if ( config.m_VideoMode.m_Width != wide || config.m_VideoMode.m_Height != tall || config.Windowed() != bWindowed || config.NoWindowBorder() != bNoBorder )
		{
			// set mode
			char szCmd[ 256 ];
			Q_snprintf( szCmd, sizeof( szCmd ), "mat_setvideomode %i %i %i %i\n", wide, tall, bWindowed ? 1 : 0, bNoBorder ? 1 : 0 );
			engine->ClientCmd_Unrestricted( szCmd );
			engine->ClientCmd_Unrestricted( "wait 5;hud_reloadscheme\n" );
		}
		// Update the current video config file.
		int nAspectRatioMode = GetScreenAspectMode( config.m_VideoMode.m_Width, config.m_VideoMode.m_Height );
		UpdateCurrentVideoConfig( config.m_VideoMode.m_Width, config.m_VideoMode.m_Height, nAspectRatioMode, !config.Windowed(), config.NoWindowBorder() );
	}

	// apply changes
	if ( !RequiresRestart() && HasChangedValues() )
		engine->ClientCmd_Unrestricted( "mat_savechanges\n" );

	// Splitscreen user does not have its own config file. :V
	engine->ClientCmd_Unrestricted( "host_writeconfig\n" );
	engine->ClientCmd_Unrestricted( "host_writeconfig_ss 1\n" );

	// Reload our UI after we saved
	if ( RequiresRestart() && HasChangedValues() )
		engine->ClientCmd_Unrestricted( "wait 5;_apprestart\n" );

	CONTUI::NewGameOptions* self = static_cast< CONTUI::NewGameOptions * >( CONTUI::CL4DBasePanel::GetSingleton().GetWindow( CONTUI::WT_OPTIONS_NEW ) );
	if ( self )
	{
		self->SetValueChanged( false );
		self->SetRequiresRestart( false );
	}
}

void COptionsFrame::ApplyDefault()
{
	for (int i = 0; i < frame_val[m_TabChoice->GetTabValue()]->GetItemCount(); i++)
	{
		COptionVarData *mp = dynamic_cast<COptionVarData*>(frame_val[m_TabChoice->GetTabValue()]->GetItem(i));
		if ( !mp ) continue;
		COptionVarItem *pValue = dynamic_cast<COptionVarItem*>(mp->pControl);
		if (!pValue) continue;
		pValue->ApplyDefault(mp->m_cvar.c_str());
	}
}

void COptionsFrame::AspectRatioChanged(int asp)
{
	bool bWindow = GetDisplayMode() == 1;
	for (int i = 0; i < frame_val[m_TabChoice->GetTabValue()]->GetItemCount(); i++)
	{
		COptionVarData *mp = dynamic_cast<COptionVarData*>(frame_val[m_TabChoice->GetTabValue()]->GetItem(i));
		if ( !mp ) continue;
		COptionVarItem *pValue = dynamic_cast<COptionVarItem*>(mp->pControl);
		if (!pValue) continue;
		if (!pValue->IsSpecial("video_res")) continue;
		// Apply New Items
		pValue->SetResolutionList(asp, bWindow);
	}
}

void COptionsFrame::ChangeImageFrame(int frame)
{
	frame_info->SetImageFrame( frame );
}

void COptionsFrame::ChangeHelpImage_PhoneBackground(int img)
{
	// Same as UpdateHelpBox, except, we only update image
	UpdateImage( CFmtStr( "objective/background%s%i", (((img+1) < 10) ? "0" : ""), img+1 ) );
}

void COptionsFrame::UpdateHelpBox(const std::string& szName)
{
	if ( szName == "" )
	{
		frame_info->SetText( "" );
		frame_info->SetVideo( nullptr );
		frame_info->SetImage( nullptr );
		return;
	}

	for (int i = 0; i < frame_val[m_TabChoice->GetTabValue()]->GetItemCount(); i++)
	{
		COptionVarData *mp = dynamic_cast<COptionVarData*>(frame_val[m_TabChoice->GetTabValue()]->GetItem(i));
		if ( !mp ) continue;
		if (!FStrEq(mp->GetName(), szName.c_str())) continue;
		COptionVarItem *pValue = dynamic_cast<COptionVarItem*>(mp->pControl);
		if (!pValue) continue;
		m_iItemSelected = i;
		frame_info->SetText( mp->m_help );
		frame_info->SetVideo( mp->m_help_vid.c_str() );
		frame_info->SetImage( mp->m_help_img.c_str() );
		break;
	}
}

void COptionsFrame::DeselectAllItems()
{
	for (int i = 0; i < frame_val[m_TabChoice->GetTabValue()]->GetItemCount(); i++)
	{
		COptionVarData *mp = dynamic_cast<COptionVarData*>(frame_val[m_TabChoice->GetTabValue()]->GetItem(i));
		if ( !mp ) continue;
		COptionVarItem *pValue = dynamic_cast<COptionVarItem*>(mp->pControl);
		if (!pValue) continue;
		if (!pValue->IsInButtonState()) continue;
		pValue->SetSelected(COptionVarItem::SelectedState_t::STATE_NONE);
	}
}

void COptionsFrame::DeselectHoverItem()
{
	for (int i = 0; i < frame_val[m_TabChoice->GetTabValue()]->GetItemCount(); i++)
	{
		COptionVarData *mp = dynamic_cast<COptionVarData*>(frame_val[m_TabChoice->GetTabValue()]->GetItem(i));
		if ( !mp ) continue;
		COptionVarItem *pValue = dynamic_cast<COptionVarItem*>(mp->pControl);
		if (!pValue) continue;
		if (pValue->GetSelected() != COptionVarItem::SelectedState_t::STATE_HOVER) continue;
		pValue->SetSelected(COptionVarItem::SelectedState_t::STATE_NONE);
	}
}

void COptionsFrame::SelectedItemAction(SelectedItemAction_t state)
{
	CONTUI::CL4DBasePanel::GetSingleton().PlayUISound( CONTUI::UISOUND_CLICK );

	COptionVarData* mp = dynamic_cast<COptionVarData*>(frame_val[m_TabChoice->GetTabValue()]->GetItem(m_iItemSelected));
	if (!mp) return;
	COptionVarItem* pValue = dynamic_cast<COptionVarItem*>(mp->pControl);
	if (!pValue) return;
	switch ( state )
	{
		case Action_Press:
			pValue->SetInputState(COptionVarItem::ButtonInputState_t::BUTTON_PRESS);
		break;

		case Action_Left:
			pValue->SetInputState(COptionVarItem::ButtonInputState_t::BUTTON_LEFT);
		break;

		case Action_Right:
			pValue->SetInputState(COptionVarItem::ButtonInputState_t::BUTTON_RIGHT);
		break;
	}
}

void COptionsFrame::SelectNewItem(bool next)
{
	if ( !CanSelectNewItemTab() ) return;
	DeselectHoverItem();
	int iSelected = GetNextAvailableItem(next);
	if ( iSelected == -1 )
	{
		SelectItem( 0 );
		return;
	}
	SelectItem( iSelected );
}

void COptionsFrame::SelectItem(int item)
{
	// Clamp it
	item = clamp(item, 0, frame_val[m_TabChoice->GetTabValue()]->GetItemCount() - 1);
	// Select it
	COptionVarData *mp = dynamic_cast<COptionVarData*>(frame_val[m_TabChoice->GetTabValue()]->GetItem(item));
	if ( !mp ) return;
	COptionVarItem *pValue = dynamic_cast<COptionVarItem*>(mp->pControl);
	if (!pValue) return;
	pValue->SetSelected(COptionVarItem::SelectedState_t::STATE_BUTTON);
	CONTUI::CL4DBasePanel::GetSingleton().PlayUISound( CONTUI::UISOUND_HOVER );
	m_iItemSelected = item;
	// Make sure to scroll here!
	frame_val[m_TabChoice->GetTabValue()]->ScrollToPos(item);
}

bool COptionsFrame::CanSelectNewItemTab()
{
	for (int i = 0; i < frame_val[m_TabChoice->GetTabValue()]->GetItemCount(); i++)
	{
		COptionVarData *mp = dynamic_cast<COptionVarData*>(frame_val[m_TabChoice->GetTabValue()]->GetItem(i));
		if ( !mp ) continue;
		COptionVarItem *pValue = dynamic_cast<COptionVarItem*>(mp->pControl);
		if (!pValue) continue;
		if (pValue->IsBindingKey()) return false;
		if (pValue->IsTyping()) return false;
	}
	return true;
}

void COptionsFrame::SelectNewTab(bool next)
{
	if ( !CanSelectNewItemTab() ) return;
	CONTUI::CL4DBasePanel::GetSingleton().PlayUISound( CONTUI::UISOUND_CLICK );
	int y = 0;
	for (int i = 0; i < tabs.Count(); i++)
	{
		COptionsTab* tab = tabs[i];
		if (!tab) continue;
		if (m_TabChoice == tab)
		{
			if (next)
				y = i + 1;
			else
				y = i - 1;
			break;
		}
	}
	// Clamp it
	y = clamp( y, 0, tabs.Count() - 1 );
	LoadOptionsTab( tabs[y], true );
}

void COptionsFrame::ReloadCurrentTab()
{
	LoadOptionsTab( m_TabChoice, false );
}

void COptionsFrame::HideAllItems(COptionsTab* item)
{
	frame_val[item->GetTabValue()]->SetVisible(true);
	frame_val[item->GetTabValue()]->SetSize(0, 0);	// Don't draw this
}

void COptionsFrame::ReloadAndSetTab()
{
	int saved_tab = m_TabChoice->GetTabValue();
	for each (COptionsTab * item in tabs)
	{
		item->SetActive(false);
		HideAllItems(item);
	}
	ClearVectors();
	BuildOptionsTabs();
	LoadOptionsTab(tabs[saved_tab], true);
}

void COptionsFrame::ReloadOnBind()
{
	int saved_tab = m_TabChoice->GetTabValue();
	LoadOptionsTab(tabs[saved_tab], false);
}

bool COptionsFrame::CheckIfTitle(int item)
{
	if (item == -1) return false;
	COptionVarData* mp = dynamic_cast<COptionVarData*>(frame_val[m_TabChoice->GetTabValue()]->GetItem(item));
	if (!mp) return false;
	COptionVarItem* pValue = dynamic_cast<COptionVarItem*>(mp->pControl);
	if (!pValue) return false;
	return pValue->IsTitle();
}

int COptionsFrame::GetNextAvailableItem(bool next)
{
	// Don't care if we are -1
	if ( m_iItemSelected == -1 ) return -1;
	int temp = -1;
	int y = -1;
	// Loop trough from current item selection
	for (int i = m_iItemSelected; i < frame_val[m_TabChoice->GetTabValue()]->GetItemCount(); i++)
	{
		COptionVarData* mp = dynamic_cast<COptionVarData*>(frame_val[m_TabChoice->GetTabValue()]->GetItem(i));
		if (!mp) continue;
		COptionVarItem* pValue = dynamic_cast<COptionVarItem*>(mp->pControl);
		if (!pValue) continue;
		pValue->SetSelected(COptionVarItem::SelectedState_t::STATE_NONE);
		if (next) temp = m_iItemSelected + 1;
		else temp = m_iItemSelected - 1;
		y = next ? i+1 : i-1;
		break;
	}

	// if title, then add another.
	if (CheckIfTitle(y))
	{
		if (next) temp++;
		else temp--;
	}

	return temp;
}

void COptionsFrame::UpdateImage(const char* szImage)
{
	frame_info->SetImage( szImage, true );
}

void COptionsFrame::ClearVectors()
{
	tabs.Purge();
	frame_val.Purge();
}

int COptionsFrame::GetDisplayMode()
{
	for (int i = 0; i < frame_val[m_TabChoice->GetTabValue()]->GetItemCount(); i++)
	{
		COptionVarData *mp = dynamic_cast<COptionVarData*>(frame_val[m_TabChoice->GetTabValue()]->GetItem(i));
		if ( !mp ) continue;
		COptionVarItem *pValue = dynamic_cast<COptionVarItem*>(mp->pControl);
		if (!pValue) continue;
		if (!pValue->IsSpecial("video_disp")) continue;
		return pValue->GetValue();
	}
	return 0;
}

CON_COMMAND( hud_reloadoptions, "Reloads Options UI" )
{
	CONTUI::NewGameOptions* self = static_cast< CONTUI::NewGameOptions * >( CONTUI::CL4DBasePanel::GetSingleton().GetWindow( CONTUI::WT_OPTIONS_NEW ) );
	if ( self )
	{
		if ( FStrEq( args[1], "1" ) )
			self->OnCommand( "OnBind" );
		else
			self->OnCommand( "Reactivate" );
	}
}