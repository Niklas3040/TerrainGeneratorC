// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RealtimeMeshComponent.h"
#include "RealtimeMeshSimple.h"
#include "NoiseStruct.h"
#include "RMC_Chunk.generated.h"

/**
 * 
 */
UCLASS()
class TERRAINGENERATORC_API ARMC_Chunk : public AActor
{
	GENERATED_BODY()
public:
	ARMC_Chunk();
	UFUNCTION(BlueprintCallable)
	FRealtimeMeshSimpleMeshData CalculateMeshData(int ChunkSize, int Resolution1D, int NoiseSeed, int BiomSeed, float BiomFrequency, TArray<FBiomStruct> BiomStructs);
	UFUNCTION(BlueprintCallable)
	void generateTerrain(int ChunkSize, int Resolution1D, int NoiseSeed, int BiomSeed, float BiomFrequency, TArray<FBiomStruct> BiomStructs);
	float getLayeredNoise(int seed, TArray<FNoiseStruct> noiseStructs, float x, float y, UFastNoiseWrapper* FastNoiseWrapper);
	float getBiomNoise(float x, float y, int NoiseSeed, int BiomSeed,float BiomFrequency, TArray<FBiomStruct> BiomStructs, UFastNoiseWrapper* NoiseWrapper);
	static TArray<FVector> CalculateNormals(TArray<int32> Triangles, TArray<FVector> Vertices);
	TArray<int> GetTriangles(UDynamicMesh* DynamicMesh);
	UFUNCTION(BlueprintCallable)
	void copyFromDynamicMesh(UDynamicMesh* DynamicMesh, bool bResetMesh);

	UPROPERTY(BlueprintReadWrite)
	int Resolution_1D;
	UPROPERTY(BlueprintReadOnly)
	URealtimeMeshComponent* RealtimeMeshComponent;
	UPROPERTY(BlueprintReadOnly)
	URealtimeMeshSimple* RealtimeMeshSimple;
	UPROPERTY()
	UMaterialInterface* Material;
};
