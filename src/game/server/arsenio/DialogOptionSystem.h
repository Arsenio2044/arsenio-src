#ifndef DIALOGOPTIONSYSTEM_H
#define DIALOGOPTIONSYSTEM_H

#include "baseentity.h"
#include "DialogOptionPanel.h"

class CDialogOptionSystem : public CBaseEntity
{
    DECLARE_CLASS(CDialogOptionSystem, CBaseEntity);

public:
    void Spawn() override;
    void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
    bool SetOptionsFromFile(const char* optionsFileName);

private:
    CDialogOptionPanel* m_pOptionPanel; // Add the m_pOptionPanel member variable
    const char* m_Options[10];
    int m_NumOptions;
    string_t m_OnOptionSelectedOutput;
};

#endif // DIALOGOPTIONSYSTEM_H
