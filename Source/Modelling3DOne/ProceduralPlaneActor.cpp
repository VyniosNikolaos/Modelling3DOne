// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralPlaneActor.h"


// Sets default values
AProceduralPlaneActor::AProceduralPlaneActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create the procedural mesh component
	ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));
	RootComponent = ProceduralMesh;
	
	// Enable collision
	ProceduralMesh->bUseAsyncCooking = true;
}


// Called when the game starts or when spawned
void AProceduralPlaneActor::BeginPlay()
{
	Super::BeginPlay();
	GeneratePlane();
}


void AProceduralPlaneActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	GeneratePlane();
}


void AProceduralPlaneActor::CreateTriangle(TArray<FVector>& Vertices, TArray<int32>& Triangles, 
                                          TArray<FVector>& Normals, TArray<FVector2D>& UVs, 
                                          const FVector& V0, const FVector& V1, const FVector& V2)
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

void AProceduralPlaneActor::GeneratePlane()
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

	// Generate a grid of quads, each made of 2 triangles
	// The plane will be in the XY plane (horizontal)
	for (int32 Row = 0; Row < Nb_Lignes; Row++)
	{
		for (int32 Col = 0; Col < Nb_Colones; Col++)
		{
			// Calculate the four corners of the quad
			FVector BottomLeft = FVector(Col * QuadSize, Row * QuadSize, 0);
			FVector BottomRight = FVector((Col + 1) * QuadSize, Row * QuadSize, 0);
			FVector TopLeft = FVector(Col * QuadSize, (Row + 1) * QuadSize, 0);
			FVector TopRight = FVector((Col + 1) * QuadSize, (Row + 1) * QuadSize, 0);

			// Create first triangle (Bottom-Left, Top-Left, Bottom-Right)
			CreateTriangle(Vertices, Triangles, Normals, UVs, 
						  BottomLeft, TopLeft, BottomRight);

			// Create second triangle (Bottom-Right, Top-Left, Top-Right)
			CreateTriangle(Vertices, Triangles, Normals, UVs, 
						  BottomRight, TopLeft, TopRight);
		}
	}

	// Create the mesh section
	ProceduralMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, 
	                                  VertexColors, Tangents, true);

	// Apply material if set
	if (PlaneMaterial)
	{
		ProceduralMesh->SetMaterial(0, PlaneMaterial);
	}

	// Enable collision
	ProceduralMesh->ContainsPhysicsTriMeshData(true);
}
