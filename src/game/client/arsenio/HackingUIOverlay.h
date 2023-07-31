// HackingUIOverlay.h
#ifndef HACKINGUIOVERLAY_H
#define HACKINGUIOVERLAY_H

#include <vgui_controls/Panel.h>

class CHackingUIOverlay : public vgui::Panel
{
    DECLARE_CLASS_SIMPLE(CHackingUIOverlay, vgui::Panel);

public:
    CHackingUIOverlay(vgui::Panel* parent, const char* panelName);

    // Show the overlay with hacking options
    void ShowHackingOptions(const char* options);

    // Clear the overlay
    void ClearOptions();

protected:
    virtual void OnCommand(const char* command);

private:
    vgui::Button* m_hackingButtons[4]; // You can adjust the number of buttons as needed
};

#endif // HACKINGUIOVERLAY_H
