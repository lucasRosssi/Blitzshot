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

UENUM(BlueprintType)
enum class EItemState : uint8
{
  EIS_Pickup UMETA(DisplayName = "Pickup"),
  EIS_EquipInterping UMETA(DisplayName = "EquipInterping"),
  EIS_PickedUp UMETA(DisplayName = "PickedUp"),
  EIS_Equipped UMETA(DisplayName = "Equipped"),
  EIS_Falling UMETA(DisplayName = "Falling"),
  
  EIS_MAX UMETA(DisplayName = "DefaultMAX")
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

  /** Sets properties of the Item's components based on State */
  void SetItemProperties(EItemState State);

  /** Called when ItemInterpTimer is finished */
  void FinishInterping();

  /** Handles item interpolation when in the EquipInterping state */
  void ItemInterp(float DeltaTime);

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

  /** State of the Item */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
  EItemState ItemState;

  /** The curve asset to use for the item's Z location when interping */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
  class UCurveFloat* ItemZCurve;

  /** Starting location when interping begins */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
  FVector ItemInterpStartLocation;

  /** Target interp locatio in front of the camera */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
  FVector CameraTargetLocation;

  /** True when interping */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
  bool bInterping;

  /** Plays when we start interping */
  FTimerHandle ItemInterpTimer;

  /** Duration of the curve and timer */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
  float ZCurveTime;

  /** Pointer to the character */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
  class AShooterCharacter* Character;

  /** X and Y for the Item while interping in the EquipInterping step */
  float ItemInterpX;
  float ItemInterpY;

  /** Curve used to scale the item when interping */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
  UCurveFloat* ItemScaleCurve;

  /** Sound played when item is picked up */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
  class USoundCue* PickupSound;

  /** Sound played when item is equipped */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
  USoundCue* EquipSound;

public:
  FORCEINLINE UWidgetComponent* GetPickupWidget() const { return PickupWidget; }
  FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
  FORCEINLINE UBoxComponent* GetCollisionBox() const { return CollisionBox; }
  FORCEINLINE EItemState GetItemState() const { return ItemState; }
  void SetItemState(EItemState State);
  FORCEINLINE USkeletalMeshComponent* GetItemMesh() const { return ItemMesh; }
  FORCEINLINE USoundCue* GetPickupSound() const { return PickupSound; }
  FORCEINLINE USoundCue* GetEquipSound() const { return EquipSound; }

  /** Called from the AShooterCharacter class */
  void StartItemCurve(AShooterCharacter* Char);

};
