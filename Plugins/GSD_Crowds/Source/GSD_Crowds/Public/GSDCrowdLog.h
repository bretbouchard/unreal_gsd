// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// Declare log category for GSD_Crowds
GSD_CROWDS_API DECLARE_LOG_CATEGORY_EXTERN(LOG_GSDCROWDS, Log, All);

// Convenience macros for consistent logging
#define GSD_CROWD_LOG(Verbosity, Format, ...) \
    UE_LOG(LOG_GSDCROWDS, Verbosity, Format, ##__VA_ARGS__)

#define GSD_CROWD_WARN(Format, ...) \
    UE_LOG(LOG_GSDCROWDS, Warning, Format, ##__VA_ARGS__)

#define GSD_CROWD_ERROR(Format, ...) \
    UE_LOG(LOG_GSDCROWDS, Error, Format, ##__VA_ARGS__)

#define GSD_CROWD_TRACE(Format, ...) \
    UE_LOG(LOG_GSDCROWDS, Verbose, Format, ##__VA_ARGS__)

#define GSD_CROWD_VERY_TRACE(Format, ...) \
    UE_LOG(LOG_GSDCROWDS, VeryVerbose, Format, ##__VA_ARGS__)
