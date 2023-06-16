// Fill out your copyright notice in the Description page of Project Settings.


#include "RMC_Chunk.h"

#include "FastNoiseWrapper.h"
#include "KismetProceduralMeshLibrary.h"
#include "UDynamicMesh.h"
#include "GeometryScript/ListUtilityFunctions.h"
#include "Kismet/KismetMathLibrary.h"


ARMC_Chunk::ARMC_Chunk()
{
	RealtimeMeshComponent = CreateDefaultSubobject<URealtimeMeshComponent>(TEXT("RMC_Component"));
	RealtimeMeshComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
}

FRealtimeMeshSimpleMeshData ARMC_Chunk::CalculateMeshData(int ChunkSize, int Resolution1D, int NoiseSeed, int BiomSeed, float BiomFrequency, TArray<FBiomStruct> BiomStructs)
{
	UFastNoiseWrapper* NoiseWrapper = NewObject<UFastNoiseWrapper>();
	
	TArray<int32> Triangles;
	TArray<FVector> Positions;
	TArray<FVector> Normals;
	TArray<FVector> Tangents;
	TArray<FVector> Binormals;
	TArray<FLinearColor> Colours;
	TArray<FVector2d> UV0;

	Positions.SetNum((Resolution1D+1)*(Resolution1D+1));

	const int QuadSize = ChunkSize/Resolution1D;
	int Index = 0;
	for (float y = 0; y <= Resolution1D; y++)
	{
		for (float x = 0; x <= Resolution1D; x++) {
			float X = x * QuadSize;
			float Y = y * QuadSize;
			Positions[Index] = FVector{X, Y,getBiomNoise(X,Y,NoiseSeed, BiomSeed, BiomFrequency,BiomStructs, NoiseWrapper)};
			Index++;
		}
	}
	
	Triangles.SetNum(Resolution1D * Resolution1D * 6);

	int vert = 0;
	int tris = 0;
	
	for (int y = 0; y < Resolution1D; y++)
	{
		for (int x = 0; x < Resolution1D; x++)
		{
			Triangles[tris] = vert;
			Triangles[tris + 1] = vert + Resolution1D + 1;
			Triangles[tris + 2] = vert + 1;
			Triangles[tris + 3] = vert + 1;
			Triangles[tris + 4] = vert + Resolution1D + 1;
			Triangles[tris + 5] = vert + Resolution1D + 2;

			vert++;
			tris += 6;
		}
		vert++;
	}

	/*for (int x = 0; x < Positions.Num(); x++)
	{
		Normals.Add(FVector{0.0f, 0.0f, 1.0f});
	}*/
	Normals = CalculateNormals(Triangles, Positions);
	
	UV0.SetNum(Positions.Num());
	int i = 0;
	for (int x = 0; x <= Resolution1D; x++)
	{
		for (int y = 0; y <= Resolution1D; y++)
		{
			double res = Resolution1D;
			UV0[i] = FVector2d{x / res, y / res};
			i++;
		}
	}
	
	FRealtimeMeshSimpleMeshData MeshData =
	{
		Triangles,
		Positions,
		Normals,
		Tangents,
		Binormals,
		Colours,
		UV0
	};
	return MeshData;
}

float ARMC_Chunk::getLayeredNoise(int seed, TArray<FNoiseStruct> noiseStructs, float x, float y, UFastNoiseWrapper* FastNoiseWrapper)
{
	float noise = 0;
	for (FNoiseStruct Noise : noiseStructs)
	{
		if(Noise.enable)
		{
			FastNoiseWrapper->SetupFastNoise(Noise.type, seed, Noise.frequency, EFastNoise_Interp::Quintic, EFastNoise_FractalType::FBM, 3, 2.0f);
			if(Noise.ridgeNoise)
			{
				noise += FMath::Abs((FastNoiseWrapper->GetNoise2D(x, y)*Noise.multiplier -0.5f) * 2.0f);
			}
			else
				noise += FastNoiseWrapper->GetNoise2D(x, y) * Noise.multiplier;
		}
	}
	return noise;
}

