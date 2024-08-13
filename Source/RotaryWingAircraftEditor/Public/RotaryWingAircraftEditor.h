#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"


class FRotaryWingAircraftEditorModule : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;
};
