#include "RWA/InputTrigger_DecayedRelease.h"

#define Self UInputTrigger_RWA_DecayedRelease


Self::Self(FObjectInitializer const& init)
	: Super(init)
{
	ActuationThreshold = 0.f;
	bShouldAlwaysTick = true;
}

auto Self::GetSupportedTriggerEvents() const -> ETriggerEventsSupported
{
	// Includes ETriggerEvents of Started, Ongoing, Canceled, and Triggered
	return ETriggerEventsSupported::Ongoing;
}

auto Self::GetDebugState() const -> FString
{
	switch (LastValue.GetValueType()) {
		case EInputActionValueType::Boolean:
			return LastValue.Get<bool>() ? "ON" : "OFF";
		
		case EInputActionValueType::Axis1D:
			return FString::Printf(TEXT("%.3f"), LastValue.Get<float>());

		case EInputActionValueType::Axis2D: {
			auto value = LastValue.Get<FVector2D>();
			return FString::Printf(TEXT("( %.3f, %.3f )"), value.X, value.Y);
		}
		case EInputActionValueType::Axis3D: {
			auto value = LastValue.Get<FVector>();
			return FString::Printf(TEXT("( %.3f, %.3f, %.3f )"), value.X, value.Y, value.Z);
		}
	}
	return "";
}

auto Self::GetTriggerType_Implementation() const -> ETriggerType
{
	return ETriggerType::Implicit;
}

auto Self::UpdateState_Implementation(
	UEnhancedPlayerInput const* cmp,
	FInputActionValue input,
	float dt)
	-> ETriggerState
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage((uint64)this, 0, FColor::White, GetDebugState());
	
	return ETriggerState::Triggered;
}


#undef Self
