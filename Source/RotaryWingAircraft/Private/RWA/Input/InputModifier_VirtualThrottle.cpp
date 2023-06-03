#include "RWA/Input/InputModifier_VirtualThrottle.h"

#define Self UInputModifier_RWA_VirtualThrottle


TMap<FName,FInputActionValue> Self::s_AxisValues {};

auto Self::ModifyRaw_Implementation(
	UEnhancedPlayerInput const* input,
	FInputActionValue value,
	float deltaTime)
	-> FInputActionValue
{
	if (!ensureMsgf(AxisID != EName::None,
		TEXT("RWA Virtual Throttle modifier requires an Axis ID to function properly!")))
	{
		return 0;
	}

	auto& current = s_AxisValues.FindOrAdd(AxisID, {});

	if (value.IsNonZero()) {
		auto next = current + (value * Sensitivity * deltaTime);

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
		auto currentValue = current.Get<float>();
		if (FMath::Abs(currentValue) <= Detent) {
			current = currentValue + (-FMath::Sign(currentValue) * Sensitivity * deltaTime * 0.25f);

			return current;
		}
	}

	return current;
}


#undef Self
