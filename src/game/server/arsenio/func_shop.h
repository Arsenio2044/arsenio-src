#ifndef	BUYZONE_H
#define	BUYZONE_H

#ifdef _WIN32
#pragma once
#endif

class CShop : public CBaseEntity
{
public:
	DECLARE_CLASS(CShop, CBaseEntity);

	int	TeamNum;
	void	ShopTouch(CBaseEntity* pOther);
	bool	isinshop;

	CShop();

	DECLARE_DATADESC();
};

#endif