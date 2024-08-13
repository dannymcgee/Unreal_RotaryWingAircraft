#pragma once

#include "CoreMinimal.h"
#include "BoneControllers/AnimNode_SkeletalControlBase.h"
#include "AnimNode_RotorController.generated.h"

struct FRWA_HeliAnimInstanceProxy;


struct FRWA_RotorLookupData
{
	int32 Index;
	FBoneReference BoneRef;

	FRWA_RotorLookupData() = default;
	FRWA_RotorLookupData(int32 idx, FBoneReference const& boneRef)
		: Index(idx)
		, BoneRef(boneRef)
	{}
};


USTRUCT()
struct ROTARYWINGAIRCRAFT_API FAnimNode_RWA_RotorController
	: public FAnimNode_SkeletalControlBase
{
	GENERATED_BODY()

public:
	FAnimNode_RWA_RotorController() = default;

	void GatherDebugData(FNodeDebugData& data) override;
	void Initialize_AnyThread(FAnimationInitializeContext const& ctx) override;

	bool IsValidToEvaluate(
		USkeleton const* skel,
		FBoneContainer const& requiredBones)
		override;

	void EvaluateSkeletalControl_AnyThread(
		FComponentSpacePoseContext& inout_ctx,
		TArray<FBoneTransform>& out_boneTransforms)
		override;

private:
	FRWA_HeliAnimInstanceProxy const* m_Proxy = nullptr;
	TArray<FRWA_RotorLookupData> m_Rotors = {};

	void InitializeBoneReferences(FBoneContainer const& requiredBones) override;
};
