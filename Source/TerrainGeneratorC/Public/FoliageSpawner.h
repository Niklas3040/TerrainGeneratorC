// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GeometryScript/MeshSpatialFunctions.h"
#include "NoiseStruct.h"
#include "FoliageSpawner.generated.h"


UCLASS()
class TERRAINGENERATORC_API AFoliageSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFoliageSpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void GenerateFoliage(TArray<FFoliagePositions> FoliagePositions);
	void InitFoliageSpawner();
	
	UPROPERTY(EditAnywhere)
	UHierarchicalInstancedStaticMeshComponent* HismComponent = nullptr;

	UPROPERTY(EditAnywhere)
	UStaticMesh* StaticMesh;

};
