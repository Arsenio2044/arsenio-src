//========= Copyright Glitch Software, All rights reserved. ============//
//
// Purpose: Yet another SMG!
//
//=============================================================================//

			//	INCLUDES
// ==============================================================
#include "cbase.h"
#include "basehlcombatweapon.h"
#include "NPCevent.h"
#include "basecombatcharacter.h"
#include "ai_basenpc.h"
#include "player.h"
#include "game.h"
#include "in_buttons.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// ==============================================================

class CWeaponMP5 : public CHLSelectFireMachineGun
{
public:
	DECLARE_CLASS( CWeaponMP5, CHLSelectFireMachineGun );

	CWeaponMP5();

	DECLARE_SERVERCLASS();

	const Vector	&GetBulletSpread( void );

	void			Precache( void );
	void			AddViewKick( void );
	void			Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );

	float			GetFireRate( void ) { return 0.1f; }
	int				CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	DECLARE_ACTTABLE();
};


IMPLEMENT_SERVERCLASS_ST(CWeaponMP5, DT_WeaponMP5)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_mp5, CWeaponMP5 );
PRECACHE_WEAPON_REGISTER(weapon_mp5);

acttable_t	CWeaponMP5::m_acttable[] = 
{
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SMG2, true },
};

IMPLEMENT_ACTTABLE(CWeaponMP5);

//=========================================================
CWeaponMP5::CWeaponMP5( )
{
	m_fMaxRange1		= 520;  
	m_fMinRange1		= 32;

	m_iFireMode			= FIREMODE_3RNDBURST;
}

void CWeaponMP5::Precache( void )
{
	BaseClass::Precache();  
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : const Vector
//-----------------------------------------------------------------------------
const Vector &CWeaponMP5::GetBulletSpread( void )
{
	static const Vector cone = VECTOR_CONE_12DEGREES; 
	return cone;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEvent - 
//			*pOperator - 
//-----------------------------------------------------------------------------
void CWeaponMP5::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	switch( pEvent->event )
	{
		case EVENT_WEAPON_SMG2:
		{
			Vector vecShootOrigin, vecShootDir;
			vecShootOrigin = pOperator->Weapon_ShootPosition( );

			CAI_BaseNPC *npc = pOperator->MyNPCPointer();
			ASSERT( npc != NULL );
			vecShootDir = npc->GetActualShootTrajectory( vecShootOrigin );

			WeaponSound(SINGLE_NPC);
			pOperator->FireBullets( 1, vecShootOrigin, vecShootDir, VECTOR_CONE_PRECALCULATED, MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2 );
			pOperator->DoMuzzleFlash();
			m_iClip1 = m_iClip1 - 1;
		}
		break;
		default:
			BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
			break;
	}
}

