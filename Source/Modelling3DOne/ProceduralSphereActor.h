// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "ProceduralSphereActor.generated.h"

UCLASS()
class MODELLING3DONE_API AProceduralSphereActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AProceduralSphereActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

public:
	// The procedural mesh component that will hold our geometry
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	UProceduralMeshComponent* ProceduralMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sphere")
	float Radius = 100.0f;
 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sphere", meta=(ClampMin="3"))
	int32 NumParallels = 16;
 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sphere", meta=(ClampMin="3"))
	int32 NumMeridians = 32;

	
	// Material to apply to the mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sphere Parameters")
	UMaterialInterface* SphereMaterial;

	// Show wireframe overlay
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowWireframe = false;
	
	// Function to generate the sphere mesh
	UFUNCTION(BlueprintCallable, Category = "Mesh Generation")
	void GenerateSphere();
	
private:
	// Helper function to create a single triangle
	void CreateTriangle(TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FVector>& Normals, 
					   TArray<FVector2D>& UVs, const FVector& V0, const FVector& V1, const FVector& V2);
};