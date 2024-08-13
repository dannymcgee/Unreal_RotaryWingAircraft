#include "RWA/HeliAnimInstance.h"

#include "RWA/Heli.h"
#include "RWA/HeliMovement.h"


// Animation Data --------------------------------------------------------------

FRWA_RotorAnimData::FRWA_RotorAnimData(FRWA_RotorSetup const& rotor)
	: BoneName(rotor.BoneName)
	, TorqueNormal(rotor.TorqueNormal)
{}


// Animation instance proxy ----------------------------------------------------

void FRWA_HeliAnimInstanceProxy::SetMovementComponent(URWA_HeliMovementComponent const* mc)
{
	TArray<FRWA_RotorSetup> const& rotors = mc->Rotors;
	m_RotorInstances.Empty(rotors.Num());

	for (FRWA_RotorSetup const& rotor : rotors)
		m_RotorInstances.Add({ rotor });
}

void FRWA_HeliAnimInstanceProxy::PreUpdate(UAnimInstance* instance, float deltaTime)
{
	Super::PreUpdate(instance, deltaTime);

	auto const* inst = CastChecked<URWA_HeliAnimInstance>(instance);
	if (!inst) return;

	URWA_HeliMovementComponent const* mc = inst->GetMovementComponent();
	if (!mc) return;

	float deltaAngle = mc->GetCurrentRPM() * deltaTime * -k_RpmToRadsPerSec;
	m_RotorAngle = FMath::Fmod(m_RotorAngle + deltaAngle, 2 * PI);

	for (FRWA_RotorAnimData& rotor : m_RotorInstances)
		rotor.Rotation = FQuat(rotor.TorqueNormal, m_RotorAngle).Rotator();
}

TArray<FRWA_RotorAnimData> const& FRWA_HeliAnimInstanceProxy::GetAnimData() const 
{
	return m_RotorInstances;
}


// Animation instance ----------------------------------------------------------

void URWA_HeliAnimInstance::SetMovementComponent(URWA_HeliMovementComponent const* mc)
{
	m_MovementComponent = mc;
	m_Proxy.SetMovementComponent(mc);
}

URWA_HeliMovementComponent const* URWA_HeliAnimInstance::GetMovementComponent() const
{
	return m_MovementComponent;
}

ARWA_Heli* URWA_HeliAnimInstance::GetVehicle() const 
{
	return Cast<ARWA_Heli>(GetOwningActor());
}

void URWA_HeliAnimInstance::NativeInitializeAnimation()
{
	if (AActor* actor = GetOwningActor())
		if (auto* mc = actor->FindComponentByClass<URWA_HeliMovementComponent>())
			SetMovementComponent(mc);
}

FAnimInstanceProxy* URWA_HeliAnimInstance::CreateAnimInstanceProxy() 
{
	return &m_Proxy;
}

void URWA_HeliAnimInstance::DestroyAnimInstanceProxy(FAnimInstanceProxy* proxy)
{}
