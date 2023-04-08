// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
  EIR_Common UMETA(DisplayName = "Common"),
  EIR_Uncommon UMETA(DisplayName = "Uncommon"),
  EIR_Rare UMETA(DisplayName = "Rare"),
  EIR_Epic UMETA(DisplayName = "Epic"),
  EIR_Legendary UMETA(DisplayName = "Legendary"),
  
  EIR_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class MONSTERSHOOTER_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

  /** Called when overlapping AreaSphere */
  UFUNCTION()
  void OnSphereOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult
  );

  /** Called when ending overlap in AreaSphere */
  UFUNCTION()
  void OnSphereEndOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex
  );

  /** Sets the ActiveStars array of bools based on rarity */
  void SetActiveStars();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
  /** Skeletal Mesh for the item */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
  USkeletalMeshComponent* ItemMesh;

  /** Line trace collides with box to show HUD widgets */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
  class UBoxComponent* CollisionBox;

  /** Popup widget for when the player look at the item */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
  class UWidgetComponent* PickupWidget;

  /** Enables line trace when near the item */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
  class USphereComponent* AreaSphere;

  /** The name that appears in the pickup widget */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
  FString ItemName;

  /** The count (ammo...) that shows in the pickup widget */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
  int32 ItemCount;

  /** Item rarity - determines number of stars and color in pickup widget */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
  EItemRarity ItemRarity;

  /** Active stars showing in the pickup widget, based on rarity */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
  TArray<bool> ActiveStars;

public:
  FORCEINLINE UWidgetComponent* GetPickupWidget() const { return PickupWidget; }

};
