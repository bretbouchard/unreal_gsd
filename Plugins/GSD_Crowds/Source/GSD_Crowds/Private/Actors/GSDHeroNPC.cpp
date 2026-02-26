// Copyright Bret Bouchard. All Rights Reserved.

#include "Actors/GSDHeroNPC.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "AI/GSDHeroAIController.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"

AGSDHeroNPC::AGSDHeroNPC()
{
    // Disable tick for performance
    PrimaryActorTick.bCanEverTick = false;

    // Create capsule component (root)
    CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
    RootComponent = CapsuleComponent;
    CapsuleComponent->InitCapsuleSize(DefaultCapsuleRadius, DefaultCapsuleHalfHeight);
    CapsuleComponent->SetCollisionProfileName(TEXT("Pawn"));

    // Create skeletal mesh
    SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
    SkeletalMesh->SetupAttachment(RootComponent);

    // Create character movement
    CharacterMovement = CreateDefaultSubobject<UCharacterMovementComponent>(TEXT("CharacterMovement"));
    CharacterMovement->UpdatedComponent = RootComponent;
    CharacterMovement->MaxWalkSpeed = 400.0f;

    // Create perception stimuli source (CRITICAL for being detected - Pitfall 3)
    PerceptionStimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(
        TEXT("PerceptionStimuliSource")
    );

    // Register for sight and hearing senses
    PerceptionStimuliSource->RegisterForSense(UAISense_Sight::StaticClass());
    PerceptionStimuliSource->RegisterForSense(UAISense_Hearing::StaticClass());
    PerceptionStimuliSource->RegisterWithPerceptionSystem();

    // Set default AI controller
    AIControllerClass = AGSDHeroAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    bIsSpawned = false;
}

void AGSDHeroNPC::BeginPlay()
{
    Super::BeginPlay();

    // Ensure stimuli source is registered (redundant but safe)
    if (PerceptionStimuliSource)
    {
        PerceptionStimuliSource->RegisterWithPerceptionSystem();
    }
}

void AGSDHeroNPC::SpawnFromConfig(UGSDDataAsset* Config)
{
    if (!Config)
    {
        return;
    }

    // Store the config
    SpawnConfigAsset = Config;

    // Mark as spawned
    bIsSpawned = true;

    // Config application logic can be extended in Blueprint subclasses
    // Example: Set appearance, behavior tree override, stats, etc.
}

UGSDDataAsset* AGSDHeroNPC::GetSpawnConfig()
{
    return SpawnConfigAsset;
}

bool AGSDHeroNPC::IsSpawned()
{
    return bIsSpawned;
}

void AGSDHeroNPC::Despawn()
{
    bIsSpawned = false;
    SpawnConfigAsset = nullptr;
}

void AGSDHeroNPC::ResetSpawnState()
{
    Despawn();
}
