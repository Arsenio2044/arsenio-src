#include "cbase.h"
#include "func_buyzone.h"
//#include "saverestore_utlvector.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_DATADESC(CBuyZone)

DEFINE_FIELD(TeamNum, FIELD_INTEGER),

//DEFINE_UTLVECTOR(s_BuyZone, FIELD_EMBEDDED),

// Function Pointers
#ifdef lol
DEFINE_FUNCTION(BuyZoneTouch),
DEFINE_THINKFUNC(BuyZoneThink),
#endif

END_DATADESC()

LINK_ENTITY_TO_CLASS(func_buyzone, CBuyZone);

CUtlVector< CBuyZone * >	CBuyZone::s_BuyZone;
CBuyZone::CBuyZone(void)
{
	TeamNum = 0;
	s_BuyZone.AddToTail(this);
}

CBuyZone::~CBuyZone()
{
	s_BuyZone.FindAndRemove(this);
	//UTIL_Remove(this);
}

int CBuyZone::GetBuyZoneCount()
{
	return s_BuyZone.Count();
}

CBuyZone *CBuyZone::GetBuyZone(int index)
{
	if (index < 0 || index >= s_BuyZone.Count())
		return NULL;

	return s_BuyZone[index];
}


void CBuyZone::Spawn()
{
	BaseClass::Spawn();

	// Entity is symbolid
	SetSolid(SOLID_NONE);
	SetMoveType(MOVETYPE_NONE);
	SetCollisionGroup(COLLISION_GROUP_NONE);

	//AddFlag( FL_WORLDBRUSH );
	SetModelName(NULL_STRING);

	// Make entity invisible
	AddEffects(EF_NODRAW);
	// No model but should still network
	AddEFlags(EFL_FORCE_CHECK_TRANSMIT);
#ifdef lol
	SetTouch(&CBuyZone::BuyZoneTouch);
	SetThink(&CBuyZone::BuyZoneThink);
	SetNextThink(gpGlobals->curtime + 0.1f);
#endif
}

#ifdef lol
void CBuyZone::BuyZoneTouch(CBaseEntity *pOther)
{
	CBaseEntity *pPlayer = UTIL_GetNearestPlayer(GetAbsOrigin());
	if (pOther == pPlayer)
		isplayerinbuyzone = true;
}

void CBuyZone::BuyZoneThink()
{
	if (isplayerinbuyzone == true)
	{
		CBaseEntity *pPlayer = UTIL_GetNearestPlayer(GetAbsOrigin());
		float flDist = this->GetAbsOrigin().DistTo(pPlayer->GetAbsOrigin());
		if (flDist > 60)
			isplayerinbuyzone = false;
	}
	SetNextThink(gpGlobals->curtime + 0.1f);
}
#endif