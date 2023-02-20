//========= Copyright Glitch Software, All rights reserved. ============//
//
// Purpose: The cut concept from the HL2 beta.
// TODO: Overhaul file because this is old af.
//
//=============================================================================//

#include "cbase.h"
#include "ai_hull.h"
#include "ai_motor.h"
#include "npc_overlord.h"
#include "bitstring.h"
#include "engine/IEngineSound.h"
#include "soundent.h"
#include "ndebugoverlay.h"
#include "npcevent.h"
#include "hl2/hl2_player.h"
#include "game.h"
#include "ammodef.h"
#include "explode.h"
#include "ai_memory.h"
#include "Sprite.h"
#include "soundenvelope.h"
#include "weapon_physcannon.h"
#include "hl2_gamerules.h"
#include "gameweaponmanager.h"
#include "movevars_shared.h"
#include "vehicle_base.h"


#ifdef AR
#include "arsenio_baseshadownpc.h" // TUX: For future use.
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar	sk_overlord_health("sk_overlord_health", "15000");
ConVar	sk_overlord_kick("sk_overlord_kick", "35");

ConVar sk_overlorda_health("sk_overlorda_health", "0");
ConVar sk_overlorda_kick("sk_overlorda_kick", "0");

// Whether or not the synth guard should spawn health on death
ConVar overlorda_spawn_health("overlorda_spawn_health", "1");



extern ConVar sk_plr_dmg_buckshot;
extern ConVar sk_plr_num_shotgun_pellets;

//Whether or not the synth should spawn health on death
ConVar	overlordpawn_health("overlordpawn_health", "1");

ConVar  overlord_soldier_prob("overlord_soldier_prob", "0", 0,
	"Every synth soldier has this chance to spawn a hunter");

LINK_ENTITY_TO_CLASS(npc_overlord, CNPC_OverLord);


#define AE_SOLDIER_BLOCK_PHYSICS		20 // trying to block an incoming physics object
#define COMBINE_AE_ROCKET				( 2 )
#define	OVERLORD_AE_HOP			1

#define MD_BC_YAW		0
#define MD_BC_PITCH		1
#define MD_AP_LGUN		2
#define MD_AP_RGUN		1
#define MD_YAW_SPEED	24
#define MD_PITCH_SPEED  12

#define MD_FULLAMMO	500

#define	OVERLORD_AE_SHOVE	3

#define OVERLORD_AE_SHAKEIMPACT 22

