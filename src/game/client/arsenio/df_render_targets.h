//========= Copyright 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Downfall render targets are specified by and accessable through this singleton
//
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//
#ifndef DFRENDERTARGETS_H_
#define DFRENDERTARGETS_H_
#ifdef _WIN32
#pragma once
#endif

#include "baseclientrendertargets.h" // Base class, with interfaces called by engine and inherited members to init common render targets

// externs
class IMaterialSystem;
class IMaterialSystemHardwareConfig;

class CDFRenderTargets : public CBaseClientRenderTargets
{
	// no networked vars
	DECLARE_CLASS_GAMEROOT( CDFRenderTargets, CBaseClientRenderTargets );
public:
	virtual void InitClientRenderTargets( IMaterialSystem* pMaterialSystem, IMaterialSystemHardwareConfig* pHardwareConfig );
	virtual void ShutdownClientRenderTargets();


private:

	ITexture* InitViewMask( IMaterialSystem* pMaterialSystem, const char *szName  );


private:


	CTextureReference		m_ViewMask[2];

};

extern CDFRenderTargets* g_pDFRenderTargets;


#endif //DFRENDERTARGETS_H_