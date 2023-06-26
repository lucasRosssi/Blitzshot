// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ShooterAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EOffsetState : uint8
{
  EOS_Aiming UMETA(DisplayName = "Aiming"),
  EOS_Hip UMETA(DisplayName = "Hip"),
  EOS_Reloading UMETA(DisplayName = "Reloading"),
  EOS_InAir UMETA(DisplayName = "InAir"),

  EOS_MAX UMETA(DisplayName = "DefaultMAX")
};

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

  /** Handle leaning while running */
  void Lean(float DeltaTime);

private:
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
  class AShooterCharacter *ShooterCharacter;

  // The speed of the character
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
  float Speed;

  // Whether or not the character is in the air
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
  bool bIsInAir;

  // Whether or not the character is moving
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
  bool bIsAccelerating;

  // Offset yaw used for strafing
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
  float MovementOffsetYaw;

  // Offset yaw the frame before we stopped moving
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
  float LastMovementOffsetYaw;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
  bool bAiming;

  /** Turn in place Yaw of the character this frame */
  float TIPCharacterYaw;

  /** Turn in place Yaw of the character the previous frame */
  float TIPCharacterYawLastFrame;

  /** Yaw offset of root bone to work with hip aim and turn in place */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
  float RootYawOffset;

  /** Rotation curve value this frame */
  float RotationCurve;

  /** Rotation curve value last frame */
  float RotationCurveLastFrame;

  /** Pitch of aim rotation, used for Aim Offset */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
  float Pitch;

  /** True when reloading, used to prevent Aim Offset while reloading */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
  bool bReloading;

  /** State used to determine which aim offset to use */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
  EOffsetState OffsetState;

  /** Character rotation this frame */
  FRotator CharacterRotation;

  /** Character rotation last frame */
  FRotator CharacterRotationLastFrame;

  /** Yaw delta used for leaning */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Lean, meta = (AllowPrivateAccess = "true"))
  float YawDelta;

  /** True when crouching */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Crouching, meta = (AllowPrivateAccess = "true"))
  bool bCrouching;

  /** Whether or not we should use FABRIK in the animation */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
  bool bShouldUseFABRIK;
};
