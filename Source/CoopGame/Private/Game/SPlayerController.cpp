#include "SPlayerController.h"
#include "Engine.h"
#include "SGameState.h"
#include "Blueprint/UserWidget.h"
#include "SPlayerState.h"
#include "Public/Config.h"

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

void ASPlayerController::SetMouseSensitivity()
{
	UConfig* Conf = UConfig::GetConfigSingleton();
	float Sens = Conf->GetMouseSensitivity();
	InputYawScale = Sens;
	InputPitchScale = -Sens; // This is not a mistake - it will invert pitch without -
}

void ASPlayerController::BeginPlay()
{
    Super::BeginPlay();

	SetMouseSensitivity();

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
	InputComponent->BindAction("Ready", IE_Pressed, this, &ASPlayerController::OnReadyForWaves);
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
			MenuWidget = nullptr;
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

void ASPlayerController::OnReadyForWaves()
{
	OnPlayerReady.Broadcast(this);

	if (!HasAuthority())
	{
		ServerOnReadyForWaves();
	}
}

void ASPlayerController::ServerOnReadyForWaves_Implementation()
{
	OnReadyForWaves();
}

bool ASPlayerController::ServerOnReadyForWaves_Validate()
{
	return true;
}


void ASPlayerController::SetPawn(APawn* InPawn)
{
    Super::SetPawn(InPawn);
    OnPawnChanged.Broadcast(InPawn);
    OnPawnChange();
}
