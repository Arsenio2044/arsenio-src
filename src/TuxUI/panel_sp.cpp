//========= Copyright Glitch Software, All rights reserved. ===================//
//
// Purpose: Replace GameUI with TuxUI.
//
//
//=============================================================================//

#include "gameui2_interface.h"
#include "panel_sp.h"

#include "ienginevgui.h"
#include "vgui/ILocalize.h"
#include "vgui/ISurface.h"
#include "vgui/IVGui.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// A simple menu for the campaign.

Panel_Singleplayer::Panel_Singleplayer(vgui::VPANEL Parent, const char* PanelName) : BaseClass(nullptr, PanelName)
{

	vgui::HScheme Scheme = vgui::scheme()->LoadSchemeFromFile("TuxUI/schemepanel.res", "SchemePanel");

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

	PanelTitle = GetGameUI2().ConvertToLocalizedString("#AR_SP");
	PanelTitle = wcsupr(PanelTitle);

	m_pBtnNewGame = new Button_Panel(this, this, "opennewgamedialog");
	m_pBtnNewGame->SetButtonText("#NG");
	m_pBtnNewGame->SetButtonDescription("#GameUI2_NewGameDescription");

	m_pBtnLoadGame = new Button_Panel(this, this, "openloadgamedialog");
	m_pBtnLoadGame->SetButtonText("#LG");
	m_pBtnLoadGame->SetButtonDescription("#GameUI2_LoadGameDescription");

	m_pBtnBack = new Button_Panel(this, this, "action_back");
	m_pBtnBack->SetButtonText("#GameUI2_Back");
	m_pBtnBack->SetButtonDescription("");
}

void Panel_Singleplayer::ApplySchemeSettings(vgui::IScheme* Scheme)
{
	BaseClass::ApplySchemeSettings(Scheme);

	BackgroundGradientTop = GetSchemeColor("Panel.Background.Gradient.Top", Scheme);
	BackgroundGradientBottom = GetSchemeColor("Panel.Background.Gradient.Bottom", Scheme);

	TitleColor = GetSchemeColor("Panel.Title.Color", Scheme);

	TitleOffsetX = atof(Scheme->GetResourceString("Panel.Title.OffsetX"));
	TitleOffsetY = atof(Scheme->GetResourceString("Panel.Title.OffsetY"));

	TitleFont = Scheme->GetFont("Panel.Title.Font");
}

void Panel_Singleplayer::Animations()
{
	if (AnimationController != nullptr)
		AnimationController->UpdateAnimations(GetGameUI2().GetTime());

	SetBounds(0, 0, GetGameUI2().GetViewport().x, GetGameUI2().GetViewport().y);
}

void Panel_Singleplayer::OnThink()
{
	BaseClass::OnThink();

	SetContentBounds();
	Animations();

	if (IsVisible() == false)
	{
		ConColorMsg(Color(0, 148, 255, 255), "Singleplayer panel is not visible, running all animations to completion...\n");

		if (AnimationController != nullptr)
			AnimationController->RunAllAnimationsToCompletion();
	}
}

void Panel_Singleplayer::SetContentBounds()
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

void Panel_Singleplayer::Paint()
{
	BaseClass::Paint();

	DrawBackground();
	DrawTitle();
	DrawTabs();
	DrawBasicButtons();
}

void Panel_Singleplayer::PaintBlurMask()
{
	BaseClass::PaintBlurMask();
}

void Panel_Singleplayer::DrawBackground()
{
	vgui::surface()->DrawSetColor(BackgroundGradientTop);
	vgui::surface()->DrawFilledRectFade(0, 0, GetWide() + 0, GetTall() + 0, 255, 0, false);

	vgui::surface()->DrawSetColor(BackgroundGradientBottom);
	vgui::surface()->DrawFilledRectFade(0, 0, GetWide() + 0, GetTall() + 0, 0, 255, false);
}


void Panel_Singleplayer::DrawTitle()
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

void Panel_Singleplayer::DrawTabs()
{
	// TEST!
	int32 X0, Y0;
	m_pBtnNewGame->GetPos(X0, Y0);
	
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

void Panel_Singleplayer::DrawBasicButtons()
{
	m_pBtnNewGame->SetPos(ContentX0, ContentY1 - m_pBtnNewGame->GetTall());
	m_pBtnNewGame->SetVisible(true);

	int32 X0, Y0;
	m_pBtnNewGame->GetPos(X0, Y0);
	
	m_pBtnLoadGame->SetPos(X0 + m_pBtnLoadGame->GetWide(), ContentY1 - m_pBtnLoadGame->GetTall());
	m_pBtnLoadGame->SetVisible(true);
	
	m_pBtnBack->SetPos(ContentX1 - m_pBtnBack->GetWide(), ContentY1 - m_pBtnBack->GetTall());
	m_pBtnBack->SetVisible(true);
}

void Panel_Singleplayer::OnCommand(char const* Command)
{
	if (Q_stristr(Command, "action_back"))
		Close();
	else if (Q_stristr(Command, "opennewgamedialog"))
		GetGameUI2().GetEngineClient()->ClientCmd_Unrestricted("gamemenucommand opennewgamedialog");
	else if (Q_stristr(Command, "openloadgamedialog"))
		GetGameUI2().GetEngineClient()->ClientCmd_Unrestricted("gamemenucommand openloadgamedialog");
	else
		BaseClass::OnCommand(Command);
}

CON_COMMAND(gameui2_openspdialog, "")
{
	new Panel_Singleplayer(GetGameUI2().GetVPanel(), "");
}