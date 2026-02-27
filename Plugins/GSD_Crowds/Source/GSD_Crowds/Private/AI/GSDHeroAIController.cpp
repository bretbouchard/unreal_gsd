// Copyright Bret Bouchard. All Rights Reserved.

#include "AI/GSDHeroAIController.h"
#include "DataAssets/GSDHeroAIConfig.h"
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
    // Create components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));

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
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;

    // Apply configs to perception
    PerceptionComponent->ConfigureSense(*SightConfig);
    PerceptionComponent->ConfigureSense(*HearingConfig);
    PerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Initialize state
    bBehaviorTreeRunning = false;
}

void AGSDHeroAIController::BeginPlay()
{
    Super::BeginPlay();

    // Initialize perception if config is available
    if (AIConfig)
    {
        InitializePerception();
    }
    else
    {
        // Bind perception delegate with default settings
        PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(
            this,
            &AGSDHeroAIController::OnTargetPerceptionUpdated
        );
    }
}

void AGSDHeroAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // CRITICAL: Run behavior tree immediately on possess (Pitfall 5 from research)
    UBehaviorTree* BTreeToRun = nullptr;

    // Priority: AIConfig -> Legacy BehaviorTree property
    if (AIConfig && AIConfig->DefaultBehaviorTree)
    {
        BTreeToRun = AIConfig->DefaultBehaviorTree;
    }
    else if (BehaviorTree)
    {
        BTreeToRun = BehaviorTree;
    }

    if (BTreeToRun)
    {
        RunBehaviorTreeAsset(BTreeToRun);
    }
}

void AGSDHeroAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Optional: Add tick logic for AI decision making
    // This can be used for custom AI behaviors not covered by BT
}

bool AGSDHeroAIController::RunBehaviorTreeAsset(UBehaviorTree* BTAsset)
{
    if (!BTAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("RunBehaviorTreeAsset: Null Behavior Tree"));
        return false;
    }

    // Initialize blackboard
    if (!BlackboardComponent->InitializeBlackboard(*BTAsset->BlackboardAsset))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to initialize blackboard for BT: %s"), *BTAsset->GetName());
        return false;
    }

    // Start behavior tree
    BehaviorTreeComponent->StartTree(*BTAsset);
    bBehaviorTreeRunning = true;

    UE_LOG(LogTemp, Log, TEXT("Behavior Tree started: %s"), *BTAsset->GetName());
    return true;
}

void AGSDHeroAIController::StopBehaviorTree()
{
    if (BehaviorTreeComponent)
    {
        BehaviorTreeComponent->StopTree();
        bBehaviorTreeRunning = false;
    }
}

TArray<AActor*> AGSDHeroAIController::GetPerceivedActors(TSubclassOf<UAISense> Sense) const
{
    TArray<AActor*> PerceivedActors;
    if (PerceptionComponent)
    {
        PerceptionComponent->GetCurrentlyPerceivedActors(Sense, PerceivedActors);
    }
    return PerceivedActors;
}

bool AGSDHeroAIController::HasLineOfSightTo(AActor* Target) const
{
    if (!Target || !GetPawn())
    {
        return false;
    }

    FHitResult HitResult;
    FVector StartLocation = GetPawn()->GetActorLocation();
    FVector EndLocation = Target->GetActorLocation();

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetPawn());

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECC_Visibility,
        QueryParams
    );

    return !bHit || HitResult.GetActor() == Target;
}

void AGSDHeroAIController::SetAIConfig(UGSDHeroAIConfig* NewConfig)
{
    AIConfig = NewConfig;

    if (AIConfig)
    {
        InitializePerception();

        // Restart behavior tree if new config has different BT
        if (AIConfig->DefaultBehaviorTree && bBehaviorTreeRunning)
        {
            RunBehaviorTreeAsset(AIConfig->DefaultBehaviorTree);
        }
        else if (AIConfig->DefaultBehaviorTree && !bBehaviorTreeRunning)
        {
            // Start BT if not running
            RunBehaviorTreeAsset(AIConfig->DefaultBehaviorTree);
        }
    }
}

void AGSDHeroAIController::InitializePerception()
{
    if (!AIConfig || !PerceptionComponent)
    {
        return;
    }

    // Configure senses from AIConfig
    ConfigureSight();
    ConfigureHearing();

    // Bind perception delegate
    PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(
        this,
        &AGSDHeroAIController::OnTargetPerceptionUpdated
    );
}

void AGSDHeroAIController::ConfigureSight()
{
    if (!SightConfig || !AIConfig)
    {
        return;
    }

    // Configure sight from AIConfig
    SightConfig->SightRadius = AIConfig->SightRadius;
    SightConfig->LoseSightRadius = AIConfig->LoseSightRadius;
    SightConfig->PeripheralVisionAngleDegrees = AIConfig->PeripheralVisionAngle;

    // Detection settings
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

    // Set auto success range for close-range detection
    SightConfig->AutoSuccessRangeFromLastSeenLocation = 500.0f;

    // Apply to perception component
    PerceptionComponent->ConfigureSense(*SightConfig);
}

void AGSDHeroAIController::ConfigureHearing()
{
    if (!HearingConfig || !AIConfig)
    {
        return;
    }

    // Configure hearing from AIConfig
    HearingConfig->HearingRange = AIConfig->HearingRange;
    HearingConfig->SetMaxAge(AIConfig->HearingStimulusMaxAge);

    // Detection settings
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;

    // Apply to perception component
    PerceptionComponent->ConfigureSense(*HearingConfig);
}

void AGSDHeroAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor)
    {
        return;
    }

    UBlackboardComponent* BB = BlackboardComponent;
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

            UE_LOG(LogTemp, Log, TEXT("Perceived actor via sight: %s"), *Actor->GetName());
        }
        else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
        {
            // Hearing detection
            BB->SetValueAsBool(HeardNoiseKey, true);
            BB->SetValueAsVector(NoiseLocationKey, Stimulus.StimulusLocation);

            UE_LOG(LogTemp, Log, TEXT("Perceived actor via hearing: %s"), *Actor->GetName());
        }
    }
    else
    {
        // Lost perception
        if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
        {
            BB->SetValueAsBool(CanSeeTargetKey, false);
            BB->SetValueAsVector(LastKnownPositionKey, Stimulus.StimulusLocation);

            UE_LOG(LogTemp, Log, TEXT("Lost sight of actor: %s"), *Actor->GetName());
        }
    }
}
