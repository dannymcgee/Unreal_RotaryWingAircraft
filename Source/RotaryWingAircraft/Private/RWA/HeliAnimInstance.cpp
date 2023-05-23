#include "RWA/HeliAnimInstance.h"

#include "RWA/Heli.h"
#include "RWA/HeliMovement.h"

#define AnimData FRWA_RotorAnimData
#define Proxy FRWA_HeliAnimInstanceProxy
#define Self URWA_HeliAnimInstance


// Animation Data --------------------------------------------------------------

AnimData::FRWA_RotorAnimData(FRWA_RotorSetup const& rotor)
	: BoneName(rotor.BoneName)
	, TorqueNormal(rotor.TorqueNormal)
{}


// Animation instance proxy ----------------------------------------------------

void Proxy::SetMovementComponent(URWA_HeliMovementComponent const* mc)
{
	auto const& rotors = mc->Rotors;
	m_RotorInstances.Empty(rotors.Num());

	for (auto const& rotor : rotors)
		m_RotorInstances.Add({ rotor });
}

void Proxy::PreUpdate(UAnimInstance* instance, float deltaTime)
{
	Super::PreUpdate(instance, deltaTime);

	auto const* inst = CastChecked<URWA_HeliAnimInstance>(instance);
	if (!inst) return;

	auto const* mc = inst->GetMovementComponent();
	if (!mc) return;

	auto deltaAngle = mc->GetCurrentRPM() * deltaTime * -k_RpmToRadsPerSec;
	m_RotorAngle = FMath::Fmod(m_RotorAngle + deltaAngle, 2 * PI);

	for (auto& rotor : m_RotorInstances)
		rotor.Rotation = FQuat(rotor.TorqueNormal, m_RotorAngle).Rotator();
}

auto Proxy::GetAnimData() const -> TArray<FRWA_RotorAnimData> const&
{
	return m_RotorInstances;
}


// Animation instance ----------------------------------------------------------

void Self::SetMovementComponent(URWA_HeliMovementComponent const* mc)
{
	m_MovementComponent = mc;
	m_Proxy.SetMovementComponent(mc);
}

auto Self::GetMovementComponent() const -> URWA_HeliMovementComponent const*
{
	return m_MovementComponent;
}

auto Self::GetVehicle() const -> ARWA_Heli*
{
	return Cast<ARWA_Heli>(GetOwningActor());
}

void Self::NativeInitializeAnimation()
{
	if (auto* actor = GetOwningActor())
		if (auto* mc = actor->FindComponentByClass<URWA_HeliMovementComponent>())
			SetMovementComponent(mc);
}

auto Self::CreateAnimInstanceProxy() -> FAnimInstanceProxy*
{
	return &m_Proxy;
}

void Self::DestroyAnimInstanceProxy(FAnimInstanceProxy* proxy)
{}


#undef Self
#undef Proxy
#undef AnimData
