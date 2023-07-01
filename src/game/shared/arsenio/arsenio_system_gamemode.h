#pragma once

#include "shareddefs.h"

#ifdef CLIENT_DLL
#define CArsenioPlayer C_ArsenioPlayer
#endif

class CArsenioPlayer;

enum class GameModeHUDCapability_t
{
    CAP_HUD_SYNC = 0,
    CAP_HUD_SYNC_BAR,
    CAP_HUD_KEYPRESS_STRAFES,
    CAP_HUD_KEYPRESS_JUMPS,
    CAP_HUD_KEYPRESS_ATTACK,
    CAP_HUD_KEYPRESS_WALK,
    CAP_HUD_KEYPRESS_SPRINT,
};

class IGameMode
{
public:
    virtual GameMode_t  GetType() = 0;
   // virtual const char* GetStatusString() = 0;
   // virtual const char* GetDiscordIcon() = 0;
    virtual const char* GetMapPrefix() = 0;
    virtual const char* GetGameModeCfg() = 0;


    virtual void        OnPlayerSpawn(CArsenioPlayer *pPlayer) = 0;
    virtual void        ExecGameModeCfg() = 0;



    virtual ~IGameMode() {}
};

// Unknown ("default") game mode
class CGameModeBase : public IGameMode
{
public:
    GameMode_t GetType() override { return GAMEMODE_UNKNOWN; }
   // const char* GetStatusString() override { return "Playing"; }
  //  const char* GetDiscordIcon() override { return "mom"; }
    const char* GetMapPrefix() override { return "ar_"; }
    const char* GetGameModeCfg() override { return nullptr; }

    void OnPlayerSpawn(CArsenioPlayer *pPlayer) override;
    void ExecGameModeCfg() override;


};




class CGameModeSystem : public CAutoGameSystem
{
public:
    CGameModeSystem();
    ~CGameModeSystem();


    // Extra methods
    /// Gets current game mode
    IGameMode *GetGameMode() const { return m_pCurrentGameMode; }
    /// Gets a specific game mode instance by type
    IGameMode *GetGameMode(int eMode) const;
    /// Gets a game mode based off of the map name
    IGameMode *GetGameModeFromMapName(const char *pMapName);
    /// Checks if the game mode is the given one.
    /// (convenience method; functionally equivalent to `GetGameMode()->GetType() == eCheck`)
    bool GameModeIs(GameMode_t eCheck) const { return m_pCurrentGameMode->GetType() == eCheck; }
    /// Sets the game mode directly
    void SetGameMode(GameMode_t eMode);
    /// Sets the game mode from a map name (backup method)
    void SetGameModeFromMapName(const char *pMapName);
    /// Prints out the game mode's vars
    void PrintGameModeVars();

private:
    IGameMode *m_pCurrentGameMode;
    CUtlVector<IGameMode*> m_vecGameModes;
};

extern CGameModeSystem *g_pGameModeSystem;