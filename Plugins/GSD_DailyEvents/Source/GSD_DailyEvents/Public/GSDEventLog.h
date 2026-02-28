// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// Declare log category for GSD_DailyEvents
GSD_DAILYEVENTS_API DECLARE_LOG_CATEGORY_EXTERN(LOG_GSDEVENTS, Log, All);

// Convenience macros for consistent logging
#define GSDEVENT_LOG(Verbosity, Format, ...) \
    UE_LOG(LOG_GSDEVENTS, Verbosity, Format, ##__VA_ARGS__)

#define GSDEVENT_WARN(Format, ...) \
    UE_LOG(LOG_GSDEVENTS, Warning, Format, ##__VA_ARGS__)

#define GSDEVENT_ERROR(Format, ...) \
    UE_LOG(LOG_GSDEVENTS, Error, Format, ##__VA_ARGS__)

#define GSDEVENT_TRACE(Format, ...) \
    UE_LOG(LOG_GSDEVENTS, Verbose, Format, ##__VA_ARGS__)

#define GSDEVENT_VERY_TRACE(Format, ...) \
    UE_LOG(LOG_GSDEVENTS, VeryVerbose, Format, ##__VA_ARGS__)
