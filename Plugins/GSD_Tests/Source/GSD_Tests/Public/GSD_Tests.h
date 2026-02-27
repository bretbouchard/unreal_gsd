// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class GSD_TESTS_API FGSD_TestsModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
