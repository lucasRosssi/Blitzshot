// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "ShooterCharacter.h"
#include "Camera/CameraComponent.h"

// Sets default values
AItem::AItem():
  ItemName(FString("Default")),
  ItemCount(0),
  ItemRarity(EItemRarity::EIR_Common),
  ItemState(EItemState::EIS_Pickup),
  // Interp variables
  ItemInterpStartLocation(FVector(0.f)),
  CameraTargetLocation(FVector(0.f)),
  bInterping(false),
  ZCurveTime(0.7f),
  ItemInterpX(0.f),
  ItemInterpY(0.f)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

  ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemMesh"));
  SetRootComponent(ItemMesh);

  CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
  CollisionBox->SetupAttachment(ItemMesh);
  CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
  CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

  PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
  PickupWidget->SetupAttachment(GetRootComponent());

  AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
  AreaSphere->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();
	// Hide Pickup Widget
  if (PickupWidget)
  {
    PickupWidget->SetVisibility(false);
  }

  SetActiveStars();

  // Setup overlap for AreaSphere
  AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnSphereOverlap);
  AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AItem::OnSphereEndOverlap);

  // Set Item properties based on ItemState
  SetItemProperties(ItemState);
}

void AItem::OnSphereOverlap(
  UPrimitiveComponent* OverlappedComponent,
  AActor* OtherActor,
  UPrimitiveComponent* OtherComp,
  int32 OtherBodyIndex,
  bool bFromSweep,
  const FHitResult& SweepResult
) 
{
  if (OtherActor)
  {
    AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);

    if (ShooterCharacter)
    {
      ShooterCharacter->IncrementOverlappedItemCount(1);
    }
  }
}

void AItem::OnSphereEndOverlap(
  UPrimitiveComponent* OverlappedComponent,
  AActor* OtherActor,
  UPrimitiveComponent* OtherComp,
  int32 OtherBodyIndex
) 
{
  if (OtherActor)
  {
    AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);

    if (ShooterCharacter)
    {
      ShooterCharacter->IncrementOverlappedItemCount(-1);
    }
  }
}

void AItem::SetActiveStars()
{
  // The 0 element isn't used
  for (int32 i = 0; i <= 5; i++)
  {
    ActiveStars.Add(false);
  }

  switch (ItemRarity)
  {
    case EItemRarity::EIR_Common:
      ActiveStars[1] = true;
      break;
    case EItemRarity::EIR_Uncommon:
      ActiveStars[1] = true;
      ActiveStars[2] = true;
      break;
    case EItemRarity::EIR_Rare:
      ActiveStars[1] = true;
      ActiveStars[2] = true;
      ActiveStars[3] = true;
      break;
    case EItemRarity::EIR_Epic:
      ActiveStars[1] = true;
      ActiveStars[2] = true;
      ActiveStars[3] = true;
      ActiveStars[4] = true;
      break;
    case EItemRarity::EIR_Legendary:
      ActiveStars[1] = true;
      ActiveStars[2] = true;
      ActiveStars[3] = true;
      ActiveStars[4] = true;
      ActiveStars[5] = true;
      break;
  }
}

