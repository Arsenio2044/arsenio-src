//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "baseviewmodel_shared.h"
#include "datacache/imdlcache.h"

#if defined( CLIENT_DLL )
#include "iprediction.h"
#include "prediction.h"
#include "client_virtualreality.h"
#include "sourcevr/isourcevirtualreality.h"
#include "view.h"

#else
#include "vguiscreen.h"
#endif

#if defined( CLIENT_DLL ) && defined( SIXENSE )
#include "sixense/in_sixense.h"
#include "sixense/sixense_convars_extern.h"
#endif

#ifdef SIXENSE
extern ConVar in_forceuser;
#include "iclientmode.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define VIEWMODEL_ANIMATION_PARITY_BITS 3
#define SCREEN_OVERLAY_MATERIAL "vgui/screens/vgui_overlay"



//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CBaseViewModel::CBaseViewModel()
{
#if defined( CLIENT_DLL )
	// NOTE: We do this here because the color is never transmitted for the view model.
	m_nOldAnimationParity = 0;
	m_EntClientFlags |= ENTCLIENTFLAG_ALWAYS_INTERPOLATE;
#endif
	SetRenderColor(255, 255, 255, 255);

	// View model of this weapon
	m_sVMName = NULL_STRING;
	// Prefix of the animations that should be used by the player carrying this weapon
	m_sAnimationPrefix = NULL_STRING;

	m_nViewModelIndex = 0;

	m_nAnimationParity = 0;
#if CLIENT_DLL
	m_angEyeAngles = QAngle(0.0f, 0.0f, 0.0f);
	m_angViewPunch = QAngle(0.0f, 0.0f, 0.0f);
	m_angOldFacing = QAngle(0.0f, 0.0f, 0.0f);
	m_angDelta = QAngle(0.0f, 0.0f, 0.0f);
	m_angMotion = QAngle(0.0f, 0.0f, 0.0f);
	m_angCounterMotion = QAngle(0.0f, 0.0f, 0.0f);
	m_angCompensation = QAngle(0.0f, 0.0f, 0.0f);

	m_flSideTiltResult = 1.0f;
	m_flSideTiltDifference = 1.0f;
	m_flForwardOffsetResult = 1.0f;
	m_flForwardOffsetDifference = 1.0f;



#endif // CLIENT_DLL
}
ConVar arsenio_sway("arsenio_sway", "1.0");
ConVar arsenio_sway_rate("arsenio_sway_rate", "1.0");
ConVar arsenio_sway_wiggle_rate("arsenio_sway_wiggle_rate", "1.0");
ConVar arsenio_sway_tilt("arsenio_sway_tilt", "280.0");
ConVar arsenio_sway_offset("arsenio_sway_offset", "5.0");
ConVar arsenio_sway_jump_velocity_division("arsenio_sway_jump_velocity_division", "24.0");
ConVar arsenio_vm_crouch_rotatespeed("arsenio_vm_crouch_rotatespeed", "0.4");
ConVar arsenio_vm_crouch_angle("arsenio_vm_crouch_angle", "-6");
ConVar arsenio_vm_crouch_offset("arsenio_vm_crouch_offset", "-1");




//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CBaseViewModel::~CBaseViewModel()
{
}

