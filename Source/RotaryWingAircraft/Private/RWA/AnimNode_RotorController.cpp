#include "RWA/AnimNode_RotorController.h"
#include "RWA/HeliAnimInstance.h"


void FAnimNode_RWA_RotorController::GatherDebugData(FNodeDebugData& data)
{
	Super::GatherDebugData(data);
	// TODO
}

void FAnimNode_RWA_RotorController::Initialize_AnyThread(FAnimationInitializeContext const& ctx)
{
	m_Proxy = static_cast<FRWA_HeliAnimInstanceProxy*>(ctx.AnimInstanceProxy);
}

void FAnimNode_RWA_RotorController::InitializeBoneReferences(FBoneContainer const& requiredBones)
{
	TArray<FRWA_RotorAnimData> const& data = m_Proxy->GetAnimData();
	int32 len = data.Num();
	m_Rotors.Empty(len);

	for (int32 i = 0; i < len; ++i) {
		auto* rotor = new (m_Rotors) FRWA_RotorLookupData { i, { data[i].BoneName }};
		rotor->BoneRef.Initialize(requiredBones);
	}

	// Sort by bone index
	m_Rotors.Sort([](FRWA_RotorLookupData const& a, FRWA_RotorLookupData const& b) -> bool {
		return a.BoneRef.BoneIndex < b.BoneRef.BoneIndex;
	});
}

bool FAnimNode_RWA_RotorController::IsValidToEvaluate(
	USkeleton const* skel,
	FBoneContainer const& requiredBones)
{
	for (auto const& rotor : m_Rotors)
		if (rotor.BoneRef.IsValidToEvaluate(requiredBones))
			return true;

	return false;
}

void FAnimNode_RWA_RotorController::EvaluateSkeletalControl_AnyThread(
	FComponentSpacePoseContext& inout_ctx,
	TArray<FBoneTransform>& out_boneTransforms)
{
	check(out_boneTransforms.Num() == 0);

	TArray<FRWA_RotorAnimData> const& data = m_Proxy->GetAnimData();
	FBoneContainer const& container = inout_ctx.Pose.GetPose().GetBoneContainer();

	for (FRWA_RotorLookupData const& rotor : m_Rotors) {
		if (!rotor.BoneRef.IsValidToEvaluate())
			continue;

		FCompactPoseBoneIndex idx = rotor.BoneRef.GetCompactPoseIndex(container);
		FTransform xform = inout_ctx.Pose.GetComponentSpaceTransform(idx);

		FAnimationRuntime::ConvertCSTransformToBoneSpace(
			inout_ctx.AnimInstanceProxy->GetComponentTransform(),
			inout_ctx.Pose,
			xform,
			idx,
			BCS_ComponentSpace
		);

		FQuat q { data[rotor.Index].Rotation };
		xform.SetRotation(q * xform.GetRotation());

		FAnimationRuntime::ConvertBoneSpaceTransformToCS(
			inout_ctx.AnimInstanceProxy->GetComponentTransform(),
			inout_ctx.Pose,
			xform,
			idx,
			BCS_ComponentSpace
		);

		out_boneTransforms.Emplace(idx, xform);
	}
}
