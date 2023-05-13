#pragma once

#include "CoreMinimal.h"

namespace RWA::Util {

template <typename T>
FORCEINLINE auto InverseLerp(T value, T min, T max) -> T
{
	return FMath::Clamp((value - min) / (max - min), 0, 1);
}

MIX_FLOATS_3_ARGS(InverseLerp);

/** Fit a [0, 1] alpha value to a sine curve */
template <typename T>
FORCEINLINE auto CurveSin(T alpha) -> T
{
	if (alpha < 0.5) return -1.0 * FMath::Cos(alpha * UE_HALF_PI) + 1.0;
	return FMath::Sin(alpha * UE_HALF_PI);
}

}
