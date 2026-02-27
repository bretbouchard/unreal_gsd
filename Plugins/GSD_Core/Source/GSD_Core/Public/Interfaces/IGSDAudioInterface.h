#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IGSDAudioInterface.generated.h"

class USoundClass;
class USoundSubmixBase;
class USoundBase;

/**
 * Interface for audio systems in the GSD Platform.
 * Provides access to Sound Classes, Submixes, and audio utilities.
 */
UINTERFACE(MinimalAPI, BlueprintType, Category = "GSD", Meta = (CannotImplementInterfaceInBlueprint))
class UGSDAudioInterface : public UInterface
{
    GENERATED_BODY()
};

class GSD_CORE_API IGSDAudioInterface
{
    GENERATED_BODY()

public:
    /**
     * Get the Master Sound Class (root of all audio).
     * @return Master Sound Class asset
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Audio")
    virtual USoundClass* GetMasterSoundClass() const = 0;

    /**
     * Get a Sound Class by name.
     * @param ClassName Name of the Sound Class (e.g., "Music", "SFX", "Voice")
     * @return Sound Class asset, or nullptr if not found
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Audio")
    virtual USoundClass* GetSoundClass(FName ClassName) const = 0;

    /**
     * Get a Sound Submix by name.
     * @param SubmixName Name of the Submix (e.g., "Reverb", "EQ", "Master")
     * @return Sound Submix asset, or nullptr if not found
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Audio")
    virtual USoundSubmixBase* GetSubmix(FName SubmixName) const = 0;

    /**
     * Set volume for a Sound Class.
     * @param ClassName Name of the Sound Class
     * @param Volume New volume level (0.0 to 1.0)
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Audio")
    virtual void SetSoundClassVolume(FName ClassName, float Volume) = 0;

    /**
     * Mute or unmute a Sound Class.
     * @param ClassName Name of the Sound Class
     * @param bMute True to mute, false to unmute
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Audio")
    virtual void SetSoundClassMuted(FName ClassName, bool bMute) = 0;

    /**
     * Play a sound at a specific location with spatialization.
     * @param Sound Sound asset to play
     * @param Location World location
     * @param ClassName Sound Class to use for routing
     * @param Volume Volume multiplier (default: 1.0)
     * @param Pitch Pitch multiplier (default: 1.0)
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Audio")
    virtual void PlaySoundAtLocation(
        USoundBase* Sound,
        FVector Location,
        FName ClassName,
        float Volume = 1.0f,
        float Pitch = 1.0f
    ) = 0;
};
