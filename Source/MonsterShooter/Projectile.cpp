// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Enemy.h"
#include "Weapon.h"

// Sets default values
AProjectile::AProjectile()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root Component"));

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Projectile Mesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionProfileName(FName("NoCollision"));

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement Component"));
	ProjectileMovementComponent->InitialSpeed = 6500.f;
	ProjectileMovementComponent->MaxSpeed = 6500.f;

	CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision Component"));
	CollisionComponent->SetupAttachment(ProjectileMesh);
}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	CollisionComponent->OnComponentBeginOverlap.AddDynamic(
			this,
			&AProjectile::OnProjectileOverlap);

	SetLifeSpan(20.f);
}

void AProjectile::OnProjectileOverlap(
		UPrimitiveComponent *OverlappedComponent,
		AActor *OtherActor,
		UPrimitiveComponent *OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult &SweepResult)
{
	if (!OtherActor || Cast<AProjectile>(OtherActor))
	{
		return;
	}

	FAttachmentTransformRules AttachmentRules = {
			EAttachmentRule::KeepWorld,
			EAttachmentRule::KeepWorld,
			EAttachmentRule::KeepWorld,
			true};

	ProjectileMovementComponent->StopMovementImmediately();
	ProjectileMovementComponent->ProjectileGravityScale = 0;
	AttachToActor(OtherActor, AttachmentRules);

	if (HitParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				HitParticles,
				CollisionComponent->GetComponentLocation(),
				CollisionComponent->GetComponentRotation(),
				true);
	}

	AEnemy *HitEnemy = Cast<AEnemy>(OtherActor);
	if (HitEnemy && !HitEnemy->IsDead() && Weapon && Weapon->GetShooter())
	{
		int32 DamageDealt{};
		bool bWeakspot = false;

		if (SweepResult.BoneName.ToString() == HitEnemy->GetWeakspotBone())
		// Weakspot shot
		{
			DamageDealt = Weapon->GetWeakspotDamage();
			bWeakspot = true;
		}
		// Normal shot
		else
		{
			DamageDealt = Weapon->GetDamage();
		}

		UGameplayStatics::ApplyDamage(
				HitEnemy,
				DamageDealt,
				Weapon->GetShooter()->GetController(),
				this,
				UDamageType::StaticClass());

		HitEnemy->ShowHitNumber(DamageDealt, SweepResult.Location, bWeakspot);

		if (HitEnemy->GetHealth() - DamageDealt > 0)
		{
			HitEnemy->TakeBalanceDamage(Weapon->GetBalanceDamage());
		}
	}

	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
