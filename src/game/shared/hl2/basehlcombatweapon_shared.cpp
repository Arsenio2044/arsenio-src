//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "basehlcombatweapon_shared.h"

#include "hl2_player_shared.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"
#include "engine/IEngineSound.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS(basehlcombatweapon, CBaseHLCombatWeapon);

IMPLEMENT_NETWORKCLASS_ALIASED(BaseHLCombatWeapon, DT_BaseHLCombatWeapon)

BEGIN_NETWORK_TABLE(CBaseHLCombatWeapon, DT_BaseHLCombatWeapon)
#if !defined( CLIENT_DLL )
//	SendPropInt( SENDINFO( m_bReflectViewModelAnimations ), 1, SPROP_UNSIGNED ),
#else
//	RecvPropInt( RECVINFO( m_bReflectViewModelAnimations ) ),
#endif
END_NETWORK_TABLE()


#if !defined( CLIENT_DLL )

#include "globalstate.h"

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC(CBaseHLCombatWeapon)

DEFINE_FIELD(m_bLowered, FIELD_BOOLEAN),
DEFINE_FIELD(m_flRaiseTime, FIELD_TIME),
DEFINE_FIELD(m_flHolsterTime, FIELD_TIME),
DEFINE_FIELD(m_iPrimaryAttacks, FIELD_INTEGER),
DEFINE_FIELD(m_iSecondaryAttacks, FIELD_INTEGER),


END_DATADESC()

#endif

BEGIN_PREDICTION_DATA(CBaseHLCombatWeapon)
END_PREDICTION_DATA()

ConVar sk_auto_reload_time("sk_auto_reload_time", "3", FCVAR_REPLICATED);

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseHLCombatWeapon::ItemHolsterFrame(void)
{
	BaseClass::ItemHolsterFrame();

	// Must be player held
	if (GetOwner() && GetOwner()->IsPlayer() == false)
		return;

	// We can't be active
	if (GetOwner()->GetActiveWeapon() == this)
		return;

	// If it's been longer than three seconds, reload
	if ((gpGlobals->curtime - m_flHolsterTime) > sk_auto_reload_time.GetFloat())
	{
		// Just load the clip with no animations
		FinishReload();
		m_flHolsterTime = gpGlobals->curtime;
	}
}

bool CBaseHLCombatWeapon::CanSprint()
{
	CHL2_Player *pPlayer = assert_cast<CHL2_Player *>(GetOwner());
	if (pPlayer && SelectWeightedSequence(ACT_VM_SPRINT) == ACTIVITY_NOT_AVAILABLE)
		return false;

	return true;
}

