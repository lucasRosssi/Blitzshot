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
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Ammo.h"
#include "BulletHitInterface.h"
#include "Enemy.h"

// Sets default values
AShooterCharacter::AShooterCharacter() : bAiming(false),
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
                                         // Item trace variables
                                         bShouldTraceForItems(false),
                                         OverlappedItemCount(0),
                                         // Camera interp location variables
                                         CameraInterpDistance(150.f),
                                         CameraInterpElevation(35.f),
                                         // Ammo variables
                                         Starting9mmAmmo(85),
                                         StartingARAmmo(150),
                                         // Combat variables
                                         CombatState(ECombatState::ECS_Unoccupied),
                                         bCrouching(false),
                                         // Movement speed variables
                                         BaseMovementSpeed(500.f),
                                         AimMovementSpeed(250.f),
                                         // Pickup sound timer properties
                                         bShouldPlayPickupSound(true),
                                         bShouldPlayEquipSound(true),
                                         PickupSoundResetTime(0.2f),
                                         EquipSoundResetTime(0.2f),
                                         RecoilCameraSpeed(5.f),
                                         RecoilAmount(0.8f),
                                         DodgeMontageSection(TEXT("DodgeBackward")),
                                         bCanDodge(true),
                                         MaxHealth(100.f),
                                         Health(MaxHealth)
{
  // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
  PrimaryActorTick.bCanEverTick = true;

  // Create a camera boom (pulls in towards the character if there is a collision)
  CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
  CameraBoom->SetupAttachment(RootComponent);
  CameraBoom->TargetArmLength = 180.f;        // The camera follows at this distance behind the character
  CameraBoom->bUsePawnControlRotation = true; // Rotate the arm with the player controller
  CameraBoom->SocketOffset = FVector(0.f, 70.f, 75.f);

  // Create follow camera
  FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
  FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach camera to end of boom
  FollowCamera->bUsePawnControlRotation = false;                              // Camera does not rotate relative to arm

  // Don't rotate when the controller rotates. Let the controller only affect the camera
  bUseControllerRotationPitch = false;
  bUseControllerRotationYaw = true;
  bUseControllerRotationRoll = false;

  // Create Hand Scene component
  HandSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("HandSceneComp"));

  // Configure character movement
  GetCharacterMovement()->bOrientRotationToMovement = false;        // Character moves in the direction of input...
  GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f); // ...at this rotation rate
  GetCharacterMovement()->JumpZVelocity = 600.f;
  GetCharacterMovement()->AirControl = 0.2f;

  // Create interpolation components
  WeaponInterpComp = CreateDefaultSubobject<USceneComponent>(TEXT("Weapon Interpolation Component"));
  WeaponInterpComp->SetupAttachment(GetFollowCamera());
  InterpComp1 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 1"));
  InterpComp1->SetupAttachment(GetFollowCamera());
  InterpComp2 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 2"));
  InterpComp2->SetupAttachment(GetFollowCamera());
  InterpComp3 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 3"));
  InterpComp3->SetupAttachment(GetFollowCamera());
  InterpComp4 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 4"));
  InterpComp4->SetupAttachment(GetFollowCamera());
  InterpComp5 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 5"));
  InterpComp5->SetupAttachment(GetFollowCamera());
  InterpComp6 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 6"));
  InterpComp6->SetupAttachment(GetFollowCamera());
}

