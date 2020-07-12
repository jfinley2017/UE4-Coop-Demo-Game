#include "STrackerBot.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "GameFramework/Character.h"
#include "NavigationPath.h"
#include "SHealthComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "CoopGame.h"
#include "GameFramework/GameState.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Sound/SoundCue.h"
#include "TeamComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "TimerManager.h"
#include "Game/Modes/SGameState.h"
#include "Net/UnrealNetwork.h"

static int32 DebugTrackerBot = 0;
FAutoConsoleVariableRef CVARDebugTrackerBot(
	TEXT("COOP.DebugTrackerBot"),
	DebugTrackerBot,
	TEXT("Draws debug objects for tracker bot and enable verbose logs"),
	ECVF_Cheat);

ASTrackerBot::ASTrackerBot()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.1f;

    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComp->SetCanEverAffectNavigation(false);
    MeshComp->SetSimulatePhysics(true);
    RootComponent = MeshComp;

    HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("Health Component"));
    HealthComp->OnHealthChanged.AddDynamic(this, &ASTrackerBot::OnTakeDamage);
    
    TeamComp = CreateDefaultSubobject<UTeamComponent>(TEXT("Team Component"));

    ProximityExplosionRadius = CreateDefaultSubobject<USphereComponent>(TEXT("ProximityRadius"));
    ProximityExplosionRadius->SetupAttachment(RootComponent);
    ProximityExplosionRadius->SetCollisionResponseToAllChannels(ECR_Ignore);
    ProximityExplosionRadius->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

// Called when the game starts or when spawned
void ASTrackerBot::BeginPlay()
{
	Super::BeginPlay();

    ProximityExplosionRadius->OnComponentBeginOverlap.AddDynamic(this, &ASTrackerBot::OnProximityRadiusOverlap);
    ProximityExplosionRadius->SetSphereRadius(ProximityRadius, true);

    if (Role == ROLE_Authority)
    {
        NextPathPoint = GetNextPathPoint();
    }

    if (!MatInstance && MeshComp)
    {
        MatInstance = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
    }
}

void ASTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bSelfDestructionAttached && !bExploded && Role == ROLE_Authority)
	{
		MoveTowardsTarget(DeltaTime);
	}
}

void ASTrackerBot::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASTrackerBot, bExploded);
	DOREPLIFETIME(ASTrackerBot, ExplodeTime);
	DOREPLIFETIME(ASTrackerBot, bSelfDestructionAttached);
}

AActor* ASTrackerBot::FindBestTarget()
{
    AActor* BestTarget = nullptr;
    float NearestTargetDistance = FLT_MAX;

    for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
    {
        APawn* TestPawn = It->Get();
        
		if (!TestPawn || TestPawn == this || 
			!UTeamComponent::AreActorTeamsValid(this, TestPawn) || UTeamComponent::IsActorFriendly(this, TestPawn))
        {
            continue;
        }

        USHealthComponent* PawnHealthComp = TestPawn->FindComponentByClass<USHealthComponent>();

		if (PawnHealthComp && PawnHealthComp->GetHealth() > 0)
        {
            // Target is still alive, check and store nearest target if this target is closer than the previous nearest
            float Distance = (TestPawn->GetActorLocation() - GetActorLocation()).Size();
            
			if (NearestTargetDistance > Distance)
            {
                BestTarget = TestPawn;
                NearestTargetDistance = Distance;
            }
        }
    }

	if (DebugTrackerBot && BestTarget)
	{
		UE_LOG(TrackerBot, Log, TEXT("Tracker bot found best target %s"), *BestTarget->GetName());
	}

	return BestTarget;
}

FVector ASTrackerBot::GetNextPathPoint()
{
    GetWorldTimerManager().ClearTimer(TimerHandle_RefreshPath);
	GetWorldTimerManager().SetTimer(TimerHandle_RefreshPath, this, &ASTrackerBot::RefreshPath, RefreshInterval, false);

	AActor* BestTarget = FindBestTarget();

	if (!BestTarget)
	{
		return GetActorLocation();
	}

	UNavigationPath* NavPath = UNavigationSystemV1::FindPathToActorSynchronously(GetWorld(), GetActorLocation(), BestTarget);

	if (NavPath && NavPath->PathPoints.Num() > 0)
	{
		if (DebugTrackerBot)
		{
			DrawDebugSphere(GetWorld(), NavPath->PathPoints[1], 100, 12, FColor::Red, true, 5.0f, 0, 1);
		}

		return NavPath->PathPoints[1];
	}
	else
	{
		return BestTarget->GetActorLocation();
	}
}

