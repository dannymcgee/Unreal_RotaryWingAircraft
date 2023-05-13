#include "RWA/HeliAnimInstance.h"

#include "RWA/Heli.h"
#include "RWA/HeliMovement.h"


// Animation Data --------------------------------------------------------------

FRotorAnimData::FRotorAnimData(const FRotorSetup& rotor)
	: BoneName { rotor.BoneName }
	, TorqueNormal { rotor.TorqueNormal }
{}


// Animation instance proxy ----------------------------------------------------

void FHeliAnimInstanceProxy::SetMovementComponent(const UHeliMvmtCmp* mc) {
	const auto& rotors = mc->Rotors;
	_RotorInstances.Empty(rotors.Num());

	for (const auto& rotor : rotors)
		_RotorInstances.Add({ rotor });
}

void FHeliAnimInstanceProxy::PreUpdate(UAnimInstance* instance, float deltaTime) {
	Super::PreUpdate(instance, deltaTime);

	const auto* inst = CastChecked<UHeliAnimInstance>(instance);
	if (!inst) return;

	const auto* mc = inst->GetMovementComponent();
	if (!mc) return;

	auto deltaAngle = mc->GetCurrentRPM() * deltaTime * -k_RpmToRadsPerSec;
	_RotorAngle = FMath::Fmod(_RotorAngle + deltaAngle, 2 * PI);

	for (auto& rotor : _RotorInstances)
		rotor.Rotation = FQuat { rotor.TorqueNormal, _RotorAngle }.Rotator();
}

auto FHeliAnimInstanceProxy::GetAnimData() const -> const TArray<FRotorAnimData>& {
	return _RotorInstances;
}


// Animation instance ----------------------------------------------------------

void UHeliAnimInstance::SetMovementComponent(const UHeliMvmtCmp* mc) {
	_MovementComponent = mc;
	_Proxy.SetMovementComponent(mc);
}

auto UHeliAnimInstance::GetMovementComponent() const -> const UHeliMvmtCmp* {
	return _MovementComponent;
}

auto UHeliAnimInstance::GetVehicle() const -> AHeli* {
	return Cast<AHeli>(GetOwningActor());
}

void UHeliAnimInstance::NativeInitializeAnimation() {
	if (auto* actor = GetOwningActor())
		if (auto* mc = actor->FindComponentByClass<UHeliMvmtCmp>())
			SetMovementComponent(mc);
}

auto UHeliAnimInstance::CreateAnimInstanceProxy() -> FAnimInstanceProxy* {
	return &_Proxy;
}

void UHeliAnimInstance::DestroyAnimInstanceProxy(FAnimInstanceProxy* proxy) {}
