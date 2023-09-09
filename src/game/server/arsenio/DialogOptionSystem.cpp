#include "cbase.h"
#include "DialogOptionSystem.h"
#include "DialogOptionPanel.h"
#include "filesystem.h"




LINK_ENTITY_TO_CLASS(dialog_option_system, CDialogOptionSystem);

CDialogOptionSystem::CDialogOptionSystem()
{
    m_pOptionPanel = nullptr; // Initialize the m_pOptionPanel pointer to nullptr
    m_OnOptionSelectedOutput = AllocPooledString("OnOptionSelected");
    m_NumOptions = 0;
}

void CDialogOptionSystem::Spawn()
{
    Precache();
    SetSolid(SOLID_BBOX);
    AddSolidFlags(FSOLID_NOT_SOLID);
    SetMoveType(MOVETYPE_NONE);

    m_OnOptionSelectedOutput = AllocPooledString("OnOptionSelected");
    m_NumOptions = 0;

}

bool CDialogOptionSystem::SetOptionsFromFile(const char* optionsFileName)
{
    KeyValues* kvOptions = new KeyValues("Options");
    if (!kvOptions->LoadFromFile(g_pFullFileSystem, optionsFileName, "MOD"))
    {
        DevMsg("Failed to load dialog options file: %s\n", optionsFileName);
        kvOptions->deleteThis();
        return false;
    }

    // Clear previous options
    m_NumOptions = 0;
    KeyValues* kvOption = kvOptions->GetFirstSubKey();
    while (kvOption && m_NumOptions < 10) // Limit the number of options to 10
    {
        const char* optionText = kvOption->GetString();
        if (optionText && optionText[0] != '\0')
        {
            m_Options[m_NumOptions] = AllocPooledString(optionText);
            m_NumOptions++;
        }

        kvOption = kvOption->GetNextKey();
    }

    kvOptions->deleteThis();
    return true;
}

void CDialogOptionSystem::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
    if (useType != USE_ON)
        return;

    if (!m_NumOptions)
    {
        DevMsg("No options defined for dialog_option_system entity!\n");
        return;
    }

    if (!m_pOptionPanel)
    {
        m_pOptionPanel = new CDialogOptionPanel(enginevgui->GetPanel(PANEL_GAMEUIDLL));
        m_pOptionPanel->SetVisible(false); // Hide initially
    }

    // Set the output to our custom handler function
    m_pOptionPanel->SetCommand(new KeyValues("OnOptionSelected", "optionIndex", new CUtlDelegate<void(int)>(OnOptionSelectedOutput, pActivator)));
}

// Remove the function from CDialogOptionSystem class
// void CDialogOptionSystem::OnOptionSelected(int optionIndex)
// {
//     // ...
// }

// Add a new global function to handle the output
void OnOptionSelectedOutput(CBaseEntity* pActivator, int optionIndex)
{
    // Handle the output here based on the selected optionIndex
    CDialogOptionSystem* pOptionSystem = dynamic_cast<CDialogOptionSystem*>(pActivator);
    if (pOptionSystem)
    {
        pOptionSystem->OnOptionSelected(optionIndex);
    }
}