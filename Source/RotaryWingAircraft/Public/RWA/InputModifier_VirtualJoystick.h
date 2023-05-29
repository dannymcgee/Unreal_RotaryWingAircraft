#pragma once

#include "CoreMinimal.h"
#include "InputModifiers.h"
#include "InputModifier_VirtualJoystick.generated.h"


USTRUCT()
struct FCubicBezier {
	GENERATED_BODY()
private:
	using Point = FVector2f;
public:
	Point P0 { 0, 0 };
	Point P1 { 0, 0 };
	Point P2 { 1, 1 };
	Point P3 { 1, 1 };

	FCubicBezier() = default;
	FCubicBezier(Point const& p0, Point const& p1, Point const& p2, Point const& p3)
		: P0(p0), P1(p1), P2(p2), P3(p3)
	{}

	FORCEINLINE auto Duration() const -> float
	{
		return P3.X - P0.X;
	}

	/**
	 * Binary-search for the point along the curve where x ~= the provided value,
	 * and return the corresponding y.
	 */
	FORCEINLINE auto YForX(float x) const -> float
	{
		int32 iterations = 1;
		float lower = 0.f;
		float upper = 1.f;
		float t = (upper + lower) / 2.f;

		auto result = Get(t);

		while (FMath::Abs(x - result.X) > 0.001f) {
			if (++iterations > 9999)
				break;

			if (x > result.X)
				lower = t;
			else
				upper = t;

			t = (upper + lower) / 2.f;
			result = Get(t);
		}

		return result.Y;
	}

	/**
	 * Binary-search for the point along the curve where y ~= the provided value,
	 * and return the corresponding x.
	 */
	FORCEINLINE auto XForY(float y) const -> float
	{
		int32 iterations = 1;
		float lower = 0.f;
		float upper = 1.f;
		float t = (upper + lower) / 2.f;

		auto result = Get(t);

		while (FMath::Abs(y - result.Y) > 0.001f) {
			if (++iterations > 9999)
				break;

			if (y > result.Y)
				lower = t;
			else
				upper = t;

			t = (upper + lower) / 2.f;
			result = Get(t);
		}

		return result.X;
	}

	FORCEINLINE auto Get(float t) const -> FVector2f
	{
		auto a = FMath::Lerp(P0,P1,t);
		auto b = FMath::Lerp(P1,P2,t);
		auto c = FMath::Lerp(P2,P3,t);
		auto d = FMath::Lerp(a,b,t);
		auto e = FMath::Lerp(b,c,t);
		return FMath::Lerp(d,e,t);
	}

	auto ToString() const -> FString
	{
		return FString::Printf(
			TEXT("P0[ %.3f, %.3f ], P1[ %.3f, %.3f ], P2[ %.3f, %.3f ], P3[ %.3f, %.3f ]"),
			P0.X, P0.Y, P1.X, P1.Y, P2.X, P2.Y, P3.X, P3.Y);
	}
};

/**
 * Modifies an analog axis controlled by digital button/key presses to allow
 * them to behave more like a physical analog input device, with configurable
 * stiffness and damping.
 */
UCLASS(NotBlueprintable, MinimalAPI, meta=(DisplayName="RWA Virtual Joystick"))
class UInputModifier_RWA_VirtualJoystick : public UInputModifier {
	GENERATED_BODY()

public:
	/**
	 * Uniformly scales the "time" axis of the input curve. With all other
	 * settings at 0, a scale of 1 represents a linear interpolation over the
	 * course of 1 second. A scale of 5 would increase that to 5 seconds, etc.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings", meta=(
		UIMin="1", ClampMin="1"))
	float Resistance = 1.f;
	
	/**
	 * How fast the value begins accelerating toward its target value, before
	 * accounting for spring stiffness. You can think of this like a sensitivity
	 * factor.
	 *
	 * With Impulse=1 and SpringStiffness=0, the input moves immediately to the
	 * target value, like a digital input.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings", meta=(
		UIMin="0", UIMax="1", ClampMin="0", ClampMax="1"))
	float Impulse = 0.75f;
	
	/**
	 * Emulates the stiffness of the joystick's "spring," providing greater
	 * resistance to outward movement the further it is from rest, and increasing
	 * the speed at which it returns to rest once the input is released.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings", meta=(
		UIMin="0", UIMax="1", ClampMin="0", ClampMax="1"))
	float SpringStifness = 0.75f;

	/**
	 * Weights the spring's stiffness to more prominently affect the inner or
	 * outer range of the axis.
	 *
	 * This is pretty difficult to clearly summarize, so I'll try to describe
	 * some of its effects:
	 *
	 * * A value of 0.5 will "smooth" the overall shape of the curve by
	 *   distributing the effect of the easing function over its entire length.
	 * * A value of 1 means that the initial slope of the curve is constant
	 *   (solely dependent on Impulse) and sharply decelerates to the target
	 *   value toward the end of the range.
	 * * A value of 0 will sharply increase the initial slope, but decelerate
	 *   much more gradually while easing out to the target value.
	 * * These modifications reshape the curve without having any impact on its
	 *   total duration or its total cumulative delta of acceleration.
	 * * The effects described above for 1 and 0 are exactly inverted when easing
	 *   back to the rest position.
	 * * The behavior of a _real_ joystick spring is probably most accurately
	 *   modeled by a value of 1, but I am not a physicist, so take that with a
	 *   grain of salt.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings", meta=(
		UIMin="0", UIMax="1", ClampMin="0", ClampMax="1"))
	float SpringBalance = 1.f;

	/**
	 * Provides a "cushion" that eases the rate of deceleration as the input
	 * springs back to rest after being released.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings", meta=(
		UIMin="0", UIMax="1", ClampMin="-1", ClampMax="1"))
	float Damping = 0.f;

protected:
	// UInputModifier interface
	auto ModifyRaw_Implementation(
		UEnhancedPlayerInput const* input,
		FInputActionValue value,
		float deltaTime)
		-> FInputActionValue override;

private:
	void SetupCurves();
	
	auto ModifyRaw(float value, float deltaTime) -> FInputActionValue;
	auto ModifyRaw(FVector2D value, float deltaTime) const -> FInputActionValue;
	auto ModifyRaw(FVector value, float deltaTime) const -> FInputActionValue;

	enum EPhase { None, Rising, Falling };

	FCubicBezier m_CurveIn;
	FCubicBezier m_CurveOut;

	FCubicBezier const* m_ActiveCurve = nullptr;
	FCubicBezier const* m_PrevCurve = nullptr;

	FInputActionValue m_PrevInput = 0;
	float m_PrevX = 0;
	float m_PrevY = 0;

	EPhase m_Phase = None;

	bool m_NeedsInit = true;
};
