// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemy.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/KismetMathLibrary.h"
#include "EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SphereComponent.h"
#include "ShooterCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "HealthComponent.h"

// Sets default values
AEnemy::AEnemy() : HealthBarDisplayTime(4.f),
                   bCanHitReact(true),
                   HitReactTimeMin(0.1f),
                   HitReactTimeMax(0.5f),
                   HitNumberDestroyTime(1.5f),
                   Balance(100.f),
                   MaxBalance(100.f),
                   BalanceRecoveryRate(25.f),
                   AttackL(TEXT("AttackL")),
                   AttackR(TEXT("AttackR")),
                   AttackLFast(TEXT("AttackLFast")),
                   AttackRFast(TEXT("AttackRFast")),
                   RushAttackSection(TEXT("RushAttack")),
                   BasicAttackDamage(20.f),
                   bCanAttack(true),
                   AttackWaitTime(1.f),
                   bDead(false),
                   EnemyState(EEnemyState::EES_Unoccupied),
                   BaseMovementSpeed(400.0f)
{
  // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
  PrimaryActorTick.bCanEverTick = true;

  HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("Health Component"));
  HealthComponent->MaxHealth = 100.f;
  HealthComponent->bHealthRegenActive = false,

  // Create the Agro Sphere
      AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroSphere"));
  AgroSphere->SetupAttachment(GetRootComponent());

  // Create the Combat Range Sphere
  CombatRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatRangeSphere"));
  CombatRangeSphere->SetupAttachment(GetRootComponent());

  // Create the left and right weapons collision boxes
  LeftWeaponCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Left Weapon Box"));
  LeftWeaponCollision->SetupAttachment(GetMesh(), FName("LeftWeaponBone"));
  RightWeaponCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Right Weapon Box"));
  RightWeaponCollision->SetupAttachment(GetMesh(), FName("RightWeaponBone"));
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
  Super::BeginPlay();

  AgroSphere->OnComponentBeginOverlap.AddDynamic(
      this,
      &AEnemy::AgroSphereOverlap);
  CombatRangeSphere->OnComponentBeginOverlap.AddDynamic(
      this,
      &AEnemy::CombatRangeOverlap);
  CombatRangeSphere->OnComponentEndOverlap.AddDynamic(
      this,
      &AEnemy::CombatRangeEndOverlap);

  // Bind functions to overlap events for weapon boxes
  LeftWeaponCollision->OnComponentBeginOverlap.AddDynamic(
      this,
      &AEnemy::OnWeaponOverlap);
  RightWeaponCollision->OnComponentBeginOverlap.AddDynamic(
      this,
      &AEnemy::OnWeaponOverlap);
  // Set collision presets for weapon boxes
  LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  LeftWeaponCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
  LeftWeaponCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
  LeftWeaponCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
  RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  RightWeaponCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
  RightWeaponCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
  RightWeaponCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

  GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
  // Ignore camera from collision
  GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
  GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

  // Get the AI controller
  if (GetController())
  {
    EnemyController = Cast<AEnemyController>(GetController());
  }

  const FVector WorldPatrolPoint1 = UKismetMathLibrary::TransformLocation(
      GetActorTransform(),
      PatrolPoint1);
  const FVector WorldPatrolPoint2 = UKismetMathLibrary::TransformLocation(
      GetActorTransform(),
      PatrolPoint2);

  if (EnemyController)
  {
    EnemyController->GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolPoint1"), WorldPatrolPoint1);
    EnemyController->GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolPoint2"), WorldPatrolPoint2);
    EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("CanAttack"), true);

    EnemyController->RunBehaviorTree(BehaviorTree);
  }

  HealthComponent->Health = HealthComponent->MaxHealth;

  BaseMovementSpeed = GetCharacterMovement()->MaxWalkSpeed;
}

void AEnemy::ShowHealthBar_Implementation()
{
  GetWorldTimerManager().ClearTimer(HealthBarTimer);

  GetWorldTimerManager().SetTimer(
      HealthBarTimer,
      this,
      &AEnemy::HideHealthBar,
      HealthBarDisplayTime);
}

void AEnemy::Die()
{
  if (bDead)
    return;

  bDead = true;
  HideHealthBar();

  PlayMontage(DeathMontage, FName("DeathA"));

  SetActorEnableCollision(false);

  if (EnemyController)
  {
    EnemyController->GetBlackboardComponent()->SetValueAsBool(
        FName("Dead"),
        true);

    EnemyController->StopMovement();
  }
}

