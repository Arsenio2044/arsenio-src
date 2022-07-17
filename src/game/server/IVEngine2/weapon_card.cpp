//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Card - an old favorite
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "weapon_card.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


IMPLEMENT_SERVERCLASS_ST(CWeaponCard, DT_WeaponCard)
END_SEND_TABLE()

#ifndef HL2MP
LINK_ENTITY_TO_CLASS( weapon_card, CWeaponCard );
PRECACHE_WEAPON_REGISTER( weapon_card );
#endif

BEGIN_DATADESC( CWeaponCard )
	DEFINE_KEYFIELD( m_nID, FIELD_INTEGER, "ID" ),
END_DATADESC()


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CWeaponCard::CWeaponCard( void )
{
}


/*void CWeaponCard::PrimaryAttack( void )
{

}*/