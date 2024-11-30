# NavMeshRuntimeRender
Actor for rendering the NavMesh during runtime. The mesh can be rebuilt during runtime, but will have a significant performance impact during creation.

Note: This actor does not use collision, but saving the mesh allows for Collision to be added (Complex as Simple is used by default)

UVs are created by predefined floor heights. This allows stacked floors to have different UV locations. All UVs are within the range of 0 and 1. A float has been included to add margins between floors.

A function has been added for easy UV hit detection.

The rendered mesh can now be saved and exported for further customization. The asset is saved as a StaticMesh along with the assigned Material.

How to use: (This is a video showing the initial prototype of this plugin. An updated video will be added tomorrow night!)

https://github.com/user-attachments/assets/9de551a5-f4af-4778-af9e-32ec88e8f912

