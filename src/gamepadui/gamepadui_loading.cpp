#include "gamepadui_loading.h"
#include "vgui/IInput.h"
#include "vgui_controls/ImagePanel.h"
#include "vgui/IVGui.h"
#include "vgui_controls/Frame.h"
#include "vgui_controls/ProgressBar.h"
#include "ienginevgui.h"
#include "KeyValues.h"
//#include "cdll_int.h"

using namespace vgui;

GamepadUILoading* g_pGamepadUILoading = NULL;
IFileSystem* filesystem = NULL;
IUniformRandomStream* random = NULL;
IVEngineClient* engine = NULL;
IEngineVGui* enginevgui = NULL;

#define ASS_COLOR		    Color( 255, 255, 255, 255 )

//-----------------------------------------------------------------------------
// Purpose: Performs a var args printf into a static return buffer
// Input  : *format - 
//			... - 
// Output : char
//-----------------------------------------------------------------------------
char* VarArgs(const char* format, ...)
{
	va_list		argptr;
	static char		string[1024];

	va_start(argptr, format);
	Q_vsnprintf(string, sizeof(string), format, argptr);
	va_end(argptr);

	return string;
}

void GetHudSize(int& w, int& h)
{
	vgui::surface()->GetScreenSize(w, h);

	VPANEL hudParent = enginevgui->GetPanel(PANEL_CLIENTDLL);
	if (hudParent)
	{
		vgui::ipanel()->GetSize(hudParent, w, h);
	}
}

//-----------------------------------------------------------------------------
// Purpose: ScreenHeight returns the height of the screen, in pixels
// Output : int
//-----------------------------------------------------------------------------
int ScreenHeight(void)
{
	int w, h;
	GetHudSize(w, h);
	return h;
}

//-----------------------------------------------------------------------------
// Purpose: ScreenWidth returns the width of the screen, in pixels
// Output : int
//-----------------------------------------------------------------------------
int ScreenWidth(void)
{
	int w, h;
	GetHudSize(w, h);
	return w;
}

GamepadUILoading::GamepadUILoading(VPANEL parent) : vgui::Panel(NULL, "LoadingDialog")
{
	SetParent(parent);
	SetVisible(false);
	MakePopup(false);
	SetProportional(true);
	SetPaintBackgroundEnabled(false);
	SetPaintBorderEnabled(false);



	m_iBarX = m_iBarY = m_iBarW = m_iBarH = 0;
	m_pBGImage = new vgui::ImagePanel(this, "BackgroundImage");
	m_bResolved = false;
	m_pDialog = nullptr;
	m_pProgress = nullptr;

	m_pProgressMirror = new vgui::ProgressBar(this, "ProgressMirror");
	m_pSpinnerImage = new vgui::ImagePanel(this, "SteamDeckSpinner");
	m_pLogoImage = new vgui::ImagePanel(this, "SteamDeckLogo");

	// Loading Tips
	m_pTextLoadingTip = vgui::SETUP_PANEL(new vgui::Label(this, "TextTip", ""));
	m_pTextLoadingTip->SetZPos(180);
	m_pTextLoadingTip->SetFgColor(ASS_COLOR);
	m_pTextLoadingTip->SetText("");

	m_SpinnerFrame = 0;



	InvalidateLayout(false, true);
}

void GamepadUILoading::ApplySchemeSettings(IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_pTextLoadingTip->SetFont(pScheme->GetFont("arial"));


	int wide, tall;
	vgui::surface()->GetScreenSize(wide, tall);
	SetSize(wide, tall);
	SetPos(0, 0);

	m_pProgressMirror->SetFgColor(Color(255, 134, 44, 255));
	m_pSpinnerImage->SetDrawColor(Color(255, 125, 20, 255));

	m_pSpinnerImage->SetImage("../gamepadui/spinner.vmt");
	m_pLogoImage->SetImage("../gamepadui/game_logo.vmt");
}

