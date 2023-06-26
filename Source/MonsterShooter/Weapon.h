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

protected:
  void StopFalling();

  virtual void OnConstruction(const FTransform &Transform) override;

private:
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

  void ReloadAmmo(int32 Amount);

  FORCEINLINE void SetMovingClip(bool Move) { bMovingClip = Move; }

  bool ClipIsFull();
};
