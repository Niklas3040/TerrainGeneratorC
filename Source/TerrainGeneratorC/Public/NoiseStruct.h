// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FastNoiseWrapper.h"
#include "GeometryScript/MeshQueryFunctions.h"
#include "NoiseStruct.generated.h"


/**
 * 
 */
USTRUCT(BlueprintType)
struct FNoiseStruct
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool enable = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float frequency = 0.00001f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int multiplier = 100;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EFastNoise_NoiseType type = EFastNoise_NoiseType::Cellular;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool ridgeNoise = false;
};

USTRUCT(BlueprintType)
struct FBiomStruct
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool enable = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FNoiseStruct> NoiseStructs;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString BiomName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Humidity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Temperature;
};

USTRUCT()
struct FNoiseWeightStruct
{
	GENERATED_BODY()
	float weight;
	float Noise;
};

USTRUCT(BlueprintType)
struct FFoliagePositions
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGeometryScriptTrianglePoint Point;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector TriangleFaceNormal;
};

