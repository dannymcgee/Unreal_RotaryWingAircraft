#pragma once

#include "CoreMinimal.h"
#include "CubicBezier.generated.h"


USTRUCT()
struct FCubicBezier {
	GENERATED_BODY()

public:
	using Point = FVector2f;

	UPROPERTY(VisibleAnywhere) FVector2f P0 { 0, 0 };
	UPROPERTY(VisibleAnywhere) FVector2f P1 { 0, 0 };
	UPROPERTY(VisibleAnywhere) FVector2f P2 { 1, 1 };
	UPROPERTY(VisibleAnywhere) FVector2f P3 { 1, 1 };

	FCubicBezier() = default;
	FCubicBezier(Point const& p0, Point const& p1, Point const& p2, Point const& p3)
		: P0(p0), P1(p1), P2(p2), P3(p3)
	{}

	FORCEINLINE static auto IsNearlyEqual(FCubicBezier const& lhs, FCubicBezier const& rhs) -> bool
	{
		return FMath::IsNearlyEqual(lhs.P0.X, rhs.P0.X) && FMath::IsNearlyEqual(lhs.P0.Y, rhs.P0.Y)
			&& FMath::IsNearlyEqual(lhs.P1.X, rhs.P1.X) && FMath::IsNearlyEqual(lhs.P1.Y, rhs.P1.Y)
			&& FMath::IsNearlyEqual(lhs.P2.X, rhs.P2.X) && FMath::IsNearlyEqual(lhs.P2.Y, rhs.P2.Y)
			&& FMath::IsNearlyEqual(lhs.P3.X, rhs.P3.X) && FMath::IsNearlyEqual(lhs.P3.Y, rhs.P3.Y);
	}

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

	/** Evaluate the curve for the given `t` value. */
	FORCEINLINE auto Get(float t) const -> Point
	{
		auto a = FMath::Lerp(P0,P1,t);
		auto b = FMath::Lerp(P1,P2,t);
		auto c = FMath::Lerp(P2,P3,t);
		auto d = FMath::Lerp(a,b,t);
		auto e = FMath::Lerp(b,c,t);
		return FMath::Lerp(d,e,t);
	}
};
