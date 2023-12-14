// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DynamicMeshActor.h"
#include "GeometryScript/MeshPrimitiveFunctions.h"
#include "GeometryScript/MeshQueryFunctions.h"
#include "GeometryScript/ListUtilityFunctions.h"
#include "GeometryScript/MeshBasicEditFunctions.h"
#include "GeometryScript/MeshUVFunctions.h"
#include "FastNoiseWrapper.h"
#include "GeometryScript/MeshNormalsFunctions.h"
#include "PerfChunk.generated.h"


/**
 * 
 */
UCLASS()
class TERRAINGENERATORC_API APerfChunk : public ADynamicMeshActor
{
	GENERATED_BODY()
public:
	APerfChunk();
	UFUNCTION(BlueprintCallable)
	void GenerateTerrain();

	FGeometryScriptSimpleMeshBuffers GetMeshDataForAllQuads() const;

	static void SetQuadTriangles(const int QuadIndex, TArray<FIntVector>& Triangles);
	void SetQuadVertices(const FVector& ActorLocation, const double X, const double Y, const double X1, const double Y1, TArray<FVector>&
						 Vertices, const int QuadIndex) const;

	float GetVertexHeight(const FVector& ActorLocation, double X, double Y) const;



	UPROPERTY()
	UStaticMeshComponent* Plane;
	UPROPERTY(BlueprintReadOnly)
	UFastNoiseWrapper* FastNoiseWrapper;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int Resolution_1D = 4;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int ChunkSize = 1000;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Seed = 4.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float frequency = 0.0001f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float multiplicator = 1.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EFastNoise_NoiseType Type = EFastNoise_NoiseType::SimplexFractal;
};