bool CBaseHLCombatWeapon::CanWalkBob()
{
	CHL2_Player *pPlayer = assert_cast<CHL2_Player *>(GetOwner());
	if (pPlayer && SelectWeightedSequence(ACT_VM_WALK) == ACTIVITY_NOT_AVAILABLE)
		return false;

	return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CBaseHLCombatWeapon::CanLower()
{
	if (SelectWeightedSequence(ACT_VM_IDLE_LOWERED) == ACTIVITY_NOT_AVAILABLE)
		return false;
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Drops the weapon into a lowered pose
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseHLCombatWeapon::Lower(void)
{
	//Don't bother if we don't have the animation
	if (SelectWeightedSequence(ACT_VM_IDLE_LOWERED) == ACTIVITY_NOT_AVAILABLE)
		return false;

	m_bLowered = true;
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Brings the weapon up to the ready position
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseHLCombatWeapon::Ready(void)
{
	//Don't bother if we don't have the animation
	if (SelectWeightedSequence(ACT_VM_LOWERED_TO_IDLE) == ACTIVITY_NOT_AVAILABLE)
		return false;

	m_bLowered = false;
	m_flRaiseTime = gpGlobals->curtime + 0.5f;
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseHLCombatWeapon::Deploy(void)
{
	// If we should be lowered, deploy in the lowered position
	// We have to ask the player if the last time it checked, the weapon was lowered
	if (GetOwner() && GetOwner()->IsPlayer())
	{
		CHL2_Player *pPlayer = assert_cast<CHL2_Player*>(GetOwner());
		if (pPlayer->IsWeaponLowered())
		{
			if (SelectWeightedSequence(ACT_VM_IDLE_LOWERED) != ACTIVITY_NOT_AVAILABLE)
			{
				if (DefaultDeploy((char*)GetViewModel(), (char*)GetWorldModel(), ACT_VM_IDLE_LOWERED, (char*)GetAnimPrefix()))
				{
					m_bLowered = true;

					// Stomp the next attack time to fix the fact that the lower idles are long
					pPlayer->SetNextAttack(gpGlobals->curtime + 1.0);
					m_flNextPrimaryAttack = gpGlobals->curtime + 1.0;
					m_flNextSecondaryAttack = gpGlobals->curtime + 1.0;
					return true;
				}
			}
		}
	}

	m_bLowered = false;
	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseHLCombatWeapon::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	if (BaseClass::Holster(pSwitchingTo))
	{
		m_flHolsterTime = gpGlobals->curtime;
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseHLCombatWeapon::WeaponShouldBeLowered(void)
{
	// Can't be in the middle of another animation
	if (GetIdealActivity() != ACT_VM_IDLE_LOWERED && GetIdealActivity() != ACT_VM_IDLE &&
		GetIdealActivity() != ACT_VM_IDLE_TO_LOWERED && GetIdealActivity() != ACT_VM_LOWERED_TO_IDLE)
		return false;

	if (m_bLowered)
		return true;

#if !defined( CLIENT_DLL )

	if (GlobalEntity_GetState("friendly_encounter") == GLOBAL_ON)
		return true;

#endif

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Allows the weapon to choose proper weapon idle animation
//-----------------------------------------------------------------------------
void CBaseHLCombatWeapon::WeaponIdle(void)
{
	CHL2_Player *pPlayer = static_cast<CHL2_Player *>(GetOwner());
	if (!pPlayer)
		return;

	float speed = pPlayer->GetLocalVelocity().Length2D();

	if ( pPlayer->IsSprinting() && speed >= 290 )
	{
		int iActivity = GetActivity();
		if (HasWeaponIdleTimeElapsed() || 
			
			(
			GetActivity() == GetIdleActivity() ||
			GetActivity() == GetWalkActivity() ||
			GetActivity() == GetIdleLoweredActivity()
			) ||
			GetActivity() == ACT_VM_IDLE_TO_LOWERED ||
			GetActivity() == ACT_VM_LOWERED_TO_IDLE
			)
			
			// (idling || not kicking && activity == idle, walk or lowered || 
		{
			iActivity = GetSprintActivity();
		}

		int iSequence = SelectWeightedSequence(GetIdleActivity());
		if (iSequence >= 0 && iActivity != GetActivity())
		{
			SendWeaponAnim(iActivity);
		}
}

	//See if we should idle high or low
	else if (WeaponShouldBeLowered())
	{
#if !defined( CLIENT_DLL )
		pPlayer->Weapon_Lower();
#endif

		// Move to lowered position if we're not there yet
		if (GetActivity() != GetIdleLoweredActivity() && GetActivity() != ACT_VM_IDLE_TO_LOWERED
			&& GetActivity() != ACT_TRANSITION)
		{
			SendWeaponAnim(GetIdleLoweredActivity());
		}
		else if (HasWeaponIdleTimeElapsed())
		{
			// Keep idling low
			SendWeaponAnim(GetIdleLoweredActivity());
		}
	}
	else if (CanWalkBob() && speed >= 110 && pPlayer->GetWaterLevel() != 3 && (pPlayer->GetFlags() & FL_ONGROUND))
	{
		if (GetActivity() != GetWalkActivity() && (GetActivity() == GetIdleActivity() || GetActivity() == GetSprintActivity()))
		{
			SendWeaponAnim(GetWalkActivity());
		}
		else if (HasWeaponIdleTimeElapsed())
		{
			SendWeaponAnim(GetWalkActivity());
		}
	}
	else
	{
		// See if we need to raise immediately
		if (m_flRaiseTime < gpGlobals->curtime && GetActivity() == GetIdleLoweredActivity())
		{
			SendWeaponAnim(GetIdleActivity());
		}

		else if (speed <= (pPlayer->IsSuitEquipped() ? 300 : 200) && GetActivity() == GetSprintActivity())
		{
			SendWeaponAnim(GetIdleActivity());
		}
		else if (speed <= 100 && GetActivity() == GetWalkActivity())
		{
			SendWeaponAnim(GetIdleActivity());
		}
		else if (HasWeaponIdleTimeElapsed())
		{
			if (gpGlobals->curtime >= m_flNextFidgetTime)
			{
				//DevMsg("ACT_VM_FIDGET time!\n");
				SendWeaponAnim(ACT_VM_FIDGET);
				m_flNextFidgetTime = gpGlobals->curtime + 9.0f;

				// Player will say random voice lines.
				CPASAttenuationFilter filter(this);
				filter.UsePredictionRules();
				EmitSound(filter, entindex(), "Player.Rand");
			}
			else
			{
				SendWeaponAnim(GetIdleActivity());
			}
		}
	}
}


float	g_lateralBob;
float	g_verticalBob;

#if defined( CLIENT_DLL ) && ( !defined( HL2MP ) && defined( IVENGINE2 ) )

#define	HL2_BOB_CYCLE_MIN	0.5f
#define	HL2_BOB_CYCLE_MAX	0.45f
#define	HL2_BOB			0.002f
#define	HL2_BOB_UP		0.5f


static ConVar	cl_bobcycle("cl_bobcycle", "0.5");
static ConVar	cl_bob("cl_bob", "0.002");
static ConVar	cl_bobup("cl_bobup", "0.5");

// Register these cvars if needed for easy tweaking TUX: HAHAHAHAHAHAHAHAHA
static ConVar	v_iyaw_cycle("v_iyaw_cycle", "2"/*, FCVAR_UNREGISTERED*/);
static ConVar	v_iroll_cycle("v_iroll_cycle", "0.5"/*, FCVAR_UNREGISTERED*/);
static ConVar	v_ipitch_cycle("v_ipitch_cycle", "1"/*, FCVAR_UNREGISTERED*/);
static ConVar	v_iyaw_level("v_iyaw_level", "0.3"/*, FCVAR_UNREGISTERED*/);
static ConVar	v_iroll_level("v_iroll_level", "0.1"/*, FCVAR_UNREGISTERED*/);
static ConVar	v_ipitch_level("v_ipitch_level", "0.3"/*, FCVAR_UNREGISTERED*/);



//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
float CBaseHLCombatWeapon::CalcViewmodelBob(void)
{


		static	float bobtime;
		static	float lastbobtime;
		float	cycle;

		CBasePlayer* player = ToBasePlayer(GetOwner());
		//Assert( player );

		//NOTENOTE: For now, let this cycle continue when in the air, because it snaps badly without it

		if ((!gpGlobals->frametime) || (player == NULL))
		{
			//NOTENOTE: We don't use this return value in our case (need to restructure the calculation function setup!)
			return 0.0f;// just use old value
		}

		//Find the speed of the player
		float speed = player->GetLocalVelocity().Length2D();



		//FIXME: This maximum speed value must come from the server.
		//		 MaxSpeed() is not sufficient for dealing with sprinting - jdw

		speed = clamp(speed, -320, 320);

		float bob_offset = RemapVal(speed, 0, 320, 0.0f, 1.0f);

		float time_delta = (gpGlobals->curtime - lastbobtime);

		bobtime += time_delta * bob_offset;
		lastbobtime = gpGlobals->curtime;

		//Calculate the vertical bob
		cycle = bobtime - (int)(bobtime / HL2_BOB_CYCLE_MAX) * HL2_BOB_CYCLE_MAX;
		cycle /= HL2_BOB_CYCLE_MAX;

		if (cycle < HL2_BOB_UP)
		{
			cycle = M_PI * cycle / HL2_BOB_UP;
		}
		else
		{
			cycle = M_PI + M_PI * (cycle - HL2_BOB_UP) / (1.0 - HL2_BOB_UP);
		}




		g_verticalBob = speed * 0.0015f;
		g_verticalBob = g_verticalBob * 0.3 + g_verticalBob * 0.7 * sin(cycle);



		// Calculate the vertical bob kick
		float speed_delta_z = player->GetLocalVelocity().z - m_flPrevPlayerVelZ;

		// The bigger the accel upwards, the more the kick downwards
		if (speed_delta_z > 30)
		{
			m_bTgtBobKickZ = true;
		}
		if (m_bTgtBobKickZ)
		{
			m_flBobKickZ -= 25 * time_delta; // 40 units/sec
			if (g_verticalBob + m_flBobKickZ < -2.5)
			{
				// reached the lowest point
				m_bTgtBobKickZ = false;
			}
		}
		else if (m_flBobKickZ < 0) {
			// decay the bob kick...
			m_flBobKickZ += 13 * time_delta;
		}
		else {
			m_flBobKickZ = 0;
		}

		g_verticalBob = clamp(g_verticalBob + m_flBobKickZ, -8.0f, 5.0f);

		m_flPrevPlayerVelZ = player->GetLocalVelocity().z;


		//Calculate the lateral bob
		cycle = bobtime - (int)(bobtime / HL2_BOB_CYCLE_MAX * 2) * HL2_BOB_CYCLE_MAX * 2;
		cycle /= HL2_BOB_CYCLE_MAX * 2;

		if (cycle < HL2_BOB_UP)
		{
			cycle = M_PI * cycle / HL2_BOB_UP;
		}
		else
		{
			cycle = M_PI + M_PI * (cycle - HL2_BOB_UP) / (1.0 - HL2_BOB_UP);
		}

		g_lateralBob = speed * 0.015f;
		g_lateralBob = g_lateralBob * 0.3 + g_lateralBob * 0.7 * sin(cycle);
		g_lateralBob = clamp(g_lateralBob, -7.0f, 4.0f);

		//NOTENOTE: We don't use this return value in our case (need to restructure the calculation function setup!)
		return 0.0f;

	
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &origin - 
//			&angles - 
//			viewmodelindex - 
//-----------------------------------------------------------------------------
void CBaseHLCombatWeapon::AddViewmodelBob(CBaseViewModel *viewmodel, Vector &origin, QAngle &angles)
{



		Vector    forward, right;
		AngleVectors(angles, &forward, &right, NULL);

		CalcViewmodelBob();

		// Apply bob, but scaled down to 40%
		VectorMA(origin, g_verticalBob * 0.3f, forward, origin);

		// Z bob a bit more
		origin[2] += g_verticalBob * 0.3f;

	// bob the angles
	angles[ROLL] += g_verticalBob * 0.3f;
	angles[PITCH] -= g_verticalBob * 0.3f;

	angles[YAW] -= g_lateralBob  * 0.3f;



	// special adjustment for crowbar slash attack
	C_BasePlayer* pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer && pPlayer->m_nMeleeState == MELEE_SLASH &&
		FClassnameIs(STRING(pPlayer->GetActiveWeapon()), "weapon_crowbar"))
	{
		m_flRollAdj = 50.0f;
	}
	else
	{
		m_flRollAdj = MAX(m_flRollAdj - (gpGlobals->frametime * 360), 0.0f);
	}
	angles[ROLL] += m_flRollAdj;
	angles[YAW] -= m_flRollAdj / 4.0f;
	if (m_flBobKickZ < 5.0f && m_flBobKickZ > -5.0f)
		angles[PITCH] -= m_flBobKickZ;

	VectorMA(origin, g_lateralBob * 0.3f, right, origin);

	//ConVar g_lateralBob("g_lateralBob", "0.3", FCVAR_REPLICATED);

	// Command won't work


}

//-----------------------------------------------------------------------------
Vector CBaseHLCombatWeapon::GetBulletSpread(WeaponProficiency_t proficiency)
{
	return BaseClass::GetBulletSpread(proficiency);
}

//-----------------------------------------------------------------------------
float CBaseHLCombatWeapon::GetSpreadBias(WeaponProficiency_t proficiency)
{
	return BaseClass::GetSpreadBias(proficiency);
}
//-----------------------------------------------------------------------------

const WeaponProficiencyInfo_t *CBaseHLCombatWeapon::GetProficiencyValues()
{
	return NULL;
}

#else

// Server stubs
float CBaseHLCombatWeapon::CalcViewmodelBob(void)
{
	return 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &origin - 
//			&angles - 
//			viewmodelindex - 
//-----------------------------------------------------------------------------
void CBaseHLCombatWeapon::AddViewmodelBob(CBaseViewModel *viewmodel, Vector &origin, QAngle &angles)
{
}


//-----------------------------------------------------------------------------
Vector CBaseHLCombatWeapon::GetBulletSpread(WeaponProficiency_t proficiency)
{
	Vector baseSpread = BaseClass::GetBulletSpread(proficiency);

	const WeaponProficiencyInfo_t *pProficiencyValues = GetProficiencyValues();
	float flModifier = (pProficiencyValues)[proficiency].spreadscale;
	return (baseSpread * flModifier);
}

//-----------------------------------------------------------------------------
float CBaseHLCombatWeapon::GetSpreadBias(WeaponProficiency_t proficiency)
{
	const WeaponProficiencyInfo_t *pProficiencyValues = GetProficiencyValues();
	return (pProficiencyValues)[proficiency].bias;
}

//-----------------------------------------------------------------------------
const WeaponProficiencyInfo_t *CBaseHLCombatWeapon::GetProficiencyValues()
{
	return GetDefaultProficiencyValues();
}

//-----------------------------------------------------------------------------
const WeaponProficiencyInfo_t *CBaseHLCombatWeapon::GetDefaultProficiencyValues()
{
	// Weapon proficiency table. Keep this in sync with WeaponProficiency_t enum in the header!!
	static WeaponProficiencyInfo_t g_BaseWeaponProficiencyTable[] =
	{
		{ 2.50, 1.0 },
		{ 2.00, 1.0 },
		{ 1.50, 1.0 },
		{ 1.25, 1.0 },
		{ 1.00, 1.0 },
	};

	COMPILE_TIME_ASSERT(ARRAYSIZE(g_BaseWeaponProficiencyTable) == WEAPON_PROFICIENCY_PERFECT + 1);

	return g_BaseWeaponProficiencyTable;
}

#endif