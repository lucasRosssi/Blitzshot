// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Components/InputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystemComponent.h"
#include "Item.h"
#include "Components/WidgetComponent.h"
#include "Weapon.h"

// Sets default values
AShooterCharacter::AShooterCharacter() :
  bAiming(false),
  // Camera field of view values
  CameraDefaultFOV(0.f), // set in BeginPlay
  CameraZoomedFOV(45.f),
  CameraCurrentFOV(0.f),
  ZoomInterpSpeed(20.f),
  // Look rate
  BaseLookRate(1.f),
  HipLookRate(1.f),
  AimingLookRate(0.3f),
  // Crosshair spread factors
  CrosshairSpreadMultiplier(0.f),
  CrosshairVelocityFactor(0.f),
  CrosshairInAirFactor(0.f),
  CrosshairAimFactor(0.f),
  CrosshairShootingFactor(0.f),
  // Bullet fire timer variables
  ShootTimeDuration(0.09f),
  bFiringBullet(false),
  // Automatic fire variables
  bFireButtonPressed(false),
  bShouldFire(true),
  AutomaticFireRate(0.1f),
  // Item trace variables
  bShouldTraceForItems(false)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

  // Create a camera boom (pulls in towards the character if there is a collision)
  CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
  CameraBoom->SetupAttachment(RootComponent);
  CameraBoom->TargetArmLength = 180.f; // The camera follows at this distance behind the character
  CameraBoom->bUsePawnControlRotation = true; // Rotate the arm with the player controller
  CameraBoom->SocketOffset = FVector(0.f, 70.f, 75.f);

  // Create follow camera
  FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
  FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach camera to end of boom
  FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

  // Don't rotate when the controller rotates. Let the controller only affect the camera
  bUseControllerRotationPitch = false;
  bUseControllerRotationYaw = true;
  bUseControllerRotationRoll = false;

  // Configure character movement
  GetCharacterMovement()->bOrientRotationToMovement = false; // Character moves in the direction of input...
  GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f); // ...at this rotation rate
  GetCharacterMovement()->JumpZVelocity = 600.f;
  GetCharacterMovement()->AirControl = 0.2f;

}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

  if (FollowCamera)
  {
    CameraDefaultFOV = GetFollowCamera()->FieldOfView;
    CameraCurrentFOV = CameraDefaultFOV;
  }

  SpawnDefaultWeapon();

  if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
  {
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
    {
      Subsystem->AddMappingContext(PlayerMappingContext, 0);
    }
  }
	
}

void AShooterCharacter::Move(const FInputActionValue &Value)
{
  const FVector2D MovementVector = Value.Get<FVector2D>();

  if (!GetController())
  {
    return;
  }

  const FRotator Rotation = GetController()->GetControlRotation();
  const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

  const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
  AddMovementInput(ForwardDirection, MovementVector.Y);

  const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
  AddMovementInput(RightDirection, MovementVector.X);
  
}

void AShooterCharacter::Look(const FInputActionValue &Value)
{
  const FVector2D LookAxisVector = Value.Get<FVector2D>();

  AddControllerPitchInput(LookAxisVector.Y * BaseLookRate);
  AddControllerYawInput(LookAxisVector.X * BaseLookRate);
}

void AShooterCharacter::Jump(const FInputActionValue &Value)
{
  Super::Jump();
  bAiming = false;
}

void AShooterCharacter::FireWeapon()
{
  if (FireSound)
  {
    UGameplayStatics::PlaySound2D(this, FireSound);
  }

  const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName("BarrelSocket");
  if (BarrelSocket)
  {
    const FTransform SocketTransform = BarrelSocket->GetSocketTransform(GetMesh());
    if (MuzzleFlash)
    {
      UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
    }

    FVector BeamEnd;
    bool bBeamEnd = GetBeamEndLocation(
      SocketTransform.GetLocation(),
      BeamEnd
    );

    if (bBeamEnd)
    {
      // Spawn particles after updating correctly BeamEndPoint
      if (ImpactParticles)
        {
          UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            ImpactParticles,
            BeamEnd
          );
        }

      if (BeamParticles)
      {
        UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
          GetWorld(),
          BeamParticles,
          SocketTransform
        );

        if (Beam)
        {
          Beam->SetVectorParameter(FName("Target"), BeamEnd);
        }
      }
    }

    
  }

  UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
  if (AnimInstance && HipFireMontage)
  {
    AnimInstance->Montage_Play(HipFireMontage);
    AnimInstance->Montage_JumpToSection(FName("StartFire"));
  }
  
  // Start bullet fire timer for crosshair
  StartCrosshairBulletFire();
}

void AShooterCharacter::Aim(const FInputActionValue &Value)
{
  if (GetCharacterMovement()->IsFalling())
  {
    bAiming = false;
    return;
  }
  bAiming = Value.Get<bool>();
}