float AShooterCharacter::TakeDamage(
    float DamageAmount,
    struct FDamageEvent const &DamageEvent,
    class AController *EventInstigator,
    AActor *DamageCauser)
{
  if (Health - DamageAmount <= 0.f)
  {
    Health = 0.f;
  }
  else
  {
    Health -= DamageAmount;
  }

  return DamageAmount;
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

  if (APlayerController *PlayerController = Cast<APlayerController>(GetController()))
  {
    if (UEnhancedInputLocalPlayerSubsystem *Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
    {
      Subsystem->AddMappingContext(PlayerMappingContext, 0);
    }
  }

  GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;

  // Spawn the default weapon and equip it
  EquipWeapon(SpawnDefaultWeapon());
  Inventory.Add(EquippedWeapon);
  EquippedWeapon->SetSlotIndex(0);

  InitializeAmmoMap();

  // Create FInterpLocation structs for each interp locations and add to array
  InitializeInterpLocations();
}

/* START INPUT ACTIONS */

void AShooterCharacter::Move(const FInputActionValue &Value)
{
  const FVector2D MovementVector = Value.Get<FVector2D>();
  MovementInputX = FMath::RoundToInt32(MovementVector.X);
  MovementInputY = FMath::RoundToInt32(MovementVector.Y);

  if (!GetController())
  {
    return;
  }

  if (CombatState == ECombatState::ECS_Sprinting && MovementVector.Y < 0.5f)
  {
    EndSprint();
  }

  const FRotator Rotation = GetController()->GetControlRotation();
  const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

  const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
  const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

  if (CombatState != ECombatState::ECS_Dodging)
  {
    AddMovementInput(ForwardDirection, MovementInputY);
    if (CombatState != ECombatState::ECS_Sprinting)
    {
      AddMovementInput(RightDirection, MovementInputX);
    }
    else
    {
      AddMovementInput(RightDirection, MovementInputX / 4);
    }
  }
}

void AShooterCharacter::Look(const FInputActionValue &Value)
{
  const FVector2D LookAxisVector = Value.Get<FVector2D>();

  AddControllerPitchInput(LookAxisVector.Y * BaseLookRate);
  if (CombatState == ECombatState::ECS_Sprinting)
  {
    AddControllerYawInput(FMath::Clamp(LookAxisVector.X, -0.5f, 0.5f) * BaseLookRate);
  }
  else
  {
    AddControllerYawInput(LookAxisVector.X * BaseLookRate);
  }
}

void AShooterCharacter::Jump(const FInputActionValue &Value)
{
  Super::Jump();
  bAiming = false;
}

void AShooterCharacter::Aim(const FInputActionValue &Value)
{
  if (CombatState == ECombatState::ECS_Sprinting)
    return;

  bAiming = Value.Get<bool>();

  if (
      GetCharacterMovement()->IsFalling() ||
      CombatState == ECombatState::ECS_Reloading ||
      CombatState == ECombatState::ECS_Equipping)
  {
    bAiming = false;
  }

  if (bAiming)
  {
    GetCharacterMovement()->MaxWalkSpeed = AimMovementSpeed;
  }
  else
  {
    GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
  }
}

void AShooterCharacter::FireButtonPressed(const FInputActionValue &Value)
{
  bFireButtonPressed = true;
  FireWeapon();
}

void AShooterCharacter::FireButtonReleased(const FInputActionValue &Value)
{
  bFireButtonPressed = false;
}

void AShooterCharacter::Select(const FInputActionValue &Value)
{
  if (TraceHitItem)
  {
    auto TraceHitAmmo = Cast<AAmmo>(TraceHitItem);
    if (TraceHitAmmo)
    {
      TraceHitAmmo->GetAmmoCollisionSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    TraceHitItem->StartItemCurve(this);
    TraceHitItem = nullptr;
  }
}

void AShooterCharacter::Reload(const FInputActionValue &Value)
{
  ReloadWeapon();
}

void AShooterCharacter::Crouch(const FInputActionValue &Value)
{
  if (GetCharacterMovement()->IsFalling())
  {
    bCrouching = false;
  }

  bCrouching = !bCrouching;
}

void AShooterCharacter::Sprint(const FInputActionValue &Value)
{
  if (GetCharacterMovement()->IsFalling() || CombatState == ECombatState::ECS_Dodging)
    return;

  if (CombatState != ECombatState::ECS_Sprinting)
  {
    CombatState = ECombatState::ECS_Sprinting;

    GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed * 1.5f;
  }
  else
  {
    CombatState = ECombatState::ECS_Unoccupied;

    GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
  }
}

void AShooterCharacter::Dodge(const FInputActionValue &Value)
{
  if (GetCharacterMovement()->IsFalling() || CombatState == ECombatState::ECS_Dodging || !bCanDodge)
    return;

  ResetCombatState(ECombatState::ECS_Dodging);

  int32 Direction = GetMovementInputDirection(MovementInputX, MovementInputY);

  PlayDodgeAnimation(Direction);
  StartDodgeTimer();
  bCanDodge = false;
  // Invulnerability
}

void AShooterCharacter::SwitchWeapon1(const FInputActionValue &Value)
{
  if (EquippedWeapon->GetSlotIndex() == 0)
    return;

  ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 0);
}

void AShooterCharacter::SwitchWeapon2(const FInputActionValue &Value)
{
  if (EquippedWeapon->GetSlotIndex() == 1)
    return;

  ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 1);
}

void AShooterCharacter::SwitchWeapon3(const FInputActionValue &Value)
{
  if (EquippedWeapon->GetSlotIndex() == 2)
    return;

  ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 2);
}

void AShooterCharacter::SwitchMelee(const FInputActionValue &Value)
{
  if (EquippedWeapon->GetSlotIndex() == 3)
    return;

  ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 3);
}

void AShooterCharacter::NextWeapon(const FInputActionValue &Value)
{
  if (Inventory.Num() == 1)
    return;

  const float AxisValue = Value.Get<float>();
  const int SlotIndex = EquippedWeapon->GetSlotIndex();

  if (AxisValue > 0.f)
  {
    if (SlotIndex == Inventory.Num() - 1)
    {
      ExchangeInventoryItems(SlotIndex, 0);
    }
    else
    {
      ExchangeInventoryItems(SlotIndex, SlotIndex + 1);
    }
  }

  if (AxisValue < 0.f)
  {
    if (SlotIndex == 0)
    {
      ExchangeInventoryItems(SlotIndex, Inventory.Num() - 1);
    }
    else
    {
      ExchangeInventoryItems(SlotIndex, SlotIndex - 1);
    }
  }
}

