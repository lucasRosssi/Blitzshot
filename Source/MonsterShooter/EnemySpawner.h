// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpawner.generated.h"

UCLASS()
class MONSTERSHOOTER_API AEnemySpawner : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AEnemySpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void SpawnEnemies();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	/** Which enemy to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning", meta = (AllowPrivateAccess = "true"))
	class UClass *EnemyClass;

	/** How many will be spawned */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning", meta = (AllowPrivateAccess = "true", ClampMin = 1, UIMin = 1))
	int32 SpawnCount;

	/** How much time between spawning enemies */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning", meta = (AllowPrivateAccess = "true", ClampMin = 0))
	float SpawnInterval;
	FTimerHandle SpawnIntervalTimer;

	/** Sphere for the spawn area */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning", meta = (AllowPrivateAccess = "true"))
	class USphereComponent *SpawnAreaSphere;

	/** Array of spawned enemies */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Spawning", meta = (AllowPrivateAccess = "true"))
	TArray<class AEnemy *> EnemiesSpawned;

	/** If the enemies should spawn already agroed towards the player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning", meta = (AllowPrivateAccess = "true"))
	bool bAgressive;

	/** If the enemies should spawn at random locations in the SpawnAreaSphere radius */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning", meta = (AllowPrivateAccess = "true"))
	bool bInSpawnArea;
};
