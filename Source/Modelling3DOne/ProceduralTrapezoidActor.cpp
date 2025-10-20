// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralTrapezoidActor.h"


// Sets default values
AProceduralTrapezoidActor::AProceduralTrapezoidActor()
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
void AProceduralTrapezoidActor::BeginPlay()
{
	Super::BeginPlay();
	GenerateTrapezoid();
}

void AProceduralTrapezoidActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	GenerateTrapezoid();
}

void AProceduralTrapezoidActor::GenerateTrapezoid()
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

	// Calculate half dimensions for centering
	float HalfTopWidth = TopWidth * 0.5f;
	float HalfBottomWidth = BottomWidth * 0.5f;
	float HalfHeight = Height * 0.5f;
	float HalfDepth = Depth * 0.5f;

	// --- Define the 8 vertices of the trapezoid prism ---
	
	// Front face (4 vertices forming a trapezoid)
	FVector FrontTopLeft = FVector(-HalfTopWidth, -HalfDepth, HalfHeight);
	FVector FrontTopRight = FVector(HalfTopWidth, -HalfDepth, HalfHeight);
	FVector FrontBottomLeft = FVector(-HalfBottomWidth, -HalfDepth, -HalfHeight);
	FVector FrontBottomRight = FVector(HalfBottomWidth, -HalfDepth, -HalfHeight);

	// Back face (4 vertices forming a trapezoid)
	FVector BackTopLeft = FVector(-HalfTopWidth, HalfDepth, HalfHeight);
	FVector BackTopRight = FVector(HalfTopWidth, HalfDepth, HalfHeight);
	FVector BackBottomLeft = FVector(-HalfBottomWidth, HalfDepth, -HalfHeight);
	FVector BackBottomRight = FVector(HalfBottomWidth, HalfDepth, -HalfHeight);

	// --- FRONT FACE ---
	int32 FrontStartIdx = Vertices.Num();
	Vertices.Append({FrontTopLeft, FrontTopRight, FrontBottomRight, FrontBottomLeft});
	FVector FrontNormal = FVector(0, -1, 0);
	for (int32 i = 0; i < 4; i++)
	{
		Normals.Add(FrontNormal);
	}
	UVs.Append({
		FVector2D(0, 1),
		FVector2D(1, 1),
		FVector2D(1, 0),
		FVector2D(0, 0)
	});
	
	// Front face triangles
	Triangles.Add(FrontStartIdx + 0);
	Triangles.Add(FrontStartIdx + 1);
	Triangles.Add(FrontStartIdx + 2);
	
	Triangles.Add(FrontStartIdx + 0);
	Triangles.Add(FrontStartIdx + 2);
	Triangles.Add(FrontStartIdx + 3);

	// --- BACK FACE ---
	int32 BackStartIdx = Vertices.Num();
	Vertices.Append({BackTopRight, BackTopLeft, BackBottomLeft, BackBottomRight});
	FVector BackNormal = FVector(0, 1, 0);
	for (int32 i = 0; i < 4; i++)
	{
		Normals.Add(BackNormal);
	}
	UVs.Append({
		FVector2D(0, 1),
		FVector2D(1, 1),
		FVector2D(1, 0),
		FVector2D(0, 0)
	});
	
	// Back face triangles
	Triangles.Add(BackStartIdx + 0);
	Triangles.Add(BackStartIdx + 1);
	Triangles.Add(BackStartIdx + 2);
	
	Triangles.Add(BackStartIdx + 0);
	Triangles.Add(BackStartIdx + 2);
	Triangles.Add(BackStartIdx + 3);

	// --- TOP FACE (Rectangle) ---
	int32 TopStartIdx = Vertices.Num();
	Vertices.Append({BackTopLeft, BackTopRight, FrontTopRight, FrontTopLeft});
	FVector TopNormal = FVector(0, 0, 1);
	for (int32 i = 0; i < 4; i++)
	{
		Normals.Add(TopNormal);
	}
	UVs.Append({
		FVector2D(0, 1),
		FVector2D(1, 1),
		FVector2D(1, 0),
		FVector2D(0, 0)
	});
	
	// Top face triangles
	Triangles.Add(TopStartIdx + 0);
	Triangles.Add(TopStartIdx + 1);
	Triangles.Add(TopStartIdx + 2);
	
	Triangles.Add(TopStartIdx + 0);
	Triangles.Add(TopStartIdx + 2);
	Triangles.Add(TopStartIdx + 3);

	// --- BOTTOM FACE (Rectangle) ---
	int32 BottomStartIdx = Vertices.Num();
	Vertices.Append({FrontBottomLeft, FrontBottomRight, BackBottomRight, BackBottomLeft});
	FVector BottomNormal = FVector(0, 0, -1);
	for (int32 i = 0; i < 4; i++)
	{
		Normals.Add(BottomNormal);
	}
	UVs.Append({
		FVector2D(0, 0),
		FVector2D(1, 0),
		FVector2D(1, 1),
		FVector2D(0, 1)
	});
	
	// Bottom face triangles
	Triangles.Add(BottomStartIdx + 0);
	Triangles.Add(BottomStartIdx + 1);
	Triangles.Add(BottomStartIdx + 2);
	
	Triangles.Add(BottomStartIdx + 0);
	Triangles.Add(BottomStartIdx + 2);
	Triangles.Add(BottomStartIdx + 3);

	// --- LEFT SLANTED FACE ---
	int32 LeftStartIdx = Vertices.Num();
	Vertices.Append({BackTopLeft, FrontTopLeft, FrontBottomLeft, BackBottomLeft});
	
	// Calculate normal for slanted face
	FVector LeftEdge1 = FrontTopLeft - BackTopLeft;
	FVector LeftEdge2 = FrontBottomLeft - BackTopLeft;
	FVector LeftNormal = FVector::CrossProduct(LeftEdge2, LeftEdge1).GetSafeNormal();
	
	for (int32 i = 0; i < 4; i++)
	{
		Normals.Add(LeftNormal);
	}
	UVs.Append({
		FVector2D(0, 1),
		FVector2D(1, 1),
		FVector2D(1, 0),
		FVector2D(0, 0)
	});
	
	// Left face triangles
	Triangles.Add(LeftStartIdx + 0);
	Triangles.Add(LeftStartIdx + 1);
	Triangles.Add(LeftStartIdx + 2);
	
	Triangles.Add(LeftStartIdx + 0);
	Triangles.Add(LeftStartIdx + 2);
	Triangles.Add(LeftStartIdx + 3);

	// --- RIGHT SLANTED FACE ---
	int32 RightStartIdx = Vertices.Num();
	Vertices.Append({FrontTopRight, BackTopRight, BackBottomRight, FrontBottomRight});
	
	// Calculate normal for slanted face
	FVector RightEdge1 = BackTopRight - FrontTopRight;
	FVector RightEdge2 = BackBottomRight - FrontTopRight;
	FVector RightNormal = FVector::CrossProduct(RightEdge2, RightEdge1).GetSafeNormal();
	
	for (int32 i = 0; i < 4; i++)
	{
		Normals.Add(RightNormal);
	}
	UVs.Append({
		FVector2D(0, 1),
		FVector2D(1, 1),
		FVector2D(1, 0),
		FVector2D(0, 0)
	});
	
	// Right face triangles
	Triangles.Add(RightStartIdx + 0);
	Triangles.Add(RightStartIdx + 1);
	Triangles.Add(RightStartIdx + 2);
	
	Triangles.Add(RightStartIdx + 0);
	Triangles.Add(RightStartIdx + 2);
	Triangles.Add(RightStartIdx + 3);

	// Create the mesh section
	ProceduralMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, 
									  VertexColors, Tangents, true);

	// Apply material if set
	if (TrapezoidMaterial)
	{
		ProceduralMesh->SetMaterial(0, TrapezoidMaterial);
	}
	
	// Enable collision
	ProceduralMesh->ContainsPhysicsTriMeshData(true);
}

void AProceduralTrapezoidActor::CreateTriangle(TArray<FVector>& Vertices, TArray<int32>& Triangles,
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




