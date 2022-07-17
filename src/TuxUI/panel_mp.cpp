//========= Copyright Glitch Software, All rights reserved. ===================//
//
// Purpose: Replace GameUI with TuxUI.
//
//
//=============================================================================//
#include "gameui2_interface.h"
#include "panel_mp.h"

#include "ienginevgui.h"
#include "vgui/ILocalize.h"
#include "vgui/ISurface.h"
#include "vgui/IVGui.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

Panel_Multiplayer::Panel_Multiplayer(vgui::VPANEL Parent, const char* PanelName) : BaseClass(nullptr, PanelName)
{
#ifdef MFS
	vgui::HScheme Scheme = vgui::scheme()->LoadSchemeFromFile("resource/schemepanel.res", "SchemePanel");
#else
	vgui::HScheme Scheme = vgui::scheme()->LoadSchemeFromFile("TuxUI/schemepanel.res", "SchemePanel");
#endif
	SetScheme(Scheme);

	AnimationController = new vgui::AnimationController(this);
	AnimationController->SetScheme(Scheme);
	AnimationController->SetProportional(false);

	SetProportional(false);
	SetPaintBorderEnabled(false);
	SetPaintBackgroundEnabled(false);
	SetDeleteSelfOnClose(true);
	SetTitleBarVisible(false);
	SetSizeable(false);
	SetMoveable(false);
	SetEnabled(true);
	SetVisible(false);
	SetParent(Parent);

	Activate();

	PanelTitle = GetGameUI2().ConvertToLocalizedString("#MFS_MP");
	PanelTitle = wcsupr(PanelTitle);

	m_pBtnCS = new Button_Panel(this, this, "OpenCreateMultiplayerGameDialog");
	m_pBtnCS->SetButtonText("#CS");
	m_pBtnCS->SetButtonDescription("#GameUI2_CSDescription");

	m_pBtnFS = new Button_Panel(this, this, "OpenServerBrowser");
	m_pBtnFS->SetButtonText("#FS");
	m_pBtnFS->SetButtonDescription("#GameUI2_FSDescription");

	m_pBtnBack = new Button_Panel(this, this, "action_back");
	m_pBtnBack->SetButtonText("#GameUI2_Back");
	m_pBtnBack->SetButtonDescription("");
}

void Panel_Multiplayer::ApplySchemeSettings(vgui::IScheme* Scheme)
{
	BaseClass::ApplySchemeSettings(Scheme);

	BackgroundGradientTop = GetSchemeColor("Panel.Background.Gradient.Top", Scheme);
	BackgroundGradientBottom = GetSchemeColor("Panel.Background.Gradient.Bottom", Scheme);

	TitleColor = GetSchemeColor("Panel.Title.Color", Scheme);

	TitleOffsetX = atof(Scheme->GetResourceString("Panel.Title.OffsetX"));
	TitleOffsetY = atof(Scheme->GetResourceString("Panel.Title.OffsetY"));

	TitleFont = Scheme->GetFont("Panel.Title.Font");
}

void Panel_Multiplayer::Animations()
{
	if (AnimationController != nullptr)
		AnimationController->UpdateAnimations(GetGameUI2().GetTime());

	SetBounds(0, 0, GetGameUI2().GetViewport().x, GetGameUI2().GetViewport().y);
}

void Panel_Multiplayer::OnThink()
{
	BaseClass::OnThink();

	SetContentBounds();
	Animations();

	if (IsVisible() == false)
	{
		ConColorMsg(Color(0, 148, 255, 255), "Multiplayer panel is not visible, running all animations to completion...\n");

		if (AnimationController != nullptr)
			AnimationController->RunAllAnimationsToCompletion();
	}
}

void Panel_Multiplayer::SetContentBounds()
{	
	ContentW = (GetWide() / 100) * 75;
	if (ContentW > 1920)
		ContentW = 1920;
	else if (ContentW < 800)
		ContentW = 800;

	ContentH = (GetTall() / 100) * 75;
	if (ContentH > 1080)
		ContentH = 1080;
	else if (ContentH < 600)
		ContentH = 600;

	ContentX0 = GetWide() / 2 - ContentW / 2;
	ContentY0 = GetTall() / 2 - ContentH / 2;

	ContentX1 = ContentX0 + ContentW;
	ContentY1 = ContentY0 + ContentH;
}

