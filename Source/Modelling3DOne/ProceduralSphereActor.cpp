// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralSphereActor.h"


// Sets default values
AProceduralSphereActor::AProceduralSphereActor()
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
void AProceduralSphereActor::BeginPlay()
{
	Super::BeginPlay();
	GenerateSphere();
}

void AProceduralSphereActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	GenerateSphere();
}

void AProceduralSphereActor::GenerateSphere()
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

	// Create North Pole vertex (index 0)
	FVector NorthPole = FVector(0, 0, Radius);
	Vertices.Add(NorthPole);
	Normals.Add(NorthPole.GetSafeNormal());
	UVs.Add(FVector2D(0.5f, 1.0f));

	// Generate vertices for parallels (latitude circles)
	// Skip poles: start from parallel 1 to Parallels-1
	for (int32 ParallelIdx = 1; ParallelIdx < Parallels; ParallelIdx++)
	{
		// Angle from north pole (0 to PI)
		float Theta = PI * float(ParallelIdx) / float(Parallels);
		float SinTheta = FMath::Sin(Theta);
		float CosTheta = FMath::Cos(Theta);

		for (int32 MeridianIdx = 0; MeridianIdx < Meridians; MeridianIdx++)
		{
			// Angle around the sphere (0 to 2*PI)
			float Phi = 2.0f * PI * float(MeridianIdx) / float(Meridians);
			float SinPhi = FMath::Sin(Phi);
			float CosPhi = FMath::Cos(Phi);

			// Spherical to Cartesian coordinates
			FVector Position = FVector(
				Radius * SinTheta * CosPhi,  // X
				Radius * SinTheta * SinPhi,  // Y
				Radius * CosTheta            // Z
			);

			Vertices.Add(Position);
			Normals.Add(Position.GetSafeNormal());

			// UV mapping
			float U = float(MeridianIdx) / float(Meridians);
			float V = float(ParallelIdx) / float(Parallels);
			UVs.Add(FVector2D(U, V));
		}
	}

	// Create South Pole vertex (last vertex)
	FVector SouthPole = FVector(0, 0, -Radius);
	int32 SouthPoleIndex = Vertices.Num();
	Vertices.Add(SouthPole);
	Normals.Add(SouthPole.GetSafeNormal());
	UVs.Add(FVector2D(0.5f, 0.0f));

	// --- Generate Triangles ---

	// North Pole cap triangles
	for (int32 MeridianIdx = 0; MeridianIdx < Meridians; MeridianIdx++)
	{
		int32 Next = (MeridianIdx + 1) % Meridians;
		
		// Triangle: North Pole -> Next meridian -> Current meridian
		Triangles.Add(0);                    // North pole
		Triangles.Add(1 + Next);             // Next vertex on first parallel
		Triangles.Add(1 + MeridianIdx);      // Current vertex on first parallel
	}


	// Middle section: quads between parallels (each quad = 2 triangles)
	for (int32 ParallelIdx = 1; ParallelIdx < Parallels - 1; ParallelIdx++)
	{
		for (int32 MeridianIdx = 0; MeridianIdx < Meridians; MeridianIdx++)
		{
			int32 Next = (MeridianIdx + 1) % Meridians;

			// Calculate vertex indices
			int32 Current = 1 + (ParallelIdx - 1) * Meridians + MeridianIdx;
			int32 CurrentNext = 1 + (ParallelIdx - 1) * Meridians + Next;
			int32 Below = 1 + ParallelIdx * Meridians + MeridianIdx;
			int32 BelowNext = 1 + ParallelIdx * Meridians + Next;

			// First triangle of the quad
			Triangles.Add(Current);
			Triangles.Add(CurrentNext);
			Triangles.Add(Below);

			// Second triangle of the quad
			Triangles.Add(CurrentNext);
			Triangles.Add(BelowNext);
			Triangles.Add(Below);
		}
	}

	// South Pole cap triangles
	int32 LastParallelStart = 1 + (Parallels - 2) * Meridians;
	for (int32 MeridianIdx = 0; MeridianIdx < Meridians; MeridianIdx++)
	{
		int32 Next = (MeridianIdx + 1) % Meridians;

		// Triangle: Current meridian -> Next meridian -> South Pole
		Triangles.Add(LastParallelStart + MeridianIdx);
		Triangles.Add(LastParallelStart + Next);
		Triangles.Add(SouthPoleIndex);
	}

	// Create the mesh section
	ProceduralMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, 
	                                  VertexColors, Tangents, true);

	// Apply material if set
	if (SphereMaterial)
	{
		ProceduralMesh->SetMaterial(0, SphereMaterial);
	}
	
	// Enable collision
	ProceduralMesh->ContainsPhysicsTriMeshData(true);
}

void AProceduralSphereActor::CreateTriangle(TArray<FVector>& Vertices, TArray<int32>& Triangles,
	TArray<FVector>& Normals, TArray<FVector2D>& UVs, const FVector& V0, const FVector& V1, const FVector& V2)
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
