#pragma once
#pragma comment(lib, "bakkesmod.lib")
#include "bakkesmod/plugin/bakkesmodplugin.h"

class InstantExit final : public BakkesMod::Plugin::BakkesModPlugin
{
public:
    void onLoad() override;
    void onUnload() override;

private:
    void exitGame() const;
    void launchTraining() const;
    void pluginEnabledChanged();
    void onMatchEnd(ServerWrapper caller, void * params, string eventName);
    void hookMatchEnded();
    void unhookMatchEnded();
    void logHookType(const char *const hookType) const;

private:
    static constexpr const char *matchEndedEvent = "Function TAGame.GameEvent_Soccar_TA.EventMatchEnded";
    static constexpr const char *enabledCvarName = "instant_exit_enabled";
    static constexpr const char *delayCvarName = "instant_exit_delay";
    static constexpr const char *trainingMapCvarName = "instant_training_map";
    static constexpr const char *bypassCasualCvarName = "instant_exit_bypass_casual";
    static constexpr const char *launchFreeplayCvarName = "instant_exit_launchfreeplay";

private:
    bool hooked = false;
};