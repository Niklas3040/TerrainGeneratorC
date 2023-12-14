// Fill out your copyright notice in the Description page of Project Settings.


#include "PerfChunk.h"

#include "GenericPlatform/GenericPlatformChunkInstall.h"


APerfChunk::APerfChunk()
{
	PrimaryActorTick.bCanEverTick = true;

}

void APerfChunk::GenerateTerrain()
{
	FastNoiseWrapper = NewObject<UFastNoiseWrapper>();
	FastNoiseWrapper->SetSeed(Seed);
	FastNoiseWrapper->SetFrequency(frequency);
	FastNoiseWrapper->SetNoiseType(Type);
	FGeometryScriptSimpleMeshBuffers Buffers = GetMeshDataForAllQuads();
	FGeometryScriptIndexList NewTriangleIndicesList;
	UDynamicMesh* TargetMesh = GetDynamicMeshComponent()->GetDynamicMesh();
	UGeometryScriptLibrary_MeshBasicEditFunctions::AppendBuffersToMesh(TargetMesh, Buffers,NewTriangleIndicesList,0,true);
	UGeometryScriptLibrary_MeshNormalsFunctions::RecomputeNormals(TargetMesh,{});
	UGeometryScriptLibrary_MeshUVFunctions::RecomputeMeshUVs(TargetMesh,0,{});
}

FGeometryScriptSimpleMeshBuffers APerfChunk::GetMeshDataForAllQuads() const
{
	FGeometryScriptSimpleMeshBuffers MeshData;

	
	const int QuadSize = ChunkSize/Resolution_1D;
	const FVector ActorLocation = GetActorLocation();
	
	TArray<FVector> Vertices;
	TArray<FIntVector> Triangles;

	int x, y;
	for (int i = 0; i < Resolution_1D * Resolution_1D; i++)
	{
		x = i % Resolution_1D;
		y = i / Resolution_1D;
		
		const int QuadIndex = Vertices.Num();
		SetQuadVertices(ActorLocation, x * QuadSize, y * QuadSize, (x+1) * QuadSize, (y+1) * QuadSize, Vertices, QuadIndex);
		
		SetQuadTriangles(QuadIndex, Triangles);
	}
	MeshData.Vertices = Vertices;
	MeshData.Triangles = Triangles;

	return MeshData;
}

void APerfChunk::SetQuadTriangles(const int QuadIndex, TArray<FIntVector>& Triangles)
{
	const int TriangleIndex = Triangles.Num();
	Triangles.SetNum(TriangleIndex + 2);
	Triangles[TriangleIndex] = {QuadIndex, QuadIndex+2,QuadIndex+3};
	Triangles[TriangleIndex+1] = {QuadIndex+4, QuadIndex+5,QuadIndex+1};
}

void APerfChunk::SetQuadVertices(const FVector& ActorLocation, const double X, const double Y, const double X1,
	const double Y1, TArray<FVector>& Vertices, const int QuadIndex) const
{
	{
		Vertices.SetNum(QuadIndex + 6);
		Vertices[QuadIndex] = FVector{X, Y, GetVertexHeight(ActorLocation, X, Y)};
		Vertices[QuadIndex+1] = FVector{X1, Y, GetVertexHeight(ActorLocation, X1, Y)};
		Vertices[QuadIndex+2] = FVector{X, Y1, GetVertexHeight(ActorLocation, X, Y1)};
		Vertices[QuadIndex+3] = FVector{X1, Y1, GetVertexHeight(ActorLocation, X1, Y1)};
		Vertices[QuadIndex+4] = Vertices[QuadIndex];
		Vertices[QuadIndex+5] = Vertices[QuadIndex+3];
	}
}

float APerfChunk::GetVertexHeight(const FVector& ActorLocation, double X, double Y) const
{
	const float x = ActorLocation.X + X;
	const float y = ActorLocation.Y + Y;
	const float Noise = FastNoiseWrapper->GetNoise2D(x,y) * multiplicator;
	UE_LOG(LogTemp, Warning, TEXT("The Noise value is: %f"), Noise);
	return Noise;
}
