// Fill out your copyright notice in the Description page of Project Settings.


#include "NoiseDebugger.h"

// Sets default values
ANoiseDebugger::ANoiseDebugger()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ANoiseDebugger::BeginPlay()
{
	NoiseWrapper = NewObject<UFastNoiseWrapper>();
	Super::BeginPlay();
}

// Called every frame
void ANoiseDebugger::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

