#include "cbase.h"

//-----------------------------------------------------------------------------
// CWeaponCard
//-----------------------------------------------------------------------------
class CWeaponCard : public CBaseCombatWeapon //CBaseHLCombatWeapon
{
public:
	DECLARE_CLASS( CWeaponCard, CBaseCombatWeapon );

	DECLARE_SERVERCLASS();

	CWeaponCard();
	int	GetId() { return m_nID; }

	//virtual void	PrimaryAttack( void );
	DECLARE_DATADESC();
private:
	int m_nID;
};