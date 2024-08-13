#pragma once

#include "CoreMinimal.h"
#include "InputTriggers.h"
#include "InputTrigger_DecayedRelease.generated.h"


/**
 * Allows an input axis to gradually "decay" back to 0 when released, by
 * continuing to emit a "Triggered" state until the transformed value (after
 * modifier processing) has reached 0.
 */
UCLASS(NotBlueprintable, MinimalAPI, meta=(DisplayName="RWA Decayed Release"))
class UInputTrigger_RWA_DecayedRelease
	: public UInputTrigger
{
	GENERATED_BODY()

public:
	UInputTrigger_RWA_DecayedRelease(FObjectInitializer const& init);

	ETriggerEventsSupported GetSupportedTriggerEvents() const override;
	FString GetDebugState() const override;

protected:
	ETriggerType GetTriggerType_Implementation() const override;

	ETriggerState UpdateState_Implementation(
		UEnhancedPlayerInput const* cmp,
		FInputActionValue input,
		float dt)
		override;
};
