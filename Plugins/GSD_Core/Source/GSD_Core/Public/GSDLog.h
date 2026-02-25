#pragma once

#include "CoreMinimal.h"

// Declare log category (header)
GSD_CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogGSD, Log, All);

// Convenience macros for consistent logging
#define GSD_LOG(Verbosity, Format, ...) \
    UE_LOG(LogGSD, Verbosity, Format, ##__VA_ARGS__)

#define GSD_WARN(Format, ...) \
    UE_LOG(LogGSD, Warning, Format, ##__VA_ARGS__)

#define GSD_ERROR(Format, ...) \
    UE_LOG(LogGSD, Error, Format, ##__VA_ARGS__)

#define GSD_TRACE(Format, ...) \
    UE_LOG(LogGSD, Verbose, Format, ##__VA_ARGS__)

#define GSD_VERY_TRACE(Format, ...) \
    UE_LOG(LogGSD, VeryVerbose, Format, ##__VA_ARGS__)