void AItem::SetItemProperties(EItemState State)
{
  switch (State)
  {
    case EItemState::EIS_Pickup:
      // Set mesh properties
      ItemMesh->SetSimulatePhysics(false);
      ItemMesh->SetEnableGravity(false);
      ItemMesh->SetVisibility(true);
      ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
      ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
      // Set area sphere properties
      AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
      AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
      // Set collision box properties
      CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
      CollisionBox->SetCollisionResponseToChannel(
        ECollisionChannel::ECC_Visibility,
        ECollisionResponse::ECR_Block
      );
      CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
      break;
    case EItemState::EIS_Equipped:
      PickupWidget->SetVisibility(false);
      // Set mesh properties
      ItemMesh->SetSimulatePhysics(false);
      ItemMesh->SetEnableGravity(false);
      ItemMesh->SetVisibility(true);
      ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
      ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
      // Set area sphere properties
      AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
      AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
      // Set collision box properties
      CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
      CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
      break;
    case EItemState::EIS_Falling:
      // Set mesh properties
      ItemMesh->SetSimulatePhysics(true);
      ItemMesh->SetEnableGravity(true);
      ItemMesh->SetVisibility(true);
      ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
      ItemMesh->SetCollisionResponseToChannel(
        ECollisionChannel::ECC_WorldStatic,
        ECollisionResponse::ECR_Block
      );
      ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
      // Set area sphere properties
      AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
      AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
      // Set collision box properties
      CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
      CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
      break;
    case EItemState::EIS_EquipInterping:
      PickupWidget->SetVisibility(false);
      // Set mesh properties
      ItemMesh->SetSimulatePhysics(false);
      ItemMesh->SetEnableGravity(false);
      ItemMesh->SetVisibility(true);
      ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
      ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
      // Set area sphere properties
      AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
      AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
      // Set collision box properties
      CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
      CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
      break;
  }
}

void AItem::FinishInterping()
{
  bInterping = false;
  if (Character)
  {
    Character->GetPickupItem(this);
  }
}

void AItem::ItemInterp(float DeltaTime)
{
  if (!bInterping || !Character || !ItemZCurve) return;

  // Elapsed time since we started ItemInterpTimer
  const float ElapsedTime = GetWorldTimerManager().GetTimerElapsed(ItemInterpTimer);
  // Get curve value corresponding to elapsed time
  const float CurveValue = ItemZCurve->GetFloatValue(ElapsedTime);
  // Get the item's location when the curve started
  FVector ItemLocation = ItemInterpStartLocation;
  // Get location in front of the camera
  const FVector CameraInterpLocation{ Character->GetCameraInterpLocation() };
  // Vector from item to camera interp location, X and Y are zeroed out
  const FVector ItemToCamera{ FVector(0.f, 0.f, (CameraInterpLocation - ItemLocation).Z) };
  // Scale factor to multiply with CurveValue
  const float DeltaZ = ItemToCamera.Size();

  const FVector CurrentLocation{ GetActorLocation() };
  // Interpolated X value
  const float InterpXValue = FMath::FInterpTo(
    CurrentLocation.X,
    CameraInterpLocation.X,
    DeltaTime,
    30.0f
  );
  // Interpolated Y value
  const float InterpYValue = FMath::FInterpTo(
    CurrentLocation.Y,
    CameraInterpLocation.Y,
    DeltaTime,
    30.0f
  );

  // Set X and Y of ItemLocation to Interped values
  ItemLocation.X = InterpXValue;
  ItemLocation.Y = InterpYValue;

  // Adding curve value to the Z component of the initial location (scaled by DeltaZ)
  ItemLocation.Z += CurveValue * DeltaZ;
  SetActorLocation(ItemLocation, true, nullptr, ETeleportType::TeleportPhysics);

  // Camera rotation this frame
  const FRotator CameraRotation{ Character->GetFollowCamera()->GetComponentRotation() };
  // Camera rotation plus initial yaw offset
  FRotator ItemRotation{ 0.f, CameraRotation.Yaw + 180.f, 0.f };
  SetActorRotation(ItemRotation, ETeleportType::TeleportPhysics);
}

// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

  ItemInterp(DeltaTime);
}

void AItem::SetItemState(EItemState State)
{
  ItemState = State;
  SetItemProperties(State);
}

void AItem::StartItemCurve(AShooterCharacter* Char)
{
  // Store a handle to the character
  Character = Char;
  // Store initial location of the item
  ItemInterpStartLocation = GetActorLocation();
  bInterping = true;
  SetItemState(EItemState::EIS_EquipInterping);

  GetWorldTimerManager().SetTimer(
    ItemInterpTimer,
    this,
    &AItem::FinishInterping,
    ZCurveTime
  );
}


