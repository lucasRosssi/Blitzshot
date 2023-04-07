// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

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

public:
  FORCEINLINE UWidgetComponent* GetPickupWidget() const { return PickupWidget; }

};
