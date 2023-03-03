// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "ShooterCharacter.generated.h"

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
  // Called to handle fire input
  void FireWeapon(const FInputActionValue& Value);
  // Set bAiming to true or false with button pressed
  void Aim(const FInputActionValue& Value);
  // Interpolates camera zoom when aiming
  void CameraInterpZoom(float DeltaTime);
  // Set look sensitivity based on aiming
  void SetLookRate();
  // Calculate crosshair spread multiplier
  void CalculateCrosshairSpread(float DeltaTime);

  void StartCrosshairBulletFire();

  UFUNCTION()
  void FinishCrosshairBulletFire();

  bool GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation);

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

  float ShootTimeDuration;
  bool bFiringBullet;
  FTimerHandle CrosshairShootTimer;

public:
  // Returns CameraBoom subobject
  FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
  // Returns FollowCamera subobject
  FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

  FORCEINLINE bool GetAiming() const { return bAiming; }

  UFUNCTION(BlueprintCallable)
  float GetCrosshairSpreadMultiplier() const;

};