void AEnemy::PlayMontage(UAnimMontage *Montage, FName Section, float PlayRate)
{
  UAnimInstance *AnimInstance = GetMesh()->GetAnimInstance();
  if (AnimInstance && Montage)
  {
    AnimInstance->Montage_Play(Montage, PlayRate);
    AnimInstance->Montage_JumpToSection(Section, Montage);
  }
}

void AEnemy::ResetHitReactTimer()
{
  bCanHitReact = true;
}

void AEnemy::Stagger()
{
  const bool bCanStagger = !(EnemyState == EEnemyState::EES_Staggered || EnemyState == EEnemyState::EES_Dead || EnemyState == EEnemyState::EES_Roaring || EnemyState == EEnemyState::EES_Rushing);

  if (!bCanStagger)
    return;

  SetEnemyState(EEnemyState::EES_Staggered);
  PlayMontage(StaggerMontage, FName("HitReactFront"), 0.8f);
}

void AEnemy::StoreHitNumber(UUserWidget *HitNumber, FVector Location)
{
  HitNumbers.Add(HitNumber, Location);

  FTimerHandle HitNumberTimer;
  FTimerDelegate HitNumberDelegate;
  HitNumberDelegate.BindUFunction(this, FName("DestroyHitNumber"), HitNumber);
  GetWorld()->GetTimerManager().SetTimer(
      HitNumberTimer,
      HitNumberDelegate,
      HitNumberDestroyTime,
      false);
}

void AEnemy::DestroyHitNumber(UUserWidget *HitNumber, FVector Location)
{
  HitNumbers.Remove(HitNumber);
  // HitNumber->RemoveFromParent();
}

void AEnemy::AgroSphereOverlap(
    UPrimitiveComponent *OverlappedComponent,
    AActor *OtherActor,
    UPrimitiveComponent *OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult &SweepResult)
{
  if (!OtherActor)
    return;

  auto Character = Cast<AShooterCharacter>(OtherActor);
  if (Character)
  {
    // Set the value of Target Blackboard Key
    if (EnemyController && EnemyController->GetBlackboardComponent())
    {
      EnemyController->GetBlackboardComponent()->SetValueAsObject(TEXT("Target"), Character);
    }
  }
}

void AEnemy::CombatRangeOverlap(
    UPrimitiveComponent *OverlappedComponent,
    AActor *OtherActor,
    UPrimitiveComponent *OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult &SweepResult)
{
  if (!OtherActor)
    return;

  auto ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
  if (!ShooterCharacter)
    return;

  bInAttackRange = true;
  if (EnemyController)
  {
    EnemyController->GetBlackboardComponent()->SetValueAsBool(
        TEXT("InAttackRange"),
        true);
  }
}

void AEnemy::CombatRangeEndOverlap(
    UPrimitiveComponent *OverlappedComponent,
    AActor *OtherActor,
    UPrimitiveComponent *OtherComp,
    int32 OtherBodyIndex)
{
  if (!OtherActor)
    return;

  auto ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
  if (!ShooterCharacter)
    return;

  bInAttackRange = false;
  if (EnemyController)
  {
    EnemyController->GetBlackboardComponent()->SetValueAsBool(
        TEXT("InAttackRange"),
        false);
  }
}

void AEnemy::AttackPlayer(FName MontageSection)
{
  if (!bCanAttack)
    return;

  PlayMontage(AttackMontage, MontageSection);
  bCanAttack = false;
  GetWorldTimerManager().SetTimer(
      AttackWaitTimer,
      this,
      &AEnemy::ResetCanAttack,
      AttackWaitTime);

  if (EnemyController)
  {
    EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("CanAttack"), false);
  }
}

FName AEnemy::GetAttackSectionName()
{
  FName SectionName;
  const int32 Section{FMath::RandRange(1, 2)};

  if (EnemyState == EEnemyState::EES_Rushing)
  {
    return RushAttackSection;
  }

  switch (Section)
  {
  case 1:
    SectionName = AttackL;
    break;
  case 2:
    SectionName = AttackR;
    break;
  }
  return SectionName;
}

void AEnemy::RushAttackStart()
{
  SetEnemyState(EEnemyState::EES_Rushing);

  GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed * 2.5f;
}

void AEnemy::RushAttackEnd()
{
  SetEnemyState(EEnemyState::EES_Unoccupied);

  GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
}

void AEnemy::RageRoar(float Chance)
{
  if (EnemyState != EEnemyState::EES_Unoccupied)
  {
    return;
  }

  if (TriggerChance(Chance))
  {
    SetEnemyState(EEnemyState::EES_Roaring);
    PlayMontage(RoarMontage, FName("Roar"));
  }
}

void AEnemy::Taunt()
{
  PlayMontage(TauntMontage, FName("BackScratch"), 1.15f);
}

