// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon.h"
#include "Kismet/GameplayStatics.h"
#include "BulletHitInterface.h"
#include "ShooterCharacter.h"
#include "Enemy.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Particles/ParticleSystemComponent.h"

AWeapon::AWeapon() : ThrowWeaponTime(0.7f),
                     bFalling(false),
                     WeaponType(EWeaponType::EWT_SubmachineGun),
                     ReloadMontageSection(FName(TEXT("Reload SMG"))),
                     ClipBoneName(TEXT("smg_clip")),
                     bAutomatic(true)
{
  PrimaryActorTick.bCanEverTick = true;
}

void AWeapon::ThrowWeapon()
{
  FRotator MeshRotation{0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f};
  GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);

  const FVector MeshForward{GetItemMesh()->GetForwardVector()};
  const FVector MeshRight{GetItemMesh()->GetRightVector()};
  // Direction in which we throw the Weapon
  FVector ImpulseDirection = MeshRight.RotateAngleAxis(-20.f, MeshForward);

  float RandomRotation{FMath::FRandRange(15.f, 45.f)};
  ImpulseDirection = ImpulseDirection.RotateAngleAxis(RandomRotation, FVector(0.f, 0.f, 1.f));
  ImpulseDirection *= 4000.f;
  GetItemMesh()->AddImpulse(ImpulseDirection);

  bFalling = true;
  GetWorldTimerManager().SetTimer(
      ThrowWeaponTimer,
      this,
      &AWeapon::StopFalling,
      ThrowWeaponTime);
}

void AWeapon::StopFalling()
{
  bFalling = false;
  SetItemState(EItemState::EIS_Pickup);
}

void AWeapon::OnConstruction(const FTransform &Transform)
{
  Super::OnConstruction(Transform);

  const FString WeaponTablePath{TEXT("/Script/Engine.DataTable'/Game/_Game/DataTable/DT_WeaponProperties.DT_WeaponProperties'")};
  UDataTable *WeaponTableObject = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *WeaponTablePath));

  if (WeaponTableObject)
  {
    FWeaponProperties *WeaponDataRow = nullptr;

    WeaponDataRow = WeaponTableObject->FindRow<FWeaponProperties>(FName(WeaponName), TEXT(""));

    if (WeaponDataRow)
    {
      AmmoType = WeaponDataRow->AmmoType;
      Ammo = WeaponDataRow->Ammo;
      MagazineCapacity = WeaponDataRow->MagazineCapacity;
      GetItemMesh()->SetSkeletalMesh(WeaponDataRow->ItemMesh);
      SetItemName(WeaponDataRow->WeaponName);
      SetIconItem(WeaponDataRow->InventoryIcon);

      SetMaterialInstance(WeaponDataRow->MaterialInstance);
      PreviousMaterialIndex = GetMaterialIndex();
      GetItemMesh()->SetMaterial(PreviousMaterialIndex, nullptr);
      SetMaterialIndex(WeaponDataRow->MaterialIndex);
      ClipBoneName = WeaponDataRow->ClipBoneName;
      ReloadMontageSection = WeaponDataRow->ReloadMontageSection;
      GetItemMesh()->SetAnimInstanceClass(WeaponDataRow->AnimBP);
      CrosshairMiddle = WeaponDataRow->CrosshairMiddle;
      CrosshairLeft = WeaponDataRow->CrosshairLeft;
      CrosshairRight = WeaponDataRow->CrosshairRight;
      CrosshairBottom = WeaponDataRow->CrosshairBottom;
      CrosshairTop = WeaponDataRow->CrosshairTop;
      FireRate = WeaponDataRow->FireRate;
      MuzzleFlash = WeaponDataRow->MuzzleFlash;
      FireSound = WeaponDataRow->FireSound;
      AimFireMontageSection = WeaponDataRow->AimFireMontageSection;
      HipFireMontageSection = WeaponDataRow->HipFireMontageSection;
      bAutomatic = WeaponDataRow->bAutomatic;
      Damage = WeaponDataRow->Damage;
      WeakspotDamage = WeaponDataRow->WeakspotDamage;
      Accuracy = WeaponDataRow->Accuracy;
      BeamParticles = WeaponDataRow->BeamParticles;
      Stability = WeaponDataRow->Stability;
      BalanceDamage = WeaponDataRow->BalanceDamage;
    }

    if (GetMaterialInstance())
    {
      SetDynamicMaterialInstance(UMaterialInstanceDynamic::Create(GetMaterialInstance(), this));
      GetItemMesh()->SetMaterial(GetMaterialIndex(), GetDynamicMaterialInstance());
    }

    Damage *= (((RarityProperties.NumberOfStars - 1) * 0.15f) + 1.f);
    WeakspotDamage *= (((RarityProperties.NumberOfStars - 1) * 0.15f) + 1.f);
  }
}

