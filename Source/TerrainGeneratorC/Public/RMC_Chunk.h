// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RealtimeMeshComponent.h"
#include "RealtimeMeshSimple.h"
#include "NoiseStruct.h"
#include "RuntimeMeshModifierNormals.h"
#include "MulithreadingTestActor.h"
#include "RMC_Chunk.generated.h"

/**
 * 
 */
UCLASS()
class TERRAINGENERATORC_API ARMC_Chunk : public AActor
{
	GENERATED_BODY()
private:
	mutable FCriticalSection DataGuard;
	
public:
	ARMC_Chunk();
	
	void InitChunk(AActor* ChunkSpawner);
	
	void DebugVertexNormal(FVector Normal, FVector Vertex) const;
	
	UFUNCTION(BlueprintCallable)
	void generateTerrain(int ChunkSize, int Resolution1D, int NoiseSeed, int BiomSeed, float BiomFrequency, TArray<FBiomStruct> BiomStructs);

	UFUNCTION(BlueprintCallable)
	void copyFromDynamicMesh(UDynamicMesh* DynamicMesh, bool bResetMesh);
	void NavMeshFix();

	float getLayeredNoise(int seed, TArray<FNoiseStruct> noiseStructs, float x, float y, UFastNoiseWrapper* NoiseWrapper);
	float getBiomNoise(float x, float y, UFastNoiseWrapper* FastNoiseWrapper, TArray<FBiomStruct> ovBiomStructs, float ovNoiseSeed, float
	                   ovBiomSeed, float
	                   ovBiomFrequency, FLinearColor& BiomeColor);
	float getNewBiomNoise(float x, float y, UFastNoiseWrapper* FastNoiseWrapper, TArray<FBiomStruct> ovBiomStructs, float ovNoiseSeed, float
					   ovBiomSeed, float
					   ovBiomFrequency, FLinearColor& BiomeColor);
	static TArray<int> GetDynamicMeshTriangles(UDynamicMesh* DynamicMesh);
	FRealtimeMeshSimpleMeshData GetMeshDataForAllQuads(int ovChunkSize, int ovResolution1D, TArray<FBiomStruct> ovBiomStructs, float ovNoiseSeed, float
	                                                   ovBiomSeed, float ovBiomFrequency);
	static void SetQuadTriangles(const int QuadIndex, TArray<int32>& Triangles);
	void SetQuadVertices(const FVector& ActorLocation, const double X, const double Y, const double X1, const double Y1, TArray<FVector>&
	                     Vertices, const int QuadIndex, UFastNoiseWrapper* FastNoiseWrapper, TArray<FBiomStruct> ovBiomStructs, float
	                     ovNoiseSeed, float
	                     ovBiomSeed, float ovBiomFrequency, TArray<FLinearColor>& Colors);
	
	UPROPERTY(BlueprintReadWrite)
	int Resolution_1D;
	UPROPERTY(BlueprintReadOnly)
	URealtimeMeshComponent* RealtimeMeshComponent;
	UPROPERTY(BlueprintReadOnly)
	URealtimeMeshSimple* RealtimeMeshSimple;
	UPROPERTY(BlueprintReadWrite)
	UMaterialInterface* Material;
	
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* Plane;
	UPROPERTY()
	FRealtimeMeshSectionKey SectionKey;
	UPROPERTY()
	FTimerHandle TimerHandle;
	UPROPERTY()
	URuntimeMeshModifierNormals* MeshModifierNormals;
	UPROPERTY(BlueprintReadOnly)
	FRealtimeMeshSimpleMeshData MeshData;

	UPROPERTY(BlueprintReadOnly)
	int ChunkSize;
	UPROPERTY()
	int Resolution1D;
	UPROPERTY()
	int NoiseSeed;
	UPROPERTY()
	int BiomSeed;
	UPROPERTY()
	float BiomFrequency;
	UPROPERTY()
	TArray<FBiomStruct> BiomStructs;
};
