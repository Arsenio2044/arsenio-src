#include "cbase.h"
#include "hl2_player.h"
#include "weapon_card.h"

class CWeaponCard;

class CLogicCardReader : public CLogicalEntity
{
	DECLARE_CLASS( CLogicCardReader, CLogicalEntity );

private:

	DECLARE_DATADESC();

	int m_nCardID;

public:
	COutputEvent m_PlayerHasCard;
	COutputEvent m_PlayerHasNoCard;

	void InputRequestCard( inputdata_t &inputdata );
	void Activate ( void );

	EHANDLE m_hPlayer;
};

LINK_ENTITY_TO_CLASS( logic_cardreader, CLogicCardReader);

BEGIN_DATADESC( CLogicCardReader )
	DEFINE_KEYFIELD(m_nCardID, FIELD_INTEGER, "ID"),
	DEFINE_OUTPUT( m_PlayerHasCard, "PlayerHasCard" ),
	DEFINE_OUTPUT( m_PlayerHasNoCard, "PlayerHasNoCard" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "RequestCard",	InputRequestCard ),

	//DEFINE_FIELD( m_hPlayer, FIELD_EHANDLE ),
END_DATADESC()

void CLogicCardReader::InputRequestCard( inputdata_t &inputdata )
{
	if( m_hPlayer == NULL )
	{
		DevMsg("No Player %d\n",m_nCardID);
	}
		
	CHL2_Player *pPlayer = dynamic_cast<CHL2_Player*>(UTIL_GetLocalPlayer());
	for ( int i = 0 ; i < pPlayer->WeaponCount(); ++i )
	{
		CBaseCombatWeapon* weapon = pPlayer->GetWeapon( i );
		if (weapon)
			if (weapon->IsItem())
				if (!strcmp(weapon->GetName(),"weapon_card"))
				{
					//dynamic_cast<CWeaponCard*>
					CWeaponCard *card = (CWeaponCard*)weapon;
					if (card->GetId()==m_nCardID)
					{
						m_PlayerHasCard.FireOutput(this,this);
						DevMsg("Has card %d\n",card->GetId());
						return;
					}
				}
	}
	m_PlayerHasNoCard.FireOutput(this,this);
	DevMsg("Has No card\n");
	//pPlayer->GetWeapon(0);
}

void CLogicCardReader::Activate ( void )
{
	BaseClass::Activate();

	if ( m_hPlayer == NULL )
	{
		m_hPlayer = UTIL_GetLocalPlayer();
	}
}