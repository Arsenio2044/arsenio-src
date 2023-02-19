//========= Copyright Glitch Software, All rights reserved. ============//
//
// Purpose: Dialogue Scene Handler. Sends its params to the client for reading. (event)
// 
//=============================================================================//

#include "cbase.h"
#include "baseentity.h"
#include "gamerules.h"
#include "arsenio_dialogue.h"

CDialogueManager::CDialogueManager()
{
	for (int i = 0; i <= 2; i++)
	{
		m_bOptionAvailable[i] = true;
	}

	szDialogueScene = NULL_STRING;
}

void CDialogueManager::Spawn()
{
	BaseClass::Spawn();
}

void CDialogueManager::InputStartDialogue(inputdata_t &inputData)
{
	// If our options have been disabled we don't care anymore:
	if (!m_bOptionAvailable[0] && !m_bOptionAvailable[1] && !m_bOptionAvailable[2])
		return;

	OnStartDialogue.FireOutput(this, this);

	// Run Event here:
	g_pGameRules->StartDialogueScene(szDialogueScene.ToCStr(), GetEntityName().ToCStr(), m_bOptionAvailable[0], m_bOptionAvailable[1], m_bOptionAvailable[2]);
}

BEGIN_DATADESC(CDialogueManager)
DEFINE_FIELD(m_bOptionAvailable[0], FIELD_BOOLEAN),
DEFINE_FIELD(m_bOptionAvailable[1], FIELD_BOOLEAN),
DEFINE_FIELD(m_bOptionAvailable[2], FIELD_BOOLEAN),
DEFINE_KEYFIELD(szDialogueScene, FIELD_STRING, "Script"),
DEFINE_OUTPUT(OnStartDialogue, "OnStartedScene"),
DEFINE_INPUTFUNC(FIELD_VOID, "StartDialogue", InputStartDialogue),
END_DATADESC()

LINK_ENTITY_TO_CLASS(arsenio_dialogue_manager, CDialogueManager)