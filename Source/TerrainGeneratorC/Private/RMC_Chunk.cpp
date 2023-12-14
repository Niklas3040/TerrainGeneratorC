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
	MeshModifierNormals = NewObject<URuntimeMeshModifierNormals>(); //REMOVE

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

void ARMC_Chunk::SetQuadTriangles(const int QuadIndex, TArray<int32>& Triangles)
{
	const int TriangleIndex = Triangles.Num();
	Triangles.SetNum(TriangleIndex + 6);
	
	Triangles[TriangleIndex] = QuadIndex;
	Triangles[TriangleIndex+1] = QuadIndex+2;
	Triangles[TriangleIndex+2] = QuadIndex+3;
	
	Triangles[TriangleIndex+3] = QuadIndex+4;		
	Triangles[TriangleIndex+4] = QuadIndex+5;	
	Triangles[TriangleIndex+5] = QuadIndex+1;	
}

void ARMC_Chunk::SetQuadVertices(const FVector& ActorLocation, const double X, const double Y, const double X1, const double Y1, TArray<FVector>& Vertices, const int QuadIndex, UFastNoiseWrapper* FastNoiseWrapper, TArray<FBiomStruct> ovBiomStructs, float ovNoiseSeed, float ovBiomSeed,
											float ovBiomFrequency, TArray<FLinearColor>& Colors)
{
	FLinearColor BiomeColor {0.0f,0.0f,255.0f,1.0f};
	
	Vertices.SetNum(QuadIndex + 6);
	
	Vertices[QuadIndex] = FVector{X, Y, getNewBiomNoise(X+ActorLocation.X,Y+ActorLocation.Y, FastNoiseWrapper, ovBiomStructs, ovNoiseSeed, ovBiomSeed, ovBiomFrequency, BiomeColor) };

	Colors.Add(BiomeColor);
	
	const FLinearColor VertexColor0 = BiomeColor;
	
	Vertices[QuadIndex+1] = FVector{X1, Y, getNewBiomNoise(X1+ActorLocation.X,Y+ActorLocation.Y, FastNoiseWrapper, ovBiomStructs, ovNoiseSeed, ovBiomSeed, ovBiomFrequency, BiomeColor) };

	Colors.Add(BiomeColor);
	
	Vertices[QuadIndex+2] = FVector{X, Y1, getNewBiomNoise(X+ActorLocation.X,Y1+ActorLocation.Y, FastNoiseWrapper, ovBiomStructs, ovNoiseSeed, ovBiomSeed, ovBiomFrequency, BiomeColor) };

	Colors.Add(BiomeColor);
	
	Vertices[QuadIndex+3] = FVector{X1, Y1, getNewBiomNoise(X1+ActorLocation.X,Y1+ActorLocation.Y, FastNoiseWrapper, ovBiomStructs, ovNoiseSeed, ovBiomSeed, ovBiomFrequency, BiomeColor) };

	Colors.Add(BiomeColor);
	
	const FLinearColor VertexColor3 = BiomeColor;
	
	Vertices[QuadIndex+4] = Vertices[QuadIndex];
	
	Colors.Add(VertexColor0);
	
	Vertices[QuadIndex+5] = Vertices[QuadIndex+3];
	
	Colors.Add(VertexColor3);
}

FRealtimeMeshSimpleMeshData ARMC_Chunk::GetMeshDataForAllQuads(	int ovChunkSize, int ovResolution1D, TArray<FBiomStruct> ovBiomStructs, float ovNoiseSeed, float ovBiomSeed, float ovBiomFrequency)
{
	UFastNoiseWrapper* FastNoiseWrapper = NewObject<UFastNoiseWrapper>();
	
	const int QuadSize = ovChunkSize/ovResolution1D;
	const FVector ActorLocation = GetActorLocation();
	FRealtimeMeshSimpleMeshData NewMeshData;
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FLinearColor> Colors;

	int x, y;
	for (int i = 0; i < ovResolution1D * ovResolution1D; i++)
	{
		x = i % ovResolution1D;
		y = i / ovResolution1D;
		
		const int QuadIndex = Vertices.Num();
		SetQuadVertices(ActorLocation, x * QuadSize, y * QuadSize, (x+1) * QuadSize, (y+1) * QuadSize, Vertices, QuadIndex, FastNoiseWrapper, ovBiomStructs, ovNoiseSeed, ovBiomSeed, ovBiomFrequency, Colors);
		
		SetQuadTriangles(QuadIndex, Triangles);
	}
	NewMeshData.Positions = Vertices;
	NewMeshData.Triangles = Triangles;
	NewMeshData.LinearColors = Colors;
	
	URuntimeMeshModifierNormals* NormalsModifier = NewObject<URuntimeMeshModifierNormals>();
	NormalsModifier->CalculateNormalsTangents(NewMeshData);
	return NewMeshData;
}

float ARMC_Chunk::getLayeredNoise(int seed, TArray<FNoiseStruct> noiseStructs, float x, float y, UFastNoiseWrapper* NoiseWrapper)
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

