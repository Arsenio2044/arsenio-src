#ifndef DF_VIEW_SCENE_H
#define DF_VIEW_SCENE_H
#ifdef _WIN32
#pragma once
#endif

#include "viewrender.h"

//-----------------------------------------------------------------------------
// Purpose: Implements the interview to view rendering for the client .dll
//-----------------------------------------------------------------------------
class CDFViewRender : public CViewRender
{
public:
	DECLARE_CLASS(CDFViewRender, CViewRender);
	CDFViewRender();

	virtual void OnRenderStart();
	virtual void Render2DEffectsPreHUD( const CViewSetup &view );
	virtual void Render2DEffectsPostHUD( const CViewSetup &view );

	virtual bool	AllowScreenspaceFade( void ) { return false; }
	virtual void	LevelInit();

	virtual void DoCustomPostProcessing( const CViewSetup &view );

private:
	
};

void DF_SetMaterialVarFloat( IMaterial* pMat, const char* pVarName, float flValue );
void DF_SetMaterialVarInt( IMaterial* pMat, const char* pVarName, int iValue );
void DF_SetMaterialVarString( IMaterial* pMat, const char* pVarName, const char *pValue );
void DF_SetMaterialVarTexture( IMaterial* pMat, const char* pVarName, ITexture *pValue );
void DF_SetMaterialVarVector4D( IMaterial* pMat, const char* pVarName, const Vector4D &vValue );
#define ALLOW_DEBUG_RT

#endif //DF_VIEW_SCENE_H