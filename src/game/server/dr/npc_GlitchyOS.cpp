//========= Copyright Glitch Software, All rights reserved. ============//
//
// Purpose: GlitchyOS is an AI that was trapped for 1000 years, before finally being freed.
// It is similar to LeOS but can inject adrenaline into the player, increasing speed and damage for a certain amount of time.
//
//=============================================================================//

class CGlitchyOS : public CItem
{
public:
	DECLARE_CLASS(CGlitchyOS, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel("models/items/hevsuit.mdl");
		BaseClass::Spawn();

		CollisionProp()->UseTriggerBounds(false, 0);
	}
	void Precache(void)
	{
		PrecacheModel("models/items/hevsuit.mdl");
	}
	bool MyTouch(CBasePlayer* pPlayer)
	{
		if (pPlayer->IsLeOSActive())
			return FALSE;



		pPlayer->ActivateLeOS();

		return true;
	}
};

LINK_ENTITY_TO_CLASS(item_glitchyOS, CGlitchyOS);


class CNPC_GlitchyOS : public CAI_BaseActor
{
public:
	DECLARE_CLASS(CNPC_GlitchyOS, CAI_BaseActor);

	void	Spawn(void);
	void	Precache(void);
	Class_T Classify(void);
	void	HandleAnimEvent(animevent_t* pEvent);
	int		GetSoundInterests(void);
	void	SetupWithoutParent(void);
	void	PrescheduleThink(void);
};

LINK_ENTITY_TO_CLASS(npc_GlitchyOS, CNPC_GlitchyOS);

//-----------------------------------------------------------------------------
// Classify - indicates this NPC's place in the 
// relationship table.
//-----------------------------------------------------------------------------
Class_T	CNPC_GlitchyOS::Classify(void)
{
	return	CLASS_PLAYER;
}



//-----------------------------------------------------------------------------
// HandleAnimEvent - catches the NPC-specific messages
// that occur when tagged animation frames are played.
//-----------------------------------------------------------------------------
void CNPC_GlitchyOS::HandleAnimEvent(animevent_t* pEvent)
{
	switch (pEvent->event)
	{
	case 1:
	default:
		BaseClass::HandleAnimEvent(pEvent);
		break;
	}
}

//-----------------------------------------------------------------------------
// GetSoundInterests - generic NPC can't hear.
//-----------------------------------------------------------------------------
int CNPC_GlitchyOS::GetSoundInterests(void)
{
	return	NULL;
}

//-----------------------------------------------------------------------------
// LeOS does nothing without a scripted sequence when spawned as an NPC. 
//-----------------------------------------------------------------------------
void CNPC_GlitchyOS::Spawn()
{
	// LeOS is allowed to use multiple models, because he appears in the pod.
	// He defaults to his normal model.
	char* szModel = (char*)STRING(GetModelName());
	if (!szModel || !*szModel)
	{
		szModel = "models/LeOS.mdl";
		//szModel = "models/CyberJail.mdl"; // later
		SetModelName(AllocPooledString(szModel));
	}

	Precache();
	SetModel(szModel);

	BaseClass::Spawn();

	SetHullType(HULL_HUMAN);
	SetHullSizeNormal();

	// If LeOS has a parent, he's currently inside a pod. Prevent him from moving.
	if (GetMoveParent())
	{
		SetSolid(SOLID_BBOX);
		AddSolidFlags(FSOLID_NOT_STANDABLE);
		SetMoveType(MOVETYPE_NONE);

		CapabilitiesAdd(bits_CAP_ANIMATEDFACE | bits_CAP_TURN_HEAD);
		CapabilitiesAdd(bits_CAP_FRIENDLY_DMG_IMMUNE);
	}
	else
	{
		SetupWithoutParent();
	}

	AddEFlags(EFL_NO_DISSOLVE | EFL_NO_MEGAPHYSCANNON_RAGDOLL | EFL_NO_PHYSCANNON_INTERACTION);
	SetBloodColor(BLOOD_COLOR_RED);
	m_iHealth = 8;
	m_flFieldOfView = 0.5;// indicates the width of this NPC's forward view cone ( as a dotproduct result )
	m_NPCState = NPC_STATE_NONE;

	NPCInit();
}

//-----------------------------------------------------------------------------
// Precache - precaches all resources this NPC needs
//-----------------------------------------------------------------------------
void CNPC_GlitchyOS::Precache()
{
	PrecacheModel(STRING(GetModelName()));
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_GlitchyOS::SetupWithoutParent(void)
{
	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetMoveType(MOVETYPE_STEP);

	CapabilitiesAdd(bits_CAP_MOVE_GROUND | bits_CAP_OPEN_DOORS | bits_CAP_ANIMATEDFACE | bits_CAP_TURN_HEAD);
	CapabilitiesAdd(bits_CAP_FRIENDLY_DMG_IMMUNE);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_GlitchyOS::PrescheduleThink(void)
{
	BaseClass::PrescheduleThink();

	// Figure out if LeOS has just been removed from his parent
	if (GetMoveType() == MOVETYPE_NONE && !GetMoveParent())
	{
		SetupWithoutParent();
		SetupVPhysicsHull();
	}
}

//-----------------------------------------------------------------------------
// AI Schedules Specific to this NPC
//-----------------------------------------------------------------------------

#endif