float ARMC_Chunk::getBiomNoise(float x, float y, UFastNoiseWrapper* FastNoiseWrapper, TArray<FBiomStruct> ovBiomStructs, float ovNoiseSeed, float ovBiomSeed,
											float ovBiomFrequency, FLinearColor& BiomeColor)
{
	//HumidityNoise Map
	FastNoiseWrapper->SetupFastNoise(EFastNoise_NoiseType::Perlin, ovBiomSeed, ovBiomFrequency);
	float HumidityNoise = FastNoiseWrapper->GetNoise2D(x,y);
	
	//TemperatureNoise Map
	FastNoiseWrapper->SetupFastNoise(EFastNoise_NoiseType::Perlin, ovBiomSeed + 23, ovBiomFrequency*10);
	float TemperatureNoise = FastNoiseWrapper->GetNoise2D(x,y);

	
	float SumWeights = 0;
	TArray<FNoiseWeightStruct> NoiseWeightStructs;
	
	for (FBiomStruct BiomStruct : ovBiomStructs)
	{
		if (BiomStruct.enable)
		{
			const float Weight = (HumidityNoise - BiomStruct.Humidity) + (TemperatureNoise - BiomStruct.Temperature);//(SineWaveWeightAlgorithm(HumidityNoise, BiomStruct.Humidity) + SineWaveWeightAlgorithm(TemperatureNoise, BiomStruct.Temperature)) / 2;
			const float LayeredNoise = getLayeredNoise(ovNoiseSeed, BiomStruct.NoiseStructs, x, y, FastNoiseWrapper);
			NoiseWeightStructs.Add(FNoiseWeightStruct{Weight, LayeredNoise, BiomStruct.Color});
			
			SumWeights += Weight;
		}
	}
	float BiomNoise = 0;
	for (FNoiseWeightStruct WeightStruct : NoiseWeightStructs)
	{
		float actualWeight = FMath::Clamp(WeightStruct.weight/ SumWeights,-2,2);//WeightStruct.weight/ SumWeights; //Clamp Artifact Workaround (Kein plan was da abgeht aber bei Übergängen von Biomen gibt es Artifakte, da actualWeight und Sumweight da kleiner als 1 sind (TODO)

		const float actualNoise = WeightStruct.Noise * actualWeight;
		BiomNoise += actualNoise;
		BiomeColor = UKismetMathLibrary::LinearColorLerp(WeightStruct.Color, BiomeColor, FMath::Clamp(actualWeight,0,1));
	}
	return BiomNoise;
}

