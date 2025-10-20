# Modelling3DOne

Procedural geometry primitives for Unreal Engine 5.7 in modern C++. Clean examples of planes, spheres, cones, cylinders, trapezoids, and a Pac-Man-style cut sphere, built on `UProceduralMeshComponent`.

<p align="center">
  <img alt="Modelling3DOne" src="https://img.shields.io/badge/Unreal-5.7-0?logo=unrealengine">
  <img alt="C++" src="https://img.shields.io/badge/C%2B%2B-20-0?logo=c%2B%2B">
  <img alt="Platform" src="https://img.shields.io/badge/Platform-Windows-0">
</p>

> **What you get**: small, focused Actor classes that generate mesh data at runtime and push it to the GPU in a single call per section. Each generator follows the same pattern: **build arrays -> create triangles -> `CreateMeshSection` -> apply material -> enable collision**.

---

## Contents
- [Features](#features)
- [Project layout](#project-layout)
- [Quick start](#quick-start)
- [Shapes and APIs](#shapes-and-apis)
- [Core helper: `CreateTriangle`](#core-helper-createtriangle)
- [Math notes](#math-notes)
- [Materials and collision](#materials-and-collision)
- [Performance tips](#performance-tips)
- [FAQ](#faq)
- [Roadmap](#roadmap)
- [Contributing](#contributing)
- [License](#license)

---

## Features
- **Consistent code path** across all shapes
- **Deterministic winding** (CCW) and normals for correct lighting
- **UVs included** for basic planar and spherical mapping
- **Collision ready** via `CreateMeshSection(..., bCreateCollision=true)`
- **Minimal dependencies**: Unreal + `ProceduralMeshComponent`

## Project layout
```
Source/Modelling3DOne/
  ProceduralPlaneActor.*
  ProceduralSphereActor.*
  ProceduralConeActor.*
  ProceduralCylindreActor.*    // cylinder (note: 'Cylindre' spelling)
  ProceduralTrapezoidActor.*
  ProceduralPacMan.*           // Pac-Man cut sphere
```
> Each actor implements its own `CreateTriangle` helper method for mesh generation.
> Names reflect the current code. Keep them if you want plug-and-play.

## Quick start

1) **Enable plugin**: `Edit -> Plugins -> Procedural -> Procedural Mesh Component` (already enabled in `.uproject`).

2) **Add an actor** in C++ or Blueprint and call its generator:

```cpp
// Example: spawn and generate a plane
AProceduralPlaneActor* Plane = GetWorld()->SpawnActor<AProceduralPlaneActor>();
Plane->Nb_Lignes = 10;      // rows
Plane->Nb_Colones = 10;     // cols
Plane->QuadSize  = 100.f;   // world units per quad
Plane->GeneratePlane();
```

```cpp
// Example: sphere
AProceduralSphereActor* Sphere = GetWorld()->SpawnActor<AProceduralSphereActor>();
Sphere->Radius        = 150.f;
Sphere->NumParallels  = 24;
Sphere->NumMeridians  = 48;
Sphere->GenerateSphere();
```

Blueprint users: add the actor to the level, edit exposed parameters, then click your **Generate** function from the Details panel or call it at BeginPlay.

## Shapes and APIs

### Plane
Creates an XY grid of quads split into two triangles each.
```cpp
void AProceduralPlaneActor::GeneratePlane();
// Params: Nb_Lignes, Nb_Colones, QuadSize
```

### Sphere
Lat-long sphere: north/south pole caps + quad strips between parallels.
```cpp
void AProceduralSphereActor::GenerateSphere();
// Params: Radius, NumParallels, NumMeridians
```

### Cone / Frustum
Sloped body with optional top cap (if TopRadius > 0).
```cpp
void AProceduralConeActor::GenerateCone();
// Params: TopRadius, BottomRadius, Height, Meridians
```

### Cylinder
Special case of cone with equal radii.
```cpp
void AProceduralCylindreActor::GenerateCylinder();
// Params: Radius, Height, Meridians
```

### Trapezoid prism
Box-like geometry with custom top/bottom widths and depth.
```cpp
void AProceduralTrapezoidActor::GenerateTrapezoid();
// Params: TopWidth, BottomWidth, Height, Depth
```

### Pac-Man sphere
Sphere with an angular wedge removed, closed by interior "mouth" walls.
```cpp
void AProceduralPacMan::GeneratePacMan();
// Params: Radius, NumParallels, NumMeridians, MouthAngleDegrees
```

## Core helper: `CreateTriangle`
Each actor class contains its own `CreateTriangle` method that adds one triangle worth of data to all arrays and computes a flat normal. You pass references to the working arrays and three vertex positions; it appends three vertices, three indices offset by `StartIndex`, one per-vertex normal, and a UV triplet.

```cpp
void CreateTriangle(
  TArray<FVector>& Vertices,
  TArray<int32>&   Triangles,
  TArray<FVector>& Normals,
  TArray<FVector2D>& UVs,
  const FVector& V0,
  const FVector& V1,
  const FVector& V2);
```

**Winding**: CCW for outward facing.
**Normal**: `Normalize(Cross(V1 - V0, V2 - V0))`.
**UVs**: simple `(0,0), (1,0), (0,1)` as a default; replace per-shape as needed.

> **Note**: Currently each actor duplicates this helper. Future refactoring could move it to a shared utility class.

## Math notes

- **Sphere parametric form**
  - `X = r * sin(theta) * cos(phi)`
  - `Y = r * sin(theta) * sin(phi)`
  - `Z = r * cos(theta)`
  - Rings: parallels (theta), Slices: meridians (phi)

- **Cone body normals**
  - Horizontal direction `(cos(phi), sin(phi))` with vertical component `Delta_r/Height`
  - Normalize for per-vertex lighting on the sloped surface

- **Cylinder body normals**
  - `(cos(phi), sin(phi), 0)` because the side is vertical

- **Trapezoid faces**
  - Axis-aligned faces use cardinal normals; slanted faces via cross products

- **Pac-Man cut**
  - Skip meridians within `+/- MouthAngle/2` and close the gap with two triangular fans (upper and lower walls)

## Materials and collision
- Call `CreateMeshSection(SectionIndex, Vertices, Triangles, Normals, UVs, VertexColors, Tangents, /*bCreateCollision=*/true)` once per section after arrays are filled.
- Apply materials after creating the section:
```cpp
ProceduralMesh->SetMaterial(0, MaterialInstance);
```

## Performance tips
- Build all arrays first then upload once per section. Avoid per-triangle uploads.
- Cache `sin/cos` for meridians in local arrays.
- Reuse buffers between regenerations to avoid churn.
- Keep vertex duplication intentional: reuse for smooth shading or duplicate for sharp edges per face.

## FAQ
**Why CCW winding?**
Unreal uses CCW as front faces by default. Back faces are culled.

**Why duplicate vertices on flat faces?**
Different normals per face require distinct vertices for hard edges.

**How do I add tangents or vertex colors?**
Prepare matching-length arrays and pass them to `CreateMeshSection`.

**Why is the cylinder class called `ProceduralCylindreActor`?**
Original naming kept for consistency. Feel free to rename.

## Roadmap
- Refactor shared `CreateTriangle` into a utility class
- Optional indexed UV generators (cylindrical, cube, triplanar)
- LOD presets per shape
- Async generation example using tasks
- Cross-platform testing (Linux, Mac)

## Contributing
PRs welcome. Keep functions small, deterministic, and documented. Add shape-level diagrams where helpful.

## License
MIT. See `LICENSE`.
