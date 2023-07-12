// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemy.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Blueprint/UserWidget.h"

// Sets default values
AEnemy::AEnemy() : Health(100.f),
                   MaxHealth(100.f),
                   HealthBarDisplayTime(4.f),
                   bCanHitReact(true),
                   HitReactTimeMin(0.1f),
                   HitReactTimeMax(0.5f),
                   HitNumberDestroyTime(1.5f)
{
  // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
  PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
  Super::BeginPlay();

  GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
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

void AEnemy::PlayHitMontage(FName Section, float PlayRate)
{
  if (!bCanHitReact)
    return;

  UAnimInstance *AnimInstance = GetMesh()->GetAnimInstance();
  if (AnimInstance && HitMontage)
  {
    AnimInstance->Montage_Play(HitMontage, PlayRate);
    AnimInstance->Montage_JumpToSection(Section, HitMontage);
  }

  bCanHitReact = false;
  const float HitReactTime{FMath::FRandRange(HitReactTimeMin, HitReactTimeMax)};
  GetWorldTimerManager().SetTimer(
      HitReactTimer,
      this,
      &AEnemy::ResetHitReactTimer,
      HitReactTime);
}

void AEnemy::ResetHitReactTimer()
{
  bCanHitReact = true;
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
  PlayHitMontage(FName("HitReactFront"), 1.5f);
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
