//========= Copyright Glitch Software, All rights reserved. ============//
//
// Purpose:		Arms for when we dont have any actual weapons to use
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "basecombatweapon_shared.h"
#ifdef GAME_DLL
#include "basecombatweapon.h"
#include "basecombatcharacter.h"
#include "basebludgeonweapon.h"
#endif
#ifdef CLIENT_DLL
#include "c_weapon__stubs.h"
#endif
#include "gamerules.h"
#include "in_buttons.h"
#include "vstdlib/random.h"
#include "gamestats.h"
#include "saverestore.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CWeaponUnarmed : public CBaseHLBludgeonWeapon
{
public:
	DECLARE_CLASS(CWeaponUnarmed, CBaseHLBludgeonWeapon);

	DECLARE_SERVERCLASS();
	DECLARE_ACTTABLE();

	CWeaponUnarmed();

	float		GetRange(void) { return	0; }

	float		GetDamageForActivity(Activity hitActivity) { return 0; }

	void		PrimaryAttack(void) { return; }
	void		SecondaryAttack(void) { return; }
};

IMPLEMENT_SERVERCLASS_ST(CWeaponUnarmed, DT_WeaponUnarmed)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_unarmed, CWeaponUnarmed);
PRECACHE_WEAPON_REGISTER(weapon_unarmed);

acttable_t CWeaponUnarmed::m_acttable[] =
{
	{ ACT_IDLE, ACT_IDLE_RELAXED, false },
	{ ACT_IDLE_ANGRY, ACT_IDLE_RELAXED, false },
};

IMPLEMENT_ACTTABLE(CWeaponUnarmed);

CWeaponUnarmed::CWeaponUnarmed(void)
{

}