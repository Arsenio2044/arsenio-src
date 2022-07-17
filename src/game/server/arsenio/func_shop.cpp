#include "cbase.h"
#include "func_shop.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_DATADESC(CShop)

DEFINE_FIELD(TeamNum, FIELD_INTEGER),

// Function Pointers
DEFINE_FUNCTION(ShopTouch),

END_DATADESC()

LINK_ENTITY_TO_CLASS(func_shop, CShop);

CShop::CShop(void)
{
	SetTouch(&CShop::ShopTouch);
}

void CShop::ShopTouch(CBaseEntity* pOther)
{
	CBaseEntity* pPlayer = UTIL_GetNearestPlayer(GetAbsOrigin());
	if (pOther == pPlayer)
	{
		if (TeamNum != 0)
		{
			if (pPlayer->GetTeamNumber() == TeamNum)
				isinshop = true;
		}
		else
			isinshop = true;
	}
}