/* END INPUT ACTIONS */

void AShooterCharacter::EndSprint()
{
  CombatState = ECombatState::ECS_Unoccupied;
  GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
}

void AShooterCharacter::FireWeapon()
{
  if (!EquippedWeapon || CombatState != ECombatState::ECS_Unoccupied)
    return;

  if (!WeaponHasAmmo())
  {
    ReloadWeapon();
    return;
  }

  PlayFireSound();
  SendBullet();
  PlayGunFireMontage();
  TriggerRecoil();
  StartCrosshairBulletFire();
  EquippedWeapon->ConsumeAmmo();

  StartFireTimer();
}

bool AShooterCharacter::GetBeamEndLocation(
    const FVector &MuzzleSocketLocation,
    FHitResult &OutHitResult)
{
  FVector OutBeamLocation;
  // Check for crosshair trace hit
  FHitResult CrosshairHitResult;
  bool bCrosshairHit = TraceUnderCrosshair(CrosshairHitResult, OutBeamLocation, true);

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
  const FVector WeaponTraceStart{MuzzleSocketLocation};
  const FVector StartToEnd{OutBeamLocation - MuzzleSocketLocation};
  const FVector WeaponTraceEnd{MuzzleSocketLocation + StartToEnd * 1.25f};
  GetWorld()->LineTraceSingleByChannel(
      OutHitResult,
      WeaponTraceStart,
      WeaponTraceEnd,
      ECollisionChannel::ECC_Visibility);

  if (!OutHitResult.bBlockingHit) // object between barrel and BeamEndPoint?
  {
    OutHitResult.Location = OutBeamLocation;
    return false;
  }

  return true;
}

void AShooterCharacter::StartFireTimer()
{
  if (!EquippedWeapon)
    return;
  CombatState = ECombatState::ECS_FireTimerInProgress;

  GetWorldTimerManager().SetTimer(
      AutoFireTimer,
      this,
      &AShooterCharacter::AutoFireReset,
      EquippedWeapon->GetFireRate());
}

void AShooterCharacter::AutoFireReset()
{
  if (CombatState == ECombatState::ECS_FireTimerInProgress)
  {
    CombatState = ECombatState::ECS_Unoccupied;
  }

  if (!EquippedWeapon)
    return;

  if (WeaponHasAmmo())
  {
    if (bFireButtonPressed && EquippedWeapon->GetAutomatic())
    {
      FireWeapon();
    }
  }
  else
  {
    ReloadWeapon();
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
        ZoomInterpSpeed);
  }
  else
  {
    // Interpolate to default FOV
    CameraCurrentFOV = FMath::FInterpTo(
        CameraCurrentFOV,
        CameraDefaultFOV,
        DeltaTime,
        ZoomInterpSpeed);
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
  FVector2D WalkSpeedRange{0.f, 600.f};
  FVector2D VelocityMultiplierRange{0.f, 1.f};
  FVector Velocity{GetVelocity()};
  Velocity.Z = 0.f;

  // Calculate crosshair velocity factor
  CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(
      WalkSpeedRange,
      VelocityMultiplierRange,
      Velocity.Size());

  // Calculate crosshair in air factor
  if (GetCharacterMovement()->IsFalling()) // is in air?
  {
    // Spread the crosshair while in air
    CrosshairInAirFactor = FMath::FInterpTo(
        CrosshairInAirFactor,
        2.f,
        DeltaTime,
        15.f);
  }
  else // character is on the ground
  {
    // Shrink the crosshair after landing in the ground
    CrosshairInAirFactor = FMath::FInterpTo(
        CrosshairInAirFactor,
        0.f,
        DeltaTime,
        5.f);
  }

  if (bAiming) // is the character aiming?
  {
    // Shrink the crosshair when aiming
    CrosshairAimFactor = FMath::FInterpTo(
        CrosshairAimFactor,
        0.5f,
        DeltaTime,
        20.f);
  }
  else // not aiming
  {
    // Spread the crosshair back to normal when stop aiming
    CrosshairAimFactor = FMath::FInterpTo(
        CrosshairAimFactor,
        0.f,
        DeltaTime,
        20.f);
  }

  // True 0.15 seconds after firing
  if (bFiringBullet)
  {
    CrosshairShootingFactor = FMath::FInterpTo(
        CrosshairShootingFactor,
        0.4f,
        DeltaTime,
        30.f);
  }
  else
  {
    CrosshairShootingFactor = FMath::FInterpTo(
        CrosshairShootingFactor,
        0.f,
        DeltaTime,
        15.f);
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
      ShootTimeDuration);
}

void AShooterCharacter::FinishCrosshairBulletFire()
{
  bFiringBullet = false;
}

