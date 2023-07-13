// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BulletHitInterface.h"
#include "Enemy.generated.h"

UCLASS()
class MONSTERSHOOTER_API AEnemy : public ACharacter, public IBulletHitInterface
{
  GENERATED_BODY()

public:
  // Sets default values for this character's properties
  AEnemy();

protected:
  // Called when the game starts or when spawned
  virtual void BeginPlay() override;

  UFUNCTION(BlueprintNativeEvent)
  void ShowHealthBar();
  void ShowHealthBar_Implementation();

  UFUNCTION(BlueprintImplementableEvent)
  void HideHealthBar();

  void Die();

  void PlayHitMontage(FName Section, float PlayRate = 1.0f);

  void ResetHitReactTimer();

  UFUNCTION(BlueprintCallable)
  void StoreHitNumber(UUserWidget *HitNumber, FVector Location);

  UFUNCTION(BlueprintCallable)
  void DestroyHitNumber(UUserWidget *HitNumber, FVector Location);

private:
  /** Particles to spawn when hit by bullets */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
  class UParticleSystem *ImpactParticles;

  /** Sound to play when hit by bullets */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
  class USoundCue *ImpactSound;

  /** Current health of the enemy */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
  float Health;

  /** Maximum health of the enemy */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
  float MaxHealth;

  /** Name of the bone that represents the weakspot */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
  FString WeakspotBone;

  /** How much time the health bar remains displayed when the enemy is hit */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
  float HealthBarDisplayTime;

  FTimerHandle HealthBarTimer;

  /** Montage with shot hit and enemy death animations */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
  UAnimMontage *HitMontage;

  FTimerHandle HitReactTimer;

  bool bCanHitReact;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
  float HitReactTimeMin;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
  float HitReactTimeMax;

  /** Map to store HitNumber widgets and their locations */
  UPROPERTY(VisibleAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
  TMap<UUserWidget *, FVector> HitNumbers;

  /** Time before a hit number is removed from the screen */
  UPROPERTY(EditAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
  float HitNumberDestroyTime;

  /** Behavior tree for the AI Character */
  UPROPERTY(EditAnywhere, Category = "AI", meta = (AllowPrivateAccess = "true"))
  class UBehaviorTree *BehaviorTree;

  /** Point for the enemy to move to */
  UPROPERTY(EditAnywhere, Category = "AI", meta = (AllowPrivateAccess = "true", MakeEditWidget = "true"))
  FVector PatrolPoint;

public:
  // Called every frame
  virtual void Tick(float DeltaTime) override;

  // Called to bind functionality to input
  virtual void SetupPlayerInputComponent(class UInputComponent *PlayerInputComponent) override;

  virtual void BulletHit_Implementation(FHitResult HitResult) override;

  virtual float TakeDamage(float DamageAmount, struct FDamageEvent const &DamageEvent, AController *EventInstigator, AActor *DamageCauser) override;

  FORCEINLINE FString GetWeakspotBone() const { return WeakspotBone; }

  UFUNCTION(BlueprintImplementableEvent)
  void ShowHitNumber(int32 Damage, FVector HitLocation, bool bWeakspot);

  FORCEINLINE UBehaviorTree *GetBehaviorTree() const { return BehaviorTree; }
};
