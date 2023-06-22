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

void AMulithreadingTestActor::AsyncGetBiomeNoise(float x, float y, TArray<FBiomStruct> BiomStructs, float NoiseSeed, float BiomSeed,
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
}

