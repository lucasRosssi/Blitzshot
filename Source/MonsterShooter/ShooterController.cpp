// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterController.h"
#include "Blueprint/UserWidget.h"

AShooterController::AShooterController()
{

}

void AShooterController::BeginPlay()
{
  Super::BeginPlay();

  // Check our HUDOverlayClass TSubclassOf variable
  if (HUDOverlayClass)
  {
    HUDOverlay = CreateWidget<UUserWidget>(this, HUDOverlayClass);
    if (HUDOverlay)
    {
      HUDOverlay->AddToViewport();
      HUDOverlay->SetVisibility(ESlateVisibility::Visible);
    }
  }
}

