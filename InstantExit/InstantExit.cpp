#include "InstantExit.h"
#include <sstream>

BAKKESMOD_PLUGIN(InstantExit, "Instant Exit", "1.0", PLUGINTYPE_FREEPLAY)

void InstantExit::onLoad()
{
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

void InstantExit::delayCheck()
{
	float exitDelayTime = cvarManager->getCvar(delayCvarName).getFloatValue();
	gameWrapper->SetTimeout(std::bind(&InstantExit::exitGame, this), exitDelayTime);
}

void InstantExit::hookMatchEnded()
{
	gameWrapper->HookEvent(matchEndedEvent, std::bind(&InstantExit::delayCheck, this));
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
