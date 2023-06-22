// Fill out your copyright notice in the Description page of Project Settings.



#include "RMC_Chunk.h"

#include "ChunkSpawnerRMC.h"
#include "FastNoiseWrapper.h"
#include "KismetProceduralMeshLibrary.h"
#include "RealtimeMeshLibrary.h"
#include "UDynamicMesh.h"
#include "GeometryScript/ListUtilityFunctions.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

AChunkSpawnerRMC* ParentChunkSpawner = nullptr;

ARMC_Chunk::ARMC_Chunk()
{
	RealtimeMeshComponent = CreateDefaultSubobject<URealtimeMeshComponent>(TEXT("RMC_Component"));
	RealtimeMeshComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	MeshModifierNormals = NewObject<URuntimeMeshModifierNormals>();

	Plane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Plane"));
	Plane->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	Plane->SetRelativeLocation(FVector{0.0f,0.0f,-200.0f});
	
	ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("StaticMesh'/Engine/BasicShapes/Plane.Plane'"));
	Plane->SetStaticMesh(MeshAsset.Object);
	
	ConstructorHelpers::FObjectFinder<UMaterialInterface> WaterMaterial(TEXT("MaterialInstanceConstant'/Game/environment/WaterPlane/M_Toon_Water_Inst.M_Toon_Water_Inst'"));

	Plane->SetMaterial(0, WaterMaterial.Object);
	Plane->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ARMC_Chunk::InitChunk(AActor* ChunkSpawner)
{
	ParentChunkSpawner = Cast<AChunkSpawnerRMC>(ChunkSpawner);
	Plane->SetRelativeScale3D(FVector(ParentChunkSpawner->ChunkSize /100));
	Plane->SetWorldLocation(GetActorLocation());
}


/*void ARMC_Chunk::CalculateUV(int Resolution1D, const TArray<FVector>& Positions, TArray<FVector2d>& UV0)
{
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
}*/


void ARMC_Chunk::DebugVertexNormal(FVector Normal, FVector Vertex) const
{
	const FVector Location = GetActorLocation();
	const FVector VertexWorldLocation = Vertex + Location;
	UKismetSystemLibrary::DrawDebugArrow(GetWorld(),VertexWorldLocation,Normal * 300.0f + VertexWorldLocation,1.0f,FLinearColor::Blue, 100.0f, 2.0f );
}

FRealtimeMeshSimpleMeshData ARMC_Chunk::GetMeshDataForQuad()
{
	
	const int QuadSize = ChunkSize/Resolution1D;
	const FVector ActorLocation = GetActorLocation();
	double x = X * QuadSize;
	double y = Y * QuadSize;
	double X1 = (X+1) * QuadSize;
	double Y1 = (Y+1) * QuadSize;

	TArray<FVector> Vertices = MeshData.Positions;
	
	const int QuadIndex = Vertices.Num();
	Vertices.SetNum(QuadIndex + 6);
	Vertices[QuadIndex] =	FVector{x, y, getBiomNoise(x+ActorLocation.X,y+ActorLocation.Y) };
	Vertices[QuadIndex+1] = FVector{X1, y, getBiomNoise(X1+ActorLocation.X,y+ActorLocation.Y) };
	Vertices[QuadIndex+2] = FVector{x, Y1, getBiomNoise(x+ActorLocation.X,Y1+ActorLocation.Y) };
	
	Vertices[QuadIndex+3] = FVector{X1, Y1, getBiomNoise(X1+ActorLocation.X,Y1+ActorLocation.Y) };
	Vertices[QuadIndex+4] = Vertices[QuadIndex];
	Vertices[QuadIndex+5] = Vertices[QuadIndex+3];
	
	TArray<int32> Triangles = MeshData.Triangles;
	
	const int TriangleIndex = Triangles.Num();
	Triangles.SetNum(TriangleIndex + 6);
	
	Triangles[TriangleIndex] = QuadIndex;
	Triangles[TriangleIndex+1] = QuadIndex+2;
	Triangles[TriangleIndex+2] = QuadIndex+3;
	
	Triangles[TriangleIndex+3] = QuadIndex+4;
	Triangles[TriangleIndex+4] = QuadIndex+5;
	Triangles[TriangleIndex+5] = QuadIndex+1;

	TArray<FVector2d> UV0 = MeshData.UV0;
	
	FRealtimeMeshSimpleMeshData NewMeshData = MeshData;
	
	NewMeshData.Positions = Vertices;
	NewMeshData.Triangles = Triangles;
	
	return NewMeshData;
}

