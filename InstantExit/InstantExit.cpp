#include "InstantExit.h"
#include <sstream>

BAKKESMOD_PLUGIN(InstantExit, "Instant Exit", "1.0", PLUGINTYPE_FREEPLAY)

enum Mode
{
    CasualDuel = 1,
    CasualDoubles = 2,
    CasualStandard = 3,
    CasualChaos = 4,
    Private = 6,
    RankedDuel = 10,
    RankedDoubles = 11,
    RankedSoloStandard = 12,
    RankedStandard = 13,
    MutatorMashup = 14,
    Tournament = 22,
    RankedHoops = 27,
    RankedRumble = 28,
    RankedDropshot = 29,
    RankedSnowday = 30
};

void InstantExit::onLoad()
{
    cvarManager->registerCvar(bypassCasual, "0", "Don't automatically exit when ending a casual game.");
    cvarManager->registerCvar(enabledCvarName, "1", "Determines whether the InstantExit plugin is enabled or disabled.").addOnValueChanged(std::bind(&InstantExit::pluginEnabledChanged, this));
    cvarManager->registerCvar(delayCvarName, "0", "Wait X amount of seconds before exiting");
    hookMatchEnded();
}

void InstantExit::onUnload()
{
}

void InstantExit::exitGame() const
{
    cvarManager->executeCommand("unreal_command disconnect");
}

void InstantExit::pluginEnabledChanged()
{
    const bool enabled = cvarManager->getCvar(enabledCvarName).getBoolValue();

    if (enabled)
    {
        if (!hooked)
        {
            hookMatchEnded();
        }
    }
    else
    {
        if (hooked)
        {
            unhookMatchEnded();
        }
    }
}

void InstantExit::delayCheck(ServerWrapper server, void * params, string eventName)
{
    auto playlist = (Mode)server.GetPlaylist().GetPlaylistId();

    bool bypass = cvarManager->getCvar(bypassCasual).getBoolValue();

    if (bypass && (playlist == CasualChaos ||
                   playlist == CasualDoubles ||
                   playlist == CasualDuel ||
                   playlist == CasualStandard))
    {
        return;
    }

    float exitDelayTime = cvarManager->getCvar(delayCvarName).getFloatValue();
    gameWrapper->SetTimeout(std::bind(&InstantExit::exitGame, this), exitDelayTime);
}

void InstantExit::hookMatchEnded()
{
    gameWrapper->HookEventWithCaller<ServerWrapper>(
        "Function TAGame.GameEvent_Soccar_TA.EventMatchEnded",
        std::bind(
            &InstantExit::delayCheck,
            this,
            placeholders::_1,
            placeholders::_2,
            placeholders::_3
        )
    );

    hooked = true;
    logHookType("Hooked");
}

void InstantExit::unhookMatchEnded()
{
    gameWrapper->UnhookEvent(matchEndedEvent);
    hooked = false;
    logHookType("Unhooked");
}

void InstantExit::logHookType(const char *const hookType) const
{
    std::stringstream logBuffer;
    logBuffer << hookType << " match ended event.";
    cvarManager->log(logBuffer.str());
}