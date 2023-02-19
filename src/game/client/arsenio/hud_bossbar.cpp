//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Displays the bossbar when fighting npcs. This is sent from tfo_boss_engager. (usermessage)
//
//=============================================================================//

#include "cbase.h" 
#include "hud.h" 
#include "hud_macros.h" 
#include "c_baseplayer.h"
#include "c_basehlplayer.h"
#include "iclientmode.h" 
#include "vgui/ISurface.h"
#include "usermessages.h"
#include "hudelement.h"
#include "hud_numericdisplay.h"
#include "vgui_controls/AnimationController.h"
#include <vgui/ILocalize.h>

#include "tier0/memdbgon.h" 

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: TFO Boss HUD
//-----------------------------------------------------------------------------

class CHudBossBar : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CHudBossBar, vgui::Panel);

public:
	CHudBossBar(const char * pElementName);

	virtual void Init(void);
	virtual void Reset(void);
	virtual void OnThink(void);

	void MsgFunc_BossData(bf_read &msg);

protected:
	virtual void Paint();
	virtual void PaintBackground();

	// Base Textures
	int m_nTexture_Bar[101];

	// Logic
	int BossHealth;
	int DivideFactor;
	bool m_bShouldShow;
	wchar_t szEntName[64];

private:

	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "HUD_TFO_Health");
	CPanelAnimationVar(Color, m_TextColor, "TextColor", "255 255 255 160");

	CPanelAnimationVarAliasType(float, m_flBarInsetX, "BarInsetX", "26", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarInsetY, "BarInsetY", "3", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarWidth, "BarWidth", "84", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarHeight, "BarHeight", "4", "proportional_float");

	CPanelAnimationVarAliasType(float, text_ypos, "text_ypos", "0", "proportional_float");
};

DECLARE_HUDELEMENT(CHudBossBar);
DECLARE_HUD_MESSAGE(CHudBossBar, BossData);

//------------------------------------------------------------------------
// Purpose: Constructor - Precached images
//------------------------------------------------------------------------
CHudBossBar::CHudBossBar(const char * pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudBossBar")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	for (int i = 0; i <= 100; i++)
		m_nTexture_Bar[i] = surface()->CreateNewTextureID();

	surface()->DrawSetTextureFile(m_nTexture_Bar[0], "vgui/hud/bossbar/000", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[1], "vgui/hud/bossbar/001", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[2], "vgui/hud/bossbar/002", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[3], "vgui/hud/bossbar/003", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[4], "vgui/hud/bossbar/004", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[5], "vgui/hud/bossbar/005", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[6], "vgui/hud/bossbar/006", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[7], "vgui/hud/bossbar/007", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[8], "vgui/hud/bossbar/008", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[9], "vgui/hud/bossbar/009", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[10], "vgui/hud/bossbar/010", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[11], "vgui/hud/bossbar/011", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[12], "vgui/hud/bossbar/012", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[13], "vgui/hud/bossbar/013", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[14], "vgui/hud/bossbar/014", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[15], "vgui/hud/bossbar/015", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[16], "vgui/hud/bossbar/016", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[17], "vgui/hud/bossbar/017", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[18], "vgui/hud/bossbar/018", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[19], "vgui/hud/bossbar/019", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[20], "vgui/hud/bossbar/020", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[21], "vgui/hud/bossbar/021", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[22], "vgui/hud/bossbar/022", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[23], "vgui/hud/bossbar/023", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[24], "vgui/hud/bossbar/024", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[25], "vgui/hud/bossbar/025", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[26], "vgui/hud/bossbar/026", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[27], "vgui/hud/bossbar/027", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[28], "vgui/hud/bossbar/028", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[29], "vgui/hud/bossbar/029", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[30], "vgui/hud/bossbar/030", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[31], "vgui/hud/bossbar/031", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[32], "vgui/hud/bossbar/032", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[33], "vgui/hud/bossbar/033", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[34], "vgui/hud/bossbar/034", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[35], "vgui/hud/bossbar/035", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[36], "vgui/hud/bossbar/036", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[37], "vgui/hud/bossbar/037", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[38], "vgui/hud/bossbar/038", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[39], "vgui/hud/bossbar/039", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[40], "vgui/hud/bossbar/040", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[41], "vgui/hud/bossbar/041", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[42], "vgui/hud/bossbar/042", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[43], "vgui/hud/bossbar/043", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[44], "vgui/hud/bossbar/044", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[45], "vgui/hud/bossbar/045", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[46], "vgui/hud/bossbar/046", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[47], "vgui/hud/bossbar/047", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[48], "vgui/hud/bossbar/048", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[49], "vgui/hud/bossbar/049", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[50], "vgui/hud/bossbar/050", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[51], "vgui/hud/bossbar/051", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[52], "vgui/hud/bossbar/052", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[53], "vgui/hud/bossbar/053", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[54], "vgui/hud/bossbar/054", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[55], "vgui/hud/bossbar/055", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[56], "vgui/hud/bossbar/056", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[57], "vgui/hud/bossbar/057", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[58], "vgui/hud/bossbar/058", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[59], "vgui/hud/bossbar/059", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[60], "vgui/hud/bossbar/060", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[61], "vgui/hud/bossbar/061", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[62], "vgui/hud/bossbar/062", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[63], "vgui/hud/bossbar/063", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[64], "vgui/hud/bossbar/064", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[65], "vgui/hud/bossbar/065", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[66], "vgui/hud/bossbar/066", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[67], "vgui/hud/bossbar/067", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[68], "vgui/hud/bossbar/068", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[69], "vgui/hud/bossbar/069", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[70], "vgui/hud/bossbar/070", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[71], "vgui/hud/bossbar/071", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[72], "vgui/hud/bossbar/072", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[73], "vgui/hud/bossbar/073", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[74], "vgui/hud/bossbar/074", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[75], "vgui/hud/bossbar/075", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[76], "vgui/hud/bossbar/076", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[77], "vgui/hud/bossbar/077", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[78], "vgui/hud/bossbar/078", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[79], "vgui/hud/bossbar/079", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[80], "vgui/hud/bossbar/080", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[81], "vgui/hud/bossbar/081", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[82], "vgui/hud/bossbar/082", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[83], "vgui/hud/bossbar/083", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[84], "vgui/hud/bossbar/084", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[85], "vgui/hud/bossbar/085", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[86], "vgui/hud/bossbar/086", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[87], "vgui/hud/bossbar/087", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[88], "vgui/hud/bossbar/088", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[89], "vgui/hud/bossbar/089", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[90], "vgui/hud/bossbar/090", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[91], "vgui/hud/bossbar/091", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[92], "vgui/hud/bossbar/092", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[93], "vgui/hud/bossbar/093", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[94], "vgui/hud/bossbar/094", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[95], "vgui/hud/bossbar/095", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[96], "vgui/hud/bossbar/096", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[97], "vgui/hud/bossbar/097", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[98], "vgui/hud/bossbar/098", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[99], "vgui/hud/bossbar/099", true, false);
	surface()->DrawSetTextureFile(m_nTexture_Bar[100], "vgui/hud/bossbar/100", true, false);

	SetHiddenBits(HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD );
}

