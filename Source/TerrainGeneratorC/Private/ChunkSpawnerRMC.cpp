// Fill out your copyright notice in the Description page of Project Settings.


#include "ChunkSpawnerRMC.h"

#include "FoliageSpawner.h"
#include "K2Node_SpawnActorFromClass.h"
#include "RMC_Chunk.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

int ChunkIndex = 0;
TArray<FVector> RMCChunkPositionsToLoad;
TMap<FVector, ARMC_Chunk*> LoadedChunks;
bool bIsRunning = false;

AChunkSpawnerRMC::AChunkSpawnerRMC()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AChunkSpawnerRMC::BeginPlay()
{
	Super::BeginPlay();
	RMCChunkPositionsToLoad.Empty();
	LoadedChunks.Empty();
	ChunkIndex = 0;
	bIsRunning = false;
	PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
}

void AChunkSpawnerRMC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CalculateChunkPositions();
	
	if (!bIsRunning)
	{
		if (RMCChunkPositionsToLoad.IsValidIndex(ChunkIndex))
		{
			bIsRunning = true;
			SpawnChunk(FTransform{RMCChunkPositionsToLoad[ChunkIndex]},ChunkIndex);
		}	
	}

}

ARMC_Chunk* AChunkSpawnerRMC::SpawnChunk(FTransform Transform, int Index)
{
	if(GetWorld())
	{
		bIsRunning = true;
		//UE_LOG(LogTemp, Warning,TEXT("chunk Created: %s"), *Transform.ToString())
		
		ARMC_Chunk* Chunk = GetWorld()->SpawnActor<ARMC_Chunk>(ARMC_Chunk::StaticClass(), Transform);
		Chunk->Material = ChunkMaterial;
		Chunk->InitChunk(this);
		
		LoadedChunks.Add(Transform.GetLocation(), Chunk);
		RMCChunkPositionsToLoad.RemoveSingle(Transform.GetLocation());
		
		Chunk->AttachToActor(this, FAttachmentTransformRules{EAttachmentRule::KeepRelative, false});
		
		TArray<FNoiseStruct> NoiseStructs; //TODO remove NoiseStructs and only use BiomStructs

		Chunk->generateTerrain(ChunkSize, Resolution, Seed, Seed+23,BiomFrequency,BiomStructs);
		//Chunk->applyNoise(Seed,BiomStructs, BiomFrequency, Index);

		return Chunk;
	}
	return nullptr;
}

void AChunkSpawnerRMC::SpawnNextChunk()
{
	//UE_LOG(LogTemp, Warning, TEXT("Generate next Chunk"));
	//ChunkIndex++;
	if (RMCChunkPositionsToLoad.IsValidIndex(ChunkIndex))
	{
		SpawnChunk(FTransform{RMCChunkPositionsToLoad[ChunkIndex]}, ChunkIndex);
	}
	else
	{
		bIsRunning = false;
		//UE_LOG(LogTemp, Warning, TEXT("Index not Valid!"));
	}
}

void AChunkSpawnerRMC::CalculateChunkPositions()
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
				if (!(LoadedChunks.Contains(ChunkVector)||RMCChunkPositionsToLoad.Contains(ChunkVector)) )
				{
					double Distance2D = UKismetMathLibrary::Distance2D(FVector2d{PlayerLocation}, FVector2d{ChunkVector});
					if (Distance2D < ChunkRenderDistance )
					{
						RMCChunkPositionsToLoad.Add(ChunkVector);
						//UE_LOG(LogTemp, Warning,TEXT("Added Position: %s  ") ,*ChunkVector.ToString());	
					}
				}
			}
			/*for (auto LoadedChunk : LoadedChunks)
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
				if (Distance2D < FoliageRenderDistance)
				{
					if(!LoadedChunk.Value->bIsFoliageGenerated)
					{
						TArray<FFoliagePositions> FoliagePos = LoadedChunk.Value->GetFoliagePositions(50,100);
						AllFoliagePositions.Append(FoliagePos);
					}
				}
			}*/
		}
	}
}

FVector AChunkSpawnerRMC::RoundDownToChunkSize(FVector Vector)
{
	const int orX = Vector.X;
	const int orY = Vector.Y;
	const float x = orX - (orX % ChunkSize);
	const float y = orY - (orY % ChunkSize);
	
	return FVector{x, y, Vector.Z};
}

void AChunkSpawnerRMC::RemoveChunkFromPositionList(FVector Vector)
{
	LoadedChunks.Remove(Vector);
	//UE_LOG(LogTemp, Warning, TEXT("Added Vector with Position: %s"), *Vector.ToString());
}

void AChunkSpawnerRMC::RemoveFoliagePositionByIndex(int Index)
{
	AllFoliagePositions.RemoveAt(Index);
}


void AChunkSpawnerRMC::SpawnChunksByRow()
{
	ChunkIndex = 0;
	DestroyChunks();
	FVector ActorLocation = GetActorLocation();
	for(int x = (Row/2) *-1; x < Row/2; x++)
	{
		for(int y = (Row/2) *-1; y < Row/2; y++)
		{
			FVector Vector{ActorLocation.X + x*ChunkSize, ActorLocation.Y + y*ChunkSize, ActorLocation.Y};
			RMCChunkPositionsToLoad.Add(Vector);
			//UE_LOG(LogTemp, Warning, TEXT("Added Vector with Position: %s"), *Vector.ToString());
		}
	}
	SpawnChunk(FTransform{RMCChunkPositionsToLoad[0]}, 0);
}

void AChunkSpawnerRMC::DestroyChunks()
{
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARMC_Chunk::StaticClass(), OutActors);

	for (AActor* Chunk : OutActors)
	{
		Chunk->Destroy();
	}
}

