// Fill out your copyright notice in the Description page of Project Settings.

#include "BowAndArrow.h"

ABowAndArrow::ABowAndArrow()
{
  PrimaryActorTick.bCanEverTick = true;
}

void ABowAndArrow::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);
}