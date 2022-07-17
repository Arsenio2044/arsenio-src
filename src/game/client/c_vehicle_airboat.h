#include "cbase.h"
#include "c_prop_vehicle.h"
#include "datacache/imdlcache.h"
#include "flashlighteffect.h"
#include "movevars_shared.h"
#include "ammodef.h"
#include "SpriteTrail.h"
#include "beamdraw.h"
#include "enginesprite.h"
#include "fx_quad.h"
#include "fx.h"
#include "fx_water.h"
#include "engine/ivdebugoverlay.h"
#include "view.h"
#include "clienteffectprecachesystem.h"
#include "c_basehlplayer.h"
#include "vgui_controls/Controls.h"
#include "vgui/ISurface.h"

#define	MAX_WAKE_POINTS	16
#define	WAKE_POINT_MASK (MAX_WAKE_POINTS-1)

#define	WAKE_LIFETIME	0.5f

// Keep same as in vehicle_airboat.cpp !
#define AIRBOAT_ROCKETS 8


//=============================================================================
//
// Client-side Airboat Class
//
class C_PropAirboat : public C_PropVehicleDriveable
{
	DECLARE_CLASS( C_PropAirboat, C_PropVehicleDriveable );

public:

	DECLARE_CLIENTCLASS();
	DECLARE_INTERPOLATION();
	DECLARE_DATADESC();

	C_PropAirboat();
	~C_PropAirboat();

public:

	// C_BaseEntity
	virtual void Simulate();

	// IClientVehicle
	virtual void UpdateViewAngles( C_BasePlayer *pLocalPlayer, CUserCmd *pCmd );
	virtual void OnEnteredVehicle( C_BasePlayer *pPlayer );
	virtual int GetPrimaryAmmoType() const;
	virtual int GetPrimaryAmmoClip() const;
	virtual bool PrimaryAmmoUsesClips() const;
	virtual int GetPrimaryAmmoCount() const;
	virtual int GetCurrentSpeed() const;
	virtual int GetJoystickResponseCurve() const;

	int		DrawModel( int flags );

	// Draws crosshair in the forward direction of the boat
	void DrawHudElements();
	void DrawHudRocketDisplay();
	void DrawHudTargetMarkers();

	// For the hud
	int GetRocketsReady();
	int GetRocketsQueued();
	EHANDLE* GetRocketTargets();


private:

	void DrawPropWake( Vector origin, float speed );
	void DrawPontoonSplash( Vector position, Vector direction, float speed );
	void DrawPontoonWake( Vector startPos, Vector wakeDir, float wakeLength, float speed );

	void DampenEyePosition( Vector &vecVehicleEyePos, QAngle &vecVehicleEyeAngles );
	void DampenForwardMotion( Vector &vecVehicleEyePos, QAngle &vecVehicleEyeAngles, float flFrameTime );
	void DampenUpMotion( Vector &vecVehicleEyePos, QAngle &vecVehicleEyeAngles, float flFrameTime );
	void ComputePDControllerCoefficients( float *pCoefficientsOut, float flFrequency, float flDampening, float flDeltaTime );

	void UpdateHeadlight( void );
	void UpdateWake( void );
	int	 DrawWake( void );
	void DrawSegment( const BeamSeg_t &beamSeg, const Vector &vNormal );

	TrailPoint_t *GetTrailPoint( int n )
	{
		int nIndex = (n + m_nFirstStep) & WAKE_POINT_MASK;
		return &m_vecSteps[nIndex];
	}

private:

	Vector		m_vecLastEyePos;
	Vector		m_vecLastEyeTarget;
	Vector		m_vecEyeSpeed;
	Vector		m_vecTargetSpeed;

	float		m_flViewAngleDeltaTime;

	bool		m_bHeadlightIsOn;
	int			m_nAmmoCount;
	CHeadlightEffect *m_pHeadlight;

	// ROCKET STUFF
	int m_nRocketsReady;
	int m_nRocketsQueued;
	EHANDLE m_hRocketTargets[AIRBOAT_ROCKETS];
	//

	int				m_nExactWaterLevel;

	TrailPoint_t	m_vecSteps[MAX_WAKE_POINTS];
	int				m_nFirstStep;
	int				m_nStepCount;
	float			m_flUpdateTime;

	TimedEvent		m_SplashTime;
	CMeshBuilder	m_Mesh;

	Vector			m_vecPhysVelocity;
	float           m_flVelocity;

	vgui::HFont     m_hTrebHuge;
};