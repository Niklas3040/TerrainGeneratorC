// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RealtimeMeshSimple.h"
#include "RuntimeMeshModifierNormals.generated.h"

/**
 * 
 */
UCLASS()
class TERRAINGENERATORC_API URuntimeMeshModifierNormals : public UObject
{
	GENERATED_BODY()

public:
	void CalculateNormalsTangents(FRealtimeMeshSimpleMeshData& MeshData) const;
};
