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
  void Look(const FInputActionValue& Value);
  void Jump(const FInputActionValue& Value);

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
  class UInputMappingContext* PlayerMappingContext;
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
  class UInputAction* MoveAction;
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
  UInputAction* LookAction;
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
  UInputAction* JumpAction;

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

public:
  // Returns CameraBoom subobject
  FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
  // Returns FollowCamera subobject
  FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

};