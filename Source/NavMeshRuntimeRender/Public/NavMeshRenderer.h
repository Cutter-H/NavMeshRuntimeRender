// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NavMeshRenderer.generated.h"

class UDynamicNavMeshRendererComponent;
class UCustomMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNavMeshRenderGenericSignature);

/**
 * 
 */
UCLASS(NotBlueprintable, Placeable, meta = (BlueprintSpawnableComponent))
class NAVMESHRUNTIMERENDER_API ANavMeshRenderer : public AActor
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FOnNavMeshRenderGenericSignature OnMeshUpdate;
	ANavMeshRenderer();
	virtual void OnConstruction(const FTransform& transform) override;
	/*
	* Returns the rendered component.
	*/UFUNCTION(BlueprintCallable, Category = "NavMesh Render")
	const UDynamicMeshComponent* GetNavMeshRender() const;
	/*
	* Returns the UV coordinate at the given locatino.
	*/UFUNCTION(BlueprintCallable, Category = "NavMesh Render")
	FVector2D GetUV_Coordinate(const FVector& location) const;
	/*
	* Updates the render to fit the world's NavMesh.
	* WARNING: If done during runtime this will cause momentary freezing
	*/UFUNCTION(BlueprintCallable, CallInEditor, Category = "NavMesh Render")
	void UpdateMesh();
	/*
	* Deletes the render's tris.
	*/UFUNCTION(BlueprintCallable, Category = "NavMesh Render", meta=(DisplayName = "Clear Mesh"))
	void ClearMesh();
	/*
	* Saves the currently rendered mesh as a Static Mesh Asset.
	*/UFUNCTION(CallInEditor, Category = "NavMesh Render", meta = (DisplayName = "Save Mesh"))
	void z_SaveMesh();
	/*
	* Where to save the mesh at. Empty will save it inside the Content folder.
	*/UPROPERTY(EditAnywhere, Category = "NavMesh Render|Saving", meta=(DisplayName="Save Folder: /Game/"))
	FString SaveLocation;
	/*
	* The name of the Static Mesh Asset this will be saved as.
	*/UPROPERTY(EditAnywhere, Category = "NavMesh Render|Saving", meta = (DisplayName = "Asset Name:"))
	FString SaveName = FString("SM_NavMesh");

private:
	/*
	* Assigns a new vertex along with UVs, normals, etc.
	*/UFUNCTION()
	int AssignNewVert(const FVector& location);
	/*
	* Calculates a UV coordinate for the given locaiton.
	*/UFUNCTION()
	FVector2f UVCoord(const FVector& location) const;
	UFUNCTION()
	const ARecastNavMesh* GetNavMesh() const;
	/*
	* Raises/Lowers the rendered mesh. 
	* This does not affect UV allocation for floors.
	*/UPROPERTY(EditAnywhere, Category = "NavMesh Render")
	int ZOffset = 0.f;
	/*
	* Debug value that shows the number of floors.
	*/UPROPERTY(VisibleAnywhere, Category = "NavMesh Render")
	int NumberOfFloors = -1;
	/*
	* Debug value to show the number of verts.
	*/UPROPERTY(VisibleAnywhere, Category = "NavMesh Render")
	int NumberOfVerts = -1;
	/*
	* Debug value to show the number of tris.
	*/UPROPERTY(VisibleAnywhere, Category = "NavMesh Render")
	int NumberOfTris = -1;
	/*
	* Material used for the floor debug.
	*/UPROPERTY(EditAnywhere, Category = "NavMesh Render|UVs", meta = (DisplayThumbnail = "false"))
	TObjectPtr<UMaterialInterface> FloorDebugMaterial;
	/*
	* Debug to show where floors are sectioned.
	*/UPROPERTY(EditAnywhere, Category = "NavMesh Render|UVs")
	bool bShowFloorDebug = false;
	/*
	* The component that renders the Navigation Mesh.
	*/UPROPERTY()
	TObjectPtr<UCustomMeshComponent> FloorDebug;
	/*
	* The UVs will be separated by floors
	*/UPROPERTY(EditAnywhere, Category = "NavMesh Render|UVs", meta = (UIMin = 0.f, UIMax = 0.f))
	TArray<float> AdditionalFloorHeights;
	/*
	* Space between the floors on UVs.
	*/UPROPERTY(EditAnywhere, Category = "NavMesh Render|UVs", meta = (UIMin = 0.f, UIMax = 1.f, ClampMin = 0.f, ClampMax = 1.f))
	float UV_IslandMargins = 0.f;
	/*
	* Material to be used on the renderer.
	*/UPROPERTY(EditAnywhere, Category = "NavMesh Render", meta = (DisplayThumbnail = "false"))
	TObjectPtr<UMaterialInterface> NavMeshMaterial;
	/*
	* The component that renders the Navigation Mesh.
	*/UPROPERTY()
	TObjectPtr<UDynamicNavMeshRendererComponent> DynamicNavMeshRender;
	/*
	* The negative corner of the nav mesh. This is used for UVs.
	*/UPROPERTY()
	FVector NavMeshCorner;
	/*
	* The overall size of the nav mesh. This is used for UVs and bounds.
	*/UPROPERTY()
	FVector NavMeshSize;
	/*
	* This is also the column size, since the UVs should be organized into square arrays.
	*/UPROPERTY()
	int FloorRowSize = 1;
	UPROPERTY()
	bool bRebuildOnConstruct = false;
	/*
	* Called after the mesh has been updated.
	*/UFUNCTION()
	void FinishedProcessing();
	/*
	* Used to draw the debug for floors.
	*/UFUNCTION()
	void DrawFloors();
	
};
