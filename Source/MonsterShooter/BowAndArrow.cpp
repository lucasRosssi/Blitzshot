// Fill out your copyright notice in the Description page of Project Settings.

#include "BowAndArrow.h"
#include "Projectile.h"
#include "Engine/SkeletalMeshSocket.h"
#include "ShooterCharacter.h"

ABowAndArrow::ABowAndArrow()
{
  PrimaryActorTick.bCanEverTick = true;

  WeaponType = EWeaponType::EWT_BowAndArrow;
  bReloadable = false;
}

void ABowAndArrow::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);
}

void ABowAndArrow::SendProjectile()
{
  const USkeletalMeshSocket *ArrowSocket = GetItemMesh()->GetSocketByName("ArrowSocket");

  if (ArrowSocket)
  {
    FVector SpawnLocation = ArrowSocket->GetSocketLocation(GetItemMesh());
    FRotator SpawnRotation = GetActorRotation();

    FActorSpawnParameters SpawnParameters;
    SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    auto Player = Cast<AShooterCharacter>(Shooter);

    FVector ImpactPoint;
    FHitResult CrosshairHitResult;
    bool bTraceHit = Player->TraceUnderCrosshair(CrosshairHitResult, ImpactPoint, true);

    if (bTraceHit)
    {
      FVector Vector = ImpactPoint - SpawnLocation;
      SpawnRotation = Vector.Rotation();
    }

    if (ProjectileClass)
    {
      AActor *NewActor = GetWorld()->SpawnActor(
          ProjectileClass,
          &SpawnLocation,
          &SpawnRotation,
          SpawnParameters);

      AProjectile *NewProjectile = Cast<AProjectile>(NewActor);
      if (NewProjectile)
      {
        NewProjectile->SetWeapon(this);
      }
    }
  }
}