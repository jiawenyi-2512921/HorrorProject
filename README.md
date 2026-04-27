# HorrorProject

Unreal Engine 5.6 first-person horror project.

## Repository Baseline

- Engine: Unreal Engine 5.6
- Project: `HorrorProject/HorrorProject/HorrorProject.uproject`
- Source modules: `HorrorProject`, `HorrorProjectEditor`
- Runtime default map: `/Game/Variant_Horror/Lvl_Horror`
- Large third-party/sample asset packs are intentionally excluded from Git unless explicitly promoted to project-owned content and tracked through Git LFS.

## Local Build

```powershell
& "D:\UnrealEngine\UE_5.6\Engine\Build\BatchFiles\Build.bat" HorrorProject Win64 Development -Project="D:\gptzuo\HorrorProject\HorrorProject\HorrorProject.uproject" -WaitMutex -NoHotReload
```

```powershell
& "D:\UnrealEngine\UE_5.6\Engine\Build\BatchFiles\Build.bat" HorrorProjectEditor Win64 Development -Project="D:\gptzuo\HorrorProject\HorrorProject\HorrorProject.uproject" -WaitMutex -NoHotReload
```

## Source Control Notes

- Do not commit `Saved/`, `Intermediate/`, `Binaries/`, `DerivedDataCache/`, packaged builds, or local agent state.
- Commit `.uasset`, `.umap`, and other binary assets through Git LFS.
- GitHub Actions are not enabled in this baseline because Unreal builds require a configured UE 5.6 Windows runner.
