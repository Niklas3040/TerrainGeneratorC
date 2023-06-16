// Fill out your copyright notice in the Description page of Project Settings.


#include "Chunk.h"
#include "ChunkSpawner.h"

#include "../../../../../../../../Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.30.30705/INCLUDE/filesystem"
#include "GeometryScript/MeshAssetFunctions.h"
#include "GeometryScript/MeshSpatialFunctions.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/TransformCalculus3D.h"
#include "solver/PxSolverDefs.h"
AChunkSpawner* ParentChunkSpawner = nullptr;

AChunk::AChunk()
{
	PrimaryActorTick.bCanEverTick = true;
	
	Plane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Plane"));
	Plane->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	Plane->SetRelativeLocation(FVector{0.0f,0.0f,-200.0f});
	
	ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("StaticMesh'/Engine/BasicShapes/Plane.Plane'"));
	Plane->SetStaticMesh(MeshAsset.Object);
	
	ConstructorHelpers::FObjectFinder< UMaterialInterface> Material(TEXT("MaterialInstanceConstant'/Game/environment/WaterPlane/M_Toon_Water_Inst.M_Toon_Water_Inst'"));

	Plane->SetMaterial(0, Material.Object);
	Plane->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//GetDynamicMeshComponent()->EnableComplexAsSimpleCollision(); //TODO Collision

	ChildActor = CreateDefaultSubobject<UChildActorComponent>(TEXT("rmc"));
	ChildActor->SetupAttachment(GetRootComponent());
	ChildActor->SetChildActorClass(ARMC_Chunk::StaticClass());
	
}


void AChunk::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (ParentChunkSpawner)
	{
		if (ParentChunkSpawner->PlayerPawn)
		{
			FVector PlayerLocation = ParentChunkSpawner->PlayerPawn->GetActorLocation();
			FVector ActorLocation = GetActorLocation();
			double Distance2D = UKismetMathLibrary::Distance2D(FVector2d{PlayerLocation}, FVector2d{ActorLocation});
			if (Distance2D >= ParentChunkSpawner->ChunkRenderDistance)
			{
				ParentChunkSpawner->RemoveChunkFromPositionList(ActorLocation);
				Destroy();
				UE_LOG(LogTemp, Warning, TEXT("killChunk"));
			}
		}
	}
}

void AChunk::createGrid(FGeometryScriptPrimitiveOptions PrimitiveOptions, int chunkSize = 128, int resolution = 4)
{
	int32 steps = sqrt(resolution) + 1;
	UDynamicMesh* DynamicMesh = DynamicMeshComponent->GetDynamicMesh();

	const FTransform Transform{
		FRotator{},													// Rotation
		FVector{0.0f, 0.0f, 0.0f},						// Translation
		FVector{1.0f, 1.0f, 1.0f}							// Scale
	};
	
	UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendRectangleXY(DynamicMesh, PrimitiveOptions, Transform, chunkSize, chunkSize, steps, steps);
}


void AChunk::applyNoise(int seed, TArray<FBiomStruct> BiomStructs, float BiomFrequency, int ChunkIndex)
{
	//GAME THREAD
	UDynamicMesh* DynamicMesh = DynamicMeshComponent->GetDynamicMesh();
		
	FGeometryScriptVectorList PositionList;
	bool HasVertexIDGaps = false;
		
	UGeometryScriptLibrary_MeshQueryFunctions::GetAllVertexPositions(DynamicMesh, PositionList,false, HasVertexIDGaps);

	TArray<FVector> Vertices;
	UGeometryScriptLibrary_ListUtilityFunctions::ConvertVectorListToArray(PositionList, Vertices);

	FVector ActorLocation = GetActorLocation();
	
	//OTHER THREAD
	UFastNoiseWrapper* NoiseWrapper = NewObject<UFastNoiseWrapper>();
	int VertexID = 0;
	for (FVector Vector : Vertices)
	{
		FVector NoiseVector = ActorLocation + Vector;
		float MultipliedNoise = getBiomNoise(NoiseVector.X, NoiseVector.Y, seed, seed + 1.0f, BiomFrequency, BiomStructs, NoiseWrapper); //MT Problem?
			
		FVector NewPosition = Vector + FVector(0.0f, 0.0f, MultipliedNoise);
			
		//GAME THREAD
		bool bIsValidVertex = UGeometryScriptLibrary_MeshQueryFunctions::IsValidVertexID(DynamicMesh, VertexID);
		UGeometryScriptLibrary_MeshBasicEditFunctions::SetVertexPosition(DynamicMesh, VertexID, NewPosition, bIsValidVertex);
		VertexID++;
	}
	//GAME THREAD
	UGeometryScriptLibrary_MeshNormalsFunctions::SetPerFaceNormals(DynamicMesh);
	//CreateCollision();
	//UGeometryScriptLibrary_MeshNormalsFunctions::RecomputeNormals(DynamicMesh, FGeometryScriptCalculateNormalsOptions{true, true});
			
	FTimerHandle TimerHandle;
	
	//NEW RMC STUFF
	RMC = Cast<ARMC_Chunk>(ChildActor->GetChildActor());
	RMC->Material = ChunkMaterial;
	RMC->copyFromDynamicMesh(GetDynamicMeshComponent()->GetDynamicMesh(), true);
	
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, ParentChunkSpawner, &AChunkSpawner::SpawnNextChunk, 0.0000001f, false);
	//CreateCollision();
	//ChunkSpawner->SpawnNextChunk();
}

