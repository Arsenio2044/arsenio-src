//========= Copyright Glitch Software, All rights reserved. ============//
//
// Purpose: The Kilo 141, an SMG that fires AR/LMG rounds and holds 30 rounds in a single magazine.
// Will probably be removed in the end, if so, I leave my mark. Made by glitchy~
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
// #include "MissionChooser\wep\BaseCustomization.h" (not implemented yet)

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// ==============================================================

class CWeaponKILO : public CHLSelectFireMachineGun
{
public:
	DECLARE_CLASS( CWeaponKILO, CHLSelectFireMachineGun );

	CWeaponKILO();

	DECLARE_SERVERCLASS();

	const Vector	&GetBulletSpread( void );

	void			Precache( void );
	void			AddViewKick( void );
	void			Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );

	float			GetFireRate( void ) { return 0.1f; } // GLITCH: do you want me to change the firerate, tux? @tux
	int				CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	DECLARE_ACTTABLE();
};


IMPLEMENT_SERVERCLASS_ST(CWeaponKILO, DT_WeaponKILO)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_kilo, CWeaponKILO );
PRECACHE_WEAPON_REGISTER(weapon_kilo);

acttable_t	CWeaponKILO::m_acttable[] = 
{
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SMG2, true },
};

IMPLEMENT_ACTTABLE(CWeaponKILO);

//=========================================================
CWeaponKILO::CWeaponKILO( )
{
	m_fMaxRange1		= 1200;  // TUX: Me lazy
	m_fMinRange1		= 92;

	m_iFireMode			= FIREMODE_3RNDBURST; // GLITCHY: this probably dont work you bastard
}

void CWeaponKILO::Precache( void )
{
	BaseClass::Precache();  
}

//-----------------------------------------------------------------------------
// Purpose:  Bullet spread (not accuracy!
// Output : const Vector
//-----------------------------------------------------------------------------
const Vector &CWeaponKILO::GetBulletSpread( void )
{
	static const Vector cone = VECTOR_CONE_10DEGREES; // Might change this sometime soon.
	return cone; // LEO EATS SWEDISH CHEESE AND HE FUCKING LOVES IT!!!!!!!
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEvent - 
//			*pOperator - 
//-----------------------------------------------------------------------------
void CWeaponKILO::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
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
void CWeaponKILO::AddViewKick( void )
{
#define	EASY_DAMPEN			2.5f	// Yes.
#define	MAX_VERTICAL_KICK	11.0f	//Degrees - was 1.0
#define	SLIDE_LIMIT			2.0f	//Seconds - was 2.0
#define m_fFireDuration     3.0f    //Tf does this do? I either dont remember or dont know... Or its unused!

	// Credit Breadman for the viewkick, btw E:Z 2 looks really good!


	//Get the view kick
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer == NULL)
		return;

	DoMachineGunKick(pPlayer, EASY_DAMPEN, MAX_VERTICAL_KICK, m_fFireDuration, SLIDE_LIMIT);
}

