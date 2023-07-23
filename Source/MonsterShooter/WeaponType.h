#pragma once

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
  EWT_SubmachineGun UMETA(DisplayName = "SubmachineGun"),
  EWT_AssaultRifle UMETA(DisplayName = "AssaultRifle"),
  EWT_Pistol UMETA(DisplayName = "Pistol"),
  EWT_Uzi UMETA(DisplayName = "Uzi"),
  EWT_AK47 UMETA(DisplayName = "AK47"),

  EWT_MAX UMETA(DisplayName = "DefaultMax")
};