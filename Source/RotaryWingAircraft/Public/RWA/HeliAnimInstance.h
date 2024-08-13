#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstanceProxy.h"
#include "HeliAnimInstance.generated.h"

class ARWA_Heli;
class URWA_HeliMovementComponent;
struct FRWA_RotorSetup;


struct FRWA_RotorAnimData
{
	FName BoneName = EName::None;
	FRotator Rotation = FRotator::ZeroRotator;
	FVector TorqueNormal = FVector::UpVector;

	FRWA_RotorAnimData() = default;
	FRWA_RotorAnimData(FRWA_RotorSetup const& rotor);
};


USTRUCT()
struct ROTARYWINGAIRCRAFT_API FRWA_HeliAnimInstanceProxy
	: public FAnimInstanceProxy
{
	GENERATED_BODY()

	FRWA_HeliAnimInstanceProxy() = default;
	FRWA_HeliAnimInstanceProxy(UAnimInstance* inst) : Super(inst) {}

public:
	void SetMovementComponent(URWA_HeliMovementComponent const* mc);
	void PreUpdate(UAnimInstance* instance, float deltaTime) override;
	TArray<FRWA_RotorAnimData> const& GetAnimData() const;

private:
	inline static constexpr
	float k_RpmToRadsPerSec = 0.10472;

	TArray<FRWA_RotorAnimData> m_RotorInstances = {};
	float m_RotorSpeed = 0;
	float m_RotorAngle = 0;
};


UCLASS(Transient)
class ROTARYWINGAIRCRAFT_API URWA_HeliAnimInstance
	: public UAnimInstance
{
	GENERATED_BODY()

public:
	URWA_HeliAnimInstance() : Super() {}
	void SetMovementComponent(URWA_HeliMovementComponent const* mc);
	URWA_HeliMovementComponent const* GetMovementComponent() const;

private:
	FRWA_HeliAnimInstanceProxy m_Proxy = {};

	UPROPERTY(Transient, DisplayName="Movement Component")
	URWA_HeliMovementComponent const* m_MovementComponent;

	UFUNCTION(BlueprintPure, Category="Animation")
	ARWA_Heli* GetVehicle() const;

	void NativeInitializeAnimation() override;
	FAnimInstanceProxy* CreateAnimInstanceProxy() override;
	void DestroyAnimInstanceProxy(FAnimInstanceProxy* proxy) override;
};
