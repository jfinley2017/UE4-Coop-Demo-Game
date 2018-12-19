#include "SCharacter.h"
#include "Camera/CameraComponent.h"
#include "SWeapon.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "SHealthComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UnrealNetwork.h"
#include "SWeaponComponent.h"
#include "SWeaponWidget.h"
#include "Gameplay/GameplayComponents/TeamComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "UnrealNames.h"
#include "GameFramework/PlayerState.h"
#include "Materials/MaterialInstanceDynamic.h"

ASCharacter::ASCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

    SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
    SpringArmComp->SetupAttachment(RootComponent);
    SpringArmComp->bUsePawnControlRotation = true;

    CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
    CameraComp->SetupAttachment(SpringArmComp);

    HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComponent"));
    WeaponComp = CreateDefaultSubobject<USWeaponComponent>(TEXT("WeaponComponent"));
    TeamComp = CreateDefaultSubobject<UTeamComponent>(TEXT("TeamComponent"));

	SetReplicates(true);
    GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

}

void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);
    PlayerInputComponent->BindAxis("LookUp", this, &ASCharacter::AddControllerPitchInput);
    PlayerInputComponent->BindAxis("Turn", this, &ASCharacter::AddControllerYawInput);

    PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ASCharacter::BeginSprint);
    PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ASCharacter::EndSprint);
    PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASCharacter::BeginCrouch);
    PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASCharacter::EndCrouch);
    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASCharacter::Jump);
    PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &ASCharacter::BeginZoom);
    PlayerInputComponent->BindAction("Zoom", IE_Released, this, &ASCharacter::EndZoom);

	FInputActionBinding ReloadPressedAB("Reload", IE_Pressed);

	ReloadPressedAB.ActionDelegate.GetDelegateForManualSet().BindLambda([&]()
	{
		WeaponComp->GetCurrentWeapon()->Reload();
	});

	PlayerInputComponent->AddActionBinding(ReloadPressedAB);

    PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASCharacter::StartFire);
    PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASCharacter::StopFire);

    PlayerInputComponent->BindAction("WeaponSwap", IE_Pressed, this, &ASCharacter::ChangeWeapon);
}

void ASCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASCharacter, bDied);
	DOREPLIFETIME_CONDITION(ASCharacter, bWantsToZoom, COND_SkipOwner);
}

void ASCharacter::BeginPlay()
{
	Super::BeginPlay();

	DefaultFOV = CameraComp->FieldOfView;
    DefaultCameraPosition = CameraComp->RelativeLocation;

	UMaterialInstanceDynamic* MatInstance = GetMesh()->CreateDynamicMaterialInstance(0, GetMesh()->GetMaterial(0));

	if (MatInstance)
	{
		MatInstance->SetVectorParameterValue("BodyColor", BodyColor);
		GetMesh()->SetMaterial(0, MatInstance);
	}

    HealthComp->OnHealthChanged.AddDynamic(this, &ASCharacter::OnHealthChanged);
}

void ASCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (Role == ROLE_SimulatedProxy && IsPlayerControlled())
	{
		GetMesh()->SetRenderCustomDepth(true);
		GetMesh()->SetCustomDepthStencilValue(0);
	}
}

void ASCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);


    FVector TargetCameraPosition = bWantsToZoom ? ZoomedCameraPosition : DefaultCameraPosition;
    FVector NewPosition = FMath::VInterpTo(CameraComp->RelativeLocation, TargetCameraPosition, DeltaTime, ZoomInterpSpeed);
    CameraComp->SetRelativeLocation(NewPosition, false);


    float TargetFOV = bWantsToZoom ? ZoomedFOV : DefaultFOV;
    float NewFOV = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, ZoomInterpSpeed);
    CameraComp->SetFieldOfView(NewFOV);

}

void ASCharacter::MoveForward(float RelativeSpeed)
{
    AddMovementInput(GetActorForwardVector() * RelativeSpeed);
}

void ASCharacter::MoveRight(float RelativeSpeed)
{
    AddMovementInput(GetActorRightVector() * RelativeSpeed);
}

void ASCharacter::BeginCrouch()
{
    Crouch();
}

void ASCharacter::EndCrouch()
{
    UnCrouch();
}