bool AShooterCharacter::TraceUnderCrosshair(FHitResult &OutHitResult, FVector &OutHitLocation, bool bShooting)
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
  FVector2D WeaponAccuracySpread;
  FVector2D FinalLocation = CrosshairLocation;
  if (bShooting)
  {
    float const BulletSpreadFactor = (100.0f - EquippedWeapon->GetAccuracy()) / 100.0f;

    if (bAiming)
    {
      WeaponAccuracySpread = FMath::RandPointInCircle(
          80.f * (BulletSpreadFactor));
    }
    else
    {
      WeaponAccuracySpread = FMath::RandPointInCircle(
          80.f * BulletSpreadFactor *
          2.0f);
    }
    FinalLocation = CrosshairLocation + WeaponAccuracySpread;
  }

  // Get world position and direction of crosshair
  bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
      UGameplayStatics::GetPlayerController(this, 0),
      FinalLocation,
      CrosshairWorldPosition,
      CrosshairWorldDirection);

  if (bScreenToWorld)
  {
    // Trace from Crosshair world location outward
    const FVector Start{CrosshairWorldPosition};
    const FVector End{Start + CrosshairWorldDirection * 50'000.f};
    OutHitLocation = End;
    GetWorld()->LineTraceSingleByChannel(
        OutHitResult,
        Start,
        End,
        ECollisionChannel::ECC_Visibility);

    if (OutHitResult.bBlockingHit)
    {
      OutHitLocation = OutHitResult.Location;
      return true;
    }
  }

  return false;
}

void AShooterCharacter::ApplyRecoil(float DeltaTime)
{
  if (!VerticalRecoil && !HorizontalRecoil)
    return;

  float ApplyPitch;
  float ApplyYaw;

  VerticalRecoil = FMath::FInterpTo(
      VerticalRecoil,
      0,
      DeltaTime,
      RecoilCameraSpeed);
  VerticalRecoilRecovery = FMath::FInterpTo(
      VerticalRecoilRecovery,
      -VerticalRecoil,
      DeltaTime,
      RecoilCameraSpeed * 2);

  HorizontalRecoil = FMath::FInterpTo(
      HorizontalRecoil,
      0,
      DeltaTime,
      RecoilCameraSpeed);
  HorizontalRecoilRecovery = FMath::FInterpTo(
      HorizontalRecoilRecovery,
      -HorizontalRecoil,
      DeltaTime,
      RecoilCameraSpeed * 2);

  ApplyPitch = VerticalRecoil + VerticalRecoilRecovery;
  ApplyYaw = HorizontalRecoil + HorizontalRecoilRecovery;

  if (ApplyPitch)
    AddControllerPitchInput(ApplyPitch);
  if (ApplyYaw)
    AddControllerYawInput(ApplyYaw);
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
      LastTraceHitItem->DisableGlowMaterial();
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

  TraceHitItem = Cast<AItem>(ItemTraceResult.GetActor());

  if (TraceHitItem && TraceHitItem->GetItemState() == EItemState::EIS_EquipInterping)
  {
    TraceHitItem = nullptr;
  }

  if (TraceHitItem && TraceHitItem->GetPickupWidget())
  {
    // Show item's Pickup Widget
    TraceHitItem->GetPickupWidget()->SetVisibility(true);
    TraceHitItem->EnableGlowMaterial();
  }

  if (LastTraceHitItem && LastTraceHitItem != TraceHitItem)
  {
    // Hide widget from previous HitItem
    LastTraceHitItem->GetPickupWidget()->SetVisibility(false);
    LastTraceHitItem->DisableGlowMaterial();
  }

  // Store a reference of HitItem
  LastTraceHitItem = TraceHitItem;
}

AWeapon *AShooterCharacter::SpawnDefaultWeapon()
{
  // Check the TSubclassOf variable
  if (!DefaultWeaponClass)
  {
    return nullptr;
  }

  // Spawn the Weapon
  return GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
}

void AShooterCharacter::EquipWeapon(AWeapon *WeaponToEquip)
{
  if (!WeaponToEquip)
  {
    return;
  }

  // Get the Hand Socket
  const USkeletalMeshSocket *HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));

  if (HandSocket)
  {
    // Attach the Weapon to the hand socket RightHandSocket
    HandSocket->AttachActor(WeaponToEquip, GetMesh());
  }

  if (!EquippedWeapon)
  {
    // -1 no equipped weapon yet
    EquipItemDelegate.Broadcast(-1, WeaponToEquip->GetSlotIndex());
  }

  // Set EquippedWeapon to the newly spawned Weapon
  EquippedWeapon = WeaponToEquip;
  EquippedWeapon->SetItemState(EItemState::EIS_Equipped);
}

void AShooterCharacter::DropWeapon()
{
  if (!EquippedWeapon)
  {
    return;
  }

  FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
  EquippedWeapon->GetItemMesh()->DetachFromComponent(DetachmentTransformRules);

  EquippedWeapon->SetItemState(EItemState::EIS_Falling);
  EquippedWeapon->ThrowWeapon();
}

