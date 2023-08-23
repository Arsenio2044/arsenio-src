#include "cbase.h"
#include "actual_bullet.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar debug_actual_bullet("debug_actual_bullet", "0", FCVAR_GAMEDLL);

LINK_ENTITY_TO_CLASS(actual_bullet, CActualBullet);

BEGIN_DATADESC(CActualBullet)
END_DATADESC()

void CActualBullet::Start(void)
{
	SetThink(&CActualBullet::Think);
	SetNextThink(gpGlobals->curtime);
	SetOwnerEntity(info.m_pAttacker);
}
void CActualBullet::Think(void)
{
	SetNextThink(gpGlobals->curtime + 0.05f);
	Vector vecStart;
	Vector vecEnd;
	float flInterval;

	flInterval = gpGlobals->curtime - GetLastThink();
	vecStart = GetAbsOrigin();
	vecEnd = vecStart + (m_vecDir * (m_Speed * flInterval));
	float flDist = (vecStart - vecEnd).Length();

	trace_t tr;
	UTIL_TraceLine(vecStart, vecEnd, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);
	if (debug_actual_bullet.GetBool() == true)
		DebugDrawLine(vecStart, vecEnd, 0, 128, 255, false, 0.05f);

	if (tr.fraction != 1.0)
	{
		FireBulletsInfo_t info2;
		info2.m_iShots = 1;
		info2.m_vecSrc = vecStart;
		info2.m_vecSpread = vec3_origin;
		info2.m_vecDirShooting = m_vecDir;
		info2.m_flDistance = flDist;
		info2.m_iAmmoType = info.m_iAmmoType;
		info2.m_iTracerFreq = 0;
		GetOwnerEntity()->FireBullets(info2);
		Stop();
	}
	else
	{
		SetAbsOrigin(vecEnd);
	}
}

void CActualBullet::Stop(void)
{
	SetThink(NULL);
	UTIL_Remove(this);
}