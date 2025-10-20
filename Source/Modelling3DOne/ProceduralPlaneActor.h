// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "ProceduralPlaneActor.generated.h"

UCLASS()
class MODELLING3DONE_API AProceduralPlaneActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AProceduralPlaneActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;


public:	
	// The procedural mesh component that will hold our geometry
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	UProceduralMeshComponent* ProceduralMesh;

	// Number of rows (lines) in the plane
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plane Parameters", meta = (ClampMin = "1"))
	int32 Nb_Lignes = 5;

	// Number of columns in the plane
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plane Parameters", meta = (ClampMin = "1"))
	int32 Nb_Colones = 5;

	// Size of each quad in the plane
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plane Parameters")
	float QuadSize = 100.0f;

	// Material to apply to the mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plane Parameters")
	UMaterialInterface* PlaneMaterial;

	// Function to generate the plane mesh
	UFUNCTION(BlueprintCallable, Category = "Mesh Generation")
	void GeneratePlane();

private:
	// Helper function to create a single triangle
	void CreateTriangle(TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FVector>& Normals, 
					   TArray<FVector2D>& UVs, const FVector& V0, const FVector& V1, const FVector& V2);
};

