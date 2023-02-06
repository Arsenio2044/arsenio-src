//========= Copyright Glitch Software, All rights reserved. ============//
//
// Purpose: Common stuff for npc's related to SHADOW, mainly bosses.
//=============================================================================//

#ifndef ARSENIO_BASESHADOWNPC_H
#define ARSENIO_BASESHADOWNPC_H

#pragma warning(push)
#include <set>
#pragma warning(pop)

#ifdef _WIN32
#pragma once
#endif


#include "ai_basenpc.h"
#include "AI_Motor.h"


class CBaseShadowNPC : public CAI_BaseNPC
{
	DECLARE_CLASS( CBaseShadowNPC, CAI_BaseNPC );
	
public:
	CBaseShadowNPC( void )
	{

	}

	void	TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator );
	bool	ShouldGib( const CTakeDamageInfo &info );
	bool	CorpseGib( const CTakeDamageInfo &info );

	bool	HasAlienGibs( void );
	bool	HasHumanGibs( void );

	void	Precache( void );

	int		IRelationPriority( CBaseEntity *pTarget );
	bool	NoFriendlyFire( void );

	void	EjectShell( const Vector &vecOrigin, const Vector &vecVelocity, float rotation, int iType );

	virtual int		SelectDeadSchedule();
};

#endif //ARSENIO_BASESHADOWNPC_H