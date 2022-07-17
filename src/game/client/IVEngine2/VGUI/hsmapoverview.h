//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef HSMAPOVERVIEW_H
#define HSMAPOVERVIEW_H
#ifdef _WIN32
#pragma once
#endif

#include "spectatorgui.h"
#include "mapoverview.h"
#include "hl2_shareddefs.h"

using namespace vgui;

extern ConVar mp_playerid; // in cs_gamerules.h
extern ConVar mp_forcecamera; // in gamevars_shared.h
extern ConVar mp_fadetoblack;

#define DESIRED_RADAR_RESOLUTION 450 
//-----------------------------------------------------------------------------
// Purpose: Halo: Source map overview
//-----------------------------------------------------------------------------
class CHSMapOverview : public CMapOverview
{
	DECLARE_CLASS_SIMPLE( CHSMapOverview, CMapOverview );

public:	

	enum
	{
		MAP_ICON_T = 0,
		MAP_ICON_CT,
		MAP_ICON_COUNT
	};

	CHSMapOverview( const char *pElementName );
	virtual ~CHSMapOverview();

	virtual bool ShouldDraw( void );
	Panel *GetAsPanel(){ return this; }
	virtual bool AllowConCommandsWhileAlive(){return false;}
	virtual void SetPlayerPreferredMode( int mode );
	virtual void SetPlayerPreferredViewSize( float viewSize );
	virtual void ApplySchemeSettings( IScheme *scheme );

protected:	// Private structures & types

	// List of game events the HLTV takes care of

	typedef struct
	{
		int		xpos;
		int		ypos;
	} FootStep_t;	

	// Extra stuff in a this-level parallel array
	typedef struct HSMapPlayer_s {
		int		overrideIcon; // if not -1, the icon to use instead
		int		overrideIconOffscreen; // to use with overrideIcon
		float	overrideFadeTime; // Time to start fading the override icon
		float	overrideExpirationTime; // Time to not use the override icon any more
		Vector	overridePosition; // Where the overridden icon will draw
		QAngle	overrideAngle; // And at what angle
		bool	isDead;		// Death latch, since client can be behind the times on health messages.
		float	timeLastSeen; // curtime that we last saw this guy.
		float	timeFirstSeen; // curtime that we started seeing this guy
		float	flashUntilTime;
		float	nextFlashPeakTime;
		int		currentFlashAlpha;
	} HSMapPlayer_t;

	typedef struct HSMapGoal_s
	{
		Vector position;
		int iconToUse;
	} HSMapGoal_t;

public: // IViewPortPanel interface:

	virtual void Update();
	virtual void Init( void );

	// both Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
	virtual bool IsVisible() { return BaseClass::IsVisible(); }
	virtual void SetParent(VPANEL parent) { BaseClass::SetParent(parent); }

	// IGameEventListener

	virtual void FireGameEvent( IGameEvent *event);

	// VGUI overrides

	// Player settings:
	void SetPlayerSeen( int index );

	// general settings:
	virtual void SetMap(const char * map);
	virtual void SetMode( int mode );

	// Object settings
	virtual void	FlashEntity( int entityID );

	// rules that define if you can see a player on the overview or not
	virtual bool CanPlayerBeSeen(MapPlayer_t *player);

	virtual int GetIconNumberFromTeamNumber( int teamNumber );

protected:

	virtual void	DrawCamera();
	virtual void	DrawMapTexture();
	virtual void	DrawMapPlayers();
	void			DrawGoalIcons();
	virtual void	ResetRound();
	virtual void	InitTeamColorsAndIcons();
	virtual void	UpdateSizeAndPosition();
	void			UpdateGoalIcons();
	void			ClearGoalIcons();
	virtual bool	IsRadarLocked();
	Vector2D		PanelToMap( const Vector2D &panelPos );

	bool			AdjustPointToPanel(Vector2D *pos);
	MapPlayer_t*	GetPlayerByEntityID( int entityID );
	virtual void	UpdatePlayers();
	void			UpdateFlashes();
	bool			CreateRadarImage(const char *mapName, const char *radarFileName);
	virtual bool	RunHudAnimations(){ return false; }

private:
	bool DrawIconHS
	(
		int textureID,
		int offscreenTextureID,
		Vector pos,
		float scale,
		float angle,
		int alpha,
		bool allowRotation = true,
		const char *text = NULL,
		Color *textColor = NULL,
		float status = -1,
		Color *statusColor = NULL 
	);

	int GetMasterAlpha( void ); // The main alpha that the map part should be, determined by using the mode to look at the right convar
	int GetBorderSize( void ); // How far in from the edge of the panel we draw, based on mode.  Let's the background fancy corners show.
	HSMapPlayer_t* GetHSInfoForPlayerIndex( int index );
	HSMapPlayer_t* GetHSInfoForPlayer(MapPlayer_t *player);

	HSMapPlayer_t	m_PlayersHSInfo[MAX_PLAYERS];

	CUtlVector< HSMapGoal_t > m_goalIcons;
	bool	m_goalIconsLoaded;

	int		m_TeamIconsSelf[MAP_ICON_COUNT];
	int		m_TeamIconsDead[MAP_ICON_COUNT];
	int		m_TeamIconsOffscreen[MAP_ICON_COUNT];
	int		m_TeamIconsDeadOffscreen[MAP_ICON_COUNT];

	int		m_radioFlash;
	int		m_radioFlashOffscreen;
	int		m_radarTint;
	int		m_playerFacing;
	int		m_cameraIconFirst;
	int		m_cameraIconThird;
	int		m_cameraIconFree;

	int		m_nRadarMapTextureID;	// Texture ID for radar version of current overview image

	int		m_playerPreferredMode; // The mode the player wants to be in for when we aren't being the radar
};

#endif // HSMAPOVERVIEW_H
