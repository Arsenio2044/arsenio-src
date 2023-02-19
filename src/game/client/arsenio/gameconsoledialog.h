

#ifndef GAMECONSOLEDIALOG_H
#define GAMECONSOLEDIALOG_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "vgui_controls/consoledialog.h"
#include <Color.h>
#include "UtlVector.h"
#include "vgui_controls/Frame.h"

//-----------------------------------------------------------------------------
// Purpose: Game/dev console dialog
//-----------------------------------------------------------------------------
class CGameConsoleDialog : public vgui::CConsoleDialog
{
	DECLARE_CLASS_SIMPLE(CGameConsoleDialog, vgui::CConsoleDialog);

public:
	CGameConsoleDialog();

	void ToggleConsole(bool bVisibile, bool bForceOff = false);
	void OnThink();

private:
	MESSAGE_FUNC_CHARPTR(OnCommandSubmitted, "CommandSubmitted", command);
	bool bShouldShow;
	bool bShouldClose;
	float m_flLerp;

protected:
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void OnKeyCodeTyped(vgui::KeyCode code);
	virtual void PaintBackground();
};

#endif // GAMECONSOLEDIALOG_H