void AShooterCharacter::SwapWeapon(AWeapon *WeaponToSwap)
{
  if (Inventory.Num() - 1 >= EquippedWeapon->GetSlotIndex())
  {
    Inventory[EquippedWeapon->GetSlotIndex()] = WeaponToSwap;
    WeaponToSwap->SetSlotIndex(EquippedWeapon->GetSlotIndex());
  }

  DropWeapon();
  EquipWeapon(WeaponToSwap);
  TraceHitItem = nullptr;
  LastTraceHitItem = nullptr;
}

void AShooterCharacter::InitializeAmmoMap()
{
  AmmoMap.Add(EAmmoType::EAT_9mm, Starting9mmAmmo);
  AmmoMap.Add(EAmmoType::EAT_AssaultRifle, StartingARAmmo);
}

bool AShooterCharacter::WeaponHasAmmo()
{
  if (!EquippedWeapon)
    return false;

  return EquippedWeapon->GetAmmo() > 0;
}

void AShooterCharacter::PlayFireSound()
{
  if (EquippedWeapon->GetFireSound())
  {
    UGameplayStatics::PlaySound2D(this, EquippedWeapon->GetFireSound());
  }
}

void AShooterCharacter::SendBullet()
{
  const USkeletalMeshSocket *BarrelSocket = EquippedWeapon->GetItemMesh()->GetSocketByName("BarrelSocket");
  if (BarrelSocket)
  {
    const FTransform SocketTransform = BarrelSocket->GetSocketTransform(EquippedWeapon->GetItemMesh());

    if (EquippedWeapon->GetMuzzleFlash())
    {
      UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EquippedWeapon->GetMuzzleFlash(), SocketTransform);
    }

    FHitResult BeamHitResult;
    bool bBeamEnd = GetBeamEndLocation(
        SocketTransform.GetLocation(),
        BeamHitResult);

    if (bBeamEnd)
    {
      // Spawn particles after updating correctly BeamEndPoint
      // Check if hit actor implement BulletHitInterface
      if (BeamHitResult.GetActor())
      {
        IBulletHitInterface *BulletHitInterface = Cast<IBulletHitInterface>(BeamHitResult.GetActor());

        if (BulletHitInterface)
        {
          BulletHitInterface->BulletHit_Implementation(BeamHitResult);
        }
        else if (ImpactParticles) // Default particles
        {
          UGameplayStatics::SpawnEmitterAtLocation(
              GetWorld(),
              ImpactParticles,
              BeamHitResult.Location,
              FRotator(0.f),
              true);
        }

        AEnemy *HitEnemy = Cast<AEnemy>(BeamHitResult.GetActor());
        if (HitEnemy)
        {
          int32 Damage{};
          bool bWeakspot = false;

          if (BeamHitResult.BoneName.ToString() == HitEnemy->GetWeakspotBone())
          // Weakspot shot
          {
            Damage = EquippedWeapon->GetWeakspotDamage();
            bWeakspot = true;
          }
          // Normal shot
          else
          {
            Damage = EquippedWeapon->GetDamage();
          }
          UGameplayStatics::ApplyDamage(
              HitEnemy,
              Damage,
              GetController(),
              EquippedWeapon,
              UDamageType::StaticClass());

          HitEnemy->ShowHitNumber(Damage, BeamHitResult.Location, bWeakspot);
          HitEnemy->TakeBalanceDamage(EquippedWeapon->GetBalanceDamage());
        }
      }

      if (EquippedWeapon->GetBeamParticles())
      {
        UParticleSystemComponent *Beam = UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            EquippedWeapon->GetBeamParticles(),
            SocketTransform);

        if (Beam)
        {
          Beam->SetVectorParameter(FName("Target"), BeamHitResult.Location);
        }
      }
    }
  }
}

void AShooterCharacter::PlayGunFireMontage()
{
  UAnimInstance *AnimInstance = GetMesh()->GetAnimInstance();
  if (AnimInstance && HipFireMontage && !bAiming)
  {
    AnimInstance->Montage_Play(HipFireMontage);
    AnimInstance->Montage_JumpToSection(EquippedWeapon->GetHipFireMontageSection());
  }

  if (AnimInstance && AimFireMontage && bAiming)
  {
    AnimInstance->Montage_Play(AimFireMontage);
    AnimInstance->Montage_JumpToSection(EquippedWeapon->GetAimFireMontageSection());
  }
}

void AShooterCharacter::ReloadWeapon()
{
  if (!EquippedWeapon || CombatState != ECombatState::ECS_Unoccupied)
    return;

  // Do we have ammo of the correct type?
  if (CarryingAmmo() && !EquippedWeapon->ClipIsFull())
  {
    CombatState = ECombatState::ECS_Reloading;

    UAnimInstance *AnimInstance = GetMesh()->GetAnimInstance();
    if (AnimInstance && ReloadMontage)
    {
      AnimInstance->Montage_Play(ReloadMontage, 1.2f);
      AnimInstance->Montage_JumpToSection(EquippedWeapon->GetReloadMontageSection());
    }
  }
}

