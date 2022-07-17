//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "player.h"

//#include "utlfixedlinkedlist.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


//-----------------------------------------------------------------------------
// Link between entities and cameras
//-----------------------------------------------------------------------------
class CInfoSetStress : public CLogicalEntity
{
	DECLARE_CLASS( CInfoSetStress, CLogicalEntity );
 	DECLARE_DATADESC();

public:
	CInfoSetStress();
	~CInfoSetStress();

	void Think();
	virtual void Activate();
	void InputSetStress(inputdata_t &inputdata);
	void					SetStress(int i, float fSpeed = 2.f);


	

private:
	//void Think();
	int m_iStressLevel;
	float m_iTime;	
	int m_iNextTime;

	
	//friend CBaseEntity *CreateInfoSetStress( CBaseEntity *pTarget, CPointCamera *pCamera );
};



//-----------------------------------------------------------------------------
// Save/load
//-----------------------------------------------------------------------------
BEGIN_DATADESC( CInfoSetStress )

	DEFINE_KEYFIELD( m_iStressLevel, FIELD_INTEGER, "Stresslevel" ),
	DEFINE_KEYFIELD( m_iTime, FIELD_FLOAT, "Time" ),

	DEFINE_FUNCTION( Think ),
	// Outputs
	DEFINE_INPUTFUNC( FIELD_STRING, "SetStress", InputSetStress ),

END_DATADESC()


LINK_ENTITY_TO_CLASS( info_setstress, CInfoSetStress );


//-----------------------------------------------------------------------------
// Constructor, destructor
//-----------------------------------------------------------------------------
CInfoSetStress::CInfoSetStress()
{		
}

CInfoSetStress::~CInfoSetStress()
{
}


//-----------------------------------------------------------------------------
// Purpose: Called after all entities have spawned and after a load game.
//-----------------------------------------------------------------------------
void CInfoSetStress::Activate( )
{
	BaseClass::Activate();
}



//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CInfoSetStress::InputSetStress(inputdata_t &inputdata )
{
	DevMsg("Set stress : %d : %d\n",atoi(inputdata.value.String()),m_iStressLevel);

	//CBasePlayer *pPlayer = (CBasePlayer*)gEntList.FindEntityByClassname( NULL, "player" );
	SetStress(m_iStressLevel);

	//SetThink( &CInfoSetStress::Think );
	//m_iNextTime = gpGlobals->curtime;
	//SetNextThink( gpGlobals->curtime );
}

void CInfoSetStress::Think()
{
	//DevMsg("Think Stress\n",NULL,NULL);

	//CBasePlayer *pPlayer = (CBasePlayer*)gEntList.FindEntityByClassname( NULL, "player" );
	SetStress(m_iStressLevel);

	/*
	if ( gpGlobals->curtime - m_iNextTime > m_iTime )
	{
		m_iNextTime = gpGlobals->curtime;
		
		if (pPlayer->GetStress() < m_iStressLevel)
			pPlayer->SetStress(pPlayer->GetStress()+1);
		else
			pPlayer->SetStress(pPlayer->GetStress()-1);

		if (pPlayer->GetStress() == m_iStressLevel)	
			SetThink( NULL );			
		else
			SetNextThink( gpGlobals->curtime );
	}else
		SetNextThink( gpGlobals->curtime );
	*/
}
