#include "RWA/AnimGraphNode_RotorController.h"

#include "RWA/HeliAnimInstance.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/CompilerResultsLog.h"

#define LOCTEXT_NAMESPACE "A3Nodes"


FText UAnimGraphNode_RWA_RotorController::GetNodeTitle(ENodeTitleType::Type type) const
{
	if (type == ENodeTitleType::ListView || type == ENodeTitleType::MenuTitle)
		return GetControllerDescription();

	return LOCTEXT("AnimGraphNode_RWA_RotorController", "Rotor Controller");
}

FText UAnimGraphNode_RWA_RotorController::GetTooltipText() const
{
	return LOCTEXT(
		"AnimGraphNode_RWA_RotorController_Tooltip",
		"Rotates the aircraft's rotor systems based on Heli Movement Component "
			"setup. This only works when the owner is a Heli actor."
	);
}

bool UAnimGraphNode_RWA_RotorController::IsCompatibleWithGraph(UEdGraph const* graph) const
{
	UBlueprint* bp = FBlueprintEditorUtils::FindBlueprintForGraph(graph);
	return (
		bp != nullptr
		&& bp->ParentClass->IsChildOf<URWA_HeliAnimInstance>()
		&& Super::IsCompatibleWithGraph(graph)
	);
}

void UAnimGraphNode_RWA_RotorController::ValidateAnimNodePostCompile(
	FCompilerResultsLog& msgLog,
	UAnimBlueprintGeneratedClass* compiledClass,
	int32 compiledNodeIdx)
{
	if (!compiledClass->IsChildOf<URWA_HeliAnimInstance>())
		msgLog.Error(
			TEXT("@@ is only allowed in HeliAnimInstance. Change animation "
				"blueprint parent to HeliAnimInstance to fix."),
			this
		);
}

FText UAnimGraphNode_RWA_RotorController::GetControllerDescription() const
{
	return LOCTEXT(
		"AnimGraphNode_RWA_RotorController",
		"Rotor Controller for Rotary-Wing Aircraft"
	);
}

FAnimNode_SkeletalControlBase const* UAnimGraphNode_RWA_RotorController::GetNode() const
{
	return &m_Node;
}


#undef LOCTEXT_NAMESPACE
