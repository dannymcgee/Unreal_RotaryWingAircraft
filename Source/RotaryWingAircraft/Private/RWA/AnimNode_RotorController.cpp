#include "RWA/AnimNode_RotorController.h"
#include "RWA/HeliAnimInstance.h"

#define Self FAnimNode_RWA_RotorController


void Self::GatherDebugData(FNodeDebugData& data) {
	Super::GatherDebugData(data);
	// TODO
}

void Self::Initialize_AnyThread(const FAnimationInitializeContext& ctx) {
	_Proxy = static_cast<FRWA_HeliAnimInstanceProxy*>(ctx.AnimInstanceProxy);
}

void Self::InitializeBoneReferences(const FBoneContainer& requiredBones) {
	const auto& data = _Proxy->GetAnimData();
	auto len = data.Num();
	_Rotors.Empty(len);

	for (auto i = 0; i < len; ++i) {
		auto* rotor = new (_Rotors) FRWA_RotorLookupData { i, { data[i].BoneName }};
		rotor->BoneRef.Initialize(requiredBones);
	}

	// Sort by bone index
	_Rotors.Sort([](const FRWA_RotorLookupData& a, const FRWA_RotorLookupData& b) -> bool {
		return a.BoneRef.BoneIndex < b.BoneRef.BoneIndex;
	});
}

auto Self::IsValidToEvaluate(
	const USkeleton* skel,
	const FBoneContainer& requiredBones
) -> bool {
	for (const auto& rotor : _Rotors)
		if (rotor.BoneRef.IsValidToEvaluate(requiredBones))
			return true;

	return false;
}

void Self::EvaluateSkeletalControl_AnyThread(
	FComponentSpacePoseContext& inout_ctx,
	TArray<FBoneTransform>& out_boneTransforms
) {
	check(out_boneTransforms.Num() == 0);

	const auto& data = _Proxy->GetAnimData();
	const auto& container = inout_ctx.Pose.GetPose().GetBoneContainer();

	for (const auto& rotor : _Rotors) {
		if (!rotor.BoneRef.IsValidToEvaluate())
			continue;

		auto idx = rotor.BoneRef.GetCompactPoseIndex(container);
		auto xform = inout_ctx.Pose.GetComponentSpaceTransform(idx);

		FAnimationRuntime::ConvertCSTransformToBoneSpace(
			inout_ctx.AnimInstanceProxy->GetComponentTransform(),
			inout_ctx.Pose,
			xform,
			idx,
			BCS_ComponentSpace
		);

		auto q = FQuat { data[rotor.Index].Rotation };
		xform.SetRotation(q * xform.GetRotation());

		FAnimationRuntime::ConvertBoneSpaceTransformToCS(
			inout_ctx.AnimInstanceProxy->GetComponentTransform(),
			inout_ctx.Pose,
			xform,
			idx,
			BCS_ComponentSpace
		);

		out_boneTransforms.Add({ idx, xform });
	}
}


#undef Self
