#pragma once

#include "CoreMinimal.h"
#include "AnimGraphNode_SkeletalControlBase.h"
#include "RWA/AnimNode_RotorController.h"

#include "AnimGraphNode_RotorController.generated.h"


UCLASS(MinimalAPI)
class UAnimGraphNode_RotorController : public UAnimGraphNode_SkeletalControlBase {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Settings")
	FAnimNode_RotorController _Node;

public:
	UAnimGraphNode_RotorController() : Super() {}

	virtual auto GetNodeTitle(ENodeTitleType::Type type) const -> FText override;
	virtual auto GetTooltipText() const -> FText override;
	virtual auto IsCompatibleWithGraph(const UEdGraph* graph) const -> bool override;

	virtual void ValidateAnimNodePostCompile(
		FCompilerResultsLog& msgLog,
		UAnimBlueprintGeneratedClass* compiledClass,
		int32 compiledNodeIdx
	) override;

protected:
	virtual auto GetControllerDescription() const -> FText override;
	virtual auto GetNode() const -> const FAnimNode_SkeletalControlBase* override;
};
