#include "RWA\InputModifier_VirtualJoystick.h"

#include "EnhancedPlayerInput.h"

#define Self UInputModifier_RWA_VirtualJoystick


// FTickableGameObject interface -----------------------------------------------

// TODO: Remove this interface implementation
void Self::Tick(float dt)
{
	// UE_LOG(LogTemp, Log, TEXT("Tick: %f"), dt);
}

auto Self::IsTickable() const -> bool
{
	return m_ReadyToTick;
}

auto Self::GetStatId() const -> TStatId
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UInputModifier_RWA_VirtualJoystick, STATGROUP_Tickables);
}


// UInputModifier interface ----------------------------------------------------

auto Self::ModifyRaw_Implementation(
	UEnhancedPlayerInput const* input,
	FInputActionValue value,
	float deltaTime)
	-> FInputActionValue
{
	if (m_NeedsInit)
		SetupCurves();

	if (!m_ReadyToTick)
		m_ReadyToTick = true;

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
	m_PrevOutput = result;

	return result;
}


void Self::SetupCurves()
{
	if (DebugOutput) {
		UE_LOG(LogTemp, Log, TEXT("-- INITIALIZING CURVES ------------------------"));
	}
	
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

	if (DebugOutput) {
		UE_LOG(LogTemp, Log, TEXT("Resistance:     %.3f"), Resistance);
		UE_LOG(LogTemp, Log, TEXT("Impulse:        %.3f"), Impulse);
		UE_LOG(LogTemp, Log, TEXT("SpringStifness: %.3f"), SpringStifness);
		UE_LOG(LogTemp, Log, TEXT("Damping:        %.3f"), Damping);

		UE_LOG(LogTemp, Log, TEXT("m_CurveIn:  %s"), *m_CurveIn.ToString());
		UE_LOG(LogTemp, Log, TEXT("m_CurveOut: %s"), *m_CurveOut.ToString());
	}
}

auto Self::ModifyRaw(float value, float deltaTime) -> FInputActionValue
{
	float prev = m_PrevInput.Get<float>();
	float delta = value - prev;

	// Have we changed direction this tick?
	if (FMath::Abs(delta) > 0.5f) {
		m_InputDuration = deltaTime;
		m_Phase = FMath::IsNearlyZero(value) ? Falling : Rising;

		if (DebugOutput)
			if (m_Phase == Rising) {
				UE_LOG(LogTemp, Log, TEXT("-- RISING -------------------------------"));
			} else {
				UE_LOG(LogTemp, Log, TEXT("-- FALLING ------------------------------"));
			}

		if (m_PrevCurve) {
			Swap(m_PrevCurve, m_ActiveCurve);
			if (DebugOutput) {
				UE_LOG(LogTemp, Log, TEXT("Prev:          %.3f, %.3f"), m_PrevX, m_PrevY);
			}
			m_PrevX = m_ActiveCurve->XForY(1.f - m_PrevY);
			if (DebugOutput) {
				UE_LOG(LogTemp, Log, TEXT("Phase-flipped: %.3f, %.3f"), m_PrevX, 1.f - m_PrevY);
			}
		}
		else {
			m_PrevCurve = m_ActiveCurve;
			m_ActiveCurve = m_Phase == Falling ? &m_CurveOut : &m_CurveIn;
		}
	}
	else if (m_Phase != None) {
		m_InputDuration += deltaTime;
	}

	if (m_Phase == None)
		return value;

	float x = FMath::Min(
		m_ActiveCurve->Duration(),
		m_PrevX + (deltaTime * m_ActiveCurve->Duration()));

	float result = m_ActiveCurve->YForX(x);

	m_PrevX = x;
	m_PrevY = result;

	if (DebugOutput) {
		UE_LOG(LogTemp, Log, TEXT("[%.3f] -> %.3f"), x, result);
	}

	if (FMath::IsNearlyEqual(result, value)) {
		m_Phase = None;
		m_InputDuration = m_ActiveCurve->Duration();

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
