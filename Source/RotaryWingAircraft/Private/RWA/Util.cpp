#include "RWA/Util.h"

#include "RWA/Input/CubicBezier.h"
#include "InputActionValue.h"


namespace RWA::Util {

FString ToString(FInputActionValue const& input) 
{
	switch (input.GetValueType()) {
		case EInputActionValueType::Boolean:
			return input.Get<bool>() ? "ON" : "OFF";

		case EInputActionValueType::Axis1D:
			return FString::Printf(TEXT("%.3f"), input.Get<float>());

		case EInputActionValueType::Axis2D: {
			auto value = input.Get<FVector2D>();
			return FString::Printf(TEXT("( %.3f, %.3f )"), value.X, value.Y);
		}
		case EInputActionValueType::Axis3D: {
			auto value = input.Get<FVector>();
			return FString::Printf(TEXT("( %.3f, %.3f, %.3f )"), value.X, value.Y, value.Z);
		}
	}
	return "";
}

FString ToString(FCubicBezier const& curve)
{
	return FString::Printf(
		TEXT("P0[ %.3f, %.3f ], P1[ %.3f, %.3f ], P2[ %.3f, %.3f ], P3[ %.3f, %.3f ]"),
		curve.P0.X, curve.P0.Y,
		curve.P1.X, curve.P1.Y,
		curve.P2.X, curve.P2.Y,
		curve.P3.X, curve.P3.Y);
}

}