void AEnemy::Dodge(float Chance)
{
  const bool bCanDodge = EnemyState == EEnemyState::EES_Unoccupied;

  if (!bCanDodge)
    return;

  if (TriggerChance(Chance))
  {
    SetEnemyState(EEnemyState::EES_Dodging);

    FName DodgeDirection;

    const int32 RandomRoll = FMath::RandRange(0, 1);
    switch (RandomRoll)
    {
    case 0:
      DodgeDirection = FName("DodgeL");
      break;
    case 1:
      DodgeDirection = FName("DodgeR");
      break;
    }

    PlayMontage(DodgeMontage, DodgeDirection, 1.5f);
  }
}

void AEnemy::DoDamage(AActor *Target, const FHitResult &SweepResult)
{
  if (!Target)
    return;

  auto Character = Cast<AShooterCharacter>(Target);
  if (!Character || Character->IsInvulnerable())
    return;

  UGameplayStatics::ApplyDamage(Character,
                                BasicAttackDamage,
                                EnemyController,
                                this,
                                UDamageType::StaticClass());

  if (Character->IsDead())
  {
    Taunt();
    SetEnemyState(EEnemyState::EES_Taunting);
  }

  if (Character->GetMeleeImpactSound())
  {
    UGameplayStatics::PlaySoundAtLocation(
        this,
        Character->GetMeleeImpactSound(),
        Character->GetActorLocation());
  }

  if (Character->GetBloodParticles())
  {
    const int32 BoneIndex = Character->GetMesh()->GetBoneIndex(SweepResult.BoneName);

    UGameplayStatics::SpawnEmitterAtLocation(
        GetWorld(),
        Character->GetBloodParticles(),
        SweepResult.ImpactPoint);
  }

  Character->Stagger();
}

void AEnemy::OnWeaponOverlap(
    UPrimitiveComponent *OverlappedComponent,
    AActor *OtherActor,
    UPrimitiveComponent *OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult &SweepResult)
{
  DoDamage(OtherActor, SweepResult);
  // Character->GetMesh()->GetBoneLocation()
}

void AEnemy::ActivateLeftWeapon()
{
  LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeactivateLeftWeapon()
{
  LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::ActivateRightWeapon()
{
  RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeactivateRightWeapon()
{
  RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::ResetCanAttack()
{
  bCanAttack = true;
  if (EnemyController)
  {
    EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("CanAttack"), true);
  }
}

void AEnemy::FinishDeath()
{
  GetMesh()->bPauseAnims = true;

  SetLifeSpan(10.f);
}

bool AEnemy::TriggerChance(float Chance)
{
  float Trigger = FMath::RandRange(0.f, 1.f);

  if (Trigger <= Chance)
  {
    return true;
  }

  return false;
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
  Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AEnemy::BulletHit_Implementation(FHitResult HitResult, AActor *Shooter, AController *InstigatorController)
{
  if (ImpactSound)
  {
    UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
  }

  if (ImpactParticles)
  {
    UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, HitResult.Location, FRotator(0.f), true);
  }
}

float AEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const &DamageEvent, AController *EventInstigator, AActor *DamageCauser)
{
  if (EnemyController)
  {
    EnemyController->GetBlackboardComponent()->SetValueAsObject(
        FName("Target"),
        EventInstigator->GetPawn());
  }

  HealthComponent->TakeDamage(DamageAmount);

  if (HealthComponent->bDead)
  {
    Die();
  }
  else
  {
    ShowHealthBar();

    if (bCanAttack && EnemyState == EEnemyState::EES_Unoccupied)
    {
      if (TriggerChance(0.33f))
      {
        PlayMontage(HitMontage, FName("HitFront"));
      }
    }

    Dodge(0.06f);
    RageRoar(0.015f);
  }

  return DamageAmount;
}

void AEnemy::TakeBalanceDamage(float Amount)
{
  const bool bCanTakeBalanceDamage = !(EnemyState == EEnemyState::EES_Staggered || EnemyState == EEnemyState::EES_Rushing || EnemyState == EEnemyState::EES_Roaring);

  if (!bCanTakeBalanceDamage)
    return;

  if (Balance - Amount <= 0.f)
  {
    Balance = 0;
    Stagger();
  }
  else
  {
    Balance -= Amount;
  }
}

void AEnemy::SetEnemyState(EEnemyState State)
{
  EnemyState = State;
  uint8 EnumByte = (uint8)State;
  if (EnemyController && EnemyController->GetBlackboardComponent())
  {
    EnemyController->GetBlackboardComponent()->SetValueAsEnum(FName("EnemyState"), EnumByte);
  }
}

float AEnemy::GetHealth() const
{
  return HealthComponent->Health;
}
