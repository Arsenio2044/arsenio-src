//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Speedometer
//
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "hud_numericdisplay.h"
#include "iclientmode.h"
#include "iclientvehicle.h"
#include <vgui_controls/AnimationController.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include "ihudlcd.h"
#include "movevars_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// NOTE: I thought game units were inches. I have since read that
// they are 16ths of a foot, in which case this formula is wrong.

// inches/s to km/h     mm     sec in hr / 
#define INPS2KPH(X) ( X * 25.4 * 3600 / 1000000 )

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Displays current ammunition level
//-----------------------------------------------------------------------------
class CHudSpeedo : public CHudNumericDisplay, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudSpeedo, CHudNumericDisplay );

public:
	CHudSpeedo( const char *pElementName );
	void Init( void );
	void VidInit( void );
	void Reset();

	virtual void Paint( void );

protected:
	virtual void OnThink();

	void UpdateSpeedoDisplay();
	void UpdatePlayerSpeed( C_BasePlayer *player );
	
private:
	float       m_flNextTime;
	float		m_flSpeed;
};

DECLARE_HUDELEMENT( CHudSpeedo );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudSpeedo::CHudSpeedo( const char *pElementName ) : 
		BaseClass( NULL, "HudSpeedo" ), CHudElement( pElementName )
{
	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT | HIDEHUD_WEAPONSELECTION );

	hudlcd->SetGlobalStat( "(player_speed)", "0" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudSpeedo::Init( void )
{
	m_flSpeed = 0.0;
	m_flNextTime = 0.0;
	wchar_t *tempString = g_pVGuiLocalize->Find( "#Valve_Hud_SPEED" );
	if (tempString)
	{
		SetLabelText( tempString );
	}
	else
	{
		SetLabelText( L"SPEED" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudSpeedo::VidInit( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Resets hud after save/restore
//-----------------------------------------------------------------------------
void CHudSpeedo::Reset()
{
	BaseClass::Reset();
	m_flNextTime = 0.0;
	m_flSpeed = 0.0;
	UpdateSpeedoDisplay();
}

//-----------------------------------------------------------------------------
// Purpose: called every frame to get player speed
//-----------------------------------------------------------------------------
void CHudSpeedo::UpdatePlayerSpeed( C_BasePlayer *player )
{
	if (!player)
	{
		hudlcd->SetGlobalStat( "(player_speed)", "n/a" );

		SetPaintEnabled( false );
		SetPaintBackgroundEnabled( false );
		return;
	}

	SetPaintEnabled( true );
	SetPaintBackgroundEnabled( true );
	float flCurSpeed;
	// get the player speed
	if (IClientVehicle *pVehicle = player->GetVehicle() ) {
		flCurSpeed = 1.0 * pVehicle->GetCurrentSpeed();
	}
	else {
		flCurSpeed = VectorLength( player->GetLocalVelocity() );
	}
	// smoothed velocity sort of
	m_flSpeed = 0.9 * m_flSpeed + 0.1 * flCurSpeed;

	//hudlcd->SetGlobalStat( "(player_speed)", VarArgs( "%0.0f", m_flSpeed ) );
}


//-----------------------------------------------------------------------------
// Purpose: called every frame to get speed and draw speedo
//-----------------------------------------------------------------------------
void CHudSpeedo::OnThink()
{
	int rate = cl_show_speedo.GetInt();
	if (rate == 0) {
		this->SetVisible( false );
		return;
	} 
	this->SetVisible( true );

	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	UpdatePlayerSpeed( player );

	if (gpGlobals->curtime >= m_flNextTime)
	{
		UpdateSpeedoDisplay();
		m_flNextTime = m_flNextTime + (1.0 / rate);
	}


}

//-----------------------------------------------------------------------------
// Purpose: updates the ammo display counts
//-----------------------------------------------------------------------------
void CHudSpeedo::UpdateSpeedoDisplay()
{
	

	SetDisplayValue( Floor2Int( INPS2KPH( m_flSpeed ) ) );
}


//-----------------------------------------------------------------------------
// Purpose: We add an icon into the 
//-----------------------------------------------------------------------------
void CHudSpeedo::Paint( void )
{
	BaseClass::Paint();
}
