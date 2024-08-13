#include "RWA/Input/InputModifier_VirtualThrottle.h"


TMap<FName,FInputActionValue> UInputModifier_RWA_VirtualThrottle::s_AxisValues {};

FInputActionValue UInputModifier_RWA_VirtualThrottle::ModifyRaw_Implementation(
	UEnhancedPlayerInput const* input,
	FInputActionValue value,
	float deltaTime)
{
	if (!ensureMsgf(AxisID != EName::None,
		TEXT("RWA Virtual Throttle modifier requires an Axis ID to function properly!")))
	{
		return 0;
	}

	FInputActionValue& current = s_AxisValues.FindOrAdd(AxisID, {});

	if (value.IsNonZero())
	{
		FInputActionValue next = current + (value * Sensitivity * deltaTime);

		// Clamp to a max-length of 1
		float mag2 = next.GetMagnitudeSq();
		if (mag2 > 1.f)
			next *= (1.f / FMath::Sqrt(mag2));

		current = next;

		return next;
	}
	else if (current.IsNonZero()
		&& Detent > 0.f
		&& current.GetValueType() == EInputActionValueType::Axis1D)
	{
		float currentValue = current.Get<float>();
		if (FMath::Abs(currentValue) <= Detent)
		{
			current = currentValue + (-FMath::Sign(currentValue) * Sensitivity * deltaTime * 0.25f);

			return current;
		}
	}

	return current;
}
