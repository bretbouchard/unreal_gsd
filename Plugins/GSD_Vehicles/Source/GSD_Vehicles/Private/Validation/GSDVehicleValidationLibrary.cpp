// Copyright Bret Bouchard. All Rights Reserved.

#include "Validation/GSDVehicleValidationLibrary.h"
#include "Engine/SkeletalMesh.h"
#include "Rendering/SkeletalMeshModel.h"
#include "GSDVehicleLog.h"

bool UGSDVehicleValidationLibrary::ValidateWheelBoneOrientation(USkeletalMesh* SkeletalMesh, FName BoneName, FString& OutError)
{
	OutError.Empty();

	// Check skeletal mesh is valid
	if (!SkeletalMesh)
	{
		OutError = TEXT("SkeletalMesh is null");
		GSD_VEHICLE_ERROR(TEXT("ValidateWheelBoneOrientation: %s"), *OutError);
		return false;
	}

	// Get reference skeleton
	const FReferenceSkeleton& RefSkeleton = SkeletalMesh->GetRefSkeleton();

	// Find bone index
	const int32 BoneIndex = RefSkeleton.FindBoneIndex(BoneName);
	if (BoneIndex == INDEX_NONE)
	{
		OutError = FString::Printf(TEXT("Bone '%s' not found in skeletal mesh"), *BoneName.ToString());
		GSD_VEHICLE_ERROR(TEXT("ValidateWheelBoneOrientation: %s"), *OutError);
		return false;
	}

	// Get bone transform from reference pose
	const TArray<FTransform>& RefBonePose = RefSkeleton.GetRefBonePose();
	const FTransform& BoneTransform = RefBonePose[BoneIndex];
	const FQuat& BoneRotation = BoneTransform.GetRotation();

	// Get forward axis (X) and up axis (Z)
	const FVector ForwardAxis = BoneRotation.GetAxisX();
	const FVector UpAxis = BoneRotation.GetAxisZ();

	// Tolerance for axis validation (0.1 allows for slight misalignment)
	const float Tolerance = 0.1f;

	// Validate forward axis: X component should be close to 1.0 or -1.0
	const bool bForwardValid = FMath::Abs(FMath::Abs(ForwardAxis.X) - 1.0f) < Tolerance;
	if (!bForwardValid)
	{
		OutError = FString::Printf(
			TEXT("Bone '%s': X-axis (%.2f, %.2f, %.2f) should point forward (1, 0, 0) or (-1, 0, 0)"),
			*BoneName.ToString(),
			ForwardAxis.X,
			ForwardAxis.Y,
			ForwardAxis.Z
		);
		GSD_VEHICLE_WARN(TEXT("Wheel bone validation failed: %s"), *OutError);
		return false;
	}

	// Validate up axis: Z component should be close to 1.0 or -1.0
	const bool bUpValid = FMath::Abs(FMath::Abs(UpAxis.Z) - 1.0f) < Tolerance;
	if (!bUpValid)
	{
		OutError = FString::Printf(
			TEXT("Bone '%s': Z-axis (%.2f, %.2f, %.2f) should point up (0, 0, 1) or (0, 0, -1)"),
			*BoneName.ToString(),
			UpAxis.X,
			UpAxis.Y,
			UpAxis.Z
		);
		GSD_VEHICLE_WARN(TEXT("Wheel bone validation failed: %s"), *OutError);
		return false;
	}

	// Bone orientation is valid
	GSD_VEHICLE_TRACE(TEXT("Bone '%s' orientation validated successfully"), *BoneName.ToString());
	return true;
}

bool UGSDVehicleValidationLibrary::ValidateAllWheelBones(USkeletalMesh* SkeletalMesh, const TArray<FName>& WheelBoneNames, TArray<FString>& OutErrors)
{
	OutErrors.Empty();

	// Validate each wheel bone
	for (const FName& BoneName : WheelBoneNames)
	{
		FString Error;
		if (!ValidateWheelBoneOrientation(SkeletalMesh, BoneName, Error))
		{
			OutErrors.Add(Error);
		}
	}

	// Return true if no errors
	const bool bAllValid = OutErrors.Num() == 0;
	if (bAllValid)
	{
		GSD_VEHICLE_LOG(Log, TEXT("All %d wheel bones validated successfully"), WheelBoneNames.Num());
	}
	else
	{
		GSD_VEHICLE_WARN(TEXT("%d/%d wheel bones failed validation"), OutErrors.Num(), WheelBoneNames.Num());
	}

	return bAllValid;
}
