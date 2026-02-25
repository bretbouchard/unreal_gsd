#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IGSDStreamable.generated.h"

UINTERFACE(MinimalAPI, Blueprintable, Category = "GSD")
class UGSDStreamable : public UInterface
{
    GENERATED_BODY()
};

class GSD_CORE_API IGSDStreamable
{
    GENERATED_BODY()

public:
    // Get streaming bounds for World Partition
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Streaming")
    FBoxSphereBounds GetStreamingBounds();
    virtual FBoxSphereBounds GetStreamingBounds_Implementation();

    // Called when actor streams in (cell loaded)
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Streaming")
    void OnStreamIn();
    virtual void OnStreamIn_Implementation();

    // Called when actor streams out (cell unloaded)
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Streaming")
    void OnStreamOut();
    virtual void OnStreamOut_Implementation();

    // Get streaming priority (higher = more important)
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Streaming")
    int32 GetStreamingPriority();
    virtual int32 GetStreamingPriority_Implementation();

    // Should this actor persist across level changes?
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Streaming")
    bool ShouldPersist();
    virtual bool ShouldPersist_Implementation();

    // Get distance override for streaming (0 = use default)
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Streaming")
    float GetStreamingDistanceOverride();
    virtual float GetStreamingDistanceOverride_Implementation();

    // Is this actor currently streamed in?
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Streaming")
    bool IsCurrentlyStreamedIn() const;
    virtual bool IsCurrentlyStreamedIn_Implementation() const;
};
