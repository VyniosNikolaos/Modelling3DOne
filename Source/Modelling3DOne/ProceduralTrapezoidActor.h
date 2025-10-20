// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "ProceduralTrapezoidActor.generated.h"

UCLASS()
class MODELLING3DONE_API AProceduralTrapezoidActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AProceduralTrapezoidActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

public:
	// The procedural mesh component that will hold our geometry
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	UProceduralMeshComponent* ProceduralMesh;
	
	// Top base width (smaller parallel side)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trapezoid", meta=(ClampMin="0.1"))
	float TopWidth = 50.0f;
	
	// Bottom base width (larger parallel side)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trapezoid", meta=(ClampMin="0.1"))
	float BottomWidth = 100.0f;
	
	// Height of the trapezoid
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trapezoid", meta=(ClampMin="0.1"))
	float Height = 100.0f;
	
	// Depth/thickness of the trapezoid (extrusion)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trapezoid", meta=(ClampMin="0.1"))
	float Depth = 50.0f;

	// Material to apply to the mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trapezoid Parameters")
	UMaterialInterface* TrapezoidMaterial;

	// Show wireframe overlay
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowWireframe = false;
	
	// Function to generate the sphere mesh
	UFUNCTION(BlueprintCallable, Category = "Mesh Generation")
	void GenerateTrapezoid();
	
private:
	// Helper function to create a single triangle
	void CreateTriangle(TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FVector>& Normals, 
					   TArray<FVector2D>& UVs, const FVector& V0, const FVector& V1, const FVector& V2);
};
