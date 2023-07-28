#include "cbase.h"
#include "DialogOptionPanel.h"
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>

using namespace vgui;

CDialogOptionPanel::CDialogOptionPanel(vgui::VPANEL parent) : BaseClass(nullptr, "DialogOptionPanel")
{
    SetParent(parent);
    SetScheme("SourceScheme");
    SetTitleBarVisible(false);
    SetMoveable(false);
    SetSizeable(false);
    SetProportional(true);
    SetKeyBoardInputEnabled(true);
    SetMouseInputEnabled(true);
    SetVisible(true);
    SetBgColor(Color(0, 0, 0, 150));

    m_pOptionsLabel = new Label(this, "OptionsLabel", "");
    m_SelectedOption = -1;
    m_NumOptions = 0;

    // Perform the screen dimension calculations here
    int screenWidth, screenHeight;
    surface()->GetScreenSize(screenWidth, screenHeight);
    SetPos(screenWidth * 0.4, screenHeight * 0.4);
    SetSize(screenWidth * 0.2, screenHeight * 0.2);
    SetVisible(false); // Hide initially
}

void CDialogOptionPanel::SetOptions(const char** options, int numOptions)
{
    if (numOptions <= 0 || !options)
    {
        DevMsg("Invalid options for dialog panel!\n");
        return;
    }

    m_NumOptions = numOptions;

    char fullText[2048];
    fullText[0] = '\0';

    for (int i = 0; i < numOptions; ++i)
    {
        Q_snprintf(fullText, sizeof(fullText), "%s\n", options[i]);
    }

    m_pOptionsLabel->SetText(fullText);
    InvalidateLayout();
}

void CDialogOptionPanel::OnOptionSelected(int optionIndex)
{
    m_SelectedOption = optionIndex;
    DevMsg("Selected Option: %d\n", optionIndex);
}

void CDialogOptionPanel::OnKeyCodePressed(KeyCode code)
{
    if (m_SelectedOption == -1)
    {
        m_SelectedOption = 0;
    }

    int oldOption = m_SelectedOption;

    switch (code)
    {
    case KEY_UP:
        m_SelectedOption = (m_SelectedOption - 1 + m_NumOptions) % m_NumOptions;
        break;
    case KEY_DOWN:
        m_SelectedOption = (m_SelectedOption + 1) % m_NumOptions;
        break;
    case KEY_ENTER:
        PostMessage(this, new KeyValues("OptionSelected", "optionIndex", m_SelectedOption));
        break;
    default:
        break;
    }

    if (oldOption != m_SelectedOption)
    {
        PostMessage(this, new KeyValues("OnOptionSelected", "optionIndex", m_SelectedOption));
    }
}
