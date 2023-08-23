#pragma once

#include "shareddefs.h"

struct Tips;

class CTipManager
{
public:
    CTipManager();

    const char *GetTipForGamemode(GameMode_t gameMode);

private:
    CUtlVector<Tips *> m_vecTips;
};