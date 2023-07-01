#include "cbase.h"

#include "arsenio_system_gamemode.h"
#include "movevars_shared.h"
#include "fmtstr.h"

#ifdef GAME_DLL
#include "momentum/tickset.h"
#endif

#include "tier0/memdbgon.h"

#ifdef GAME_DLL

CON_COMMAND(mom_print_gamemode_vars, "Prints out the currently set values for commands like sv_maxvelocity, airaccel, etc\n")
{
    g_pGameModeSystem->PrintGameModeVars();
}

extern ConVar sv_interval_per_tick;
static void OnGamemodeChanged(IConVar* var, const char* pOldValue, float fOldValue)
{
    ConVarRef gm(var);
    const auto gamemode = gm.GetInt();

    g_pGameModeSystem->SetGameMode((GameMode_t)gamemode);

    TickSet::SetTickrate(g_pGameModeSystem->GetGameMode()->GetIntervalPerTick());

    sv_interval_per_tick.SetValue(TickSet::GetTickrate());
}

static MAKE_CONVAR_C(mom_gamemode, "0", FCVAR_REPLICATED | FCVAR_NOT_CONNECTED | FCVAR_HIDDEN | FCVAR_CLIENTCMD_CAN_EXECUTE, "", 0, GAMEMODE_COUNT - 1, OnGamemodeChanged);

#endif

ConVar mom_gamemode_override( "mom_gamemode_override", "0", FCVAR_REPLICATED | FCVAR_NOT_CONNECTED, "Forces the gamemode to the given gamemode number, ignoring the map's specified gamemode.\n");





void CGameModeBase::OnPlayerSpawn(CArsenioPlayer* pPlayer)
{
#ifdef GAME_DLL
    pPlayer->SetAutoBhopEnabled(PlayerHasAutoBhop());
#endif
}

void CGameModeBase::ExecGameModeCfg()
{
#ifdef CLIENT_DLL
    if (GetGameModeCfg())
    {
        engine->ClientCmd_Unrestricted(CFmtStr("exec %s", GetGameModeCfg()));
    }
#endif
}


CGameModeSystem::CGameModeSystem() : CAutoGameSystem("CGameModeSystem")
{
    m_pCurrentGameMode = new CGameModeBase; // Unknown game mode
    m_vecGameModes.AddToTail(m_pCurrentGameMode);

}

CGameModeSystem::~CGameModeSystem()
{
    m_vecGameModes.PurgeAndDeleteElements();
}


IGameMode* CGameModeSystem::GetGameMode(int eMode) const
{
    if (eMode < GAMEMODE_UNKNOWN || eMode >= GAMEMODE_COUNT)
    {
        Warning("Attempted to get invalid game mode %i !\n", eMode);
        eMode = GAMEMODE_UNKNOWN;
    }

    return m_vecGameModes[eMode];
}

IGameMode* CGameModeSystem::GetGameModeFromMapName(const char* pMapName)
{
    if (!pMapName || !pMapName[0])
        return m_vecGameModes[GAMEMODE_UNKNOWN];

    // Skip over unknown in the loop
    for (auto i = 1; i < m_vecGameModes.Count(); ++i)
    {
        const auto pPrefix = m_vecGameModes[i]->GetMapPrefix();
        const auto strLen = Q_strlen(pPrefix);
        if (!Q_strnicmp(pPrefix, pMapName, strLen))
        {
            return m_vecGameModes[i];
        }
    }

    return m_vecGameModes[GAMEMODE_UNKNOWN];
}

void CGameModeSystem::SetGameMode(GameMode_t eMode)
{
    m_pCurrentGameMode = m_vecGameModes[eMode];
#ifdef CLIENT_DLL
    static ConVarRef mom_gamemode("mom_gamemode");
    // Throw the change to the server too
    mom_gamemode.SetValue(m_pCurrentGameMode->GetType());
#endif
}

void CGameModeSystem::SetGameModeFromMapName(const char* pMapName)
{
    // Set to unknown for now
    m_pCurrentGameMode = m_vecGameModes[GAMEMODE_UNKNOWN];

    const auto pFoundMode = GetGameModeFromMapName(pMapName);
    if (pFoundMode)
    {
        m_pCurrentGameMode = pFoundMode;
    }

#ifdef CLIENT_DLL
    static ConVarRef mom_gamemode("mom_gamemode");
    // Throw the change to the server too
    mom_gamemode.SetValue(m_pCurrentGameMode->GetType());
#endif
}

void CGameModeSystem::PrintGameModeVars()
{
    Msg("Set game mode ConVars:\n\n"
        "sv_maxvelocity: %i\n"
        "sv_airaccelerate: %i\n"
        "sv_accelerate: %i\n"
        "sv_maxspeed: %i\n"
        "sv_gravity: %i\n"
        "sv_friction: %i\n",
        sv_maxvelocity.GetInt(),
        sv_airaccelerate.GetInt(),
        sv_accelerate.GetInt(),
        sv_maxspeed.GetInt(),
        sv_gravity.GetInt(),
        sv_friction.GetInt());
}

// Expose to DLL
CGameModeSystem s_GameModeSys;
CGameModeSystem* g_pGameModeSystem = &s_GameModeSys;