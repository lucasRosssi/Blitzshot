// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UShooterAnimInstance::NativeInitializeAnimation()
{
  ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
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
  // Get the speed of the character from velocity
  FVector Velocity{ ShooterCharacter->GetVelocity() };
  Velocity.Z = 0;
  Speed = Velocity.Size();

  // Is the character in the air?
  bIsInAir = ShooterCharacter->GetCharacterMovement()->IsFalling();

  // Is the character accelerating
  if (ShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0)
  {
    bIsAccelerating = true;
  }
  else
  {
    bIsAccelerating = false;
  }

}
