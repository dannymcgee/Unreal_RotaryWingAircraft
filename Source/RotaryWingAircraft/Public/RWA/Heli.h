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
class ROTARYWINGAIRCRAFT_API ARWA_Heli
	: public APawn
{
	GENERATED_BODY()

	using Self = ARWA_Heli;

public:

	ARWA_Heli(FObjectInitializer const& init);

	void SetupPlayerInputComponent(UInputComponent* inputCmp) override;

	UFUNCTION(BlueprintGetter)
	USkeletalMeshComponent* GetMesh() const;

	UFUNCTION(BlueprintGetter)
	URWA_HeliMovementComponent* GetVehicleMovement() const;

	UPawnMovementComponent* GetMovementComponent() const override;

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
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> CyclicAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> CollectiveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input", DisplayName="Anti-Torque Action")
	TObjectPtr<UInputAction> AntiTorqueAction;

	void BeginPlay() override;

	virtual IEnhancedInputSubsystemInterface* GetInputSubsystem() const;


private:

	UPROPERTY(EditDefaultsOnly, BlueprintGetter="GetMesh", Category="Vehicle", DisplayName="Mesh")
	TObjectPtr<USkeletalMeshComponent> m_Mesh;

	UPROPERTY(EditDefaultsOnly, BlueprintGetter="GetVehicleMovement", Category="Vehicle", DisplayName="Vehicle Movement")
	TObjectPtr<URWA_HeliMovementComponent> m_VehicleMovement;

	USkeletalMeshComponent* InitSkelMesh();
	URWA_HeliMovementComponent* InitVehicleMovement(USkeletalMeshComponent* mesh);

	void OnCyclic(FInputActionValue const& value);
	void OnCollective(FInputActionValue const& value);
	void OnAntiTorque(FInputActionValue const& value);
};