void Panel_Multiplayer::Paint()
{
	BaseClass::Paint();

	DrawBackground();
	DrawTitle();
	DrawTabs();
	DrawBasicButtons();
}

void Panel_Multiplayer::PaintBlurMask()
{
	BaseClass::PaintBlurMask();
}

void Panel_Multiplayer::DrawBackground()
{
	vgui::surface()->DrawSetColor(BackgroundGradientTop);
	vgui::surface()->DrawFilledRectFade(0, 0, GetWide() + 0, GetTall() + 0, 255, 0, false);

	vgui::surface()->DrawSetColor(BackgroundGradientBottom);
	vgui::surface()->DrawFilledRectFade(0, 0, GetWide() + 0, GetTall() + 0, 0, 255, false);
}


void Panel_Multiplayer::DrawTitle()
{
	if (PanelTitle == nullptr)
		return;
	
	vgui::surface()->DrawSetTextColor(TitleColor);
	vgui::surface()->DrawSetTextFont(TitleFont);

	vgui::surface()->GetTextSize(TitleFont, PanelTitle, TitleSizeX, TitleSizeY);
	TitlePositionX = ContentX0 + TitleOffsetX;
	TitlePositionY = ContentY0 + TitleOffsetY;
	
	vgui::surface()->DrawSetTextPos(TitlePositionX, TitlePositionY);
	vgui::surface()->DrawPrintText(PanelTitle, wcslen(PanelTitle));
}

void Panel_Multiplayer::DrawTabs()
{
	// TEST!
	int32 X0, Y0;
	m_pBtnCS->GetPos(X0, Y0);
	
	int8 ObjectHeight = 48;
	int16 ContentHeight = Y0 - (TitlePositionY - TitleOffsetY + TitleSizeY);
	int8 ObjectsPerHeight = ContentHeight / ObjectHeight;

	for (int8 i = 0; i < ObjectsPerHeight; i++)
	{
		if (i % 2)
			vgui::surface()->DrawSetColor(Color(0, 0, 0, 20));
		else
			vgui::surface()->DrawSetColor(Color(255, 255, 255, 1));

		int32 ObjectY = TitlePositionY + TitleSizeY + (ObjectHeight * (i + 1)) - ObjectHeight;
		vgui::surface()->DrawFilledRect(ContentX0, ObjectY, ContentW + ContentX0, ObjectHeight + ObjectY);
	}
	// TEST!
}

void Panel_Multiplayer::DrawBasicButtons()
{
	m_pBtnCS->SetPos(ContentX0, ContentY1 - m_pBtnCS->GetTall());
	m_pBtnCS->SetVisible(true);

	int32 X0, Y0;
	m_pBtnCS->GetPos(X0, Y0);
	
	m_pBtnFS->SetPos(X0 + m_pBtnFS->GetWide(), ContentY1 - m_pBtnFS->GetTall());
	m_pBtnFS->SetVisible(true);
	
	m_pBtnBack->SetPos(ContentX1 - m_pBtnBack->GetWide(), ContentY1 - m_pBtnBack->GetTall());
	m_pBtnBack->SetVisible(true);
}

void Panel_Multiplayer::OnCommand(char const* Command)
{
	if (Q_stristr(Command, "action_back"))
		Close();
	else if (Q_stristr(Command, "OpenCreateMultiplayerGameDialog"))
		GetGameUI2().GetEngineClient()->ClientCmd_Unrestricted("gamemenucommand OpenCreateMultiplayerGameDialog");
	else if (Q_stristr(Command, "OpenServerBrowser"))
		GetGameUI2().GetEngineClient()->ClientCmd_Unrestricted("gamemenucommand OpenServerBrowser");
	else
		BaseClass::OnCommand(Command);
}

CON_COMMAND(gameui2_openmpdialog, "")
{
	new Panel_Multiplayer(GetGameUI2().GetVPanel(), "");
}