//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include <vgui/ISurface.h>
#include "hud_numericdisplay.h"
#include "iclientmode.h"
#include "gamerules.h"
#include "hl2_gamerules.h" // ?
#include "c_playerresource.h"
#include <coordsize.h>
#include "hud_macros.h"
#include "vgui/IVGui.h"
#include "vgui/ILocalize.h"
#include "mapoverview.h"
#include "hud_radar_panel.h"

// Credit to Halo: Source for the code.


#define RADAR_DOT_NORMAL		0
#define RADAR_DOT_ENEMY			(1<<0)
#define RADAR_IGNORE_Z			(1<<6)	//always draw this item as if it was at the same Z as the player

ConVar cl_radartype( "cl_radartype", "0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE );
ConVar cl_radaralpha( "cl_radaralpha", "200", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, NULL, true, 0, true, 255 );

DECLARE_HUDELEMENT( CHudRadarPanel );
DECLARE_HUD_MESSAGE( CHudRadarPanel, UpdateRadar );

static CHudRadarPanel *s_Radar = NULL;
CUtlVector<CPlayerRadarFlash> g_RadarFlashes;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudRadarPanel::CHudRadarPanel( const char *pName ) : Panel( NULL, "HudRadar" ), CHudElement( pName )
{
	SetParent( g_pClientMode->GetViewport() );

	m_pBackground = NULL;
	m_pBackgroundTrans = NULL;

	m_flNextBombFlashTime = 0.0;
	m_bBombFlash = true;

	m_flNextHostageFlashTime = 0.0;
	m_bHostageFlash = true;
	m_bHideRadar = false;

	s_Radar = this;

	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT );
}

//---------------------------------------------------------
// Purpose: 
//---------------------------------------------------------
CHudRadarPanel::~CHudRadarPanel()
{
	s_Radar = NULL;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudRadarPanel::Init()
{
	HOOK_HUD_MESSAGE( CHudRadarPanel, UpdateRadar );
}

void CHudRadarPanel::LevelInit()
{
	m_flNextBombFlashTime = 0.0;
	m_bBombFlash = true;

	m_flNextHostageFlashTime = 0.0;
	m_bHostageFlash = true;

	g_RadarFlashes.RemoveAll();

	// Map Overview handles radar duties now.
	if( g_pMapOverview )
	{
		g_pMapOverview->SetMode(CMapOverview::MAP_MODE_RADAR);
	}
}

//---------------------------------------------------------
// Purpose: 
//---------------------------------------------------------
void CHudRadarPanel::Reset()
{
	CHudElement::Reset();

	if (g_pMapOverview)
	{
		g_pMapOverview->SetMode(CMapOverview::MAP_MODE_RADAR);
	}
}

//---------------------------------------------------------
// Purpose: 
//---------------------------------------------------------
void CHudRadarPanel::MsgFunc_UpdateRadar(bf_read &msg )
{
	int iPlayerEntity = msg.ReadByte();

	// Draw objects on the radar
	C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();

	if ( !pLocalPlayer )
	{
		return;
	}

	C_PlayerResource *pPR = (C_PlayerResource*)GameResources();

	if ( !pPR )
	{
		return;
	}

	// Players
	for(int i=1;i<=MAX_PLAYERS;i++)
	{
		if( i == pLocalPlayer->entindex() )
		{
			continue;
		}

		C_BasePlayer *pPlayer = ToBasePlayer(UTIL_PlayerByIndex(i));

		if ( pPlayer )
		{
			pPlayer->m_bDetected = false;
		}
	}

	while ( iPlayerEntity > 0 )
	{
		int x = msg.ReadSBitLong( COORD_INTEGER_BITS-1 ) * 4;
		int y = msg.ReadSBitLong( COORD_INTEGER_BITS-1 ) * 4;
		int z = msg.ReadSBitLong( COORD_INTEGER_BITS-1 ) * 4;
		int a = msg.ReadSBitLong( 9 );

		C_BasePlayer *pPlayer = ToBasePlayer(UTIL_PlayerByIndex(iPlayerEntity));

		Vector origin( x, y, z );
		QAngle angles( 0, a, 0 );

		if ( g_pMapOverview )
		{
			g_pMapOverview->SetPlayerPositions( iPlayerEntity-1, origin, angles );
		}

		iPlayerEntity = msg.ReadByte(); // Read index for next player

		if ( !pPlayer )
		{
			continue;
		}

		bool bOppositeTeams = (pLocalPlayer->GetTeamNumber() != TEAM_UNASSIGNED && pPR->GetTeam( pPlayer->entindex() ) != pLocalPlayer->GetTeamNumber());

		// Don't update dead players or if they are in PVS
		if ( pPlayer->IsObserver() || (!pPlayer->IsDormant() && bOppositeTeams == false ) )
		{
			continue;
		}

		// Update origin and angle for players out of my PVS
		origin = pPlayer->GetAbsOrigin();
		angles = pPlayer->GetAbsAngles();

		origin.x = x;
		origin.y = y;
		angles.y = a;

		pPlayer->SetAbsOrigin( origin );
		pPlayer->SetAbsAngles( angles );
		pPlayer->m_bDetected = true;
	}
}

//---------------------------------------------------------
// Purpose: 
//---------------------------------------------------------
bool CHudRadarPanel::ShouldDraw()
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	return pPlayer && pPlayer->IsAlive() && !m_bHideRadar;
}

