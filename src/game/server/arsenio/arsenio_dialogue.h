//========= Copyright Glitch Software, All rights reserved. ============//
//
// Purpose: Dialogue Scene Handler. Sends its params to the client for reading. (event)
// 
//=============================================================================//

#include "cbase.h"
#include "baseentity.h"
#include "gamerules.h"

class CDialogueManager : public CLogicalEntity
{
	DECLARE_CLASS(CDialogueManager, CLogicalEntity);
	DECLARE_DATADESC();

public:

	CDialogueManager();
	void Spawn();
	bool m_bOptionAvailable[3];

private:

	string_t szDialogueScene;
	void InputStartDialogue(inputdata_t &inputData);
	COutputEvent OnStartDialogue;
};