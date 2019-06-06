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
	void pluginEnabledChanged();
	void delayCheck();
	void hookMatchEnded();
	void unhookMatchEnded();
	void logHookType(const char *const hookType) const;

private:
	static constexpr const char *matchEndedEvent = "Function TAGame.GameEvent_Soccar_TA.EventMatchEnded";
	static constexpr const char *enabledCvarName = "instant_exit_enabled";
	static constexpr const char *delayCvarName = "instant_exit_delay";

private:
	bool hooked = false;
};