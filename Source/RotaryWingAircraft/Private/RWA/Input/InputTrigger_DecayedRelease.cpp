#include "RWA/Input/InputTrigger_DecayedRelease.h"

#include "RWA/Util.h"

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
	return RWA::Util::ToString(LastValue);
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
	bool isDormant = FMath::IsNearlyZero(LastValue.GetMagnitudeSq())
		&& FMath::IsNearlyZero(input.GetMagnitudeSq());

	return isDormant ? ETriggerState::None : ETriggerState::Triggered;
}


#undef Self