float ARMC_Chunk::getBiomNoise(float x, float y, int NoiseSeed, int BiomSeed,float BiomFrequency, TArray<FBiomStruct> BiomStructs, UFastNoiseWrapper* NoiseWrapper)
{
	//HumidityNoise Map
	NoiseWrapper->SetupFastNoise(EFastNoise_NoiseType::Perlin, BiomSeed, BiomFrequency);
	float HumidityNoise = NoiseWrapper->GetNoise2D(x,y);
	//TemperatureNoise Map
	NoiseWrapper->SetupFastNoise(EFastNoise_NoiseType::Perlin, BiomSeed + 23, BiomFrequency*10);
	float TemperatureNoise = NoiseWrapper->GetNoise2D(x,y);

	
	float SumWeights = 0;
	TArray<FNoiseWeightStruct> NoiseWeightStructs;
	
	for (FBiomStruct BiomStruct : BiomStructs)
	{
		if (BiomStruct.enable)
		{
			const float Weight = (HumidityNoise - BiomStruct.Humidity) + (TemperatureNoise - BiomStruct.Temperature);//(SineWaveWeightAlgorithm(HumidityNoise, BiomStruct.Humidity) + SineWaveWeightAlgorithm(TemperatureNoise, BiomStruct.Temperature)) / 2;
			const float LayeredNoise = getLayeredNoise(NoiseSeed, BiomStruct.NoiseStructs, x, y, NoiseWrapper);
			NoiseWeightStructs.Add(FNoiseWeightStruct{Weight, LayeredNoise});
		
			SumWeights += Weight;
		}
	}
	float BiomNoise = 0;
	for (FNoiseWeightStruct WeightStruct : NoiseWeightStructs)
	{
		float actualWeight = FMath::Clamp(WeightStruct.weight/ SumWeights,-2,2);//WeightStruct.weight/ SumWeights; //Clamp Artifact Workaround (Kein plan was da abgeht aber bei Übergängen von Biomen gibt es Artifakte, da actualWeight und Sumweight da kleiner als 1 sind (TODO)

		const float actualNoise = WeightStruct.Noise * actualWeight;
		BiomNoise += actualNoise;
	}
	return BiomNoise;
}

TArray<FVector> ARMC_Chunk::CalculateNormals(TArray<int32> Triangles, TArray<FVector> Vertices)
{
	TArray<FVector> Normals;
	Normals.SetNum(Vertices.Num());
	
	int TriangleCount = Triangles.Num() / 3;
	for (int i = 0; i < TriangleCount; i++)
	{
		int triangleIndex = i * 3;
		FVector vertex1 = Vertices[Triangles[triangleIndex]];
		FVector vertex2 = Vertices[Triangles[triangleIndex+1]];
		FVector vertex3 = Vertices[Triangles[triangleIndex+2]];

		FVector side1 = vertex2 - vertex1;
		FVector side2 = vertex3 - vertex1;
		
		FVector TriangleNormal = side1.Cross(side2);
		TriangleNormal.Normalize();
		

		Normals[Triangles[triangleIndex]] += TriangleNormal;
		Normals[Triangles[triangleIndex + 1]] += TriangleNormal;
		Normals[Triangles[triangleIndex + 2]] += TriangleNormal;
	}
	for (int i = 0; i < Vertices.Num(); i++)
	{
		Normals[i].Normalize();
	}
	return Normals;
}

TArray<int> ARMC_Chunk::GetTriangles(UDynamicMesh* DynamicMesh)
{
	TArray<int> Triangles; 

	bool bHasTriangleIDGaps;

	TArray<FIntVector> TriangleIndices;
	FGeometryScriptTriangleList TriangleList;
	UGeometryScriptLibrary_MeshQueryFunctions::GetAllTriangleIndices(DynamicMesh,TriangleList,false,bHasTriangleIDGaps);
	UGeometryScriptLibrary_ListUtilityFunctions::ConvertTriangleListToArray(TriangleList, TriangleIndices);
	for (auto IntVector : TriangleIndices)
	{
		Triangles.Add(IntVector.X);
		Triangles.Add(IntVector.Y);
		Triangles.Add(IntVector.Z);
	}
	return Triangles;
}

