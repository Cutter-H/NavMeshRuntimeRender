// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/DynamicMeshComponent.h"
#include "DynamicNavMeshRendererComponent.generated.h"

/**
 * This class was just created to guarantee that proper bounds were set.
 */
UCLASS(NotBlueprintable, NotBlueprintType)
class NAVMESHRUNTIMERENDER_API UDynamicNavMeshRendererComponent : public UDynamicMeshComponent
{
	GENERATED_BODY()
public:
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	virtual uint64 GetHiddenEditorViews() const override;
	UPROPERTY()
	FVector BoundsExtents;
};
