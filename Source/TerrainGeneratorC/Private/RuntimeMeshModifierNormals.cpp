// Fill out your copyright notice in the Description page of Project Settings.


#include "RuntimeMeshModifierNormals.h"

void URuntimeMeshModifierNormals::CalculateNormalsTangents(FRealtimeMeshSimpleMeshData& MeshData) const
{
	int32 NumVertices = MeshData.Positions.Num();
	int32 NumIndices = MeshData.Triangles.Num();
	MeshData.Normals.SetNum(NumVertices);
	MeshData.Tangents.SetNum(NumVertices);
	
	//int32 NumUVs = MeshData.TexCoords.Num();

	// Resize the tangents array to fit the new normals/tangents
	MeshData.Tangents.SetNum(NumVertices, true);
	
	const TMultiMap<uint32, uint32> DuplicateVertexMap = TMultiMap<uint32, uint32>();
	
	// Number of triangles
	const int32 NumTris = NumIndices / 3;

	// Map of vertex to triangles in Triangles array
	TMultiMap<uint32, uint32> VertToTriMap;
	// Map of vertex to triangles to consider for normal calculation
	TMultiMap<uint32, uint32> VertToTriSmoothMap;

	// Normal/tangents for each face
	TArray<FVector3f> FaceTangentX, FaceTangentY, FaceTangentZ;
	FaceTangentX.AddUninitialized(NumTris);
	FaceTangentY.AddUninitialized(NumTris);
	FaceTangentZ.AddUninitialized(NumTris);

	// Iterate over triangles
	for (int32 TriIdx = 0; TriIdx < NumTris; TriIdx++)
	{
		uint32 CornerIndex[3];
		FVector3f P[3];

		for (int32 CornerIdx = 0; CornerIdx < 3; CornerIdx++)
		{
			// Find vert index (clamped within range)
			uint32 VertIndex = FMath::Min(MeshData.Triangles[TriIdx * 3 + CornerIdx], NumVertices - 1); //GetVertexIndex(TriIdx * 3 + CornerIdx), (uint32)NumVertices - 1);

			CornerIndex[CornerIdx] = VertIndex;
			P[CornerIdx] = FVector3f{MeshData.Positions[VertIndex]};

			// Find/add this vert to index buffer
			TArray<uint32> VertOverlaps;
			DuplicateVertexMap.MultiFind(VertIndex, VertOverlaps);

			// Remember which triangles map to this vert
			VertToTriMap.AddUnique(VertIndex, TriIdx);
			VertToTriSmoothMap.AddUnique(VertIndex, TriIdx);

			// Also update map of triangles that 'overlap' this vert (ie don't match UV, but do match smoothing) and should be considered when calculating normal
			for (int32 OverlapIdx = 0; OverlapIdx < VertOverlaps.Num(); OverlapIdx++)
			{
				// For each vert we overlap..
				int32 OverlapVertIdx = VertOverlaps[OverlapIdx];

				// Add this triangle to that vert
				VertToTriSmoothMap.AddUnique(OverlapVertIdx, TriIdx);

				// And add all of its triangles to us
				TArray<uint32> OverlapTris;
				VertToTriMap.MultiFind(OverlapVertIdx, OverlapTris);
				for (int32 OverlapTriIdx = 0; OverlapTriIdx < OverlapTris.Num(); OverlapTriIdx++)
				{
					VertToTriSmoothMap.AddUnique(VertIndex, OverlapTris[OverlapTriIdx]);
				}
			}
		}

		// Calculate triangle edge vectors and normal
		const FVector3f Edge21 = P[1] - P[2];
		const FVector3f Edge20 = P[0] - P[2];
		const FVector3f TriNormal = (Edge21 ^ Edge20).GetSafeNormal();


		FaceTangentX[TriIdx] = Edge20.GetSafeNormal();
		FaceTangentY[TriIdx] = (FaceTangentX[TriIdx] ^ TriNormal).GetSafeNormal();

		FaceTangentZ[TriIdx] = TriNormal;
	}


	// Arrays to accumulate tangents into
	TArray<FVector3f> VertexTangentXSum, VertexTangentYSum, VertexTangentZSum;
	VertexTangentXSum.AddZeroed(NumVertices);
	VertexTangentYSum.AddZeroed(NumVertices);
	VertexTangentZSum.AddZeroed(NumVertices);

	// For each vertex..
	for (int VertxIdx = 0; VertxIdx < NumVertices; VertxIdx++)
	{
		// Find relevant triangles for normal
		TArray<uint32> SmoothTris;
		VertToTriSmoothMap.MultiFind(VertxIdx, SmoothTris);

		for (int i = 0; i < SmoothTris.Num(); i++)
		{
			uint32 TriIdx = SmoothTris[i];
			VertexTangentZSum[VertxIdx] += FaceTangentZ[TriIdx];
		}

		// Find relevant triangles for tangents
		TArray<uint32> TangentTris;
		VertToTriMap.MultiFind(VertxIdx, TangentTris);

		for (int i = 0; i < TangentTris.Num(); i++)
		{
			uint32 TriIdx = TangentTris[i];
			VertexTangentXSum[VertxIdx] += FaceTangentX[TriIdx];
			VertexTangentYSum[VertxIdx] += FaceTangentY[TriIdx];
		}
	}

	// Finally, normalize tangents and build output arrays	
	for (int VertxIdx = 0; VertxIdx < NumVertices; VertxIdx++)
	{
		FVector3f& TangentX = VertexTangentXSum[VertxIdx];
		FVector3f& TangentY = VertexTangentYSum[VertxIdx];
		FVector3f& TangentZ = VertexTangentZSum[VertxIdx];

		TangentX.Normalize();
		//TangentY.Normalize();
		TangentZ.Normalize();

		// Use Gram-Schmidt orthogonalization to make sure X is orthonormal with Z
		TangentX -= TangentZ * (TangentZ | TangentX);
		TangentX.Normalize();
		TangentY.Normalize();

		MeshData.Normals[VertxIdx] = FVector{TangentZ};
		MeshData.Tangents[VertxIdx] = FVector{TangentX};
	}
}
