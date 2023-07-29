// Fill out your copyright notice in the Description page of Project Settings.

#include "HealthComponent.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent() : MaxHealth(100.f),
																			 Health(MaxHealth),
																			 HealthRegen(0.f),
																			 HealthRegenCooldown(5.f),
																			 bCanRegenerateHealth(false),
																			 bHealthRegenActive(false),
																			 bDead(false)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}

void UHealthComponent::HealthRegenReset()
{
	bCanRegenerateHealth = true;
}

void UHealthComponent::RegenerateHealth(float DeltaTime)
{
	if (!bHealthRegenActive || !bCanRegenerateHealth || Health == MaxHealth || bDead)
		return;

	if (Health + (HealthRegen * DeltaTime) > MaxHealth)
	{
		Health = MaxHealth;
	}
	else
	{
		Health += HealthRegen * DeltaTime;
	}
}

// Called every frame
void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	RegenerateHealth(DeltaTime);
}

void UHealthComponent::TakeDamage(float Damage)
{
	bCanRegenerateHealth = false;

	if (Health - Damage <= 0.f)
	{
		Health = 0.f;
		bDead = true;

		GetWorld()->GetTimerManager().ClearTimer(HealthRegenStartTimer);
	}
	else
	{
		Health -= Damage;

		GetWorld()->GetTimerManager().SetTimer(
				HealthRegenStartTimer,
				this,
				&UHealthComponent::HealthRegenReset,
				HealthRegenCooldown);
	}
}
