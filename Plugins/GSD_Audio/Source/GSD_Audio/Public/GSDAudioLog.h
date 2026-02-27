// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// Declare log category for GSD_Audio
GSD_AUDIO_API DECLARE_LOG_CATEGORY_EXTERN(LogGSDAudio, Log, All);

// Convenience macros for consistent logging
#define GSD_AUDIO_LOG(Verbosity, Format, ...) \
    UE_LOG(LogGSDAudio, Verbosity, Format, ##__VA_ARGS__)

#define GSD_AUDIO_WARN(Format, ...) \
    UE_LOG(LogGSDAudio, Warning, Format, ##__VA_ARGS__)

#define GSD_AUDIO_ERROR(Format, ...) \
    UE_LOG(LogGSDAudio, Error, Format, ##__VA_ARGS__)

#define GSD_AUDIO_TRACE(Format, ...) \
    UE_LOG(LogGSDAudio, Verbose, Format, ##__VA_ARGS__)
