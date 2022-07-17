
#include "COptionVarItem.h"
#include "COptionsTab.h"
#include "COptionSlider.h"
#include "L4D360UI/NewGameOptions.h"

#include "language.h"

#include <vgui/IInput.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>

#include <vgui_controls/ProgressBar.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/TextEntry.h>

#include "game/client/IGameClientExports.h"
#include "inputsystem/iinputsystem.h"
#include "ivoicetweak.h"
#include "materialsystem/materialsystem_config.h"
#include "IGameUIFuncs.h"
#include "videocfg/videocfg.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

int GetScreenAspectMode( int width, int height );
void GetResolutionName( vmode_t *mode, char *sz, int sizeofsz );

int s_ArrowLeft = -1;
int s_ArrowRight = -1;

IVoiceTweak* g_pEngineVoiceTweak = nullptr;		// Engine voice tweak API.

#define DEFINE_ARROW( _CAM, _ID ) \
if ( _ID == -1 ) \
{ \
	if ( const char *szRes = GetLangTextureFile( "materials", _CAM, ".vtf" ) ) \
	{ \
		_ID = vgui::surface()->DrawGetTextureId( szRes ); \
		if ( _ID == -1 ) \
		{ \
			_ID = vgui::surface()->CreateNewTextureID(); \
			vgui::surface()->DrawSetTextureFile( _ID, szRes, true, false ); \
		} \
	} \
}

struct AAMode_t
{
	int m_nNumSamples;
	int m_nQualityLevel;
};

int g_m_PreviewTune = 0;
int g_m_AspectRatio = -1;	// Global value, so we can place the button properly
int g_m_nNumAAModes;
AAMode_t g_m_nAAModes[16];
bool g_mHasAppliedAAModes = false;

// A fix for applying AA on the same value (causes the game to just lock up)
static ConVar mat_aasetting( "mat_aasetting", "0" );

void Add_AA_Mode( int sample, int quality )
{
	if ( g_mHasAppliedAAModes ) return;
	g_m_nAAModes[g_m_nNumAAModes].m_nNumSamples = sample;
	g_m_nAAModes[g_m_nNumAAModes].m_nQualityLevel = quality;
	g_m_nNumAAModes++;
}

