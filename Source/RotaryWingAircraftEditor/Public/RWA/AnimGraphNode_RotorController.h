#pragma once

#include "CoreMinimal.h"
#include "AnimGraphNode_SkeletalControlBase.h"
#include "RWA/AnimNode_RotorController.h"

#include "AnimGraphNode_RotorController.generated.h"


UCLASS(MinimalAPI)
class UAnimGraphNode_RWA_RotorController
	: public UAnimGraphNode_SkeletalControlBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Settings", DisplayName="Node")
	FAnimNode_RWA_RotorController m_Node;

public:
	UAnimGraphNode_RWA_RotorController() : Super() {}

	FText GetNodeTitle(ENodeTitleType::Type type) const override;
	FText GetTooltipText() const override;
	bool IsCompatibleWithGraph(UEdGraph const* graph) const override;

	void ValidateAnimNodePostCompile(
		FCompilerResultsLog& msgLog,
		UAnimBlueprintGeneratedClass* compiledClass,
		int32 compiledNodeIdx)
		override;

protected:
	FText GetControllerDescription() const override;
	FAnimNode_SkeletalControlBase const* GetNode() const override;
};
