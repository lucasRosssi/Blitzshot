// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "AmmoType.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
  EWT_SubmachineGun UMETA(DisplayName = "SubmachineGun"),
  EWT_AssaultRifle UMETA(DisplayName = "AssaultRifle"),

  EWT_MAX UMETA(DisplayName = "DefaultMax")
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

private:
  FTimerHandle ThrowWeaponTimer;
  float ThrowWeaponTime;
  bool bFalling;

  /** Maximum ammo the weapon can hold */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
  int32 MagazineCapacity;

  /** Ammo count for this weapon */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
  int32 Ammo;

  /** Type of the weapon */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
  EWeaponType WeaponType;

  /** Ammo type that the weapon uses */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
  EAmmoType AmmoType;

  /** Name of the weapon reload montage section */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
  FName ReloadMontageSection;

  /** True when moving the clip while reloading */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
  bool bMovingClip;

  /** Name for the clip bone */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
  FName ClipBoneName;

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

};
