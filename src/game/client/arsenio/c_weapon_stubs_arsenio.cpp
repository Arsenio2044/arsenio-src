//========= Copyright Glitch Software, All rights reserved. ============//
//
// Purpose: Custom weapon stubs file for ARSENIO.
//
//=============================================================================//

//==================================
//		INCLUDES
//==================================
#include "cbase.h"
#include "c_weapon__stubs.h"
#include "basehlcombatweapon_shared.h"
#include "c_basehlcombatweapon.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"



//==================================
//		WEAPONS
//==================================

STUB_WEAPON_CLASS(cycler_weapon, WeaponCycler, C_BaseCombatWeapon);
STUB_WEAPON_CLASS(weapon_bugbait, WeaponBugBait, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_flaregun, Flaregun, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_annabelle, WeaponAnnabelle, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_cubemap, WeaponCubemap, C_BaseCombatWeapon);
STUB_WEAPON_CLASS(weapon_alyxgun, WeaponAlyxGun, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS(weapon_citizenpackage, WeaponCitizenPackage, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_citizensuitcase, WeaponCitizenSuitcase, C_WeaponCitizenPackage);
STUB_WEAPON_CLASS(weapon_ar2, WeaponAR2, C_HLMachineGun);
STUB_WEAPON_CLASS(weapon_frag, WeaponFrag, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_rpg, WeaponRPG, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_pistol, WeaponPistol, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_shotgun, WeaponShotgun, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_357, Weapon357, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_crossbow, WeaponCrossbow, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_slam, Weapon_SLAM, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_crowbar, WeaponCrowbar, C_BaseHLBludgeonWeapon);
STUB_WEAPON_CLASS(weapon_hopwire, WeaponHopwire, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_mp5k, WeaponMP5K, C_HLSelectFireMachineGun ); 
STUB_WEAPON_CLASS(weapon_pro836, WeaponPRO836, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS(weapon_gauss, WeaponGaussGun, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_addamm557, WeaponAdamm557, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_mobileturret, WeaponMobileTurret, C_BaseHLBludgeonWeapon);
STUB_WEAPON_CLASS(weapon_glock, WeaponGlock, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_k189, WeaponK189, C_HLMachineGun );
STUB_WEAPON_CLASS(weapon_smg1, WeaponSMG1, C_HLSelectFireMachineGun); 

// Coming soon
#ifdef BALLS
STUB_WEAPON_CLASS(weapon_BJ69, WeaponBJ69, C_HLSelectFireMachineGun); // nicknamed sex gun.
STUB_WEAPON_CLASS(weapon_KLO99, WeaponKLO99, C_HLMachineGun); // High power LMG
STUB_WEAPON_CLASS(weapon_MP99K, WeaponMP599K, C_BaseHLCombatWeapon); // Mp5K for modern world.
STUB_WEAPON_CLASS(weapon_overlordgun, WeaponOverlord, C_BaseHLCombatWeapon); // Used for final boss.


#endif





#ifdef ARSENIO_CUT
STUB_WEAPON_CLASS( weapon_egon, WeaponEgon, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS(weapon_smg3, WeaponSMG3, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS(weapon_kilo, WeaponKILO, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS(weapon_50cal, Weapon50cal, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_minigun, WeaponMiniGun, C_HLMachineGun);
STUB_WEAPON_CLASS(weapon_oicw, WeaponOICW, C_HLMachineGun); // yeeted vol 2
STUB_WEAPON_CLASS(weapon_binoculars, WeaponBinoculars, C_BaseHLCombatWeapon); // yeeted vol 4
STUB_WEAPON_CLASS(weapon_bulk, WeaponBULK, C_BaseHLCombatWeapon); // yeeted vol 5
// yeeted

#endif


