//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: When you take damage or you damage someone and blood splats on 
//          you then you'll draw that blood on your hands/weapon. (overlay) Thanks to Bernt Andreas Eide for this.
//
//=============================================================================//

#include "cbase.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/imaterialproxy.h"
#include "baseviewmodel_shared.h"
#include "c_baseplayer.h"
#include "toolframework_client.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class C_BloodyTextureProxy : public IMaterialProxy
{
public:
	C_BloodyTextureProxy();
	virtual ~C_BloodyTextureProxy();

	virtual bool Init( IMaterial *pMaterial, KeyValues *pKeyValues );
	C_BaseEntity *BindArgToEntity( void *pArg );
	virtual void OnBind( void *pC_BaseEntity );
	virtual void Release() { delete this; }
	IMaterial *GetMaterial();

private:
	IMaterialVar *m_pBlendFactor;
};

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
C_BloodyTextureProxy::C_BloodyTextureProxy()
{
	m_pBlendFactor = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
C_BloodyTextureProxy::~C_BloodyTextureProxy()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool C_BloodyTextureProxy::Init( IMaterial *pMaterial, KeyValues *pKeyValues )
{
	bool found;

	m_pBlendFactor = pMaterial->FindVar( "$detailblendfactor", &found, false );
	if ( !found )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_BaseEntity *C_BloodyTextureProxy::BindArgToEntity( void *pArg )
{
	IClientRenderable *pRend = (IClientRenderable *)pArg;
	return pRend ? pRend->GetIClientUnknown()->GetBaseEntity() : NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_BloodyTextureProxy::OnBind( void *pC_BaseEntity )
{
	if ( !pC_BaseEntity )
		return;

	C_BaseEntity *pEntity = BindArgToEntity( pC_BaseEntity );
	C_BaseViewModel *pViewModel = dynamic_cast<C_BaseViewModel *>( pEntity );
	if ( pViewModel )
	{
		C_BasePlayer *pOwner = ToBasePlayer( pViewModel->GetOwner() );
		if ( pOwner )
			m_pBlendFactor->SetFloatValue( pOwner->m_bShouldDrawBloodOverlay ? 1.0f : 0.0f );
	}

	if ( ToolsEnabled() )
		ToolFramework_RecordMaterialParams( GetMaterial() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
IMaterial *C_BloodyTextureProxy::GetMaterial()
{
	return m_pBlendFactor->GetOwningMaterial();
}

EXPOSE_INTERFACE( C_BloodyTextureProxy, IMaterialProxy, "BloodyTexture" IMATERIAL_PROXY_INTERFACE_VERSION );