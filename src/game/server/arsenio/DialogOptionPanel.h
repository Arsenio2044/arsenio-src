#ifndef DIALOGOPTIONPANEL_H
#define DIALOGOPTIONPANEL_H

#include <vgui_controls/Frame.h>
#include <vgui_controls/Label.h>

class CDialogOptionPanel : public vgui::Frame
{
    DECLARE_CLASS_SIMPLE(CDialogOptionPanel, vgui::Frame);

public:
    CDialogOptionPanel(vgui::VPANEL parent);
    void SetOptions(const char** options, int numOptions);

private:
    vgui::Label* m_pOptionsLabel;
    int m_SelectedOption;
    int m_NumOptions;

    MESSAGE_FUNC_INT(OnOptionSelected, "OptionSelected", optionIndex);
    MESSAGE_FUNC_INT(OnKeyCodePressed, "OnKeyCodePressed", keyCode);
};

#endif // DIALOGOPTIONPANEL_H
