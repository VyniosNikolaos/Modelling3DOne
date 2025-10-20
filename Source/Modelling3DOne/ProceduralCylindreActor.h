// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "ProceduralCylindreActor.generated.h"

UCLASS()
class MODELLING3DONE_API AProceduralCylindreActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AProceduralCylindreActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

public:
	// The procedural mesh component that will hold our geometry
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	UProceduralMeshComponent* ProceduralMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cylinder")
	float Radius = 50.0f;
 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cylinder")
	float Height = 200.0f;
 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cylinder", meta=(ClampMin="3"))
	int32 NumMeridians = 32;

	
	// Material to apply to the mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cone Parameters")
	UMaterialInterface* CylinderMaterial;

	// Show wireframe overlay
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowWireframe = false;

	// Function to generate the plane mesh
	UFUNCTION(BlueprintCallable, Category = "Mesh Generation")
	void GenerateCylinder();

private:
	// Helper function to create a single triangle
	void CreateTriangle(TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FVector>& Normals, 
					   TArray<FVector2D>& UVs, const FVector& V0, const FVector& V1, const FVector& V2);
};

