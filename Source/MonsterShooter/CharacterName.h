#pragma once

UENUM(BlueprintType)
enum class ECharacterName : uint8
{
  ECN_Belica UMETA(DisplayName = "Belica"),
  ECN_TwinBlast UMETA(DisplayName = "TwinBlast"),
  ECN_Commando UMETA(DisplayName = "Commando"),
  ECN_Revenant UMETA(DisplayName = "Revenant"),

  ECN_MAX UMETA(DisplayName = "DefaultMax")
};