bool AShooterCharacter::CarryingAmmo()
{
  if (!EquippedWeapon)
    return false;

  auto AmmoType = EquippedWeapon->GetAmmoType();

  if (AmmoMap.Contains(AmmoType))
  {
    return AmmoMap[AmmoType] > 0;
  }

  return false;
}

void AShooterCharacter::GrabClip()
{
  if (!EquippedWeapon || !HandSceneComponent)
    return;

  USkeletalMeshComponent *EquippedWeaponMesh = EquippedWeapon->GetItemMesh();
  FName ClipBoneName = EquippedWeapon->GetClipBoneName();
  // Index for the clip bone on the Equipped Weapon
  int32 ClipBoneIndex{EquippedWeaponMesh->GetBoneIndex(ClipBoneName)};
  // Store the transform of the clip
  ClipTransform = EquippedWeaponMesh->GetBoneTransform(ClipBoneIndex);

  FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepRelative, true);
  HandSceneComponent->AttachToComponent(GetMesh(), AttachmentRules, FName(TEXT("Hand_L")));
  HandSceneComponent->SetWorldTransform(ClipTransform);

  EquippedWeapon->SetMovingClip(true);
}

void AShooterCharacter::ReleaseClip()
{
  EquippedWeapon->SetMovingClip(false);
}

void AShooterCharacter::PickupAmmo(AAmmo *Ammo)
{
  const auto AmmoType = Ammo->GetAmmoType();
  // Check if AmmoMap contains Ammo's ammo type
  if (AmmoMap.Find(AmmoType))
  {
    // Get amount of ammo in our AmmoMap for the ammo type
    int32 AmmoCount{AmmoMap[AmmoType]};
    AmmoCount += Ammo->GetItemCount();
    // Set the amount of ammo in the map for this type
    AmmoMap[AmmoType] = AmmoCount;
  }

  if (EquippedWeapon->GetAmmoType() == AmmoType)
  {
    // Check if the gun is empty
    if (EquippedWeapon->GetAmmo() == 0)
    {
      ReloadWeapon();
    }
  }

  Ammo->Destroy();
}

void AShooterCharacter::InitializeInterpLocations()
{
  FInterpLocation WeaponLocation{WeaponInterpComp, 0};
  InterpLocations.Add(WeaponLocation);

  FInterpLocation InterpLocation1{InterpComp1, 0};
  InterpLocations.Add(InterpLocation1);
  FInterpLocation InterpLocation2{InterpComp2, 0};
  InterpLocations.Add(InterpLocation2);
  FInterpLocation InterpLocation3{InterpComp3, 0};
  InterpLocations.Add(InterpLocation3);
  FInterpLocation InterpLocation4{InterpComp4, 0};
  InterpLocations.Add(InterpLocation4);
  FInterpLocation InterpLocation5{InterpComp5, 0};
  InterpLocations.Add(InterpLocation5);
  FInterpLocation InterpLocation6{InterpComp6, 0};
  InterpLocations.Add(InterpLocation6);
}

void AShooterCharacter::ExchangeInventoryItems(int32 CurrentItemIndex, int32 NewItemIndex)
{
  bool bCanSwitchWeapon = (CombatState == ECombatState::ECS_Unoccupied || CombatState == ECombatState::ECS_Reloading || CombatState == ECombatState::ECS_Equipping);

  if ((CurrentItemIndex == NewItemIndex) || (NewItemIndex >= Inventory.Num()) || NewItemIndex == LastSlotIndexDelegate || !bCanSwitchWeapon)
  {
    return;
  }

  UAnimInstance *AnimInstance = GetMesh()->GetAnimInstance();

  if (AnimInstance && EquipMontage)
  {
    if (CombatState == ECombatState::ECS_Reloading)
    {
      EquippedWeapon->SetMovingClip(false);
    }

    CombatState = ECombatState::ECS_Equipping;

    AnimInstance->Montage_Play(EquipMontage, 1.0f);
    AnimInstance->Montage_JumpToSection(FName("Equip"));
  }

  auto NewWeapon = Cast<AWeapon>(Inventory[NewItemIndex]);
  WeaponToGrab = NewWeapon;

  EquipItemDelegate.Broadcast(LastSlotIndexDelegate, NewWeapon->GetSlotIndex());

  LastSlotIndexDelegate = NewWeapon->GetSlotIndex();
}

void AShooterCharacter::GrabWeapon()
{
  EquippedWeapon->SetItemState(EItemState::EIS_PickedUp);
  EquipWeapon(WeaponToGrab);
  WeaponToGrab = nullptr;
}

void AShooterCharacter::FinishEquipping()
{
  CombatState = ECombatState::ECS_Unoccupied;
}

void AShooterCharacter::TriggerRecoil()
{
  float const RecoilFactor = (100.0f - EquippedWeapon->GetStability()) / 100.0f;

  if (bAiming)
  {
    VerticalRecoil = -RecoilAmount * RecoilFactor;
    HorizontalRecoil = FMath::RandRange(-RecoilAmount / 4, RecoilAmount / 4) * RecoilFactor;
  }
  else
  {
    VerticalRecoil = -RecoilAmount * RecoilFactor * 2.0f;
    HorizontalRecoil = FMath::RandRange(-RecoilAmount / 4, RecoilAmount / 4) * RecoilFactor * 1.5f;
  }
}

