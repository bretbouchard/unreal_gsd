// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// Declare log category for GSD_Telemetry
GSD_TELEMETRY_API DECLARE_LOG_CATEGORY_EXTERN(LogGSDTelemetry, Log, All);

// Convenience macros for consistent logging
#define GSDTELEMETRY_LOG(Verbosity, Format, ...) \
    UE_LOG(LogGSDTelemetry, Verbosity, Format, ##__VA_ARGS__)

#define GSDTELEMETRY_WARN(Format, ...) \
    UE_LOG(LogGSDTelemetry, Warning, Format, ##__VA_ARGS__)

#define GSDTELEMETRY_ERROR(Format, ...) \
    UE_LOG(LogGSDTelemetry, Error, Format, ##__VA_ARGS__)

#define GSDTELEMETRY_VERBOSE(Format, ...) \
    UE_LOG(LogGSDTelemetry, Verbose, Format, ##__VA_ARGS__)

#define GSDTELEMETRY_VERY_VERBOSE(Format, ...) \
    UE_LOG(LogGSDTelemetry, VeryVerbose, Format, ##__VA_ARGS__)