void ARMC_Chunk::copyFromDynamicMesh(UDynamicMesh* DynamicMesh, bool bResetMesh)
{
	RealtimeMeshSimple = RealtimeMeshComponent->InitializeRealtimeMesh<URealtimeMeshSimple>();
	
	TArray<int> Triangles = GetTriangles(DynamicMesh);
	

	FGeometryScriptVectorList PositionList;
	bool bHasVertexIDGaps;
	UGeometryScriptLibrary_MeshQueryFunctions::GetAllVertexPositions(DynamicMesh, PositionList, false, bHasVertexIDGaps);
	TArray<FVector> Positions;
	UGeometryScriptLibrary_ListUtilityFunctions::ConvertVectorListToArray(PositionList, Positions);

	TArray<FVector2d> UV;
	TArray<FVector> Normals;

	/*TArray<int> IndexArray;
	FGeometryScriptIndexList TriangleIDList;
	bool bHasTriangleIDGap;
	UGeometryScriptLibrary_MeshQueryFunctions::GetAllTriangleIDs(DynamicMesh, TriangleIDList, bHasTriangleIDGap);
	UGeometryScriptLibrary_ListUtilityFunctions::ConvertIndexListToArray(TriangleIDList, IndexArray);*/
	for (auto index : Triangles)
	{
		bool bIsValid;
		
		Normals.Add(UGeometryScriptLibrary_MeshQueryFunctions::GetTriangleFaceNormal(DynamicMesh, index , bIsValid));

		FVector2D UV1;
		FVector2D UV2;
		FVector2D UV3;
		bool bHaveValidUVs;

		UGeometryScriptLibrary_MeshQueryFunctions::GetTriangleUVs(DynamicMesh, 0, index,UV1,UV2,UV3,bHaveValidUVs);
		UV.Add(UV1);
	}

	TArray<FProcMeshTangent> ProcMeshTangents;
	TArray<FVector> Tangents;
	TArray<FVector> NewNormals;
	UKismetProceduralMeshLibrary::CalculateTangentsForMesh(Positions, Triangles, UV, NewNormals, ProcMeshTangents);
	for (auto Tangent : ProcMeshTangents)
	{
		Tangents.Add(Tangent.TangentX);
	}

	FRealtimeMeshCollisionConfiguration config;
	config.bUseAsyncCook = true;
	config.bUseComplexAsSimpleCollision = true;
	
	RealtimeMeshSimple->SetCollisionConfig(config);
	const FRealtimeMeshLODKey LODKey{0}; 
	const FRealtimeMeshSectionConfig Config{}; 
	const FRealtimeMeshSimpleMeshData MeshData{Triangles, Positions, NewNormals,Tangents};
	RealtimeMeshSimple->CreateMeshSection(LODKey, Config, MeshData, true);
	//RealtimeMeshSimple->UpdateCollision(true);
	RealtimeMeshSimple->SetupMaterialSlot(0,"None", Material);
	DynamicMesh->Reset();
}

void ARMC_Chunk::generateTerrain(int ChunkSize, int Resolution1D, int NoiseSeed, int BiomSeed, float BiomFrequency, TArray<FBiomStruct> BiomStructs)
{
	//URealtimeMeshSimpleGeometryFunctionLibrary::
	//URealtimeMesh

	RealtimeMeshSimple = RealtimeMeshComponent->InitializeRealtimeMesh<URealtimeMeshSimple>();

	FRealtimeMeshSimpleMeshData MeshData = CalculateMeshData(ChunkSize, Resolution1D, NoiseSeed, BiomSeed, BiomFrequency, BiomStructs);
	RealtimeMeshSimple->CreateMeshSection(0, FRealtimeMeshSectionConfig{ERealtimeMeshSectionDrawType::Static, 0}, MeshData, false);
	
}
