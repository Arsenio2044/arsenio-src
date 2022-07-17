
#include "NewGameOptions.h"
#include "gameui_util.h"
#include "VFlyoutMenu.h"
#include "base/COptionsFrame.h"
#include <vgui/ILocalize.h>
#include "base/COptionsTab.h"
#include <vgui_controls/ImagePanel.h>
#include "videocfg/videocfg.h"
#include "materialsystem/materialsystem_config.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
using namespace CONTUI;

extern bool g_bIsUsingKeyboard;

// Used for our options
static ConVar voice_transmit( "voice_transmit", "100", FCVAR_RELEASE );
static ConVar voice_boost( "voice_boost", "1", FCVAR_RELEASE );
static ConVar ss_playername( "ss_playername", "", FCVAR_USERINFO|FCVAR_ARCHIVE|FCVAR_SERVER_CAN_EXECUTE );

NewGameOptions::NewGameOptions( Panel *parent, const char *panelName ):
	BaseClass(parent, panelName, true, true)
{
	GameUI().PreventEngineHideGameUI();
	SetDeleteSelfOnClose(true);
	SetProportional( true );
	SetTitle("", false);
	SetPaintBackgroundEnabled( false );
	mainframe = new COptionsFrame(this, "MainFrame");
	mainframe->SetPos(60, 60);
	mainframe->SetSize(600, 380);
	m_bStateChanges = false;
	m_bRequiresRestart = false;
}

//=============================================================================
NewGameOptions::~NewGameOptions()
{
	GameUI().AllowEngineHideGameUI();
}

void NewGameOptions::ApplySettings(KeyValues* inResourceData)
{
	BaseClass::ApplySettings(inResourceData);

	m_nTabWide = vgui::scheme()->GetProportionalScaledValue( 80 );
	m_nTabTall = vgui::scheme()->GetProportionalScaledValue( 20 );

	mainframe->SetTabSizes(m_nTabWide, m_nTabTall);
}

void NewGameOptions::ApplySchemeSettings(vgui::IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}

