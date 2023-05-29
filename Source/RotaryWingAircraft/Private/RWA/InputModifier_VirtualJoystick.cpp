#include "RWA\InputModifier_VirtualJoystick.h"

#include "EnhancedPlayerInput.h"

#define Self UInputModifier_RWA_VirtualJoystick


auto Self::ModifyRaw_Implementation(
	UEnhancedPlayerInput const* input,
	FInputActionValue value,
	float deltaTime)
	-> FInputActionValue
{
	if (m_NeedsInit)
		SetupCurves();

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

	m_PrevInput = value;

	return result;
}


void Self::SetupCurves()
{
	auto setupCurve = [](FCubicBezier& curve, float scaleX, float slope, float easing, float balance)
	{
		auto p2_wgt1 = FVector2f{ (1.f - slope) * scaleX, 1 };
		auto p3 = FVector2f{ p2_wgt1.X + (easing * scaleX), 1 };

		auto p2_wgt0 = FMath::Lerp({ 0, 0 }, p2_wgt1, 0.5f);
		p2_wgt0 = FMath::Lerp(p2_wgt0, { 0, 1 }, easing * 0.5f);

		auto p2 = FMath::Lerp(p2_wgt0, p2_wgt1, balance);

		curve.P0 = { 0, 0 };
		curve.P1 = { 0, 0 };
		curve.P2 = p2;
		curve.P3 = p3;
	};

	setupCurve(m_CurveIn, Resistance, Impulse, SpringStifness, SpringBalance);
	setupCurve(m_CurveOut, Resistance, SpringStifness, Damping, 1.f - SpringBalance);

	m_ActiveCurve = &m_CurveIn;
	m_NeedsInit = false;
}

auto Self::ModifyRaw(float value, float deltaTime) -> FInputActionValue
{
	float prev = m_PrevInput.Get<float>();
	float delta = value - prev;

	// Have we changed direction this tick?
	if (FMath::Abs(delta) > 0.5f) {
		m_Phase = FMath::IsNearlyZero(value) ? Falling : Rising;

		if (m_PrevCurve) {
			Swap(m_PrevCurve, m_ActiveCurve);
			m_PrevX = m_ActiveCurve->XForY(1.f - m_PrevY);
		}
		else {
			m_PrevCurve = m_ActiveCurve;
			m_ActiveCurve = m_Phase == Falling ? &m_CurveOut : &m_CurveIn;
		}
	}

	if (m_Phase == None)
		return value;

	float x = FMath::Min(
		m_ActiveCurve->Duration(),
		m_PrevX + (deltaTime * m_ActiveCurve->Duration()));

	float result = m_ActiveCurve->YForX(x);

	m_PrevX = x;
	m_PrevY = result;

	if (FMath::IsNearlyEqual(result, value)) {
		m_Phase = None;

		return result;
	}

	if (m_Phase == Falling)
		return 1 - result;

	return result;
}

auto Self::ModifyRaw(FVector2D value, float deltaTime) const -> FInputActionValue
{
	if (value.IsNearlyZero())
		return { value };

	auto prev = m_PrevInput.Get<FVector2D>();
	auto result = FMath::Vector2DInterpTo(prev, value, deltaTime, SpringStifness);
	return { result };
}

auto Self::ModifyRaw(FVector value, float deltaTime) const -> FInputActionValue
{
	if (value.IsNearlyZero())
		return { value };

	auto prev = m_PrevInput.Get<FVector>();
	auto result = FMath::VInterpTo(prev, value, deltaTime, SpringStifness);
	return { result };
}


#undef Self
