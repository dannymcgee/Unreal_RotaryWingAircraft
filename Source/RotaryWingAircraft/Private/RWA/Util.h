#pragma once

#include "CoreMinimal.h"

struct FInputActionValue;
struct FCubicBezier;


namespace RWA::Util {

template <typename T>
FORCEINLINE T InverseLerp(T value, T min, T max)
{
	return FMath::Clamp((value - min) / (max - min), 0, 1);
}

MIX_FLOATS_3_ARGS(InverseLerp);

/** Fit a [0, 1] alpha value to a sine curve */
template <typename T>
FORCEINLINE T CurveSin(T alpha)
{
	if (alpha < 0.5) return -1.0 * FMath::Cos(alpha * UE_HALF_PI) + 1.0;
	return FMath::Sin(alpha * UE_HALF_PI);
}

FString ToString(FInputActionValue const& input);
FString ToString(FCubicBezier const& curve);

}
