// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

GSD_VALIDATIONTOOLS_API DECLARE_LOG_CATEGORY_EXTERN(LogGSDValidation, Log, All);

// Convenience macros for validation logging
#define GSDVALIDATION_LOG(Verbosity, Format, ...) \
    UE_LOG(LogGSDValidation, Verbosity, Format, ##__VA_ARGS__)

#define GSDVALIDATION_WARN(Format, ...) \
    UE_LOG(LogGSDValidation, Warning, Format, ##__VA_ARGS__)

#define GSDVALIDATION_ERROR(Format, ...) \
    UE_LOG(LogGSDValidation, Error, Format, ##__VA_ARGS__)

#define GSDVALIDATION_VERBOSE(Format, ...) \
    UE_LOG(LogGSDValidation, Verbose, Format, ##__VA_ARGS__)
