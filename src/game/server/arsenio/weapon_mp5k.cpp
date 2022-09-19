//========= Copyright Glitch Software, All rights reserved. ============//
//
// Purpose: glitchys favorite weapon that isn't improved.
//
//=============================================================================//

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

ConVar    arsenio_mp5k_accuracy("arsenio_mp5k_accuracy", "1");

#define    MP5K_ACCURACY_MAXIMUM_PENALTY_TIME    3.5f    // Maximum penalty to deal out

class CWeaponMP5K : public CHLSelectFireMachineGun
{
public:
	DECLARE_CLASS( CWeaponMP5K, CHLSelectFireMachineGun );

	CWeaponMP5K();

	DECLARE_SERVERCLASS();

    virtual const Vector& GetBulletSpread(void)
    {
        // Handle NPCs first
        static Vector npcCone = VECTOR_CONE_5DEGREES;
        if (GetOwner() && GetOwner()->IsNPC())
            return npcCone;

        static Vector cone;

        /*
        if ( pistol_use_new_accuracy.GetBool() )
        {
            float ramp = RemapValClamped(    m_flAccuracyPenalty,
                                            0.0f,
                                            PISTOL_ACCURACY_MAXIMUM_PENALTY_TIME,
                                            0.0f,
                                            1.0f );

            // We lerp from very accurate to inaccurate over time
            VectorLerp( VECTOR_CONE_1DEGREES, VECTOR_CONE_6DEGREES, ramp, cone );
        }
        else
        {
            // Old value
            cone = VECTOR_CONE_4DEGREES;
        }
        */

        if (m_nNumShotsFired >= 0 && m_nNumShotsFired < 3)
        {
            cone = VECTOR_CONE_1DEGREES;
        }
        else if (m_nNumShotsFired >= 3 && m_nNumShotsFired < 5)
        {
            cone = VECTOR_CONE_3DEGREES;
        }
        else if (m_nNumShotsFired >= 5 && m_nNumShotsFired < 7)
        {
            cone = VECTOR_CONE_5DEGREES;
        }
        else if (m_nNumShotsFired >= 7 && m_nNumShotsFired < 10)
        {
            cone = VECTOR_CONE_10DEGREES;
        }
        else if (m_nNumShotsFired >= 10)
        {
            cone = VECTOR_CONE_20DEGREES;
        }

        return cone;
    }
	void			Precache( void );
	void			AddViewKick( void );
	void			Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );

	float			GetFireRate( void ) { return 0.1f; }
	int				CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

private:

	float	m_flAccuracyPenalty;
    int		m_nNumShotsFired;



	DECLARE_ACTTABLE();
};

// TUX: Change server classes.
IMPLEMENT_SERVERCLASS_ST(CWeaponMP5K, DT_WeaponMP5K)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_mp5k, CWeaponMP5K );
PRECACHE_WEAPON_REGISTER(weapon_mp5k);

acttable_t	CWeaponMP5K::m_acttable[] = 
{
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SMG2, true },
};

IMPLEMENT_ACTTABLE(CWeaponMP5K);

//=========================================================
CWeaponMP5K::CWeaponMP5K( )
{
	m_fMaxRange1		= 2000;  // TUX: Update firerates.
	m_fMinRange1		= 32;

	m_iFireMode			= FIREMODE_3RNDBURST;
}

void CWeaponMP5K::Precache( void )
{
	BaseClass::Precache();  // TUX: We don't got shit to precache
}



//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEvent - 
//			*pOperator - 
//-----------------------------------------------------------------------------
void CWeaponMP5K::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
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
void CWeaponMP5K::AddViewKick( void )
{
	#define	EASY_DAMPEN			0.5f
	#define	MAX_VERTICAL_KICK	2.0f	//Degrees
	#define	SLIDE_LIMIT			1.0f	//Seconds
	
	//Get the view kick
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if (!pPlayer)
		return;

	DoMachineGunKick( pPlayer, EASY_DAMPEN, MAX_VERTICAL_KICK, m_fFireDuration, SLIDE_LIMIT );
}
