#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstanceProxy.h"
#include "HeliAnimInstance.generated.h"

class ARWA_Heli;
class URWA_HeliMovementComponent;
struct FRWA_RotorSetup;


struct FRWA_RotorAnimData {
	FName BoneName = EName::None;
	FRotator Rotation = FRotator::ZeroRotator;
	FVector TorqueNormal = FVector::UpVector;

	FRWA_RotorAnimData() = default;
	FRWA_RotorAnimData(const FRWA_RotorSetup& rotor);
};


USTRUCT()
struct ROTARYWINGAIRCRAFT_API FRWA_HeliAnimInstanceProxy : public FAnimInstanceProxy {
	GENERATED_BODY()

	FRWA_HeliAnimInstanceProxy() = default;
	FRWA_HeliAnimInstanceProxy(UAnimInstance* inst) : Super(inst) {}

public:
	void SetMovementComponent(const URWA_HeliMovementComponent* mc);
	virtual void PreUpdate(UAnimInstance* instance, float deltaTime) override;
	auto GetAnimData() const -> const TArray<FRWA_RotorAnimData>&;

private:
	inline static constexpr
	float k_RpmToRadsPerSec = 0.10472;

	TArray<FRWA_RotorAnimData> _RotorInstances = {};
	float _RotorSpeed = 0;
	float _RotorAngle = 0;
};


UCLASS(Transient)
class ROTARYWINGAIRCRAFT_API URWA_HeliAnimInstance : public UAnimInstance {
	GENERATED_BODY()

public:
	URWA_HeliAnimInstance() : Super() {}
	void SetMovementComponent(const URWA_HeliMovementComponent* mc);
	auto GetMovementComponent() const -> const URWA_HeliMovementComponent*;

private:
	FRWA_HeliAnimInstanceProxy _Proxy = {};

	UPROPERTY(Transient)
	const URWA_HeliMovementComponent* _MovementComponent;

	UFUNCTION(BlueprintPure, Category="Animation")
	ARWA_Heli* GetVehicle() const;

	virtual void NativeInitializeAnimation() override;
	virtual auto CreateAnimInstanceProxy() -> FAnimInstanceProxy* override;
	virtual void DestroyAnimInstanceProxy(FAnimInstanceProxy* proxy) override;
};
