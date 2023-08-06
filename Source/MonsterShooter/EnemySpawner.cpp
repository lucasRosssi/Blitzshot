// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemySpawner.h"
#include "Enemy.h"
#include "Components/SphereComponent.h"
#include "EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"

// Sets default values
AEnemySpawner::AEnemySpawner() : EnemyClass(AEnemy::StaticClass()),
																 SpawnCount(1),
																 SpawnInterval(0.f)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();
}

void AEnemySpawner::SpawnEnemies()
{
	auto Actor = GetWorld()->SpawnActor(EnemyClass);
	auto Enemy = Cast<AEnemy>(Actor);

	if (Enemy)
	{
		auto EnemyController = Cast<AEnemyController>(Enemy->GetController());
		if (EnemyController && EnemyController->GetBlackboardComponent())
		{
			auto Character = GetWorld()->GetFirstPlayerController()->GetPawn();

			EnemyController->GetBlackboardComponent()->SetValueAsObject(TEXT("Target"), Character);
		}
	}
}

// Called every frame
void AEnemySpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
