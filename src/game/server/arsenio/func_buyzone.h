#ifndef	BUYZONE_H
#define	BUYZONE_H

#ifdef _WIN32
#pragma once
#endif

class CBuyZone : public CBaseEntity
{
public:
	DECLARE_CLASS(CBuyZone, CBaseEntity);
	DECLARE_DATADESC();

	CBuyZone();
	~CBuyZone();

	virtual void Spawn();

	int	TeamNum;
	CBuyZone* GetBuyZone(int index);
	virtual CBuyZone *MyBuyZonePointer( void ) { return this; }
	static CUtlVector<CBuyZone *>	s_BuyZone;
	int GetBuyZoneCount();
#ifdef lol
	void	BuyZoneTouch(CBaseEntity *pOther);
	bool	isplayerinbuyzone;
	/*virtual */void	BuyZoneThink(void);
#endif
};
#endif