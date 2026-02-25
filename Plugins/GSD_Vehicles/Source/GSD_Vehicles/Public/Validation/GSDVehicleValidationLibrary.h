// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GSDVehicleValidationLibrary.generated.h"

/**
 * Validation utility for checking wheel bone orientation in skeletal meshes.
 *
 * UE5 expects wheel bones with:
 * - X-axis pointing Forward
 * - Y-axis pointing Left
 * - Z-axis pointing Up
 *
 * Incorrect orientation causes physics bugs:
 * - Wheels rotating sideways
 * - Erratic vehicle handling
 * - Suspension behaving incorrectly
 *
 * Call these functions during:
 * - Asset import validation
 * - Editor validation passes
 * - Runtime sanity checks
 */
UCLASS()
class GSD_VEHICLES_API UGSDVehicleValidationLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Validates a single wheel bone's orientation.
	 *
	 * @param SkeletalMesh The skeletal mesh containing the wheel bone
	 * @param BoneName Name of the wheel bone to validate
	 * @param OutError Error message if validation fails, empty if successful
	 * @return True if bone orientation is valid (X=Forward, Z=Up)
	 */
	UFUNCTION(BlueprintCallable, Category = "GSD|Vehicles|Validation")
	static bool ValidateWheelBoneOrientation(USkeletalMesh* SkeletalMesh, FName BoneName, FString& OutError);

	/**
	 * Validates all wheel bones in a skeletal mesh.
	 *
	 * @param SkeletalMesh The skeletal mesh containing wheel bones
	 * @param WheelBoneNames Array of wheel bone names to validate
	 * @param OutErrors Array of error messages for failed validations
	 * @return True if all bone orientations are valid
	 */
	UFUNCTION(BlueprintCallable, Category = "GSD|Vehicles|Validation")
	static bool ValidateAllWheelBones(USkeletalMesh* SkeletalMesh, const TArray<FName>& WheelBoneNames, TArray<FString>& OutErrors);
};