void NewGameOptions::OnCommand( const char* command )
{
	if ( Q_stricmp( "ApplyChangesNotify", command ) == 0 )
	{
		GenericConfirmation* confirmation = static_cast< GenericConfirmation* >( CL4DBasePanel::GetSingleton().OpenWindow( WT_GENERICCONFIRMATION, this, false ) );
		GenericConfirmation::Data_t data;

		data.pWindowTitle = "#CONTUI_NewGameOptions_NewChangesFound";

		std::string szMsg = g_pVGuiLocalizeV2->GetTranslationKey( "#CONTUI_NewGameOptions_NewChangesFound_Msg" );
		data.pMessageText = szMsg.c_str();
		data.bOkButtonEnabled = true;
		data.pfnOkCallback = &IgnoreNewChanges;
		data.bCancelButtonEnabled = true;

		confirmation->SetUsageData(data);
		FlyoutMenu::CloseActiveMenu();
	}
	else if ( Q_stricmp( "ResetController", command ) == 0 )
	{
		GenericConfirmation* confirmation = static_cast< GenericConfirmation* >( CL4DBasePanel::GetSingleton().OpenWindow( WT_GENERICCONFIRMATION, this, false ) );
		GenericConfirmation::Data_t data;

		data.pWindowTitle = "#CONTUI_Controller_Default";
		data.pMessageText = "#GameUI_ControllerSettingsText";
		data.bOkButtonEnabled = true;
		data.pfnOkCallback = &ResetKeyControllerSettings;
		data.bCancelButtonEnabled = true;

		confirmation->SetUsageData(data);
		FlyoutMenu::CloseActiveMenu();
	}
	else if ( Q_stricmp( "ResetKeyBinds", command ) == 0 )
	{
		GenericConfirmation* confirmation = static_cast< GenericConfirmation* >( CL4DBasePanel::GetSingleton().OpenWindow( WT_GENERICCONFIRMATION, this, false ) );
		GenericConfirmation::Data_t data;

		data.pWindowTitle = "#CONTUI_Controller_Default";
		data.pMessageText = "#GameUI_KeyboardSettingsText";
		data.bOkButtonEnabled = true;
		data.pfnOkCallback = &ResetKeyBinds;
		data.bCancelButtonEnabled = true;

		confirmation->SetUsageData(data);
		FlyoutMenu::CloseActiveMenu();
	}
	else if ( Q_stricmp( "ResetCurrentTab", command ) == 0 )
	{
		GenericConfirmation* confirmation = static_cast< GenericConfirmation* >( CL4DBasePanel::GetSingleton().OpenWindow( WT_GENERICCONFIRMATION, this, false ) );
		GenericConfirmation::Data_t data;

		data.pWindowTitle = "#CONTUI_Controller_Default";
		data.pMessageText = "#CONTUI_Controller_Default_Details";
		data.bOkButtonEnabled = true;
		data.pfnOkCallback = &ResetCurrentTab;
		data.bCancelButtonEnabled = true;

		confirmation->SetUsageData(data);
		FlyoutMenu::CloseActiveMenu();
	}
	else if ( Q_stricmp( "ApplyChangesOnBack", command ) == 0 )
	{
		GenericConfirmation* confirmation = static_cast< GenericConfirmation* >( CL4DBasePanel::GetSingleton().OpenWindow( WT_GENERICCONFIRMATION, this, false ) );
		GenericConfirmation::Data_t data;

		data.pWindowTitle = "#CONTUI_NewGameOptions_NewChangesFound";

		std::string szMsg = g_pVGuiLocalizeV2->GetTranslationKey( "#CONTUI_NewGameOptions_NewChangesFound_Msg" );
		data.pMessageText = szMsg.c_str();
		data.bOkButtonEnabled = true;
		data.pfnOkCallback = &IgnoreNewChangesBack;
		data.bCancelButtonEnabled = true;

		confirmation->SetUsageData(data);
		FlyoutMenu::CloseActiveMenu();
	}
	else if ( Q_stricmp( "ChangeToRecommended", command ) == 0 )
	{
		GenericConfirmation* confirmation = static_cast< GenericConfirmation* >( CL4DBasePanel::GetSingleton().OpenWindow( WT_GENERICCONFIRMATION, this, false ) );
		GenericConfirmation::Data_t data;
		data.pWindowTitle = "#CONTUI_UseRecommended_Confirm";
		data.pMessageText = "#CONTUI_UseRecommended_ConfirmMsg";
		data.bOkButtonEnabled = true;
		data.pfnOkCallback = &ApplyRecommended;
		data.bCancelButtonEnabled = true;

		confirmation->SetUsageData(data);
		FlyoutMenu::CloseActiveMenu();
	}
	else if ( Q_stricmp( "3rdPartyCredits", command ) == 0 )
	{
		GenericConfirmation* confirmation = static_cast< GenericConfirmation* >( CL4DBasePanel::GetSingleton().OpenWindow( WT_GENERICCONFIRMATION, this, false ) );
		GenericConfirmation::Data_t data;

		data.pWindowTitle = "#GameUI_ThirdPartyAudio_Title";

		std::string szMsg = g_pVGuiLocalizeV2->GetTranslationKey( "#GameUI_WWise" );
		szMsg += "\n\n";
		szMsg += g_pVGuiLocalizeV2->GetTranslationKey( "#GameUI_Miles_Voice" );
		szMsg += "\n\n";
		szMsg += g_pVGuiLocalizeV2->GetTranslationKey( "#GameUI_Bink" );
		data.pMessageText = szMsg.c_str();
		data.bOkButtonEnabled = true;

		confirmation->SetUsageData(data);
		FlyoutMenu::CloseActiveMenu();
	}
	else if ( Q_stricmp( "Apply", command ) == 0 )
	{
		if ( RequiresRestart() && GetFrame()->HasChangedValues() )
		{
			GenericConfirmation* confirmation = static_cast< GenericConfirmation* >( CL4DBasePanel::GetSingleton().OpenWindow( WT_GENERICCONFIRMATION, this, false ) );
			GenericConfirmation::Data_t data;
			data.pWindowTitle = "#CONTUI_RestartRequired_Confirm";
			data.pMessageText = "#CONTUI_RestartRequired_ConfirmMsg";
			data.bOkButtonEnabled = true;
			data.pfnOkCallback = &ApplyRestart;
			data.bCancelButtonEnabled = true;

			confirmation->SetUsageData(data);
			FlyoutMenu::CloseActiveMenu();
		}
		else
			GetFrame()->ApplyChanges();
	}
	else if ( Q_stricmp( "ApplyForce", command ) == 0 )
		GetFrame()->ApplyChanges();
	else if ( Q_stricmp( "OnBind", command ) == 0 )
		GetFrame()->ReloadOnBind();
	else if ( Q_stricmp( "Reactivate", command ) == 0 )
		GetFrame()->ReloadAndSetTab();
	else if ( Q_stricmp( "Back", command ) == 0 )
	{
		if ( GetFrame()->HasChangedValues() )
			OnCommand( "ApplyChangesOnBack" );
		else
			OnCommand( "Close" );
	}
	else if ( Q_stricmp( "Close", command ) == 0 )
	{
		CL4DBasePanel::GetSingleton().PlayUISound( UISOUND_BACK );
		BaseClass::OnKeyCodePressed( KEY_XBUTTON_B );
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

void NewGameOptions::OnKeyCodePressed(vgui::KeyCode code)
{
	vgui::KeyCode code2 = GetBaseButtonCode( code );
	if ( code2 == KEY_XBUTTON_B && GetFrame()->HasChangedValues() )
	{
		// Don't do this if in-game
		if ( !engine->IsInGame() )
		{
			OnCommand( "ApplyChangesOnBack" );
			return;
		}
	}

	if ( code >= JOYSTICK_FIRST && code <= JOYSTICK_LAST )
		g_bIsUsingKeyboard = false;
	else
		g_bIsUsingKeyboard = true;

	switch ( code2 )
	{
		case KEY_XBUTTON_X:
			OnCommand( "Apply" );
		break;

		case KEY_XBUTTON_A:
		case KEY_ENTER:
			GetFrame()->SelectedItemAction(COptionsFrame::SelectedItemAction_t::Action_Press);
		break;

		case KEY_XBUTTON_LEFT:
		case KEY_LEFT:
			GetFrame()->SelectedItemAction(COptionsFrame::SelectedItemAction_t::Action_Left);
		break;

		case KEY_XBUTTON_RIGHT:
		case KEY_RIGHT:
			GetFrame()->SelectedItemAction(COptionsFrame::SelectedItemAction_t::Action_Right);
		break;

		case KEY_XBUTTON_UP:
		case KEY_UP:
			GetFrame()->SelectNewItem(false);
		break;

		case KEY_XBUTTON_DOWN:
		case KEY_DOWN:
			GetFrame()->SelectNewItem(true);
		break;

		case KEY_Q:
		case KEY_PAGEDOWN:
		case KEY_XBUTTON_LEFT_SHOULDER:
			GetFrame()->SelectNewTab(false);
		break;

		case KEY_E:
		case KEY_PAGEUP:
		case KEY_XBUTTON_RIGHT_SHOULDER:
			GetFrame()->SelectNewTab(true);
		break;
	}
	BaseClass::OnKeyCodePressed(code);
}

void NewGameOptions::OnAspectRatioChanged(int iAspect)
{
	GetFrame()->AspectRatioChanged( iAspect );
}

void NewGameOptions::OnChangeImageFrame(int iFrame)
{
	GetFrame()->ChangeImageFrame( iFrame );
}

void NewGameOptions::OnPhoneBackgroundChanged( int iImage)
{
	GetFrame()->ChangeHelpImage_PhoneBackground( iImage );
}

void NewGameOptions::OnHelpUpdate(const std::string& szName)
{
	GetFrame()->UpdateHelpBox( szName );
}

void NewGameOptions::DeselectAllItems()
{
	GetFrame()->DeselectAllItems();
}

void NewGameOptions::ResetKeyControllerSettings()
{
	NewGameOptions* self = static_cast< NewGameOptions * >( CL4DBasePanel::GetSingleton().GetWindow( WT_OPTIONS_NEW ) );
	if ( self )
	{
		self->GetFrame()->ApplyDefault();
		self->SetValueChanged( false );
		self->SetRequiresRestart( false );
	}
	engine->ClientCmd_Unrestricted( "exec joy_setup" );
	engine->ClientCmd_Unrestricted( "host_writeconfig\n" );
	engine->ClientCmd_Unrestricted( "wait 10;hud_reloadoptions\n" );
}

void NewGameOptions::ResetKeyBinds()
{
	NewGameOptions* self = static_cast< NewGameOptions * >( CL4DBasePanel::GetSingleton().GetWindow( WT_OPTIONS_NEW ) );
	if ( self )
	{
		self->GetFrame()->ApplyDefault();
		self->SetValueChanged( false );
		self->SetRequiresRestart( false );
	}
	engine->ClientCmd_Unrestricted( "exec keybinds_setup" );
	engine->ClientCmd_Unrestricted( "host_writeconfig\n" );
	engine->ClientCmd_Unrestricted( "wait 10;hud_reloadoptions\n" );
}

void NewGameOptions::ResetCurrentTab()
{
	NewGameOptions* self = static_cast< NewGameOptions * >( CL4DBasePanel::GetSingleton().GetWindow( WT_OPTIONS_NEW ) );
	if ( self )
	{
		self->GetFrame()->ApplyDefault();
		self->SetValueChanged( false );
		self->SetRequiresRestart( false );
	}
	engine->ClientCmd_Unrestricted( "wait 10;hud_reloadoptions\n" );
}

void NewGameOptions::IgnoreNewChanges()
{
	NewGameOptions* self = static_cast< NewGameOptions * >( CL4DBasePanel::GetSingleton().GetWindow( WT_OPTIONS_NEW ) );
	if ( self )
		self->GetFrame()->LoadOptionsTab(self->GetFrame()->GetTabChoice(), true);
}

void NewGameOptions::IgnoreNewChangesBack()
{
	NewGameOptions* self = static_cast< NewGameOptions * >( CL4DBasePanel::GetSingleton().GetWindow( WT_OPTIONS_NEW ) );
	if ( self )
		self->OnCommand( "Close" );
}

void NewGameOptions::ApplyRecommended()
{
	NewGameOptions* self = static_cast< NewGameOptions * >( CL4DBasePanel::GetSingleton().GetWindow( WT_OPTIONS_NEW ) );
	if ( self )
	{
		// Force it
		FlyoutMenu::CloseActiveMenu();
		// Apply our values
		self->SetRecommendedSettings();
	}
}

void NewGameOptions::ApplyRestart()
{
	NewGameOptions* self = static_cast< NewGameOptions * >( CL4DBasePanel::GetSingleton().GetWindow( WT_OPTIONS_NEW ) );
	if ( self )
	{
		// Force it
		FlyoutMenu::CloseActiveMenu();
		// Apply our recommended values
		self->OnCommand( "ApplyForce" );
	}
}

#define APPLY_RECOMMENDED_VALUE( _VAL, _DEF ) \
CGameUIConVarRef _VAL( #_VAL ); \
_VAL.SetValue( _DEF );

int GetScreenAspectMode( int width, int height );
void NewGameOptions::SetRecommendedSettings()
{
	KeyValues *pConfigKeys = new KeyValues( "VideoConfig" );
	pConfigKeys->SetKV2( true );
	if ( !pConfigKeys ) return;

	if ( !ReadCurrentVideoConfig( pConfigKeys, true ) )
	{
		pConfigKeys->deleteThis();
		return;
	}

	// Read our default video data
	int iResolutionWidth = pConfigKeys->GetInt( "setting.defaultres", 1280 );
	int iResolutionHeight = pConfigKeys->GetInt( "setting.defaultresheight", 720 );
	bool bWindowed = !pConfigKeys->GetBool( "setting.fullscreen", true );
	bool bNoBorder = pConfigKeys->GetBool( "setting.nowindowborder", false );
	int iModelTextureDetail = clamp( pConfigKeys->GetInt( "setting.gpu_mem_level", 2 ), 0, 2 );
	int iPagedPoolMem = clamp( pConfigKeys->GetInt( "setting.mem_level", 2 ), 0, 2 );
	int nAASamples = pConfigKeys->GetInt( "setting.mat_antialias", 0 );
	int nAAQuality = pConfigKeys->GetInt( "setting.mat_aaquality", 0 );
	int iFiltering = pConfigKeys->GetInt( "setting.mat_forceaniso", 1 );
	bool bVSync = pConfigKeys->GetBool( "setting.mat_vsync", true );
	bool bTripleBuffered = pConfigKeys->GetBool( "setting.mat_triplebuffered", false );
	int iGPUDetail = pConfigKeys->GetInt( "setting.gpu_level", 0 );
	int iCPUDetail = pConfigKeys->GetInt( "setting.cpu_level", 0 );
	int iQueuedMode = pConfigKeys->GetInt( "setting.mat_queue_mode", -1 );

	// Delete our keyvalues, we don't need em anymore
	pConfigKeys->deleteThis();

	// Apply default values for this video tab
	GetFrame()->ApplyDefault();
	SetValueChanged( false );
	SetRequiresRestart( false );

	// Apply our stuff
	APPLY_RECOMMENDED_VALUE( gpu_level, iGPUDetail );
	APPLY_RECOMMENDED_VALUE( cpu_level, iCPUDetail );
	APPLY_RECOMMENDED_VALUE( mat_queue_mode, iQueuedMode );
	APPLY_RECOMMENDED_VALUE( mat_antialias, nAASamples );
	APPLY_RECOMMENDED_VALUE( mat_aaquality, nAAQuality );
	APPLY_RECOMMENDED_VALUE( gpu_mem_level, iModelTextureDetail );
	APPLY_RECOMMENDED_VALUE( mem_level, iPagedPoolMem );
	APPLY_RECOMMENDED_VALUE( mat_forceaniso, iFiltering );
	APPLY_RECOMMENDED_VALUE( mat_vsync, bVSync );
	APPLY_RECOMMENDED_VALUE( mat_triplebuffered, bTripleBuffered );

	char szCmd[ 256 ];
	Q_snprintf( szCmd, sizeof( szCmd ), "mat_setvideomode %i %i %i %i\n", iResolutionWidth, iResolutionHeight, bWindowed ? 1 : 0, bNoBorder ? 1 : 0 );
	engine->ClientCmd_Unrestricted( szCmd );

	// Save and write our changes
	engine->ClientCmd_Unrestricted( "mat_savechanges\n" );
	engine->ClientCmd_Unrestricted( "host_writeconfig\n" );
	engine->ClientCmd_Unrestricted( "wait 5;hud_reloadscheme;wait 5;hud_reloadoptions\n" );
}
