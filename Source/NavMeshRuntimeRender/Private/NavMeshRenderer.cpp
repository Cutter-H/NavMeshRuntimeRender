// Fill out your copyright notice in the Description page of Project Settings.


#include "NavMeshRenderer.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"
#include "CustomMeshComponent.h"
#include "Net/UnrealNetwork.h"


ANavMeshRenderer::ANavMeshRenderer() {
	SetActorTickEnabled(false);
	NavMeshRender = CreateDefaultSubobject<UCustomMeshComponent>(TEXT("NavMeshRenderer"));
	if (IsValid(NavMeshRender)) {
		NavMeshRender->SetupAttachment(GetRootComponent());
		if (LoadedTris.Num() > 0) {
			UpdateNavMeshRender();
		}
	}

}

void ANavMeshRenderer::OnConstruction(const FTransform& transform) {
	SetActorTransform(FTransform());
	Super::OnConstruction(transform);
	if (IsValid(NavMeshRender)) {
		NavMeshRender->SetCustomMeshTriangles(LoadedTris);
		if (IsValid(NavMeshMaterial)) {
			NavMeshRender->SetMaterial(0, NavMeshMaterial);
		}
	}
}

void ANavMeshRenderer::BeginPlay() {
	Super::BeginPlay();
	NavMeshRender->SetCustomMeshTriangles(LoadedTris);
	OnNavMeshLoaded.AddDynamic(this, &ANavMeshRenderer::UpdateNavMeshRender);
}

void ANavMeshRenderer::UpdateNavMeshRender() {
	if (IsValid(NavMeshRender)) {
		NavMeshRender->SetCustomMeshTriangles(LoadedTris);
		if (GWorld) {
			if (!GWorld->HasBegunPlay()) {
				MarkPackageDirty();
				NumberOfTris = LoadedTris.Num();
			}
		}
	}
}

UCustomMeshComponent* ANavMeshRenderer::GetNavMeshRender() const {
    return NavMeshRender;
}

 void ANavMeshRenderer::UpdateWorldTris(){
	 if (!IsValid(GetWorld())) {
		 UE_LOG(LogNavigation, Warning, TEXT("NavMeshRenderer found no World"));
		 return;
	 }
	 const UNavigationSystemV1* navSys = Cast<UNavigationSystemV1>(GetWorld()->GetNavigationSystem());
	 if (!IsValid(navSys)) {
		 UE_LOG(LogNavigation, Warning, TEXT("NavMeshRenderer found no Navigation System"));
		 return;
	 }
	 const ARecastNavMesh* navMesh = Cast<ARecastNavMesh>(navSys->MainNavData);
	 if (!IsValid(navMesh)) {
		 UE_LOG(LogNavigation, Warning, TEXT("NavMeshRenderer found no Navigation Mesh"));
		 return;
	 }
	 LoadedTris.Empty();
	 const int tileCount = navMesh->GetNavMeshTilesCount();
	 TArray<FNavPoly> finishedPolys;
	 TArray<FNavPoly> currentPolys;
	 FCustomMeshTriangle currentTri;
	 TArray<FVector> currentVerts;
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
			 currentTri.Vertex0 = poly.Center;
			 for (int i = 1; i < currentVerts.Num(); i++) {
				 currentTri.Vertex1 = currentVerts[i - 1];
				 currentTri.Vertex2 = currentVerts[i];
				 if (LoadedTris.ContainsByPredicate([currentTri](const FCustomMeshTriangle& tri) {
					 return ((tri.Vertex0 + tri.Vertex1 + tri.Vertex2) / 3) ==
						 ((currentTri.Vertex0 + currentTri.Vertex1 + currentTri.Vertex2) / 3);
					 })) {
					 continue;
				 }
				 LoadedTris.Add(currentTri);
			 }
			 // Complete the circle
			 currentTri.Vertex1 = currentTri.Vertex2;
			 currentTri.Vertex2 = currentVerts[0];
			 LoadedTris.Add(currentTri);
		 }

	 }
	 UpdateNavMeshRender();
	 
}

 void ANavMeshRenderer::ClearWorldTris() {
	 LoadedTris.Empty();
	 UpdateNavMeshRender();
 }

 void ANavMeshRenderer::LazyUpdateWorldTris(ARecastNavMesh* navMesh, int numOfTiles, int currentTile) {
	 NavMeshRender->SetCustomMeshTriangles(LoadedTris);
	 if (currentTile >= numOfTiles) {
		 OnNavMeshLoaded.Broadcast();
		 return;
	 }
	 if (currentTile == 0) {
		 LoadedTris.Empty();
	 }
	 TArray<FNavPoly> currentPolys;
	 TArray<FNavPoly> finishedPolys;
	 TArray<FVector> currentVerts;
	 FCustomMeshTriangle currentTri;
	 if (navMesh->GetPolysInTile(currentTile, currentPolys)) {
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
			 currentTri.Vertex0 = poly.Center;
			 for (int i = 1; i < currentVerts.Num(); i++) {
				 currentTri.Vertex1 = currentVerts[i - 1];
				 currentTri.Vertex2 = currentVerts[i];
				 if (LoadedTris.ContainsByPredicate([currentTri](const FCustomMeshTriangle& tri) {
					 return ((tri.Vertex0 + tri.Vertex1 + tri.Vertex2) / 3) ==
						 ((currentTri.Vertex0 + currentTri.Vertex1 + currentTri.Vertex2) / 3);
					 })) {
					 continue;
				 }
				 LoadedTris.Add(currentTri);
			 }
			 // Complete the circle
			 currentTri.Vertex1 = currentTri.Vertex2;
			 currentTri.Vertex2 = currentVerts[0];
			 LoadedTris.Add(currentTri);
		 }
	 }
	 LazyUpdateWorldTris(navMesh, numOfTiles, currentTile + 1);
	//FTimerDelegate timerDel;
	 //timerDel.BindUObject(this, &ANavMeshRenderer::LazyUpdateWorldTris, navMesh, numOfTiles, currentTile + 1);
	 //GetWorld()->GetTimerManager().SetTimerForNextTick(timerDel);
 }