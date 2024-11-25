// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NavMeshRenderer.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGenericNavMeshRendererSignature);

class UCustomMeshComponent;
struct FCustomMeshTriangle;

/**
 * 
 */
UCLASS()
class NAVMESHRUNTIMERENDER_API ANavMeshRenderer : public AActor
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FGenericNavMeshRendererSignature OnNavMeshLoaded;

	ANavMeshRenderer();
	virtual void OnConstruction(const FTransform& transform) override;
	virtual void BeginPlay() override;
	/*
	* Returns the rendered component.
	*/UFUNCTION(BlueprintCallable, Category = "NavMesh Render")
	UCustomMeshComponent* GetNavMeshRender() const;
	/*
	* Updates the render to fit the world's NavMesh.
	* WARNING: If done during runtime this will cause momentary freezing
	*/UFUNCTION(BlueprintCallable, CallInEditor, Category = "NavMesh Render")
	void UpdateWorldTris();
	/*
	* Deletes the render's tris.
	*/UFUNCTION(BlueprintCallable, CallInEditor, Category = "NavMesh Render")
	void ClearWorldTris();
	/*
	* This is extremely slot but does provide minimal impact to performance.
	*/UFUNCTION(BlueprintCallable, Category = "NavMesh Render")
	void LazyUpdateWorldTris(ARecastNavMesh* navMesh, int numOfTiles, int currentTile = 0);
private:
	UFUNCTION()
	void UpdateNavMeshRender();

	/*
	* Note that UVs are not generated for the nav mesh. World aligned or solid materials are recommended here.
	*/UPROPERTY(EditAnywhere, Category = "NavMesh Render")
	TObjectPtr<UMaterialInterface> NavMeshMaterial;
	/*
	* This is what's rendered into the world.
	*/UPROPERTY(EditAnywhere, Category = "Components")
	TObjectPtr<UCustomMeshComponent> NavMeshRender;
	/*
	* Variable housing the navmesh's tris.
	*/UPROPERTY()
	TArray<FCustomMeshTriangle> LoadedTris;
	UPROPERTY(VisibleAnywhere, Category = "NavMesh Render")
	int NumberOfTris = LoadedTris.Num();
};
