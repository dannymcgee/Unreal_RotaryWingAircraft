#pragma once

#include "CoreMinimal.h"
#include "InputModifiers.h"
#include "InputModifier_Dampen.generated.h"


UCLASS(NotBlueprintable, MinimalAPI, meta=(DisplayName="Dampen"))
class UInputModifier_RWA_Dampen : public UInputModifier {
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings")
	float LerpSpeed = 5.f;

protected:
	auto ModifyRaw_Implementation(
		UEnhancedPlayerInput const* input,
		FInputActionValue value,
		float deltaTime)
		-> FInputActionValue override;

private:
	FInputActionValue m_LastValue;

	auto ModifyRaw(float value, float deltaTime) const -> FInputActionValue;
	auto ModifyRaw(FVector2D value, float deltaTime) const -> FInputActionValue;
	auto ModifyRaw(FVector value, float deltaTime) const -> FInputActionValue;
};
