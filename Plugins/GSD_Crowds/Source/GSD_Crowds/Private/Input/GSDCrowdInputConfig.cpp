// Copyright Bret Bouchard
// EnhancedInput Configuration for Crowd Debug Controls

#include "Input/GSDCrowdInputConfig.h"

UGSDCrowdInputConfig::UGSDCrowdInputConfig()
    : MappingContextPriority(100)
{
    // Default priority set to 100 for debug controls
    // Debug controls should have higher priority than gameplay controls
}
