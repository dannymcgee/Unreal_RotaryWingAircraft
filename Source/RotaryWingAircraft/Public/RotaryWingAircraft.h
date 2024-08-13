#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"


class FRotaryWingAircraftModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	void StartupModule() override {}
	void ShutdownModule() override {}
};
