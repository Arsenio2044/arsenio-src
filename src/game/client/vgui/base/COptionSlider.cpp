#include "COptionSlider.h"
#include "COptionVarItem.h"

#include <vgui/ISurface.h>
#include <vgui/IInput.h>

#include "L4D360UI/NewGameOptions.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

COptionSlider::COptionSlider( Panel* parent, const char* szName ) :
	BaseClass( parent, szName )
{
	m_cvar = "";
	m_focusColor = Color( 255, 255, 255 );
	m_unfocusColor = Color( 100, 100, 100 );
	m_bDragging = false;
	m_bDisplayValue = false;
	m_bDisplayInFloat = false;
	m_iFloatFormat = 0;
	m_min = 0.0f;
	m_max = 1.0f;
	m_curValue = 0.5f;
	m_stepSize = 1.0f;
}

COptionSlider::~COptionSlider()
{
	SetConCommand( nullptr );
}

void COptionSlider::SetCurrentValue(float value, bool bReset)
{
	float fNewValue = MAX( MIN( value, GetMax() ), GetMin() );

	// If we're just resetting the value don't play sound effects or ignore the same value being set
	if ( !bReset )
	{
		if ( fNewValue == m_curValue )
		{
			CONTUI::CL4DBasePanel::GetSingleton().PlayUISound( CONTUI::UISOUND_INVALID );
			return;
		}

		CONTUI::CL4DBasePanel::GetSingleton().PlayUISound( CONTUI::UISOUND_CLICK );
	}

	m_curValue = fNewValue;

	UpdateProgressBar();
}

float COptionSlider::Increment(float stepSize)
{
	SetCurrentValue( GetCurrentValue() + stepSize );
	return GetCurrentValue();
}

float COptionSlider::Decrement(float stepSize)
{
	SetCurrentValue( GetCurrentValue() - stepSize );
	return GetCurrentValue();
}

void COptionSlider::OnMousePressed(vgui::MouseCode code)
{
	switch ( code )
	{
	case MOUSE_LEFT:
		HandleMouseInput( true );
		break;
	}
}

void COptionSlider::OnMouseReleased(vgui::MouseCode code)
{
	switch ( code )
	{
	case MOUSE_LEFT:
		m_bDragging = false;
		break;
	}
}

void COptionSlider::OnCursorEntered()
{
	BaseClass::OnCursorEntered();
	CONTUI::CL4DBasePanel::GetSingleton().PlayUISound( CONTUI::UISOUND_FOCUS );
	if( GetParent() )
		GetParent()->NavigateToChild( this );
	else
		NavigateTo();
}

void COptionSlider::OnCursorExited()
{
	BaseClass::OnCursorExited();
	if ( !m_bDragging )
		NavigateFrom();
}

void COptionSlider::HandleMouseInput(bool bDrag)
{
	// See if we clicked within the bounds of the slider bar or are already dragging it
	if ( m_bDragging || IsCursorOver() )
	{
		float fMin = GetMin();
		float fMax = GetMax();
		int iSliderWide = GetWide();

		if ( iSliderWide > 0 )
		{
			int iClickPosX;
			int iClickPosY;

			vgui::input()->GetCursorPos( iClickPosX, iClickPosY );

			int iSliderPosX, iSliderPosY;
			GetPos( iSliderPosX, iSliderPosY );

			float flRelPos = (float)(iClickPosX - iSliderPosX);
			float fNormalizedPosition = clamp( static_cast<float>( flRelPos ) / static_cast<float>( iSliderWide ), 0.0f, 1.0f );

			if ( GetInversed() )
			{
				fNormalizedPosition = 1.0f - fNormalizedPosition;
			}

			SetCurrentValue( fNormalizedPosition * ( fMax - fMin ) + fMin, true );

			if ( bDrag )
			{
				m_bDragging = true;
			}
		}
	}
}

void COptionSlider::ValidateMousePos()
{
	if ( !m_bDragging ) return;
	if ( !IsCursorOver() )
	{
		OnCursorExited();
		m_bDragging = false;
	}
}

