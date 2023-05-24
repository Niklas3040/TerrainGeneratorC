// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NoiseStruct.h"
#include "FoliageSpawner.h"
#include "Chunk.h"
#include "ChunkSpawner.generated.h"

UCLASS()
class TERRAINGENERATORC_API AChunkSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	AChunkSpawner();

	
	UPROPERTY(EditAnywhere)
	int ChunkSize = 2000;
	UPROPERTY(EditAnywhere)
	int Seed = 150518;
	UPROPERTY(EditAnywhere)
	int Row = 32;
	UPROPERTY(EditAnywhere)
	int Resolution = 16;
	UPROPERTY(EditAnywhere)
	float BiomFrequency = 0.000005f;
	UPROPERTY(EditAnywhere)
	TArray<FBiomStruct> BiomStructs;
	UPROPERTY(EditAnywhere)
	UMaterialInterface* ChunkMaterial;

	UPROPERTY(EditAnywhere)
	int ChunkRenderDistance = 20000;
	UPROPERTY()
	APawn* PlayerPawn = nullptr;
	UPROPERTY()
	AFoliageSpawner* FoliageSpawner = nullptr;
	UPROPERTY(BlueprintReadOnly)
	TArray<FFoliagePositions> AllFoliagePositions;
	
	
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	AChunk* SpawnChunk(FTransform Transform, int ChunkIndex);
	void SpawnChunksByRow();
	UFUNCTION(BlueprintCallable, Category="ProceduralStuff")
	void DestroyChunks();
	void SpawnNextChunk();
	void CalculateChunkPositions();
	FVector RoundDownToChunkSize(FVector Vector);
	void RemoveChunkFromPositionList(FVector Vector);
	UFUNCTION(BlueprintCallable, Category="ProceduralStuff")
	void RemoveFoliagePositionByIndex(int Index);
};
