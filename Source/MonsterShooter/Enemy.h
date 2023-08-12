// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BulletHitInterface.h"
#include "Enemy.generated.h"

UENUM(BlueprintType)
enum class EEnemyState : uint8
{
  EES_Unoccupied UMETA(DisplayName = "Unoccupied"),
  EES_Attacking UMETA(DisplayName = "Attacking"),
  EES_Rushing UMETA(DisplayName = "Rushing"),

  EES_Staggered UMETA(DisplayName = "Staggered"),
  EES_Dead UMETA(DisplayName = "Dead"),

  EES_MAX UMETA(DisplayName = "DefaultMAX")
};

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

  void PlayMontage(UAnimMontage *Montage, FName Section, float PlayRate = 1.0f);

  void ResetHitReactTimer();

  void Stagger();

  UFUNCTION(BlueprintCallable)
  void StoreHitNumber(UUserWidget *HitNumber, FVector Location);

  UFUNCTION(BlueprintCallable)
  void DestroyHitNumber(UUserWidget *HitNumber, FVector Location);

  /** Called when something overlaps with the agro sphere */
  UFUNCTION()
  void AgroSphereOverlap(
      UPrimitiveComponent *OverlappedComponent,
      AActor *OtherActor,
      UPrimitiveComponent *OtherComp,
      int32 OtherBodyIndex,
      bool bFromSweep,
      const FHitResult &SweepResult);

  UFUNCTION(BlueprintCallable)
  void SetStaggered(bool Staggered);

  UFUNCTION()
  void CombatRangeOverlap(
      UPrimitiveComponent *OverlappedComponent,
      AActor *OtherActor,
      UPrimitiveComponent *OtherComp,
      int32 OtherBodyIndex,
      bool bFromSweep,
      const FHitResult &SweepResult);

  UFUNCTION()
  void CombatRangeEndOverlap(
      UPrimitiveComponent *OverlappedComponent,
      AActor *OtherActor,
      UPrimitiveComponent *OtherComp,
      int32 OtherBodyIndex);

  UFUNCTION(BlueprintCallable)
  void AttackPlayer(FName MontageSection);

  UFUNCTION(BlueprintPure)
  FName GetAttackSectionName();

  UFUNCTION(BlueprintCallable)
  void RushAttackStart();

  UFUNCTION(BlueprintCallable)
  void RushAttackEnd();

  UFUNCTION()
  void OnWeaponOverlap(
      UPrimitiveComponent *OverlappedComponent,
      AActor *OtherActor,
      UPrimitiveComponent *OtherComp,
      int32 OtherBodyIndex,
      bool bFromSweep,
      const FHitResult &SweepResult);

  // Activate / deactivate weapon collision
  UFUNCTION(BlueprintCallable)
  void ActivateLeftWeapon();
  UFUNCTION(BlueprintCallable)
  void DeactivateLeftWeapon();
  UFUNCTION(BlueprintCallable)
  void ActivateRightWeapon();
  UFUNCTION(BlueprintCallable)
  void DeactivateRightWeapon();

  void DoDamage(AActor *Target, const FHitResult &SweepResult);

  void ResetCanAttack();

  UFUNCTION(BlueprintCallable)
  void FinishDeath();

private:
  /** Particles to spawn when hit by bullets */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
  class UParticleSystem *ImpactParticles;

  /** Sound to play when hit by bullets */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
  class USoundCue *ImpactSound;

  /** Name of the bone that represents the weakspot */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
  FString WeakspotBone;

  /** How much time the health bar remains displayed when the enemy is hit */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
  float HealthBarDisplayTime;

  FTimerHandle HealthBarTimer;

  /** Montage with stagger hit */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
  UAnimMontage *StaggerMontage;

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
  FVector PatrolPoint1;

  /** Point for the enemy to move to */
  UPROPERTY(EditAnywhere, Category = "AI", meta = (AllowPrivateAccess = "true", MakeEditWidget = "true"))
  FVector PatrolPoint2;

  class AEnemyController *EnemyController;

  /** Overlap sphere for when the enemy becomes hostile */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
  class USphereComponent *AgroSphere;

  /** Whether the enemy is staggered */
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
  bool bStaggered;

  /** Current balance value. When it reaches zero, the enemy gets staggered */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
  float Balance;

  /** Maximum balance capacity */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
  float MaxBalance;

  /** Rate at which the balance bar is recovered */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
  float BalanceRecoveryRate;

  /** True when in attack range, time to attack */
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
  bool bInAttackRange;

  /** Sphere for attack range */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
  USphereComponent *CombatRangeSphere;

  /** Montage with attack animations */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
  UAnimMontage *AttackMontage;

  // Attack montage section names
  FName AttackL;
  FName AttackR;
  FName AttackLFast;
  FName AttackRFast;
  FName RushAttackSection;

  /** Collision volume for the left weapon */
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
  class UBoxComponent *LeftWeaponCollision;

  /** Collision volume for the right weapon */
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
  UBoxComponent *RightWeaponCollision;

  /** Damage dealt by basic attacks */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
  float BasicAttackDamage;

  /** Whether the enemy can attack or not */
  UPROPERTY(VisibleAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
  bool bCanAttack;

  FTimerHandle AttackWaitTimer;

  /** Wait time between attacks */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
  float AttackWaitTime;

  /** Death anim montage */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
  UAnimMontage *DeathMontage;

  /** Whether the enemy is dead or not */
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
  bool bDead;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
  class UHealthComponent *HealthComponent;

  /** Whether the enemy is dead or not */
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
  EEnemyState EnemyState;

  float BaseMovementSpeed;

public:
  // Called every frame
  virtual void Tick(float DeltaTime) override;

  // Called to bind functionality to input
  virtual void SetupPlayerInputComponent(class UInputComponent *PlayerInputComponent) override;

  virtual void BulletHit_Implementation(FHitResult HitResult, AActor *Shooter, AController *InstigatorController) override;

  virtual float TakeDamage(float DamageAmount, struct FDamageEvent const &DamageEvent, AController *EventInstigator, AActor *DamageCauser) override;

  void TakeBalanceDamage(float Amount);

  void SetEnemyState(EEnemyState State);

  UFUNCTION(BlueprintImplementableEvent)
  void ShowHitNumber(int32 Damage, FVector HitLocation, bool bWeakspot);

  FORCEINLINE FString GetWeakspotBone() const { return WeakspotBone; }
  FORCEINLINE UBehaviorTree *GetBehaviorTree() const { return BehaviorTree; }

  FORCEINLINE void SetBalance(float Amount) { Balance = Amount; }
  float GetHealth() const;
  FORCEINLINE bool IsDead() const { return bDead; }
};
