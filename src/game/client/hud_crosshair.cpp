//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "hud_crosshair.h"
#include "iclientmode.h"
#include "view.h"
#include "vgui_controls/Controls.h"
#include "vgui/ISurface.h"
#include "ivrenderview.h"
#include "materialsystem/imaterialsystem.h"
#include "VGuiMatSurface/IMatSystemSurface.h"
#include "client_virtualreality.h"
#include "sourcevr/isourcevirtualreality.h"

#ifdef SIXENSE
#include "sixense/in_sixense.h"
#endif

#ifdef PORTAL
#include "c_portal_player.h"
#endif // PORTAL

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar crosshair( "crosshair", "1", FCVAR_ARCHIVE );
ConVar cl_observercrosshair( "cl_observercrosshair", "1", FCVAR_ARCHIVE );

ConVar cl_hitmarks( "cl_hitmarks", "1", FCVAR_ARCHIVE );
ConVar cl_hitmark_fadetime( "cl_hitmark_fadetime", "0.6", FCVAR_ARCHIVE );
ConVar cl_hitmark_scale( "cl_hitmark_scale", "1.0", FCVAR_ARCHIVE );

ConVar cl_shotgun_hitmarks( "cl_shotgun_hitmarks", "1", FCVAR_ARCHIVE );
ConVar cl_shotgun_hitmarksize( "cl_shotgun_hitmarksize", "5", FCVAR_ARCHIVE, "Size of shotgun hitmarkers (1 is smallest, 6 is biggest)" );
ConVar cl_shotgun_hitmarkspread( "cl_shotgun_hitmarkspread", "1.0", FCVAR_ARCHIVE, "Scale of shotgun hitmark spread" );
ConVar cl_shotgun_hitmarkoffset( "cl_shotgun_hitmarkoffset", "0.15", FCVAR_ARCHIVE, "Vertical offset of hitmarks (-1 to 1)" );
ConVar cl_horizon( "cl_horizon", "1", FCVAR_ARCHIVE );

using namespace vgui;

int ScreenTransform( const Vector& point, Vector& screen );

#ifdef TF_CLIENT_DLL
// If running TF, we use CHudTFCrosshair instead (which is derived from CHudCrosshair)
#else
DECLARE_HUDELEMENT( CHudCrosshair );
#endif

CHudCrosshair::CHudCrosshair( const char *pElementName ) :
		CHudElement( pElementName ), BaseClass( NULL, "HudCrosshair" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	m_pCrosshair = 0;

	m_clrCrosshair = Color( 0, 0, 0, 0 );

	m_vecCrossHairOffsetAngle.Init();

	SetHiddenBits( HIDEHUD_PLAYERDEAD | HIDEHUD_CROSSHAIR );
}

CHudCrosshair::~CHudCrosshair()
{
}

void CHudCrosshair::ApplySchemeSettings( IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );

	m_pDefaultCrosshair = gHUD.GetIcon("crosshair_default");
	SetPaintBackgroundEnabled( false );

    SetSize( ScreenWidth(), ScreenHeight() );

	SetForceStereoRenderToFrameBuffer( true );
}

