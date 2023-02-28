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

// Sets default values
AShooterCharacter::AShooterCharacter() :
  bAiming(false),
  CameraDefaultFOV(0.f), // set in BeginPlay
  CameraZoomedFOV(40.f)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

  // Create a camera boom (pulls in towards the character if there is a collision)
  CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
  CameraBoom->SetupAttachment(RootComponent);
  CameraBoom->TargetArmLength = 260.f; // The camera follows at this distance behind the character
  CameraBoom->bUsePawnControlRotation = true; // Rotate the arm with the player controller
  CameraBoom->SocketOffset = FVector(0.f, 75.f, 70.f);

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
  }

  
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

  AddControllerPitchInput(LookAxisVector.Y);
  AddControllerYawInput(LookAxisVector.X);
}

void AShooterCharacter::Jump(const FInputActionValue &Value)
{
  Super::Jump();
}

void AShooterCharacter::FireWeapon(const FInputActionValue &Value)
{
  const bool bIsFiring = Value.Get<bool>();
  if (!bIsFiring) return;

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
  
}

void AShooterCharacter::Aim(const FInputActionValue &Value)
{
  bAiming = Value.Get<bool>();

  if (bAiming)
  {
    GetFollowCamera()->SetFieldOfView(CameraZoomedFOV);
  }
  else
  {
    GetFollowCamera()->SetFieldOfView(CameraDefaultFOV);
  }
}

bool AShooterCharacter::GetBeamEndLocation(
  const FVector &MuzzleSocketLocation,
  FVector & OutBeamLocation
)
{
  // Get current size of the viewport
  FVector2D ViewportSize;
  if (GEngine && GEngine->GameViewport)
  {
    GEngine->GameViewport->GetViewportSize(ViewportSize);
  }

  // Get screen space location of crosshair
  FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
  CrosshairLocation.Y -= 50.f;
  FVector CrosshairWorldPosition;
  FVector CrosshairWorldDirection;

  // Get world position and direction of crosshair
  bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
    UGameplayStatics::GetPlayerController(this, 0),
    CrosshairLocation,
    CrosshairWorldPosition,
    CrosshairWorldDirection
  );

  if (bScreenToWorld) // was the deprojection successful?
    {
      FHitResult ScreenTraceHit;
      const FVector Start{ CrosshairWorldPosition };
      const FVector End{ CrosshairWorldPosition + CrosshairWorldDirection * 50'000.f };

      // Set beam end point to line trace end point
      OutBeamLocation = End;
      // Trace outward from crosshair world location
      GetWorld()->LineTraceSingleByChannel(
        ScreenTraceHit,
        Start,
        End,
        ECollisionChannel::ECC_Visibility
      );

      if (ScreenTraceHit.bBlockingHit) // was there a trace hit?
      {
        // Beam end point is now trace hit location
        OutBeamLocation = ScreenTraceHit.Location;
      }

      // Perform a second trace from the gun barrel
      FHitResult WeaponTraceHit;
      const FVector WeaponTraceStart{ MuzzleSocketLocation };
      const FVector WeaponTraceEnd{ OutBeamLocation };
      GetWorld()->LineTraceSingleByChannel(
        WeaponTraceHit,
        WeaponTraceStart,
        WeaponTraceEnd,
        ECollisionChannel::ECC_Visibility
      );

      if (WeaponTraceHit.bBlockingHit) // object between barrel and BeamEndPoint?
      {
        OutBeamLocation = WeaponTraceHit.Location;
      }
      return true;
    }

  return false;
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

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
    EnhancedInputComponent->BindAction(FireWeaponAction, ETriggerEvent::Triggered, this, &AShooterCharacter::FireWeapon);
    EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &AShooterCharacter::Aim);
    EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &AShooterCharacter::Aim);
  }
}
