#pragma once

#include "CoreMinimal.h"
#include "AnimGraphNode_SkeletalControlBase.h"
#include "RWA/AnimNode_RotorController.h"

#include "AnimGraphNode_RotorController.generated.h"


UCLASS(MinimalAPI)
class UAnimGraphNode_RWA_RotorController : public UAnimGraphNode_SkeletalControlBase {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Settings", DisplayName="Node")
	FAnimNode_RWA_RotorController m_Node;

public:
	UAnimGraphNode_RWA_RotorController() : Super() {}

	auto GetNodeTitle(ENodeTitleType::Type type) const -> FText override;
	auto GetTooltipText() const -> FText override;
	auto IsCompatibleWithGraph(UEdGraph const* graph) const -> bool override;

	void ValidateAnimNodePostCompile(
		FCompilerResultsLog& msgLog,
		UAnimBlueprintGeneratedClass* compiledClass,
		int32 compiledNodeIdx)
		override;

protected:
	auto GetControllerDescription() const -> FText override;
	auto GetNode() const -> FAnimNode_SkeletalControlBase const* override;
};
