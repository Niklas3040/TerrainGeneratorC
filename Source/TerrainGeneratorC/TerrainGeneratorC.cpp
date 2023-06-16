// Fill out your copyright notice in the Description page of Project Settings.

#include "TerrainGeneratorC.h"
#include "Modules/ModuleManager.h"
#include "Misc/Paths.h"

void FTerrainGeneratorCModule::StartupModule()
{
	FString ShaderDirectory = FPaths::Combine(FPaths::ProjectDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping("/Project", ShaderDirectory);
}

void FTerrainGeneratorCModule::ShutdownModule(){}

IMPLEMENT_PRIMARY_GAME_MODULE( FTerrainGeneratorCModule, TerrainGeneratorC, "TerrainGeneratorC" );