FVector ASCharacter::GetSize()
{
    return FVector(
        GetCapsuleComponent()->GetScaledCapsuleRadius(),
        GetCapsuleComponent()->GetScaledCapsuleRadius(),
        GetCapsuleComponent()->GetScaledCapsuleHalfHeight()*2);
}

float ASCharacter::GetReloadSpeed()
{
	if (WeaponComp && WeaponComp->GetCurrentWeapon())
	{
		return WeaponComp->GetCurrentWeapon()->TimeToReload;
	}

	return 10.0f; // this should make the anim look horribly wrong so that something is obviously wrong
}

bool ASCharacter::GetIsReloading()
{
	if (WeaponComp && WeaponComp->GetCurrentWeapon())
	{
		return WeaponComp->GetCurrentWeapon()->bIsReloading;
	}

	return false;
}

void ASCharacter::StartFire()
{
    if (WeaponComp)
    {
        WeaponComp->StartFire();
    }
}

void ASCharacter::StopFire()
{
    if (WeaponComp)
    {
        WeaponComp->StopFire();
    }
}

FVector ASCharacter::GetPawnViewLocation() const
{
	return CameraComp ? CameraComp->GetComponentLocation() : Super::GetPawnViewLocation();
}

void ASCharacter::ChangeWeapon()
{
    if (WeaponComp)
    {
        WeaponComp->ChangeWeapon();
    }
}

void ASCharacter::DisableInput(APlayerController* PlayerController)
{
	Super::DisableInput(PlayerController);

	StopFire();
}

void ASCharacter::OnHealthChanged(USHealthComponent * ChangedHealthComp, float Health, float HealthDelta, const UDamageType * DamageType, AController * InstigatedBy, AActor * DamageCauser)
{
    if (Health <= 0.0f && !bDied)
    {
        bDied = true;
        StopFire();

        // Die
        GetMovementComponent()->StopMovementImmediately();

        APlayerController* PC = Cast<APlayerController>(GetController());

        if (PC)
        {
            PC->PlayerState->bIsSpectator = true;
            PC->ChangeState(NAME_Spectating);
            PC->ClientGotoState(NAME_Spectating);
        }

        GetMesh()->SetCollisionResponseToAllChannels(ECR_Ignore);
        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        DetachFromControllerPendingDestroy();
        PrimaryActorTick.bCanEverTick = false;
        SetLifeSpan(3.0f);
    }
}

/** This is empty for now, but the hook remains in case something like stuns appear */
bool ASCharacter::CanSprint()
{
    return true;
}

void ASCharacter::BeginSprint()
{
    if (bWantsToZoom)
    {
        EndZoom();
    }

    if (!HasAuthority())
    {
        ServerSetSprint(true);
    }

    bWantsToSprint = true;
    GetCharacterMovement()->MaxWalkSpeed += SprintSpeedModifier;
}

void ASCharacter::EndSprint()
{
    if (!bWantsToSprint) { return; }

    if (!HasAuthority())
    {
        ServerSetSprint(false);
    }

    bWantsToSprint = false;
    GetCharacterMovement()->MaxWalkSpeed += -SprintSpeedModifier;
}

void ASCharacter::ServerSetSprint_Implementation(bool bSprint)
{
    if (bSprint && CanSprint())
    {
        BeginSprint();
    }
    else
    {
        EndSprint();
    }
}
bool ASCharacter::ServerSetSprint_Validate(bool bSprint) { return true; }

/** This is empty for now, but the hook remains in case something like stuns appear */
bool ASCharacter::CanZoom()
{
    return true;
}

void ASCharacter::BeginZoom()
{
    if (bWantsToSprint)
    {
        EndSprint();
    }

    if (!HasAuthority())
    {
        ServerSetZoom(true);
    }

    bWantsToZoom = true;
    GetCharacterMovement()->MaxWalkSpeed += ZoomedMovespeedPenalty;
}

void ASCharacter::EndZoom()
{
    if (!bWantsToZoom) { return; }

    if (!HasAuthority())
    {
        ServerSetZoom(false);
    }

    bWantsToZoom = false;
    GetCharacterMovement()->MaxWalkSpeed += -ZoomedMovespeedPenalty;
}

void ASCharacter::ServerSetZoom_Implementation(bool bZoom)
{
    if (bZoom && CanZoom())
    {
        BeginZoom();
    }
    else
    {
        EndZoom();
    }
}

bool ASCharacter::ServerSetZoom_Validate(bool bZoom) { return true; }

