#include "RotaryWingAircraftEditor.h"

#include "RWA/CubicBezierCustomization.h"


void FRotaryWingAircraftEditorModule::StartupModule()
{
	using Callback = FOnGetPropertyTypeCustomizationInstance;

	auto& pem = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	pem.RegisterCustomPropertyTypeLayout(
		"CubicBezier", Callback::CreateStatic(&FCubicBezierStructCustomization::MakeInstance));
}

void FRotaryWingAircraftEditorModule::ShutdownModule()
{}


IMPLEMENT_MODULE(FRotaryWingAircraftEditorModule, RotaryWingAircraftEditor)