#define XRES(x)	( x  * ( ( float )GetWide() / 640.0 ) )
#define YRES(y)	( y  * ( ( float )GetTall() / 480.0 ) )
void GamepadUILoading::PerformLayout()
{
	int wide, tall;
	vgui::surface()->GetScreenSize(wide, tall);
	SetSize(wide, tall);

	m_pTextLoadingTip->SetFgColor(ASS_COLOR);


	m_pProgressMirror->SetProportional(false);
	m_pProgressMirror->SetBarInset(0);
	m_pProgressMirror->SetMargin(0);
	m_pProgressMirror->SetSegmentInfo(0, 1);
	m_pProgressMirror->SetPaintBorderEnabled(false);
	m_pProgressMirror->SetPaintBackgroundEnabled(false);
	m_pProgressMirror->IsRightAligned();
	// no one will notice it's not exactly correct, but they'll notice it less than there being gaps
	m_pProgressMirror->SetSize(wide + 2, YRES(20));
	m_pProgressMirror->SetPos(-2, (tall + 2) - YRES(20));

	int imageSize = XRES(40);
	int imageMargin = XRES(10);

	m_pSpinnerImage->SetSize(imageSize, imageSize);
	m_pSpinnerImage->SetShouldScaleImage(true);
	m_pSpinnerImage->SetPos(wide - imageSize - imageMargin, imageMargin);

	m_pTextLoadingTip->SetSize(ScreenWidth(), scheme()->GetProportionalScaledValueEx(GetScheme(), 30));

	int tipW, tipH;
	m_pTextLoadingTip->GetSize(tipW, tipH);

	m_pTextLoadingTip->SetPos(((ScreenWidth() / 2) - (tipW / 2)), scheme()->GetProportionalScaledValueEx(GetScheme(), 395));






	m_pLogoImage->SetSize(imageSize, imageSize);
	m_pLogoImage->SetShouldScaleImage(true);
	m_pLogoImage->SetPos(wide - imageSize - imageMargin, imageMargin);

	m_pBGImage->SetSize(wide, tall);
	m_pBGImage->SetShouldScaleImage(true);

	BaseClass::PerformLayout();
}

void GamepadUILoading::SetRandomLoadingTip()
{
	KeyValues* kvLoadingTips = new KeyValues("LoadingTipData");
	if (kvLoadingTips->LoadFromFile( filesystem, "resource/data/settings/Tips.txt", "MOD"))
	{
		int iAmountTips = 0;
		for (KeyValues* sub = kvLoadingTips->GetFirstSubKey(); sub; sub = sub->GetNextKey())
			iAmountTips++;

		
		//Msg("Loading tips\n");

		KeyValues* kvSelectedTip = kvLoadingTips->FindKey(VarArgs("%i", random->RandomInt(1, iAmountTips)));
		if (kvSelectedTip)
			m_pTextLoadingTip->SetText(kvSelectedTip->GetString());
	}
	else
	{
		m_pTextLoadingTip->SetText("");
		//Msg("No loading tips\n");
	}

	kvLoadingTips->deleteThis();
}

void GamepadUILoading::OnActivate()
{
	m_pDialog = nullptr;
	m_pProgress = nullptr;
	m_bResolved = false;

	m_SpinnerFrame = 0;

	m_pTextLoadingTip->SetVisible(true);


	int wide, tall;
	vgui::surface()->GetScreenSize(wide, tall);

	char szBGName[128];
	GamepadUI::GetInstance().GetEngineClient()->GetMainMenuBackgroundName(szBGName, sizeof(szBGName));
	char szImage[128];
	Q_snprintf(szImage, sizeof(szImage), "../console/%s", szBGName);
	bool bWidescreen = (((float)wide / (float)tall) < 1.5 ? false : true);
	if (bWidescreen)
		Q_strcat(szImage, "_widescreen", sizeof(szImage));

	// Tips / Poems / Conclusions:
	if (m_flTipDisplayTime <= engine->Time())
	{
		m_flTipDisplayTime = engine->Time() + 1.5f;
		SetRandomLoadingTip();
	}

	m_pBGImage->SetImage(szImage);
}


void GamepadUILoading::SolveEnginePanel()
{
	if (!m_bResolved && vgui::input()->GetAppModalSurface())
	{
		VPANEL _panel = vgui::input()->GetAppModalSurface();
		VPANEL _progress = ipanel()->GetChild(_panel, 2);

		m_pDialog = dynamic_cast<vgui::Frame*>(ipanel()->GetPanel(_panel, "GameUI"));
		m_pProgress = dynamic_cast<vgui::ProgressBar*>(ipanel()->GetPanel(_progress, "GameUI"));

		if (m_pDialog && m_pProgress)
		{
			m_pDialog->SetProportional(true);
			m_pDialog->SetSize(GetWide(), GetTall());
			m_pDialog->SetPos(0, 0);
			m_pDialog->SetPaintBackgroundEnabled(false);
			m_pDialog->SetCloseButtonVisible(false);

			m_pProgress->SetVisible(false);
			m_bResolved = true;
		}
	}
}

void GamepadUILoading::Paint()
{
	SolveEnginePanel();

	if (m_bResolved && m_pProgress)
	{
		m_pProgressMirror->SetProgress(m_pProgress->GetProgress());
		int spinnerFrame = m_SpinnerFrame++ % m_pSpinnerImage->GetNumFrames();
		m_pSpinnerImage->SetFrame(spinnerFrame);
	}
}

