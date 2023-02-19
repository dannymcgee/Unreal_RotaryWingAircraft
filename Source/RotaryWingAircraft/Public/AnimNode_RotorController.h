#pragma once

#include "CoreMinimal.h"
#include "BoneControllers/AnimNode_SkeletalControlBase.h"
#include "AnimNode_RotorController.generated.h"

struct FHeliAnimInstanceProxy;


struct FRotorLookupData {
	int32 Index;
	FBoneReference BoneRef;

	FRotorLookupData() = default;
	FRotorLookupData(int32 idx, FBoneReference boneRef)
		: Index { idx }
		, BoneRef { boneRef }
	{}
};


USTRUCT()
struct ROTARYWINGAIRCRAFT_API FAnimNode_RotorController : public FAnimNode_SkeletalControlBase {
	GENERATED_BODY()

public:
	FAnimNode_RotorController() = default;

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
	const FHeliAnimInstanceProxy* _Proxy = nullptr;
	TArray<FRotorLookupData> _Rotors = {};

	virtual void InitializeBoneReferences(const FBoneContainer& requiredBones) override;
};