//-----------------------------------------------------------------------------
// Purpose: Save CPU cycles by letting the HUD system early cull
// costly traversal.  Called per frame, return true if thinking and 
// painting need to occur.
//-----------------------------------------------------------------------------
bool CHudCrosshair::ShouldDraw( void )
{
	bool bNeedsDraw;

	if ( m_bHideCrosshair )
		return false;

	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
		return false;

	C_BaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();
	if ( pWeapon && !pWeapon->ShouldDrawCrosshair() )
		return false;

#ifdef PORTAL
	C_Portal_Player *portalPlayer = ToPortalPlayer(pPlayer);
	if ( portalPlayer && portalPlayer->IsSuppressingCrosshair() )
		return false;
#endif // PORTAL

	/* disabled to avoid assuming it's an HL2 player.
	// suppress crosshair in zoom.
	if ( pPlayer->m_HL2Local.m_bZooming )
		return false;
	*/

	// draw a crosshair only if alive or spectating in eye
	if ( IsX360() )
	{
		bNeedsDraw = m_pCrosshair && 
			!engine->IsDrawingLoadingImage() &&
			!engine->IsPaused() && 
			( !pPlayer->IsSuitEquipped() || g_pGameRules->IsMultiplayer() ) &&
			g_pClientMode->ShouldDrawCrosshair() &&
			!( pPlayer->GetFlags() & FL_FROZEN ) &&
			( pPlayer->entindex() == render->GetViewEntity() ) &&
			( pPlayer->IsAlive() ||	( pPlayer->GetObserverMode() == OBS_MODE_IN_EYE ) || ( cl_observercrosshair.GetBool() && pPlayer->GetObserverMode() == OBS_MODE_ROAMING ) );
	}
	else
	{
		bNeedsDraw = m_pCrosshair && 
			crosshair.GetInt() &&
			!engine->IsDrawingLoadingImage() &&
			!engine->IsPaused() && 
			g_pClientMode->ShouldDrawCrosshair() &&
			!( pPlayer->GetFlags() & FL_FROZEN ) &&
			( pPlayer->entindex() == render->GetViewEntity() ) &&
			!pPlayer->IsInVGuiInputMode() &&
			( pPlayer->IsAlive() ||	( pPlayer->GetObserverMode() == OBS_MODE_IN_EYE ) || ( cl_observercrosshair.GetBool() && pPlayer->GetObserverMode() == OBS_MODE_ROAMING ) );
	}

	return ( bNeedsDraw && CHudElement::ShouldDraw() );
}

#ifdef TF_CLIENT_DLL
extern ConVar cl_crosshair_red;
extern ConVar cl_crosshair_green;
extern ConVar cl_crosshair_blue;
extern ConVar cl_crosshair_scale;
#endif


void CHudCrosshair::GetDrawPosition ( float *pX, float *pY, bool *pbBehindCamera, QAngle angleCrosshairOffset )
{
	QAngle curViewAngles = CurrentViewAngles();
	Vector curViewOrigin = CurrentViewOrigin();

	int vx, vy, vw, vh;
	vgui::surface()->GetFullscreenViewport( vx, vy, vw, vh );

	float screenWidth = vw;
	float screenHeight = vh;

	float x = screenWidth / 2;
	float y = screenHeight / 2;

	bool bBehindCamera = false;

	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( ( pPlayer != NULL ) && ( pPlayer->GetObserverMode()==OBS_MODE_NONE ) )
	{
		bool bUseOffset = false;
		
		Vector vecStart;
		Vector vecEnd;

		if ( UseVR() )
		{
			// These are the correct values to use, but they lag the high-speed view data...
			vecStart = pPlayer->Weapon_ShootPosition();
			Vector vecAimDirection = pPlayer->GetAutoaimVector( 1.0f );
			// ...so in some aim modes, they get zapped by something completely up-to-date.
			g_ClientVirtualReality.OverrideWeaponHudAimVectors ( &vecStart, &vecAimDirection );
			vecEnd = vecStart + vecAimDirection * MAX_TRACE_LENGTH;

			bUseOffset = true;
		}

#ifdef SIXENSE
		// TODO: actually test this Sixsense code interaction with things like HMDs & stereo.
        if ( g_pSixenseInput->IsEnabled() && !UseVR() )
		{
			// Never autoaim a predicted weapon (for now)
			vecStart = pPlayer->Weapon_ShootPosition();
			Vector aimVector;
			AngleVectors( CurrentViewAngles() - g_pSixenseInput->GetViewAngleOffset(), &aimVector );
			// calculate where the bullet would go so we can draw the cross appropriately
			vecEnd = vecStart + aimVector * MAX_TRACE_LENGTH;
			bUseOffset = true;
		}
#endif

		if ( bUseOffset )
		{
			trace_t tr;
			UTIL_TraceLine( vecStart, vecEnd, MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr );

			Vector screen;
			screen.Init();
			bBehindCamera = ScreenTransform(tr.endpos, screen) != 0;

			x = 0.5f * ( 1.0f + screen[0] ) * screenWidth + 0.5f;
			y = 0.5f * ( 1.0f - screen[1] ) * screenHeight + 0.5f;
		}
	}

	// MattB - angleCrosshairOffset is the autoaim angle.
	// if we're not using autoaim, just draw in the middle of the 
	// screen
	if ( angleCrosshairOffset != vec3_angle )
	{
		QAngle angles;
		Vector forward;
		Vector point, screen;

		// this code is wrong
		angles = curViewAngles + angleCrosshairOffset;
		AngleVectors( angles, &forward );
		VectorAdd( curViewOrigin, forward, point );
		ScreenTransform( point, screen );

		x += 0.5f * screen[0] * screenWidth + 0.5f;
		y += 0.5f * screen[1] * screenHeight + 0.5f;
	}

	*pX = x;
	*pY = y;
	*pbBehindCamera = bBehindCamera;
}