void ASTrackerBot::RefreshPath()
{
	NextPathPoint = GetNextPathPoint();
}

void ASTrackerBot::OnTakeDamage(USHealthComponent * ChangedHealthComp, float Health, float HealthDelta, 
    const UDamageType * DamageType, AController * InstigatedBy, AActor * DamageCauser)
{
    if (Health <= 0)
    {
        SelfDestruct();
    }
}

void ASTrackerBot::SelfDestruct()
{
    if (bExploded) { return; }

    bExploded = true;
    OnRep_Exploded();

    if (Role == ROLE_Authority)
    {
        TArray<AActor*> IgnoredActors = { };
        
		float ActualDamage = ((GetDamageModifier() / 100) * ExplosionDamage) + ExplosionDamage;
		bool bScaleDamageByDistance = bBotAttachesToPlayer;

        UGameplayStatics::ApplyRadialDamage(this, ActualDamage, GetActorLocation(), ExplosionRadius,nullptr, IgnoredActors,this,GetController(), bScaleDamageByDistance);
        // Kill ourselves
        UGameplayStatics::ApplyDamage(this, HealthComp->GetHealth(), GetController(), this, nullptr);
        
        // Give clients a chance to play effects
        SetLifeSpan(4.0);
    }

	MeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
}

void ASTrackerBot::IncreaseMovespeed(float PercentIncrease)
{
	if (DebugTrackerBot)
	{
		UE_LOG(TrackerBot, Log, TEXT("Speed increased %f"), PercentIncrease);

	}
	MovementForce = OriginalMovementForce + OriginalMovementForce * PercentIncrease;
	MaxSpeed = OriginalMaxSpeed + OriginalMaxSpeed * PercentIncrease;
}


void ASTrackerBot::MoveTowardsTarget(float DeltaTime)
{
    float DistanceToTarget = (GetActorLocation() - NextPathPoint).Size();

    if (DistanceToTarget <= RequiredDistanceToTarget)
    {
        NextPathPoint = GetNextPathPoint();
    }

	if (DebugTrackerBot)
	{
		DrawDebugSphere(GetWorld(), NextPathPoint, 100, 12, FColor::Yellow, true, 5.0f);
	}

	FVector TargetLookAtDirection = NextPathPoint - GetActorLocation();
	TargetLookAtDirection.Normalize();

	FVector ActualMoveDirection = GetVelocity();
	float ActualMoveSpeed = ActualMoveDirection.Size();
	ActualMoveDirection.Normalize();

	float SpeedTowardsTarget = (((TargetLookAtDirection+ ActualMoveDirection) / 2) * ActualMoveSpeed).Size();
	
	if (DebugTrackerBot)
	{
		float ArrowLengthScale = 1;
		FVector ALoc = GetActorLocation();
		FVector TargetArrowEnd = ALoc + (TargetLookAtDirection * (MaxSpeed / ArrowLengthScale));
		FVector ActualMovementArrowEnd = ALoc + (GetVelocity() / ArrowLengthScale);
		DrawDebugDirectionalArrow(GetWorld(), ALoc, ActualMovementArrowEnd, 50, FColor::Green);
		DrawDebugDirectionalArrow(GetWorld(), ALoc, TargetArrowEnd, 50, FColor::Yellow);
		
		FVector CorrectionVector = (TargetLookAtDirection * MaxSpeed) - GetVelocity();
		CorrectionVector.Normalize();

		DrawDebugDirectionalArrow(GetWorld(), ActualMovementArrowEnd, ActualMovementArrowEnd +  (CorrectionVector * MaxSpeed / ArrowLengthScale), 50, FColor::Blue);
	}

	if (SpeedTowardsTarget <= MaxSpeed)
	{
		FVector ResultForce = (TargetLookAtDirection * MaxSpeed) - GetVelocity();
		ResultForce.Z = 0; // no flying
		ResultForce.Normalize();
		ResultForce *= MovementForce * (DeltaTime * 100);

		MeshComp->AddForce(ResultForce, NAME_None, bUseVelocityChange);
	}
}

