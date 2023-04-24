// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ShooterAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class MONSTERSHOOTER_API UShooterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
  UShooterAnimInstance();

  UFUNCTION(BlueprintCallable)
  void UpdateAnimationProperties(float DeltaTime);

  virtual void NativeInitializeAnimation() override;

protected:
  /** Handle turning in place variables */
  void TurnInPlace();

private:
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
  class AShooterCharacter* ShooterCharacter;

  // The speed of the character
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
  float Speed;

  // Whether or not the character is in the air
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
  bool bIsInAir;

  // Whether or not the character is moving
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
  bool bIsAccelerating;

  // Offset yaw used for strafing
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
  float MovementOffsetYaw;

  // Offset yaw the frame before we stopped moving
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
  float LastMovementOffsetYaw;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
  bool bAiming;

  /** Yaw of the character this frame */
  float CharacterYaw;

  /** Yaw of the character the previous frame */
  float CharacterYawLastFrame;

  /** Yaw offset of root bone to work with hip aim and turn in place */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
  float RootYawOffset;

  /** Rotation curve value this frame */
  float RotationCurve;

  /** Rotation curve value last frame */
  float RotationCurveLastFrame;

  /** Pitch of aim rotation, used for Aim Offset */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
  float Pitch;

  /** True when reloading, used to prevent Aim Offset while reloading */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
  bool bReloading;
};
