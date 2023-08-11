// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemySpawner.h"
#include "Enemy.h"
#include "Components/SphereComponent.h"
#include "EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"

// Sets default values
AEnemySpawner::AEnemySpawner() : EnemyClass(AEnemy::StaticClass()),
																 SpawnCount(1),
																 SpawnInterval(0.5f),
																 bAgressive(true),
																 bInSpawnArea(true),
																 bActive(false)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SpawnAreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("SpawnAreaSphere"));
	SpawnAreaSphere->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	SpawnEnemies();
}

void AEnemySpawner::SpawnEnemies()
{
	FVector Location;
	FRotator Rotation = GetActorRotation();
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	if (bInSpawnArea && SpawnAreaSphere)
	{
		FVector2D Point = FMath::RandPointInCircle(SpawnAreaSphere->GetScaledSphereRadius());
		Location = GetActorLocation() + FVector(Point.X, Point.Y, 0.f);
	}
	else
	{
		Location = GetActorLocation();
	}

	auto Actor = GetWorld()->SpawnActor(
			EnemyClass,
			&Location,
			&Rotation,
			SpawnParameters);

	auto Enemy = Cast<AEnemy>(Actor);

	if (Enemy)
	{
		EnemiesSpawned.Add(Enemy);

		auto EnemyController = Cast<AEnemyController>(Enemy->GetController());
		if (EnemyController && EnemyController->GetBlackboardComponent() && bAgressive)
		{
			auto Character = GetWorld()->GetFirstPlayerController()->GetPawn();

			EnemyController->GetBlackboardComponent()->SetValueAsObject(TEXT("Target"), Character);
		}
	}

	if (EnemiesSpawned.Num() < SpawnCount)
	{
		GetWorldTimerManager().SetTimer(SpawnIntervalTimer,
																		this,
																		&AEnemySpawner::SpawnEnemies,
																		SpawnInterval);
	}
}

// Called every frame
void AEnemySpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
