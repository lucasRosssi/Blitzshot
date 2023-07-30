// Fill out your copyright notice in the Description page of Project Settings.

#include "Explosive.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AExplosive::AExplosive() : BaseDamage(100.f)
{
  // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
  PrimaryActorTick.bCanEverTick = true;

  ExplosiveMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ExplosiveMesh"));
  SetRootComponent(ExplosiveMesh);

  OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
  OverlapSphere->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AExplosive::BeginPlay()
{
  Super::BeginPlay();
}

// Called every frame
void AExplosive::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);
}

void AExplosive::BulletHit_Implementation(FHitResult HitResult, AActor *Shooter, AController *InstigatorController)
{
  if (ImpactSound)
  {
    UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
  }

  if (ExplodeParticles)
  {
    UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplodeParticles, HitResult.Location, FRotator(0.f), true);
  }

  // Apply explosive damage
  TArray<AActor *> OverlappingActors;
  GetOverlappingActors(OverlappingActors, ACharacter::StaticClass());

  UGameplayStatics::ApplyRadialDamageWithFalloff(
      GetWorld(),
      BaseDamage,
      BaseDamage * 0.1f,
      GetActorLocation(),
      OverlapSphere->GetScaledSphereRadius() * 0.3f,
      OverlapSphere->GetScaledSphereRadius(),
      100.f,
      UDamageType::StaticClass(),
      {},
      this,
      InstigatorController);

  Destroy();
}