void AShooterCharacter::ResetCombatState(ECombatState NewCombatState)
{
  if (!InInterruptableCombatState())
    return;

  switch (CombatState)
  {
  case ECombatState::ECS_Sprinting:
  {
    EndSprint();
    break;
  }
  case ECombatState::ECS_Equipping:
  case ECombatState::ECS_Reloading:
  {
    GetMesh()->GetAnimInstance()->StopAllMontages(0);
    break;
  }
  }

  CombatState = NewCombatState;
}

bool AShooterCharacter::InInterruptableCombatState()
{
  return CombatState == ECombatState::ECS_Unoccupied ||
         CombatState == ECombatState::ECS_Equipping ||
         CombatState == ECombatState::ECS_Reloading ||
         CombatState == ECombatState::ECS_Sprinting;
}

void AShooterCharacter::FinishDodge()
{
  CombatState = ECombatState::ECS_Unoccupied;
}

int32 AShooterCharacter::GetMovementInputDirection(int32 InputX, int32 InputY)
{
  switch (InputY)
  {
  case 1:
  {
    switch (InputX)
    {
    case 1:
    {
      return 45;
    }
    case 0:
    {
      return 0;
    }
    case -1:
    {
      return 315;
    }
    }
  }
  case 0:
  {
    switch (InputX)
    {
    case 1:
    {
      return 90;
    }
    case 0:
    {
      return -1;
    }
    case -1:
    {
      return 270;
    }
    }
  }
  case -1:
  {
    switch (InputX)
    {
    case 1:
    {
      return 135;
    }
    case 0:
    {
      return 180;
    }
    case -1:
    {
      return 225;
    }
    }
  }
  default:
  {
    return -1;
  }
  }
}

void AShooterCharacter::PlayDodgeAnimation(int32 Direction)
{
  UAnimInstance *AnimInstance = GetMesh()->GetAnimInstance();
  if (AnimInstance && DodgeMontage)
  {
    AnimInstance->Montage_Play(DodgeMontage);

    switch (Direction)
    {
    case 0: // Forward
    {
      AnimInstance->Montage_JumpToSection(FName("DodgeForward"));
      break;
    }
    case 45: // Forward-Right
    {
      AnimInstance->Montage_JumpToSection(FName("DodgeFR"));
      break;
    }
    case 90: // Right
    {
      AnimInstance->Montage_JumpToSection(FName("DodgeRight"));
      break;
    }
    case 135: // Backward-Right
    {
      AnimInstance->Montage_JumpToSection(FName("DodgeBR"));
      break;
    }
    case 180: // Backward
    {
      AnimInstance->Montage_JumpToSection(FName("DodgeBackward"));
      break;
    }
    case 225: // Backward-Left
    {
      AnimInstance->Montage_JumpToSection(FName("DodgeBL"));
      break;
    }
    case 270: // Left
    {
      AnimInstance->Montage_JumpToSection(FName("DodgeLeft"));
      break;
    }
    case 315: // Forward-Left
    {
      AnimInstance->Montage_JumpToSection(FName("DodgeFL"));
      break;
    }
    default:
    {
      AnimInstance->Montage_JumpToSection(FName("DodgeBackward"));
    }
    }
  }
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  CameraInterpZoom(DeltaTime);
  SetLookRate();
  CalculateCrosshairSpread(DeltaTime);
  TraceForItems();
  ApplyRecoil(DeltaTime);
}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
  Super::SetupPlayerInputComponent(PlayerInputComponent);

  if (UEnhancedInputComponent *EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
  {
    EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AShooterCharacter::Move);
    EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AShooterCharacter::EndSprint);
    EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AShooterCharacter::Look);
    EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AShooterCharacter::Jump);
    EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AShooterCharacter::Sprint);
    EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Started, this, &AShooterCharacter::Dodge);

    EnhancedInputComponent->BindAction(FireWeaponAction, ETriggerEvent::Started, this, &AShooterCharacter::FireButtonPressed);
    EnhancedInputComponent->BindAction(FireWeaponAction, ETriggerEvent::Completed, this, &AShooterCharacter::FireButtonReleased);

    EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this, &AShooterCharacter::Aim);
    EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &AShooterCharacter::Aim);

    EnhancedInputComponent->BindAction(SelectAction, ETriggerEvent::Started, this, &AShooterCharacter::Select);

    EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &AShooterCharacter::Reload);

    EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AShooterCharacter::Crouch);

    EnhancedInputComponent->BindAction(SwitchWeapon1Action, ETriggerEvent::Started, this, &AShooterCharacter::SwitchWeapon1);
    EnhancedInputComponent->BindAction(SwitchWeapon2Action, ETriggerEvent::Started, this, &AShooterCharacter::SwitchWeapon2);
    EnhancedInputComponent->BindAction(SwitchWeapon3Action, ETriggerEvent::Started, this, &AShooterCharacter::SwitchWeapon3);
    EnhancedInputComponent->BindAction(SwitchMeleeAction, ETriggerEvent::Started, this, &AShooterCharacter::SwitchMelee);
    EnhancedInputComponent->BindAction(NextWeaponAction, ETriggerEvent::Started, this, &AShooterCharacter::NextWeapon);
  }
}

