// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BulletHitInterface.h"
#include "Explosive.generated.h"

UCLASS()
class MONSTERSHOOTER_API AExplosive : public AActor, public IBulletHitInterface
{
  GENERATED_BODY()

public:
  // Sets default values for this actor's properties
  AExplosive();

protected:
  // Called when the game starts or when spawned
  virtual void BeginPlay() override;

private:
  /** Explosion when hit by a bullet */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
  class UParticleSystem *ExplodeParticles;

  /** Sound to play when hit by bullets */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
  class USoundCue *ImpactSound;

  /** Mesh for the explosive */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
  class UStaticMeshComponent *ExplosiveMesh;

  /** Volume used to determine the Actors that overlap it during explosion */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
  class USphereComponent *OverlapSphere;

  /** Amount of damage dealt in the center of the explosion */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
  float BaseDamage;

public:
  // Called every frame
  virtual void Tick(float DeltaTime) override;

  virtual void BulletHit_Implementation(FHitResult HitResult, AActor *Shooter, AController *InstigatorController) override;
};
