#include "RotaryWingAircraftEditor.h"

#include "RWA/CubicBezierCustomization.h"

#define Self FRotaryWingAircraftEditorModule


void Self::StartupModule()
{
	using Callback = FOnGetPropertyTypeCustomizationInstance;

	auto& pem = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	pem.RegisterCustomPropertyTypeLayout(
		"CubicBezier", Callback::CreateStatic(&FCubicBezierStructCustomization::MakeInstance));
}

void Self::ShutdownModule()
{}


#undef Self

IMPLEMENT_MODULE(FRotaryWingAircraftEditorModule, RotaryWingAircraftEditor)