void AShooterCharacter::FinishReloading()
{
  // Update the combat state
  CombatState = ECombatState::ECS_Unoccupied;

  if (!EquippedWeapon)
    return;

  const auto AmmoType = EquippedWeapon->GetAmmoType();
  // Update carrying ammo amount
  if (AmmoMap.Contains(AmmoType))
  {
    int32 CarriedAmmo = AmmoMap[AmmoType];

    const int32 MagEmptySpace = EquippedWeapon->GetMagazineCapacity() - EquippedWeapon->GetAmmo();

    if (MagEmptySpace > CarriedAmmo)
    {
      // Reload the magazine with all the ammo we are carrying
      EquippedWeapon->ReloadAmmo(CarriedAmmo);
      CarriedAmmo = 0;
      AmmoMap.Add(AmmoType, CarriedAmmo);
    }
    else
    {
      // Fill the magazine
      EquippedWeapon->ReloadAmmo(MagEmptySpace);
      CarriedAmmo -= MagEmptySpace;
      AmmoMap.Add(AmmoType, CarriedAmmo);
    }
  }

  if (bFireButtonPressed && EquippedWeapon->GetAutomatic())
  {
    FireWeapon();
  }
}

void AShooterCharacter::ResetPickupSoundTimer()
{
  bShouldPlayPickupSound = true;
}

void AShooterCharacter::ResetEquipSoundTimer()
{
  bShouldPlayEquipSound = true;
}

float AShooterCharacter::GetCrosshairSpreadMultiplier() const
{
  return CrosshairSpreadMultiplier;
}

// No longer needed
// FVector AShooterCharacter::GetCameraInterpLocation()
// {
//   const FVector CameraWorldLocation{ FollowCamera->GetComponentLocation() };
//   const FVector CameraForward{ FollowCamera->GetForwardVector() };
//   // Desired = CameraWorldLocation + Forward * A + Upward * B
//   return CameraWorldLocation + CameraForward * CameraInterpDistance + FVector(0.f, 0.f, CameraInterpElevation);
// }

void AShooterCharacter::GetPickupItem(AItem *Item)
{
  Item->PlayEquipSound();

  auto Weapon = Cast<AWeapon>(Item);
  if (Weapon)
  {
    if (Inventory.Num() < MAIN_INVENTORY_CAPACITY)
    {
      Weapon->SetSlotIndex(Inventory.Num());
      Inventory.Add(Weapon);
      Weapon->SetItemState(EItemState::EIS_PickedUp);
    }
    else
    {
      SwapWeapon(Weapon);
    }
  }

  auto Ammo = Cast<AAmmo>(Item);
  if (Ammo)
  {
    PickupAmmo(Ammo);
  }
}

FInterpLocation AShooterCharacter::GetInterpLocation(int32 Index)
{
  if (Index <= InterpLocations.Num())
  {
    return InterpLocations[Index];
  }

  return FInterpLocation();
}

int32 AShooterCharacter::GetInterpLocationIndex()
{
  int32 LowestIndex = 1;
  int32 LowestCount = INT_MAX;
  for (int32 i = 1; i < InterpLocations.Num(); i++)
  {
    if (InterpLocations[i].ItemCount < LowestCount)
    {
      LowestIndex = i;
      LowestCount = InterpLocations[i].ItemCount;
    }
  }

  return LowestIndex;
}

void AShooterCharacter::IncrementInterpLocItemCount(int32 Index, int32 Amount)
{
  if (Amount < -1 || Amount > 1)
    return;

  if (InterpLocations.Num() >= Index)
  {
    InterpLocations[Index].ItemCount += Amount;
  }
}

void AShooterCharacter::StartPickupSoundTimer()
{
  bShouldPlayPickupSound = false;
  GetWorldTimerManager().SetTimer(
      PickupSoundTimer,
      this,
      &AShooterCharacter::ResetPickupSoundTimer,
      PickupSoundResetTime);
}

void AShooterCharacter::StartEquipSoundTimer()
{
  bShouldPlayEquipSound = false;
  GetWorldTimerManager().SetTimer(
      EquipSoundTimer,
      this,
      &AShooterCharacter::ResetEquipSoundTimer,
      EquipSoundResetTime);
}

void AShooterCharacter::StartDodgeTimer()
{
  bCanDodge = false;
  GetWorldTimerManager().SetTimer(
      DodgeTimer,
      this,
      &AShooterCharacter::ResetDodgeTimer,
      1.5f);
}

void AShooterCharacter::ResetDodgeTimer()
{
  bCanDodge = true;
}