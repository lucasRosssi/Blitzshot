// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "AmmoType.h"
#include "ShooterCharacter.generated.h"

UENUM(BlueprintType)
enum class ECombatState : uint8
{
  ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
  ECS_FireTimerInProgress UMETA(DisplayName = "FireTimerInProgress"),
  ECS_Reloading UMETA(DisplayName = "Reloading"),

  ECS_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class MONSTERSHOOTER_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShooterCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

  // Called for forwards/backwards input
  void Move(const FInputActionValue& Value);
  // Called to handle looking input
  void Look(const FInputActionValue& Value);
  // Called to handle jump input
  void Jump(const FInputActionValue& Value);
  // Set bAiming to true or false with button pressed
  void Aim(const FInputActionValue& Value);
  // Called to handle fire input
  void FireButtonPressed(const FInputActionValue& Value);
  // Called to handle objects interactions
  void Select(const FInputActionValue& Value);
  // Called to reload currently equipped weapon
  void Reload(const FInputActionValue& Value);
  // Called to handle crouching
  void Crouch(const FInputActionValue& Value);

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
  class UInputMappingContext* PlayerMappingContext;
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
  class UInputAction* MoveAction;
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
  UInputAction* LookAction;
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
  UInputAction* JumpAction;
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
  UInputAction* FireWeaponAction;
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
  UInputAction* AimAction;
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
  UInputAction* SelectAction;
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
  UInputAction* ReloadAction;
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
  UInputAction* CrouchAction;

  // Interpolates camera zoom when aiming
  void CameraInterpZoom(float DeltaTime);
  // Set look sensitivity based on aiming
  void SetLookRate();
  
  void FireWeapon();
  // Calculate crosshair spread multiplier
  void CalculateCrosshairSpread(float DeltaTime);

  void StartCrosshairBulletFire();

  void FinishCrosshairBulletFire();

