#include "RWA/InputModifier_Dampen.h"

#include "EnhancedPlayerInput.h"

#define Self UInputModifier_Dampen


auto Self::ModifyRaw_Implementation(
	const UEnhancedPlayerInput* input,
	FInputActionValue value,
	float deltaTime
) -> FInputActionValue {
	FInputActionValue result;

	switch (value.GetValueType()) {
		case EInputActionValueType::Boolean:
			return value;

		case EInputActionValueType::Axis1D:
			result = ModifyRaw(value.Get<float>(), deltaTime);
			break;

		case EInputActionValueType::Axis2D:
			result = ModifyRaw(value.Get<FVector2D>(), deltaTime);
			break;

		case EInputActionValueType::Axis3D:
			result = ModifyRaw(value.Get<FVector>(), deltaTime);
			break;
	}

	_LastValue = result;

	return result;
}

// TODO: Figure out how to ease back to zero values

auto Self::ModifyRaw(float value, float deltaTime) const -> FInputActionValue {
	if (FMath::IsNearlyZero(value))
		return { 0.f };

	auto prev = _LastValue.Get<float>();
	auto result = FMath::FInterpTo(prev, value, deltaTime, LerpSpeed);
	return { result };
}

auto Self::ModifyRaw(FVector2D value, float deltaTime) const -> FInputActionValue {
	if (value.IsNearlyZero())
		return { value };

	auto prev = _LastValue.Get<FVector2D>();
	auto result = FMath::Vector2DInterpTo(prev, value, deltaTime, LerpSpeed);
	return { result };
}

auto Self::ModifyRaw(FVector value, float deltaTime) const -> FInputActionValue {
	if (value.IsNearlyZero())
		return { value };

	auto prev = _LastValue.Get<FVector>();
	auto result = FMath::VInterpTo(prev, value, deltaTime, LerpSpeed);
	return { result };
}


#undef Self
