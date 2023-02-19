

#include "cbase.h"
#include "gameconsoledialog.h"
#include "vgui/iinput.h"
#include "vgui/isurface.h"
#include "vgui/keycode.h"
#include "GameBase_Client.h"
#include "ienginevgui.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CGameConsoleDialog::CGameConsoleDialog() : BaseClass(NULL, "GameConsole", false)
{
	AddActionSignalTarget(this);
	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/TFOConsole.res", "TFOConsole"));
	SetVisible(false);
	//
	SetMinimizeButtonVisible(false);
	SetMaximizeButtonVisible(false);
	SetCloseButtonVisible(false);
	SetSizeable(false);
	SetMoveable(false);
	//
	SetTitle("TFO Console", true);
	bShouldShow = false;
	bShouldClose = false;

	SetSize(ScreenWidth(), 512);
	SetPos(0, -512);

	PerformLayout();

	InvalidateLayout();

	m_flLerp = 0.0f;
}

void CGameConsoleDialog::OnCommandSubmitted(const char *pCommand)
{
	const char *szCommand = pCommand;

	if (!Q_strnicmp(szCommand, "map ", 4))
	{
		szCommand += 4;
	}
	else if (!Q_strnicmp(szCommand, "load ", 5))
	{
		szCommand += 5;
	}
	else if (!Q_strnicmp(szCommand, "save ", 5))
	{
		szCommand += 5;
		int iSlot = atoi(szCommand);
	}
	else if (!Q_strnicmp(szCommand, "reload", 6))
	{
	}
	else if (Q_stristr(szCommand, "disconnect") || Q_stristr(szCommand, "startupmenu"))
	{
		engine->ClientCmd_Unrestricted("tfo_mainmenu\n");
	}
	else
		engine->ClientCmd_Unrestricted(pCommand);
}

void CGameConsoleDialog::OnThink()
{
	int x, y;
	GetPos(x, y);

	if (bShouldClose || bShouldShow)
	{
		float flMilli = MAX(0.0f, 1000.0f - m_flLerp);
		float flSec = flMilli * 0.001f;
		if ((flSec > 0.5))
		{
			SetPos(x, bShouldShow ? 0 : -512);

			if (bShouldClose)
			{
				bShouldClose = false;
				engine->ClientCmd_Unrestricted("gameui_allowescapetoshow\n");
				SetVisible(false);
			}

			if (bShouldShow)
				bShouldShow = false;
		}
		else
		{
			float flFrac = SimpleSpline(flSec / 0.5);
			SetPos(x, (bShouldShow ? (1.0f - flFrac) : flFrac) * -512);
		}
	}

	// Update Timer:
	float frame_msec = 1000.0f * gpGlobals->frametime;

	if (m_flLerp > 0)
	{
		m_flLerp -= frame_msec;
		if (m_flLerp < 0)
			m_flLerp = 0;
	}

	BaseClass::OnThink();
}

void CGameConsoleDialog::ToggleConsole(bool bVisibile, bool bForceOff)
{
	// Force Console to Close = QUICK CLOSE...
	if (bForceOff)
	{
		SetSize(ScreenWidth(), 512);
		bShouldShow = false;
		bShouldClose = false;
		SetPos(0, -512);
		engine->ClientCmd_Unrestricted("gameui_allowescapetoshow\n");
		SetVisible(false);
		return;
	}

	if (bShouldClose || bShouldShow)
		return;

	SetKeyBoardInputEnabled(bVisibile);
	SetMouseInputEnabled(bVisibile);

	SetSize(ScreenWidth(), 512);
	bShouldShow = false;
	bShouldClose = false;

	vgui::surface()->PlaySound("common/console.wav");
	m_flLerp = 1000.0f;
	InvalidateLayout(false, true);

	if (bVisibile)
	{
		SetVisible(true);
		Activate();
		RequestFocus();
		bShouldShow = true;

		engine->ClientCmd_Unrestricted("gameui_preventescapetoshow\n");
	}
	else
	{
		bShouldClose = true;
	}
}

void CGameConsoleDialog::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetBorder(NULL);
	SetPaintBorderEnabled(false);
}

void CGameConsoleDialog::OnKeyCodeTyped(vgui::KeyCode code)
{
	if (code == KEY_ESCAPE)
	{
		ToggleConsole(false);
	}
	else
	{
		BaseClass::OnKeyCodeTyped(code);
	}
}

void CGameConsoleDialog::PaintBackground()
{
	SetFgColor(Color(20, 20, 20, 240));
	SetBgColor(Color(20, 16, 16, 230));
	SetPaintBackgroundType(0);
	SetPaintBackgroundEnabled(true);
	BaseClass::PaintBackground();
}