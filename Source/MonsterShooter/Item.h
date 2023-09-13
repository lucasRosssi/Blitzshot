// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
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

UENUM(BlueprintType)
enum class EItemType : uint8
{
  EIT_Ammo UMETA(DisplayName = "Ammo"),
  EIT_Weapon UMETA(DisplayName = "Weapon"),

  EIT_MAX UMETA(DisplayName = "DefaultMAX")
};

USTRUCT(BlueprintType)
struct FItemRarityTable : public FTableRowBase
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FLinearColor GlowColor;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FLinearColor LightColor;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FLinearColor DarkColor;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  int32 NumberOfStars;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  UTexture2D *IconBackground;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  int32 CustomDepthStencil;
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
      UPrimitiveComponent *OverlappedComponent,
      AActor *OtherActor,
      UPrimitiveComponent *OtherComp,
      int32 OtherBodyIndex,
      bool bFromSweep,
      const FHitResult &SweepResult);

  /** Called when ending overlap in AreaSphere */
  UFUNCTION()
  void OnSphereEndOverlap(
      UPrimitiveComponent *OverlappedComponent,
      AActor *OtherActor,
      UPrimitiveComponent *OtherComp,
      int32 OtherBodyIndex);

  /** Sets the ActiveStars array of bools based on rarity */
  void SetActiveStars();

  /** Sets properties of the Item's components based on State */
  virtual void SetItemProperties(EItemState State);

  /** Called when ItemInterpTimer is finished */
  void FinishInterping();

  /** Handles item interpolation when in the EquipInterping state */
  void ItemInterp(float DeltaTime);

  /** Get interp location based on the item type */
  FVector GetInterpLocation();

  void PlayPickupSound();

  virtual void InitializeCustomDepth();

  virtual void OnConstruction(const FTransform &Transform) override;

  // void ResetPulseTimer();

  // void UpdatePulse();

public:
  // Called every frame
  virtual void Tick(float DeltaTime) override;

  // Called in ShooterCharacter::GetPickupItem()
  void PlayEquipSound();

private:
  /** Skeletal Mesh for the item */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
  USkeletalMeshComponent *ItemMesh;

  /** Line trace collides with box to show HUD widgets */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
  class UBoxComponent *CollisionBox;

  /** Popup widget for when the player look at the item */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
  class UWidgetComponent *PickupWidget;

  /** Enables line trace when near the item */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
  class USphereComponent *AreaSphere;

  /** The name that appears in the pickup widget */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
  FString ItemName;

  /** The count (ammo...) that shows in the pickup widget */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
  int32 ItemCount;

  /** Item rarity - determines number of stars and color in pickup widget */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rarity", meta = (AllowPrivateAccess = "true"))
  EItemRarity ItemRarity;

  /** Active stars showing in the pickup widget, based on rarity */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
  TArray<bool> ActiveStars;

  /** State of the Item */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
  EItemState ItemState;

  /** The curve asset to use for the item's Z location when interping */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
  class UCurveFloat *ItemZCurve;

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
  class AShooterCharacter *Character;

  /** X and Y for the Item while interping in the EquipInterping step */
  float ItemInterpX;
  float ItemInterpY;

  /** Curve used to scale the item when interping */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
  UCurveFloat *ItemScaleCurve;

  /** Sound played when item is picked up */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
  class USoundCue *PickupSound;

  /** Sound played when item is equipped */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
  USoundCue *EquipSound;

  /** Type of the item */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
  EItemType ItemType;

  /** Index of the interp location this item is interping to */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
  int32 InterpLocIndex;

  /** Index for the material to change at runtime */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
  int32 MaterialIndex;

  /** Dynamic instance that we can change at runtime */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
  UMaterialInstanceDynamic *DynamicMaterialInstance;

  /** Material instance used with Dynamic Material Instance */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
  UMaterialInstance *MaterialInstance;

  // /** Curve to drive the dynamic material parameters */
  // UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
  // class UCurveVector* PulseCurve;

  // FTimerHandle PulseTimer;

  // /** Time for the pulse timer */
  // UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
  // float PulseCurveTime;

  // UPROPERTY(VisibleAnywhere, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
  // float GlowAmount;

  // UPROPERTY(VisibleAnywhere, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
  // float FresnelExponent;

  // UPROPERTY(VisibleAnywhere, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
  // float FresnelReflectFraction;

  /** Icon for this item in the inventory */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
  UTexture2D *IconItem;

  /** Slot in the inventory array */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
  int32 SlotIndex;

  /** Item rarity data table */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
  class UDataTable *ItemRarityDataTable;

protected:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rarity", meta = (AllowPrivateAccess = "true"))
  FItemRarityTable RarityProperties;

public:
  FORCEINLINE UWidgetComponent *GetPickupWidget() const { return PickupWidget; }
  FORCEINLINE USphereComponent *GetAreaSphere() const { return AreaSphere; }
  FORCEINLINE UBoxComponent *GetCollisionBox() const { return CollisionBox; }
  FORCEINLINE EItemState GetItemState() const { return ItemState; }
  void SetItemState(EItemState State);
  FORCEINLINE USkeletalMeshComponent *GetItemMesh() const { return ItemMesh; }
  FORCEINLINE USoundCue *GetPickupSound() const { return PickupSound; }
  FORCEINLINE USoundCue *GetEquipSound() const { return EquipSound; }
  FORCEINLINE int32 GetItemCount() const { return ItemCount; }
  FORCEINLINE int32 GetSlotIndex() const { return SlotIndex; }
  FORCEINLINE void SetSlotIndex(int32 Index) { SlotIndex = Index; }
  FORCEINLINE void SetItemName(FString Name) { ItemName = Name; }
  FORCEINLINE FString GetItemName() const { return ItemName; }
  FORCEINLINE void SetIconItem(UTexture2D *Icon) { IconItem = Icon; }
  FORCEINLINE void SetMaterialInstance(UMaterialInstance *Instance) { MaterialInstance = Instance; }
  FORCEINLINE UMaterialInstance *GetMaterialInstance() const { return MaterialInstance; }
  FORCEINLINE void SetDynamicMaterialInstance(UMaterialInstanceDynamic *Instance) { DynamicMaterialInstance = Instance; }
  FORCEINLINE UMaterialInstanceDynamic *GetDynamicMaterialInstance() const { return DynamicMaterialInstance; }
  FORCEINLINE int32 GetMaterialIndex() const { return MaterialIndex; }
  FORCEINLINE void SetMaterialIndex(int32 Index) { MaterialIndex = Index; }

  /** Called from the AShooterCharacter class */
  void StartItemCurve(AShooterCharacter *Char);

  virtual void EnableCustomDepth();
  virtual void DisableCustomDepth();
  void EnableGlowMaterial();
  void DisableGlowMaterial();
};