void CBaseViewModel::UpdateOnRemove( void )
{
	BaseClass::UpdateOnRemove();

	DestroyControlPanels();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseViewModel::Precache( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseViewModel::Spawn( void )
{
	Precache( );
	SetSize( Vector( -8, -4, -2), Vector(8, 4, 2) );
	SetSolid( SOLID_NONE );
}


#if defined ( CSTRIKE_DLL ) && !defined ( CLIENT_DLL )
#define VGUI_CONTROL_PANELS
#endif

#if defined ( TF_DLL )
#define VGUI_CONTROL_PANELS
#endif

#ifdef INVASION_DLL
#define VGUI_CONTROL_PANELS
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseViewModel::SetControlPanelsActive( bool bState )
{
#if defined( VGUI_CONTROL_PANELS )
	// Activate control panel screens
	for ( int i = m_hScreens.Count(); --i >= 0; )
	{
		if (m_hScreens[i].Get())
		{
			m_hScreens[i]->SetActive( bState );
		}
	}
#endif
}

//-----------------------------------------------------------------------------
// This is called by the base object when it's time to spawn the control panels
//-----------------------------------------------------------------------------
void CBaseViewModel::SpawnControlPanels()
{
#if defined( VGUI_CONTROL_PANELS )
	char buf[64];

	// Destroy existing panels
	DestroyControlPanels();

	CBaseCombatWeapon *weapon = m_hWeapon.Get();

	if ( weapon == NULL )
	{
		return;
	}

	MDLCACHE_CRITICAL_SECTION();

	// FIXME: Deal with dynamically resizing control panels?

	// If we're attached to an entity, spawn control panels on it instead of use
	CBaseAnimating *pEntityToSpawnOn = this;
	char *pOrgLL = "controlpanel%d_ll";
	char *pOrgUR = "controlpanel%d_ur";
	char *pAttachmentNameLL = pOrgLL;
	char *pAttachmentNameUR = pOrgUR;
	/*
	if ( IsBuiltOnAttachment() )
	{
		pEntityToSpawnOn = dynamic_cast<CBaseAnimating*>((CBaseEntity*)m_hBuiltOnEntity.Get());
		if ( pEntityToSpawnOn )
		{
			char sBuildPointLL[64];
			char sBuildPointUR[64];
			Q_snprintf( sBuildPointLL, sizeof( sBuildPointLL ), "bp%d_controlpanel%%d_ll", m_iBuiltOnPoint );
			Q_snprintf( sBuildPointUR, sizeof( sBuildPointUR ), "bp%d_controlpanel%%d_ur", m_iBuiltOnPoint );
			pAttachmentNameLL = sBuildPointLL;
			pAttachmentNameUR = sBuildPointUR;
		}
		else
		{
			pEntityToSpawnOn = this;
		}
	}
	*/

	Assert( pEntityToSpawnOn );

	// Lookup the attachment point...
	int nPanel;
	for ( nPanel = 0; true; ++nPanel )
	{
		Q_snprintf( buf, sizeof( buf ), pAttachmentNameLL, nPanel );
		int nLLAttachmentIndex = pEntityToSpawnOn->LookupAttachment(buf);
		if (nLLAttachmentIndex <= 0)
		{
			// Try and use my panels then
			pEntityToSpawnOn = this;
			Q_snprintf( buf, sizeof( buf ), pOrgLL, nPanel );
			nLLAttachmentIndex = pEntityToSpawnOn->LookupAttachment(buf);
			if (nLLAttachmentIndex <= 0)
				return;
		}

		Q_snprintf( buf, sizeof( buf ), pAttachmentNameUR, nPanel );
		int nURAttachmentIndex = pEntityToSpawnOn->LookupAttachment(buf);
		if (nURAttachmentIndex <= 0)
		{
			// Try and use my panels then
			Q_snprintf( buf, sizeof( buf ), pOrgUR, nPanel );
			nURAttachmentIndex = pEntityToSpawnOn->LookupAttachment(buf);
			if (nURAttachmentIndex <= 0)
				return;
		}

		const char *pScreenName;
		weapon->GetControlPanelInfo( nPanel, pScreenName );
		if (!pScreenName)
			continue;

		const char *pScreenClassname;
		weapon->GetControlPanelClassName( nPanel, pScreenClassname );
		if ( !pScreenClassname )
			continue;

		// Compute the screen size from the attachment points...
		matrix3x4_t	panelToWorld;
		pEntityToSpawnOn->GetAttachment( nLLAttachmentIndex, panelToWorld );

		matrix3x4_t	worldToPanel;
		MatrixInvert( panelToWorld, worldToPanel );

		// Now get the lower right position + transform into panel space
		Vector lr, lrlocal;
		pEntityToSpawnOn->GetAttachment( nURAttachmentIndex, panelToWorld );
		MatrixGetColumn( panelToWorld, 3, lr );
		VectorTransform( lr, worldToPanel, lrlocal );

		float flWidth = lrlocal.x;
		float flHeight = lrlocal.y;

		CVGuiScreen *pScreen = CreateVGuiScreen( pScreenClassname, pScreenName, pEntityToSpawnOn, this, nLLAttachmentIndex );
		pScreen->ChangeTeam( GetTeamNumber() );
		pScreen->SetActualSize( flWidth, flHeight );
		pScreen->SetActive( false );
		pScreen->MakeVisibleOnlyToTeammates( false );
	
#ifdef INVASION_DLL
		pScreen->SetOverlayMaterial( SCREEN_OVERLAY_MATERIAL );
#endif
		pScreen->SetAttachedToViewModel( true );
		int nScreen = m_hScreens.AddToTail( );
		m_hScreens[nScreen].Set( pScreen );
	}
#endif
}

void CBaseViewModel::DestroyControlPanels()
{
#if defined( VGUI_CONTROL_PANELS )
	// Kill the control panels
	int i;
	for ( i = m_hScreens.Count(); --i >= 0; )
	{
		DestroyVGuiScreen( m_hScreens[i].Get() );
	}
	m_hScreens.RemoveAll();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEntity - 
//-----------------------------------------------------------------------------
void CBaseViewModel::SetOwner( CBaseEntity *pEntity )
{
	m_hOwner = pEntity;
#if !defined( CLIENT_DLL )
	// Make sure we're linked into hierarchy
	//SetParent( pEntity );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : nIndex - 
//-----------------------------------------------------------------------------
void CBaseViewModel::SetIndex( int nIndex )
{
	m_nViewModelIndex = nIndex;
	Assert( m_nViewModelIndex < (1 << VIEWMODEL_INDEX_BITS) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBaseViewModel::ViewModelIndex( ) const
{
	return m_nViewModelIndex;
}

//-----------------------------------------------------------------------------
// Purpose: Pass our visibility on to our child screens
//-----------------------------------------------------------------------------
void CBaseViewModel::AddEffects( int nEffects )
{
	if ( nEffects & EF_NODRAW )
	{
		SetControlPanelsActive( false );
	}

	BaseClass::AddEffects( nEffects );
}

//-----------------------------------------------------------------------------
// Purpose: Pass our visibility on to our child screens
//-----------------------------------------------------------------------------
void CBaseViewModel::RemoveEffects( int nEffects )
{
	if ( nEffects & EF_NODRAW )
	{
		SetControlPanelsActive( true );
	}

	BaseClass::RemoveEffects( nEffects );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *modelname - 
//-----------------------------------------------------------------------------
void CBaseViewModel::SetWeaponModel( const char *modelname, CBaseCombatWeapon *weapon )
{
	m_hWeapon = weapon;

#if defined( CLIENT_DLL )
	SetModel( modelname );
#else
	string_t str;
	if ( modelname != NULL )
	{
		str = MAKE_STRING( modelname );
	}
	else
	{
		str = NULL_STRING;
	}

	if ( str != m_sVMName )
	{
		// Msg( "SetWeaponModel %s at %f\n", modelname, gpGlobals->curtime );
		m_sVMName = str;
		SetModel( STRING( m_sVMName ) );

		// Create any vgui control panels associated with the weapon
		SpawnControlPanels();

		bool showControlPanels = weapon && weapon->ShouldShowControlPanels();
		SetControlPanelsActive( showControlPanels );
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : CBaseCombatWeapon
//-----------------------------------------------------------------------------
CBaseCombatWeapon *CBaseViewModel::GetOwningWeapon( void )
{
	return m_hWeapon.Get();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : sequence - 
//-----------------------------------------------------------------------------
void CBaseViewModel::SendViewModelMatchingSequence( int sequence )
{
	// since all we do is send a sequence number down to the client, 
	// set this here so other weapons code knows which sequence is playing.
	SetSequence( sequence );

	m_nAnimationParity = ( m_nAnimationParity + 1 ) & ( (1<<VIEWMODEL_ANIMATION_PARITY_BITS) - 1 );

#if defined( CLIENT_DLL )
	m_nOldAnimationParity = m_nAnimationParity;

	// Force frame interpolation to start at exactly frame zero
	m_flAnimTime			= gpGlobals->curtime;
#else
	CBaseCombatWeapon *weapon = m_hWeapon.Get();
	bool showControlPanels = weapon && weapon->ShouldShowControlPanels();
	SetControlPanelsActive( showControlPanels );
#endif

	// Restart animation at frame 0
	SetCycle( 0 );
	ResetSequenceInfo();
}

#if defined( CLIENT_DLL )
#include "ivieweffects.h"
#endif



#if defined( CLIENT_DLL )

void CBaseViewModel::AddViewModelBob(CBasePlayer *owner, Vector& eyePosition, QAngle& eyeAngles)
{
	if (!owner)
		return;



	float dotForward = RemapVal(DotProduct(owner->GetLocalVelocity(), MainViewForward()), -arsenio_sway_offset.GetFloat(), arsenio_sway_offset.GetFloat(), -10.0f, 10.0f);
	float movement = abs(dotForward) > 0.5f ? arsenio_sway_offset.GetFloat() : 0;
	m_flForwardOffsetResult = Approach(movement, m_flForwardOffsetResult, gpGlobals->frametime * 10.0f * m_flForwardOffsetDifference);
	m_flForwardOffsetDifference = fabs(movement - m_flForwardOffsetResult);

	float dotRight = RemapVal(DotProduct(owner->GetLocalVelocity(), MainViewRight()), -arsenio_sway_tilt.GetFloat(), arsenio_sway_tilt.GetFloat(), -1.0f, 1.0f) * 15 * 0.5f;
	m_flSideTiltResult = Approach(dotRight, m_flSideTiltResult, gpGlobals->frametime * 10.0f * m_flSideTiltDifference);
	m_flSideTiltDifference = fabs(dotRight - m_flSideTiltResult);

	float rollZOffset = -clamp(m_flSideTiltResult, -10, 0) * 0.1f;
	eyePosition -= MainViewUp() * rollZOffset;
	eyeAngles[ROLL] += m_flSideTiltResult;

	eyePosition -= MainViewForward() * abs(m_flForwardOffsetResult) * 0.1f;
	eyePosition -= MainViewUp() * abs(m_flForwardOffsetResult) * 0.075f;


}


#define LAG_POSITION_COMPENSATION	0.5f
#define LAG_FLIP_FACTOR				1.0f


#ifdef ARSENIO_OLD
float g_fMaxViewModelLag = 0.5f;
#endif


void CBaseViewModel::CalcViewModelLag(Vector& origin, QAngle& angles, QAngle& original_angles)
{
	CBasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	Vector dirForward, dirRight, dirUp;
	AngleVectors(pPlayer->EyeAngles(), &dirForward, &dirRight, &dirUp);

	if (gpGlobals->frametime != 0.0f)
	{
		float flFrametime = clamp(gpGlobals->frametime, 0.001, 1.0f / 20.0f);
		float flWiggleFactor = (1.0f - arsenio_sway_wiggle_rate.GetFloat()) / 0.6f + 0.15f;
		float flSwayRate = powf(arsenio_sway_rate.GetFloat(), 1.5f) * 10.0f;
		float clampFac = 1.1f - MIN((fabs(m_angMotion[PITCH]) + fabs(m_angMotion[YAW]) + fabs(m_angMotion[ROLL])) / 20.0f, 1.0f);

		m_angViewPunch = pPlayer->m_Local.m_vecPunchAngle;
		m_angEyeAngles = pPlayer->EyeAngles() - m_angViewPunch;

		m_angDelta[PITCH] = UTIL_AngleDiff(m_angEyeAngles[PITCH], m_angOldFacing[PITCH]) / flFrametime / 120.0f * clampFac;
		m_angDelta[YAW] = UTIL_AngleDiff(m_angEyeAngles[YAW], m_angOldFacing[YAW]) / flFrametime / 120.0f * clampFac;
		m_angDelta[ROLL] = UTIL_AngleDiff(m_angEyeAngles[ROLL], m_angOldFacing[ROLL]) / flFrametime / 120.0f * clampFac;

		Vector deltaForward;
		AngleVectors(m_angDelta, &deltaForward, NULL, NULL);
		VectorNormalize(deltaForward);

		m_angOldFacing = m_angEyeAngles;

		m_angOldFacing[PITCH] -= (pPlayer->GetLocalVelocity().z / MAX(1, arsenio_sway_jump_velocity_division.GetFloat()));

		m_angCounterMotion = Lerp(flFrametime * (flSwayRate * (0.75f + (0.5f - flWiggleFactor))), m_angCounterMotion, -m_angMotion);
		m_angCompensation[PITCH] = AngleDiff(m_angMotion[PITCH], -m_angCounterMotion[PITCH]);
		m_angCompensation[YAW] = AngleDiff(m_angMotion[YAW], -m_angCounterMotion[YAW]);

		m_angMotion = Lerp(flFrametime * flSwayRate, m_angMotion, m_angDelta + m_angCompensation);
	}

	float flFraction = arsenio_sway.GetFloat();
	origin += (m_angMotion[YAW] * LAG_POSITION_COMPENSATION * 0.66f * dirRight * LAG_FLIP_FACTOR) * flFraction;
	origin += (m_angMotion[PITCH] * LAG_POSITION_COMPENSATION * dirUp) * flFraction;

	angles[PITCH] += (m_angMotion[PITCH]) * flFraction;
	angles[YAW] += (m_angMotion[YAW] * 0.66f * LAG_FLIP_FACTOR) * flFraction;
	angles[ROLL] += (m_angCounterMotion[ROLL] * 0.5f * LAG_FLIP_FACTOR) * flFraction;
#ifdef ARSENIO_OLD
		Vector vOriginalOrigin = origin;
	QAngle vOriginalAngles = angles;

	// Calculate our drift
	Vector	forward;
	AngleVectors(angles, &forward, NULL, NULL);

	if (gpGlobals->frametime != 0.0f)
	{
		Vector vDifference;
		VectorSubtract(forward, m_vecLastFacing, vDifference);

		float flSpeed = 10.0f;

		// If we start to lag too far behind, we'll increase the "catch up" speed.  Solves the problem with fast cl_yawspeed, m_yaw or joysticks
		//  rotating quickly.  The old code would slam lastfacing with origin causing the viewmodel to pop to a new position
		float flDiff = vDifference.Length();
		if ((flDiff > g_fMaxViewModelLag) && (g_fMaxViewModelLag > 0.0f))
		{
			float flScale = flDiff / g_fMaxViewModelLag;
			flSpeed *= flScale;
		}

		// FIXME:  Needs to be predictable?
		VectorMA(m_vecLastFacing, flSpeed * gpGlobals->frametime, vDifference, m_vecLastFacing);
		// Make sure it doesn't grow out of control!!!
		VectorNormalize(m_vecLastFacing);
		VectorMA(origin, 5.0f, vDifference * -1.0f, origin);

		Assert(m_vecLastFacing.IsValid());
	}

	Vector right, up;
	AngleVectors(original_angles, &forward, &right, &up);

	float pitch = original_angles[PITCH];
	if (pitch > 180.0f)
		pitch -= 360.0f;
	else if (pitch < -180.0f)
		pitch += 360.0f;

	if (g_fMaxViewModelLag == 0.0f)
	{
		origin = vOriginalOrigin;
		angles = vOriginalAngles;
	}

	//FIXME: These are the old settings that caused too many exposed polys on some models
	VectorMA(origin, -pitch * 0.035f, forward, origin);
	VectorMA(origin, -pitch * 0.03f, right, origin);
	VectorMA(origin, -pitch * 0.02f, up, origin);
#endif
}




#endif



#if defined ( CLIENT_DLL )


//deals with additional offsets from crouching and jumping and the like
void CBaseViewModel::CalcViewModelBasePose(Vector& origin, QAngle& angles, CBasePlayer* owner)
{
	Vector forward, right, up;
	AngleVectors(owner->EyeAngles(), &forward, &right, &up);
	//crouching: we are ducked or ducking, but we arent ducked AND ducking (which happens when standing up), and we are on the ground not crouch jumping
	if ((owner->GetFlags() & FL_DUCKING || owner->m_Local.m_bDucking) && !(owner->GetFlags() & FL_DUCKING && owner->m_Local.m_bDucking) && owner->GetFlags() & FL_ONGROUND) {
		m_flDucking += gpGlobals->frametime / arsenio_vm_crouch_rotatespeed.GetFloat();
	}
	else {
		m_flDucking -= gpGlobals->frametime / arsenio_vm_crouch_rotatespeed.GetFloat();
	}
	m_flDucking = Clamp(m_flDucking, 0.0f, 1.0f);
	float flDuckingEased = (m_flDucking < 0.5 ? 4 * m_flDucking * m_flDucking * m_flDucking : 1 - powf(-2 * m_flDucking + 2, 3) / 2); //easeInOutCubic
	angles += QAngle(0.0f, 0.0f, arsenio_vm_crouch_angle.GetFloat()) * flDuckingEased;
	origin += right * arsenio_vm_crouch_offset.GetFloat() * flDuckingEased;
}


void CBaseViewModel::CalcViewModelCollision(Vector& origin, QAngle& angles, CBasePlayer* owner)
{
	CBaseCombatWeapon* pWeapon = GetWeapon();
	if (pWeapon->GetWpnData().iWeaponLength == 0)
		return;

	Vector forward, right, up;
	AngleVectors(owner->EyeAngles(), &forward, &right, &up);
	trace_t tr;
	UTIL_TraceLine(owner->EyePosition(), owner->EyePosition() + forward * pWeapon->GetWpnData().iWeaponLength, MASK_SHOT, owner, COLLISION_GROUP_NONE, &tr);
	m_flCurrentDistance = Approach(tr.fraction, m_flCurrentDistance, gpGlobals->frametime * 10.0f * m_flDistanceDifference);
	m_flDistanceDifference = fabs(tr.fraction - m_flCurrentDistance);

	origin += forward * pWeapon->GetWpnData().vCollisionOffset.x * (1.0f - m_flCurrentDistance);
	origin += right * pWeapon->GetWpnData().vCollisionOffset.y * (1.0f - m_flCurrentDistance);
	origin += up * pWeapon->GetWpnData().vCollisionOffset.z * (1.0f - m_flCurrentDistance);

	angles += pWeapon->GetWpnData().angCollisionRotation * (1.0f - m_flCurrentDistance);



}

#endif

void CBaseViewModel::CalcViewModelView(CBasePlayer *owner, const Vector& eyePosition, const QAngle& eyeAngles)
{
	// UNDONE: Calc this on the server?  Disabled for now as it seems unnecessary to have this info on the server
#if defined( CLIENT_DLL )
	QAngle vmangoriginal = eyeAngles;
	QAngle vmangles = eyeAngles;
	Vector vmorigin = eyePosition;

	CBaseCombatWeapon *pWeapon = m_hWeapon.Get();
	//Allow weapon lagging
	if (pWeapon != NULL)
	{

		CalcViewModelCollision(vmorigin, vmangles, owner);
		//CalcViewmodelBob();
		CalcViewModelBasePose(vmorigin, vmangles, owner);



#if defined( CLIENT_DLL )
		if (!prediction->InPrediction())
#endif
		{
			// add weapon-specific bob 
			pWeapon->AddViewmodelBob(this, vmorigin, vmangles);

		}
	}
	// Add model-specific bob even if no weapon associated (for head bob for off hand models)
	AddViewModelBob(owner, vmorigin, vmangles);
#if !defined ( CSTRIKE_DLL )
	// This was causing weapon jitter when rotating in updated CS:S; original Source had this in above InPrediction block  07/14/10
	// Add lag
	CalcViewModelLag(vmorigin, vmangles, vmangoriginal);
#endif

#if defined( CLIENT_DLL )
	if (!prediction->InPrediction())
	{
		// Let the viewmodel shake at about 10% of the amplitude of the player's view
		vieweffects->ApplyShake(vmorigin, vmangles, 0.1);
	}
#endif

	if (UseVR())
	{
		g_ClientVirtualReality.OverrideViewModelTransform(vmorigin, vmangles, pWeapon && pWeapon->ShouldUseLargeViewModelVROverride());
	}

	SetLocalOrigin(vmorigin);
	SetLocalAngles(vmangles);

#ifdef SIXENSE
	if (g_pSixenseInput->IsEnabled() && (owner->GetObserverMode() == OBS_MODE_NONE) && !UseVR())
	{
		const float max_gun_pitch = 20.0f;

		float viewmodel_fov_ratio = g_pClientMode->GetViewModelFOV() / owner->GetFOV();
		QAngle gun_angles = g_pSixenseInput->GetViewAngleOffset() * -viewmodel_fov_ratio;

		// Clamp pitch a bit to minimize seeing back of viewmodel
		if (gun_angles[PITCH] < -max_gun_pitch)
		{
			gun_angles[PITCH] = -max_gun_pitch;
		}

#ifdef WIN32 // ShouldFlipViewModel comes up unresolved on osx? Mabye because it's defined inline? fixme
		if (ShouldFlipViewModel())
		{
			gun_angles[YAW] *= -1.0f;
		}
#endif

		vmangles = EyeAngles() + gun_angles;

		SetLocalAngles(vmangles);
	}
#endif
#endif

}



//-----------------------------------------------------------------------------
// Stub to keep networking consistent for DEM files
//-----------------------------------------------------------------------------
#if defined( CLIENT_DLL )
  extern void RecvProxy_EffectFlags( const CRecvProxyData *pData, void *pStruct, void *pOut );
  void RecvProxy_SequenceNum( const CRecvProxyData *pData, void *pStruct, void *pOut );
#endif

//-----------------------------------------------------------------------------
// Purpose: Resets anim cycle when the server changes the weapon on us
//-----------------------------------------------------------------------------
#if defined( CLIENT_DLL )
static void RecvProxy_Weapon( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	CBaseViewModel *pViewModel = ((CBaseViewModel*)pStruct);
	CBaseCombatWeapon *pOldWeapon = pViewModel->GetOwningWeapon();

	// Chain through to the default recieve proxy ...
	RecvProxy_IntToEHandle( pData, pStruct, pOut );

	// ... and reset our cycle index if the server is switching weapons on us
	CBaseCombatWeapon *pNewWeapon = pViewModel->GetOwningWeapon();
	if ( pNewWeapon != pOldWeapon )
	{
		// Restart animation at frame 0
		pViewModel->SetCycle( 0 );
		pViewModel->m_flAnimTime = gpGlobals->curtime;
	}
}
#endif


LINK_ENTITY_TO_CLASS( viewmodel, CBaseViewModel );

IMPLEMENT_NETWORKCLASS_ALIASED( BaseViewModel, DT_BaseViewModel )

BEGIN_NETWORK_TABLE_NOBASE(CBaseViewModel, DT_BaseViewModel)
#if !defined( CLIENT_DLL )
	SendPropModelIndex(SENDINFO(m_nModelIndex)),
	SendPropInt		(SENDINFO(m_nBody), 8),
	SendPropInt		(SENDINFO(m_nSkin), 10),
	SendPropInt		(SENDINFO(m_nSequence),	8, SPROP_UNSIGNED),
	SendPropInt		(SENDINFO(m_nViewModelIndex), VIEWMODEL_INDEX_BITS, SPROP_UNSIGNED),
	SendPropFloat	(SENDINFO(m_flPlaybackRate),	8,	SPROP_ROUNDUP,	-4.0,	12.0f),
	SendPropInt		(SENDINFO(m_fEffects),		10, SPROP_UNSIGNED),
	SendPropInt		(SENDINFO(m_nAnimationParity), 3, SPROP_UNSIGNED ),
	SendPropEHandle (SENDINFO(m_hWeapon)),
	SendPropEHandle (SENDINFO(m_hOwner)),

	SendPropInt( SENDINFO( m_nNewSequenceParity ), EF_PARITY_BITS, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_nResetEventsParity ), EF_PARITY_BITS, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_nMuzzleFlashParity ), EF_MUZZLEFLASH_BITS, SPROP_UNSIGNED ),

#if !defined( INVASION_DLL ) && !defined( INVASION_CLIENT_DLL )
	SendPropArray	(SendPropFloat(SENDINFO_ARRAY(m_flPoseParameter),	8, 0, 0.0f, 1.0f), m_flPoseParameter),
#endif
#else
	RecvPropInt		(RECVINFO(m_nModelIndex)),
	RecvPropInt		(RECVINFO(m_nSkin)),
	RecvPropInt		(RECVINFO(m_nBody)),
	RecvPropInt		(RECVINFO(m_nSequence), 0, RecvProxy_SequenceNum ),
	RecvPropInt		(RECVINFO(m_nViewModelIndex)),
	RecvPropFloat	(RECVINFO(m_flPlaybackRate)),
	RecvPropInt		(RECVINFO(m_fEffects), 0, RecvProxy_EffectFlags ),
	RecvPropInt		(RECVINFO(m_nAnimationParity)),
	RecvPropEHandle (RECVINFO(m_hWeapon), RecvProxy_Weapon ),
	RecvPropEHandle (RECVINFO(m_hOwner)),

	RecvPropInt( RECVINFO( m_nNewSequenceParity )),
	RecvPropInt( RECVINFO( m_nResetEventsParity )),
	RecvPropInt( RECVINFO( m_nMuzzleFlashParity )),

#if !defined( INVASION_DLL ) && !defined( INVASION_CLIENT_DLL )
	RecvPropArray(RecvPropFloat(RECVINFO(m_flPoseParameter[0]) ), m_flPoseParameter ),
#endif
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL

BEGIN_PREDICTION_DATA( CBaseViewModel )

	// Networked
	DEFINE_PRED_FIELD( m_nModelIndex, FIELD_SHORT, FTYPEDESC_INSENDTABLE | FTYPEDESC_MODELINDEX ),
	DEFINE_PRED_FIELD( m_nSkin, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_nBody, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_nSequence, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_nViewModelIndex, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD_TOL( m_flPlaybackRate, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, 0.125f ),
	DEFINE_PRED_FIELD( m_fEffects, FIELD_INTEGER, FTYPEDESC_INSENDTABLE | FTYPEDESC_OVERRIDE ),
	DEFINE_PRED_FIELD( m_nAnimationParity, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_hWeapon, FIELD_EHANDLE, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flAnimTime, FIELD_FLOAT, 0 ),

	DEFINE_FIELD( m_hOwner, FIELD_EHANDLE ),
	DEFINE_FIELD( m_flTimeWeaponIdle, FIELD_FLOAT ),
	DEFINE_FIELD( m_Activity, FIELD_INTEGER ),
	DEFINE_PRED_FIELD( m_flCycle, FIELD_FLOAT, FTYPEDESC_PRIVATE | FTYPEDESC_OVERRIDE | FTYPEDESC_NOERRORCHECK ),

END_PREDICTION_DATA()

void RecvProxy_SequenceNum( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	CBaseViewModel *model = (CBaseViewModel *)pStruct;
	if (pData->m_Value.m_Int != model->GetSequence())
	{
		MDLCACHE_CRITICAL_SECTION();

		model->SetSequence(pData->m_Value.m_Int);
		model->m_flAnimTime = gpGlobals->curtime;
		model->SetCycle(0);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int	CBaseViewModel::LookupAttachment( const char *pAttachmentName )
{
	if ( m_hWeapon.Get() && m_hWeapon.Get()->WantsToOverrideViewmodelAttachments() )
		return m_hWeapon.Get()->LookupAttachment( pAttachmentName );

	return BaseClass::LookupAttachment( pAttachmentName );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseViewModel::GetAttachment( int number, matrix3x4_t &matrix )
{
	if ( m_hWeapon.Get() && m_hWeapon.Get()->WantsToOverrideViewmodelAttachments() )
		return m_hWeapon.Get()->GetAttachment( number, matrix );

	return BaseClass::GetAttachment( number, matrix );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseViewModel::GetAttachment( int number, Vector &origin )
{
	if ( m_hWeapon.Get() && m_hWeapon.Get()->WantsToOverrideViewmodelAttachments() )
		return m_hWeapon.Get()->GetAttachment( number, origin );

	return BaseClass::GetAttachment( number, origin );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseViewModel::GetAttachment( int number, Vector &origin, QAngle &angles )
{
	if ( m_hWeapon.Get() && m_hWeapon.Get()->WantsToOverrideViewmodelAttachments() )
		return m_hWeapon.Get()->GetAttachment( number, origin, angles );

	return BaseClass::GetAttachment( number, origin, angles );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseViewModel::GetAttachmentVelocity( int number, Vector &originVel, Quaternion &angleVel )
{
	if ( m_hWeapon.Get() && m_hWeapon.Get()->WantsToOverrideViewmodelAttachments() )
		return m_hWeapon.Get()->GetAttachmentVelocity( number, originVel, angleVel );

	return BaseClass::GetAttachmentVelocity( number, originVel, angleVel );
}

#endif