float ARMC_Chunk::getLayeredNoise(int seed, TArray<FNoiseStruct> noiseStructs, float x, float y)
{
	float noise = 0;
	for (FNoiseStruct Noise : noiseStructs)
	{
		if(Noise.enable)
		{
			NoiseWrapper->SetupFastNoise(Noise.type, seed, Noise.frequency, EFastNoise_Interp::Quintic, EFastNoise_FractalType::FBM, 3, 2.0f);
			if(Noise.ridgeNoise)
			{
				noise += FMath::Abs((NoiseWrapper->GetNoise2D(x, y)*Noise.multiplier -0.5f) * 2.0f);
			}
			else
				noise += NoiseWrapper->GetNoise2D(x, y) * Noise.multiplier;
		}
	}
	return noise;
}

float ARMC_Chunk::getBiomNoise(float x, float y)
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
			const float LayeredNoise = getLayeredNoise(NoiseSeed, BiomStruct.NoiseStructs, x, y);
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

TArray<int> ARMC_Chunk::GetDynamicMeshTriangles(UDynamicMesh* DynamicMesh)
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
	
	TArray<int> Triangles = GetDynamicMeshTriangles(DynamicMesh);
	

	FGeometryScriptVectorList PositionList;
	bool bHasVertexIDGaps;
	UGeometryScriptLibrary_MeshQueryFunctions::GetAllVertexPositions(DynamicMesh, PositionList, false, bHasVertexIDGaps);
	TArray<FVector> Positions;
	UGeometryScriptLibrary_ListUtilityFunctions::ConvertVectorListToArray(PositionList, Positions);

	TArray<FVector2d> UV;
	TArray<FVector> Normals;

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
	const FRealtimeMeshSimpleMeshData SimpleMeshData{Triangles, Positions, NewNormals,Tangents};
	RealtimeMeshSimple->CreateMeshSection(LODKey, Config, SimpleMeshData, true);
	RealtimeMeshSimple->SetupMaterialSlot(0,"None", Material);
	DynamicMesh->Reset();
}

void ARMC_Chunk::ConstructNextQuadSection()
{
	if (X == Resolution1D)
	{
		Y++;
		X = 0;
	}
	else
		X++;
	if(Y <= Resolution1D)
	{
		ConstructQuadSection();
	}
	//finishedConstructingQuads
	else
	{
		MeshModifierNormals->CalculateNormalsTangents(MeshData);
		RealtimeMeshSimple->UpdateSectionMesh(SectionKey, MeshData);

		GetWorld()->GetTimerManager().SetTimerForNextTick(ParentChunkSpawner, &AChunkSpawnerRMC::SpawnNextChunk);
	}
}

void ARMC_Chunk::ConstructQuadSection()
{
	UE_LOG(LogTemp, Warning, TEXT("NextQuad"));
	
	MeshData = GetMeshDataForQuad();
	
	//RealtimeMeshSimple->UpdateSectionMesh(SectionKey, MeshData);
	
	ConstructNextQuadSection();
}

void ARMC_Chunk::DebugTriangleNormals(int TriangleNumber)
{
	TArray<FVector> Vertices = MeshData.Positions;
	TArray<int32> Triangles = MeshData.Triangles;
	TArray<FVector> Normals = MeshData.Normals;
	const int TriangleIndex = TriangleNumber*3;
	
	DebugVertexNormal(Normals[Triangles[TriangleIndex]], Vertices[Triangles[TriangleIndex]]);
	DebugVertexNormal(Normals[Triangles[TriangleIndex+1]], Vertices[Triangles[TriangleIndex+1]]);
	DebugVertexNormal(Normals[Triangles[TriangleIndex+1]], Vertices[Triangles[TriangleIndex+2]]);
}

void ARMC_Chunk::generateTerrain(int ovChunkSize, int ovResolution1D, int ovNoiseSeed, int ovBiomSeed, float ovBiomFrequency, TArray<FBiomStruct> ovBiomStructs)
{
	RealtimeMeshSimple = RealtimeMeshComponent->InitializeRealtimeMesh<URealtimeMeshSimple>();
	RealtimeMeshSimple->SetupMaterialSlot(0,"None",Material);

	NoiseWrapper = NewObject<UFastNoiseWrapper>();
	MeshData = {};
	ChunkSize = ovChunkSize;
	Resolution1D = ovResolution1D;
	NoiseSeed = ovNoiseSeed;
	BiomSeed = ovBiomSeed;
	BiomFrequency = ovBiomFrequency;
	BiomStructs = ovBiomStructs;
	
	SectionKey = RealtimeMeshSimple->CreateMeshSection(0,FRealtimeMeshSectionConfig{ERealtimeMeshSectionDrawType::Dynamic,0}, MeshData, true);

	X = 0;
	Y = 0;
	
	//GetWorldTimerManager().SetTimer(TimerHandle, this, &ARMC_Chunk::ConstructQuadSection, 1.0f, true, 1.0f);
	ConstructQuadSection();
	//loop
}
