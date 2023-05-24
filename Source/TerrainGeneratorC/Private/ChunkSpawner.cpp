// Fill out your copyright notice in the Description page of Project Settings.


#include "ChunkSpawner.h"

#include "FoliageSpawner.h"
#include "K2Node_SpawnActorFromClass.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

int ChunkIndex = 0;
TArray<FVector> ChunkPositionsToLoad;
TMap<FVector, AChunk*> LoadedChunks;
bool bIsRunning = false;

AChunkSpawner::AChunkSpawner()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AChunkSpawner::BeginPlay()
{
	Super::BeginPlay();

	FoliageSpawner = NewObject<AFoliageSpawner>();
	FoliageSpawner->AttachToActor(this, FAttachmentTransformRules{EAttachmentRule::KeepRelative,false});
	FoliageSpawner->InitFoliageSpawner();
	
	ChunkPositionsToLoad.Empty();
	LoadedChunks.Empty();
	ChunkIndex = 0;
	bIsRunning = false;
	PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
}

void AChunkSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CalculateChunkPositions();
	
	if (!bIsRunning)
	{
		if (ChunkPositionsToLoad.IsValidIndex(ChunkIndex))
		{
			bIsRunning = true;
			SpawnChunk(FTransform{ChunkPositionsToLoad[ChunkIndex]},ChunkIndex);
		}	
	}

}

AChunk* AChunkSpawner::SpawnChunk(FTransform Transform, int Index)
{
	if(GetWorld())
	{
		bIsRunning = true;
		UE_LOG(LogTemp, Warning,TEXT("chunk Created: %s"), *Transform.ToString())
		
		AChunk* Chunk = GetWorld()->SpawnActor<AChunk>(AChunk::StaticClass(), Transform);
		Chunk->InitChunk(this);
		
		LoadedChunks.Add(Transform.GetLocation(), Chunk);
		ChunkPositionsToLoad.RemoveSingle(Transform.GetLocation());
	
		Chunk->SetMaterial(ChunkMaterial);
		Chunk->AttachToActor(this, FAttachmentTransformRules{EAttachmentRule::KeepRelative, false});
		Chunk->createGrid(FGeometryScriptPrimitiveOptions{EGeometryScriptPrimitivePolygroupMode::PerFace, false, EGeometryScriptPrimitiveUVMode::Uniform}, ChunkSize, Resolution);
		
		TArray<FNoiseStruct> NoiseStructs; //TODO remove NoiseStructs and only use BiomStructs
		
		Chunk->applyNoise(Seed,BiomStructs, BiomFrequency, Index);

		return Chunk;
	}
	return nullptr;
}

void AChunkSpawner::SpawnNextChunk()
{
	UE_LOG(LogTemp, Warning, TEXT("Generate next Chunk"));
	//ChunkIndex++;
	if (ChunkPositionsToLoad.IsValidIndex(ChunkIndex))
	{
		SpawnChunk(FTransform{ChunkPositionsToLoad[ChunkIndex]}, ChunkIndex);
	}
	else
	{
		bIsRunning = false;
		UE_LOG(LogTemp, Warning, TEXT("Index not Valid!"));
	}
}

void AChunkSpawner::CalculateChunkPositions()
{
	if(UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
	{
		FVector PlayerLocation = PlayerPawn->GetActorLocation();

		FVector MinVector = RoundDownToChunkSize(FVector{PlayerLocation.X - ChunkRenderDistance,PlayerLocation.Y - ChunkRenderDistance,0.0f});

		FVector MaxVector = RoundDownToChunkSize(FVector{PlayerLocation.X + ChunkRenderDistance,PlayerLocation.Y + ChunkRenderDistance,0.0f});
		
		for (float x = MinVector.X; x <= MaxVector.X; x += ChunkSize)
		{
			for (float y = MinVector.Y; y <= MaxVector.Y; y += ChunkSize)
			{
				FVector ChunkVector{x,y, 0.0f};
				if (!(LoadedChunks.Contains(ChunkVector)||ChunkPositionsToLoad.Contains(ChunkVector)) )
				{
					double Distance2D = UKismetMathLibrary::Distance2D(FVector2d{PlayerLocation}, FVector2d{ChunkVector});
					if (Distance2D < ChunkRenderDistance )
					{
						ChunkPositionsToLoad.Add(ChunkVector);
						UE_LOG(LogTemp, Warning,TEXT("Added Position: %s  ") ,*ChunkVector.ToString());	
					}
				}
			}
			for (auto LoadedChunk : LoadedChunks)
			{
				FVector ChunkVector = LoadedChunk.Value->GetActorLocation();
				double Distance2D = UKismetMathLibrary::Distance2D(FVector2d{PlayerLocation}, FVector2d{ChunkVector});
				if (Distance2D < ChunkSize)
				{
					LoadedChunk.Value->GetDynamicMeshComponent()->SetComplexAsSimpleCollisionEnabled(true);
				}
				else
				{
					LoadedChunk.Value->GetDynamicMeshComponent()->SetComplexAsSimpleCollisionEnabled(false);
				}
				if (Distance2D < 20000)
				{
					if(!LoadedChunk.Value->bIsFoliageGenerated)
					{
						TArray<FFoliagePositions> FoliagePos = LoadedChunk.Value->GetFoliagePositions(50,100);
						AllFoliagePositions.Append(FoliagePos);
					}
				}
			}
		}
	}
}

FVector AChunkSpawner::RoundDownToChunkSize(FVector Vector)
{
	const int orX = Vector.X;
	const int orY = Vector.Y;
	const float x = orX - (orX % ChunkSize);
	const float y = orY - (orY % ChunkSize);
	
	return FVector{x, y, Vector.Z};
}

void AChunkSpawner::RemoveChunkFromPositionList(FVector Vector)
{
	LoadedChunks.Remove(Vector);
	UE_LOG(LogTemp, Warning, TEXT("Added Vector with Position: %s"), *Vector.ToString());
}

void AChunkSpawner::RemoveFoliagePositionByIndex(int Index)
{
	AllFoliagePositions.RemoveAt(Index);
}


void AChunkSpawner::SpawnChunksByRow()
{
	ChunkIndex = 0;
	DestroyChunks();
	FVector ActorLocation = GetActorLocation();
	for(int x = (Row/2) *-1; x < Row/2; x++)
	{
		for(int y = (Row/2) *-1; y < Row/2; y++)
		{
			FVector Vector{ActorLocation.X + x*ChunkSize, ActorLocation.Y + y*ChunkSize, ActorLocation.Y};
			ChunkPositionsToLoad.Add(Vector);
			//UE_LOG(LogTemp, Warning, TEXT("Added Vector with Position: %s"), *Vector.ToString());
		}
	}
	SpawnChunk(FTransform{ChunkPositionsToLoad[0]}, 0);
}

void AChunkSpawner::DestroyChunks()
{
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AChunk::StaticClass(), OutActors);

	for (AActor* Chunk : OutActors)
	{
		Chunk->Destroy();
	}
}

