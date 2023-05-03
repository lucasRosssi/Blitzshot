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
  BaseMovementSpeed(600.f),
  AimMovementSpeed(300.f),
  // Pickup sound timer properties
  bShouldPlayPickupSound(true),
  bShouldPlayEquipSound(true),
  PickupSoundResetTime(0.2f),
  EquipSoundResetTime(0.2f)
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

  // Create Hand Scene component
  HandSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("HandSceneComp"));

  // Configure character movement
  GetCharacterMovement()->bOrientRotationToMovement = false; // Character moves in the direction of input...
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

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

  if (FollowCamera)
  {
    CameraDefaultFOV = GetFollowCamera()->FieldOfView;
    CameraCurrentFOV = CameraDefaultFOV;
  }

  if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
  {
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
    {
      Subsystem->AddMappingContext(PlayerMappingContext, 0);
    }
  }

  GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;

  // Spawn the default weapon and equip it
  EquipWeapon(SpawnDefaultWeapon());

  InitializeAmmoMap();

  // Create FInterpLocation structs for each interp locations and add to array
  InitializeInterpLocations();
	
}

/* START INPUT ACTIONS */

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

void AShooterCharacter::Aim(const FInputActionValue &Value)
{
  bAiming = Value.Get<bool>();

  if (
    GetCharacterMovement()->IsFalling() ||
    CombatState == ECombatState::ECS_Reloading
  )
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

void AShooterCharacter::FireButtonPressed(const FInputActionValue& Value)
{
  bFireButtonPressed = Value.Get<bool>();
  FireWeapon();
}

void AShooterCharacter::Select(const FInputActionValue& Value)
{
  if (TraceHitItem)
  {
    auto TraceHitAmmo = Cast<AAmmo>(TraceHitItem);
    if (TraceHitAmmo)
    {
      TraceHitAmmo->GetAmmoCollisionSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    TraceHitItem->StartItemCurve(this);
  }
}

void AShooterCharacter::Reload(const FInputActionValue& Value)
{
  ReloadWeapon();
}

void AShooterCharacter::Crouch(const FInputActionValue& Value)
{
  if (GetCharacterMovement()->IsFalling())
  {
    bCrouching = false;
  }

  bCrouching = !bCrouching;
}

/* END INPUT ACTIONS */

void AShooterCharacter::FireWeapon()
{
  if (!EquippedWeapon || CombatState != ECombatState::ECS_Unoccupied) return;

  if (!WeaponHasAmmo())
  {
    ReloadWeapon();
    return;
  }

  PlayFireSound();
  SendBullet();
  PlayGunFireMontage();
  StartCrosshairBulletFire();
  EquippedWeapon->ConsumeAmmo();

  StartFireTimer();
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

void AShooterCharacter::StartFireTimer()
{
  CombatState = ECombatState::ECS_FireTimerInProgress;

  GetWorldTimerManager().SetTimer(
    AutoFireTimer,
    this,
    &AShooterCharacter::AutoFireReset,
    AutomaticFireRate
  );
  
}

void AShooterCharacter::AutoFireReset()
{
  CombatState = ECombatState::ECS_Unoccupied;

  if (WeaponHasAmmo())
  {
    if (bFireButtonPressed)
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

  TraceHitItem = Cast<AItem>(ItemTraceResult.GetActor());
  if (TraceHitItem && TraceHitItem->GetPickupWidget())
  {
    // Show item's Pickup Widget
    TraceHitItem->GetPickupWidget()->SetVisibility(true);
  }

  if (LastTraceHitItem && LastTraceHitItem != TraceHitItem)
  {
    // Hide widget from previous HitItem
    LastTraceHitItem->GetPickupWidget()->SetVisibility(false);
  }

  // Store a reference of HitItem
  LastTraceHitItem = TraceHitItem;
}

AWeapon* AShooterCharacter::SpawnDefaultWeapon()
{
  // Check the TSubclassOf variable
  if (!DefaultWeaponClass)
  {
    return nullptr;
  }

  // Spawn the Weapon
  return GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
}

void AShooterCharacter::EquipWeapon(class AWeapon* WeaponToEquip)
{
  if (!WeaponToEquip)
  {
    return;
  }

  // Get the Hand Socket
  const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));

  if (HandSocket)
  {
    // Attach the Weapon to the hand socket RightHandSocket
    HandSocket->AttachActor(WeaponToEquip, GetMesh());
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

void AShooterCharacter::SwapWeapon(AWeapon* WeaponToSwap)
{
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
  if (!EquippedWeapon) return false;

  return EquippedWeapon->GetAmmo() > 0;
}

void AShooterCharacter::PlayFireSound()
{
  if (FireSound)
  {
    UGameplayStatics::PlaySound2D(this, FireSound);
  }
}

void AShooterCharacter::SendBullet()
{
  const USkeletalMeshSocket* BarrelSocket = EquippedWeapon->GetItemMesh()->GetSocketByName("BarrelSocket");
  if (BarrelSocket)
  {
    const FTransform SocketTransform = BarrelSocket->GetSocketTransform(EquippedWeapon->GetItemMesh());
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
}

void AShooterCharacter::PlayGunFireMontage()
{
  UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
  if (AnimInstance && HipFireMontage && !bAiming)
  {
    AnimInstance->Montage_Play(HipFireMontage);
    AnimInstance->Montage_JumpToSection(FName("StartFire"));
  }

  if (AnimInstance && AimFireMontage && bAiming)
  {
    AnimInstance->Montage_Play(AimFireMontage);
    AnimInstance->Montage_JumpToSection(FName("StartAimFire"));
  }
}

void AShooterCharacter::ReloadWeapon()
{
  if (!EquippedWeapon || CombatState != ECombatState::ECS_Unoccupied) return;

  // Do we have ammo of the correct type?
  if (CarryingAmmo() && !EquippedWeapon->ClipIsFull())
  {
    CombatState = ECombatState::ECS_Reloading;

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (AnimInstance && ReloadMontage)
    {
      AnimInstance->Montage_Play(ReloadMontage, 1.2f);
      AnimInstance->Montage_JumpToSection(EquippedWeapon->GetReloadMontageSection());
    }
  }
}

bool AShooterCharacter::CarryingAmmo()
{
  if (!EquippedWeapon) return false;

  auto AmmoType = EquippedWeapon->GetAmmoType();

  if (AmmoMap.Contains(AmmoType))
  {
    return AmmoMap[AmmoType] > 0;
  }

  return false;
}

void AShooterCharacter::GrabClip()
{
  if (!EquippedWeapon || !HandSceneComponent) return;

  USkeletalMeshComponent* EquippedWeaponMesh = EquippedWeapon->GetItemMesh();
  FName ClipBoneName = EquippedWeapon->GetClipBoneName();
  // Index for the clip bone on the Equipped Weapon
  int32 ClipBoneIndex{ EquippedWeaponMesh->GetBoneIndex(ClipBoneName) };
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

void AShooterCharacter::PickupAmmo(AAmmo* Ammo)
{
  const auto AmmoType = Ammo->GetAmmoType();
  // Check if AmmoMap contains Ammo's ammo type
  if (AmmoMap.Find(AmmoType))
  {
    // Get amount of ammo in our AmmoMap for the ammo type
    int32 AmmoCount{ AmmoMap[AmmoType] };
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
  FInterpLocation WeaponLocation{ WeaponInterpComp, 0 };
  InterpLocations.Add(WeaponLocation);

  FInterpLocation InterpLocation1{ InterpComp1, 0 };
  InterpLocations.Add(InterpLocation1);
  FInterpLocation InterpLocation2{ InterpComp2, 0 };
  InterpLocations.Add(InterpLocation2);
  FInterpLocation InterpLocation3{ InterpComp3, 0 };
  InterpLocations.Add(InterpLocation3);
  FInterpLocation InterpLocation4{ InterpComp4, 0 };
  InterpLocations.Add(InterpLocation4);
  FInterpLocation InterpLocation5{ InterpComp5, 0 };
  InterpLocations.Add(InterpLocation5);
  FInterpLocation InterpLocation6{ InterpComp6, 0 };
  InterpLocations.Add(InterpLocation6);
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
    EnhancedInputComponent->BindAction(SelectAction, ETriggerEvent::Started, this, &AShooterCharacter::Select);
    EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &AShooterCharacter::Reload);
    EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AShooterCharacter::Crouch);
    
  }
}

void AShooterCharacter::FinishReloading()
{
  // Update the combat state
  CombatState = ECombatState::ECS_Unoccupied;

  if (!EquippedWeapon) return;

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

void AShooterCharacter::GetPickupItem(AItem* Item)
{
  Item->PlayEquipSound();

  auto Weapon = Cast<AWeapon>(Item);
  if (Weapon)
  {
    SwapWeapon(Weapon);
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
  if (Amount < -1 || Amount > 1) return;

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
    PickupSoundResetTime
  );
}

void AShooterCharacter::StartEquipSoundTimer()
{
  bShouldPlayEquipSound = false;
  GetWorldTimerManager().SetTimer(
    EquipSoundTimer,
    this,
    &AShooterCharacter::ResetEquipSoundTimer,
    EquipSoundResetTime
  );
}