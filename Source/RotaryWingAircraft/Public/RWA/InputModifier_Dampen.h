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
	virtual auto ModifyRaw_Implementation(
		const UEnhancedPlayerInput* input,
		FInputActionValue value,
		float deltaTime
	) -> FInputActionValue override;

private:
	FInputActionValue _LastValue;

	auto ModifyRaw(float value, float deltaTime) const -> FInputActionValue;
	auto ModifyRaw(FVector2D value, float deltaTime) const -> FInputActionValue;
	auto ModifyRaw(FVector value, float deltaTime) const -> FInputActionValue;
};
