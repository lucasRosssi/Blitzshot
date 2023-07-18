// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemy.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/KismetMathLibrary.h"
#include "EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SphereComponent.h"
#include "ShooterCharacter.h"

// Sets default values
AEnemy::AEnemy() : Health(100.f),
                   MaxHealth(100.f),
                   HealthBarDisplayTime(4.f),
                   bCanHitReact(true),
                   HitReactTimeMin(0.1f),
                   HitReactTimeMax(0.5f),
                   HitNumberDestroyTime(1.5f),
                   bStaggered(false),
                   Balance(100.f),
                   MaxBalance(100.f),
                   BalanceRecoveryRate(25.f)
{
  // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
  PrimaryActorTick.bCanEverTick = true;

  AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroSphere"));
  AgroSphere->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
  Super::BeginPlay();

  AgroSphere->OnComponentBeginOverlap.AddDynamic(
      this,
      &AEnemy::AgroSphereOverlap);

  GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

  // Get the AI controller
  EnemyController = Cast<AEnemyController>(GetController());

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

    EnemyController->RunBehaviorTree(BehaviorTree);
  }
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
  HideHealthBar();
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
  if (bStaggered)
    return;

  SetStaggered(true);
  PlayMontage(StaggerMontage, FName("HitReactFront"), 0.75f);
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
    EnemyController->GetBlackboardComponent()->SetValueAsObject(TEXT("Target"), Character);
  }
}

void AEnemy::SetStaggered(bool Staggered)
{
  bStaggered = Staggered;
  if (EnemyController)
  {
    EnemyController->GetBlackboardComponent()->SetValueAsBool(
        TEXT("Staggered"),
        Staggered);
  }
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

void AEnemy::BulletHit_Implementation(FHitResult HitResult)
{
  if (ImpactSound)
  {
    UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
  }

  if (ImpactParticles)
  {
    UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, HitResult.Location, FRotator(0.f), true);
  }

  ShowHealthBar();
  if (bStaggered)
    return;

  float Chance = FMath::RandRange(0.f, 1.f);
  if (Chance < 0.25f)
  {
    PlayMontage(HitMontage, FName("HitFront"));
  }
}

float AEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const &DamageEvent, AController *EventInstigator, AActor *DamageCauser)
{
  if (Health - DamageAmount <= 0.f)
  {
    Health = 0;
    Die();
  }
  else
  {
    Health -= DamageAmount;
  }

  return DamageAmount;
}

void AEnemy::TakeBalanceDamage(float Amount)
{
  if (bStaggered)
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
