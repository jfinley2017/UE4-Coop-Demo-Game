#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DamageDealer.h"
#include "Game/SPawn.h"
#include "Library/SDamageTypes.h"
#include "SCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class USWeaponComponent;
class UHealthComponent;
class UTeamComponent;
class UAnimMontage;
class USoundBase;

UCLASS()
class COOPGAME_API ASCharacter : public ACharacter, public IDamageDealer, public ISPawn
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacter();

    virtual void Tick(float DeltaTime) override;
    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void AddControllerPitchInput(float Val);
	virtual void AddControllerYawInput(float Val);

    virtual FVector GetPawnViewLocation() const override;
    virtual void GetActorEyesViewPoint(FVector&, FRotator& ) const override;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PlayerWeapon")
    float GetReloadSpeed();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PlayerWeapon")
    bool GetIsReloading();

    /**  Weapon operations, calling these from character for now because we might want to play animations/whatever
     *	 before firing/on changing weapons
    */
    UFUNCTION(BlueprintCallable, Category = "PlayerWeapon")
    void StartFire();

    UFUNCTION(BlueprintCallable, Category = "PlayerWeapon")
    void StopFire();

    UFUNCTION()
    void Reload();

    void ChangeWeapon();

    /* Override so that we can stop firing when disabled */
    virtual void DisableInput(APlayerController* PlayerController) override;

    /** Implements IDamageDealer */
    virtual float GetDamageModifier() override { return DamageModifier; }

    /** Implements ISPawn */
    /** Gets the current size of this unit */
    virtual FVector GetSize();

    /** Gets the current bounty worth of this unit */
	virtual float GetOnKillScore() override { return KillBounty; }

	/** Whether or not this character wants to zoom in */
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "PlayerWeapon")
	bool bWantsToZoom = false;

protected:
	float MouseSensitivity = 1.0f;

    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    UCameraComponent* CameraComp = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpringArmComponent")
    USpringArmComponent* SpringArmComp = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapons")
    USWeaponComponent* WeaponComp = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
    USHealthComponent* HealthComp = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Team")
    UTeamComponent* TeamComp = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player")
	USoundBase* PlayerDeathSound = nullptr;

    //////////////////////////////
    /** Gameplay Variables */

    /** Position the camera should interp to when zoomed in */
    UPROPERTY(EditDefaultsOnly, Category = "Player")
    FVector ZoomedCameraPosition;

    /** The default camera position, this is set on spawn automatically based on default camera position. Used to interp back after a zoom */
    FVector DefaultCameraPosition;

    /** How fast should we interp between default and zoomed camera positions? */
    UPROPERTY(EditDefaultsOnly, Category = "Player", meta = (ClampMin = 0.0, ClampMax = 100))
    float ZoomInterpSpeed = 20.0f;

    /** When zoomed, how much should we cut movespeed */
    UPROPERTY(EditDefaultsOnly, Category = "Player")
    float ZoomedMovespeedPenalty = -400.0f;

    /** The default FOV, this is set on spawn automatically based on default camera FOV. Used to interp back after a zoom */
    float DefaultFOV;

    /** FOV we should interp to on zoom */
    UPROPERTY(EditDefaultsOnly, Category = "Player")
    float ZoomedFOV = 65.0f;

    /** Amount that the characters speed should change on sprint*/
    UPROPERTY(EditDefaultsOnly, Category = "Player")
    float SprintSpeedModifier = 200.0f;

    // SPawn variables
    /** How much this unit is worth on kill */
    UPROPERTY(EditDefaultsOnly, Category = "SPawn")
    float KillBounty = 20.0f;

    /** Default color of this character */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player", meta = (ExposeOnSpawn = true))
    FLinearColor BodyColor = FLinearColor(1.0f, 1.0f, 1.0f, 0);

    // IDamageDealer variables 
    /** This unit's current damage modifier */
    UPROPERTY(BlueprintReadWrite, Category = "Power")
    float DamageModifier = 1.0f;

    //////////////////////////////
    // ActionState Flags

    /** Whether the character wants to sprint or not */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
    bool bWantsToSprint = false;

    /** Whether the character has died or not */
    UPROPERTY(ReplicatedUsing=OnRep_bDied, BlueprintReadOnly, Category = "Player")
    bool bDied = false;

	UFUNCTION()
	void NotifyDied(const FSDamageInstance& Damage);

    /** Hook to be called when this unit takes damage */
    UFUNCTION()
    void NotifyHealthChanged(USHealthComponent* ChangedHealthComp, float Health, float MaxHealth);
    
    //////////////////////////////
    /** Action functions */ 
    void MoveForward(float RelativeSpeed);
    void MoveRight(float RelativeSpeed);

    void BeginCrouch();
    void EndCrouch();

    void BeginZoom();
    void EndZoom();

    void BeginSprint();
    void EndSprint();




    UFUNCTION(BlueprintCallable, Category = "PlayerActions")
    bool CanSprint();

    UFUNCTION(BlueprintCallable, Category = "PlayerActions")
    bool CanZoom();

    /** RPC to tell the server that we intend to sprint */
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerSetSprint(bool bSprint);

    /** RPC to tell the server that we intend to zoom in */
    UFUNCTION(Server, Unreliable, WithValidation)
    void ServerSetZoom(bool bZoom);

    UFUNCTION()
    void OnRep_bDied();

    // Override to set player glows, won't work if other team also has player controlled SCharacters
    virtual void OnRep_PlayerState() override;

};