bool AShooterCharacter::GetBeamEndLocation(
  const FVector &MuzzleSocketLocation,
  FVector & OutBeamLocation
)
{
  // Check for crosshair trace hit
  FHitResult CrosshairHitResult;
  bool bCrosshairHit = TraceUnderCrosshair(CrosshairHitResult, OutBeamLocation);

  if (bCrosshairHit)
  {
    // Tentative beam location - still need to trace from gun
    OutBeamLocation = CrosshairHitResult.Location;
  }
  else // no crosshair trace hit
  {
    // OutBeamLocation is the End location for the line trace
  }

  // Perform trace from gun barrel
  FHitResult WeaponTraceHit;
  const FVector WeaponTraceStart{ MuzzleSocketLocation };
  const FVector StartToEnd{ OutBeamLocation - MuzzleSocketLocation };
  const FVector WeaponTraceEnd{ MuzzleSocketLocation + StartToEnd * 1.25f };
  GetWorld()->LineTraceSingleByChannel(
    WeaponTraceHit,
    WeaponTraceStart,
    WeaponTraceEnd,
    ECollisionChannel::ECC_Visibility
  );

  if (WeaponTraceHit.bBlockingHit) // object between barrel and BeamEndPoint?
  {
    OutBeamLocation = WeaponTraceHit.Location;
    return true;
  }

  return false;
}

void AShooterCharacter::FireButtonPressed(const FInputActionValue& Value)
{
  bFireButtonPressed = Value.Get<bool>();
  StartFireTimer();
}

void AShooterCharacter::StartFireTimer()
{
  if (bShouldFire && bFireButtonPressed)
  {
    FireWeapon();
    bShouldFire = false;
    GetWorldTimerManager().SetTimer(
      AutoFireTimer,
      this,
      &AShooterCharacter::AutoFireReset,
      AutomaticFireRate
    );
  }
}

void AShooterCharacter::AutoFireReset()
{
  bShouldFire = true;
  if (bFireButtonPressed)
  {
    StartFireTimer();
  }
}

// OLD WAY !! //

// void AShooterCharacter::MoveForward(const FInputActionValue& Value)
// {
//   if (Controller && Value != 0.f)
//   {
//     // find out which way is forward
//     const FRotator Rotation{ Controller->GetControlRotation() };
//     const FRotator YawRotation{ 0, Rotation.Yaw, 0  };

//     const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X) };
//     AddMovementInput(Direction, Value);
//   }
// }

// void AShooterCharacter::Strafe(float Value)
// {
//   if (Controller && Value != 0.f)
//   {
//     // find out which way is right
//     const FRotator Rotation{ Controller->GetControlRotation() };
//     const FRotator YawRotation{ 0, Rotation.Yaw, 0  };

//     const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y) };
//     AddMovementInput(Direction, Value);
//   }
// }

void AShooterCharacter::CameraInterpZoom(float DeltaTime)
{
  // Set current camera field of view
  if (bAiming)
  {
    // Interpolate to zoomed FOV
    CameraCurrentFOV = FMath::FInterpTo(
      CameraCurrentFOV,
      CameraZoomedFOV,
      DeltaTime,
      ZoomInterpSpeed
    );
  }
  else
  {
    // Interpolate to default FOV
    CameraCurrentFOV = FMath::FInterpTo(
      CameraCurrentFOV,
      CameraDefaultFOV,
      DeltaTime,
      ZoomInterpSpeed
    );
  }
  GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);
}

void AShooterCharacter::SetLookRate()
{
  if (bAiming)
  {
    BaseLookRate = AimingLookRate;
  }
  else
  {
    BaseLookRate = HipLookRate;
  }
}

void AShooterCharacter::CalculateCrosshairSpread(float DeltaTime)
{
  FVector2D WalkSpeedRange{ 0.f, 600.f };
  FVector2D VelocityMultiplierRange{ 0.f, 1.f };
  FVector Velocity{ GetVelocity() };
  Velocity.Z = 0.f;

  // Calculate crosshair velocity factor
  CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(
    WalkSpeedRange,
    VelocityMultiplierRange,
    Velocity.Size()
  );

  // Calculate crosshair in air factor
  if (GetCharacterMovement()->IsFalling()) // is in air?
  {
    // Spread the crosshair while in air
    CrosshairInAirFactor = FMath::FInterpTo(
      CrosshairInAirFactor,
      2.f,
      DeltaTime,
      15.f
    );
  }
  else // character is on the ground
  {
    // Shrink the crosshair after landing in the ground
    CrosshairInAirFactor = FMath::FInterpTo(
      CrosshairInAirFactor,
      0.f,
      DeltaTime,
      5.f
    );
  }

  if (bAiming) // is the character aiming?
  {
    // Shrink the crosshair when aiming
    CrosshairAimFactor = FMath::FInterpTo(
      CrosshairAimFactor,
      0.5f,
      DeltaTime,
      20.f
    );
  }
  else // not aiming
  {
    // Spread the crosshair back to normal when stop aiming
    CrosshairAimFactor = FMath::FInterpTo(
      CrosshairAimFactor,
      0.f,
      DeltaTime,
      20.f
    );
  }

  // True 0.15 seconds after firing
  if (bFiringBullet)
  {
    CrosshairShootingFactor = FMath::FInterpTo(
      CrosshairShootingFactor,
      0.4f,
      DeltaTime,
      30.f
    );
  }
  else
  {
    CrosshairShootingFactor = FMath::FInterpTo(
      CrosshairShootingFactor,
      0.f,
      DeltaTime,
      15.f
    );
  }

  CrosshairSpreadMultiplier =
    0.5f +
    CrosshairVelocityFactor +
    CrosshairInAirFactor +
    CrosshairShootingFactor -
    CrosshairAimFactor;
}

