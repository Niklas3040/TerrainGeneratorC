// Fill out your copyright notice in the Description page of Project Settings.


#include "MulithreadingTestActor.h"

#include "../../../../../../../../Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.30.30705/INCLUDE/mutex"
#include "DSP/Osc.h"

// Sets default values
AMulithreadingTestActor::AMulithreadingTestActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AMulithreadingTestActor::BeginPlay()
{
	Super::BeginPlay();
	//AsyncGetRandomNoise();
}

// Called every frame
void AMulithreadingTestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMulithreadingTestActor::AsyncGetRandomNoise()
{
	AsyncTask(ENamedThreads::AnyThread, [this]()
	{
		float FullNoise = 0;
		UFastNoiseWrapper* NoiseWrapper = NewObject<UFastNoiseWrapper>();
	
		for (int x = 0; x < 100000; x++)
		{
			NoiseWrapper->SetupFastNoise(EFastNoise_NoiseType::Perlin, x);
			float Noise2D = NoiseWrapper->GetNoise2D(x,x+27);
			FullNoise += Noise2D;
			//UE_LOG(LogTemp, Warning, TEXT("nextNoise: %f"), Noise2D);
		}
		AsyncTask(ENamedThreads::GameThread, [this, FullNoise]()
		{
			LogNoise(FullNoise);
		});
	});
}

void AMulithreadingTestActor::LogNoise(int Noise)
{
	UE_LOG(LogTemp, Warning, TEXT("full Noise: %d"), Noise);
}


/*
float AMulithreadingTestActor::GetLayeredNoise(int seed, TArray<FNoiseStruct> noiseStructs, float x, float y, UFastNoiseWrapper* NoiseWrapper)
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
*/

/*void AMulithreadingTestActor::AsyncGetBiomeNoise(float x, float y, TArray<FBiomStruct> BiomStructs, float NoiseSeed, float BiomSeed,
                                            float BiomFrequency)
{
	AsyncTask(ENamedThreads::AnyThread, [this, x, y, BiomStructs, BiomSeed, BiomFrequency, NoiseSeed]()
	{
		UFastNoiseWrapper* NoiseWrapper = NewObject<UFastNoiseWrapper>();
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
				const float LayeredNoise = GetLayeredNoise(NoiseSeed, BiomStruct.NoiseStructs, x, y, NoiseWrapper);
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
	
		AsyncTask(ENamedThreads::GameThread, [this, BiomNoise]()
		{
			LogNoise(BiomNoise);
		});
	});
}*/


void AMulithreadingTestActor::SetQuadTriangles(const int QuadIndex, TArray<int32>& Triangles)
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

void AMulithreadingTestActor::SetQuadVertices(const FVector& ActorLocation, const double X, const double Y, const double X1, const double Y1, TArray<FVector>& Vertices, const int QuadIndex, UFastNoiseWrapper* FastNoiseWrapper, TArray<FBiomStruct> ovBiomStructs, float ovNoiseSeed, float ovBiomSeed,
											float ovBiomFrequency)
{
	Vertices.SetNum(QuadIndex + 6);
	Vertices[QuadIndex] =	FVector{X, Y, getBiomNoise(X+ActorLocation.X,Y+ActorLocation.Y, FastNoiseWrapper, ovBiomStructs, ovNoiseSeed, ovBiomSeed, ovBiomFrequency) };
	Vertices[QuadIndex+1] = FVector{X1, Y, getBiomNoise(X1+ActorLocation.X,Y+ActorLocation.Y, FastNoiseWrapper, ovBiomStructs, ovNoiseSeed, ovBiomSeed, ovBiomFrequency) };
	Vertices[QuadIndex+2] = FVector{X, Y1, getBiomNoise(X+ActorLocation.X,Y1+ActorLocation.Y, FastNoiseWrapper, ovBiomStructs, ovNoiseSeed, ovBiomSeed, ovBiomFrequency) };
	
	Vertices[QuadIndex+3] = FVector{X1, Y1, getBiomNoise(X1+ActorLocation.X,Y1+ActorLocation.Y, FastNoiseWrapper, ovBiomStructs, ovNoiseSeed, ovBiomSeed, ovBiomFrequency) };
	Vertices[QuadIndex+4] = Vertices[QuadIndex];
	Vertices[QuadIndex+5] = Vertices[QuadIndex+3];
}

FRealtimeMeshSimpleMeshData AMulithreadingTestActor::GetMeshDataForAllQuads(int ovChunkSize, int ovResolution1D, TArray<FBiomStruct> ovBiomStructs, float ovNoiseSeed, float ovBiomSeed, float ovBiomFrequency, FVector Location)
{
	UFastNoiseWrapper* FastNoiseWrapper = NewObject<UFastNoiseWrapper>();
	
	const int QuadSize = ovChunkSize/ovResolution1D;
	const FVector ActorLocation = Location;
	FRealtimeMeshSimpleMeshData NewMeshData;
	TArray<FVector> Vertices;
	TArray<int32> Triangles;

	int x, y;
	for (int i = 0; i < ovResolution1D * ovResolution1D; i++)
	{
		x = i % ovResolution1D;
		y = i / ovResolution1D;
		
		const int QuadIndex = Vertices.Num();
		SetQuadVertices(ActorLocation, x * QuadSize, y * QuadSize, (x+1) * QuadSize, (y+1) * QuadSize, Vertices, QuadIndex, FastNoiseWrapper, ovBiomStructs, ovNoiseSeed, ovBiomSeed, ovBiomFrequency);
		
		SetQuadTriangles(QuadIndex, Triangles);
	}
	NewMeshData.Positions = Vertices;
	NewMeshData.Triangles = Triangles;
	
	URuntimeMeshModifierNormals* NormalsModifier = NewObject<URuntimeMeshModifierNormals>();
	NormalsModifier->CalculateNormalsTangents(NewMeshData);
	
	return NewMeshData;
}

float AMulithreadingTestActor::getLayeredNoise(int seed, TArray<FNoiseStruct> noiseStructs, float x, float y, UFastNoiseWrapper* NoiseWrapper)
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

float AMulithreadingTestActor::getBiomNoise(float x, float y, UFastNoiseWrapper* FastNoiseWrapper, TArray<FBiomStruct> ovBiomStructs, float ovNoiseSeed, float ovBiomSeed,
											float ovBiomFrequency)
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


