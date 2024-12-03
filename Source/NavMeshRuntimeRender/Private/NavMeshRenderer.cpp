// Cutter H // 2024


#include "NavMeshRenderer.h"
#include "DynamicNavMeshRendererComponent.h"

#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"

#include "CustomMeshComponent.h"
#include "Components/DynamicMeshComponent.h"

#include "DynamicMesh/MeshAttributeUtil.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"

#include "ModelingObjectsCreationAPI.h"
#include "AssetUtils/CreateStaticMeshUtil.h"

ANavMeshRenderer::ANavMeshRenderer() {
	/* Set default materials from plugin dir */ {
		if (!IsValid(NavMeshMaterial)) {
			const FString defaultNavMatDir = ("/Script/Engine.Material'/NavMeshRuntimeRender/DefaultMaterials/M_DefaultRenderer.M_DefaultRenderer'");
			static ConstructorHelpers::FObjectFinder<UMaterialInterface> defaultNavMat(*defaultNavMatDir);
			if (defaultNavMat.Object) {
				NavMeshMaterial = defaultNavMat.Object;
			}
		}
		if (!IsValid(FloorDebugMaterial)) {
			const FString defaultFloorMatDir = ("/Script/Engine.Material'/NavMeshRuntimeRender/DefaultMaterials/M_FloorDebug.M_FloorDebug'");
			static ConstructorHelpers::FObjectFinder<UMaterialInterface> defaultFloorMat(*defaultFloorMatDir);
			if (defaultFloorMat.Object) {
				FloorDebugMaterial = defaultFloorMat.Object;
			}
		}
	}
	
	DynamicNavMeshRender = CreateDefaultSubobject<UDynamicNavMeshRendererComponent>(TEXT("NavMeshRender"));
	if (IsValid(DynamicNavMeshRender)) {
		DynamicNavMeshRender->SetupAttachment(GetRootComponent());
		if (NumberOfTris > 0) {
			if (IsValid(NavMeshMaterial)) {
				DynamicNavMeshRender->SetMaterial(0, NavMeshMaterial);
			}
		}
	}
	SetActorTickEnabled(false);
#if WITH_EDITOR
	if (IsSelectedInEditor() && bShowFloorDebug) {
		DrawFloors();
	}
	else {
		if (IsValid(FloorDebug)) {
			FloorDebug->DestroyComponent();
		}
	}
#endif
}

void ANavMeshRenderer::PostLoadSubobjects(FObjectInstancingGraph* OuterInstanceGraph) {
	UpdateMesh();
	Super::PostLoadSubobjects(OuterInstanceGraph);
}

void ANavMeshRenderer::OnConstruction(const FTransform& transform) {
		SetActorTransform(FTransform());
#if WITH_EDITOR
	if(bShowFloorDebug) {
		DrawFloors();
	}
	else {
		if (IsValid(FloorDebug)) {
			FloorDebug->DestroyComponent();
		}
	}
#else
	if (IsValid(FloorDebug)) {
		FloorDebug->DestroyComponent();
	}
#endif
}

void ANavMeshRenderer::BeginPlay() {
	Super::BeginPlay();
	if (IsValid(FloorDebug)) {
		FloorDebug->DestroyComponent();
	}
}

const UDynamicMeshComponent* ANavMeshRenderer::GetNavMeshRender() const {
    return DynamicNavMeshRender;
}

