#ifndef WEAPON_PHASESHIFTER_H
#define WEAPON_PHASESHIFTER_H

#include "basehlcombatweapon.h"

class CWeaponPhaseShifter : public CBaseHLCombatWeapon
{
public:
	DECLARE_CLASS(CWeaponPhaseShifter, CBaseHLCombatWeapon);
	DECLARE_DATADESC();

	CWeaponPhaseShifter();

	virtual void Precache();
	virtual void PrimaryAttack();
	virtual void ItemPostFrame();

	float GetPhaseOutDuration() const { return m_flPhaseOutDuration; }
	float GetProjectileSpeed() const { return m_flProjectileSpeed; }

private:
	float m_flPhaseOutDuration; // Duration of phase-out effect on enemies
	float m_flProjectileSpeed;  // Speed of the Phase Rounds

	void PhaseOutEnemy(CBaseEntity* pEnemy);
	void RestoreEnemy();
};

#endif // WEAPON_PHASESHIFTER_H