float ARMC_Chunk::getNewBiomNoise(float x, float y, UFastNoiseWrapper* FastNoiseWrapper,
	TArray<FBiomStruct> ovBiomStructs, float ovNoiseSeed, float ovBiomSeed, float ovBiomFrequency,
	FLinearColor& BiomeColor)
{
	float transition = 0.7f;
	
	//HumidityNoise Map
	FastNoiseWrapper->SetupFastNoise(EFastNoise_NoiseType::Perlin, ovBiomSeed, ovBiomFrequency);
	float HumidityNoise = FastNoiseWrapper->GetNoise2D(x,y);
	
	//TemperatureNoise Map
	FastNoiseWrapper->SetupFastNoise(EFastNoise_NoiseType::Perlin, ovBiomSeed + 23, ovBiomFrequency*2);
	float TemperatureNoise = FastNoiseWrapper->GetNoise2D(x,y);


	const FBiomStruct BaseLayer = BiomStructs[0];
	/*float SumWeights = 0;
	TArray<FNoiseWeightStruct> NoiseWeightStructs;
	
	for (FBiomStruct BiomStruct : ovBiomStructs)
	{
		if (BiomStruct.enable)
		{
			const float Weight = (HumidityNoise - BiomStruct.Humidity) + (TemperatureNoise - BiomStruct.Temperature);//(SineWaveWeightAlgorithm(HumidityNoise, BiomStruct.Humidity) + SineWaveWeightAlgorithm(TemperatureNoise, BiomStruct.Temperature)) / 2;
			const float LayeredNoise = getLayeredNoise(ovNoiseSeed, BiomStruct.NoiseStructs, x, y, FastNoiseWrapper);
			NoiseWeightStructs.Add(FNoiseWeightStruct{Weight, LayeredNoise, BiomStruct.Color});
			
			SumWeights += Weight;
		}
	}
	float BiomNoise = 0;
	for (FNoiseWeightStruct WeightStruct : NoiseWeightStructs)
	{
		float actualWeight = FMath::Clamp(WeightStruct.weight/ SumWeights,-2,2);//WeightStruct.weight/ SumWeights; //Clamp Artifact Workaround (Kein plan was da abgeht aber bei Übergängen von Biomen gibt es Artifakte, da actualWeight und Sumweight da kleiner als 1 sind (TODO)

		const float actualNoise = WeightStruct.Noise * actualWeight;
		BiomNoise += actualNoise;
		BiomeColor = UKismetMathLibrary::LinearColorLerp(WeightStruct.Color, BiomeColor, FMath::Clamp(actualWeight,0,1));
	}*/
	const float BaseLayerNoise = getLayeredNoise(ovNoiseSeed, BaseLayer.NoiseStructs, x, y, FastNoiseWrapper);

	float BiomNoise = 0.0f;
	
	BiomNoise = BaseLayerNoise;
	BiomeColor = BaseLayer.Color;
	for (int i = 1; i < ovBiomStructs.Num(); i++)
	{
		FBiomStruct BiomStruct = ovBiomStructs[i];
		if (BiomStruct.enable)
		{
			float HBiomeStart = BiomStruct.Humidity - BiomStruct.transition;
			float HBiomeEnd = BiomStruct.Humidity + BiomStruct.transition;

			float TBiomeStart = BiomStruct.Temperature - BiomStruct.transition;
			float TBiomeEnd = BiomStruct.Temperature + BiomStruct.transition;
	
			if ((HumidityNoise < HBiomeEnd) && (HumidityNoise > HBiomeStart) && (TemperatureNoise < TBiomeEnd) && (TemperatureNoise > TBiomeStart))
			{
				const float BiomStructNoise = getLayeredNoise(ovNoiseSeed, BiomStruct.NoiseStructs, x, y, FastNoiseWrapper);
				BiomNoise = BiomStructNoise + BaseLayerNoise;
				BiomeColor = BiomStruct.Color;
				UE_LOG(LogTemp, Warning, TEXT("BiomVertex Found for Biome %s!"),*BiomStruct.BiomName);
			}
		}
	}
	return BiomNoise;

	//const FBiomStruct Biome1 = BiomStructs[1];

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

/*void ARMC_Chunk::DebugTriangleNormals(int TriangleNumber)
{
	TArray<FVector> Vertices = MeshData.Positions;
	TArray<int32> Triangles = MeshData.Triangles;
	TArray<FVector> Normals = MeshData.Normals;
	const int TriangleIndex = TriangleNumber*3;
	
	DebugVertexNormal(Normals[Triangles[TriangleIndex]], Vertices[Triangles[TriangleIndex]]);
	DebugVertexNormal(Normals[Triangles[TriangleIndex+1]], Vertices[Triangles[TriangleIndex+1]]);
	DebugVertexNormal(Normals[Triangles[TriangleIndex+1]], Vertices[Triangles[TriangleIndex+2]]);
}*/

void ARMC_Chunk::NavMeshFix()
{
	FVector ActorLocation = GetActorLocation();
	SetActorLocation({ActorLocation.X, ActorLocation.Y, ActorLocation.Z + 1.0f });
	SetActorLocation(ActorLocation);
}

void ARMC_Chunk::generateTerrain(int ovChunkSize, int ovResolution1D, int ovNoiseSeed, int ovBiomSeed, float ovBiomFrequency, TArray<FBiomStruct> ovBiomStructs)
{
	RealtimeMeshSimple = RealtimeMeshComponent->InitializeRealtimeMesh<URealtimeMeshSimple>();
	RealtimeMeshSimple->SetupMaterialSlot(0,"None",Material);
	
	ChunkSize = ovChunkSize;
	Resolution1D = ovResolution1D;
	BiomStructs = ovBiomStructs;
	BiomSeed = ovBiomSeed;
	BiomFrequency = ovBiomFrequency;
	NoiseSeed = ovNoiseSeed;

	/*AsyncTask(ENamedThreads::AnyThread, [this]()
	{
		FScopeLock Lock(&DataGuard);
		MeshData = GetMeshDataForAllQuads(ChunkSize, Resolution1D, BiomStructs, NoiseSeed, BiomSeed, BiomFrequency);
		AsyncTask(ENamedThreads::GameThread, [this]()
		{
			SectionKey = RealtimeMeshSimple->CreateMeshSection(0,FRealtimeMeshSectionConfig{ERealtimeMeshSectionDrawType::Static,0}, MeshData, true);
			ParentChunkSpawner->SpawnNextChunk();
		});
	});*/
	FRealtimeMeshCollisionConfiguration config;
	config.bUseAsyncCook = true;
	config.bUseComplexAsSimpleCollision = true;
	config.bShouldFastCookMeshes = true;
	RealtimeMeshSimple->SetCollisionConfig(config);
	MeshData = GetMeshDataForAllQuads(ChunkSize, Resolution1D, BiomStructs, NoiseSeed, BiomSeed, BiomFrequency);
	SectionKey = RealtimeMeshSimple->CreateMeshSection(0,FRealtimeMeshSectionConfig{ERealtimeMeshSectionDrawType::Dynamic,0}, MeshData, true);

	RealtimeMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	FVector ActorLocation = GetActorLocation();
	SetActorLocation({ActorLocation.X, ActorLocation.Y, ActorLocation.Z + 200.0f });
	
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ARMC_Chunk::NavMeshFix, 1.0f, false, 1.0f);
	//NavMeshFix();
	
	GetWorld()->GetTimerManager().SetTimerForNextTick(ParentChunkSpawner, &AChunkSpawnerRMC::SpawnNextChunk);
	//ParentChunkSpawner->SpawnNextChunk();
}