//------------------------------------------------------------------------
// Purpose: Init reset func duh...
//------------------------------------------------------------------------
void CHudBossBar::Init()
{
	HOOK_HUD_MESSAGE(CHudBossBar, BossData);
	Reset();
}

//------------------------------------------------------------------------
// Purpose: Default on spawn, keep alpha to 0!
//-----------------------------------------------------------------------
void CHudBossBar::Reset(void)
{
	m_bShouldShow = false;
	BossHealth = 1;
	DivideFactor = 1;

	SetPaintEnabled(true);
	SetPaintBackgroundEnabled(true);
	SetBgColor(Color(0, 0, 0, 0));
	SetFgColor(Color(255, 255, 255, 255));
	SetAlpha(0);
}

//------------------------------------------------------------------------
// Purpose: Check if we need to display this HUD:
//------------------------------------------------------------------------
void CHudBossBar::OnThink(void)
{
	C_BaseHLPlayer *pPlayer = (C_BaseHLPlayer *)C_BasePlayer::GetLocalPlayer();
	if (!pPlayer || !engine->IsInGame() || engine->IsLevelMainMenuBackground())
	{
		m_bShouldShow = false;
		g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "alpha", 0.0f, 0.0f, 0.4f, AnimationController::INTERPOLATOR_LINEAR);
		return;
	}

	if (m_bShouldShow && pPlayer->IsAlive())
		g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "alpha", 256.0f, 0.0f, 0.4f, AnimationController::INTERPOLATOR_LINEAR);
	else
	{
		m_bShouldShow = false;
		g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "alpha", 0.0f, 0.0f, 0.4f, AnimationController::INTERPOLATOR_LINEAR);
		return;
	}
}

//------------------------------------------------------------------------
// Purpose: Paint func
//------------------------------------------------------------------------
void CHudBossBar::Paint()
{
	int iOffset = (BossHealth > 0) ? (BossHealth / DivideFactor) : 0;

	surface()->DrawSetColor(GetFgColor());
	surface()->DrawSetTexture(m_nTexture_Bar[iOffset]);
	surface()->DrawTexturedRect(m_flBarInsetX, m_flBarInsetY, m_flBarWidth + m_flBarInsetX, m_flBarHeight + m_flBarInsetY);

	surface()->DrawSetColor(GetFgColor());
	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextColor(m_TextColor);

	// Set Lenght/Size
	int positionX = m_flBarInsetX + ((m_flBarWidth / 2) - (UTIL_ComputeStringWidth(m_hTextFont, szEntName) / 2));
	vgui::surface()->DrawSetTextPos(positionX, text_ypos);
	surface()->DrawUnicodeString(szEntName);
}

void CHudBossBar::PaintBackground()
{
	SetBgColor(Color(0, 0, 0, 0));
	SetPaintBorderEnabled(false);
	BaseClass::PaintBackground();
}

// Get packets from the server:
void CHudBossBar::MsgFunc_BossData(bf_read &msg)
{
	int iShouldShow = (int)msg.ReadByte();
	int iCurrHP = (int)msg.ReadFloat();
	int iMaxHP = (int)msg.ReadFloat();

	// Read the string(s)
	char szString[64];
	msg.ReadString(szString, 64);
	g_pVGuiLocalize->ConvertANSIToUnicode(szString, szEntName, 64);

	m_bShouldShow = (iShouldShow >= 1) ? true : false;
	BossHealth = iCurrHP;

	if (iMaxHP > 0)
		DivideFactor = ((iMaxHP / 100) > 1) ? (iMaxHP / 100) : 1;
	else
		DivideFactor = 1;
}