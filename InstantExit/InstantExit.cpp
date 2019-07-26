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
    cvarManager->registerCvar(enabledCvarName, "1", "Determines whether the InstantExit plugin is enabled or disabled.").addOnValueChanged(std::bind(&InstantExit::pluginEnabledChanged, this));
    cvarManager->registerCvar(delayCvarName, "0", "Wait X amount of seconds before exiting");
    cvarManager->registerCvar(bypassCasualCvarName, "0", "Don't automatically exit when ending a casual game.");
    cvarManager->registerCvar(launchFreeplayCvarName, "0", "Launch freeplay on exit");
    cvarManager->registerCvar(trainingMapCvarName, "EuroStadium_Night_P", "Determines the map Instant Training mod will launch on match end.");

    hookMatchEnded();
}

void InstantExit::onUnload()
{
}

void InstantExit::exitGame() const
{
    cvarManager->executeCommand("unreal_command disconnect");
}

void InstantExit::launchTraining() const
{
    std::stringstream launchTrainingCommandBuilder;
    std::string mapname = cvarManager->getCvar(trainingMapCvarName).getStringValue();
    if (mapname.compare("random") == 0)
    {
        mapname = gameWrapper->GetRandomMap();
    }
    launchTrainingCommandBuilder << "start " << mapname << "?Game=TAGame.GameInfo_Tutorial_TA?GameTags=Freeplay";

    const std::string launchTrainingCommand = launchTrainingCommandBuilder.str();
    gameWrapper->ExecuteUnrealCommand(launchTrainingCommand);
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

void InstantExit::onMatchEnd(ServerWrapper server, void * params, string eventName)
{
    // Are we bypassing auto exit for casual games?
    bool bypassCasual = cvarManager->getCvar(bypassCasualCvarName).getBoolValue();
    if (!server.IsNull() && bypassCasual)
    {
        auto playlist = (Mode)server.GetPlaylist().GetPlaylistId();

        if (playlist == CasualChaos ||
            playlist == CasualDoubles ||
            playlist == CasualDuel ||
            playlist == CasualStandard)
        {
            return;
        }
    }

    // Calculate Delay time before exit
    float exitDelayTime = cvarManager->getCvar(delayCvarName).getFloatValue();
    bool autoGG = cvarManager->getCvar("ranked_autogg").getBoolValue();
    if (autoGG)
    {
        float autoGGDelayTime = cvarManager->getCvar("ranked_autogg_delay").getFloatValue() / 1000;
        exitDelayTime += autoGGDelayTime;
    }

    // Where are we exiting too? Freeplay or Main Menu?
    bool launchFreeplay = cvarManager->getCvar(launchFreeplayCvarName).getBoolValue();
    if (launchFreeplay)
    {
        gameWrapper->SetTimeout(std::bind(&InstantExit::launchTraining, this), exitDelayTime);
    }
    else
    {
        gameWrapper->SetTimeout(std::bind(&InstantExit::exitGame, this), exitDelayTime);
    }
}

void InstantExit::hookMatchEnded()
{
    gameWrapper->HookEventWithCaller<ServerWrapper>(
        "Function TAGame.GameEvent_Soccar_TA.EventMatchEnded",
        std::bind(
            &InstantExit::onMatchEnd,
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