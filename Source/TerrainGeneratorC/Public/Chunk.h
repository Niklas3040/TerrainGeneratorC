// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DynamicMeshActor.h"
#include "GeometryScript/MeshPrimitiveFunctions.h"
#include "GeometryScript/MeshQueryFunctions.h"
#include "GeometryScript/ListUtilityFunctions.h"
#include "GeometryScript/MeshBasicEditFunctions.h"
#include "GeometryScript/MeshNormalsFunctions.h"
#include "Components/StaticMeshComponent.h"
#include "FastNoiseWrapper.h"
#include "DrawDebugHelpers.h"
#include "NoiseStruct.h"
#include "GeometryScript/CollisionFunctions.h"
#include "Chunk.generated.h"

/**
 * 
 */


UCLASS()
class TERRAINGENERATORC_API AChunk : public ADynamicMeshActor
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* Plane;
	UPROPERTY()
	bool bIsFoliageGenerated = false;
	
	AChunk();
	virtual void Tick(float DeltaTime) override;
	UFUNCTION(BlueprintCallable, Category="ProceduralStuff")
	void createGrid(FGeometryScriptPrimitiveOptions PrimitiveOptions, int chunkSize, int resolution);
	UFUNCTION(BlueprintCallable, Category="ProceduralStuff")
	void applyNoise(int seed, TArray<FBiomStruct> BiomStructs, float BiomFrequency, int ChunkIndex);
	float getLayeredNoise(int seed, TArray<FNoiseStruct> noiseStructs, float x, float y, UFastNoiseWrapper* FastNoiseWrapper);
	float getBiomNoise(float x, float y, int NoiseSeed, int BiomSeed, float BiomFrequency, TArray<FBiomStruct> BiomStructs, UFastNoiseWrapper* NoiseWrapper);
	void SetMaterial(UMaterialInterface* Material);
	void InitChunk(AActor* ChunkSpawner);
	void CreateCollision();
	TArray<FFoliagePositions> GetFoliagePositions(int MinIterations, int MaxIterations);
	
	
};