void AShooterCharacter::StartCrosshairBulletFire()
{
  bFiringBullet = true;

  GetWorldTimerManager().SetTimer(
    CrosshairShootTimer,
    this,
    &AShooterCharacter::FinishCrosshairBulletFire,
    ShootTimeDuration
  );
}

void AShooterCharacter::FinishCrosshairBulletFire()
{
  bFiringBullet = false;
}

bool AShooterCharacter::TraceUnderCrosshair(FHitResult& OutHitResult, FVector& OutHitLocation)
{
  // Get Viewport size
  FVector2D ViewportSize;
  if (GEngine && GEngine->GameViewport)
  {
    GEngine->GameViewport->GetViewportSize(ViewportSize);
  }

  // Get screen space location of crosshair
  FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
  FVector CrosshairWorldPosition;
  FVector CrosshairWorldDirection;

  // Get world position and direction of crosshair
  bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
    UGameplayStatics::GetPlayerController(this, 0),
    CrosshairLocation,
    CrosshairWorldPosition,
    CrosshairWorldDirection
  );

  if (bScreenToWorld)
  {
    // Trace from Crosshair world location outward
    const FVector Start{ CrosshairWorldPosition };
    const FVector End{ Start + CrosshairWorldDirection * 50'000.f };
    OutHitLocation = End;
    GetWorld()->LineTraceSingleByChannel(
      OutHitResult,
      Start,
      End,
      ECollisionChannel::ECC_Visibility
    );

    if (OutHitResult.bBlockingHit)
    {
      OutHitLocation = OutHitResult.Location;
      return true;
    }
  }

  return false;
}

void AShooterCharacter::IncrementOverlappedItemCount(int8 Amount) 
{
  if (OverlappedItemCount + Amount <= 0)
  {
    OverlappedItemCount = 0;
    bShouldTraceForItems = false;
  }
  else
  {
    OverlappedItemCount += Amount;
    bShouldTraceForItems = true;
  }
}

void AShooterCharacter::TraceForItems()
{
  if (!bShouldTraceForItems)
  {
    if (LastTraceHitItem && LastTraceHitItem->GetPickupWidget())
    {
      LastTraceHitItem->GetPickupWidget()->SetVisibility(false);
    }

    return;
  }

  FHitResult ItemTraceResult;
  FVector HitLocation;
  TraceUnderCrosshair(ItemTraceResult, HitLocation);

  if (!ItemTraceResult.bBlockingHit)
  {
    return;
  }

  AItem* HitItem = Cast<AItem>(ItemTraceResult.GetActor());
  if (HitItem && HitItem->GetPickupWidget())
  {
    // Show item's Pickup Widget
    HitItem->GetPickupWidget()->SetVisibility(true);
  }

  if (LastTraceHitItem && LastTraceHitItem != HitItem)
  {
    // Hide widget from previous HitItem
    LastTraceHitItem->GetPickupWidget()->SetVisibility(false);
  }

  // Store a reference of HitItem
  LastTraceHitItem = HitItem;
}

void AShooterCharacter::SpawnDefaultWeapon()
{
  // Check the TSubclassOf variable
  if (!DefaultWeaponClass)
  {
    return;
  }

  // Spawn the Weapon
  AWeapon* DefaultWeapon = GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
  // Get the Hand Socket
  const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));

  if (HandSocket)
  {
    // Attach the Weapon to the hand socket RightHandSocket
    HandSocket->AttachActor(DefaultWeapon, GetMesh());
  }

  // Set EquippedWeapon to the newly spawned Weapon
  EquippedWeapon = DefaultWeapon;
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

  CameraInterpZoom(DeltaTime);
  SetLookRate();
  CalculateCrosshairSpread(DeltaTime);
  TraceForItems();
  
}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

  if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
  {
    EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AShooterCharacter::Move);
    EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AShooterCharacter::Look);
    EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AShooterCharacter::Jump);
    EnhancedInputComponent->BindAction(FireWeaponAction, ETriggerEvent::Triggered, this, &AShooterCharacter::FireButtonPressed);
    EnhancedInputComponent->BindAction(FireWeaponAction, ETriggerEvent::Completed, this, &AShooterCharacter::FireButtonPressed);
    EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this, &AShooterCharacter::Aim);
    EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &AShooterCharacter::Aim);
  }
}

float AShooterCharacter::GetCrosshairSpreadMultiplier() const
{
  return CrosshairSpreadMultiplier;
}
