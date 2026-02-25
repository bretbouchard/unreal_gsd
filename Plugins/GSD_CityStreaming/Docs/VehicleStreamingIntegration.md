# Vehicle Streaming Integration Guide

## Overview

Vehicles need streaming sources for predictive loading as they traverse the city. This guide explains how to integrate the `GSDStreamingSourceComponent` with your vehicle classes.

### Why Vehicles Need Streaming Sources

- **Predictive Loading**: Fast vehicles need cells loaded ahead of their path
- **Performance Optimization**: Parked vehicles should disable streaming to save resources
- **World Partition Integration**: Works with UE5's streaming system

### Performance Considerations

| Scenario | Without Optimization | With Event-Driven + Hibernation |
|----------|---------------------|--------------------------------|
| 100 parked vehicles | 0.5-1.0ms/frame | 0ms/frame |
| 1 fast vehicle | Normal loading | Predictive loading |
| Stop-and-go traffic | Rapid toggling | Hysteresis prevents flicker |

## Adding Component to Vehicle

### C++ Constructor

```cpp
#include "Components/GSDStreamingSourceComponent.h"

AVehicleBase::AVehicleBase()
{
    StreamingSource = CreateDefaultSubobject<UGSDStreamingSourceComponent>(TEXT("StreamingSource"));
    StreamingSource->SetupAttachment(RootComponent);
}
```

### Blueprint

1. Add **GSDStreamingSourceComponent** to your vehicle Blueprint
2. Configure properties in the Details panel

## EVENT-DRIVEN State Changes (CRITICAL)

### The Right Way: Event-Driven

```cpp
// CORRECT: Event-driven from vehicle state delegate
void AVehicleBase::OnVehicleStateChanged(EVehicleState NewState)
{
    if (StreamingSource)
    {
        bool bIsDriving = (NewState == EVehicleState::Driving);
        float Velocity = GetVelocity().Size();
        StreamingSource->OnVehicleStateChanged(bIsDriving, Velocity);
    }
}
```

### The Wrong Way: Tick Polling

```cpp
// WRONG: Never poll velocity in Tick
void AVehicleBase::Tick(float DeltaTime)
{
    // DON'T DO THIS - polling 100+ vehicles per frame
    float Speed = GetVelocity().Size();
    if (Speed < Threshold) { ... }  // BAD
}
```

**Why is tick polling bad?**
- 100 vehicles = 100 velocity checks per frame
- Each check involves actor component lookups
- Adds 0.5-1.0ms per frame with many vehicles
- Event-driven pattern = 0ms when nothing changes

## Hysteresis System

The hysteresis system prevents rapid enable/disable cycling:

```
Vehicle Stops
     │
     ▼  [5 second delay]
Streaming Disabled
     │
     ▼  [30 second delay]
Hibernation Mode
```

### Configuration

| Property | Default | Description |
|----------|---------|-------------|
| `ParkingHysteresisDelay` | 5.0s | Delay before disabling streaming |
| `HibernationDelay` | 30.0s | Delay before hibernation mode |

### Why Hysteresis?

Without hysteresis, stop-and-go traffic causes:
- Rapid enable/disable cycles
- Streaming source re-registration overhead
- Visible pop-in when rapidly stopping/starting

With hysteresis:
- 5 second grace period
- No cycling in normal traffic
- Smooth streaming behavior

## Hibernation System

Hibernation completely disables streaming for long-parked vehicles:

### Features

- **Zero overhead** when hibernating
- **Automatic wake** when vehicle starts driving
- **Configurable delay** (default 30 seconds)

### Manual Control

```cpp
// Force hibernation immediately
StreamingSource->EnableHibernationMode(0.0f);

// Cancel pending hibernation
StreamingSource->CancelHibernation();

// Check if hibernating
if (StreamingSource->IsHibernating())
{
    // Vehicle is in deep sleep mode
}
```

## Fast Vehicle Configuration

Fast vehicles (sports cars, etc.) get extended loading range:

### C++ Setup

```cpp
void AVehicleBase::BeginPlay()
{
    Super::BeginPlay();

    if (StreamingSource)
    {
        // Configure for fast vehicle
        bool bIsFastVehicle = TopSpeed > 5000.0f; // 50 m/s = 180 km/h
        StreamingSource->ConfigureForVehicle(bIsFastVehicle, 1000.0f);
    }
}
```

### Properties

| Property | Default | Description |
|----------|---------|-------------|
| `FastVehicleThreshold` | 2000.0 | Velocity for "fast" (cm/s) |
| `FastVehicleRangeMultiplier` | 2.0 | Range multiplier for fast vehicles |

### How It Works

1. **Normal speed**: Standard loading range
2. **Above FastVehicleThreshold**: Range doubled
3. **Predictive loading**: Cells loaded ahead of movement

## Performance Best Practices

### DO

- Use `OnVehicleStateChanged()` from state change delegates
- Configure vehicle type at initialization
- Let hysteresis handle parking transitions
- Use hibernation for long-term parked vehicles

### DON'T

- Poll velocity in Tick
- Manually enable/disable without state tracking
- Skip the hysteresis system
- Ignore hibernation for AI vehicles

### Performance Impact

| Vehicle Count | Bad Pattern | Good Pattern |
|---------------|-------------|--------------|
| 10 vehicles | 0.1ms/frame | 0ms/frame |
| 50 vehicles | 0.3ms/frame | 0ms/frame |
| 100 vehicles | 0.5-1.0ms/frame | 0ms/frame |

## Blueprint Integration

All functions are Blueprint-callable:

1. **ConfigureForVehicle** - Set up vehicle streaming
2. **OnVehicleStateChanged** - Update on state change
3. **IsHibernating** - Check hibernation status
4. **CancelHibernation** - Wake from hibernation

### Blueprint Event Setup

1. Create a **Custom Event** in your vehicle Blueprint
2. Call it from your state machine or input handling
3. Connect to `OnVehicleStateChanged` on the streaming component

## Summary

| Feature | Purpose | Impact |
|---------|---------|--------|
| Event-driven pattern | Avoid tick polling | 0ms overhead when idle |
| Hysteresis | Prevent rapid toggling | Smooth transitions |
| Hibernation | Long-term parking | Zero overhead |
| Fast vehicle config | Extended loading range | Better streaming |

For questions or issues, see the GSD_CityStreaming plugin documentation.
