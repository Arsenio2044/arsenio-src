//========= Copyright Glitch Software, All rights reserved. ============//
//
// Purpose: LeOS is a prototype AI designed by GreyBox Labs to aid the Combine during and outside of combat. 
// LeOS aids Arsenio during his journey by providing details on enemies and managing his body.
//
//=============================================================================//

#include "cbase.h"
#include "player.h"
#include "gamerules.h"
#include "items.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define SF_SUIT_SHORTLOGON		0x0001

class CLeOS : public CItem
{
public:
	DECLARE_CLASS(CLeOS, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel("models/items/hevsuit.mdl");
		BaseClass::Spawn();

		CollisionProp()->UseTriggerBounds(false, 0);
	}
	void Precache(void)
	{
		PrecacheModel("models/items/hevsuit.mdl");
	}
	bool MyTouch(CBasePlayer* pPlayer)
	{
		if (pPlayer->IsLeOSActive())
			return FALSE;



		pPlayer->ActivateLeOS();

		return true;
	}
};

LINK_ENTITY_TO_CLASS(npc_LeOS, CLeOS);
