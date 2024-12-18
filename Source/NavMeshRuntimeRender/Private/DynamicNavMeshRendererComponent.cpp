// Cutter H // 2024


#include "DynamicNavMeshRendererComponent.h"

FBoxSphereBounds UDynamicNavMeshRendererComponent::CalcBounds(const FTransform& LocalToWorld) const {
	FBox LocalBoundingBox = (FBox)LocalBounds;
	FBoxSphereBounds retVal(LocalBoundingBox.TransformBy(LocalToWorld));
	retVal.BoxExtent = BoundsExtents;
	retVal.SphereRadius = FMath::Max3(BoundsExtents.X, BoundsExtents.Y, BoundsExtents.Z);
	return retVal;
}

uint64 UDynamicNavMeshRendererComponent::GetHiddenEditorViews() const {
	return 0;
}
