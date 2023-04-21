// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

AWeapon::AWeapon() :
  ThrowWeaponTime(0.7f),
  bFalling(false),
  MagazineCapacity(30),
  Ammo(MagazineCapacity),
  WeaponType(EWeaponType::EWT_SubmachineGun),
  AmmoType(EAmmoType::EAT_9mm),
  ReloadMontageSection(FName(TEXT("Reload SMG"))),
  ClipBoneName(TEXT("smg_clip"))
{
  PrimaryActorTick.bCanEverTick = true;
}

void AWeapon::ThrowWeapon()
{
  FRotator MeshRotation{ 0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f };
  GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);

  const FVector MeshForward{ GetItemMesh()->GetForwardVector() };
  const FVector MeshRight{ GetItemMesh()->GetRightVector() };
  // Direction in which we throw the Weapon
  FVector ImpulseDirection = MeshRight.RotateAngleAxis(-20.f, MeshForward);

  float RandomRotation{ FMath::FRandRange(15.f, 45.f) };
  ImpulseDirection = ImpulseDirection.RotateAngleAxis(RandomRotation, FVector(0.f, 0.f, 1.f));
  ImpulseDirection *= 4000.f;
  GetItemMesh()->AddImpulse(ImpulseDirection);

  bFalling = true;
  GetWorldTimerManager().SetTimer(
    ThrowWeaponTimer,
    this,
    &AWeapon::StopFalling,
    ThrowWeaponTime
  );
}

void AWeapon::StopFalling()
{
  bFalling = false;
  SetItemState(EItemState::EIS_Pickup);
}

void AWeapon::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  if (GetItemState() == EItemState::EIS_Falling && bFalling)
  {
    FRotator MeshRotation{ 0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f };
    GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);
  }
}

void AWeapon::ConsumeAmmo()
{
  if (Ammo - 1 <= 0)
  {
    Ammo = 0;
  }
  else
  {
    --Ammo;
  }
}

void AWeapon::ReloadAmmo(int32 Amount)
{
  checkf(Ammo + Amount <= MagazineCapacity, TEXT("Attempted to reload with more than magazine capacity"));
  Ammo += Amount;
}
