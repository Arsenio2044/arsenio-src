#include "cbase.h"
#include "weapon_phaseshifter.h"

LINK_ENTITY_TO_CLASS(weapon_phaseshifter, CWeaponPhaseShifter);

BEGIN_DATADESC(CWeaponPhaseShifter)
DEFINE_FIELD(m_flPhaseOutDuration, FIELD_FLOAT),
DEFINE_FIELD(m_flProjectileSpeed, FIELD_FLOAT),
END_DATADESC()

CWeaponPhaseShifter::CWeaponPhaseShifter()
{
	m_flPhaseOutDuration = 15.0f; // Set the default phase-out duration (in seconds)
	m_flProjectileSpeed = 5000.0f; // Set the default projectile speed (adjust as needed)
}

void CWeaponPhaseShifter::Precache()
{
	BaseClass::Precache();
	// Precache any necessary particle effects and sounds
}

void CWeaponPhaseShifter::PrimaryAttack()
{
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;

	if (m_flNextPrimaryAttack > gpGlobals->curtime)
		return;
	
	// Play weapon sound, muzzle flash, and other visual effects

	Vector vecShootOrigin, vecShootDir;
	vecShootOrigin = pPlayer->Weapon_ShootPosition();
	AngleVectors(pPlayer->EyeAngles(), &vecShootDir);

	Vector vecEnd = vecShootOrigin + vecShootDir * MAX_TRACE_LENGTH;

	trace_t tr;
	UTIL_TraceLine(vecShootOrigin, vecEnd, MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr);

	if (tr.DidHit())
	{
		// Check if the trace hit an enemy
		CBaseEntity* pEntity = tr.m_pEnt;
		if (pEntity && pEntity->IsNPC())
		{
			PhaseOutEnemy(pEntity);
		}
	}

	m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
}

void CWeaponPhaseShifter::ItemPostFrame()
{
	BaseClass::ItemPostFrame();

	// Add code here for any continuous effects or updates
}

void CWeaponPhaseShifter::PhaseOutEnemy(CBaseEntity* pEnemy)
{
	if (!pEnemy || !pEnemy->IsNPC())
		return;

	CBaseCombatCharacter* pNPC = pEnemy->MyCombatCharacterPointer();
	if (!pNPC)
		return;

	// Apply the phase-out effect to the enemy (disable movement, attack, etc.)
	pNPC->AddEffects(EF_NODRAW); // Make the enemy invisible
	pNPC->AddFlag(FL_NOTARGET); // Make the enemy not targetable
	pNPC->SetSolid(SOLID_NONE); // Make the enemy non-solid (cannot be hit)

	// Set a timer to restore the enemy to normal state after m_flPhaseOutDuration
	pEnemy->SetContextThink(&CWeaponPhaseShifter::RestoreEnemy, gpGlobals->curtime + GetPhaseOutDuration(), "RestoreEnemyThink");
}

void CWeaponPhaseShifter::RestoreEnemy()
{
	CBaseEntity* pEnemy = this; // The weapon entity is passed as the context in the think function

	if (!pEnemy || !pEnemy->IsNPC())
		return;

	CBaseCombatCharacter* pNPC = pEnemy->MyCombatCharacterPointer();
	if (!pNPC)
		return;

	// Restore the enemy to normal state
	pNPC->RemoveEffects(EF_NODRAW);
	pNPC->RemoveFlag(FL_NOTARGET);
	pNPC->SetSolid(SOLID_BBOX); // Restore the enemy's collision box

	// Add any other logic for restoring the enemy to a normal state after the phase-out effect
}