// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// Declare stats group for telemetry
DECLARE_STATS_GROUP(TEXT("GSD Telemetry"), STATGROUP_GSDTelemetry, STATCAT_Advanced);

// Performance tracking stats
DECLARE_CYCLE_STAT(TEXT("RecordFrameTime"), STAT_GSDRecordFrameTime, STATGROUP_GSDTelemetry);
DECLARE_CYCLE_STAT(TEXT("RecordHitch"), STAT_GSDRecordHitch, STATGROUP_GSDTelemetry);
DECLARE_CYCLE_STAT(TEXT("CountActors"), STAT_GSDCountActors, STATGROUP_GSDTelemetry);
DECLARE_CYCLE_STAT(TEXT("RecordCellLoadTime"), STAT_GSDRecordCellLoadTime, STATGROUP_GSDTelemetry);

// Counter stats
DECLARE_DWORD_COUNTER_STAT(TEXT("TotalFrameCount"), STAT_GSDTotalFrameCount, STATGROUP_GSDTelemetry);
DECLARE_DWORD_COUNTER_STAT(TEXT("TotalHitchCount"), STAT_GSDTotalHitchCount, STATGROUP_GSDTelemetry);
DECLARE_DWORD_COUNTER_STAT(TEXT("VehicleCount"), STAT_GSDVehicleCount, STATGROUP_GSDTelemetry);
DECLARE_DWORD_COUNTER_STAT(TEXT("ZombieCount"), STAT_GSDZombieCount, STATGROUP_GSDTelemetry);
DECLARE_DWORD_COUNTER_STAT(TEXT("HumanCount"), STAT_GSDHumanCount, STATGROUP_GSDTelemetry);
