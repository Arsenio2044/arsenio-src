//========= Copyright Glitch Software, All rights reserved. ============//
//
// Purpose: Handles Film Grain Strength.
//
//=============================================================================//

#include "cbase.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/imaterialproxy.h"
#include "toolframework_client.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class C_FilmGrainProxy : public IMaterialProxy
{
public:

	C_FilmGrainProxy();
	virtual ~C_FilmGrainProxy();

	virtual bool Init(IMaterial *pMaterial, KeyValues *pKeyValues);
	C_BaseEntity *BindArgToEntity(void *pArg);
	virtual void OnBind(void *pC_BaseEntity);
	virtual void Release(void) { delete this; }
	IMaterial *GetMaterial(void);

private:

	IMaterialVar* blendFactor;
};

C_FilmGrainProxy::C_FilmGrainProxy()
{
	blendFactor = NULL;
}

C_FilmGrainProxy::~C_FilmGrainProxy()
{
}

bool C_FilmGrainProxy::Init(IMaterial *pMaterial, KeyValues *pKeyValues)
{
	bool found;

	blendFactor = pMaterial->FindVar("$basetexturetransform", &found, false);
	if (!found)
		return false;

	return true;
}

C_BaseEntity *C_FilmGrainProxy::BindArgToEntity(void *pArg)
{ 
	IClientRenderable *pRend = (IClientRenderable *)pArg;
	return pRend ? pRend->GetIClientUnknown()->GetBaseEntity() : NULL;
}

void C_FilmGrainProxy::OnBind(void* pC_BaseEntity)
{
	//ConVarRef filmGrain("opout_fx_filmgrain");
	ConVarRef filmGrainStr("opout_fx_filmgrain_strength");

	//if (!filmGrain.GetBool())
		return;

	// Determine the scale of the film grain here:
	VMatrix mat(filmGrainStr.GetFloat(), 0.0f, 0.0f, 0.0f,
		0.0f, filmGrainStr.GetFloat(), 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	blendFactor->SetMatrixValue(mat);

	if (ToolsEnabled())
		ToolFramework_RecordMaterialParams(GetMaterial());
}

IMaterial *C_FilmGrainProxy::GetMaterial()
{
	return blendFactor->GetOwningMaterial();
}

EXPOSE_INTERFACE(C_FilmGrainProxy, IMaterialProxy, "FilmGrain" IMATERIAL_PROXY_INTERFACE_VERSION);