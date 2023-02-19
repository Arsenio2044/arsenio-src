//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Boss Engager - Starts boss fights. Sends a message to the hud_bossbar for reading the name of the npc and his health.
//
//=============================================================================//

#include "cbase.h"
#include "usermessages.h"
#include "baseentity.h"
#include "player.h"
#include "gamerules.h"
#include "util.h"
#include "GameEventListener.h"

// Get class deffs
#include "npc_overlord.h"


class CBossEngager : public CLogicalEntity, public CGameEventListener
{
	DECLARE_CLASS( CBossEngager, CLogicalEntity );
	DECLARE_DATADESC();

public:

	CBossEngager();
	void Spawn();
	void OnThink();
	void InputStart( inputdata_t &input );

	COutputEvent OnStartedBossFight;

private:
	string_t szTargetEnt;
	CBaseEntity *pTargetEnt;

protected:

	void FireGameEvent( IGameEvent *event );
};

CBossEngager::CBossEngager()
{
	ListenForGameEvent( "entity_killed" );
	pTargetEnt = NULL;
}

BEGIN_DATADESC( CBossEngager )

	DEFINE_THINKFUNC( OnThink ),

	DEFINE_KEYFIELD( szTargetEnt, FIELD_STRING, "target" ),
	DEFINE_OUTPUT( OnStartedBossFight, "OnStart" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "StartFight", InputStart ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( tfo_boss_engager, CBossEngager );

void CBossEngager::Spawn()
{
	BaseClass::Spawn();
}

void CBossEngager::OnThink()
{
	pTargetEnt = gEntList.FindEntityByName( NULL, szTargetEnt.ToCStr() ); // Make sure we still can find it...
	if ( pTargetEnt )
	{
		CNPC_OverLord *pOverLord = dynamic_cast< CNPC_OverLord * > ( pTargetEnt );
		char szMessageName[64];
		int iMaxHealth = 0;
		int m_iCurrHealth = 0;

		if ( pOverLord )
		{
			Q_strncpy( szMessageName, pOverLord->GetEntName(), 64 );
			iMaxHealth = pOverLord->GetMaxHP();
			m_iCurrHealth = pOverLord->GetHealth();
		}


		CRecipientFilter user;
		user.AddAllPlayers();
		user.MakeReliable();
		UserMessageBegin( user, "BossData" );
		WRITE_BYTE( 1 );
		WRITE_FLOAT( m_iCurrHealth );
		WRITE_FLOAT( iMaxHealth );
		WRITE_STRING( szMessageName );
		MessageEnd();
	}
	else
	{
		SetThink( NULL );
		pTargetEnt = NULL;

		// Reset Data
		CRecipientFilter user;
		user.AddAllPlayers();
		user.MakeReliable();
		UserMessageBegin( user, "BossData" );
		WRITE_BYTE( 0 );
		WRITE_FLOAT( 0 );
		WRITE_FLOAT( 1 );
		WRITE_STRING( "" );
		MessageEnd();

		return;
	}

	SetNextThink( gpGlobals->curtime + 0.01 );
}

void CBossEngager::InputStart( inputdata_t &input )
{
	pTargetEnt = gEntList.FindEntityByName( NULL, szTargetEnt.ToCStr() );
	if ( pTargetEnt )
	{
		SetThink( &CBossEngager::OnThink );
		SetNextThink( gpGlobals->curtime + 0.01 );
	}
	else
	{
		pTargetEnt = NULL;
		SetThink( NULL );
	}
}

void CBossEngager::FireGameEvent( IGameEvent *event )
{
	const char *type = event->GetName();

	if( !strcmp( type, "entity_killed" )  )
	{
		CBaseEntity *pVictim = UTIL_EntityByIndex( event->GetInt( "entindex_killed", 0 ) );

		if ( pTargetEnt )
		{
			if ( pTargetEnt->entindex() == pVictim->entindex() )
			{
				SetThink( NULL );
				pTargetEnt = NULL;

				// Reset Data
				CRecipientFilter user;
				user.AddAllPlayers();
				user.MakeReliable();
				UserMessageBegin( user, "BossData" );
				WRITE_BYTE( 0 );
				WRITE_FLOAT( 0 );
				WRITE_FLOAT( 1 );
				WRITE_STRING( "" );
				MessageEnd();
			}
		}
	}
}