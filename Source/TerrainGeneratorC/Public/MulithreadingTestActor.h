// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NoiseStruct.h"
#include "FastNoiseWrapper.h"
#include "RealtimeMeshSimple.h"
#include "RuntimeMeshModifierNormals.h"
#include "MulithreadingTestActor.generated.h"

UCLASS()
class TERRAINGENERATORC_API AMulithreadingTestActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMulithreadingTestActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void AsyncGetRandomNoise();
	void LogNoise(int Noise);
	/*static float GetLayeredNoise(int seed, TArray<FNoiseStruct> noiseStructs, float x, float y, UFastNoiseWrapper* NoiseWrapper);
	UFUNCTION(BlueprintCallable)
	void AsyncGetBiomeNoise(float x, float y, TArray<FBiomStruct> BiomStructs, float NoiseSeed, float BiomSeed, float BiomFrequency);*/
	void SetQuadTriangles(const int QuadIndex, TArray<int32>& Triangles);
	void SetQuadVertices(const FVector& ActorLocation, const double X, const double Y, const double X1, const double Y1, TArray<FVector>& Vertices, const int QuadIndex, UFastNoiseWrapper* FastNoiseWrapper, TArray<FBiomStruct> ovBiomStructs, float ovNoiseSeed, float ovBiomSeed,
											float ovBiomFrequency);
	FRealtimeMeshSimpleMeshData GetMeshDataForAllQuads(int ovChunkSize, int ovResolution1D, TArray<FBiomStruct> ovBiomStructs, float ovNoiseSeed, float ovBiomSeed, float
	                                                   ovBiomFrequency, FVector Location);
	static float getLayeredNoise(int seed, TArray<FNoiseStruct> noiseStructs, float x, float y, UFastNoiseWrapper* NoiseWrapper);
	float getBiomNoise(float x, float y, UFastNoiseWrapper* FastNoiseWrapper, TArray<FBiomStruct> ovBiomStructs, float ovNoiseSeed, float ovBiomSeed,
											float ovBiomFrequency);
};
