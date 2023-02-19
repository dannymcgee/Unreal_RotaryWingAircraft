#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstanceProxy.h"
#include "HeliAnimInstance.generated.h"

class AHeli;
class UHeliMvmtCmp;
struct FRotorSetup;


struct FRotorAnimData {
	FName BoneName = EName::None;
	FRotator Rotation = FRotator::ZeroRotator;
	FVector TorqueNormal = FVector::UpVector;

	FRotorAnimData() = default;
	FRotorAnimData(const FRotorSetup& rotor);
};


USTRUCT()
struct ROTARYWINGAIRCRAFT_API FHeliAnimInstanceProxy : public FAnimInstanceProxy {
	GENERATED_BODY()

	FHeliAnimInstanceProxy() : Super() {}
	FHeliAnimInstanceProxy(UAnimInstance* inst) : Super(inst) {}

public:
	void SetMovementComponent(const UHeliMvmtCmp* mc);
	virtual void PreUpdate(UAnimInstance* instance, float deltaTime) override;
	auto GetAnimData() const -> const TArray<FRotorAnimData>&;

private:
	inline static constexpr
	float k_RpmToRadsPerSec = 0.10472;
	
	TArray<FRotorAnimData> _RotorInstances = {};
	float _RotorSpeed = 0;
	float _RotorAngle = 0;
};


UCLASS(Transient)
class ROTARYWINGAIRCRAFT_API UHeliAnimInstance : public UAnimInstance {
	GENERATED_BODY()

public:
	UHeliAnimInstance() : Super() {}
	void SetMovementComponent(const UHeliMvmtCmp* mc);
	auto GetMovementComponent() const -> const UHeliMvmtCmp*;

private:
	FHeliAnimInstanceProxy _Proxy = {};

	UPROPERTY(Transient)
	const UHeliMvmtCmp* _MovementComponent;

	UFUNCTION(BlueprintPure, Category="Animation")
	AHeli* GetVehicle() const;

	virtual void NativeInitializeAnimation() override;
	virtual auto CreateAnimInstanceProxy() -> FAnimInstanceProxy* override;
	virtual void DestroyAnimInstanceProxy(FAnimInstanceProxy* proxy) override;
};