int FindMSAAMode( int nAASamples, int nAAQuality )
{
	DevMsg( 1, "FindMSAAMode( {green}%i{default}, {cyan}%i{white} )\n", nAASamples, nAAQuality );
	// Run through the AA Modes supported by the device
    for ( int nAAMode = 0; nAAMode < g_m_nNumAAModes; nAAMode++ )
	{
		// If we found the mode that matches what we're looking for, return the index
		if ( ( g_m_nAAModes[nAAMode].m_nNumSamples == nAASamples ) && ( g_m_nAAModes[nAAMode].m_nQualityLevel == nAAQuality ) )
		{
			DevMsg( 1, "return {green}%i{default};\n", nAAMode );
			return nAAMode;
		}
	}
	return 0;	// Didn't find what we're looking for, so no AA
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
COptionVarItem::COptionVarItem( Panel *parent, char const *panelName, const char* szLabel, vgui::Panel* pActionSignalTarget, const char* szCommand ) : Panel( parent, panelName )
{
	if ( !g_pEngineVoiceTweak )
		g_pEngineVoiceTweak = engine->GetVoiceTweakAPI();
	m_iSelectedState = STATE_NONE;
	m_Active = false;
	m_bTitle = false;
	m_RestartRequired = false;
	m_bInCaptureMode = false;
	m_szKeyBind = "";
	m_szController = "";
	m_szCvar = "";
	m_szSpecial = "";
	m_flRefreshingTime = 0.0f;
	m_bcButton = BUTTON_CODE_INVALID;

	m_pMicMeter = nullptr;
	m_pMicMeter2 = nullptr;
	m_pMicMeterIndicator = nullptr;

	m_pStringInput = new TextEntry( this, "Input" );
	m_pStringInput->SetVisible( false );

	SetBgColor( Color(35, 35, 35, 160) );

	m_label_text = new Label( this, "label1", szLabel );
//	m_label_text->SetAllCaps( true );
	m_label_value = new Label( this, "label2", "Default" );

	pHiddenButton = new COptionHiddenButton( this, "butt", "", pActionSignalTarget, szCommand );
	pChoicesButton = new COptionHiddenButton( this, "choice", "", this, "Increase", "Decrease" );
	pChoicesButton->SetVarItem( true );

	pCamRight = new COptionHiddenButton( this, "arr_r", "", this, "Increase" );
	pCamRight->SetVarItem( true );

	pCamLeft = new COptionHiddenButton( this, "arr_l", "", this, "Decrease" );
	pCamLeft->SetVarItem( true );

	pHiddenButton->SetPos(0, 0);
	pHiddenButton->SetSize(0, 0);

	pChoicesButton->SetPos(0, 0);
	pChoicesButton->SetSize(0, 0);

	pCamRight->SetPos(0, 0);
	pCamRight->SetSize(0, 0);

	pCamLeft->SetPos(0, 0);
	pCamLeft->SetSize(0, 0);

	pSlider = nullptr;

	m_label_value->SetPos(0, 0);
	m_label_value->SetSize(0, 0);

	m_label_text->SetContentAlignment( Label::Alignment::a_west );
	m_label_value->SetContentAlignment( Label::Alignment::a_west );

	m_iCurrent = 0;

	m_sx = 0;
	m_sy = 0;
}

COptionVarItem::~COptionVarItem()
{
	if ( g_pEngineVoiceTweak && g_pEngineVoiceTweak->IsStillTweaking() )
		g_pEngineVoiceTweak->EndVoiceTweakMode();
}

void COptionVarItem::SetValue(int input)
{
	int max = m_list.Count();
	if (max == 0) return;
	m_iCurrent = GetSelectedValue( input );
	if ( m_label_value )
		m_label_value->SetText(GetCurrentName(clamp(m_iCurrent, 0, max - 1)).c_str());
}

void COptionVarItem::CheckConvar(ConVar* cvar_ref)
{
	if ( !cvar_ref ) return;
	if ( Q_stricmp( "voice_boost", cvar_ref->GetName() ) == 0 && g_pEngineVoiceTweak )
		g_pEngineVoiceTweak->SetControlFloat( MicBoost, cvar_ref->GetBool() ? 1.0f : 0.0f );
	else if ( Q_stricmp( "cont_phoneringtone", cvar_ref->GetName() ) == 0 )
		g_m_PreviewTune = cvar_ref->GetInt();
}

void COptionVarItem::GetStringValue( char *buf, int len )
{
	if ( !m_pStringInput ) return;
	m_pStringInput->GetText( buf, len );
}

void COptionVarItem::SetStringValue( const char* buf )
{
	if ( !m_pStringInput ) return;
	m_pStringInput->SetText( buf );
}

void COptionVarItem::ClearList()
{
	m_list.RemoveAll();
}

int COptionVarItem::AddItem(const char* szText, std::string value)
{
	OptionItemValues_t val;
	val.name = szText;
	val.value = value;
	return m_list.AddToTail( val );
}

void COptionVarItem::Paint()
{
	BaseClass::Paint();

	int w, t;
	GetSize(w, t);

	// If special
	if ( !IsSpecial( "" ) && m_list.Count() == 0 || m_szKeyBind != "" )
	{
		if ( pChoicesButton )
		{
			pChoicesButton->SetPos(0, 0);
			pChoicesButton->SetSize(w, t);
			pChoicesButton->MoveToFront();
		}
	}
	else
	{
		// Hidden BOTTAN
		if ( pHiddenButton )
		{
			pHiddenButton->SetPos(0, 0);
			pHiddenButton->SetSize(w, t);
			pHiddenButton->MoveToFront();
		}
	}

	// Paint background
	DrawBackground(w, t);

	int iSize = (w / 2);

	if ( m_bTitle )
	{
		iSize = w;
		if ( m_label_text )
			m_label_text->SetContentAlignment( Label::Alignment::a_center );
	}

	if ( m_label_text )
	{
		m_label_text->SetPos( vgui::scheme()->GetProportionalScaledValue( 5 ), vgui::scheme()->GetProportionalScaledValue( 3 ) );
		m_label_text->SetSize( iSize, vgui::scheme()->GetProportionalScaledValue( 15 ) );
	}

	int iValuePos = 95;
	int iValuePosIcon = 63;
	int iValuePosAdd = 0;
	switch ( g_m_AspectRatio )
	{
		// Normal ratio (4:3)
		case 0:
			iValuePos = 35;
			iValuePosIcon = 25;
		break;
		// Widescreen (10:9)
		case 2:
			iValuePos = 80;
			iValuePosIcon = 50;
		break;
	}

	int iValueSize = 98;

	if ( pSlider )
	{
		pSlider->SetPos( iSize + vgui::scheme()->GetProportionalScaledValue( iValuePos ), vgui::scheme()->GetProportionalScaledValue( 3 ) );
		pSlider->SetSize( vgui::scheme()->GetProportionalScaledValue( iValueSize ), vgui::scheme()->GetProportionalScaledValue( 15 ) );

		// Draw our display value
		if ( pSlider->GetDisplayValue() && m_label_value )
		{
			m_label_value->SetSize( vgui::scheme()->GetProportionalScaledValue( 80 ), vgui::scheme()->GetProportionalScaledValue( 15 ) );
			m_label_value->SetContentAlignment( Label::a_east );

			int x, y;
			pSlider->GetPos( x, y );
			m_label_value->SetPos( x - m_label_value->GetWide() - 10, y );
		}

		// Move the slider to front
		pSlider->MoveToFront();
		return;
	}

	// Microphone
	if ( IsSpecial( "microphone_test" ) && m_pMicMeter && m_pMicMeter2 && m_pMicMeterIndicator )
	{
		m_pMicMeter->SetPos( iSize + vgui::scheme()->GetProportionalScaledValue( iValuePos ), vgui::scheme()->GetProportionalScaledValue( 3 ) );
		m_pMicMeter->SetSize( vgui::scheme()->GetProportionalScaledValue( iValueSize ), vgui::scheme()->GetProportionalScaledValue( 15 ) );
		m_pMicMeter2->SetPos( iSize + vgui::scheme()->GetProportionalScaledValue( iValuePos ), vgui::scheme()->GetProportionalScaledValue( 3 ) );
		m_pMicMeter2->SetSize( vgui::scheme()->GetProportionalScaledValue( iValueSize ), vgui::scheme()->GetProportionalScaledValue( 15 ) );
		m_pMicMeterIndicator->SetSize( vgui::scheme()->GetProportionalScaledValue( 30 ), vgui::scheme()->GetProportionalScaledValue( 15 ) );

		// Set pos for mic meter 2 & indicator
		int wide, tall;
		m_pMicMeter->GetSize(wide, tall);

		int indicatorWide, indicatorTall;
		m_pMicMeterIndicator->GetSize(indicatorWide, indicatorTall);

		int iXPos, iYPos;
		m_pMicMeter2->GetPos( iXPos, iYPos );

		int iFinalPos = iXPos - ( indicatorWide / 2 ) + static_cast<float>( wide ) * g_pEngineVoiceTweak->GetControlFloat( SpeakingVolume );

		m_pMicMeterIndicator->GetPos( iXPos, iYPos );

		iFinalPos = Approach( iFinalPos, iXPos, vgui::scheme()->GetProportionalScaledValue( 4 ) );

		m_pMicMeterIndicator->SetPos( iFinalPos, iYPos );
		return;
	}

	// String input
	else if ( IsSpecial( "string" ) )
	{
		if ( pHiddenButton )
		{
			pHiddenButton->SetPos(0, 0);
			pHiddenButton->SetSize(0, 0);
		}
		if ( pChoicesButton )
		{
			pChoicesButton->SetPos(0, 0);
			pChoicesButton->SetSize(0, 0);
		}
		if ( m_pStringInput )
		{
			m_pStringInput->SetPos( iSize + vgui::scheme()->GetProportionalScaledValue( iValuePos ), vgui::scheme()->GetProportionalScaledValue( 3 ) );
			m_pStringInput->SetSize( vgui::scheme()->GetProportionalScaledValue( iValueSize ), vgui::scheme()->GetProportionalScaledValue( 15 ) );
			m_pStringInput->MoveToFront();
		}
		return;
	}

	if ( m_szKeyBind != "" && m_pMicMeter )
	{
		m_pMicMeter->SetPos( iSize + vgui::scheme()->GetProportionalScaledValue( iValuePosIcon ), vgui::scheme()->GetProportionalScaledValue( 3 ) );
		if ( g_m_AspectRatio == 0 )
			iValuePosAdd = 20;
	}

	if ( m_label_value )
	{
		m_label_value->SetPos( iSize + vgui::scheme()->GetProportionalScaledValue( iValuePos + iValuePosAdd ), vgui::scheme()->GetProportionalScaledValue( 3 ) );
		m_label_value->SetSize( vgui::scheme()->GetProportionalScaledValue( iValueSize ), vgui::scheme()->GetProportionalScaledValue( 15 ) );
	}

	// We got nothing, don't draw any choices
	if ( m_list.Count() == 0 )
		return;

	if ( pChoicesButton )
	{
		pChoicesButton->SetPos( iSize + vgui::scheme()->GetProportionalScaledValue( iValuePos ), vgui::scheme()->GetProportionalScaledValue( 3 ) );
		pChoicesButton->SetSize( vgui::scheme()->GetProportionalScaledValue( iValueSize ), vgui::scheme()->GetProportionalScaledValue( 15 ) );
		pChoicesButton->MoveToFront();
	}

	DrawValueChoices(iSize, iValuePos, iValueSize);
}

void COptionVarItem::DrawBackground(int w, int t)
{
	// If title, don't draw this
	if (m_bTitle) return;
	if (m_Active)
		vgui::surface()->DrawSetColor( 150, 150, 150, 160 );
	else
	{
		if ( IsCursorOver() )
		{
			// Only set the state to hover, if we are moving our mouse
			if ( IsCursorMoving() )
				SetSelected( STATE_HOVER );
		}
		else
		{
			// Only if we aren't in keyboard state
			if ( m_iSelectedState != STATE_BUTTON )
				SetSelected( STATE_NONE );
		}

		switch ( m_iSelectedState )
		{
			case STATE_BUTTON:
			case STATE_HOVER:
				vgui::surface()->DrawSetColor( 100, 100, 100, 160 );
			break;

			default:
				vgui::surface()->DrawSetColor( 50, 50, 50, 160 );
			break;
		}
	}
	vgui::surface()->DrawFilledRect( 0, 0, 0 + w, 0 + t );
}

void COptionVarItem::DrawValueChoices(int pos, int posx, int wide)
{
	int xpos = pos + vgui::scheme()->GetProportionalScaledValue( posx ) - vgui::scheme()->GetProportionalScaledValue( 5 );
	int border_size = 1;
	int border_pos = vgui::scheme()->GetProportionalScaledValue( 5 );

	// Draw our icons
	int icon_wide = vgui::scheme()->GetProportionalScaledValue( 10 );
	int icon_tall = vgui::scheme()->GetProportionalScaledValue( 10 );
	SetCamTexture( pCamRight, s_ArrowRight, xpos - icon_wide, border_pos, icon_wide, icon_tall );
	SetCamTexture( pCamLeft, s_ArrowLeft, xpos - icon_wide - 5 - icon_wide, border_pos, icon_wide, icon_tall );

	// Set our default color
	vgui::surface()->DrawSetColor(87, 87, 87, 220);

	// Draw the side border
	vgui::surface()->DrawFilledRect(xpos, border_pos, xpos + border_size, border_pos + vgui::scheme()->GetProportionalScaledValue( 10 ) );

	// Draw the bottom border
	xpos = pos + vgui::scheme()->GetProportionalScaledValue( posx );
	border_pos = vgui::scheme()->GetProportionalScaledValue( 18 );
	vgui::surface()->DrawFilledRect(xpos, border_pos, xpos + vgui::scheme()->GetProportionalScaledValue( wide ), border_pos + border_size);

	// Our main border size
	int iSize = vgui::scheme()->GetProportionalScaledValue( wide );

	// If we end up with odd numbers, use these values
	bool bEndedWithOdd = false;

	int iDivide = 2;
	int iCount = m_list.Count();
	if ( iCount >= 8 )
		iDivide = 4;
	if ( iCount >= 16 )
		iDivide = 6;
	if ( iCount >= 24 )
		iDivide = 8;

	if ( m_iCustomDivide > 0 )
		iDivide = m_iCustomDivide;

	// Calculate how big our borders should be
	for ( int i = 0; i < iCount; i++ )
	{
		// 1 is our smallest size
		if ( iSize <= 1 )
		{
			iSize = vgui::scheme()->GetProportionalScaledValue( 5 );
			break;
		}
		if (i % iDivide == 0)
		{
			iSize = (iSize / 2);
			bEndedWithOdd = false;
		}
		else
			bEndedWithOdd = true;
	}

	if ( !bEndedWithOdd )
		iSize += (iSize/3);

	if ( iCount == 12 )
		iSize -= vgui::scheme()->GetProportionalScaledValue( 4 );

	// Make sure we are above the border itself
	border_pos -= 1;
	int border_pos_current = 0;
	int xCoice = xpos;
	for (int i = 0; i < iCount; i++)
	{
		if ( i == m_iCurrent )
		{
			vgui::surface()->DrawSetColor( 187, 187, 187, 220 );
			border_pos_current = border_pos - 2;
			border_size = 3;
		}
		else
		{
			vgui::surface()->DrawSetColor( 143, 143, 143, 220 );
			border_pos_current = border_pos;
			border_size = 1;
		}

		vgui::surface()->DrawFilledRect( xCoice, border_pos_current, xCoice + iSize, border_pos_current + border_size );
		xCoice += iSize;
	}
}

void COptionVarItem::SetValueText(const char* szText)
{
	if ( m_label_value )
		m_label_value->SetText(szText);
}

void COptionVarItem::SetValueText(const wchar_t* szText)
{
	if ( m_label_value )
		m_label_value->SetText(szText);
}

void COptionVarItem::OnCommand(const char* command)
{
	// Specials
	if ( IsSpecial( "microphone_test" ) )
	{
		if ( g_pEngineVoiceTweak && m_pMicMeter2 )
		{
			CONTUI::CL4DBasePanel::GetSingleton().PlayUISound( CONTUI::UISOUND_CLICK );
			if ( !m_pMicMeter2->IsVisible() )
				StartTestMicrophone();
			else
				EndTestMicrophone();
		}
		return;
	}
	else if ( IsSpecial( "phone_tune" ) )
		GameClientExports()->PreviewRingtone( g_m_PreviewTune );
	// If keybind
	else if ( m_szKeyBind != "" )
	{
		CONTUI::CL4DBasePanel::GetSingleton().PlayUISound( CONTUI::UISOUND_CLICK );

		SetValueText( "#CONTUI_NewGameOptions_WaitingForButton" );
		if ( m_pMicMeter )
			m_pMicMeter->SetImage( "common/cont_spinner" );
		m_bInCaptureMode = true;

		vgui::input()->SetMouseFocus(GetVPanel());
		vgui::input()->SetMouseCapture(GetVPanel());

		engine->StartKeyTrapMode();
		return;
	}

	if ( Q_stricmp( "Increase", command ) == 0 )
		CurrentPosChange(true);
	else if ( Q_stricmp( "Decrease", command ) == 0 )
		CurrentPosChange(false);
	else
		BaseClass::OnCommand(command);

	// Do the phone_bg last
	if ( IsSpecial( "phone_bg" ) )
	{
		CONTUI::NewGameOptions* self = static_cast< CONTUI::NewGameOptions * >( CONTUI::CL4DBasePanel::GetSingleton().GetWindow( CONTUI::WT_OPTIONS_NEW ) );
		if ( self )
			self->OnPhoneBackgroundChanged( m_iCurrent );
	}
}

void COptionVarItem::OnThink()
{
	BaseClass::OnThink();
	if ( m_bInCaptureMode )
	{
		ButtonCode_t bCode = BUTTON_CODE_INVALID;
		if ( engine->CheckDoneKeyTrapping( bCode ) )
		{
			m_bInCaptureMode = false;
			vgui::input()->SetMouseCapture(NULL);
			RequestFocus();
			vgui::input()->SetMouseFocus(GetVPanel());

			char const *pszKeyName = g_pInputSystem->ButtonCodeToString( bCode );
			if ( pszKeyName && *pszKeyName )
			{
				if ( bCode != KEY_DELETE && bCode != KEY_ESCAPE )
				{
					// Set the text
					SetValueText( pszKeyName );

					// Draw this again
					if ( m_szController != "" && std::stoi( m_szController ) > 0 )
						SetKeyIcon( bCode );
					else
					{
						if ( m_pMicMeter )
							m_pMicMeter->SetVisible( false );
					}
				}
				else
				{
					switch ( bCode )
					{
						// If we delete
						case KEY_DELETE:
						{
							SetValueText( "" );
							if ( m_pMicMeter )
								m_pMicMeter->SetVisible( false );
							bCode = BUTTON_CODE_INVALID;
						}
						break;
						// Don't override, just cancel.
						case KEY_ESCAPE:
						{
							GrabButtonBind();
							return;
						}
						break;
					}
				}

				// Apply the keybind
				ButtonBinding_t ret = TryApplyKeyBind( bCode, pszKeyName );
				switch ( ret )
				{
					case ButtonBinding_t::BIND_ERROR_INPUT_KEBOARD:
					{
						GrabButtonBind();
						CONTUI::NewGameOptions* self = static_cast< CONTUI::NewGameOptions * >( CONTUI::CL4DBasePanel::GetSingleton().GetWindow( CONTUI::WT_OPTIONS_NEW ) );
						if ( self )
						{
							CONTUI::GenericConfirmation* confirmation = static_cast< CONTUI::GenericConfirmation* >( CONTUI::CL4DBasePanel::GetSingleton().OpenWindow( CONTUI::WT_GENERICCONFIRMATION, self, false ) );
							CONTUI::GenericConfirmation::Data_t data;

							data.pWindowTitle = "#CONTUI_NewGameOptions_Binding_Error";

							std::string szMsg = g_pVGuiLocalizeV2->GetTranslationKey( "#CONTUI_NewGameOptions_Binding_Error_Keyboard" );
							data.pMessageText = szMsg.c_str();
							data.bOkButtonEnabled = true;

							confirmation->SetUsageData(data);
						}
					}
					break;

					case ButtonBinding_t::BIND_ERROR_INPUT_CONTROLLER:
					{
						GrabButtonBind();
						CONTUI::NewGameOptions* self = static_cast< CONTUI::NewGameOptions * >( CONTUI::CL4DBasePanel::GetSingleton().GetWindow( CONTUI::WT_OPTIONS_NEW ) );
						if ( self )
						{
							CONTUI::GenericConfirmation* confirmation = static_cast< CONTUI::GenericConfirmation* >( CONTUI::CL4DBasePanel::GetSingleton().OpenWindow( CONTUI::WT_GENERICCONFIRMATION, self, false ) );
							CONTUI::GenericConfirmation::Data_t data;

							data.pWindowTitle = "#CONTUI_NewGameOptions_Binding_Error";

							std::string szMsg = g_pVGuiLocalizeV2->GetTranslationKey( "#CONTUI_NewGameOptions_Binding_Error_Controller" );
							data.pMessageText = szMsg.c_str();
							data.bOkButtonEnabled = true;

							confirmation->SetUsageData(data);
						}
					}
					break;
				}
			}
		}
		else
		{
			if ( m_pMicMeter && m_flRefreshingTime < Plat_FloatTime() )
			{
				m_flRefreshingTime = Plat_FloatTime() + 0.032f;
				m_pMicMeter->SetVisible( true );
				m_pMicMeter->SetFrame( m_pMicMeter->GetFrame()+1 );
				m_pMicMeter->SetSize( vgui::scheme()->GetProportionalScaledValue( 14 ), vgui::scheme()->GetProportionalScaledValue( 15 ) );
			}
		}
	}

	// If visible, check our string matches or not, if not, alert about new changes
	if ( m_pStringInput && m_pStringInput->IsVisible() )
	{
		auto cvar_ref = g_pCVar->FindVar( GetCvar().c_str() );
		if ( !cvar_ref ) return;
		char buffer[512];
		m_pStringInput->GetText( buffer, sizeof(buffer) );
		if ( !FStrEq( buffer, cvar_ref->GetString() ) )
		{
			CONTUI::NewGameOptions* self = static_cast< CONTUI::NewGameOptions * >( CONTUI::CL4DBasePanel::GetSingleton().GetWindow( CONTUI::WT_OPTIONS_NEW ) );
			if ( self )
			{
				self->SetRequiresRestart( RestartRequired() );
				self->SetValueChanged( true );
			}
		}
	}
}

void COptionVarItem::CurrentPosChange(bool increase)
{
	if (m_list.Count() <= 0) return;
	// Increase it, or decrease it
	if ( increase ) m_iCurrent++;
	else m_iCurrent--;

	// Clamp it
	if ( m_iCurrent >= m_list.Count())
		m_iCurrent = 0;
	if ( m_iCurrent < 0 )
		m_iCurrent = m_list.Count() -1;

	// Set our value
	if ( m_label_value )
		m_label_value->SetText(GetCurrentName(m_iCurrent).c_str());

	CONTUI::CL4DBasePanel::GetSingleton().PlayUISound( CONTUI::UISOUND_CLICK );

	// Tell our frame that we changed our aspect ratio values
	if ( IsSpecial( "video_asp" ) )
	{
		CONTUI::NewGameOptions* self = static_cast< CONTUI::NewGameOptions * >( CONTUI::CL4DBasePanel::GetSingleton().GetWindow( CONTUI::WT_OPTIONS_NEW ) );
		if ( self )
			self->OnAspectRatioChanged( m_iCurrent );
	}
	else if ( IsSpecial( "phone_tune_set" ) )
		g_m_PreviewTune = m_iCurrent;

	CONTUI::NewGameOptions* self = static_cast< CONTUI::NewGameOptions * >( CONTUI::CL4DBasePanel::GetSingleton().GetWindow( CONTUI::WT_OPTIONS_NEW ) );
	if ( self )
	{
		self->OnChangeImageFrame( m_iCurrent );
		self->SetRequiresRestart( RestartRequired() );
		self->SetValueChanged( true );
	}
}

void COptionVarItem::SetTitle(bool state, bool stateSS)
{
	m_bTitle = state;
	if ( stateSS && m_label_text )
	{
		char sz[256];
		m_label_text->GetText(sz, 256);
		std::string newtitle(sz);
		newtitle += g_pVGuiLocalizeV2->GetTranslationKey( "#CONTUI_NewGameOptions_SS_Add" );
		m_label_text->SetText(newtitle.c_str());
	}
}

void COptionVarItem::SetKeyBind(const std::string& bind)
{
	m_szKeyBind = bind;

	m_pMicMeter = new ImagePanel( this, "KeyBindImage" );
	m_pMicMeter->SetShouldScaleImage( true );
	m_pMicMeter->SetVisible( false );

	GrabButtonBind();
}

void COptionVarItem::SetSpecial(const std::string& bind, const std::string& cvar)
{
	m_szSpecial = bind;

	// Once set, override the choice
	if ( IsSpecial( "microphone_test" ) )
	{
		m_pMicMeter = new ImagePanel( this, "MicMeter" );
		m_pMicMeter->SetImage( "resource/mic_meter_dead" );
		m_pMicMeter->SetShouldScaleImage( true );
		m_pMicMeter->SetVisible( false );

		m_pMicMeter2 = new ImagePanel( this, "MicMeter2" );
		m_pMicMeter2->SetImage( "resource/mic_meter_live" );
		m_pMicMeter2->SetShouldScaleImage( true );
		m_pMicMeter2->SetVisible( false );

		m_pMicMeterIndicator = new ImagePanel( this, "MicMeterIndicator" );
		m_pMicMeterIndicator->SetImage( "resource/mic_meter_indicator" );
		m_pMicMeterIndicator->SetShouldScaleImage( true );
		m_pMicMeterIndicator->SetVisible( false );

		m_pMicMeter->SetVisible( true );
	}
	else if ( IsSpecial( "microphone_boost" ) )
		OverrideChoicesCommand( "MicBoost" );
	else if ( IsSpecial( "string" ) )
	{
		if ( m_pStringInput )
		{
			m_pStringInput->SetVisible( true );
			auto cvar_ref = g_pCVar->FindVar( cvar.c_str() );
			if ( cvar_ref )
				m_pStringInput->SetText( cvar_ref->GetString() );
		}
	}
	else if ( IsSpecial( "revert_mouse" ) )
	{
		auto cvar_ref = g_pCVar->FindVar( "m_pitch" );
		if ( cvar_ref )
		{
			float value = cvar_ref->GetFloat();
			if ( value < 0 )
				SetValue( 1 );
			else
				SetValue( 0 );
		}
	}
	else if ( IsSpecial( "download_filter" ) )
	{
		auto cvar_ref = g_pCVar->FindVar( "cl_downloadfilter" );
		if ( cvar_ref )
		{
			if ( Q_stricmp( "none", cvar_ref->GetString() ) == 0 )
				SetValue( 0 );
			else if ( Q_stricmp( "noworkshop", cvar_ref->GetString() ) == 0 )
				SetValue( 1 );
			else if ( Q_stricmp( "all", cvar_ref->GetString() ) == 0 )
				SetValue( 2 );
		}
	}
	else if ( IsSpecial( "phone_bg" ) )
	{
		for ( int i = 0; i < 12; i++)
		{
			std::string szMsg( g_pVGuiLocalizeV2->GetTranslationKey( "#CONTUI_NewGameOptions_BackgroundItem" ) );
			CONTUI_STDReplaceString( szMsg, "%num%", std::to_string( i ) );
			AddItem( szMsg.c_str() );
		}
		auto cvar_ref = g_pCVar->FindVar( "cont_phonebackground" );
		SetValue( cvar_ref->GetInt() );
	}
	else if ( IsSpecial( "movement_stick" ) )
	{
		auto joy_legacy = g_pCVar->FindVar( "joy_legacy" );
		auto joy_movement_stick = g_pCVar->FindVar( "joy_movement_stick" );
		bool bUsingLegacy = joy_legacy->GetBool();
		bool bUsingSouthpaw = joy_movement_stick->GetBool();
		int iVal = 0;
		if ( bUsingSouthpaw )
			iVal = bUsingLegacy ? 3 : 1;
		else
			iVal = bUsingLegacy ? 2 : 0;
		SetValue( iVal );
	}
	else if ( IsSpecial( "movement_stick2" ) )
	{
		auto joy_legacy2 = g_pCVar->FindVar( "joy_legacy2" );
		auto joy_movement_stick2 = g_pCVar->FindVar( "joy_movement_stick2" );
		bool bUsingLegacy = joy_legacy2->GetBool();
		bool bUsingSouthpaw = joy_movement_stick2->GetBool();
		int iVal = 0;
		if ( bUsingSouthpaw )
			iVal = bUsingLegacy ? 3 : 1;
		else
			iVal = bUsingLegacy ? 2 : 0;
		SetValue( iVal );
	}
	// Check our aspect ratio
	else if ( IsSpecial( "video_asp" ) )
	{
		// Set the screen size mode
		const MaterialSystem_Config_t &config = materials->GetCurrentConfigForVideoCard();
		m_iCurrent = GetScreenAspectMode( config.m_VideoMode.m_Width, config.m_VideoMode.m_Height );
		SetValue( m_iCurrent );
	}
	// Check our video res
	else if ( IsSpecial( "video_res" ) )
	{
		// Grab the aspect ratio, and fill the default list
		const MaterialSystem_Config_t &config = materials->GetCurrentConfigForVideoCard();
		int iAspect = GetScreenAspectMode( config.m_VideoMode.m_Width, config.m_VideoMode.m_Height );
		bool bWindowed = config.Windowed();
		if ( config.NoWindowBorder() )
			bWindowed = false;
		SetResolutionList( iAspect, bWindowed );
	}
	// Check our display mode
	else if ( IsSpecial( "video_disp" ) )
	{
		// Setup our display mode
		const MaterialSystem_Config_t &config = materials->GetCurrentConfigForVideoCard();
		if ( config.Windowed() )
		{
			if ( config.NoWindowBorder() )
				SetValue( 2 );
			else
				SetValue( 1 );
		}
		else
			SetValue( 0 );
	}
	// Check our available languages
	else if ( IsSpecial( "video_lang" ) && SteamApps() )
	{
		char szAvailableLanguages[512] = "";
		szAvailableLanguages[0] = NULL;

		Q_strncpy( szAvailableLanguages, g_pVGuiLocalizeV2->GetAvailableLanguages().c_str(), sizeof(szAvailableLanguages) );

		if ( V_strlen( szAvailableLanguages ) )
		{
			// Populate the combo box with each available language
			CSplitString languagesList( szAvailableLanguages, "," );
			for ( int i = 0; i < languagesList.Count(); i++ )
			{
				const ELanguage languageCode = PchLanguageToELanguage( languagesList[i] );
				AddItem( GetLanguageVGUILocalization( languageCode ), languagesList[i] );
			}
		}

		// Now select our current language
		const ELanguage languageCode = PchLanguageToELanguage( g_pVGuiLocalizeV2->GetCurrentLanguage().c_str() );
		SetValue( GetSelectedValue( GetLanguageShortName( languageCode ) ) );
	}
	// Check our Antialiasing
	else if ( IsSpecial( "video_aa" ) )
	{
		AddItem( "#GameUI_None" );
		Add_AA_Mode( 1, 0 );

		if ( materials->SupportsMSAAMode(2) )
		{
			AddItem( "#GameUI_2X" );
			Add_AA_Mode( 2, 0 );
		}

		if ( materials->SupportsMSAAMode(4) )
		{
			AddItem( "#GameUI_4X" );
			Add_AA_Mode( 4, 0 );
		}

		if ( materials->SupportsMSAAMode(6) )
		{
			AddItem( "#GameUI_6X" );
			Add_AA_Mode( 6, 0 );
		}

		if ( materials->SupportsCSAAMode(4, 2) )
		{
			AddItem( "#GameUI_8X_CSAA" );
			Add_AA_Mode( 4, 2 );
		}

		if ( materials->SupportsCSAAMode(4, 4) )
		{
			AddItem( "#GameUI_16X_CSAA" );
			Add_AA_Mode( 4, 4 );
		}

		if ( materials->SupportsMSAAMode(8) )
		{
			AddItem( "#GameUI_8X" );
			Add_AA_Mode( 8, 0 );
		}

		if ( materials->SupportsCSAAMode(8, 2) )
		{
			AddItem( "#GameUI_16XQ_CSAA" );
			Add_AA_Mode( 8, 2 );
		}

		auto mat_antialias = g_pCVar->FindVar( "mat_antialias" );
		auto mat_aaquality = g_pCVar->FindVar( "mat_aaquality" );
		int nAASamples = mat_antialias->GetInt();
		int nAAQuality = mat_aaquality->GetInt();
		int nMSAAMode = FindMSAAMode( nAASamples, nAAQuality );
		m_iCurrent = nMSAAMode;
		mat_aasetting.SetValue( m_iCurrent );
		SetValue( m_iCurrent );
		if ( !g_mHasAppliedAAModes )
			g_mHasAppliedAAModes = true;
	}
}

void COptionVarItem::OverrideChoicesCommand(const char* szCommand)
{
	pChoicesButton->SetCommand(szCommand);
}

void COptionVarItem::SetSlider(const std::string& szmin, const std::string& szmax, const std::string& szstep, const std::string& szvar, bool displayvalue, bool displayfloat, bool inversefill, int floatformat, bool restartneeded)
{
	if ( szmin == "" ) return;
	if ( szmax == "" ) return;
	if ( szstep == "" ) return;

	pSlider = new COptionSlider( this, "slider" );

	if ( szvar == "voice_transmit" )
	{
		auto voice_transmit = g_pCVar->FindVar( "voice_transmit" );
		float micVolume = g_pEngineVoiceTweak ? g_pEngineVoiceTweak->GetControlFloat( MicrophoneVolume ) : 0.0f;
		voice_transmit->SetValue( (int)( 100.0f * micVolume ) );
	}

	pSlider->SetMin( std::stof(szmin) );
	pSlider->SetMax( std::stof(szmax) );
	pSlider->SetStepSize( std::stof(szstep) );
	pSlider->SetDisplayValue( displayvalue );
	pSlider->SetDisplayInFloat( displayfloat );
	pSlider->SetFloatFormat( floatformat );
	pSlider->SetInverse( inversefill );
	pSlider->SetConCommand( szvar.c_str() );
	pSlider->SetRestart( restartneeded );
}

COptionVarItem::ButtonBinding_t COptionVarItem::TryApplyKeyBind(const ButtonCode_t& bind, const char* szKey)
{
	bool bIsJoystickCode = IsJoystickCode( bind );
	const char* pszOldButtonName = g_pInputSystem->ButtonCodeToString( m_bcButton );
	// Same button, ignore...
//	if ( m_bcButton == bind ) return ButtonBinding_t::BIND_ERROR_SAMEBUTTON;

	if ( m_szController != "" && std::stoi( m_szController ) > 0 )
	{
		if ( !bIsJoystickCode ) return ButtonBinding_t::BIND_ERROR_INPUT_KEBOARD;
		ButtonCode_t temp = bind;
		if ( IsValidController( temp, std::stoi( m_szController ), true ) )
		{
			// Remove old (if found)
			if ( pszOldButtonName )
				engine->ClientCmd_Unrestricted( VarArgs( "cmd%d unbind \"%s\"\n", std::stoi( m_szController ), pszOldButtonName ) );
			// Also remove if it's already bound to something else
			engine->ClientCmd_Unrestricted( VarArgs( "cmd%d unbind \"%s\"\n", std::stoi( m_szController ), szKey ) );

			// Bind new
			if ( temp != BUTTON_CODE_INVALID )
				engine->ClientCmd_Unrestricted( VarArgs( "cmd%d bind \"%s\" \"%s\"\n", std::stoi( m_szController ), szKey, m_szKeyBind.c_str() ) );

			// Apply button
			m_bcButton = temp;

			// Reload
			engine->ClientCmd_Unrestricted( "wait 5;hud_reloadoptions\n" );
			return ButtonBinding_t::BIND_OK;
		}
		return ButtonBinding_t::BIND_ERROR_CONTROLLER_BIND;
	}
	if ( bIsJoystickCode ) return ButtonBinding_t::BIND_ERROR_INPUT_CONTROLLER;
	// Remove old (if found)
	if ( pszOldButtonName )
		engine->ClientCmd_Unrestricted( VarArgs( "unbind \"%s\"\n", pszOldButtonName ) );
	// Also remove if it's already bound to something else
	engine->ClientCmd_Unrestricted( VarArgs( "unbind \"%s\"\n", szKey ) );

	// Bind new
	if ( bind != BUTTON_CODE_INVALID )
		engine->ClientCmd_Unrestricted( VarArgs( "bind \"%s\" \"%s\"\n", szKey, m_szKeyBind.c_str() ) );

	// Apply button
	m_bcButton = bind;

	// Reload
	engine->ClientCmd_Unrestricted( "wait 5;hud_reloadoptions 1\n" );

	// All OK
	return ButtonBinding_t::BIND_OK;
}

void COptionVarItem::RebuildLabel()
{
	// Delete previous label, and create new one.
	// If we don't do this, then VGUI won't give a damn, even if we change our screen res.
	// This is fucking stupid.
	char buffer[256];
	if ( m_label_text )
		m_label_text->GetText( buffer, 256 );
	std::string buffer_label( buffer );
	if ( m_label_value )
		m_label_value->GetText( buffer, 256 );
	std::string buffer_value( buffer );
	if ( m_pStringInput )
		m_pStringInput->GetText( buffer, 256 );
	std::string buffer_input( buffer );
	bool bIsInputVisible = m_pStringInput ? m_pStringInput->IsVisible() : false;

	// Get rid of the text value, and insert our saved string.
	if ( m_label_text )
		delete m_label_text;
	m_label_text = new Label( this, "label1", buffer_label.c_str() );

	// Ditto, do the same for value...
	if ( m_label_value )
		delete m_label_value;
	m_label_value = new Label( this, "label2", buffer_value.c_str() );

	// Ditto, do the same for text input...
	if ( m_pStringInput )
		delete m_pStringInput;
	m_pStringInput = new TextEntry( this, "Input" );
	m_pStringInput->SetText( buffer_input.c_str() );
	m_pStringInput->SetVisible( bIsInputVisible );

	// Clear memory
	buffer_value.clear();
	buffer_label.clear();
	buffer_input.clear();
}

void COptionVarItem::GrabButtonBind()
{
	if ( m_szKeyBind == "" ) return;

	// Default
	SetValueText( "" );
	if ( m_pMicMeter )
		m_pMicMeter->SetVisible( false );

	// If controller, then check for IT'S bind's, not keyboard!
	if ( m_szController != "" && std::stoi( m_szController ) > 0 )
	{
		// Controller isn't that straight forward.
		for ( int i = KEY_FIRST; i < BUTTON_CODE_LAST; i++ )
		{
			ButtonCode_t bCode = ( ButtonCode_t )i;
			bool bIsJoystickCode = IsJoystickCode( bCode );
			if ( !bIsJoystickCode ) continue;
			// We found something!, now, check if it's on the correct controller!
			if ( IsValidController( bCode, std::stoi( m_szController ) ) )
			{
				const char* pszButtonName = g_pInputSystem->ButtonCodeToString( bCode );
				if ( pszButtonName )
				{
					SetValueText( pszButtonName );
					SetKeyIcon( bCode );
					m_bcButton = bCode;
				}
				break;
			}
		}
		return;
	}

	// For keyboard, it's the same, but if joystick code, skip. Simple huh?
	for ( int i = KEY_FIRST; i < BUTTON_CODE_LAST; i++ )
	{
		ButtonCode_t bCode = ( ButtonCode_t )i;
		bool bIsJoystickCode = IsJoystickCode( bCode );
		if ( bIsJoystickCode ) continue;
		// Make sure we find the bind
		const char* pszBinding = gameuifuncs->GetBindingForButtonCode( bCode );
		if ( !pszBinding ) continue;
		// We found it! now, make sure it's the one we want.
		if ( Q_stricmp( m_szKeyBind.c_str(), pszBinding ) != 0 ) continue;
		// Now make sure it's properly bound.
		const char* pszButtonName = g_pInputSystem->ButtonCodeToString( bCode );
		if ( pszButtonName )
			SetValueText( pszButtonName );
		m_bcButton = bCode;
		break;
	}
}

bool COptionVarItem::IsValidController(ButtonCode_t& bind, int controller, bool binding)
{
	// Check this first!
	if ( !IsCorrectController( bind, controller, binding ) ) return false;
	const char* pszBinding = gameuifuncs->GetBindingForButtonCode( bind );
	if ( !pszBinding ) return false;
	// We are binding, we don't care about the rest
	if ( binding ) return true;
	if ( Q_stricmp( m_szKeyBind.c_str(), pszBinding ) == 0 ) return true;
	return false;
}

bool COptionVarItem::IsCorrectController(ButtonCode_t& code, int controller, bool binding)
{
	// If we are binding, apply for both!
	if ( binding )
	{
		// Reverse it, if it's for player 2, vice versa for player 1
		// If we are using the wrong controller
		if ( controller == 2 )
		{
			SwitchButtonBind( code, KEY_XBUTTON_A, KEY_XBUTTON2_A );
			SwitchButtonBind( code, KEY_XBUTTON_B, KEY_XBUTTON2_B );
			SwitchButtonBind( code, KEY_XBUTTON_X, KEY_XBUTTON2_X );
			SwitchButtonBind( code, KEY_XBUTTON_Y, KEY_XBUTTON2_Y );
			SwitchButtonBind( code, KEY_XBUTTON_LEFT_SHOULDER, KEY_XBUTTON2_LEFT_SHOULDER );
			SwitchButtonBind( code, KEY_XBUTTON_RIGHT_SHOULDER, KEY_XBUTTON2_RIGHT_SHOULDER );
			SwitchButtonBind( code, KEY_XBUTTON_BACK, KEY_XBUTTON2_BACK );
			SwitchButtonBind( code, KEY_XBUTTON_START, KEY_XBUTTON2_START );
			SwitchButtonBind( code, KEY_XBUTTON_STICK1, KEY_XBUTTON2_STICK1 );
			SwitchButtonBind( code, KEY_XBUTTON_STICK2, KEY_XBUTTON2_STICK2 );
			SwitchButtonBind( code, KEY_XBUTTON_INACTIVE_START, KEY_XBUTTON2_INACTIVE_START );
			SwitchButtonBind( code, KEY_XBUTTON_UP, KEY_XBUTTON2_UP );
			SwitchButtonBind( code, KEY_XBUTTON_RIGHT, KEY_XBUTTON2_RIGHT );
			SwitchButtonBind( code, KEY_XBUTTON_DOWN, KEY_XBUTTON2_DOWN );
			SwitchButtonBind( code, KEY_XBUTTON_LEFT, KEY_XBUTTON2_LEFT );
			SwitchButtonBind( code, KEY_XBUTTON_LTRIGGER, KEY_XBUTTON2_LTRIGGER );
			SwitchButtonBind( code, KEY_XBUTTON_RTRIGGER, KEY_XBUTTON2_RTRIGGER );
		}
		else
		{
			SwitchButtonBind( code, KEY_XBUTTON2_A, KEY_XBUTTON_A );
			SwitchButtonBind( code, KEY_XBUTTON2_B, KEY_XBUTTON_B );
			SwitchButtonBind( code, KEY_XBUTTON2_X, KEY_XBUTTON_X );
			SwitchButtonBind( code, KEY_XBUTTON2_Y, KEY_XBUTTON_Y );
			SwitchButtonBind( code, KEY_XBUTTON2_LEFT_SHOULDER, KEY_XBUTTON_LEFT_SHOULDER );
			SwitchButtonBind( code, KEY_XBUTTON2_RIGHT_SHOULDER, KEY_XBUTTON_RIGHT_SHOULDER );
			SwitchButtonBind( code, KEY_XBUTTON2_BACK, KEY_XBUTTON_BACK );
			SwitchButtonBind( code, KEY_XBUTTON2_START, KEY_XBUTTON_START );
			SwitchButtonBind( code, KEY_XBUTTON2_STICK1, KEY_XBUTTON_STICK1 );
			SwitchButtonBind( code, KEY_XBUTTON2_STICK2, KEY_XBUTTON_STICK2 );
			SwitchButtonBind( code, KEY_XBUTTON2_INACTIVE_START, KEY_XBUTTON_INACTIVE_START );
			SwitchButtonBind( code, KEY_XBUTTON2_UP, KEY_XBUTTON_UP );
			SwitchButtonBind( code, KEY_XBUTTON2_RIGHT, KEY_XBUTTON_RIGHT );
			SwitchButtonBind( code, KEY_XBUTTON2_DOWN, KEY_XBUTTON_DOWN );
			SwitchButtonBind( code, KEY_XBUTTON2_LEFT, KEY_XBUTTON_LEFT );
			SwitchButtonBind( code, KEY_XBUTTON2_LTRIGGER, KEY_XBUTTON_LTRIGGER );
			SwitchButtonBind( code, KEY_XBUTTON2_RTRIGGER, KEY_XBUTTON_RTRIGGER );
		}
	}

	switch ( code )
	{
		// Player 1
		case KEY_XBUTTON_A:
		case KEY_XBUTTON_B:
		case KEY_XBUTTON_X:
		case KEY_XBUTTON_Y:
		case KEY_XBUTTON_LEFT_SHOULDER:
		case KEY_XBUTTON_RIGHT_SHOULDER:
		case KEY_XBUTTON_BACK:
		case KEY_XBUTTON_START:
		case KEY_XBUTTON_STICK1:
		case KEY_XBUTTON_STICK2:
		case KEY_XBUTTON_INACTIVE_START:
		case KEY_XBUTTON_UP:
		case KEY_XBUTTON_RIGHT:
		case KEY_XBUTTON_DOWN:
		case KEY_XBUTTON_LEFT:
		case KEY_XBUTTON_LTRIGGER:
		case KEY_XBUTTON_RTRIGGER:
			return controller == 1 ? true : false;
		break;

		// Player 2
		case KEY_XBUTTON2_A:
		case KEY_XBUTTON2_B:
		case KEY_XBUTTON2_X:
		case KEY_XBUTTON2_Y:
		case KEY_XBUTTON2_LEFT_SHOULDER:
		case KEY_XBUTTON2_RIGHT_SHOULDER:
		case KEY_XBUTTON2_BACK:
		case KEY_XBUTTON2_START:
		case KEY_XBUTTON2_STICK1:
		case KEY_XBUTTON2_STICK2:
		case KEY_XBUTTON2_INACTIVE_START:
		case KEY_XBUTTON2_UP:
		case KEY_XBUTTON2_RIGHT:
		case KEY_XBUTTON2_DOWN:
		case KEY_XBUTTON2_LEFT:
		case KEY_XBUTTON2_LTRIGGER:
		case KEY_XBUTTON2_RTRIGGER:
			return controller == 1 ? false : true;
		break;
	}

	return false;
}

void COptionVarItem::SwitchButtonBind( ButtonCode_t& input, const ButtonCode_t& code1, const ButtonCode_t& code2 )
{
	if ( input == code1 ) input = code2;
}

void COptionVarItem::SetKeyIcon( const ButtonCode_t& code )
{
	if ( !m_pMicMeter ) return;
	int wide;
	std::string szButton = "null";
	bool bWide = false;
	switch ( code )
	{
		case KEY_XBUTTON_A:
		case KEY_XBUTTON2_A: szButton = "button_a"; break;
		case KEY_XBUTTON_B:
		case KEY_XBUTTON2_B: szButton = "button_b"; break;
		case KEY_XBUTTON_X:
		case KEY_XBUTTON2_X: szButton = "button_x"; break;
		case KEY_XBUTTON_Y:
		case KEY_XBUTTON2_Y: szButton = "button_y"; break;

		case KEY_XBUTTON_BACK:
		case KEY_XBUTTON2_BACK: szButton = "back"; break;
		case KEY_XBUTTON_START:
		case KEY_XBUTTON2_START: szButton = "start"; break;

		case KEY_XBUTTON_STICK1:
		case KEY_XBUTTON2_STICK1: szButton = "stick_l"; break;
		case KEY_XBUTTON_STICK2:
		case KEY_XBUTTON2_STICK2: szButton = "stick_r"; break;

		case KEY_XBUTTON_LTRIGGER:
		case KEY_XBUTTON2_LTRIGGER: szButton = "trigger_l"; break;
		case KEY_XBUTTON_RTRIGGER:
		case KEY_XBUTTON2_RTRIGGER: szButton = "trigger_r"; break;

		case KEY_XBUTTON_LEFT_SHOULDER:
		case KEY_XBUTTON2_LEFT_SHOULDER: szButton = "trigger_lb"; bWide = true; break;
		case KEY_XBUTTON_RIGHT_SHOULDER:
		case KEY_XBUTTON2_RIGHT_SHOULDER: szButton = "trigger_rb"; bWide = true; break;

		case KEY_XBUTTON_UP:
		case KEY_XBUTTON2_UP: szButton = "dpad_up"; break;
		case KEY_XBUTTON_DOWN:
		case KEY_XBUTTON2_DOWN: szButton = "dpad_down"; break;
		case KEY_XBUTTON_LEFT:
		case KEY_XBUTTON2_LEFT: szButton = "dpad_left"; break;
		case KEY_XBUTTON_RIGHT:
		case KEY_XBUTTON2_RIGHT: szButton = "dpad_right"; break;
	}

	bool bDisplay = !FStrEq( szButton.c_str(), "null" );
	if ( bWide ) wide = vgui::scheme()->GetProportionalScaledValue( 30 );
	else wide = vgui::scheme()->GetProportionalScaledValue( 14 );

	m_pMicMeter->SetVisible( bDisplay );
	m_pMicMeter->SetImage( GetControllerImage( szButton.c_str() ) );
	m_pMicMeter->SetSize( wide, vgui::scheme()->GetProportionalScaledValue( 15 ) );
}

bool COptionVarItem::IsCursorMoving()
{
	int x, y;
	input()->GetCursorPos(x, y);
	if ( x == m_sx && y == m_sy ) return false;
	m_sx = x;
	m_sy = y;
	return true;
}

bool COptionVarItem::IsSpecial(const std::string& special)
{
	if ( m_szSpecial == special ) return true;
	return false;
}

void COptionVarItem::ApplyChanges(const char* cvar)
{
	// Apply our slider changes
	if ( pSlider )
	{
		pSlider->ApplyChanges();
		return;
	}

	// Apply our values
	auto cvar_ref = g_pCVar->FindVar( cvar );
	if ( cvar_ref )
	{
		if ( GetCurrentValue( m_iCurrent ) != "" )
		{
			DevMsg( 1, "Applying Convar [{green}%s{default}] with value {cyan}%i{white} - {yellow}%s\n", cvar, m_iCurrent, GetCurrentValue( m_iCurrent ).c_str() );
			cvar_ref->SetValue( GetCurrentValue( m_iCurrent ).c_str() );
		}
		else
			cvar_ref->SetValue( m_iCurrent );
		// IF voice boost
		if ( Q_stricmp( "voice_boost", cvar_ref->GetName() ) == 0 && g_pEngineVoiceTweak )
			g_pEngineVoiceTweak->SetControlFloat( MicBoost, cvar_ref->GetBool() ? 1.0f : 0.0f );
		else if ( IsSpecial( "revert_mouse" ) )
		{
			switch ( m_iCurrent )
			{
				case 0: cvar_ref->SetValue( 0.022f ); break;
				case 1: cvar_ref->SetValue( -0.022f ); break;
			}
		}
		else if ( IsSpecial( "download_filter" ) )
		{
			switch ( m_iCurrent )
			{
				case 0: cvar_ref->SetValue( "none" ); break;
				case 1: cvar_ref->SetValue( "noworkshop" ); break;
				case 2: cvar_ref->SetValue( "all" ); break;
			}
		}
		else if ( IsSpecial( "string" ) && m_pStringInput )
		{
			char buffer[512];
			m_pStringInput->GetText( buffer, sizeof(buffer) );
			cvar_ref->SetValue( buffer );
			auto ss_playername = g_pCVar->FindVar( "ss_playername" );
			if ( ss_playername )
				ss_playername->SetValue( buffer );
		}
	}

	if ( IsSpecial( "video_aa" ) )
	{
		if ( mat_aasetting.GetInt() == m_iCurrent ) return;
		mat_aasetting.SetValue( m_iCurrent );
		CGameUIConVarRef mat_antialias( "mat_antialias" );
		CGameUIConVarRef mat_aaquality( "mat_aaquality" );
		FindMSAAMode( g_m_nAAModes[ m_iCurrent ].m_nNumSamples, g_m_nAAModes[ m_iCurrent ].m_nQualityLevel );
		mat_antialias.SetValue( g_m_nAAModes[ m_iCurrent ].m_nNumSamples );
		mat_aaquality.SetValue( g_m_nAAModes[ m_iCurrent ].m_nQualityLevel );
	}
	else if ( IsSpecial( "video_lang" ) )
	{
		g_pVGuiLocalizeV2->SetNewLangage( GetCurrentValue( m_iCurrent ).c_str() );
	}
	else if ( IsSpecial( "movement_stick" ) )
	{
		auto joy_legacy = g_pCVar->FindVar( "joy_legacy" );
		auto joy_movement_stick = g_pCVar->FindVar( "joy_movement_stick" );
		switch( m_iCurrent )
		{
			case 0:
			{
				joy_legacy->SetValue( 0 );
				joy_movement_stick->SetValue( 0 );
			}
			break;

			case 1:
			{
				joy_legacy->SetValue( 0 );
				joy_movement_stick->SetValue( 1 );
			}
			break;

			case 2:
			{
				joy_legacy->SetValue( 1 );
				joy_movement_stick->SetValue( 0 );
			}
			break;

			case 3:
			{
				joy_legacy->SetValue( 1 );
				joy_movement_stick->SetValue( 1 );
			}
			break;
		}
	}
	else if ( IsSpecial( "movement_stick2" ) )
	{
		auto joy_legacy2 = g_pCVar->FindVar( "joy_legacy2" );
		auto joy_movement_stick2 = g_pCVar->FindVar( "joy_movement_stick2" );
		switch( m_iCurrent )
		{
			case 0:
			{
				joy_legacy2->SetValue( 0 );
				joy_movement_stick2->SetValue( 0 );
			}
			break;

			case 1:
			{
				joy_legacy2->SetValue( 0 );
				joy_movement_stick2->SetValue( 1 );
			}
			break;

			case 2:
			{
				joy_legacy2->SetValue( 1 );
				joy_movement_stick2->SetValue( 0 );
			}
			break;

			case 3:
			{
				joy_legacy2->SetValue( 1 );
				joy_movement_stick2->SetValue( 1 );
			}
			break;
		}
	}
}

void COptionVarItem::ApplyDefault(const char* cvar)
{
	// Apply our slider changes
	if ( pSlider )
	{
		pSlider->ApplyDefault();
		return;
	}

	// Apply our values
	auto cvar_ref = g_pCVar->FindVar( cvar );
	if ( cvar_ref )
	{
		cvar_ref->SetValue( cvar_ref->GetDefault() );
		if ( IsSpecial( "string" ) && m_pStringInput )
		{
			auto ss_playername = g_pCVar->FindVar( "ss_playername" );
			if ( ss_playername )
				ss_playername->SetValue( cvar_ref->GetDefault() );
		}
	}

	if ( IsSpecial( "video_aa" ) )
	{
		if ( mat_aasetting.GetInt() == m_iCurrent ) return;
		mat_aasetting.SetValue( mat_aasetting.GetDefault() );
		auto mat_antialias = g_pCVar->FindVar( "mat_antialias" );
		if ( mat_antialias )
			mat_antialias->SetValue( mat_antialias->GetDefault() );
		auto mat_aaquality = g_pCVar->FindVar( "mat_aaquality" );
		if ( mat_aaquality )
			mat_aaquality->SetValue( mat_aaquality->GetDefault() );
	}
	else if ( IsSpecial( "video_lang" ) )
	{
		g_pVGuiLocalizeV2->SetNewLangage( "english" );
	}
	else if ( IsSpecial( "movement_stick" ) )
	{
		auto joy_legacy = g_pCVar->FindVar( "joy_legacy" );
		auto joy_movement_stick = g_pCVar->FindVar( "joy_movement_stick" );
		joy_legacy->SetValue( joy_legacy->GetDefault() );
		joy_movement_stick->SetValue( joy_movement_stick->GetDefault() );
	}
	else if ( IsSpecial( "movement_stick2" ) )
	{
		auto joy_legacy2 = g_pCVar->FindVar( "joy_legacy2" );
		auto joy_movement_stick2 = g_pCVar->FindVar( "joy_movement_stick2" );
		joy_legacy2->SetValue( joy_legacy2->GetDefault() );
		joy_movement_stick2->SetValue( joy_movement_stick2->GetDefault() );
	}
}

int COptionVarItem::GetSelectedValue( int val ) const
{
	for ( int i = 0; i < m_list.Count(); i++ )
	{
		OptionItemValues_t item = m_list[ i ];
		if ( std::to_string( val ) == item.value )
			return i;
	}
	return val;
}

int COptionVarItem::GetSelectedValue( std::string val ) const
{
	for ( int i = 0; i < m_list.Count(); i++ )
	{
		OptionItemValues_t item = m_list[ i ];
		if ( val == item.value )
			return i;
	}
	return 0;
}

std::string COptionVarItem::GetCurrentValue( int val ) const
{
	if ( m_list.Count() <= 0 ) return "";
	OptionItemValues_t item = m_list[ m_iCurrent ];
	return item.value;
}

std::string COptionVarItem::GetCurrentName( int val ) const
{
	if ( m_list.Count() <= 0 ) return "";
	OptionItemValues_t item = m_list[ m_iCurrent ];
	return item.name;
}

void COptionVarItem::SetResolutionList(int asp, bool bWindowed)
{
	// get the currently selected resolution
	char sz[256];
	if ( m_label_value )
		m_label_value->GetText(sz, 256);
	int currentWidth = 0, currentHeight = 0;
	sscanf( sz, "%i x %i", &currentWidth, &currentHeight );

	// Clear it
	ClearList();

	// get full video mode list
	vmode_t *plist = NULL;
	int count = 0;
	gameuifuncs->GetVideoModes( &plist, &count );

	int desktopWidth, desktopHeight;
	gameuifuncs->GetDesktopResolution( desktopWidth, desktopHeight );

	const MaterialSystem_Config_t &config = materials->GetCurrentConfigForVideoCard();

	// iterate all the video modes adding them to the dropdown
	bool bFoundWidescreen = false;
	int selectedItemID = -1;
	for (int i = 0; i < count; i++, plist++)
	{
		char sz[ 256 ];
		GetResolutionName( plist, sz, sizeof( sz ) );

		// don't show modes bigger than the desktop for windowed mode
		if ( bWindowed && (plist->width > desktopWidth || plist->height > desktopHeight) )
			continue;

		int itemID = -1;
		int iAspectMode = GetScreenAspectMode( plist->width, plist->height );
		if ( iAspectMode > 0 )
			bFoundWidescreen = true;

		// filter the list for those matching the current aspect
		if ( iAspectMode == asp )
			itemID = AddItem( sz );

		// try and find the best match for the resolution to be selected
		if ( plist->width == currentWidth && plist->height == currentHeight )
			selectedItemID = itemID;
		else if ( selectedItemID == -1 && plist->width == config.m_VideoMode.m_Width && plist->height == config.m_VideoMode.m_Height )
			selectedItemID = itemID;
	}
	// IF we returned -1, become 0
	if ( selectedItemID == -1 )	selectedItemID = 0;
	m_iCurrent = selectedItemID;
	if ( m_label_value )
		m_label_value->SetText(GetCurrentName(m_iCurrent).c_str());
}

void COptionVarItem::GetResSize(int& w, int& t)
{
	char sz[256];
	if ( m_label_value )
		m_label_value->GetText(sz, 256);
	int currentWidth = 0, currentHeight = 0;
	sscanf( sz, "%i x %i", &currentWidth, &currentHeight );
	w = currentWidth;
	t = currentHeight;
}

void COptionVarItem::SetSelected(SelectedState_t state)
{
	if ( m_iSelectedState == state ) return;
	m_iSelectedState = state;

	CONTUI::NewGameOptions* self = static_cast< CONTUI::NewGameOptions * >( CONTUI::CL4DBasePanel::GetSingleton().GetWindow( CONTUI::WT_OPTIONS_NEW ) );
	if ( self )
	{
		// Desellect all items in button state if we hover over something
		if ( m_iSelectedState == STATE_HOVER )
			self->DeselectAllItems();

		if ( m_iSelectedState == STATE_NONE )
			self->OnHelpUpdate( "" );
		else
		{
			// Focus on this
			RequestFocus( 0 );
			self->OnHelpUpdate( GetName() );
			if ( IsSpecial( "phone_bg" ) )
				self->OnPhoneBackgroundChanged( m_iCurrent );
			self->OnChangeImageFrame( m_iCurrent );
		}
	}
}

void COptionVarItem::SetInputState(ButtonInputState_t state)
{
	switch ( state )
	{
		case ButtonInputState_t::BUTTON_LEFT:
		{
			if ( IsSpecial( "microphone_test" ) ) break;
			if ( IsSpecial( "phone_tune" ) ) break;
			if ( pSlider )
			{
				if ( pSlider->GetInversed() )
					pSlider->Increment( pSlider->GetStepSize() );
				else
					pSlider->Decrement( pSlider->GetStepSize() );
			}
			else
				OnCommand( "Decrease" );
		}
		break;

		case ButtonInputState_t::BUTTON_RIGHT:
		{
			if ( IsSpecial( "microphone_test" ) ) break;
			if ( IsSpecial( "phone_tune" ) ) break;
			if ( pSlider )
			{
				if ( pSlider->GetInversed() )
					pSlider->Decrement( pSlider->GetStepSize() );
				else
					pSlider->Increment( pSlider->GetStepSize() );
			}
			else
				OnCommand( "Increase" );
		}
		break;

		case ButtonInputState_t::BUTTON_PRESS:
		{
			if ( IsSpecial( "microphone_test" ) )
				OnCommand( "Increase" );
			else if ( IsSpecial( "phone_tune" ) )
				OnCommand( "Increase" );
			// Fake press
			else
			{
				if ( pHiddenButton )
					pHiddenButton->FireActionSignal();
			}
		}
		break;
	}
}

bool COptionVarItem::IsTyping()
{
	return m_pStringInput ? m_pStringInput->HasFocus() : false;
}

void COptionVarItem::SetCamTexture( COptionHiddenButton *pBtn, int id, int x, int y, int w, int h )
{
	if ( pBtn )
	{
		pBtn->SetPos( x, y );
		pBtn->SetSize( w, h );
		pBtn->MoveToFront();	// So we can press this!
	}

	if ( id > -1 )
	{
		vgui::surface()->DrawSetTexture( id );
		vgui::surface()->DrawSetColor( 255, 255, 255, 255 );
		vgui::surface()->DrawTexturedRect( x, y + 5, x + w, y + h - 5 );
	}
	if ( pBtn && pBtn->IsCursorOver() )
		vgui::surface()->DrawSetColor( 100, 100, 100, 160 );
	else
		vgui::surface()->DrawSetColor( 15, 15, 15, 160 );
	vgui::surface()->DrawFilledRect( x, y, x + w, y + h );
}

void COptionVarItem::ApplySchemeSettings(vgui::IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings( pScheme );
	DEFINE_ARROW( "vgui/cam_left", s_ArrowLeft );
	DEFINE_ARROW( "vgui/cam_right", s_ArrowRight );
}

// Specials
void COptionVarItem::StartTestMicrophone()
{
	auto voice_transmit = g_pCVar->FindVar( "voice_transmit" );
	if ( g_pEngineVoiceTweak && voice_transmit )
	{
		float val = static_cast<float>( voice_transmit->GetInt() ) / 100.0f;
		g_pEngineVoiceTweak->SetControlFloat( MicrophoneVolume, val );
	}

	if ( g_pEngineVoiceTweak && g_pEngineVoiceTweak->StartVoiceTweakMode() )
	{
		if ( m_pMicMeter )
			m_pMicMeter->SetVisible( false );

		if ( m_pMicMeter2 )
			m_pMicMeter2->SetVisible( true );

		if ( m_pMicMeterIndicator )
			m_pMicMeterIndicator->SetVisible( true );
	}
}

void COptionVarItem::EndTestMicrophone()
{
	if ( g_pEngineVoiceTweak && g_pEngineVoiceTweak->IsStillTweaking() )
		g_pEngineVoiceTweak->EndVoiceTweakMode();

	if ( m_pMicMeter )
	{
		auto cvar_ref = g_pCVar->FindVar( "voice_modenable" );
		if ( cvar_ref )
			m_pMicMeter->SetVisible( cvar_ref->GetBool() );
	}

	if ( m_pMicMeter2 )
		m_pMicMeter2->SetVisible( false );

	if ( m_pMicMeterIndicator )
		m_pMicMeterIndicator->SetVisible( false );
}
