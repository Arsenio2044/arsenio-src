//========= Copyright 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Ported from TI code base to Downfall. TI specific code removed.
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "df_view_scene.h"
#include "view_scene.h"
#include "df_render_targets.h"
#include "materialsystem/imaterialvar.h"
#include "input.h"
#include "iclientmode.h"
#include "viewpostprocess.h"
#include "tier1/fmtstr.h"


#include "ivieweffects.h"
// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>


// @TODO: move this parameter to an entity property rather than convar
//ConVar mat_dest_alpha_range( "mat_dest_alpha_range", "1000", 0, "Amount to scale depth values before writing into destination alpha ([0,1] range)." );


void SetRenderTargetAndViewPort(ITexture *rt);
void SetRenderTargetAndViewPort(ITexture *rt, int w, int h);

void DF_SetMaterialVarFloat(IMaterial* pMat, const char* pVarName, float flValue)
{
	Assert(pMat != NULL);
	Assert(pVarName != NULL);
	if (pMat == NULL || pVarName == NULL)
		return;

	IMaterialVar* pVar = pMat->FindVar(pVarName, NULL, false);
	if (pVar)	pVar->SetFloatValue(flValue);
}

void DF_SetMaterialVarInt(IMaterial* pMat, const char* pVarName, int iValue)
{
	Assert(pMat != NULL);
	Assert(pVarName != NULL);
	if (pMat == NULL || pVarName == NULL)
		return;

	IMaterialVar* pVar = pMat->FindVar(pVarName, NULL, false);
	if (pVar)	pVar->SetIntValue(iValue);
}
void DF_SetMaterialVarString(IMaterial* pMat, const char* pVarName, const char *pValue)
{
	Assert(pMat != NULL);
	Assert(pVarName != NULL);
	if (pMat == NULL || pVarName == NULL)
		return;

	IMaterialVar* pVar = pMat->FindVar(pVarName, NULL, false);
	if (pVar)	pVar->SetStringValue(pValue);
}
void DF_SetMaterialVarTexture(IMaterial* pMat, const char* pVarName, ITexture *pValue)
{
	Assert(pMat != NULL);
	Assert(pVarName != NULL);
	if (pMat == NULL || pVarName == NULL)
		return;

	IMaterialVar* pVar = pMat->FindVar(pVarName, NULL, false);
	if (pVar)	pVar->SetTextureValue(pValue);
}
void DF_SetMaterialVarVector(IMaterial* pMat, const char* pVarName, const Vector &vValue)
{
	Assert(pMat != NULL);
	Assert(pVarName != NULL);
	if (pMat == NULL || pVarName == NULL)
		return;

	IMaterialVar* pVar = pMat->FindVar(pVarName, NULL, false);
	if (pVar)	pVar->SetVecValue(vValue.Base(), 3);
}
void DF_SetMaterialVarVector4D( IMaterial* pMat, const char* pVarName, const Vector4D &vValue )
{
	Assert( pMat != NULL );
	Assert( pVarName != NULL );
	if ( pMat == NULL || pVarName == NULL )
		return;

	IMaterialVar* pVar = pMat->FindVar( pVarName, NULL, false );
	if ( pVar )	pVar->SetVecValue( vValue.Base(), 4 );
}

CDFViewRender g_ViewRender;

CDFViewRender::CDFViewRender()
{
	
}
void CDFViewRender::LevelInit()
{
	BaseClass::LevelInit();
}


void CDFViewRender::OnRenderStart()
{
	CViewRender::OnRenderStart();
	//CMatRenderContextPtr pRenderContext( materials );
	// reenable if downfall wants custom depth scale
	//pRenderContext->SetFloatRenderingParameter( FLOAT_RENDERPARM_DEST_ALPHA_DEPTH_SCALE, mat_dest_alpha_range.GetFloat() );
}

//-----------------------------------------------------------------------------
// Purpose: Renders extra 2D effects in derived classes while the 2D view is on the stack
//-----------------------------------------------------------------------------
void CDFViewRender::Render2DEffectsPreHUD(const CViewSetup &view)
{
}

//-----------------------------------------------------------------------------
// Purpose: Renders extra 2D effects in derived classes while the 2D view is on the stack
//-----------------------------------------------------------------------------