void CHudCrosshair::Paint( void )
{
	if ( !m_pCrosshair )
		return;

	if ( !IsCurrentViewAccessAllowed() )
		return;

	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
		return;

	float x, y;
	bool bBehindCamera;
	GetDrawPosition ( &x, &y, &bBehindCamera, m_vecCrossHairOffsetAngle );

	if( bBehindCamera )
		return;

	float flWeaponScale = 1.f;
	int iTextureW = m_pCrosshair->Width();
	int iTextureH = m_pCrosshair->Height();
	C_BaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();
	if ( pWeapon )
	{
		pWeapon->GetWeaponCrosshairScale( flWeaponScale );
	}

	float flPlayerScale = 1.0f;
#ifdef TF_CLIENT_DLL
	Color clr( cl_crosshair_red.GetInt(), cl_crosshair_green.GetInt(), cl_crosshair_blue.GetInt(), 255 );
	flPlayerScale = cl_crosshair_scale.GetFloat() / 32.0f;  // the player can change the scale in the options/multiplayer tab
#else
	Color clr = m_clrCrosshair;
#endif
	float flWidth = flWeaponScale * flPlayerScale * (float)iTextureW;
	float flHeight = flWeaponScale * flPlayerScale * (float)iTextureH;
	int iWidth = (int)( flWidth + 0.5f );
	int iHeight = (int)( flHeight + 0.5f );
	int iX = (int)( x + 0.5f );
	int iY = (int)( y + 0.5f );

	m_pCrosshair->DrawSelfCropped (
		iX-(iWidth/2), iY-(iHeight/2),
		0, 0,
		iTextureW, iTextureH,
		iWidth, iHeight,
		clr );

	// Are we doing hitmarkers at all?
	if (cl_hitmarks.GetInt() > 0)
	    PaintHitMarks(iWidth, iHeight);

	if (cl_horizon.GetInt() > 0)
		PaintHorizon( iWidth, iHeight );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudCrosshair::SetCrosshairAngle( const QAngle& angle )
{
	VectorCopy( angle, m_vecCrossHairOffsetAngle );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudCrosshair::SetCrosshair( CHudTexture *texture, const Color& clr )
{
	m_pCrosshair = texture;
	m_clrCrosshair = clr;
}

//-----------------------------------------------------------------------------
// Purpose: Resets the crosshair back to the default
//-----------------------------------------------------------------------------
void CHudCrosshair::ResetCrosshair()
{
	SetCrosshair( m_pDefaultCrosshair, Color(255, 255, 255, 255) );
}

//-----------------------------------------------------------------------------
// Purpose: Paint some red lines near the crosshair to show shots hitting
//-----------------------------------------------------------------------------
void CHudCrosshair::PaintHitMarks(const int cross_w, const int cross_h)
{

	// How long since the last hit? Ask the weapon
	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;
	C_BaseCombatWeapon* pWeapon = pPlayer->GetActiveWeapon();
	if (!pWeapon)
		return;
	float flLastHitTime = pWeapon->m_flLastHitTime; 

	// Was the last hit within the fade time?
	if (flLastHitTime + cl_hitmark_fadetime.GetFloat() < gpGlobals->curtime)
		return;

	// fadegress is fraction of progress through fade time, 0.0 - 1.0.
	// rapidly move out until turn time, then hold for a bit and slowly slide back in
	// as you fade out
	float fadegress = (gpGlobals->curtime - flLastHitTime) / MAX(cl_hitmark_fadetime.GetFloat(), 0.01f);

	const float turntime = 0.08; 

	int vx, vy, vw, vh;
	vgui::surface()->GetFullscreenViewport( vx, vy, vw, vh );
	const int halfw = vw / 2;
	const int halfh = vh / 2;
	const int minrad = RoundFloatToInt( cl_hitmark_scale.GetFloat() * cross_h / 8 );
	const int maxrad = RoundFloatToInt( cl_hitmark_scale.GetFloat() * 1.5 * minrad );
	const int maxlen = RoundFloatToInt( cl_hitmark_scale.GetFloat() * cross_w / 4);


	// Draw four red lines radiating out from the crosshair to the corners

	// We can use the same code to draw each line, we just need
	// a multiplier for each quadrant, i.e. (-1,-1),(1, -1),(-1,1),(1,1)
	int quadrant, quadx, quady;
	int startx, starty, endx, endy, alpha, basealpha;

	basealpha = cl_hitmarks.GetInt() > 1 ? cl_hitmarks.GetInt() : 255;
	for ( quadrant = 0; quadrant < 4; quadrant++ )
	{
		quadx = ((quadrant & 1) * 2) - 1;
		quady = (quadrant & 2) - 1; 

		// The position and opacity of the lines is a function of fadegress
		if (fadegress < turntime)
		{
			// Very recently had a hit - lines move rapidly outwards

			// find the start and end points
			startx = halfw + quadx * (minrad + RoundFloatToInt( (fadegress / turntime) * (maxrad - minrad) ));
			starty = halfh + quady * (minrad + RoundFloatToInt( (fadegress / turntime) * (maxrad - minrad) ));
			endx = startx + (quadx * maxlen);
			endy = starty + (quady * maxlen);

			alpha = basealpha;

		}
		else {
			// as fadegress increases toward 1.0, the distance we move from maxrad to minrad cubicly increases
			// (i.e. stays at zero for a bit then accelerates toward max
			startx = halfw + quadx * (maxrad - RoundFloatToInt( pow( fadegress, 3 ) * (maxrad - minrad) ) );
			starty = halfh + quady * (maxrad - RoundFloatToInt( pow( fadegress, 3 ) * (maxrad - minrad) ) );
			// lines get shorter at same rate they move inward
			endx = startx + quadx * (maxlen - RoundFloatToInt( pow( fadegress, 3 ) * maxlen ));
			endy = starty + quady * (maxlen - RoundFloatToInt( pow( fadegress, 3 ) * maxlen ));

			alpha = basealpha - RoundFloatToInt( pow( fadegress, 2 ) * basealpha );
		}
		vgui::surface()->DrawSetColor( 255, 20, 0, alpha );
		vgui::surface()->DrawLine( startx, starty, endx, endy );

		// Thicken and fake-AA the line
		vgui::surface()->DrawSetColor( 128, 12, 0, alpha * 0.65f );
		vgui::surface()->DrawLine( startx + quadx, starty, endx, endy - quady );
		vgui::surface()->DrawLine( startx, starty + quady, endx - quadx, endy  );
	}
	/*
	*  SHOTGUN HIT MARKERS
	*/


	if (cl_shotgun_hitmarks.GetInt() == 0)
	{
		return;
	}
	else if (cl_shotgun_hitmarks.GetInt() > 1)
	{
		basealpha = cl_shotgun_hitmarks.GetInt();
	}
		
	if (fadegress < turntime)
	{
		alpha = basealpha;
	}
	else {
		alpha = basealpha - RoundFloatToInt( pow( fadegress, 2 ) * basealpha );
	}


	Vector screen;
	vgui::surface()->DrawSetTextColor( 255, 20, 0, alpha );
	vgui::surface()->DrawSetTextFont( GetTextFont( cl_shotgun_hitmarksize.GetInt() ) );

	// Draw circles at the point the shots hit
	for (int i = 0; i < pWeapon->m_nPelletHits; i++) {

		// If we just hit some new targets, update our local screen positions.
		if (fadegress < 0.02f ) {
			Vector hitpos = pWeapon->m_vecPelletXYZs.Get( i );
			ScreenTransform( hitpos, screen );
			VectorCopy( screen, m_vecPelletMarks[i] );
		}

		// Draw a red circle with a dark outline at the position
		int x = halfw + (m_vecPelletMarks[i].x * halfw * cl_shotgun_hitmarkspread.GetFloat());
		int y = halfh - (m_vecPelletMarks[i].y * halfh * cl_shotgun_hitmarkspread.GetFloat());
	
		if (x >= 0 && x <= vw && y >= 0 && y <= vh)
		{
			vgui::surface()->DrawSetTextPos( x, y + (cl_shotgun_hitmarkoffset.GetFloat() * halfh) ); // shift down below the crosshair
			vgui::surface()->DrawUnicodeChar( BULLET_CHAR );
		}

	}

}


void CHudCrosshair::PaintHorizon( const int crosshair_w, const int crosshair_h )
{
	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	float rollangle = (float)(pPlayer->m_Local.m_vecPunchAngle[ROLL]);
	float absroll = fabs( rollangle );
	if (absroll < 1.0f)
		return; // don't draw if basically level

	const int maxalpha = cl_horizon.GetInt() > 1 ? cl_horizon.GetInt() : 150;

	int alpha = RoundFloatToInt( ( (absroll - 1.0f) / 13.0f) * maxalpha );

	vgui::surface()->DrawSetColor( 200, 150, 0, alpha );

	int vx, vy, vw, vh;
	vgui::surface()->GetFullscreenViewport( vx, vy, vw, vh );
	const int halfw = vw / 2;
	const int halfh = vh / 2;

	// draw dashed lines
	const int numlines = 8; // 8 on each side
	const int gapsize = vw / 50;
	const int innergap = gapsize * 2.5;
	const int dashheight = ( (absroll - 1.0f) / 13.0f ) * (vh / 100);
	int xpos, ypos;
	int ycent = halfh - (dashheight / 2);
	int dashCount = 0;
	while (dashCount < numlines)
	{
		int xoff = innergap + (dashCount*gapsize);
		xpos = halfw + xoff;
		ypos = ycent - ( xoff * tan( DEG2RAD( rollangle ) ) );
		surface()->DrawFilledRect( xpos, ypos, xpos + 1, ypos + dashheight );
		xpos = halfw - xoff;
		ypos = ycent + (xoff * tan( DEG2RAD( rollangle ) ));
		surface()->DrawFilledRect( xpos, ypos, xpos + 1, ypos + dashheight );

		dashCount++;
	}
}


// Target circles for multi-target rockets (airboat and EGAR)

CHudTargetCircles::CHudTargetCircles( const char *pElementName ) :
        CHudElement( pElementName ), BaseClass( NULL, "HudTargetCircles" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );
	SetHiddenBits( HIDEHUD_PLAYERDEAD | HIDEHUD_CROSSHAIR );

}

CHudTargetCircles::~CHudTargetCircles() {}

void CHudTargetCircles::ApplySchemeSettings( IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );

	SetPaintBackgroundEnabled( false );

	SetSize( ScreenWidth(), ScreenHeight() );

	SetForceStereoRenderToFrameBuffer( true );
}

