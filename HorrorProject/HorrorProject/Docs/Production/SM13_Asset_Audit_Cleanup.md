# SM-13 Asset Audit and Cleanup Plan

## Current project target

The 21-day target is a small, polished Deep Water Station found-footage vertical slice. Asset decisions should serve only that target: playable 10-15 minute route, strong bodycam/VHS presentation, stone golem encounter, horror audio, and trailer capture.

## Current asset inventory

| Folder | Size | Files | Decision |
| --- | ---: | ---: | --- |
| `/Game/DeepWaterStation` | 7969.6 MiB | 391 | Keep. Primary environment pack. |
| `/Game/SD_Art` | 7939.8 MiB | 750 | Probation/likely keep selectively. Industrial architecture may fit Deep Water Station, but it is huge and must be sampled before using. |
| `/Game/NiagaraExamples` | 1218.9 MiB | 669 | Remove unless a specific Niagara effect is migrated into `_SM13`. Full sample pack is too broad. |
| `/Game/Fantastic_Dungeon_Pack` | 817.4 MiB | 1342 | Remove. Fantasy dungeon direction conflicts with industrial underwater found footage. |
| `/Game/Scrapopolis` | 609.9 MiB | 174 | Probation. Keep only if specific scrap props are selected; otherwise remove. |
| `/Game/Grimytheus_Vol_2` | 584.7 MiB | 20 | Probation. Music could help, but only 1-2 cues should be selected. |
| `/Game/M5VFXVOL2` | 575.2 MiB | 433 | Remove unless one fire/smoke effect is migrated into `_SM13`. Full pack is too broad. |
| `/Game/Free_Magic` | 515.3 MiB | 192 | Remove. Magic VFX does not match this slice. |
| `/Game/ForestOfSpikes` | 508.6 MiB | 129 | Remove. Outdoor fantasy forest conflicts with Deep Water Station. |
| `/Game/Bodycam_VHS_Effect` | 298.9 MiB | 232 | Keep core materials/widgets/sounds; remove demo maps/mannequin demo content after redirectors are fixed. |
| `/Game/SoundsOfHorror` | 278.3 MiB | 204 | Keep. Strong fit for 21-day horror polish. |
| `/Game/ModernBridges` | 216.1 MiB | 35 | Probation. Could support exterior/industrial bridge set dressing; keep only selected meshes if used. |
| `/Game/Stone_Golem` | 141.2 MiB | 25 | Keep. Planned premium encounter. |
| `/Game/Characters` | 125.3 MiB | 128 | Probation. Likely template combat animations; remove unless needed by player/encounter. |
| `/Game/AdvancedUniversalSpawner` | 113.4 MiB | 96 | Remove. Unneeded system complexity for a scripted 21-day slice. |
| `/Game/Input` | 0.3 MiB | 16 | Keep. C++ hard references these input assets. |
| `/Game/Horror` | small | 1 | Keep. Default game mode blueprint. |
| `/Game/Blueprints` | small | 2 | Keep. Player character/controller blueprints. |
| `/Game/Variant_Horror` | small | 11 | Keep for now. It contains the active default map and UI. |
| `/Game/FirstPerson` | small | 7 | Remove after replacing old template map references and confirming no dependencies. |

## Hard references found in code/config

These paths are currently hard-referenced and must not be moved or deleted without updating code/config and fixing redirectors:

- `/Game/Variant_Horror/Lvl_Horror.Lvl_Horror` in `Config/DefaultEngine.ini`.
- `/Game/Horror/BP_HorrorGameModeBase.BP_HorrorGameModeBase_C` in `Config/DefaultEngine.ini`.
- `/Game/Input/Actions/IA_Interact.IA_Interact` in `Source/HorrorProject/Player/HorrorPlayerCharacter.cpp`.
- `/Game/Input/Actions/IA_Record.IA_Record` in `Source/HorrorProject/Player/HorrorPlayerCharacter.cpp`.
- `/Game/Input/Actions/IA_TakePhoto.IA_TakePhoto` in `Source/HorrorProject/Player/HorrorPlayerCharacter.cpp`.
- `/Game/Input/Actions/IA_Rewind.IA_Rewind` in `Source/HorrorProject/Player/HorrorPlayerCharacter.cpp`.
- `/Game/Input/Actions/IA_OpenArchive.IA_OpenArchive` in `Source/HorrorProject/Player/HorrorPlayerCharacter.cpp`.
- `/Game/Bodycam_VHS_Effect/Materials/Instances/PostProcess/MI_OldVHS.MI_OldVHS` in `Source/HorrorProject/Player/Components/VHSEffectComponent.cpp`.

## Target organization

Do not move imported marketplace/sample packs directly in the filesystem. Keep vendor packs in their original top-level folders. Put only project-authored or curated slice assets under `/Game/_SM13`.

Target folders created:

- `/Game/_SM13/Maps`
- `/Game/_SM13/Blueprints`
- `/Game/_SM13/Route`
- `/Game/_SM13/UI`
- `/Game/_SM13/Audio`
- `/Game/_SM13/VFX`
- `/Game/_SM13/Materials`
- `/Game/_SM13/Cinematics`

Rules:

1. Imported packs remain as source libraries until a specific asset is chosen.
2. Selected assets should be duplicated/migrated in-editor into `_SM13` only when the slice actually uses them.
3. After in-editor moves/deletes, run `Fix Up Redirectors in Folder` on `/Game`.
4. Never delete `__ExternalActors__` or `__ExternalObjects__` manually; they belong to World Partition/One File Per Actor maps.
5. Demo maps and demo mannequin folders are deletion candidates, but only after reference checks inside Unreal Editor.

## Safe cleanup candidates

Safe to remove after Unreal Editor confirms no references:

- `/Game/AdvancedUniversalSpawner`
- `/Game/Free_Magic`
- `/Game/ForestOfSpikes`
- `/Game/Fantastic_Dungeon_Pack`
- `/Game/NiagaraExamples`
- `/Game/M5VFXVOL2`
- `/Game/FirstPerson`

Probation candidates:

- `/Game/Scrapopolis`: keep only if it provides specific industrial debris props for the station.
- `/Game/ModernBridges`: keep only if used for trailer/exterior industrial shots.
- `/Game/Characters`: keep only if current player BP or stone golem encounter depends on it.
- `/Game/Grimytheus_Vol_2`: choose one cue or remove.
- `/Game/SD_Art`: sample immediately. If it is better than DeepWaterStation for industrial architecture, select a tiny subset; otherwise remove because it duplicates environment scope at 7.8 GiB.

## What I can do automatically

I can safely do these without Epic/Fab account UI:

- Audit code/config references.
- Create project organization folders and production docs.
- Edit C++/config to point at chosen assets.
- Run Unreal commandlets/build/tests if the editor can launch headlessly.
- Delete folders that are proven unreferenced and that you explicitly authorize deleting.
- Download direct public/open-source files if license and URL are clear and no login is needed.

## What still needs user action

You still need to do these manually:

- Add Fab/Marketplace assets through Epic Launcher/Fab because they require your Epic account and license acceptance.
- Confirm destructive asset deletion if the delete would remove multi-GB marketplace folders.
- Use Unreal Editor for final asset moves/deletes when redirector safety matters.
- Review any asset license that is not clearly UE Marketplace, official, CC0, or project-owned.

## Next recommended action

Open the project in Unreal Editor, create or duplicate the actual playable map into `/Game/_SM13/Maps`, then choose a small curated set from DeepWaterStation, Stone_Golem, SoundsOfHorror, Bodycam_VHS_Effect, and maybe SD_Art. After that, remove the unused broad packs above and fix redirectors.
