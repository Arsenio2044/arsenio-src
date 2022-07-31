//========= Copyright Glitch Software, All rights reserved. ============//
//
// Purpose:An M4A1
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

class CWeaponM4A1 : public CHLSelectFireMachineGun
{
public:
	DECLARE_CLASS( CWeaponM4A1, CHLSelectFireMachineGun );

	CWeaponM4A1();

	DECLARE_SERVERCLASS();

	const Vector	&GetBulletSpread( void );

	void			Precache( void );
	void			AddViewKick( void );
	void			Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );

	float			GetFireRate( void ) { return 0.1f; }
	int				CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	DECLARE_ACTTABLE();
};


IMPLEMENT_SERVERCLASS_ST(CWeaponM4A1, DT_WeaponM4A1)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_m4a1, CWeaponM4A1 );
PRECACHE_WEAPON_REGISTER(weapon_m4a1);

acttable_t	CWeaponM4A1::m_acttable[] = 
{
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SMG2, true },
};

IMPLEMENT_ACTTABLE(CWeaponM4A1);

//=========================================================
CWeaponM4A1::CWeaponM4A1( )
{
	m_fMaxRange1		= 820;  // TUX: Should be good enough.
	m_fMinRange1		= 32;

	m_iFireMode			= FIREMODE_3RNDBURST;
}

void CWeaponM4A1::Precache( void )
{
	BaseClass::Precache();  
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : const Vector
//-----------------------------------------------------------------------------
const Vector &CWeaponM4A1::GetBulletSpread( void )
{
	static const Vector cone = VECTOR_CONE_10DEGREES; 
	return cone;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEvent - 
//			*pOperator - 
//-----------------------------------------------------------------------------
void CWeaponM4A1::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
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

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponM4A1::AddViewKick( void )
{
#define	EASY_DAMPEN			2.5f	// Yes.
#define	MAX_VERTICAL_KICK	11.0f	//Degrees - was 1.0
#define	SLIDE_LIMIT			2.0f	//Seconds - was 2.0




	//Get the view kick
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer == NULL)
		return;

	DoMachineGunKick(pPlayer, EASY_DAMPEN, MAX_VERTICAL_KICK, m_fFireDuration, SLIDE_LIMIT);
}

