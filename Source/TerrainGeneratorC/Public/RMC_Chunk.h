// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RealtimeMeshComponent.h"
#include "RealtimeMeshSimple.h"
#include "NoiseStruct.h"
#include "RuntimeMeshModifierNormals.h"
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
	void InitChunk(AActor* ChunkSpawner);
	void DebugVertexNormal(FVector Normal, FVector Vertex) const;

	/*UFUNCTION(BlueprintCallable)
	FRealtimeMeshSimpleMeshData CalculateMeshData(int ChunkSize, int Resolution1D, int NoiseSeed, int BiomSeed, float BiomFrequency, TArray<FBiomStruct> BiomStructs);*/

	UFUNCTION(BlueprintCallable)
	void generateTerrain(int ChunkSize, int Resolution1D, int NoiseSeed, int BiomSeed, float BiomFrequency, TArray<FBiomStruct> BiomStructs);

	UFUNCTION(BlueprintCallable)
	void copyFromDynamicMesh(UDynamicMesh* DynamicMesh, bool bResetMesh);
	void ConstructNextQuadSection();

	float getLayeredNoise(int seed, TArray<FNoiseStruct> noiseStructs, float x, float y);
	float getBiomNoise(float x, float y);
	static TArray<int> GetDynamicMeshTriangles(UDynamicMesh* DynamicMesh);
	/*static void CalculateUV(int Resolution1D, const TArray<FVector>& Positions, TArray<FVector2d>& UV0);
	static void CalculateTriangles(const int Resolution1D, TArray<int32>& Triangles);
	static void CalculateTangentsAndNormals(const TArray<FVector>& Positions, const TArray<int32>& Triangles, const TArray<FVector2d>& UV0, TArray<FVector>&
	                                        Tangents, TArray<FVector>& Normals);
	void CalculateVertexPositions(const int ChunkSize, const int Resolution1D, const int NoiseSeed, const int BiomSeed, const float BiomFrequency, const
	                              TArray<FBiomStruct>& BiomStructs, TArray<FVector>& Positions);
	static TArray<FVector> CalculateNormals(TArray<FVector> Vertices, TArray<int32> Triangles);
	static FVector SurfaceNormalFromIndices(int indexA, int indexB, int indexC, TArray<FVector>& Vertices);*/
	FRealtimeMeshSimpleMeshData GetMeshDataForQuad();
	UFUNCTION()
	void ConstructQuadSection();
	UFUNCTION(BlueprintCallable)
	void DebugTriangleNormals(int TriangleNumber);

	UPROPERTY(BlueprintReadWrite)
	int Resolution_1D;
	UPROPERTY(BlueprintReadOnly)
	URealtimeMeshComponent* RealtimeMeshComponent;
	UPROPERTY(BlueprintReadOnly)
	URealtimeMeshSimple* RealtimeMeshSimple;
	UPROPERTY(BlueprintReadWrite)
	UMaterialInterface* Material;
	UPROPERTY()
	UFastNoiseWrapper* NoiseWrapper;
	
	
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* Plane;
	UPROPERTY()
	FRealtimeMeshSimpleMeshData MeshData;
	UPROPERTY()
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
	UPROPERTY()
	FRealtimeMeshSectionKey SectionKey;
	UPROPERTY()
	int X;
	UPROPERTY()
	int Y;
	UPROPERTY()
	FTimerHandle TimerHandle;
	UPROPERTY()
	URuntimeMeshModifierNormals* MeshModifierNormals;
};
