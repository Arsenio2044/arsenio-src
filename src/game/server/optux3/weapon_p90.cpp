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

class CWeaponP90 : public CHLMachineGun
{
public:
	DECLARE_CLASS( CWeaponP90, CHLMachineGun );

	CWeaponP90();

	DECLARE_SERVERCLASS();

	const Vector	&GetBulletSpread( void );

	void			Precache( void );
	void			AddViewKick( void );
	void			Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );

	float			GetFireRate( void ) { return 0.1f; }
	int				CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	DECLARE_ACTTABLE();
};


IMPLEMENT_SERVERCLASS_ST(CWeaponP90, DT_WeaponP90)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_p90, CWeaponP90 );
PRECACHE_WEAPON_REGISTER(weapon_p90);

acttable_t	CWeaponP90::m_acttable[] = 
{
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SMG2, true },
};

IMPLEMENT_ACTTABLE(CWeaponP90);

//=========================================================
CWeaponP90::CWeaponP90( )
{
	m_fMaxRange1		= 3000;  
	m_fMinRange1		= 32;


}

void CWeaponP90::Precache( void )
{
	BaseClass::Precache();  
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : const Vector
//-----------------------------------------------------------------------------
const Vector &CWeaponP90::GetBulletSpread( void )
{
	static const Vector cone = VECTOR_CONE_20DEGREES; 
	return cone;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEvent - 
//			*pOperator - 
//-----------------------------------------------------------------------------
void CWeaponP90::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
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

void CWeaponP90::AddViewKick(void)
{
#define	EASY_DAMPEN			2.5f	// Yes.
#define	MAX_VERTICAL_KICK	11.0f	//Degrees - was 1.0
#define	SLIDE_LIMIT			2.0f	//Seconds - was 2.0

	// Credit Breadman for the viewkick, btw E:Z 2 looks really good!


	//Get the view kick
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer == NULL)
		return;

	DoMachineGunKick(pPlayer, EASY_DAMPEN, MAX_VERTICAL_KICK, m_fFireDuration, SLIDE_LIMIT);
}



