#pragma once

#include "CoreMinimal.h"
#include "InputModifiers.h"

#include "InputModifier_VirtualThrottle.generated.h"


/**
 * Modifies an analog axis controlled by digital button/key presses to make it
 * behave like a slider that stays in place once positioned. Applying positive
 * input increases the value and applying negative input decreases it. Applying
 * zero input (
 */
UCLASS(NotBlueprintable, MinimalAPI, meta=(DisplayName="RWA Virtual Throttle"))
class UInputModifier_RWA_VirtualThrottle : public UInputModifier
{
	GENERATED_BODY()

public:
	/**
	 * Give this axis a unique identifier. This is necessary to prevent two
	 * Virtual Throttle modifiers that affect the same axis (e.g., one for
	 * throttling up and one for throttling down) from "fighting" each other.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings")
	FName AxisID = EName::None;

	/** Controls how quickly the axis values moves when applying input. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings", meta=(
		UIMin="0", UIMax="1", ClampMin="0.01", ClampMax="1"))
	float Sensitivity = 0.6f;

	/**
	 * Allows axis values that are close to zero to settle back to exactly zero
	 * once input is no longer being applied. This value represents the size of
	 * the virtual detent.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings", meta=(
		UIMin="0", UIMax="1", ClampMin="0", ClampMax="1"))
	float Detent = 0.05f;

protected:
	FInputActionValue ModifyRaw_Implementation(
		UEnhancedPlayerInput const* input,
		FInputActionValue value,
		float deltaTime)
		override;

private:
	static TMap<FName,FInputActionValue> s_AxisValues;
};
