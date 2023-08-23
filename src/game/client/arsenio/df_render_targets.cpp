//===== Copyright 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: DownFall Render Targets are specified by and accessable through this singleton
//
// $NoKeywords: $
//===========================================================================//
#include "cbase.h"
#include "df_render_targets.h"
#include "materialsystem/imaterialsystem.h"
#include "rendertexture.h"
#include "tier0/memdbgon.h"


ITexture* CDFRenderTargets::InitViewMask( IMaterialSystem* pMaterialSystem, const char *szName )
{
	/*return materials->CreateNamedRenderTargetTextureEx( szName,
		128, 128,
		RT_SIZE_HDR,
		pMaterialSystem->GetBackBufferFormat(),
		MATERIAL_RT_DEPTH_NONE,
		TEXTUREFLAGS_NOMIP | TEXTUREFLAGS_NOLOD | TEXTUREFLAGS_RENDERTARGET | TEXTUREFLAGS_NODEPTHBUFFER | TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT, 0 );*/

	return pMaterialSystem->CreateNamedRenderTargetTextureEx2(
		szName,
		128, 128, RT_SIZE_HDR,
		pMaterialSystem->GetBackBufferFormat(),
		MATERIAL_RT_DEPTH_SHARED,
		TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT,
		CREATERENDERTARGETFLAGS_HDR);
}

//-----------------------------------------------------------------------------
// Purpose: InitClientRenderTargets, interface called by the engine at material system init in the engine
// Input  : pMaterialSystem - the interface to the material system from the engine (our singleton hasn't been set up yet)
//			pHardwareConfig - the user's hardware config, useful for conditional render targets setup
//-----------------------------------------------------------------------------
void CDFRenderTargets::InitClientRenderTargets( IMaterialSystem* pMaterialSystem, IMaterialSystemHardwareConfig* pHardwareConfig )
{
	// Water effects & camera from the base class (standard HL2 targets)
	BaseClass::InitClientRenderTargets( pMaterialSystem, pHardwareConfig, 1024, 256 );

	m_ViewMask[0].Init( InitViewMask( pMaterialSystem, "_rt_SkyMask" ) );
	m_ViewMask[1].Init( InitViewMask( pMaterialSystem, "_rt_ViewmodelMask" ) );	

}

//-----------------------------------------------------------------------------
// Purpose: Shutdown client render targets. This gets called during shutdown in the engine
// Input  :  - 
//-----------------------------------------------------------------------------
void CDFRenderTargets::ShutdownClientRenderTargets()
{

	m_ViewMask[0].Shutdown();
	m_ViewMask[1].Shutdown();


	BaseClass::ShutdownClientRenderTargets();

}

//Tony; add the interface!
static CDFRenderTargets g_DFRenderTargets;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CDFRenderTargets, IClientRenderTargets, CLIENTRENDERTARGETS_INTERFACE_VERSION, g_DFRenderTargets );
CDFRenderTargets* g_pDFRenderTargets = &g_DFRenderTargets;
#if defined( _LIB )
EXPOSE_STATIC_INTERFACE( CDFRenderTargets, IClientRenderTargets );
#endif

