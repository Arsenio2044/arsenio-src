//========= TuxUI ===================//
//
// Purpose: Replace GameUI with TuxUI.
//
//
//=============================================================================//
#include "gameui2_interface.h"
#include "frame2d.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

Frame2D::Frame2D(vgui::Panel* Parent, const char* PanelName, bool bShowTaskbarIcon, bool bPopUp) : BaseClass(Parent, PanelName, bShowTaskbarIcon, bPopUp)
{
	SetBounds(0, 0, 0, 0);
	
	bSchemeLoaded = false;
}

void Frame2D::Paint()
{
	if (bSchemeLoaded == false)
		return;
	
	BaseClass::Paint();

	if (GetGameUI2().GetMaterialSystem() != nullptr &&
		GetGameUI2().GetRenderView() != nullptr &&
		GetGameUI2().GetMaskTexture() != nullptr &&
		GetGameUI2().GetFrustum() != nullptr)
	{
		bBlurEnabled = true;

		GetGameUI2().GetRenderView()->Push2DView(GetGameUI2().GetView(), 0, GetGameUI2().GetMaskTexture(), GetGameUI2().GetFrustum());
		PaintBlurMask();
		GetGameUI2().GetRenderView()->PopView(GetGameUI2().GetFrustum());
	}

	bBlurEnabled = false;
}

void Frame2D::PaintBlurMask()
{
	
}

void Frame2D::ApplySchemeSettings(vgui::IScheme* Scheme)
{
	BaseClass::ApplySchemeSettings(Scheme);

	bSchemeLoaded = true;
}