#include "RWA/Input/InputTrigger_DecayedRelease.h"

#include "RWA/Util.h"


UInputTrigger_RWA_DecayedRelease::UInputTrigger_RWA_DecayedRelease(FObjectInitializer const& init)
	: Super(init)
{
	ActuationThreshold = 0.f;
	bShouldAlwaysTick = true;
}

ETriggerEventsSupported UInputTrigger_RWA_DecayedRelease::GetSupportedTriggerEvents() const 
{
	// Includes ETriggerEvents of Started, Ongoing, Canceled, and Triggered
	return ETriggerEventsSupported::Ongoing;
}

FString UInputTrigger_RWA_DecayedRelease::GetDebugState() const
{
	return RWA::Util::ToString(LastValue);
}

ETriggerType UInputTrigger_RWA_DecayedRelease::GetTriggerType_Implementation() const 
{
	return ETriggerType::Implicit;
}

ETriggerState UInputTrigger_RWA_DecayedRelease::UpdateState_Implementation(
	UEnhancedPlayerInput const* cmp,
	FInputActionValue input,
	float dt)
{
	bool isDormant = FMath::IsNearlyZero(LastValue.GetMagnitudeSq())
		&& FMath::IsNearlyZero(input.GetMagnitudeSq());

	return isDormant ? ETriggerState::None : ETriggerState::Triggered;
}
