# NavMesh Runtime Render
Actor for rendering the NavMesh during runtime. The mesh CANNOT be rebuilt during runtime.

Note: This actor does not use collision, but saving the mesh allows for Collision to be added (Complex as Simple is used by default)

UVs are created by predefined floor heights. This allows stacked floors to have different UV locations. All UVs are within the range of 0 and 1. A float has been included to add margins between floors.

A function has been added for easy UV hit detection.

The rendered mesh can now be saved and exported for further customization. The asset is saved as a StaticMesh along with the assigned Material.

How to use:

https://github.com/user-attachments/assets/6ce7c937-0b22-478a-be4d-120e04db852a

### Example use case video.

[![Use case](https://img.youtube.com/vi/12yw97yIACY/0.jpg)](https://www.youtube.com/watch?v=12yw97yIACY)


