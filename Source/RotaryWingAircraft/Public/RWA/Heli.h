#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Heli.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogHeli, Log, All);

struct FInputActionValue;
class IEnhancedInputSubsystemInterface;
class URWA_HeliMovementComponent;
class UInputAction;
class UInputMappingContext;


UCLASS(Blueprintable)
class ROTARYWINGAIRCRAFT_API ARWA_Heli : public APawn {
	GENERATED_BODY()


public:

	ARWA_Heli();

	virtual void SetupPlayerInputComponent(UInputComponent* inputCmp) override;

	UFUNCTION(BlueprintGetter)
	USkeletalMeshComponent* GetMesh() const;

	UFUNCTION(BlueprintGetter)
	URWA_HeliMovementComponent* GetVehicleMovement() const;

	virtual auto GetMovementComponent() const -> UPawnMovementComponent* override;

	/**
	 * Adds the vehicle's input mapping context to the current player. If the
	 * player can enter/exit the vehicle during gameplay, you'll want to call
	 * this _after_ the player takes control of the vehicle.
	 *
	 * @note This method is safe to call even if the vehicle is not
	 * player-controlled.
	 */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void AddInputMappingContext() const;

	/**
	 * Removes the vehicle's input mapping context from the current player. If
	 * the player can enter/exit the vehicle during gameplay, you'll want to call
	 * this _before_ the player forfeits control of the vehicle.
	 *
	 * @note This method is safe to call even if the vehicle is not
	 * player-controlled.
	 */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void RemoveInputMappingContext() const;


protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* CyclicAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* CollectiveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input", DisplayName="Anti-Torque Action")
	UInputAction* AntiTorqueAction;

	virtual void BeginPlay() override;

	virtual auto GetInputSubsystem() const -> IEnhancedInputSubsystemInterface*;


private:

	UPROPERTY(EditDefaultsOnly, BlueprintGetter="GetMesh", Category="Vehicle")
	USkeletalMeshComponent* _Mesh;

	UPROPERTY(EditDefaultsOnly, BlueprintGetter="GetVehicleMovement", Category="Vehicle")
	URWA_HeliMovementComponent* _VehicleMovement;

	auto InitSkelMesh() -> USkeletalMeshComponent*;
	auto InitVehicleMovement(USkeletalMeshComponent* mesh) -> URWA_HeliMovementComponent*;

	void OnCyclic(const FInputActionValue& value);
	void OnCollective(const FInputActionValue& value);
	void OnAntiTorque(const FInputActionValue& value);
};
