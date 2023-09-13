// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UShooterAnimInstance::UShooterAnimInstance() : Speed(0.f),
                                               bIsInAir(false),
                                               bIsAccelerating(false),
                                               MovementOffsetYaw(0.f),
                                               LastMovementOffsetYaw(0.f),
                                               bAiming(false),
                                               TIPCharacterYaw(0.f),
                                               TIPCharacterYawLastFrame(0.f),
                                               RootYawOffset(0.f),
                                               RotationCurve(0.f),
                                               RotationCurveLastFrame(0.f),
                                               Pitch(0.f),
                                               bReloading(false),
                                               OffsetState(EOffsetState::EOS_Hip),
                                               CharacterRotation(FRotator(0.f)),
                                               CharacterRotationLastFrame(FRotator(0.f)),
                                               YawDelta(0.f),
                                               bShouldUseFABRIK(false)
{
}

void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
  if (!ShooterCharacter)
  {
    ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
  }

  if (!ShooterCharacter)
  {
    return;
  }

  ECombatState CState = ShooterCharacter->GetCombatState();

  // Is the character in the air?
  bIsInAir = ShooterCharacter->GetCharacterMovement()->IsFalling();

  bReloading = CState == ECombatState::ECS_Reloading;

  bShouldUseFABRIK = (CState == ECombatState::ECS_Unoccupied || CState == ECombatState::ECS_FireTimerInProgress) && !bIsInAir;

  // Get the speed of the character from velocity
  FVector Velocity{ShooterCharacter->GetVelocity()};
  Velocity.Z = 0;
  Speed = Velocity.Size();

  // Is the character accelerating
  bIsAccelerating = ShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0;

  FRotator AimRotation = ShooterCharacter->GetBaseAimRotation();
  FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ShooterCharacter->GetVelocity());
  MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;

  if (ShooterCharacter->GetVelocity().Size() > 0.f)
  {
    LastMovementOffsetYaw = MovementOffsetYaw;
  }

  bAiming = ShooterCharacter->GetAiming();

  if (bReloading)
  {
    OffsetState = EOffsetState::EOS_Reloading;
  }
  else if (bIsInAir)
  {
    OffsetState = EOffsetState::EOS_InAir;
  }
  else if (ShooterCharacter->GetAiming())
  {
    OffsetState = EOffsetState::EOS_Aiming;
  }
  else if (CState == ECombatState::ECS_Dodging)
  {
    OffsetState = EOffsetState::EOS_Dodging;
  }
  else
  {
    OffsetState = EOffsetState::EOS_Hip;
  }

  TurnInPlace();
  Lean(DeltaTime);
}

void UShooterAnimInstance::NativeInitializeAnimation()
{
  ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}

void UShooterAnimInstance::TurnInPlace()
{
  if (!ShooterCharacter)
    return;

  Pitch = ShooterCharacter->GetBaseAimRotation().Pitch;

  if (Speed > 0 || bIsInAir)
  {
    // Don't want to turn in place, character is moving
    RootYawOffset = 0.f;
    TIPCharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
    TIPCharacterYawLastFrame = TIPCharacterYaw;
    RotationCurveLastFrame = 0.f;
    RotationCurve = 0.f;
  }
  else
  {
    TIPCharacterYawLastFrame = TIPCharacterYaw;
    TIPCharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
    const float TIPYawDelta{TIPCharacterYaw - TIPCharacterYawLastFrame};

    // Root Yaw Offset updated and clamped to [-180, 180]
    RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset - TIPYawDelta);

    // 1.0 if turning, 0.0 if not
    const float Turning{GetCurveValue(TEXT("Turning"))};
    if (Turning > 0)
    {
      RotationCurveLastFrame = RotationCurve;
      RotationCurve = GetCurveValue(TEXT("DistanceCurve"));
      const float DeltaRotation{RotationCurve - RotationCurveLastFrame};

      // RootYawOffset > 0 => turning left, RootYawOffset < 0 => turning right
      if (RootYawOffset > 0) // Turning left
      {
        RootYawOffset -= DeltaRotation;
      }
      else // Turning right
      {
        RootYawOffset += DeltaRotation;
      }

      const float ABSRootYawOffset{FMath::Abs(RootYawOffset)};
      if (ABSRootYawOffset > 60.f)
      {
        const float YawExcess{ABSRootYawOffset - 60.f};
        RootYawOffset > 0 ? RootYawOffset -= YawExcess : RootYawOffset += YawExcess;
      }
    }

    // DEBUG MESSAGE IN GAME
    // if (GEngine) GEngine->AddOnScreenDebugMessage(
    //   1,
    //   -1,
    //   FColor::Blue,
    //   FString::Printf(TEXT("CharacterYaw: %f"), CharacterYaw)
    // );
    // if (GEngine) GEngine->AddOnScreenDebugMessage(
    //   2,
    //   -1,
    //   FColor::Red,
    //   FString::Printf(TEXT("RootYawOffset: %f"), RootYawOffset)
    // );
  }
}

void UShooterAnimInstance::Lean(float DeltaTime)
{
  if (!ShooterCharacter)
    return;

  CharacterRotationLastFrame = CharacterRotation;
  CharacterRotation = ShooterCharacter->GetActorRotation();

  const FRotator Delta{UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame)};

  const float Target = Delta.Yaw / DeltaTime;
  const float Interp{FMath::FInterpTo(YawDelta, Target, DeltaTime, 6.f)};
  YawDelta = FMath::Clamp(Interp, -60.f, 60.f);
}
