#include "RWA/HeliAnimInstance.h"

#include "RWA/Heli.h"
#include "RWA/HeliMovement.h"

#define AnimData FRWA_RotorAnimData
#define Proxy FRWA_HeliAnimInstanceProxy
#define Self URWA_HeliAnimInstance


// Animation Data --------------------------------------------------------------

AnimData::FRWA_RotorAnimData(const FRWA_RotorSetup& rotor)
	: BoneName { rotor.BoneName }
	, TorqueNormal { rotor.TorqueNormal }
{}


// Animation instance proxy ----------------------------------------------------

void Proxy::SetMovementComponent(const URWA_HeliMovementComponent* mc) {
	const auto& rotors = mc->Rotors;
	_RotorInstances.Empty(rotors.Num());

	for (const auto& rotor : rotors)
		_RotorInstances.Add({ rotor });
}

void Proxy::PreUpdate(UAnimInstance* instance, float deltaTime) {
	Super::PreUpdate(instance, deltaTime);

	const auto* inst = CastChecked<URWA_HeliAnimInstance>(instance);
	if (!inst) return;

	const auto* mc = inst->GetMovementComponent();
	if (!mc) return;

	auto deltaAngle = mc->GetCurrentRPM() * deltaTime * -k_RpmToRadsPerSec;
	_RotorAngle = FMath::Fmod(_RotorAngle + deltaAngle, 2 * PI);

	for (auto& rotor : _RotorInstances)
		rotor.Rotation = FQuat { rotor.TorqueNormal, _RotorAngle }.Rotator();
}

auto Proxy::GetAnimData() const -> const TArray<FRWA_RotorAnimData>& {
	return _RotorInstances;
}


// Animation instance ----------------------------------------------------------

void Self::SetMovementComponent(const URWA_HeliMovementComponent* mc) {
	_MovementComponent = mc;
	_Proxy.SetMovementComponent(mc);
}

auto Self::GetMovementComponent() const -> const URWA_HeliMovementComponent* {
	return _MovementComponent;
}

auto Self::GetVehicle() const -> ARWA_Heli* {
	return Cast<ARWA_Heli>(GetOwningActor());
}

void Self::NativeInitializeAnimation() {
	if (auto* actor = GetOwningActor())
		if (auto* mc = actor->FindComponentByClass<URWA_HeliMovementComponent>())
			SetMovementComponent(mc);
}

auto Self::CreateAnimInstanceProxy() -> FAnimInstanceProxy* {
	return &_Proxy;
}

void Self::DestroyAnimInstanceProxy(FAnimInstanceProxy* proxy) {}


#undef Self
#undef Proxy
#undef AnimData
