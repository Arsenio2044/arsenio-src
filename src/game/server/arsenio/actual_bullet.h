#ifndef ACTUALBULLET_H
#define ACTUALBULLET_H

#include "cbase.h"
#include "baseentity.h"
#include "baseentity_shared.h"
#include "baseanimating.h"


class CActualBullet : public CBaseAnimating
{
	DECLARE_CLASS(CActualBullet, CBaseAnimating);
	DECLARE_DATADESC();

public:
	void Start(void);
	void Think(void);
	void Stop(void);

	Vector m_vecDir;
	int m_Speed;
	FireBulletsInfo_t info;
};

///so this is the actual bullet creation function.
inline void FireActualBullet(FireBulletsInfo_t &info, int iSpeed, const char* tracertype)
{
	if (!info.m_pAttacker)
	{
		Warning("ERROR: Firing an actual bullet without an attacker specified. This will crash the game without it. Cancelling.\n");
		return;
	}
	int iShots = info.m_iShots;

	for (int i = 0; i < iShots; i++)
	{
		Vector vecSpreadSrc = info.m_vecSpread;
		Vector vecSpread = Vector(RandomFloat(-vecSpreadSrc[0], vecSpreadSrc[0]), RandomFloat(-vecSpreadSrc[1], vecSpreadSrc[1]), RandomFloat(-vecSpreadSrc[2], vecSpreadSrc[2]));
		Vector vecShot = info.m_vecDirShooting + vecSpread;
		Vector vecShotDir = vecShot.Normalized();
		trace_t tr;
		UTIL_TraceLine(info.m_vecSrc, info.m_vecSrc + (vecShotDir * MAX_TRACE_LENGTH), MASK_SHOT, info.m_pAttacker, COLLISION_GROUP_NONE, &tr);
		CActualBullet *pBullet = (CActualBullet*)CBaseEntity::Create("actual_bullet", info.m_vecSrc, vec3_angle, info.m_pAttacker);
		pBullet->m_vecDir = vecShotDir;
		pBullet->m_Speed = iSpeed;
		pBullet->SetOwnerEntity(info.m_pAttacker);
		pBullet->SetAbsOrigin(info.m_vecSrc);
		pBullet->info = info;
		pBullet->Start();
		UTIL_Tracer(info.m_vecSrc, tr.endpos, info.m_pAttacker->entindex(), -1, (float)iSpeed, false, tracertype, 0);
	}
}

#endif //ACTUALBULLET_H