bool CHudTargetCircles::ShouldDraw()
{
	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return false;

	return m_bDraw;

}

void CHudTargetCircles::Paint()
{

	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	// the if condition here would be if any of the rocket targets are
	// not null, but the only way to check is to loop through, so may
	// as well just do it every time.
	{
		vgui::surface()->DrawSetColor( 255, 220, 0, 200 );

		int vx, vy, vw, vh;
		vgui::surface()->GetFullscreenViewport( vx, vy, vw, vh );
		const int halfw = vw / 2;
		const int halfh = vh / 2;

		vgui::surface()->DrawSetTextFont( m_FontHuge );

		int fontW, fontH;
		const wchar_t ch = HOLLOW_BULLET_CHAR;
		vgui::surface()->GetTextSize( m_FontHuge, &ch, fontW, fontH );

		// When the rocket is fired the number of queued rockets is
		// decreased, but we want to keep drawing the circle until the 
		// rocket explodes, so check the whole array and draw any valid
		// targets (rocket sets target to null when it explodes)
		for (int i = 0; i < EGAR_ROCKETS; i++) {

			C_BaseEntity* pentTarget = m_hTargets[i].Get();
			if (pentTarget) {

				Vector vecTgtPos = pentTarget->WorldSpaceCenter();

				Vector screen;

				// Have a target - get world xyz and convert to screen xy
				ScreenTransform( vecTgtPos, screen );
				// screen coordinates are from -1.0 to 1.0
				int x = halfw + (screen.x * halfw);
				int y = halfh - (screen.y * halfh);

				if (x >= 0 && x <= vw && y >= 0 && y <= vh)
				{
					
					vgui::surface()->DrawSetTextPos( x - (fontW / 2), y - (fontH / 2)); // probably need to offset by half char w,h
					vgui::surface()->DrawUnicodeChar( HOLLOW_BULLET_CHAR );
				}

			}
		}
	}
}

