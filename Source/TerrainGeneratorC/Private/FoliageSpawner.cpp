// Fill out your copyright notice in the Description page of Project Settings.


#include "FoliageSpawner.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

//NOT USED, See Blueprint (Foliage/FoliageSpawner)
AFoliageSpawner::AFoliageSpawner()
{
	ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(TEXT("StaticMesh'/Game/environment/Nature2/EA04_Grass_Stick_01a.EA04_Grass_Stick_01a'"));
	if (Mesh.Succeeded())
	{
		StaticMesh = Mesh.Object;
	}
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
}

// Called when the game starts or when spawned
void AFoliageSpawner::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AFoliageSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AFoliageSpawner::GenerateFoliage(TArray<FFoliagePositions> FoliagePositions)
{
	if (HismComponent)
	{
		for (FFoliagePositions FoliagePosition : FoliagePositions)
		{
			FVector Position = FoliagePosition.Point.Position;
			FRotator Rotator = UKismetMathLibrary::MakeRotFromZ(FoliagePosition.TriangleFaceNormal);
			float RandRange = FMath::RandRange(0.5f,1.5f);
			HismComponent->AddInstance(FTransform{Rotator, Position, FVector{RandRange}});
			UKismetSystemLibrary::DrawDebugPoint(this, Position, 10.0f, FLinearColor::Blue, 5.0f);
			UE_LOG(LogTemp, Warning, TEXT("LOOP:GeneratedFoliage"));
		}
		UE_LOG(LogTemp, Warning, TEXT("GeneratedFoliage with lenght: %d"), FoliagePositions.Num());	
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No HISM_Component!"));	
	}
}

void AFoliageSpawner::InitFoliageSpawner()
{
	PrimaryActorTick.bCanEverTick = true;
	HismComponent = NewObject<UHierarchicalInstancedStaticMeshComponent>(this);
	HismComponent->SetupAttachment(GetRootComponent());
	HismComponent->RegisterComponent();
	
	if (StaticMesh)
	{
		HismComponent->SetStaticMesh(StaticMesh);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("StaticMesh not set!"));
	}
}