//---------------------------------------------------------
// Purpose: 
//---------------------------------------------------------
void CHudRadarPanel::SetVisible(bool state)
{
	BaseClass::SetVisible(state);

	if (g_pMapOverview  &&  g_pMapOverview->GetMode() == CMapOverview::MAP_MODE_RADAR)
	{
		// We are the hud element still, but he is in charge of the new style now.
		g_pMapOverview->SetVisible(state);
	}
}

//---------------------------------------------------------
// Purpose: 
//---------------------------------------------------------
void CHudRadarPanel::Paint()
{
	// We are the hud element still, but Overview is in charge of the new style now.
	return;
}

//---------------------------------------------------------
// Purpose: 
//---------------------------------------------------------
void CHudRadarPanel::WorldToRadar( const Vector location, const Vector origin, const QAngle angles, float &x, float &y, float &z_delta )
{
	float x_diff = location.x - origin.x;
	float y_diff = location.y - origin.y;
 
	int iRadarRadius = GetWide();		// Width of the panel, it resizes now!
	float fRange = 40 * iRadarRadius;	// Radar's range

	float flOffset = atan(y_diff/x_diff);
	flOffset *= 180;
	flOffset /= M_PI;

	if ((x_diff < 0) && (y_diff >= 0))
	{
		flOffset = 180 + flOffset;
	}
	else if ((x_diff < 0) && (y_diff < 0))
	{
		flOffset = 180 + flOffset;
	}
	else if ((x_diff >= 0) && (y_diff < 0))
	{
		flOffset = 360 + flOffset;
	}

	y_diff = -1*(sqrt((x_diff)*(x_diff) + (y_diff)*(y_diff)));
	x_diff = 0;

	flOffset = angles.y - flOffset;

	flOffset *= M_PI;
	flOffset /= 180;		// Now theta is in radians

	float xnew_diff = x_diff * cos(flOffset) - y_diff * sin(flOffset);
	float ynew_diff = x_diff * sin(flOffset) + y_diff * cos(flOffset);

	// The dot is out of the radar's range.. Scale it back so that it appears on the border
	if ( (-1 * y_diff) > fRange )
	{
		float flScale;

		flScale = ( -1 * y_diff) / fRange;

		xnew_diff /= flScale;
		ynew_diff /= flScale;
	}
	xnew_diff /= 32;
	ynew_diff /= 32;

	// Output
	x = (iRadarRadius/2) + (int)xnew_diff;
	y = (iRadarRadius/2) + (int)ynew_diff;
	z_delta = location.z - origin.z;
}

//---------------------------------------------------------
// Purpose: 
//---------------------------------------------------------
void CHudRadarPanel::DrawPlayerOnRadar( int iPlayer, C_BasePlayer *pLocalPlayer )
{
	float x, y, z_delta;
	int iBaseDotSize = ScreenWidth() / 256;	
	int r, g, b, a = 235;

	C_PlayerResource *pPR = (C_PlayerResource*)GameResources();

	if ( !pPR )
	{
		return;
	}

	C_BasePlayer *pPlayer = ToBasePlayer( UTIL_PlayerByIndex( iPlayer ) );

	if ( !pPlayer )
	{
		return;
	}

	bool bOppositeTeams = (pLocalPlayer->GetTeamNumber() != TEAM_UNASSIGNED && pPR->GetTeam( iPlayer ) != pLocalPlayer->GetTeamNumber());

	if ( bOppositeTeams && pPlayer->m_bDetected == false )
	{
		return;
	}
	
	WorldToRadar( pPlayer->GetAbsOrigin(), pLocalPlayer->GetAbsOrigin(), pLocalPlayer->LocalEyeAngles(), x, y, z_delta );

	if( bOppositeTeams )
	{
		r = 250; g = 0; b = 0;
	}
	else if ( 0 /*m_bTrackArray[i-1] == true */ ) // Tracked players (friends we want to keep track of on the radar)
	{
		iBaseDotSize *= 2;
		r = 185; g = 20; b = 20;
	}
	else
	{
		r = 75; g = 75; b = 250;
	}

	// Handle the radio flashes
	bool bRadarFlash = false;
	if ( g_RadarFlashes.Count() > iPlayer )
	{
		bRadarFlash = g_RadarFlashes[iPlayer].m_bRadarFlash && g_RadarFlashes[iPlayer].m_iNumRadarFlashes > 0;
	}	

	if ( bRadarFlash )
	{
		r = 230; g = 110; b = 25; a = 245; 

		DrawRadarDot( x, y, z_delta, iBaseDotSize, RADAR_DOT_ENEMY, r, g, b, a );
	}
	else
	{
		DrawRadarDot( x, y, z_delta, iBaseDotSize, RADAR_DOT_NORMAL, r, g, b, a );
	}
}