  bool GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation);

  void StartFireTimer();

  UFUNCTION()
  void AutoFireReset();

  bool TraceUnderCrosshair(FHitResult& OutHitResult, FVector& OutHitLocation);

  /** Trace for items if OverlappedItemCount >= 0 */
  void TraceForItems();

  /** Spawn the default weapon */
  class AWeapon* SpawnDefaultWeapon();

  /** Takes a weapon and attaches it to the mesh */
  void EquipWeapon(AWeapon* WeaponToEquip);

  /** Detach weapon and let it fall to the ground */
  void DropWeapon();

  /** Drops currently equipped weapon and equips TraceHitItem */
  void SwapWeapon(AWeapon* WeaponToSwap);

  /** Initialize the ammo map with ammo values */
  void InitializeAmmoMap();

  /** Check to make sure our weapon has ammo */
  bool WeaponHasAmmo();

  // Fire weapon functions
  void PlayFireSound();
  void SendBullet();
  void PlayGunFireMontage();

  // Reload functions
  void ReloadWeapon();
  UFUNCTION(BlueprintCallable)
  void FinishReloading();

  /** Checks to see if we have ammo of the EquippedWeapon's ammo type */
  bool CarryingAmmo();

  /** Called from Animation Blueprint with GrabClip notify */
  UFUNCTION(BlueprintCallable)
  void GrabClip();

  /** Called from Animation Blueprint with ReleaseClip notify */
  UFUNCTION(BlueprintCallable)
  void ReleaseClip();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
  // Camera boom positioning  the camera behind the character
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
  class USpringArmComponent* CameraBoom;

  // Camera that follows the character
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
  class UCameraComponent* FollowCamera;

  // Randomized gunshot sound cue
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
  class USoundCue* FireSound;

  // Flash spawned at BarrelSockets
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
  class UParticleSystem* MuzzleFlash;

  // Montage for firing the weapon
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
  class UAnimMontage* HipFireMontage;

  // Particles spawned upon bullet impact
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
  UParticleSystem* ImpactParticles;

  // Smoke trail for bullets
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
  UParticleSystem* BeamParticles;

  // Whether the character is aiming or not
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
  bool bAiming;

  // Default camera field of view value
  float CameraDefaultFOV;

  // Field of view value when zoomed in while aiming
  float CameraZoomedFOV;

  // Current field of view this frame
  float CameraCurrentFOV;

  // Interp speed for zooming when aiming
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
  float ZoomInterpSpeed;

  // Base look rate
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0"))
  float BaseLookRate;
  // Look rate when not aiming
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0"))
  float HipLookRate;
  // Look rate when aiming
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0"))
  float AimingLookRate;

  // Determines the spread of the crosshair
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
  float CrosshairSpreadMultiplier;
  // Velocity component for crosshair spread
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
  float CrosshairVelocityFactor;
  // In air component for crosshair spread
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
  float CrosshairInAirFactor;
  // Aim component for crosshair spread
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
  float CrosshairAimFactor;
  // Shooting component for crosshair spread
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
  float CrosshairShootingFactor;

  /** Time duration for the crosshair movement */
  float ShootTimeDuration;
  /** If the weapons is firing a bullet */
  bool bFiringBullet;
  /** Timer handle for the crosshair shoot movement */
  FTimerHandle CrosshairShootTimer;
  /** If the fire input is pressed */
  bool bFireButtonPressed;
  /** True when the weapon can fire, false when waiting for the timer */
  bool bShouldFire;
  /** Rate of automatic gun fire */
  float AutomaticFireRate;
  /** Sets a timer between gunshots */
  FTimerHandle AutoFireTimer;

  /** Memorizes the Item currently being aimed at */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
  class AItem* LastTraceHitItem;

  /** True if we should trace every frame for items */
  bool bShouldTraceForItems;

  /** Number of overlapped AItems */
  int8 OverlappedItemCount;

  /** Currently equipped weapon */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
  AWeapon* EquippedWeapon;

  /** Set this in Blueprint for the default Weapon class */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
  TSubclassOf<AWeapon> DefaultWeaponClass;

  /** The item currently hit by our trace in TraceForItems (can be null) */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
  AItem* TraceHitItem;

  /** Distance outward from the camera for the interp destination */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
  float CameraInterpDistance;

  /** Distance upward from the camera for the interp destination */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
  float CameraInterpElevation;

  /** Map to keep track of ammo of the different ammo types */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
  TMap<EAmmoType, int32> AmmoMap;

  /** Starting amount of 9mm ammo */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Items, meta = (AllowPrivateAccess = "true"))
  int32 Starting9mmAmmo;
  /** Starting amount of assault rifle ammo */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Items, meta = (AllowPrivateAccess = "true"))
  int32 StartingARAmmo;

  /** Combat state, can only fire or reload when Unoccupied */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
  ECombatState CombatState;

  /** Montage for reload animations */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
  UAnimMontage* ReloadMontage;

  /** Transform of the clip when we first grab the clip during reload */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
  FTransform ClipTransform;

  /** Scene component to attach to the character's hand during reload */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
  USceneComponent* HandSceneComponent;

  /** True when crouching */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
  bool bCrouching;

  /** Default movement speed, when not doing any special action */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
  float BaseMovementSpeed;

  /** Movement speed when aiming */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
  float AimMovementSpeed;

public:
  // Returns CameraBoom subobject
  FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
  // Returns FollowCamera subobject
  FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

  FORCEINLINE bool GetAiming() const { return bAiming; }

  UFUNCTION(BlueprintCallable)
  float GetCrosshairSpreadMultiplier() const;

  FORCEINLINE int8 GetOverlappedItemCount() const { return OverlappedItemCount; }

  /** Adds/subtracts to/from OverlappedItemCount and updates bShouldTraceForItems */
  void IncrementOverlappedItemCount(int8 Amount);

  FVector GetCameraInterpLocation();

  void GetPickupItem(AItem* Item);

  FORCEINLINE ECombatState GetCombatState() const { return CombatState; }
  FORCEINLINE bool GetCrouching() const { return bCrouching; }

};
