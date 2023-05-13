#pragma once

#include "CoreMinimal.h"
#include "BoneControllers/AnimNode_SkeletalControlBase.h"
#include "AnimNode_RotorController.generated.h"

struct FRWA_HeliAnimInstanceProxy;


struct FRWA_RotorLookupData {
	int32 Index;
	FBoneReference BoneRef;

	FRWA_RotorLookupData() = default;
	FRWA_RotorLookupData(int32 idx, FBoneReference boneRef)
		: Index { idx }
		, BoneRef { boneRef }
	{}
};


USTRUCT()
struct ROTARYWINGAIRCRAFT_API FAnimNode_RWA_RotorController : public FAnimNode_SkeletalControlBase {
	GENERATED_BODY()

public:
	FAnimNode_RWA_RotorController() = default;

	virtual void GatherDebugData(FNodeDebugData& data) override;
	virtual void Initialize_AnyThread(const FAnimationInitializeContext& ctx) override;

	virtual auto IsValidToEvaluate(
		const USkeleton* skel,
		const FBoneContainer& requiredBones
	) -> bool override;

	virtual void EvaluateSkeletalControl_AnyThread(
		FComponentSpacePoseContext& inout_ctx,
		TArray<FBoneTransform>& out_boneTransforms
	) override;

private:
	const FRWA_HeliAnimInstanceProxy* _Proxy = nullptr;
	TArray<FRWA_RotorLookupData> _Rotors = {};

	virtual void InitializeBoneReferences(const FBoneContainer& requiredBones) override;
};
