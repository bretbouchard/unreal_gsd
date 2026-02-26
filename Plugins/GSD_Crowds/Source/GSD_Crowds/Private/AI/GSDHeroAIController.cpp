// Copyright Bret Bouchard. All Rights Reserved.

#include "AI/GSDHeroAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"

// Blackboard key names
const FName AGSDHeroAIController::TargetActorKey = FName("TargetActor");
const FName AGSDHeroAIController::TargetLocationKey = FName("TargetLocation");
const FName AGSDHeroAIController::CanSeeTargetKey = FName("CanSeeTarget");
const FName AGSDHeroAIController::LastKnownPositionKey = FName("LastKnownPosition");
const FName AGSDHeroAIController::HeardNoiseKey = FName("HeardNoise");
const FName AGSDHeroAIController::NoiseLocationKey = FName("NoiseLocation");

AGSDHeroAIController::AGSDHeroAIController()
{
    // Create perception component
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));

    // Create sight config
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = DefaultSightRadius;
    SightConfig->LoseSightRadius = DefaultLoseSightRadius;
    SightConfig->PeripheralVisionAngleDegrees = DefaultPeripheralVisionAngle;
    SightConfig->SetMaxAge(DefaultStimulusMaxAge);
    SightConfig->AutoSuccessRangeFromLastSeenLocation = 500.0f;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;

    // Create hearing config
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = DefaultHearingRange;
    HearingConfig->SetMaxAge(3.0f);

    // Apply configs to perception
    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Bind perception delegate
    AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(
        this,
        &AGSDHeroAIController::OnTargetPerceptionUpdated
    );
}

void AGSDHeroAIController::BeginPlay()
{
    Super::BeginPlay();
}

void AGSDHeroAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // CRITICAL: Must start Behavior Tree in OnPossess (Pitfall 5 from research)
    if (BehaviorTree)
    {
        // Use the BT's Blackboard asset
        UBlackboardComponent* BBComponent = nullptr;
        UseBlackboard(BehaviorTree->BlackboardAsset, BBComponent);

        if (BBComponent)
        {
            RunBehaviorTree(BehaviorTree);
        }
    }
}

void AGSDHeroAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor)
    {
        return;
    }

    UBlackboardComponent* BB = GetBlackboardComponent();
    if (!BB)
    {
        return;
    }

    if (Stimulus.WasSuccessfullySensed())
    {
        // Actor detected
        if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
        {
            // Sight detection
            BB->SetValueAsObject(TargetActorKey, Actor);
            BB->SetValueAsVector(TargetLocationKey, Stimulus.StimulusLocation);
            BB->SetValueAsBool(CanSeeTargetKey, true);
            BB->SetValueAsBool(HeardNoiseKey, false);
        }
        else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
        {
            // Hearing detection
            BB->SetValueAsBool(HeardNoiseKey, true);
            BB->SetValueAsVector(NoiseLocationKey, Stimulus.StimulusLocation);
        }
    }
    else
    {
        // Lost perception
        if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
        {
            BB->SetValueAsBool(CanSeeTargetKey, false);
            BB->SetValueAsVector(LastKnownPositionKey, Stimulus.StimulusLocation);
        }
    }
}
