#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Heli.generated.h"

class UHeliMvmtCmp;


UCLASS(Blueprintable)
class ROTARYWINGAIRCRAFT_API AHeli : public APawn {
	GENERATED_BODY()
	

public:

	AHeli();

	virtual void Tick(float deltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* input) override;

	UFUNCTION(BlueprintGetter)
	USkeletalMeshComponent* GetMesh() const;

	UFUNCTION(BlueprintGetter)
	UHeliMvmtCmp* GetVehicleMovement() const;

	virtual auto GetMovementComponent() const -> UPawnMovementComponent* override;


private:

	UPROPERTY(EditDefaultsOnly, BlueprintGetter="GetMesh", Category="Vehicle")
	USkeletalMeshComponent* _Mesh;

	UPROPERTY(EditDefaultsOnly, BlueprintGetter="GetVehicleMovement", Category="Vehicle")
	UHeliMvmtCmp* _VehicleMovement;

	float _CollectiveInput = 0;
	float _YawInput = 0;

	auto InitSkelMesh() -> USkeletalMeshComponent*;
	auto InitVehicleMovement(USkeletalMeshComponent* mesh) -> UHeliMvmtCmp*;

	void OnCollectiveUp(float input);
	void OnCollectiveDown(float input);

	void IncYaw();
	void DecYaw();
};
