// Fill out your copyright notice in the Description page of Project Settings.

#include "SPlayerController.h"
#include "Engine.h"
#include "SGameState.h"
#include "Blueprint/UserWidget.h"
#include "SPlayerState.h"

ASPlayerController::ASPlayerController()
    : APlayerController()
{
    SetActorTickEnabled(true);
    PrimaryActorTick.bCanEverTick = true;
}

void ASPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

}


void ASPlayerController::BeginPlay()
{
    Super::BeginPlay();


    if (GameEventWidgetClass && IsLocalController())
    {
        GameEventWidget = CreateWidget<UUserWidget>(this, GameEventWidgetClass);
        GameEventWidget->AddToViewport();
    }

    ASGameState* GS = GetWorld()->GetGameState<ASGameState>();
    if (ensureAlways(GS))
    {
        GS->OnTeamGameOver.AddDynamic(this, &ASPlayerController::RecieveGameOver);
    }

    // Update our player state when we possess a new pawn
    ASPlayerState* PS = Cast<ASPlayerState>(PlayerState);
    if (PS)
    {
        OnPawnChanged.AddDynamic(PS, &ASPlayerState::SetCurrentPawn);
        PS->SetCurrentPawn(GetPawn());
    }
}

void ASPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    ensure(InputComponent);
    InputComponent->BindAction("ToggleMenu", IE_Pressed, this, &ASPlayerController::ToggleMenu);
}

void ASPlayerController::RecieveGameOver(ASTeam* WinningTeam)
{
   UE_LOG(LogTemp, Warning, TEXT("GameOver"));
   if (GetPawn())
   {
       GetPawn()->DisableInput(this);
   }
   OnRecieveGameOver(WinningTeam);
}

void ASPlayerController::ToggleMenu()
{
    if (!MenuWidget)
    {
        MenuWidget = CreateWidget<UUserWidget>(this, MenuWidgetClass);
        if (MenuWidget)
        {
            MenuWidget->AddToViewport();
            MenuWidget->SetVisibility(ESlateVisibility::Visible);
            GEngine->GameViewport->Viewport->LockMouseToViewport(false);
            bShowMouseCursor = true;
        }
    }
    else
    {
        if (MenuWidget->IsVisible())
        {
            MenuWidget->SetVisibility(ESlateVisibility::Hidden);
            GEngine->GameViewport->Viewport->LockMouseToViewport(true);
            bShowMouseCursor = false;
        }
        else
        {
            MenuWidget->SetVisibility(ESlateVisibility::Visible);
            GEngine->GameViewport->Viewport->LockMouseToViewport(false);
            bShowMouseCursor = true;
        }
    }
}

void ASPlayerController::SetPawn(APawn* InPawn)
{
    Super::SetPawn(InPawn);
    OnPawnChanged.Broadcast(InPawn);
    OnPawnChange();
}