extern Activity ACT_WALK_EASY;
extern Activity ACT_WALK_MARCH;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_OverLord::Spawn(void)
{
	Precache();
	SetModel(STRING(GetModelName()));

	float flGravity = sv_gravity.GetFloat();

	// throw the squid up into the air on this frame.
	if (GetFlags() & FL_ONGROUND)
	{
		SetGroundEntity(NULL);
	}

	// jump into air for 0.8 (24/30) seconds
	Vector vecVel = GetAbsVelocity();
	vecVel.z += (0.625 * flGravity) * 0.5;
	SetAbsVelocity(vecVel);

	if (IsElite())
	{
		// Stronger, tougher.
		SetHealth(sk_overlorda_health.GetFloat());
		SetMaxHealth(sk_overlorda_health.GetFloat());
		SetKickDamage(sk_overlorda_kick.GetFloat());
	}
	else
	{
		SetHealth(sk_overlord_health.GetFloat());
		SetMaxHealth(sk_overlord_health.GetFloat());
		SetKickDamage(sk_overlord_kick.GetFloat());
	}

	CapabilitiesAdd(bits_CAP_ANIMATEDFACE);
	CapabilitiesAdd(bits_CAP_MOVE_SHOOT);
	CapabilitiesAdd(bits_CAP_DOORS_GROUP);
	CapabilitiesAdd(bits_CAP_MOVE_JUMP | bits_CAP_MOVE_CLIMB);

	SetBoneController(MD_BC_YAW, 10);
	SetBoneController(MD_BC_PITCH, 0);

	BaseClass::Spawn();

#if HL2_EPISODIC
	if (m_iUseMarch && !HasSpawnFlags(SF_NPC_START_EFFICIENT))
	{
		Msg("Soldier %s is set to use march anim, but is not an efficient AI. The blended march anim can only be used for dead-ahead walks!\n", GetDebugName());
	}


	// Maybe spawn a hunter if the player wants hunters
	if (overlord_soldier_prob.GetFloat() > 0)
	{
		if (RandomFloat() <= overlord_soldier_prob.GetFloat())
		{
			// one more check - don't spawn a hunter if there's already two nearby
			int nearby_hunters = 0;
			CBaseEntity* pSearch[32];
			int nNumEnemies = UTIL_EntitiesInSphere(pSearch, ARRAYSIZE(pSearch), GetAbsOrigin(), 1000, FL_NPC);

			for (int i = 0; i < nNumEnemies && nearby_hunters < 2; i++)
			{
				// We only care about hunters
				if (pSearch[i] == NULL || pSearch[i]->Classify() != CLASS_SHADOW)
					continue;

				++nearby_hunters;
			}
			if (nearby_hunters > 2)
			{
				return;
			}

			// Yep, spawn a hunter
			CBaseEntity* pent = CreateEntityByName("npc_hulk");
			CAI_BaseNPC* pHunter = dynamic_cast<CAI_BaseNPC*>(pent);
			if (!pHunter)
				return;

			pHunter->Precache(); // should already be precached from the synth soldier's precache

			Vector vecHunterOrigin;
			int attempts = 5;
			while (attempts > 0)
			{
				if (CAI_BaseNPC::FindSpotForNPCInRadius(
					&vecHunterOrigin, GetAbsOrigin(), pHunter, 220 - (attempts * 30), true) &&
					pHunter->CanPlantHere(vecHunterOrigin))
				{
					// found a spot
					pHunter->SetAbsOrigin(vecHunterOrigin);
					pHunter->AddSpawnFlags(SF_NPC_GAG); // no FALL_TO_GROUND, i.e. teleport to ground

					pHunter->Spawn();
					break;
				}
				else {
					attempts--;
				}
			}
		}
	}
#endif

}

Class_T	CNPC_OverLord::Classify()
{

	return CLASS_SHADOW;
}