static CMaterialReference s_pDebugRT;
static ConVar r_debugrt("r_debugrt", "", FCVAR_CLIENTDLL );
static ConVar r_debugrt_size("r_debugrt_size", "128", FCVAR_ARCHIVE, "", true, 64, true, 1024 );	//save it.
static ConVar r_debugrt_additive("r_debugrt_additive", "0", FCVAR_CLIENTDLL );
void DrawDebugRT( const char *szTexName, int x, int y, int size, bool ssrect )
{
	if ( !s_pDebugRT.IsValid() )
	{
		// Create dummy material KV data
		KeyValues *kv = new KeyValues( "unlitgeneric" );
		kv->SetInt( "$ignorez", 1 );
		kv->SetInt( "$nofog", 1 );
		kv->SetInt( "$additive", 0 );
		kv->SetInt( "$translucent", 0 );
		s_pDebugRT.Init( g_pMaterialSystem->CreateMaterial( "debug_rt", kv ) );
		s_pDebugRT->Refresh();
	}
	CMatRenderContextPtr pRenderContext( materials );
	ITexture *pTex = materials->FindTexture( szTexName, TEXTURE_GROUP_RENDER_TARGET );
	DF_SetMaterialVarTexture( s_pDebugRT, "$basetexture", pTex  );
	DF_SetMaterialVarInt( s_pDebugRT, "$additive", r_debugrt_additive.GetInt() );

	if ( ssrect )
	{
		pRenderContext->DrawScreenSpaceRectangle( s_pDebugRT, x, y, size, size, pTex->GetActualWidth(), pTex->GetActualHeight(), 0, 0, pTex->GetActualWidth(), pTex->GetActualWidth() );
	}
	else
	{
		pRenderContext->Bind( s_pDebugRT );
		IMesh* pMesh = pRenderContext->GetDynamicMesh( true );
		CMeshBuilder meshBuilder;
		meshBuilder.Begin( pMesh, MATERIAL_QUADS, 1 );
		meshBuilder.Color4ub( 255, 255, 255, 255);	meshBuilder.TexCoord2f( 0,0,0 );	meshBuilder.Position3f( x, y, 0 );	meshBuilder.AdvanceVertex();
		meshBuilder.Color4ub( 255, 255, 255, 255);	meshBuilder.TexCoord2f( 0,1,0 );	meshBuilder.Position3f( x+size, y, 0 );	meshBuilder.AdvanceVertex();
		meshBuilder.Color4ub( 255, 255, 255, 255);	meshBuilder.TexCoord2f( 0,1,1 );	meshBuilder.Position3f( x+size, y+size, 0 );	meshBuilder.AdvanceVertex();
		meshBuilder.Color4ub( 255, 255, 255, 255);	meshBuilder.TexCoord2f( 0,0,1 );	meshBuilder.Position3f( x, y+size, 0 );	meshBuilder.AdvanceVertex();
		meshBuilder.End();
		pMesh->Draw();
	}
}

void CDFViewRender::Render2DEffectsPostHUD( const CViewSetup &view )
{
	{
		CUtlString rendertarget = r_debugrt.GetString();
		if ( !rendertarget.IsEmpty() )
			DrawDebugRT( rendertarget.Get(), 256, 32, r_debugrt_size.GetInt(), false );
	}
}

static CMaterialReference s_pFadeHack;
void CDFViewRender::DoCustomPostProcessing(const CViewSetup &view)
{
	//Tony; moved this HERE instead.
	//Tony; SWITCHED MY EFFECTS WITH BLOOM
	if (!building_cubemaps.GetBool())
	{
		CMatRenderContextPtr pRenderContext( materials );
		
		bool didSun = DoSunAndGlowEffects( view.x, view.y, view.width, view.height  );

		if ( didSun )
		{
			//byte color[4];
			//bool modulate;
			//vieweffects->GetFadeParams( &color[0], &color[1], &color[2], &color[3], &modulate );

			////Tony; only draw this if it has an alpha value > 0
			//if ( !modulate && color[3] > 0 )
			//{
			//	//
			//	if ( !s_pFadeHack.IsValid() )
			//	{
			//		KeyValues *kv = new KeyValues( "simple_color" );
			//		kv->SetInt( "$translucent", 1 );
			//		s_pFadeHack.Init( g_pMaterialSystem->CreateMaterial( "__fade_hack",	kv ) );
			//		s_pFadeHack->Refresh();
			//	}
			//	float clr[4];
			//	for ( int i = 0; i<4;i++ )
			//		clr[i] = color[i] / 255.0f;
			//	//debug printing to make sure it works.
			//	//Plat_DebugString( CFmtStr("fade Color: %f %f %f %f\n", clr[0], clr[1], clr[2], clr[3] ));
			//	DF_SetMaterialVarFloat( s_pFadeHack, "$ALPHA", clr[3] );
			//	DF_SetMaterialVarString( s_pFadeHack, "$COLOR", CFmtStr( "%f %f %f", clr[0], clr[1], clr[2]) );
			//	pRenderContext->DrawScreenSpaceQuad( s_pFadeHack );
			//}
		}
		//Tony; bombCamera overrides erverythign else.

		pRenderContext.SafeRelease();
	}
}

static CMaterialReference s_pMatSkyDraw;
static CMaterialReference s_pMatViewmodelDraw;
static CMaterialReference s_pMatScreenRestore;
static CMaterialReference s_pMatSkyCombineMasks;
static CMaterialReference s_pMatViewmodelCombineMasks;

