// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "BowAndArrow.generated.h"

/**
 *
 */
UCLASS()
class MONSTERSHOOTER_API ABowAndArrow : public AWeapon
{
	GENERATED_BODY()

public:
	ABowAndArrow();

	virtual void Tick(float DeltaTime) override;
};
