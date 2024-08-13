#include "RWA/Input/InputModifier_VirtualJoystick.h"

#include "EnhancedPlayerInput.h"


FInputActionValue UInputModifier_RWA_VirtualJoystick::ModifyRaw_Implementation(
	UEnhancedPlayerInput const* input,
	FInputActionValue value,
	float deltaTime)
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

#if WITH_EDITOR
void UInputModifier_RWA_VirtualJoystick::PostEditChangeProperty(FPropertyChangedEvent& event)
{
	Super::PostEditChangeProperty(event);
	SetupCurves();
}
#endif

void UInputModifier_RWA_VirtualJoystick::PostLoad()
{
	Super::PostLoad();
	SetupCurves();
}


void UInputModifier_RWA_VirtualJoystick::SetupCurves()
{
	auto setupCurve = [](
		FCubicBezier& curve,
		float scaleX, float slope, float easing, float balance)
	{
		FVector2f p2_wgt1 { (1.f - slope) * scaleX, 1 };
		FVector2f p3 { p2_wgt1.X + (easing * scaleX), 1 };

		FVector2f p2_wgt0 = FMath::Lerp({ 0, 0 }, p2_wgt1, 0.5f);
		p2_wgt0 = FMath::Lerp(p2_wgt0, { 0, 1 }, easing * 0.5f);

		FVector2f p2 = FMath::Lerp(p2_wgt0, p2_wgt1, balance);

		curve.P0 = { 0, 0 };
		curve.P1 = { 0, 0 };
		curve.P2 = p2;
		curve.P3 = p3;
	};

	setupCurve(m_CurveIn, Resistance, Attack, SpringTension, SpringBalance);
	setupCurve(m_CurveOut, Resistance, SpringTension, Damping, 1.f - SpringBalance);

	m_ActiveCurve = &m_CurveIn;
	m_NeedsInit = false;
}

FInputActionValue UInputModifier_RWA_VirtualJoystick::ModifyRaw(float value, float deltaTime) 
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
		return FInputActionValue(value);

	float x = FMath::Min(
		m_ActiveCurve->Duration(),
		m_PrevX + (deltaTime * m_ActiveCurve->Duration()));

	float result = m_ActiveCurve->YForX(x);

	m_PrevX = x;
	m_PrevY = result;

	if (FMath::IsNearlyEqual(result, value)) {
		m_Phase = None;

		return FInputActionValue(result);
	}

	if (m_Phase == Falling)
		return FInputActionValue(1.f - result);

	return FInputActionValue(result);
}

FInputActionValue UInputModifier_RWA_VirtualJoystick::ModifyRaw(
	FVector2D value,
	float deltaTime)
	const 
{
	if (value.IsNearlyZero())
		return FInputActionValue(value);

	FVector2D prev = m_PrevInput.Get<FVector2D>();
	FVector2D result = FMath::Vector2DInterpTo(prev, value, deltaTime, SpringTension);
	return FInputActionValue(result);
}

FInputActionValue UInputModifier_RWA_VirtualJoystick::ModifyRaw(
	FVector value,
	float deltaTime)
	const 
{
	if (value.IsNearlyZero())
		return FInputActionValue(value);

	FVector prev = m_PrevInput.Get<FVector>();
	FVector result = FMath::VInterpTo(prev, value, deltaTime, SpringTension);
	return FInputActionValue(result);
}