static void CheckCreateSkymaskMaterials()
{
	if ( !s_pMatSkyDraw.IsValid() )
	{
		KeyValues *kv = new KeyValues( "fill_viewmask" );
		kv->SetInt( "$COMBINEMODE", 0 );
		kv->SetInt( "$VIEWMODEL", 0 );
		s_pMatSkyDraw.Init(materials->CreateMaterial( "__skymask_draw", kv ));
		s_pMatSkyDraw->Refresh();
		Assert( s_pMatSkyDraw );
	}
	if ( !s_pMatSkyCombineMasks.IsValid() )
	{
		KeyValues *kv = new KeyValues( "fill_viewmask" );
		kv->SetString( "$BASETEXTURE", "_rt_SkyMask" );
		kv->SetInt( "$COMBINEMODE", 1 );
		kv->SetInt( "$VIEWMODEL", 0 );
		s_pMatSkyCombineMasks.Init( materials->CreateMaterial( "__skymask_combine", kv ) );
		s_pMatSkyCombineMasks->Refresh();
		Assert( s_pMatSkyCombineMasks );
	}
	if ( !s_pMatScreenRestore.IsValid() )
	{
		KeyValues *kv = new KeyValues( "FULLSCREENQUAD_WRITEA" );
		kv->SetString( "$BASETEXTURE", "_rt_FullFrameFB" );
		kv->SetInt( "$COMBINEMODE", 0 );
		s_pMatScreenRestore.Init( materials->CreateMaterial( "_skymask_restore", kv ) );
		s_pMatScreenRestore->Refresh();
		Assert( s_pMatScreenRestore );
	}

	if ( !s_pMatViewmodelDraw.IsValid() )
	{
		KeyValues *kv = new KeyValues( "fill_viewmask" );
		kv->SetInt( "$COMBINEMODE", 0 );
		kv->SetInt( "$VIEWMODEL", 1 );
		s_pMatViewmodelDraw.Init( materials->CreateMaterial( "__viewmodelmask_draw", kv ) );
		s_pMatViewmodelDraw->Refresh();
		Assert( s_pMatViewmodelDraw );
	}
	if ( !s_pMatViewmodelCombineMasks.IsValid() )
	{
		KeyValues *kv = new KeyValues( "fill_viewmask" );
		kv->SetString( "$BASETEXTURE", "_rt_ViewmodelMask" );
		kv->SetString( "$BASETEXTURE2", "_rt_SkyMask" );
		kv->SetInt( "$COMBINEMODE", 1 );	//Tony; new combine mode. omits the sky as well as the view model.
		kv->SetInt( "$VIEWMODEL", 1 );
		s_pMatViewmodelCombineMasks.Init( materials->CreateMaterial( "__viewmodelmask_combine", kv ) );
		s_pMatViewmodelCombineMasks->Refresh();
		Assert( s_pMatViewmodelCombineMasks );
	}
}

extern ConVar r_suneffects;
void UpdateViewMask( const CViewSetup &view, bool VIEWMODEL, bool combine )
{
	CheckCreateSkymaskMaterials();

	CMatRenderContextPtr pRenderContext( materials );

	int dest_width = view.width;
	int dest_height = view.height;
	float src_x1 = view.width -1;
	float src_y1 = view.height -1;

	ITexture *pFullFrame = GetFullFrameFrameBufferTexture( 0 );

	if ( VIEWMODEL )
	{
		//if ( !r_anamorphic.GetBool() ) 
			return;
		ITexture *pViewmodelMask = materials->FindTexture( "_rt_ViewmodelMask", TEXTURE_GROUP_RENDER_TARGET );
		IMaterial *pOp = pOp = combine ? s_pMatViewmodelCombineMasks : s_pMatViewmodelDraw;
	
		
		pRenderContext->PushRenderTargetAndViewport(NULL);
		pRenderContext->CopyRenderTargetToTexture( pFullFrame );
						
		pRenderContext->DrawScreenSpaceRectangle( pOp, view.x, view.y, dest_width, dest_height, 0, 0, src_x1, src_y1, view.width, view.height );
	
		pRenderContext->CopyRenderTargetToTexture( pViewmodelMask );
		pRenderContext->DrawScreenSpaceRectangle( s_pMatScreenRestore,view.x, view.y, dest_width, dest_height, 0, 0, src_x1, src_y1, view.width, view.height );
	
		pRenderContext->PopRenderTargetAndViewport();
	}
	else
	{
		if ( !r_suneffects.GetBool() ) return;
		ITexture *pSkyMask = materials->FindTexture( "_rt_SkyMask", TEXTURE_GROUP_RENDER_TARGET );
		
		IMaterial *pOp = pOp = combine ? s_pMatSkyCombineMasks : s_pMatSkyDraw;

		pRenderContext->PushRenderTargetAndViewport(NULL);
		pRenderContext->CopyRenderTargetToTexture( pFullFrame );
		
		pRenderContext->DrawScreenSpaceRectangle( pOp, view.x, view.y, dest_width, dest_height, 0, 0, src_x1, src_y1, view.width, view.height );

		pRenderContext->CopyRenderTargetToTexture( pSkyMask );

		pRenderContext->DrawScreenSpaceRectangle( s_pMatScreenRestore,view.x, view.y, dest_width, dest_height, 0, 0, src_x1, src_y1, view.width, view.height );

		pRenderContext->PopRenderTargetAndViewport();
	}
}