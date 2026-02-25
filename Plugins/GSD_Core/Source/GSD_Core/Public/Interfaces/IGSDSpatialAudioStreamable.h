#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interfaces/IGSDStreamable.h"
#include "Types/GSDSpatialAudioTypes.h"
#include "IGSDSpatialAudioStreamable.generated.h"

UINTERFACE(MinimalAPI, Blueprintable, Category = "GSD")
class UGSDSpatialAudioStreamable : public UGSDStreamable
{
    GENERATED_BODY()
};

class GSD_CORE_API IGSDSpatialAudioStreamable : public IGSDStreamable
{
    GENERATED_BODY()

public:
    // Get spatial state before stream out (for preservation)
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Audio")
    FGSDSpatialAudioState GetSpatialState() const;
    virtual FGSDSpatialAudioState GetSpatialState_Implementation() const;

    // Restore spatial state on stream in
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Audio")
    void RestoreSpatialState(const FGSDSpatialAudioState& State);
    virtual void RestoreSpatialState_Implementation(const FGSDSpatialAudioState& State);

    // Distance threshold for audio LOD
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Audio")
    float GetAudioLODDistance() const;
    virtual float GetAudioLODDistance_Implementation() const;

    // Get current audio LOD level
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Audio")
    int32 GetCurrentAudioLOD() const;
    virtual int32 GetCurrentAudioLOD_Implementation() const;

    // Set audio LOD level
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Audio")
    void SetAudioLOD(int32 LODLevel);
    virtual void SetAudioLOD_Implementation(int32 LODLevel);

    // Is audio currently playing?
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Audio")
    bool IsAudioPlaying() const;
    virtual bool IsAudioPlaying_Implementation() const;

    // Stop audio (for virtualization)
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Audio")
    void VirtualizeAudio();
    virtual void VirtualizeAudio_Implementation();

    // Resume audio from virtualized state
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Audio")
    void DevirtualizeAudio();
    virtual void DevirtualizeAudio_Implementation();
};