//---------------------------------------------------------
// Purpose: 
//---------------------------------------------------------
void CHudRadarPanel::DrawEntityOnRadar(CBaseEntity *pEnt, C_BasePlayer *pLocalPlayer, int flags, int r, int g, int b, int a)
{
	float x, y, z_delta;
	int iBaseDotSize = 4;

	WorldToRadar(pEnt->GetAbsOrigin(), pLocalPlayer->GetAbsOrigin(), pLocalPlayer->LocalEyeAngles(), x, y, z_delta);

	if (flags & RADAR_IGNORE_Z)
	{
		z_delta = 0;
	}

	DrawRadarDot(x, y, z_delta, iBaseDotSize, flags, r, g, b, a);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudRadarPanel::FillRect(int x, int y, int w, int h)
{
	int panel_x, panel_y, panel_w, panel_h;
	GetBounds(panel_x, panel_y, panel_w, panel_h);
	surface()->DrawFilledRect(x, y, x + w, y + h);
}

//---------------------------------------------------------
// Purpose:
//---------------------------------------------------------
void CHudRadarPanel::DrawRadarDot( int x, int y, float z_diff, int iBaseDotSize, int flags, int r, int g, int b, int a )
{
	surface()->DrawSetColor( r, g, b, a );

	if ( flags & RADAR_DOT_ENEMY )
	{
		FillRect( x-1, y-1, iBaseDotSize+1, iBaseDotSize+1 );
	}
	else if ( z_diff < -128 ) // Below the player
	{
		z_diff *= -1;

		if ( z_diff > 3096 )
		{
			z_diff = 3096;
		}

		int iBar = (int)( z_diff / 400 ) + 2;

		// Draw an upside-down T shape to symbolize the dot is below the player.

		iBaseDotSize /= 2;

		// Horizontal
		FillRect( x-(2*iBaseDotSize), y, 5*iBaseDotSize, iBaseDotSize );

		// Vertical
		FillRect( x, y - iBar*iBaseDotSize, iBaseDotSize, iBar*iBaseDotSize );
	}
	else if ( z_diff > 128 ) // Above the player
	{
		if ( z_diff > 3096 )
		{
			z_diff = 3096;
		}

		int iBar = (int)( z_diff / 400 ) + 2;

		iBaseDotSize /= 2;
		
		// Draw a T shape to symbolize the dot is above the player.

		// Horizontal
		FillRect( x-(2*iBaseDotSize), y, 5*iBaseDotSize, iBaseDotSize );

		// Vertical
		FillRect( x, y, iBaseDotSize, iBar*iBaseDotSize );
	}
	else 
	{
		if ( flags & RADAR_DOT_ENEMY )
		{
			FillRect( x-1, y-1, iBaseDotSize+1, iBaseDotSize+1 );
		}
		else if ( flags & RADAR_DOT_ENEMY )
		{
			if ( flags & RADAR_DOT_ENEMY )
			{
				iBaseDotSize = 2;
				// Draw an X for the planted bomb
				FillRect( x, y, iBaseDotSize, iBaseDotSize );
				FillRect( x-2, y-2, iBaseDotSize, iBaseDotSize );
				FillRect( x-2, y+2, iBaseDotSize, iBaseDotSize );	
				FillRect( x+2, y-2, iBaseDotSize, iBaseDotSize );
				FillRect( x+2, y+2, iBaseDotSize, iBaseDotSize );
			}
			else
			{
				FillRect( x-1, y-1, iBaseDotSize+1, iBaseDotSize+1 );
			}
		}
		else
		{
			FillRect( x, y, iBaseDotSize, iBaseDotSize );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Radar_FlashPlayer( int iPlayer )
{
	if ( g_RadarFlashes.Count() <= iPlayer )
	{
		g_RadarFlashes.AddMultipleToTail( iPlayer - g_RadarFlashes.Count() + 1 );
	}

	CPlayerRadarFlash *pFlash = &g_RadarFlashes[iPlayer];
	pFlash->m_flNextRadarFlashTime = gpGlobals->curtime;
	pFlash->m_iNumRadarFlashes = 16;
	pFlash->m_bRadarFlash = false;

	g_pMapOverview->FlashEntity(iPlayer);
}

CON_COMMAND( drawradar, "Draws HUD radar" )
{
	(GET_HUDELEMENT( CHudRadarPanel ))->DrawRadar();
}

CON_COMMAND( hideradar, "Hides HUD radar" )
{
	(GET_HUDELEMENT( CHudRadarPanel ))->HideRadar();
}