float AChunk::getLayeredNoise(int seed, TArray<FNoiseStruct> noiseStructs, float x, float y, UFastNoiseWrapper* FastNoiseWrapper)
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

float AChunk::getBiomNoise(float x, float y, int NoiseSeed, int BiomSeed,float BiomFrequency, TArray<FBiomStruct> BiomStructs, UFastNoiseWrapper* NoiseWrapper)
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

void AChunk::SetMaterial(UMaterialInterface* Material)
{
	if (Material)
	{
		ChunkMaterial = Material;
		DynamicMeshComponent->SetMaterial(0, Material);
	}
}

void AChunk::InitChunk(AActor* ChunkSpawner)
{
	ParentChunkSpawner = Cast<AChunkSpawner>(ChunkSpawner);
	Plane->SetRelativeScale3D(FVector(ParentChunkSpawner->ChunkSize /100));
}

void AChunk::CreateCollision()
{
	FGeometryScriptCollisionFromMeshOptions Options
	{
		true,
		EGeometryScriptCollisionGenerationMethod::ConvexHulls,
		false,
		false,
		false,
		0.1f,
		false,
		2000,
		0.1f,
		EGeometryScriptSweptHullAxis::Z,
		false,
		0
		
	};
	UGeometryScriptLibrary_CollisionFunctions::SetDynamicMeshCollisionFromMesh(GetDynamicMeshComponent()->GetDynamicMesh(),GetDynamicMeshComponent(), FGeometryScriptCollisionFromMeshOptions{});
	FreeAllComputeMeshes();
}

float AChunk::SineWaveWeightAlgorithm(float x, float h)
{
	float b = 1.5707963224218/h;
	float c = h * 4;
	return (0.5* sin(b* (x+c))+0.5);
}

TArray<FFoliagePositions> AChunk::GetFoliagePositions(int MinIterations, int MaxIterations)
{
	bIsFoliageGenerated = true;
	UDynamicMesh* DynamicMesh = DynamicMeshComponent->GetDynamicMesh();
	FGeometryScriptDynamicMeshBVH BHV;
	UGeometryScriptLibrary_MeshSpatial::BuildBVHForMesh(DynamicMesh, BHV);
	
	FBox MeshBoundingBox = UGeometryScriptLibrary_MeshQueryFunctions::GetMeshBoundingBox(DynamicMesh);
	FVector Center = (MeshBoundingBox.Min + MeshBoundingBox.Max)*FVector(0.5f,0.5f,0.5f);
	FVector HalfSize = (MeshBoundingBox.Max - MeshBoundingBox.Min)*FVector(0.5f,0.5f,0.5f);

	TArray<FFoliagePositions> ReturnArray;

	int32 RandRange = FMath::RandRange(MinIterations, MaxIterations);
	for(int x = 0; x <= RandRange; x++)
	{
		FVector RandomPointInBoundingBox = UKismetMathLibrary::RandomPointInBoundingBox(Center, HalfSize);
		TEnumAsByte<EGeometryScriptSearchOutcomePins> Outcome{};

		FGeometryScriptTrianglePoint NearestResult;
		UGeometryScriptLibrary_MeshSpatial::FindNearestPointOnMesh(DynamicMesh, BHV, RandomPointInBoundingBox,FGeometryScriptSpatialQueryOptions{}, NearestResult, Outcome);
		if (Outcome.GetValue() == Found)
		{
			bool bIsValidTriangle;
			FVector TriangleFaceNormal = UGeometryScriptLibrary_MeshQueryFunctions::GetTriangleFaceNormal(DynamicMesh, NearestResult.TriangleID, bIsValidTriangle);
			NearestResult.Position = NearestResult.Position + GetActorLocation();
			ReturnArray.Add(FFoliagePositions{NearestResult, TriangleFaceNormal});
			UE_LOG(LogTemp, Warning, TEXT("FOUND"))
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Outcome not Found!"))
		}	
	}
	return ReturnArray;
}


TArray<FTriangleStruct> AChunk::GenerateBuildingPositions()
{
	UDynamicMesh* DynamicMesh = DynamicMeshComponent->GetDynamicMesh();
	
	TArray<FTriangleStruct> ReturnArray;
	

	for(int x = 0; x < ParentChunkSpawner->Resolution*2; x++)
	{
		FVector Vertex1, Vertex2, Vertex3;
		bool bIsValidTriangle;
		UGeometryScriptLibrary_MeshQueryFunctions::GetTrianglePositions(DynamicMesh, x, bIsValidTriangle, Vertex1, Vertex2, Vertex3);

		if (bIsValidTriangle)
		{
			ReturnArray.Add(FTriangleStruct{x, Vertex1 + GetActorLocation(), Vertex2 + GetActorLocation(), Vertex3 + GetActorLocation()});	
		}
	}
	return ReturnArray;
}


