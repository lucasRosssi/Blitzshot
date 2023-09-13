// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "AmmoType.h"
#include "WeaponType.h"
#include "Engine/DataTable.h"
#include "Weapon.generated.h"

USTRUCT(BlueprintType)
struct FWeaponProperties : public FTableRowBase
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  EAmmoType AmmoType;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  int32 Ammo;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  int32 MagazineCapacity;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  USkeletalMesh *ItemMesh;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString WeaponName;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  UTexture2D *InventoryIcon;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  UMaterialInstance *MaterialInstance;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  int32 MaterialIndex;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FName ClipBoneName;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FName ReloadMontageSection;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TSubclassOf<UAnimInstance> AnimBP;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  UTexture2D *CrosshairMiddle;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  UTexture2D *CrosshairLeft;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  UTexture2D *CrosshairRight;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  UTexture2D *CrosshairBottom;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  UTexture2D *CrosshairTop;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  float FireRate;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  class UParticleSystem *MuzzleFlash;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  class USoundCue *FireSound;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FName AimFireMontageSection;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FName HipFireMontageSection;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  bool bAutomatic;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  float Damage;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  float WeakspotDamage;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  float Accuracy;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  UParticleSystem *BeamParticles;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  float Stability;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  float BalanceDamage;
};

/**
 *
 */
UCLASS()
class MONSTERSHOOTER_API AWeapon : public AItem
{
  GENERATED_BODY()
public:
  AWeapon();

  virtual void Tick(float DeltaTime) override;

  void SendProjectile();

protected:
  void StopFalling();

  virtual void OnConstruction(const FTransform &Transform) override;

protected:
  FTimerHandle ThrowWeaponTimer;
  float ThrowWeaponTime;
  bool bFalling;

  /** Type of the weapon */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
  EWeaponType WeaponType;

  /** Name of the weapon reload montage section */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
  FName ReloadMontageSection;

  /** True when moving the clip while reloading */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
  bool bMovingClip;

  /** Name for the clip bone */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
  FName ClipBoneName;

  /** Data table for the weapon properties */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Data Table", meta = (AllowPrivateAccess = "true"))
  UDataTable *WeaponDataTable;

  /** Maximum ammo the weapon can hold */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
  int32 MagazineCapacity;

  /** Ammo count for this weapon */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
  int32 Ammo;

  /** Ammo type that the weapon uses */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
  EAmmoType AmmoType;

  int32 PreviousMaterialIndex;

  /** Textures for the weapons crosshairs */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Data Table", meta = (AllowPrivateAccess = "true"))
  UTexture2D *CrosshairMiddle;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Data Table", meta = (AllowPrivateAccess = "true"))
  UTexture2D *CrosshairLeft;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Data Table", meta = (AllowPrivateAccess = "true"))
  UTexture2D *CrosshairRight;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Data Table", meta = (AllowPrivateAccess = "true"))
  UTexture2D *CrosshairBottom;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Data Table", meta = (AllowPrivateAccess = "true"))
  UTexture2D *CrosshairTop;

  /** Speed at which the weapon fires bullets */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Data Table", meta = (AllowPrivateAccess = "true"))
  float FireRate;

  /** Particle system spawned at the BarrelSocket */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Data Table", meta = (AllowPrivateAccess = "true"))
  class UParticleSystem *MuzzleFlash;

  /** Sound played when the weapon fires */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Data Table", meta = (AllowPrivateAccess = "true"))
  USoundCue *FireSound;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Data Table", meta = (AllowPrivateAccess = "true"))
  FName AimFireMontageSection;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Data Table", meta = (AllowPrivateAccess = "true"))
  FName HipFireMontageSection;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Table", meta = (AllowPrivateAccess = "true"))
  bool bAutomatic;

  /** Amount of damage caused by a bullet */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Table", meta = (AllowPrivateAccess = "true"))
  float Damage;

  /** Amount of damage when the bullet hits weakspots */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Table", meta = (AllowPrivateAccess = "true"))
  float WeakspotDamage;

  /** How precise is each shot related to the aim point. Higher values makes the shot closer to the center, up to exactly at the center at 100.f. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Table", meta = (AllowPrivateAccess = "true"))
  float Accuracy;

  // Projectile trail
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Table", meta = (AllowPrivateAccess = "true"))
  UParticleSystem *BeamParticles;

  /** How much aim stability the weapon provides. Higher values makes the aim move less, up to no movement at 100.f */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Table", meta = (AllowPrivateAccess = "true"))
  float Stability;

  /** How much balance bar damage each shot does */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Table", meta = (AllowPrivateAccess = "true"))
  float BalanceDamage;

  ACharacter *Shooter;

  // Particles spawned upon projectile impact
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
  UParticleSystem *ImpactParticles;

public:
  // Adds impulse to the thrown Weapon
  void ThrowWeapon();

  FORCEINLINE int32 GetMagazineCapacity() const { return MagazineCapacity; }
  FORCEINLINE int32 GetAmmo() const { return Ammo; }
  /** Called from Character class when firing weapon */
  void ConsumeAmmo();

  FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
  FORCEINLINE EAmmoType GetAmmoType() const { return AmmoType; }
  FORCEINLINE FName GetReloadMontageSection() const { return ReloadMontageSection; }
  FORCEINLINE FName GetClipBoneName() const { return ClipBoneName; }
  FORCEINLINE float GetFireRate() const { return FireRate; }
  FORCEINLINE UParticleSystem *GetMuzzleFlash() const { return MuzzleFlash; }
  FORCEINLINE USoundCue *GetFireSound() const { return FireSound; }
  FORCEINLINE FName GetAimFireMontageSection() const { return AimFireMontageSection; }
  FORCEINLINE FName GetHipFireMontageSection() const { return HipFireMontageSection; }
  FORCEINLINE bool GetAutomatic() const { return bAutomatic; }
  FORCEINLINE float GetDamage() const { return Damage; }
  FORCEINLINE float GetWeakspotDamage() const { return WeakspotDamage; }
  FORCEINLINE float GetAccuracy() const { return Accuracy; }
  FORCEINLINE UParticleSystem *GetBeamParticles() const { return BeamParticles; }
  FORCEINLINE float GetStability() const { return Stability; }
  FORCEINLINE float GetBalanceDamage() const { return BalanceDamage; }
  FORCEINLINE void SetMovingClip(bool Move) { bMovingClip = Move; }
  FORCEINLINE void SetShooter(ACharacter *OwnerShooter) { Shooter = OwnerShooter; }

  void ReloadAmmo(int32 Amount);

  bool ClipIsFull();
};
