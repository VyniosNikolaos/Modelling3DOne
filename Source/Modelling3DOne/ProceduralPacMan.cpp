// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralPacMan.h"


// Sets default values
AProceduralPacMan::AProceduralPacMan()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	// Create the procedural mesh component
	ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));
	RootComponent = ProceduralMesh;
	
	// Enable collision
	ProceduralMesh->bUseAsyncCooking = true;
}

// Called when the game starts or when spawned
void AProceduralPacMan::BeginPlay()
{
	Super::BeginPlay();
	GeneratePacMan();
}

void AProceduralPacMan::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	GeneratePacMan();
}

void AProceduralPacMan::GeneratePacMan()
{
	// Clear existing mesh
	ProceduralMesh->ClearAllMeshSections();

	// Arrays to hold mesh data
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;

	// Ensure minimum values
	int32 Parallels = FMath::Max(3, NumParallels);
	int32 Meridians = FMath::Max(3, NumMeridians);

	// Convert mouth angle to radians (half angle on each side)
	float HalfMouthAngleRad = FMath::DegreesToRadians(MouthAngleDegrees / 2.0f);

	// Center vertex for mouth (at origin)
	int32 CenterVertexIndex = Vertices.Num();
	Vertices.Add(FVector::ZeroVector);
	Normals.Add(FVector::ForwardVector);
	UVs.Add(FVector2D(0.5f, 0.5f));

	// Create North Pole vertex
	FVector NorthPole = FVector(0, 0, Radius);
	int32 NorthPoleIndex = Vertices.Num();
	Vertices.Add(NorthPole);
	Normals.Add(NorthPole.GetSafeNormal());
	UVs.Add(FVector2D(0.5f, 1.0f));

	// Generate vertices for parallels (latitude circles)
	for (int32 ParallelIdx = 1; ParallelIdx < Parallels; ParallelIdx++)
	{
		float Theta = PI * float(ParallelIdx) / float(Parallels);
		float SinTheta = FMath::Sin(Theta);
		float CosTheta = FMath::Cos(Theta);

		for (int32 MeridianIdx = 0; MeridianIdx < Meridians; MeridianIdx++)
		{
			float Phi = 2.0f * PI * float(MeridianIdx) / float(Meridians);
			
			// Skip vertices in the mouth region
			float PhiNormalized = Phi;
			if (PhiNormalized > PI)
				PhiNormalized = PhiNormalized - 2.0f * PI;
			
			if (FMath::Abs(PhiNormalized) < HalfMouthAngleRad)
				continue;

			float SinPhi = FMath::Sin(Phi);
			float CosPhi = FMath::Cos(Phi);

			FVector Position = FVector(
				Radius * SinTheta * CosPhi,
				Radius * SinTheta * SinPhi,
				Radius * CosTheta
			);

			Vertices.Add(Position);
			Normals.Add(Position.GetSafeNormal());

			float U = float(MeridianIdx) / float(Meridians);
			float V = float(ParallelIdx) / float(Parallels);
			UVs.Add(FVector2D(U, V));
		}
	}

	// Create South Pole vertex
	FVector SouthPole = FVector(0, 0, -Radius);
	int32 SouthPoleIndex = Vertices.Num();
	Vertices.Add(SouthPole);
	Normals.Add(SouthPole.GetSafeNormal());
	UVs.Add(FVector2D(0.5f, 0.0f));

	// --- Helper function to get vertex index ---
	auto GetVertexIndex = [&](int32 ParallelIdx, int32 MeridianIdx) -> int32
	{
		if (ParallelIdx == 0)
			return NorthPoleIndex;
		if (ParallelIdx == Parallels)
			return SouthPoleIndex;

		float Phi = 2.0f * PI * float(MeridianIdx) / float(Meridians);
		float PhiNormalized = Phi;
		if (PhiNormalized > PI)
			PhiNormalized = PhiNormalized - 2.0f * PI;

		if (FMath::Abs(PhiNormalized) < HalfMouthAngleRad)
			return -1;

		int32 Index = NorthPoleIndex + 1;
		for (int32 p = 1; p < ParallelIdx; p++)
		{
			for (int32 m = 0; m < Meridians; m++)
			{
				float TestPhi = 2.0f * PI * float(m) / float(Meridians);
				float TestPhiNorm = TestPhi > PI ? TestPhi - 2.0f * PI : TestPhi;
				if (FMath::Abs(TestPhiNorm) >= HalfMouthAngleRad)
					Index++;
			}
		}

		for (int32 m = 0; m < MeridianIdx; m++)
		{
			float TestPhi = 2.0f * PI * float(m) / float(Meridians);
			float TestPhiNorm = TestPhi > PI ? TestPhi - 2.0f * PI : TestPhi;
			if (FMath::Abs(TestPhiNorm) >= HalfMouthAngleRad)
				Index++;
		}

		return Index;
	};

	// --- Generate Triangles for the sphere surface ---

	// North Pole cap triangles
	for (int32 MeridianIdx = 0; MeridianIdx < Meridians; MeridianIdx++)
	{
		int32 Current = GetVertexIndex(1, MeridianIdx);
		int32 Next = GetVertexIndex(1, (MeridianIdx + 1) % Meridians);

		if (Current != -1 && Next != -1)
		{
			Triangles.Add(NorthPoleIndex);
			Triangles.Add(Next);
			Triangles.Add(Current);
		}
	}

	// Middle section: quads between parallels
	for (int32 ParallelIdx = 1; ParallelIdx < Parallels - 1; ParallelIdx++)
	{
		for (int32 MeridianIdx = 0; MeridianIdx < Meridians; MeridianIdx++)
		{
			int32 NextMeridianIdx = (MeridianIdx + 1) % Meridians;

			int32 Current = GetVertexIndex(ParallelIdx, MeridianIdx);
			int32 CurrentNext = GetVertexIndex(ParallelIdx, NextMeridianIdx);
			int32 Below = GetVertexIndex(ParallelIdx + 1, MeridianIdx);
			int32 BelowNext = GetVertexIndex(ParallelIdx + 1, NextMeridianIdx);

			if (Current != -1 && CurrentNext != -1 && Below != -1 && BelowNext != -1)
			{
				Triangles.Add(Current);
				Triangles.Add(CurrentNext);
				Triangles.Add(Below);

				Triangles.Add(CurrentNext);
				Triangles.Add(BelowNext);
				Triangles.Add(Below);
			}
		}
	}

	// South Pole cap triangles
	for (int32 MeridianIdx = 0; MeridianIdx < Meridians; MeridianIdx++)
	{
		int32 Current = GetVertexIndex(Parallels - 1, MeridianIdx);
		int32 Next = GetVertexIndex(Parallels - 1, (MeridianIdx + 1) % Meridians);

		if (Current != -1 && Next != -1)
		{
			Triangles.Add(Current);
			Triangles.Add(Next);
			Triangles.Add(SouthPoleIndex);
		}
	}

	// --- Create mouth wall vertices and triangles ---
	
	// Calculate normal for upper wall (perpendicular to the upper mouth edge)
	// The upper edge is at angle +HalfMouthAngleRad, so normal points perpendicular inward
	FVector UpperWallNormal = FVector(
		-FMath::Sin(HalfMouthAngleRad),  // Perpendicular to the edge
		FMath::Cos(HalfMouthAngleRad),
		0
	).GetSafeNormal();
	
	// Calculate normal for lower wall
	FVector LowerWallNormal = FVector(
		-FMath::Sin(-HalfMouthAngleRad),
		FMath::Cos(-HalfMouthAngleRad),
		0
	).GetSafeNormal();
	
	// Create vertices along the upper mouth wall edge
	TArray<int32> UpperMouthWallVertices;
	
	// Add north pole for upper wall (duplicate with different normal)
	int32 UpperNorthPole = Vertices.Num();
	Vertices.Add(NorthPole);
	Normals.Add(UpperWallNormal);
	UVs.Add(FVector2D(0.0f, 1.0f));
	UpperMouthWallVertices.Add(UpperNorthPole);
	
	for (int32 ParallelIdx = 1; ParallelIdx < Parallels; ParallelIdx++)
	{
		float Theta = PI * float(ParallelIdx) / float(Parallels);
		float SinTheta = FMath::Sin(Theta);
		float CosTheta = FMath::Cos(Theta);
		
		// Upper edge is at +HalfMouthAngleRad
		FVector Position = FVector(
			Radius * SinTheta * FMath::Cos(HalfMouthAngleRad),
			Radius * SinTheta * FMath::Sin(HalfMouthAngleRad),
			Radius * CosTheta
		);
		
		int32 VertIdx = Vertices.Num();
		Vertices.Add(Position);
		Normals.Add(UpperWallNormal);
		UVs.Add(FVector2D(0.0f, 1.0f - float(ParallelIdx) / float(Parallels)));
		UpperMouthWallVertices.Add(VertIdx);
	}
	
	// Add south pole for upper wall
	int32 UpperSouthPole = Vertices.Num();
	Vertices.Add(SouthPole);
	Normals.Add(UpperWallNormal);
	UVs.Add(FVector2D(0.0f, 0.0f));
	UpperMouthWallVertices.Add(UpperSouthPole);
	
	// Create vertices along the lower mouth wall edge
	TArray<int32> LowerMouthWallVertices;
	
	// Add north pole for lower wall
	int32 LowerNorthPole = Vertices.Num();
	Vertices.Add(NorthPole);
	Normals.Add(LowerWallNormal);
	UVs.Add(FVector2D(1.0f, 1.0f));
	LowerMouthWallVertices.Add(LowerNorthPole);
	
	for (int32 ParallelIdx = 1; ParallelIdx < Parallels; ParallelIdx++)
	{
		float Theta = PI * float(ParallelIdx) / float(Parallels);
		float SinTheta = FMath::Sin(Theta);
		float CosTheta = FMath::Cos(Theta);
		
		// Lower edge is at -HalfMouthAngleRad
		FVector Position = FVector(
			Radius * SinTheta * FMath::Cos(-HalfMouthAngleRad),
			Radius * SinTheta * FMath::Sin(-HalfMouthAngleRad),
			Radius * CosTheta
		);
		
		int32 VertIdx = Vertices.Num();
		Vertices.Add(Position);
		Normals.Add(LowerWallNormal);
		UVs.Add(FVector2D(1.0f, 1.0f - float(ParallelIdx) / float(Parallels)));
		LowerMouthWallVertices.Add(VertIdx);
	}
	
	// Add south pole for lower wall
	int32 LowerSouthPole = Vertices.Num();
	Vertices.Add(SouthPole);
	Normals.Add(LowerWallNormal);
	UVs.Add(FVector2D(1.0f, 0.0f));
	LowerMouthWallVertices.Add(LowerSouthPole);
	
	// Create center vertex duplicates for each wall
	int32 CenterUpperWall = Vertices.Num();
	Vertices.Add(FVector::ZeroVector);
	Normals.Add(UpperWallNormal);
	UVs.Add(FVector2D(0.5f, 0.5f));
	
	int32 CenterLowerWall = Vertices.Num();
	Vertices.Add(FVector::ZeroVector);
	Normals.Add(LowerWallNormal);
	UVs.Add(FVector2D(0.5f, 0.5f));
	
	// Create triangles for upper mouth wall (connecting to center)
	for (int32 i = 0; i < UpperMouthWallVertices.Num() - 1; i++)
	{
		Triangles.Add(CenterUpperWall);
		Triangles.Add(UpperMouthWallVertices[i]);
		Triangles.Add(UpperMouthWallVertices[i + 1]);
	}
	
	// Create triangles for lower mouth wall (connecting to center, reversed winding)
	for (int32 i = 0; i < LowerMouthWallVertices.Num() - 1; i++)
	{
		Triangles.Add(CenterLowerWall);
		Triangles.Add(LowerMouthWallVertices[i + 1]);
		Triangles.Add(LowerMouthWallVertices[i]);
	}


	// Create the mesh section
	ProceduralMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, 
									  VertexColors, Tangents, true);

	// Apply material if set
	if (PacManMaterial)
	{
		ProceduralMesh->SetMaterial(0, PacManMaterial);
	}
	
	// Enable collision
	ProceduralMesh->ContainsPhysicsTriMeshData(true);
}

void AProceduralPacMan::CreateTriangle(TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FVector>& Normals,
	TArray<FVector2D>& UVs, const FVector& V0, const FVector& V1, const FVector& V2)
{
	// Add vertices
	int32 StartIndex = Vertices.Num();
	Vertices.Add(V0);
	Vertices.Add(V1);
	Vertices.Add(V2);

	// Add triangle indices (counter-clockwise for proper face orientation)
	Triangles.Add(StartIndex);
	Triangles.Add(StartIndex + 1);
	Triangles.Add(StartIndex + 2);

	// Calculate normal for the triangle
	FVector Normal = FVector::CrossProduct(V1 - V0, V2 - V0).GetSafeNormal();
	Normals.Add(Normal);
	Normals.Add(Normal);
	Normals.Add(Normal);

	// Add UV coordinates (basic planar mapping)
	UVs.Add(FVector2D(0, 0));
	UVs.Add(FVector2D(1, 0));
	UVs.Add(FVector2D(0, 1));
}