void AWeapon::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  if (GetItemState() == EItemState::EIS_Falling && bFalling)
  {
    FRotator MeshRotation{0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f};
    GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);
  }
}

void AWeapon::SendProjectile()
{
  const USkeletalMeshSocket *BarrelSocket = GetItemMesh()->GetSocketByName("BarrelSocket");

  if (BarrelSocket)
  {
    const FTransform SocketTransform = BarrelSocket->GetSocketTransform(GetItemMesh());

    if (MuzzleFlash)
    {
      UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
    }

    AShooterCharacter *ShooterCharacter = Cast<AShooterCharacter>(Shooter);

    if (ShooterCharacter)
    {
      FHitResult BeamHitResult;
      bool bBeamEnd = ShooterCharacter->GetBeamEndLocation(
          SocketTransform.GetLocation(),
          BeamHitResult);

      if (bBeamEnd)
      {
        // Spawn particles after updating correctly BeamEndPoint
        // Check if hit actor implement BulletHitInterface
        if (BeamHitResult.GetActor())
        {
          IBulletHitInterface *BulletHitInterface = Cast<IBulletHitInterface>(BeamHitResult.GetActor());

          if (BulletHitInterface)
          {
            BulletHitInterface->BulletHit_Implementation(BeamHitResult, Shooter, Shooter->GetController());
          }
          else if (ImpactParticles) // Default particles
          {
            UGameplayStatics::SpawnEmitterAtLocation(
                GetWorld(),
                ImpactParticles,
                BeamHitResult.Location,
                FRotator(0.f),
                true);
          }

          AEnemy *HitEnemy = Cast<AEnemy>(BeamHitResult.GetActor());
          if (HitEnemy && !HitEnemy->IsDead())
          {
            int32 DamageDealt{};
            bool bWeakspot = false;

            if (BeamHitResult.BoneName.ToString() == HitEnemy->GetWeakspotBone())
            // Weakspot shot
            {
              DamageDealt = WeakspotDamage;
              bWeakspot = true;
            }
            // Normal shot
            else
            {
              DamageDealt = Damage;
            }

            UGameplayStatics::ApplyDamage(
                HitEnemy,
                DamageDealt,
                Shooter->GetController(),
                this,
                UDamageType::StaticClass());

            HitEnemy->ShowHitNumber(DamageDealt, BeamHitResult.Location, bWeakspot);

            if (HitEnemy->GetHealth() - DamageDealt > 0)
            {
              HitEnemy->TakeBalanceDamage(BalanceDamage);
            }
          }
        }

        if (BeamParticles)
        {
          UParticleSystemComponent *Beam = UGameplayStatics::SpawnEmitterAtLocation(
              GetWorld(),
              BeamParticles,
              SocketTransform);

          if (Beam)
          {
            Beam->SetVectorParameter(FName("Target"), BeamHitResult.Location);
          }
        }
      }
    }
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

bool AWeapon::ClipIsFull()
{
  return Ammo >= MagazineCapacity;
}
