// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralConeActor.h"

AProceduralConeActor::AProceduralConeActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	// Create the procedural mesh component
	ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));
	RootComponent = ProceduralMesh;
	
	// Enable collision
	ProceduralMesh->bUseAsyncCooking = true;
}

void AProceduralConeActor::BeginPlay()
{
	Super::BeginPlay();
	GenerateCone();
}

void AProceduralConeActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	GenerateCone();
}

void AProceduralConeActor::GenerateCone()
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
	int32 Meridians = FMath::Max(3, NumMeridians);
	float HalfHeight = Height * 0.5f;

	// Clamp TopRadius to be non-negative and not larger than BottomRadius
	float SafeTopRadius = FMath::Max(0.0f, TopRadius);
	float SafeBottomRadius = FMath::Max(0.0f, BottomRadius);

	// Determine if this is a complete cone (apex) or truncated cone (frustum)
	bool bIsTruncated = SafeTopRadius > 0.01f;

		// --- PART 1: Generate Cone Body ---
	
	// Create vertices for top and bottom circles
	for (int32 MeridianIdx = 0; MeridianIdx < Meridians; MeridianIdx++)
	{
		float Angle = 2.0f * PI * float(MeridianIdx) / float(Meridians);
		float CosAngle = FMath::Cos(Angle);
		float SinAngle = FMath::Sin(Angle);

		// Top circle vertex
		FVector TopVertex = FVector(SafeTopRadius * CosAngle, SafeTopRadius * SinAngle, HalfHeight);
		Vertices.Add(TopVertex);
		
		// Calculate cone surface normal (not vertical, slopes with the cone)
		// The normal is perpendicular to the cone surface
		float RadiusDiff = SafeBottomRadius - SafeTopRadius;
		FVector SlopeDirection = FVector(CosAngle, SinAngle, RadiusDiff / Height).GetSafeNormal();
		Normals.Add(SlopeDirection);
		
		// UV for cone body
		float U = float(MeridianIdx) / float(Meridians);
		UVs.Add(FVector2D(U, 1.0f)); // Top

		// Bottom circle vertex
		FVector BottomVertex = FVector(SafeBottomRadius * CosAngle, SafeBottomRadius * SinAngle, -HalfHeight);
		Vertices.Add(BottomVertex);
		Normals.Add(SlopeDirection);
		UVs.Add(FVector2D(U, 0.0f)); // Bottom
	}

	// Generate triangles for cone body (quads made of 2 triangles)
	for (int32 MeridianIdx = 0; MeridianIdx < Meridians; MeridianIdx++)
	{
		int32 NextIdx = (MeridianIdx + 1) % Meridians;

		// Vertex indices (alternating top/bottom pattern)
		int32 TopCurrent = MeridianIdx * 2;
		int32 BottomCurrent = MeridianIdx * 2 + 1;
		int32 TopNext = NextIdx * 2;
		int32 BottomNext = NextIdx * 2 + 1;

		// First triangle (Top-Current, Top-Next, Bottom-Current)
		Triangles.Add(TopCurrent);
		Triangles.Add(TopNext);
		Triangles.Add(BottomCurrent);

		// Second triangle (Top-Next, Bottom-Next, Bottom-Current)
		Triangles.Add(TopNext);
		Triangles.Add(BottomNext);
		Triangles.Add(BottomCurrent);
	}

	// --- PART 2: Generate Top Cap (only if truncated) ---
	
	if (bIsTruncated)
	{
		int32 TopCenterIndex = Vertices.Num();
		
		// Add center vertex for top disk
		Vertices.Add(FVector(0, 0, HalfHeight));
		Normals.Add(FVector(0, 0, 1)); // Normal points up
		UVs.Add(FVector2D(0.5f, 0.5f)); // Center of UV space

		// Add vertices around the top disk edge
		for (int32 MeridianIdx = 0; MeridianIdx < Meridians; MeridianIdx++)
		{
			float Angle = 2.0f * PI * float(MeridianIdx) / float(Meridians);
			float CosAngle = FMath::Cos(Angle);
			float SinAngle = FMath::Sin(Angle);

			FVector TopEdgeVertex = FVector(SafeTopRadius * CosAngle, SafeTopRadius * SinAngle, HalfHeight);
			Vertices.Add(TopEdgeVertex);
			Normals.Add(FVector(0, 0, 1)); // Normal points up
			
			// UV mapped in circular pattern
			float U = 0.5f + 0.5f * CosAngle;
			float V = 0.5f + 0.5f * SinAngle;
			UVs.Add(FVector2D(U, V));
		}

		// Create triangular fan for top disk
		for (int32 MeridianIdx = 0; MeridianIdx < Meridians; MeridianIdx++)
		{
			int32 NextIdx = (MeridianIdx + 1) % Meridians;
			
			// Triangle: Center -> Next edge -> Current edge
			Triangles.Add(TopCenterIndex);
			Triangles.Add(TopCenterIndex + 1 + NextIdx);
			Triangles.Add(TopCenterIndex + 1 + MeridianIdx);
		}
	}

	// --- PART 3: Generate Bottom Cap ---
	
	int32 BottomCenterIndex = Vertices.Num();
	
	// Add center vertex for bottom disk
	Vertices.Add(FVector(0, 0, -HalfHeight));
	Normals.Add(FVector(0, 0, -1)); // Normal points down
	UVs.Add(FVector2D(0.5f, 0.5f)); // Center of UV space

	// Add vertices around the bottom disk edge
	for (int32 MeridianIdx = 0; MeridianIdx < Meridians; MeridianIdx++)
	{
		float Angle = 2.0f * PI * float(MeridianIdx) / float(Meridians);
		float CosAngle = FMath::Cos(Angle);
		float SinAngle = FMath::Sin(Angle);

		FVector BottomEdgeVertex = FVector(SafeBottomRadius * CosAngle, SafeBottomRadius * SinAngle, -HalfHeight);
		Vertices.Add(BottomEdgeVertex);
		Normals.Add(FVector(0, 0, -1)); // Normal points down
		
		// UV mapped in circular pattern
		float U = 0.5f + 0.5f * CosAngle;
		float V = 0.5f + 0.5f * SinAngle;
		UVs.Add(FVector2D(U, V));
	}

	// Create triangular fan for bottom disk (reversed winding for correct facing)
	for (int32 MeridianIdx = 0; MeridianIdx < Meridians; MeridianIdx++)
	{
		int32 NextIdx = (MeridianIdx + 1) % Meridians;
		
		// Triangle: Center -> Current edge -> Next edge (reversed for downward facing)
		Triangles.Add(BottomCenterIndex);
		Triangles.Add(BottomCenterIndex + 1 + MeridianIdx);  // Current
		Triangles.Add(BottomCenterIndex + 1 + NextIdx);  
	}

	// Create the mesh section
	ProceduralMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, 
	                                  VertexColors, Tangents, true);

	// Apply material if set
	if (ConeMaterial)
	{
		ProceduralMesh->SetMaterial(0, ConeMaterial);
	}

	// Enable collision
	ProceduralMesh->ContainsPhysicsTriMeshData(true);
}


void AProceduralConeActor::CreateTriangle(TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FVector>& Normals,
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
