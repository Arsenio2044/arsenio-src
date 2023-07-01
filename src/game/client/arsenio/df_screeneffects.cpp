//==============================================================================//
//
// 
// 
// 
//
//=============================================================================//


#include "cbase.h"
#include "ScreenSpaceEffects.h"
#include "rendertexture.h"
#include "model_types.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imaterialvar.h"
#include "cdll_client_int.h"
#include "materialsystem/itexture.h"
#include "KeyValues.h"
#include "clienteffectprecachesystem.h"
#include "viewrender.h"
#include "view_scene.h"
#include "c_basehlplayer.h"
#include "tier0/vprof.h"
#include "view.h"
#include "hl2_gamerules.h"

#include "df_screeneffects.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar fov_desired;
extern ConVar r_post_healtheffects_radial;
extern CViewRender g_DefaultViewRender;
extern bool g_bRenderingScreenshot;



ConVar r_post_sunshaft_blur( "r_post_sunshaft_blur", "1", FCVAR_ARCHIVE );
ConVar r_post_sunshaft_blur_amount( "r_post_sunshaft_blur_amount", "0.5", FCVAR_CHEAT );
void CSunShaftEffect::Init( void )
{
	PrecacheMaterial( "effects/shaders/blurx" );
	PrecacheMaterial( "effects/shaders/blury" );
	PrecacheMaterial( "effects/shaders/sunshaft_base" );
	PrecacheMaterial( "effects/shaders/sunshaft_final" );

	m_SunShaft_BlurX.Init( materials->FindMaterial("effects/shaders/blurx", TEXTURE_GROUP_PIXEL_SHADERS, true) );
	m_SunShaft_BlurY.Init( materials->FindMaterial("effects/shaders/blury", TEXTURE_GROUP_PIXEL_SHADERS, true) );

	m_SunShaftBlendMat.Init( materials->FindMaterial("effects/shaders/sunshaft_final", TEXTURE_GROUP_CLIENT_EFFECTS, true) );
	m_SunShaftMask.Init( materials->FindMaterial("effects/shaders/sunshaft_base", TEXTURE_GROUP_PIXEL_SHADERS, true) );
	m_SunShaftDebug.Init( materials->FindMaterial("effects/shaders/sunshaft_base", TEXTURE_GROUP_CLIENT_EFFECTS, true) );
}

void CSunShaftEffect::Shutdown( void )
{
	m_SunShaft_BlurX.Shutdown();
	m_SunShaft_BlurY.Shutdown();

	m_SunShaftBlendMat.Shutdown();

	m_SunShaftMask.Shutdown();
	m_SunShaftDebug.Shutdown();
}

extern ConVar r_post_sunshaft;
ConVar r_post_sunshaft_debug( "r_post_sunshaft_debug", "0", FCVAR_CHEAT );

bool CSunShaftEffect::ShaftsRendering( void )
{
	return ( r_post_sunshaft.GetBool() && engine->IsSkyboxVisibleFromPoint( CurrentViewOrigin() ) && IsEnabled() );
}

static void SetRenderTargetAndViewPort(ITexture *rt)
{
	CMatRenderContextPtr pRenderContext( materials );
	pRenderContext->SetRenderTarget(rt);
	pRenderContext->Viewport(0,0,rt->GetActualWidth(),rt->GetActualHeight());
}

void CSunShaftEffect::Render( int x, int y, int w, int h )
{
	VPROF( "CSunShaftEffect::Render" );

	if( !ShaftsRendering() )
		return;

	if( r_post_sunshaft_debug.GetInt() == 1 )
	{
		DrawScreenEffectMaterial( m_SunShaftMask, x, y, w, h );
		return;
	}

	IMaterialVar *var;
	CMatRenderContextPtr pRenderContext( materials );

	pRenderContext->PushRenderTargetAndViewport();

	ITexture *dest_rt0 = materials->FindTexture( "_rt_SmallFB0", TEXTURE_GROUP_RENDER_TARGET );
	ITexture *dest_rt1 = materials->FindTexture( "_rt_SmallFB1", TEXTURE_GROUP_RENDER_TARGET );

	SetRenderTargetAndViewPort( dest_rt0 );

	pRenderContext->DrawScreenSpaceRectangle( m_SunShaftMask, 0, 0, w/4, h/4,
											0, 0, w/4-1, h/4-1,
											w/4, h/4 );

	if ( IsX360() )
	{
		pRenderContext->CopyRenderTargetToTextureEx( dest_rt0, 0, NULL, NULL );
	}

	//Render the gaussian blur pass over our shafts.
	if( r_post_sunshaft_blur.GetBool() )
	{
		var = m_SunShaft_BlurX->FindVar( "$fbtexture", NULL );
		var->SetTextureValue( dest_rt0 );
		var = m_SunShaft_BlurX->FindVar( "$resdivisor", NULL );
		var->SetIntValue( 4 );
		var = m_SunShaft_BlurX->FindVar( "$blursize", NULL );
		var->SetFloatValue( r_post_sunshaft_blur_amount.GetFloat() );

		SetRenderTargetAndViewPort( dest_rt1 );
		pRenderContext->DrawScreenSpaceRectangle(	m_SunShaft_BlurX, 0, 0, w/4, h/4,
													0, 0, w/4-1, h/4-1,
													w/4, h/4 );
		if ( IsX360() )
		{
			pRenderContext->CopyRenderTargetToTextureEx( dest_rt1, 0, NULL, NULL );
		}

		var = m_SunShaft_BlurY->FindVar( "$fbtexture", NULL );
		var->SetTextureValue( dest_rt1 );
		var = m_SunShaft_BlurY->FindVar( "$resdivisor", NULL );
		var->SetIntValue( 4 );
		var = m_SunShaft_BlurY->FindVar( "$blursize", NULL );
		var->SetFloatValue( r_post_sunshaft_blur_amount.GetFloat() );

		SetRenderTargetAndViewPort( dest_rt0 );
		pRenderContext->DrawScreenSpaceRectangle(	m_SunShaft_BlurY, 0, 0, w/4, h/4,
													0, 0, w/4-1, h/4-1,
													w/4, h/4 );
		if ( IsX360() )
		{
			pRenderContext->CopyRenderTargetToTextureEx( dest_rt0, 0, NULL, NULL );
		}
	}

	pRenderContext->PopRenderTargetAndViewport();

	if( r_post_sunshaft_debug.GetInt() == 2 )
	{
		pRenderContext->DrawScreenSpaceRectangle( m_SunShaftDebug, 0, 0, w, h,
												0, 0, w-1, h-1,
												w, h );
		return;
	}

	//Render our sun to the screen additively.
	DrawScreenEffectMaterial( m_SunShaftBlendMat, x, y, w, h );
}

// Sun Shafts
//ADD_SCREENSPACE_EFFECT(CSunShaftEffect, c17_sunshaft);