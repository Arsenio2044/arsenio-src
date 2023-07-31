// HackingUIOverlay.cpp
#include "HackingUIOverlay.h"
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui_controls/Button.h>

CHackingUIOverlay::CHackingUIOverlay(vgui::Panel* parent, const char* panelName)
    : BaseClass(parent, panelName)
{
    SetVisible(false);
    SetSize(400, 100);
    SetPos(100, 100);
    SetPaintBorderEnabled(true);
    SetPaintBackgroundEnabled(true);
    SetBgColor(Color(0, 0, 0, 200));

    for (int i = 0; i < 4; i++)
    {
        m_hackingButtons[i] = new vgui::Button(this, "", "");
        m_hackingButtons[i]->SetCommand(new KeyValues("HackingOptionSelected", "option", ""));
        m_hackingButtons[i]->SetVisible(false);
    }
}

void CHackingUIOverlay::OnCommand(const char* command)
{
    // The player clicked one of the hacking options
    PostActionSignal(new KeyValues("HackingOptionSelected", "option", command));
    SetVisible(false);
}

void CHackingUIOverlay::ShowHackingOptions(const char* options)
{
    ClearOptions();

    // Parse the options string and create buttons
    char buffer[256];
    int optionCount = 0;
    const char* option = options;
    while (*option && optionCount < 4)
    {
        sscanf(option, "%[^\\n]s", buffer);
        option += strlen(buffer) + 1;

        if (strlen(buffer) > 0)
        {
            m_hackingButtons[optionCount]->SetText(buffer);
            m_hackingButtons[optionCount]->SetCommand(new KeyValues("HackingOptionSelected", "option", buffer));
            m_hackingButtons[optionCount]->SetVisible(true);
            optionCount++;
        }
    }

    // Calculate button positions
    int buttonWidth = GetWide() / optionCount;
    int xPos = 0;
    for (int i = 0; i < optionCount; i++)
    {
        m_hackingButtons[i]->SetSize(buttonWidth, GetTall());
        m_hackingButtons[i]->SetPos(xPos, 0);
        xPos += buttonWidth;
    }

    SetVisible(true);
}

void CHackingUIOverlay::ClearOptions()
{
    for (int i = 0; i < 4; i++)
    {
        m_hackingButtons[i]->SetVisible(false);
    }
}