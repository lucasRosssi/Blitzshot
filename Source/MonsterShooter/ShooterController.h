// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShooterController.generated.h"

/**
 * 
 */
UCLASS()
class MONSTERSHOOTER_API AShooterController : public APlayerController
{
	GENERATED_BODY()
public:
  AShooterController();

protected:
  virtual void BeginPlay() override;

private:
  /** Reference to the overall HUD overlay blueprint class */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets, meta = (AllowPrivateAccess = "true"))
  TSubclassOf<class UUserWidget> HUDOverlayClass;
	
  /** Variable to hold the HUD Overlay Widget after creating it */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Widgets, meta = (AllowPrivateAccess = "true"))
  UUserWidget* HUDOverlay;
};