void COptionSlider::Paint()
{
	BaseClass::Paint();

	// Are we draggin or not?
	ValidateMousePos();

	// Handle our mouse input
	if ( m_bDragging )
		HandleMouseInput( true );

	// Draw our background and borders
	DrawBackground();

	// Paint our bar
	DrawProgressBar();

	// Tell our parent, the COptionVarItem, to draw our label!
	if ( m_bDisplayValue )
	{
		COptionVarItem *pParent = dynamic_cast<COptionVarItem *>( GetParent() );
		if ( !pParent ) return;
		wchar_t fovText[64];
		if ( m_bDisplayInFloat )
		{
			switch ( m_iFloatFormat )
			{
				case -1:
					Q_snwprintf( fovText, sizeof( fovText ), L"(%f)", GetCurrentValue() );
				break;

				case 1:
					Q_snwprintf( fovText, sizeof( fovText ), L"(%.2f)", GetCurrentValue() );
				break;

				case 2:
					Q_snwprintf( fovText, sizeof( fovText ), L"(%.3f)", GetCurrentValue() );
				break;

				case 3:
					Q_snwprintf( fovText, sizeof( fovText ), L"(%.4f)", GetCurrentValue() );
				break;

				default:
					Q_snwprintf( fovText, sizeof( fovText ), L"(%.1f)", GetCurrentValue() );
				break;
			}
		}
		else
			Q_snwprintf( fovText, sizeof( fovText ), L"(%i)", (int)GetCurrentValue() );
		pParent->SetValueText( fovText );
	}
}

void COptionSlider::DrawBorders()
{
	vgui::surface()->DrawSetColor( 150, 150, 150, 160 );
	// <------> (left > right | top border)
	vgui::surface()->DrawFilledRect( 0, 0, 0 + GetWide(), 1 );
	/* /\
	*  ||
	*  || (left > bottom | left border)
	*  ||
	*  \/
	*/
	vgui::surface()->DrawFilledRect( 0, 0, 1, 0 + GetTall() );

	// <------> (left > right | bottom border)
	int yb = GetTall() - 1;
	vgui::surface()->DrawFilledRect( 0, yb, 0 + GetWide(), yb + 1 );
	/* /\
	*  ||
	*  || (right > bottom | right border)
	*  ||
	*  \/
	*/
	int xb = GetWide() - 1;
	vgui::surface()->DrawFilledRect( xb, 0, xb + 1, 0 + GetTall() );
}

void COptionSlider::DrawBackground()
{
	DrawBorders();

	if ( IsCursorOver() )
		vgui::surface()->DrawSetColor( 25, 25, 25, 160 );
	else
		vgui::surface()->DrawSetColor( 15, 15, 15, 160 );
	vgui::surface()->DrawFilledRect( 1, 1, 0 + GetWide() - 1, 0 + GetTall() - 1 );
}

void COptionSlider::DrawProgressBar()
{
	if ( IsCursorOver() )
		vgui::surface()->DrawSetColor( m_focusColor.r(), m_focusColor.g(), m_focusColor.b(), 160 );
	else
		vgui::surface()->DrawSetColor( m_unfocusColor.r(), m_unfocusColor.r(), m_unfocusColor.r(), 160 );

	int wide = GetWide() - 1;
	int draw_bar = (int)(m_flprogress * wide);
	vgui::surface()->DrawFilledRect( 1, 1, 0 + draw_bar, 0 + GetTall() - 1 );
}

void COptionSlider::SetConCommand(const char* conCommand)
{
	if ( conCommand )
	{
		m_cvar = conCommand;
		Reset();
	}
	else
		m_cvar = "";
}

void COptionSlider::SetMin(float min)
{
	m_min = min;
	SetCurrentValue( GetCurrentValue(), true ); //make sure that the current value doesn't go out of bounds
}

void COptionSlider::SetMax(float max)
{
	m_max = max;
	SetCurrentValue( GetCurrentValue(), true ); //make sure that the current value doesn't go out of bounds
}

float COptionSlider::UpdateProgressBar()
{
	float percentage = ( GetCurrentValue() - GetMin() ) / ( GetMax() - GetMin() );
	if ( GetInversed() )
		percentage = 1.0f - percentage;

	if (percentage != m_flprogress)
	{
		m_flprogress = percentage;
		CONTUI::NewGameOptions* self = static_cast< CONTUI::NewGameOptions * >( CONTUI::CL4DBasePanel::GetSingleton().GetWindow( CONTUI::WT_OPTIONS_NEW ) );
		if ( self )
		{
			self->SetRequiresRestart( m_RestartRequired );
			self->SetValueChanged( true );
		}
	}

	return percentage;
}

void COptionSlider::Reset()
{
	auto cvar_ref = g_pCVar->FindVar( m_cvar.c_str() );
	if ( cvar_ref )
		SetCurrentValue( cvar_ref->GetFloat(), true );
}

void COptionSlider::ApplyChanges()
{
	auto cvar_ref = g_pCVar->FindVar( m_cvar.c_str() );
	if ( cvar_ref )
		cvar_ref->SetValue( GetCurrentValue() );
}

void COptionSlider::ApplyDefault()
{
	auto cvar_ref = g_pCVar->FindVar( m_cvar.c_str() );
	if ( cvar_ref )
		cvar_ref->SetValue( cvar_ref->GetDefault() );
}
