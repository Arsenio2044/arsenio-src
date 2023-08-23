//========= Copyright Glitch Software, All rights reserved. ============//
//
// Purpose: Implements a "fear" system into ARSENIO 2044. (THIS IS INCOMPLETE)
// 
//=============================================================================//

#include "cbase.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CLogicFear : public CBaseEntity
{
	DECLARE_CLASS(CLogicFear, CBaseEntity);
public:
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	virtual void Spawn(void);

	// Inputs
	void InputFearNone(inputdata_t& inputdata);
	void InputFearLittle(inputdata_t& inputdata);
	void InputFearMedium(inputdata_t& inputdata);
	void InputFearDecent(inputdata_t& inputdata);
	void InputFearHigh(inputdata_t& inputdata);
	void InputFearScary(inputdata_t& inputdata);


};

BEGIN_DATADESC(CLogicFear)


DEFINE_INPUTFUNC(FIELD_VOID, "SetFearToNone", InputFearNone),
DEFINE_INPUTFUNC(FIELD_VOID, "SetFearToLittle", InputFearLittle),
DEFINE_INPUTFUNC(FIELD_VOID, "SetFearToMedium", InputFearMedium),
DEFINE_INPUTFUNC(FIELD_VOID, "SetFearToDecent", InputFearDecent),
DEFINE_INPUTFUNC(FIELD_VOID, "SetFearToHigh", InputFearHigh),
DEFINE_INPUTFUNC(FIELD_VOID, "SetFearToScary", InputFearScary),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CLogicFear, DT_LogicFear)

END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(logic_fear, CLogicFear);

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CLogicFear::Spawn()
{
	BaseClass::Spawn();


}





//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CLogicFear::InputFearNone(inputdata_t& inputdata)
{
	engine->ClientCommand(edict(), "arsenio_fear_level_none");

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CLogicFear::InputFearLittle(inputdata_t& inputdata)
{
	engine->ClientCommand(edict(), "arsenio_fear_level_little");

}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CLogicFear::InputFearMedium(inputdata_t& inputdata)
{
	engine->ClientCommand(edict(), "arsenio_fear_level_medium");

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CLogicFear::InputFearDecent(inputdata_t& inputdata)
{
	engine->ClientCommand(edict(), "arsenio_fear_level_decent");

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CLogicFear::InputFearHigh(inputdata_t& inputdata)
{
	engine->ClientCommand(edict(), "arsenio_fear_level_high");

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CLogicFear::InputFearScary(inputdata_t& inputdata)
{
	engine->ClientCommand(edict(), "arsenio_fear_level_scary");

}