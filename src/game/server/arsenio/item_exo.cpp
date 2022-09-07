//========= Copyright Glitch Software, All rights reserved. ============//
//
// Purpose: Enables parkour.
//
//=============================================================================//

#include "cbase.h"
#include "player.h"
#include "gamerules.h"
#include "items.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define SF_SUIT_SHORTLOGON		0x0001

class CItemExo : public CItem
{
public:
	DECLARE_CLASS( CItemExo, CItem );

	void Spawn( void )
	{ 
		Precache( );
		SetModel( "models/items/hevsuit.mdl" );
		BaseClass::Spawn( );
		
		CollisionProp()->UseTriggerBounds( false, 0 );
	}
	void Precache( void )
	{
		PrecacheModel ("models/items/hevsuit.mdl");
	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
		if ( pPlayer->IsExoEquipped() )
			return FALSE;

		if ( m_spawnflags & SF_SUIT_SHORTLOGON )
			UTIL_EmitSoundSuit(pPlayer->edict(), "!HEV_A0");		// short version of suit logon,
		else
			UTIL_EmitSoundSuit(pPlayer->edict(), "!HEV_AAx");	// long version of suit logon

		pPlayer->EquipExo();
				
		return true;
	}
};

LINK_ENTITY_TO_CLASS(item_exo, CItemExo);