void ASTrackerBot::OnProximityRadiusOverlap(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
    APawn* OtherActorPawn = Cast<APawn>(OtherActor);

	if (bExploded || bTriggered || !OtherActorPawn || OtherActorPawn == this)
	{
		return;
	}

	// IsActorFriendly does check team validity, but it returns unfriendly if invalid
	// Dont explode for invalid teams
	if (!UTeamComponent::AreActorTeamsValid(this, OtherActor) || 
		UTeamComponent::IsActorFriendly(this, OtherActor))
	{
		return;
	}

	UE_LOG(TrackerBot, Warning, TEXT("Triggered by %s"), *OtherActor->GetName());
    
    if (Role == ROLE_Authority)
    {
		if (bBotAttachesToPlayer)
		{
			AttachBotToActor(OtherActorPawn);
		}
            
        GetWorldTimerManager().SetTimer(SelfDestructCountdownTimer, this, &ASTrackerBot::SelfDestruct, SelfDestructTime, false);
        ExplodeTime = GetWorld()->TimeSeconds + SelfDestructTime;
        OnRep_ExplodeTime();
    }

	// Somewhat of a hack, point is to just make the bot flash when the trigger occurs
	SelfDestructTicked(3.0f);

    UGameplayStatics::SpawnSoundAttached(TriggeredSound, RootComponent);
    bTriggered = true;
}

void ASTrackerBot::SelfDestructTick()
{

    UGameplayStatics::SpawnSoundAttached(SelfDestructTickSound, RootComponent);
    float TimeRemainingUntilDestruction = ExplodeTime - GetWorld()->TimeSeconds;
	SelfDestructTicked(TimeRemainingUntilDestruction);

    GetWorldTimerManager().SetTimer(SelfDestructionTickTimer, this, &ASTrackerBot::SelfDestructTick, TimeRemainingUntilDestruction/4, false);
}

void ASTrackerBot::OnRep_ExplodeTime()
{
    TRACE("%s started self destruct. Destruct time : %f", *GetName(), ExplodeTime);
    GetWorldTimerManager().SetTimer(SelfDestructionTickTimer, this, &ASTrackerBot::SelfDestructTick, (ExplodeTime - GetWorld()->TimeSeconds) / 4, false);
}

void ASTrackerBot::OnRep_TrackerBotAttached()
{
    MeshComp->SetSimulatePhysics(false);
    MeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
    MeshComp->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Block);
    MeshComp->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Block);
}

void ASTrackerBot::OnRep_Exploded()
{
	Exploded();
    TRACE("%s self destructed, time: %f", *GetName(), GetWorld()->TimeSeconds)
    UGameplayStatics::PlaySoundAtLocation(this, ExplodeSound, GetActorLocation());
    UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
    GetWorldTimerManager().ClearTimer(SelfDestructCountdownTimer);
    GetWorldTimerManager().ClearTimer(SelfDestructionTickTimer);

    MeshComp->SetVisibility(false);
    MeshComp->SetSimulatePhysics(false);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

bool ASTrackerBot::AttachBotToActor(APawn* OtherPawn)
{
	USkeletalMeshComponent* OtherMesh = OtherPawn->FindComponentByClass<USkeletalMeshComponent>();

	if (OtherMesh)
	{
		bool ActorHasTrackerBot = false;
		TArray<AActor*> AttachedActors;
		OtherPawn->GetAttachedActors(AttachedActors);
		for (AActor* Actor : AttachedActors)
		{
			if (Actor->IsA<ASTrackerBot>())
			{
				ActorHasTrackerBot = true;
				break;
			}
		}

		if (!ActorHasTrackerBot)
		{
			// TODO: Add function on actor (maybe even an interface) to add other actors like this one to sockets 
			// As of right now multiple trigger bots can attach to the back slot and that's bad
			OnRep_TrackerBotAttached();
			AttachToComponent(OtherMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "Back");
			bSelfDestructionAttached = true;
			return true;
		}
	}

	return false;
}

FVector ASTrackerBot::GetSize()
{
    return FVector(MeshComp->Bounds.SphereRadius, MeshComp->Bounds.SphereRadius, MeshComp->Bounds.SphereRadius);
}


