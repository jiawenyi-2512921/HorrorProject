# HorrorProject

Unreal Engine 5.6 first-person horror project.

## Repository Baseline

- Engine: Unreal Engine 5.6
- Project: `HorrorProject/HorrorProject/HorrorProject.uproject`
- Source modules: `HorrorProject`, `HorrorProjectEditor`
- Runtime default map: `/Game/Bodycam_VHS_Effect/Maps/LVL_Showcase_01`
- Campaign cook maps include `Bodycam_VHS_Effect`, `ForestOfSpikes`, `Scrapopolis`, `Fantastic_Dungeon_Pack`, and `DeepWaterStation`.
- Large third-party/sample asset packs are intentionally excluded from Git unless explicitly promoted to project-owned content and tracked through Git LFS. The current campaign-promoted packs must be committed through Git LFS.

## Local Build

```powershell
$ProjectRoot = Resolve-Path ".\HorrorProject\HorrorProject"
$UERoot = $env:UE_5_6_ROOT
if (-not $UERoot) { $UERoot = "D:\UnrealEngine\UE_5.6" }
& "$UERoot\Engine\Build\BatchFiles\Build.bat" HorrorProject Win64 Development -Project="$ProjectRoot\HorrorProject.uproject" -WaitMutex -NoHotReload
```

```powershell
$ProjectRoot = Resolve-Path ".\HorrorProject\HorrorProject"
$UERoot = $env:UE_5_6_ROOT
if (-not $UERoot) { $UERoot = "D:\UnrealEngine\UE_5.6" }
& "$UERoot\Engine\Build\BatchFiles\Build.bat" HorrorProjectEditor Win64 Development -Project="$ProjectRoot\HorrorProject.uproject" -WaitMutex -NoHotReload
```

## Source Control Notes

- Do not commit `Saved/`, `Intermediate/`, `Binaries/`, `DerivedDataCache/`, packaged builds, or local agent state.
- Commit `.uasset`, `.umap`, and other binary assets through Git LFS.
- GitHub Actions are not enabled in this baseline because Unreal builds require a configured UE 5.6 Windows runner.
