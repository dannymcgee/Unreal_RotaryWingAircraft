#include "RWA/Util.h"

#include "InputActionValue.h"


namespace RWA::Util {

auto ToString(FInputActionValue const& input) -> FString
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

}
