// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// Declare log category for GSD_Vehicles
GSD_VEHICLES_API DECLARE_LOG_CATEGORY_EXTERN(LOG_GSDVEHICLES, Log, All);

// Convenience macros for consistent logging
#define GSD_VEHICLE_LOG(Verbosity, Format, ...) \
    UE_LOG(LOG_GSDVEHICLES, Verbosity, Format, ##__VA_ARGS__)

#define GSD_VEHICLE_WARN(Format, ...) \
    UE_LOG(LOG_GSDVEHICLES, Warning, Format, ##__VA_ARGS__)

#define GSD_VEHICLE_ERROR(Format, ...) \
    UE_LOG(LOG_GSDVEHICLES, Error, Format, ##__VA_ARGS__)

#define GSD_VEHICLE_TRACE(Format, ...) \
    UE_LOG(LOG_GSDVEHICLES, Verbose, Format, ##__VA_ARGS__)

#define GSD_VEHICLE_VERY_TRACE(Format, ...) \
    UE_LOG(LOG_GSDVEHICLES, VeryVerbose, Format, ##__VA_ARGS__)