FVector2D ANavMeshRenderer::GetUV_Coordinate(const FVector& location, float centralHeight) const {
	FVector2f retVal = UVCoord(location, centralHeight);
	return FVector2D(retVal.X, retVal.Y);
}

 void ANavMeshRenderer::UpdateMesh(){
	 if (!IsValid(DynamicNavMeshRender)) {
		 DynamicNavMeshRender = CreateDefaultSubobject<UDynamicNavMeshRendererComponent>(TEXT("NavMeshRender"));
		 if (IsValid(DynamicNavMeshRender)) {
			 DynamicNavMeshRender->SetupAttachment(GetRootComponent());
			 if (NumberOfTris > 0) {
				 if (IsValid(NavMeshMaterial)) {
					 DynamicNavMeshRender->SetMaterial(0, NavMeshMaterial);
				 }
			 }
		 }
	 }
	 const ARecastNavMesh* navMesh = GetNavMesh();
	 if (!IsValid(navMesh)) {
		 UE_LOG(LogNavigation, Warning, TEXT("NavMeshRenderer found no Navigation Mesh"));
		 return;
	 }
	 NavMeshSize = navMesh->GetBounds().GetSize();
	 NavMeshCorner = navMesh->GetBounds().GetCenter() - NavMeshSize / 2;
	 NumberOfFloors = 1;
	 AdditionalFloorHeights.Sort();
	 for (float floorHeight : AdditionalFloorHeights) {
		 if (floorHeight <= NavMeshSize.Z && floorHeight > 0) {
			 NumberOfFloors++;
		 }
	 }
	 DynamicNavMeshRender->BoundsExtents = NavMeshSize;
	 DynamicNavMeshRender->UpdateBounds();
	 FloorRowSize = 1;
	 if(NumberOfFloors > 1) {
		  FloorRowSize = sqrt(NumberOfFloors)+1;
	 }
	 FDynamicMesh3* renderedMesh = DynamicNavMeshRender->GetMesh();
	 renderedMesh->Clear();
	 renderedMesh->EnableAttributes();
	 renderedMesh->EnableVertexUVs(FVector2f::Zero());
	 const int tileCount = navMesh->GetNavMeshTilesCount();
	 TArray<FNavPoly> finishedPolys;
	 TArray<FNavPoly> currentPolys;
	 int triA = 0; int triB = 0; int triC = 0; int currentGroup = 0;
	 TArray<FVector> currentVerts;
	 TArray<FVector> allVectors;
	 /* Add Verts and Tris (During this we're checking for extremes to setup bounds that exactly match our verts and tris ) */ {
		 for (int32 v = 0; v < tileCount; v++) {
			 if (!navMesh->GetPolysInTile(v, currentPolys)) {
				 continue;
			 }
			 for (const FNavPoly& poly : currentPolys) {
				 if (!finishedPolys.ContainsByPredicate([poly](const FNavPoly& tester) {
					 return poly.Center == tester.Center;
					 })) {
					 finishedPolys.Add(poly);
				 }
				 else {
					 continue;
				 }
				 if (!navMesh->GetPolyVerts(poly.Ref, currentVerts)) {
					 continue;
				 }
				 const float uvHeight = poly.Center.Z;
				 for (int i = 1; i < currentVerts.Num(); i++) {
					 triA = AssignNewVert(poly.Center, uvHeight);
					 triB = AssignNewVert(currentVerts[i - 1], uvHeight);
					 triC = AssignNewVert(currentVerts[i], uvHeight);
					 renderedMesh->AppendTriangle(triA, triB, triC, currentGroup);//allVectors.Add(currentVerts[i]);
				 }
				 // Complete the circle
				 triB = AssignNewVert(currentVerts[currentVerts.Num() - 1], uvHeight);
				 triC = AssignNewVert(currentVerts[0], uvHeight);
				 renderedMesh->AppendTriangle(triA, triB, triC, currentGroup);
			 }
			 currentGroup++;
		 }
	 }
	 DynamicNavMeshRender->GetDynamicMesh()->ProcessMesh([this](const FDynamicMesh3& ProcessMesh) {
	 FinishedProcessing();
	 });
	 DynamicNavMeshRender->UpdateBounds();
	 // Update the material to the override variable.
	 if (IsValid(NavMeshMaterial)) {
		 DynamicNavMeshRender->SetMaterial(0, NavMeshMaterial);
	 }
	 // This is debug info that is displayed in Details
	 NumberOfVerts = renderedMesh->VertexCount();
	 NumberOfTris = renderedMesh->TriangleCount();
}

 void ANavMeshRenderer::ClearMesh() {
	 NumberOfVerts = -1;
	 NumberOfTris = -1;
	 if (!IsValid(DynamicNavMeshRender)) {
		 return;
	 }
	 DynamicNavMeshRender->GetMesh()->Clear();
	 FinishedProcessing();
 }

 void ANavMeshRenderer::z_SaveMesh() {
#if WITH_EDITOR
	if (!IsValid(DynamicNavMeshRender) || !IsValid(DynamicNavMeshRender->GetDynamicMesh())) {
		UE_LOG(LogNavigation, Error, TEXT("Unable to save. No NavMeshRender."));
		return;
	}
	if (DynamicNavMeshRender->GetMesh()->TriangleCount() <= 0) {
		UE_LOG(LogNavigation, Error, TEXT("Unable to save. Render has no Tris."));
		return;
	}
	if (FPackageName::DoesPackageNameContainInvalidCharacters(SaveName) || SaveName.IsEmpty()) {
		UE_LOG(LogNavigation, Error, TEXT("Unable to save. Invalid Asset Name: %s"), *SaveName);
		return;
	}
	FString savePath = "/Game/" + SaveLocation;
	if (!FPaths::ValidatePath(savePath)) {
		UE_LOG(LogNavigation, Error, TEXT("Unable to save. Invalid path: %s"), *savePath);
		return;
	}
	FString packageName = savePath + FString("/") + SaveName;
	FPaths::RemoveDuplicateSlashes(packageName);
	// Safety measure. Shouldn't succeed.
	if (FPackageName::DoesPackageNameContainInvalidCharacters(SaveName)) {
		UE_LOG(LogNavigation, Error, TEXT("Unable to save. Invalid Package Name: %s"), *packageName);
		return;
	}
	FCreateMeshObjectParams creationParams;
	DynamicNavMeshRender->ValidateMaterialSlots();
	for (UMaterialInterface* mat : DynamicNavMeshRender->GetMaterials()) {
		creationParams.Materials.Add(mat);
	}
	creationParams.SetMesh(DynamicNavMeshRender->GetMesh());
	creationParams.MeshType = ECreateMeshObjectSourceMeshType::DynamicMesh;
	creationParams.BaseName = packageName;
	creationParams.TargetWorld = nullptr;
	UE::AssetUtils::FStaticMeshAssetOptions creationOptions;
	for (UMaterialInterface* mat : creationParams.Materials) {
		creationOptions.AssetMaterials.Add(mat);
	}
	creationOptions.NewAssetPath = packageName;
	creationOptions.NumSourceModels = 1;
	creationOptions.NumMaterialSlots = DynamicNavMeshRender->GetMaterials().Num();
	creationOptions.bEnableRecomputeNormals = creationParams.bEnableRecomputeNormals;
	creationOptions.bEnableRecomputeTangents = creationParams.bEnableRecomputeTangents;
	creationOptions.bGenerateNaniteEnabledMesh = creationParams.bEnableNanite;
	creationOptions.bCreatePhysicsBody = creationParams.bEnableCollision;
	creationOptions.CollisionType = creationParams.CollisionMode;
	creationOptions.SourceMeshes.DynamicMeshes.Add(&creationParams.DynamicMesh.GetValue());
	UE::AssetUtils::FStaticMeshResults result;
	UE::AssetUtils::ECreateStaticMeshResult creationResult = UE::AssetUtils::CreateStaticMeshAsset(creationOptions, result);
	if (creationResult == UE::AssetUtils::ECreateStaticMeshResult::Ok) {
		UE_LOG(LogNavigation, Warning, TEXT("Successfully copied to Static Mesh Asset."));
	}
	else {
		UE_LOG(LogNavigation, Error, TEXT("Failed to copy to Static Mesh Asset."));
	}
	return;
#endif
	 UE_LOG(LogNavigation, Warning, TEXT("NavMeshRenderer cannot save during runtime"));
 }

 int ANavMeshRenderer::AssignNewVert(const FVector& location, float UVHeight) {
	 UE::Geometry::FVertexInfo info;
	 info.Position = location + FVector(0.f, 0.f, ZOffset);
	 info.bHaveUV = true;
	 info.bHaveN = true;
	 info.bHaveC = true;
	 info.UV = UVCoord(location, UVHeight);
	 info.Normal = FVector3f::UpVector;
	 info.Color = FVector3f();
	 if (!IsValid(DynamicNavMeshRender)) {
		 UE_LOG(LogNavigation, Warning, TEXT("NavMeshRenderer does not have a Dynamic Mesh Component"));
		 return - 1;
	 }
	 FDynamicMesh3* renderedMesh = DynamicNavMeshRender->GetMesh();
	 
	 int retVal = renderedMesh->AppendVertex(info);
	 return retVal;
 }

 FVector2f ANavMeshRenderer::UVCoord(const FVector& location, float centralHeight) const {
	 FVector2f retVal;
	 const FVector relativeLocation = location - NavMeshCorner;
	 const float relativeCentralHeight = centralHeight - NavMeshCorner.Z;
	 int floor = 0;
	 for (float floorHeight : AdditionalFloorHeights) {
		 if (relativeCentralHeight >= floorHeight) {
			 floor++;
		 }
	 }
	 const float floorX = floor % FloorRowSize;
	 const float floorY = FMath::TruncToInt((float)floor / (float)FloorRowSize);
	 // retVal is not the correct uv position if the floor takes up the entire UV
	 retVal = FVector2f((relativeLocation.X / NavMeshSize.X) * (1 - UV_IslandMargins), 
		 (relativeLocation.Y / NavMeshSize.Y) * (1 - UV_IslandMargins));
	 // positions retVal to accomodate multiple floors in the UV
	 if (NumberOfFloors > 1){
		 retVal = FVector2f(
			 (retVal.X / (float)FloorRowSize) + (floorX / (float)FloorRowSize) + (floorX * UV_IslandMargins),
			 (retVal.Y / (float)FloorRowSize) + (floorY / (float)FloorRowSize) + (floorY * UV_IslandMargins)
		 );
	 }
	 
	 return retVal;
 }

 const ARecastNavMesh* ANavMeshRenderer::GetNavMesh() const {
	 if (!IsValid(GetWorld())) {
		 UE_LOG(LogNavigation, Warning, TEXT("NavMeshRenderer found no World"));
		 return nullptr;
	 }
	 if (!IsValid(GetWorld()->GetNavigationSystem())) {
		 UE_LOG(LogNavigation, Warning, TEXT("NavMeshRenderer found no Navigation System"));
		 return nullptr;
	 }
	 return Cast<ARecastNavMesh>(GetWorld()->GetNavigationSystem()->GetMainNavData());	 
 }

 void ANavMeshRenderer::FinishedProcessing() {
	 UE_LOG(LogNavigation, Warning, TEXT("NavMeshRenderer Finished processing mesh."));
	 UE::Geometry::CopyVertexUVsToOverlay(*DynamicNavMeshRender->GetMesh(), *DynamicNavMeshRender->GetMesh()->Attributes()->PrimaryUV());
	 //DynamicNavMeshRender->GetMesh()->Attributes()->EnableTangents();
	 DynamicNavMeshRender->UpdateBounds();
	 DynamicNavMeshRender->MarkRenderStateDirty();
#if WITH_EDITOR
	 DynamicNavMeshRender->MarkPackageDirty();
	 DynamicNavMeshRender->GetDynamicMesh()->MarkPackageDirty();
#endif
	 OnMeshUpdate.Broadcast();
 }

 void ANavMeshRenderer::DrawFloors() {
	 if (NavMeshSize.Z < 1) {
		 return;
	 }
	 if (!IsValid(FloorDebug)) {
		 FloorDebug = Cast<UCustomMeshComponent>(AddComponentByClass(UCustomMeshComponent::StaticClass(), true, FTransform(), false));
		 FloorDebug->bHiddenInGame = true;
	 }
	 bool bDrewFinal = false;
	 TArray<FCustomMeshTriangle> tris;
	 tris.Add(FCustomMeshTriangle(
		 NavMeshCorner + FVector(NavMeshSize.X, 0.f, -.1f),
		 NavMeshCorner + FVector(0.f, 0.f, -.1f),
		 NavMeshCorner + FVector(0.f, NavMeshSize.Y, -.1f)
	 ));
	 tris.Add(FCustomMeshTriangle(
		 NavMeshCorner + FVector(NavMeshSize.X, 0.f, -.1f),
		 NavMeshCorner + FVector(0.f, NavMeshSize.Y, -.1f),
		 NavMeshCorner + FVector(NavMeshSize.X, NavMeshSize.Y, -.1f)
	 ));
	 for (float floorHeight : AdditionalFloorHeights) {
		 if ((floorHeight > 0)) {
			 if (floorHeight > NavMeshSize.Z) {
				 if (bDrewFinal) {
					 continue;
				 }
				 bDrewFinal = true;
			 }
			 tris.Add(FCustomMeshTriangle(
				 NavMeshCorner + FVector(NavMeshSize.X, 0.f, floorHeight),
				 NavMeshCorner + FVector(0.f, 0.f, floorHeight),
				 NavMeshCorner + FVector(0.f, NavMeshSize.Y, floorHeight)
			 ));
			 tris.Add(FCustomMeshTriangle(
				 NavMeshCorner + FVector(NavMeshSize.X, 0.f, floorHeight),
				 NavMeshCorner + FVector(0.f, NavMeshSize.Y, floorHeight),
				 NavMeshCorner + FVector(NavMeshSize.X, NavMeshSize.Y, floorHeight)
			 ));
		 }
	 }
	 FloorDebug->SetCustomMeshTriangles(tris);
	 if (IsValid(FloorDebugMaterial)) {
		 FloorDebug->SetMaterial(0, FloorDebugMaterial);
	 }
 }