bool CNPC_OverLord::IsJumpLegal(const Vector& startPos, const Vector& apex, const Vector& endPos) const
{
	const float MAX_JUMP_RISE = 400.0f;
	const float MAX_JUMP_DISTANCE = 800.0f;
	const float MAX_JUMP_DROP = 2048.0f;

	if (BaseClass::IsJumpLegal(startPos, apex, endPos, MAX_JUMP_RISE, MAX_JUMP_DROP, MAX_JUMP_DISTANCE))
	{
		// Hang onto the jump distance. The AI is going to want it.
		m_flJumpDist = (startPos - endPos).Length();

		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CNPC_OverLord::Precache()
{
	const char* pModelName = STRING(GetModelName());

	if (!Q_stricmp(pModelName, "models/overlorduper_soldier.mdl"))
	{
		m_fIsElite = true;
	}
	else
	{
		m_fIsElite = false;
	}

	if (!GetModelName())
	{
		SetModelName(MAKE_STRING("models/overlord/overlord.mdl"));
	}

	PrecacheModel(STRING(GetModelName()));

	UTIL_PrecacheOther("item_healthvial");
	UTIL_PrecacheOther("weapon_frag");
	UTIL_PrecacheOther("item_ammo_ar2_altfire");

	PrecacheScriptSound("NPC_MissileDefense.Attack");
	PrecacheScriptSound("NPC_MissileDefense.Reload");
	PrecacheScriptSound("NPC_MissileDefense.Turn");

	if (overlord_soldier_prob.GetFloat() > 0)
	{
		UTIL_PrecacheOther("npc_hunter");
	}

	BaseClass::Precache();
}

void CNPC_OverLord::GetGunAim(Vector* vecAim)
{
	Vector vecPos;
	QAngle vecAng;

	GetAttachment(MD_AP_LGUN, vecPos, vecAng);

	vecAng.x = GetLocalAngles().x + GetBoneController(MD_BC_PITCH);
	vecAng.z = 0;
	vecAng.y = GetLocalAngles().y + GetBoneController(MD_BC_YAW);

	Vector vecForward;
	AngleVectors(vecAng, &vecForward);

	*vecAim = vecForward;
}

void CNPC_OverLord::DeathSound(const CTakeDamageInfo& info)
{
	// NOTE: The response system deals with this at the moment
	if (GetFlags() & FL_DISSOLVING)
		return;

	GetSentences()->Speak("COMBINE_DIE", SENTENCE_PRIORITY_INVALID, SENTENCE_CRITERIA_ALWAYS);
}




//-----------------------------------------------------------------------------
// Purpose: Soldiers use CAN_RANGE_ATTACK2 to indicate whether they can throw
//			a grenade. Because they check only every half-second or so, this
//			condition must persist until it is updated again by the code
//			that determines whether a grenade can be thrown, so prevent the 
//			base class from clearing it out. (sjb)
//-----------------------------------------------------------------------------
void CNPC_OverLord::ClearAttackConditions()
{
	bool fCanRangeAttack2 = HasCondition(COND_CAN_RANGE_ATTACK2);

	// Call the base class.
	BaseClass::ClearAttackConditions();

	if (fCanRangeAttack2)
	{
		// We don't allow the base class to clear this condition because we
		// don't sense for it every frame.
		SetCondition(COND_CAN_RANGE_ATTACK2);
	}
}

void CNPC_OverLord::PrescheduleThink(void)
{
	/*//FIXME: This doesn't need to be in here, it's all debug info
	if( HasCondition( COND_HEAR_PHYSICS_DANGER ) )
	{
		// Don't react unless we see the item!!
		CSound *pSound = NULL;

		pSound = GetLoudestSoundOfType( SOUND_PHYSICS_DANGER );

		if( pSound )
		{
			if( FInViewCone( pSound->GetSoundReactOrigin() ) )
			{
				DevMsg( "OH CRAP!\n" );
				NDebugOverlay::Line( EyePosition(), pSound->GetSoundReactOrigin(), 0, 0, 255, false, 2.0f );
			}
		}
	}
	*/

	BaseClass::PrescheduleThink();
}

#define	COMBINEGUARD_MELEE1_RANGE	98 //92 initially
#define	COMBINEGUARD_MELEE1_CONE	0.5f

#define	COMBINEGUARD_RANGE1_RANGE	1024
#define	COMBINEGUARD_RANGE1_CONE	0.0f

void CNPC_OverLord::Shove(void) 
{
	if (GetEnemy() == NULL)
		return;

	CBaseEntity* pHurt = NULL;

	Vector forward;
	AngleVectors(GetLocalAngles(), &forward);

	float flDist = (GetEnemy()->GetAbsOrigin() - GetAbsOrigin()).Length();
	Vector2D v2LOS = (GetEnemy()->GetAbsOrigin() - GetAbsOrigin()).AsVector2D();
	Vector2DNormalize(v2LOS);
	float flDot = DotProduct2D(v2LOS, forward.AsVector2D());

	flDist -= GetEnemy()->WorldAlignSize().x * 0.5f;

	if (flDist < COMBINEGUARD_MELEE1_RANGE && flDot >= COMBINEGUARD_MELEE1_CONE)
	{
		Vector vStart = GetAbsOrigin();
		vStart.z += WorldAlignSize().z * 0.5;

		Vector vEnd = GetEnemy()->GetAbsOrigin();
		vEnd.z += GetEnemy()->WorldAlignSize().z * 0.5;

		pHurt = CheckTraceHullAttack(vStart, vEnd, Vector(-16, -16, 0), Vector(16, 16, 24), 25, DMG_CLUB);
	}

	if (pHurt)
	{
		pHurt->ViewPunch(QAngle(-20, 0, 20));

		UTIL_ScreenShake(pHurt->GetAbsOrigin(), 100.0, 1.5, 1.0, 2, SHAKE_START);

		color32 white = { 255, 255, 255, 64 };
		UTIL_ScreenFade(pHurt, white, 0.5f, 0.1f, FFADE_IN);

		if (pHurt->IsPlayer())
		{
			Vector forward, up;
			AngleVectors(GetLocalAngles(), &forward, NULL, &up);
			pHurt->ApplyAbsVelocityImpulse(forward * 300 + up * 250);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Allows for modification of the interrupt mask for the current schedule.
//			In the most cases the base implementation should be called first.
//-----------------------------------------------------------------------------
void CNPC_OverLord::BuildScheduleTestBits(void)
{
	//Interrupt any schedule with physics danger (as long as I'm not moving or already trying to block)
	if (m_flGroundSpeed == 0.0 && !IsCurSchedule(SCHED_FLINCH_PHYSICS))
	{
		SetCustomInterruptCondition(COND_HEAR_PHYSICS_DANGER);
	}

	BaseClass::BuildScheduleTestBits();
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
int CNPC_OverLord::SelectSchedule(void)
{
	return BaseClass::SelectSchedule();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
float CNPC_OverLord::GetHitgroupDamageMultiplier(int iHitGroup, const CTakeDamageInfo& info)
{
	switch (iHitGroup)
	{
	case HITGROUP_HEAD:
	{
		// Soldiers take double headshot damage
		return 2.0f;
	}
	}

	return BaseClass::GetHitgroupDamageMultiplier(iHitGroup, info);
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_OverLord::HandleAnimEvent(animevent_t* pEvent)
{
	switch (pEvent->event)
	{
	case AE_SOLDIER_BLOCK_PHYSICS:
		DevMsg("BLOCKING!\n");
		m_fIsBlocking = true;
		break;

	case COMBINE_AE_ROCKET:

	//	CHomingMissile* missile =
	//	CHomingMissile::Create(muzzlePoint, launch_angle, this, target, m_nRocketsQueued - 1);
		break;

	case OVERLORD_AE_SHAKEIMPACT:
		Shove();
		UTIL_ScreenShake(GetAbsOrigin(), 25.0, 150.0, 1.0, 750, SHAKE_START);
		break;

	case OVERLORD_AE_HOP:
	{
		float flGravity = sv_gravity.GetFloat();

		// throw the squid up into the air on this frame.
		if (GetFlags() & FL_ONGROUND)
		{
			SetGroundEntity(NULL);
		}

		// jump into air for 0.8 (24/30) seconds
		Vector vecVel = GetAbsVelocity();
		vecVel.z += (0.625 * flGravity) * 0.5;
		SetAbsVelocity(vecVel);
	}

	break;

	case OVERLORD_AE_SHOVE:
		Shove();
		break;

	default:
		BaseClass::HandleAnimEvent(pEvent);
		break;
	}
}

void CNPC_OverLord::OnChangeActivity(Activity eNewActivity)
{
	// Any new sequence stops us blocking.
	m_fIsBlocking = false;

	BaseClass::OnChangeActivity(eNewActivity);

#if HL2_EPISODIC
	// Give each trooper a varied look for his march. Done here because if you do it earlier (eg Spawn, StartTask), the
	// pose param gets overwritten.
	if (m_iUseMarch)
	{
		SetPoseParameter("casual", RandomFloat());
	}
#endif
}

void CNPC_OverLord::OnListened()
{
	BaseClass::OnListened();

	if (HasCondition(COND_HEAR_DANGER) && HasCondition(COND_HEAR_PHYSICS_DANGER))
	{
		if (HasInterruptCondition(COND_HEAR_DANGER))
		{
			ClearCondition(COND_HEAR_PHYSICS_DANGER);
		}
	}

	// debugging to find missed schedules
#if 0
	if (HasCondition(COND_HEAR_DANGER) && !HasInterruptCondition(COND_HEAR_DANGER))
	{
		DevMsg("Ignore danger in %s\n", GetCurSchedule()->GetName());
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &info - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
void CNPC_OverLord::Event_Killed(const CTakeDamageInfo& info)
{
	// Don't bother if we've been told not to, or the player has a megaphyscannon
	if (overlordpawn_health.GetBool() == false || PlayerHasMegaPhysCannon())
	{
		BaseClass::Event_Killed(info);
		return;
	}

	CBasePlayer* pPlayer = ToBasePlayer(info.GetAttacker());

	if (!pPlayer)
	{
		CPropVehicleDriveable* pVehicle = dynamic_cast<CPropVehicleDriveable*>(info.GetAttacker());
		if (pVehicle && pVehicle->GetDriver() && pVehicle->GetDriver()->IsPlayer())
		{
			pPlayer = assert_cast<CBasePlayer*>(pVehicle->GetDriver());
		}
	}

	if (pPlayer != NULL)
	{
		// Elites drop alt-fire ammo, so long as they weren't killed by dissolving.
		if (IsElite())
		{
#ifdef HL2_EPISODIC
			if (HasSpawnFlags(SF_COMBINE_NO_AR2DROP) == false)
#endif
			{
				CBaseEntity* pItem = DropItem("item_ammo_ar2_altfire", WorldSpaceCenter() + RandomVector(-4, 4), RandomAngle(0, 360));

				if (pItem)
				{
					IPhysicsObject* pObj = pItem->VPhysicsGetObject();

					if (pObj)
					{
						Vector			vel = RandomVector(-64.0f, 64.0f);
						AngularImpulse	angImp = RandomAngularImpulse(-300.0f, 300.0f);

						vel[2] = 0.0f;
						pObj->AddVelocity(&vel, &angImp);
					}

					if (info.GetDamageType() & DMG_DISSOLVE)
					{
						CBaseAnimating* pAnimating = dynamic_cast<CBaseAnimating*>(pItem);

						if (pAnimating)
						{
							pAnimating->Dissolve(NULL, gpGlobals->curtime, false, ENTITY_DISSOLVE_NORMAL);
						}
					}
					else
					{
						WeaponManager_AddManaged(pItem);
					}
				}
			}
		}

		CHalfLife2* pHL2GameRules = static_cast<CHalfLife2*>(g_pGameRules);

		// Attempt to drop health
		if (pHL2GameRules->NPC_ShouldDropHealth(pPlayer))
		{
			DropItem("item_healthvial", WorldSpaceCenter() + RandomVector(-4, 4), RandomAngle(0, 360));
			pHL2GameRules->NPC_DroppedHealth();
		}

		if (HasSpawnFlags(SF_COMBINE_NO_GRENADEDROP) == false)
		{
			// Attempt to drop a grenade
			if (pHL2GameRules->NPC_ShouldDropGrenade(pPlayer))
			{
				DropItem("weapon_frag", WorldSpaceCenter() + RandomVector(-4, 4), RandomAngle(0, 360));
				pHL2GameRules->NPC_DroppedGrenade();
			}
		}
	}

	BaseClass::Event_Killed(info);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &info - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_OverLord::IsLightDamage(const CTakeDamageInfo& info)
{
	return BaseClass::IsLightDamage(info);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &info - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_OverLord::IsHeavyDamage(const CTakeDamageInfo& info)
{
	// Combine considers AR2 fire to be heavy damage
	if (info.GetAmmoType() == GetAmmoDef()->Index("AR2"))


		return true;

	// 357 rounds are heavy damage
	if (info.GetAmmoType() == GetAmmoDef()->Index("357"))
		return true;

	// Shotgun blasts where at least half the pellets hit me are heavy damage
	if (info.GetDamageType() & DMG_BUCKSHOT)
	{
		int iHalfMax = sk_plr_dmg_buckshot.GetFloat() * sk_plr_num_shotgun_pellets.GetInt() * 0.5;
		if (info.GetDamage() >= iHalfMax)
			return true;
	}

	// Rollermine shocks
	if ((info.GetDamageType() & DMG_SHOCK) && hl2_episodic.GetBool())
	{
		return true;
	}

	return BaseClass::IsHeavyDamage(info);
}

#if HL2_EPISODIC
//-----------------------------------------------------------------------------
// Purpose: Translate base class activities into combot activites
//-----------------------------------------------------------------------------
Activity CNPC_OverLord::NPC_TranslateActivity(Activity eNewActivity)
{
	// If the special ep2_outland_05 "use march" flag is set, use the more casual marching anim.
	if (m_iUseMarch && eNewActivity == ACT_WALK)
	{
		eNewActivity = ACT_WALK_MARCH;
	}

	return BaseClass::NPC_TranslateActivity(eNewActivity);
}


//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC(CNPC_OverLord)

DEFINE_KEYFIELD(m_iUseMarch, FIELD_INTEGER, "usemarch"),

DEFINE_FIELD(m_iAmmoLoaded, FIELD_INTEGER),
DEFINE_FIELD(m_flReloadedTime, FIELD_TIME),
DEFINE_FIELD(m_vGunAng, FIELD_VECTOR),

END_DATADESC()
#endif

#define NOISE 0.035f
#define MD_ATTN_CANNON 0.4
//------------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CNPC_OverLord::FireCannons(void)
{
	// ----------------------------------------------
	//  Make sure I have an enemy
	// ----------------------------------------------
	if (GetEnemy() == NULL)
	{
		return;
	}

	// ----------------------------------------------
	//  Make sure I have ammo
	// ----------------------------------------------
	if (m_iAmmoLoaded < 1)
	{
		return;
	}
	// ----------------------------------------------
	// Make sure gun it pointing in right direction	
	// ----------------------------------------------
	Vector vGunDir;
	GetGunAim(&vGunDir);
	Vector vTargetPos;
	EnemyShootPosition(GetEnemy(), &vTargetPos);

	Vector vTargetDir = vTargetPos - GetAbsOrigin();
	VectorNormalize(vTargetDir);

	float fDotPr = DotProduct(vGunDir, vTargetDir);
	if (fDotPr < 0.95)
	{
		return;
	}

	// ----------------------------------------------
	// Check line of sight
	// ----------------------------------------------
	trace_t tr;
	AI_TraceLine(GetEnemy()->EyePosition(), GetAbsOrigin(), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);
	if (tr.fraction < 1.0)
	{
		return;
	}

	Vector vecRight;
	Vector vecDir;
	Vector vecCenter;
	AngleVectors(GetLocalAngles(), NULL, &vecRight, NULL);

	vecCenter = WorldSpaceCenter();

	if (GetEnemy() == NULL)
	{
		return;
	}

	bool fSound = false;
	if (random->RandomInt(0, 3) == 0)
	{
		fSound = true;
	}


	EmitSound("NPC_MissileDefense.Attack");

	Vector vecGun;
	QAngle vecAng;

	GetAttachment(MD_AP_LGUN, vecGun, vecAng);

	Vector vecTarget;
	EnemyShootPosition(GetEnemy(), &vecTarget);

	vecDir = vecTarget - vecCenter;
	VectorNormalize(vecDir);
	vecDir.x += random->RandomFloat(-NOISE, NOISE);
	vecDir.y += random->RandomFloat(-NOISE, NOISE);

	Vector vecStart = vecGun + vecDir * 110;
	Vector vecEnd = vecGun + vecDir * 4096;
	UTIL_Tracer(vecStart, vecEnd, 0, TRACER_DONT_USE_ATTACHMENT, 3000 + random->RandomFloat(0, 2000), fSound);

	vecDir = vecTarget - vecCenter;
	VectorNormalize(vecDir);
	vecDir.x += random->RandomFloat(-NOISE, NOISE);
	vecDir.y += random->RandomFloat(-NOISE, NOISE);
	vecDir.z += random->RandomFloat(-NOISE, NOISE);

	GetAttachment(MD_AP_RGUN, vecGun, vecAng);
	vecStart = vecGun + vecDir * 110;
	vecEnd = vecGun + vecDir * 4096;
	UTIL_Tracer(vecStart, vecEnd, 0, TRACER_DONT_USE_ATTACHMENT, 3000 + random->RandomFloat(0, 2000));

	m_iAmmoLoaded -= 2;

	if (m_iAmmoLoaded < 1)
	{
		// Incite a reload.
		EmitSound("NPC_MissileDefense.Reload");
		m_flReloadedTime = gpGlobals->curtime + 0.3;
		return;
	}

	// Do damage to the missile based on distance.
	// if < 1, make damage 0.

	float flDist = (GetEnemy()->GetLocalOrigin() - vecGun).Length();
	float flDamage;

	flDamage = 4000 - flDist;

	flDamage /= 1000.0;

	if (flDamage > 0)
	{
		if (flDist <= 1500)
		{
			flDamage *= 2;
		}

		CTakeDamageInfo info(this, this, flDamage, DMG_MISSILEDEFENSE);
		CalculateBulletDamageForce(&info, GetAmmoDef()->Index("SMG1"), vecDir, GetEnemy()->GetAbsOrigin());
		GetEnemy()->TakeDamage(info);
	}
}

//------------------------------------------------------------------------------
// Purpose : Add a little prediction into my enemy aim position
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CNPC_OverLord::EnemyShootPosition(CBaseEntity* pEnemy, Vector* vPosition)
{
	// This should never happen, but just in case
	if (!pEnemy)
	{
		return;
	}

	*vPosition = pEnemy->GetAbsOrigin();

	// Add prediction but prevents us from flipping around as enemy approaches us
	float	flDist = (pEnemy->GetAbsOrigin() - GetAbsOrigin()).Length();
	Vector	vPredVel = pEnemy->GetSmoothedVelocity() * 0.5;
	if (flDist > vPredVel.Length())
	{
		*vPosition += vPredVel;
	}
}

//------------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CNPC_OverLord::AimGun(void)
{
	if (GetEnemy() == NULL)
	{
		StopSound("NPC_MissileDefense.Turn");
		return;
	}

	Vector forward, right, up;
	AngleVectors(GetLocalAngles(), &forward, &right, &up);

	// Get gun attachment points
	Vector vBasePos;
	QAngle vBaseAng;
	GetAttachment(MD_AP_LGUN, vBasePos, vBaseAng);

	Vector vTargetPos;
	EnemyShootPosition(GetEnemy(), &vTargetPos);

	Vector vTargetDir = vTargetPos - vBasePos;
	VectorNormalize(vTargetDir);

	Vector vecOut;
	vecOut.x = DotProduct(forward, vTargetDir);
	vecOut.y = -DotProduct(right, vTargetDir);
	vecOut.z = DotProduct(up, vTargetDir);

	QAngle angles;
	VectorAngles(vecOut, angles);

	if (angles.y > 180)
		angles.y = angles.y - 360;
	if (angles.y < -180)
		angles.y = angles.y + 360;
	if (angles.x > 180)
		angles.x = angles.x - 360;
	if (angles.x < -180)
		angles.x = angles.x + 360;

	float flOldX = m_vGunAng.x;
	float flOldY = m_vGunAng.y;

	if (angles.x > m_vGunAng.x)
		m_vGunAng.x = MIN(angles.x, m_vGunAng.x + MD_PITCH_SPEED);
	if (angles.x < m_vGunAng.x)
		m_vGunAng.x = MAX(angles.x, m_vGunAng.x - MD_PITCH_SPEED);
	if (angles.y > m_vGunAng.y)
		m_vGunAng.y = MIN(angles.y, m_vGunAng.y + MD_YAW_SPEED);
	if (angles.y < m_vGunAng.y)
		m_vGunAng.y = MAX(angles.y, m_vGunAng.y - MD_YAW_SPEED);

	m_vGunAng.y = SetBoneController(MD_BC_YAW, m_vGunAng.y);
	m_vGunAng.x = SetBoneController(MD_BC_PITCH, m_vGunAng.x);

	if (flOldX != m_vGunAng.x || flOldY != m_vGunAng.y)
	{
		EmitSound("NPC_MissileDefense.Turn");
	}
	else
	{
		StopSound("NPC_MissileDefense.Turn");
	}
}

