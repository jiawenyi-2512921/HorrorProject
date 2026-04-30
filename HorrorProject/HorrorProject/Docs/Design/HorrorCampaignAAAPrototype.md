# Horror Campaign AAA Prototype

## Core Fantasy

The player is a field recorder pulled through corrupted bodycam footage. Every map is a different layer of the same signal. The campaign begins as VHS/bodycam distortion, moves through the outdoor corruption, decodes industrial machinery, descends through the dungeon, defeats the Stone Golem, and returns to Deep Water Station for the finale.

This is an AAA-shaped vertical slice: cinematic map order, readable chapter goals, a complete boss chapter, a final map, and room for later inserted videos between chapters.

## Chapter Order

1. `Chapter.SignalCalibration`
   Map: `/Game/Bodycam_VHS_Effect/Maps/LVL_Showcase_01`
   Role: Prologue, camera/tutorial, first distress signal.

2. `Chapter.ForestOfSpikes`
   Map: `/Game/ForestOfSpikes/Levels/Level_ForestOfSpikes_Demo_Night`
   Role: Outdoor horror escalation. The forest hides the first physical relic.

3. `Chapter.Scrapopolis`
   Map: `/Game/Scrapopolis/Levels/Level_Scrapopolis_Demo`
   Role: Steampunk village puzzle layer. Restore machine power and decode the dungeon route.

4. `Chapter.DungeonEntrance`
   Map: `/Game/Fantastic_Dungeon_Pack/maps/map_dungeon_level_2_entrance`
   Role: First descent. Open the fog-door seal.

5. `Chapter.DungeonDepths`
   Map: `/Game/Fantastic_Dungeon_Pack/maps/map_dungeon_level_1_dungeon`
   Role: Archive recovery and route marking.

6. `Chapter.DungeonHall`
   Map: `/Game/Fantastic_Dungeon_Pack/maps/map_dungeon_level_3_hall`
   Role: Trap room and Stone Golem foreshadowing.

7. `Chapter.DungeonTemple`
   Map: `/Game/Fantastic_Dungeon_Pack/maps/map_dungeon_level_4_temple`
   Role: Ritual preparation, final key seal.

8. `Chapter.StoneGolemBoss`
   Map: `/Game/Fantastic_Dungeon_Pack/maps/map_dungeon_level_5_bossroom`
   Role: Complete BOSS chapter. Activate altar, break three weak points, seal the core.

9. `Chapter.DeepWaterStationFinale`
   Map: `/Game/DeepWaterStation/Maps/DemoMap_VerticalSlice_Day1`
   Role: Final chapter. Upload the relic chain to the station terminal, then use the temporary ending exit near spawn.

## Play Loop

Each chapter spawns campaign objective actors along a simple route from the player start. The player completes them in order with the existing interact key, but objectives are no longer all one-click tasks: multi-step objectives require repeated interactions, and `SurviveAmbush` objectives now start a longer pursuit beat. When a pursuit begins, the objective marker jumps to a distant escape point and a temporary Stone Golem-style pursuer wakes near the player. The objective only completes after the timer has expired and the player reaches the escape marker, so the chase now pushes the player toward a place instead of ending passively. The map-chain exit remains locked until the active chapter is complete. Non-final chapters auto-travel to the next map after completion; the exit remains as a backup route. The final Deep Water Station chapter unlocks the temporary ending exit near spawn instead of auto-traveling.

The prologue now includes `Signal.SurviveStaticPursuit`, so the first playable map has a short pressure beat instead of only two location-finding interactions. Forest's `Forest.HoldSpikeCircle` uses the same ambush threat hook, making the beacon holdout feel like a chase/defense moment instead of a passive timer. Both opening pursuits have far escape offsets and per-map pursuer tuning: the prologue uses a smaller corridor-scaled threat, while the forest uses a larger, faster outdoor threat.

Chapter completion now uses a per-chapter bridge line, so the current prototype already has narrative handoff points for later inserted videos.

The current layout is still conservative for testing, but objectives now spread farther from spawn so players follow a chapter route. Later polish should move each objective to authored landmarks inside the imported maps and insert video transitions between chapters.

## BOSS Structure

The Stone Golem BOSS uses the imported Stone Golem skeletal mesh as the arena centerpiece.

Current phase structure:

1. Activate the arena altar.
2. Break left shoulder weak point.
3. Break chest weak point.
4. Break right arm weak point.
5. Seal the exposed core.

The BOSS starts dormant when the player enters the arena. Completing the arena altar objective wakes it, enabling chase movement, attacks, and fear pressure. When all weak points are resolved, the BOSS is flagged defeated. The final core seal completes the chapter and unlocks the route to Deep Water Station.

The BOSS now also applies a fear-pressure aura while awake. Breaking weak points raises the BOSS light/VFX intensity and increases pressure, then defeating the BOSS stops movement, attacks, VFX, and the aura.

## Implemented Systems

- `FHorrorCampaign` defines chapter metadata, objective text, map order, final chapter, and BOSS chapter.
- Campaign chapter definitions include completion bridge text for later video interludes.
- `FHorrorCampaignProgress` enforces ordered objectives and BOSS weak-point completion.
- `AHorrorCampaignObjectiveActor` creates interactable in-world task points with mesh, label, collision, light, multi-step progress, and timed survival objectives.
- `AHorrorCampaignBossActor` places the Stone Golem BOSS visual in the boss map, tracks dormant/awake/defeated state, chases and attacks while awake, tracks weak-point progress, and applies fear pressure.
- `AHorrorGameModeBase` auto-spawns campaign runtime actors on map-chain maps.
- `AHorrorGameModeBase` now spawns and clears temporary ambush pursuers for timed survival objectives, with per-map scale, movement, attack, and fear-pressure tuning.
- Imported-map sanitation now clears held camera fades, restores the possessed player view, disables Scrapopolis imported global post-process volumes, clamps heavy fog, and adds a spawn rescue light to prevent black-screen-with-movement failures.
- Scrapopolis uses a lighter campaign atmosphere profile than the default horror profile so the steampunk village stays visible after travel.
- `AHorrorMapChainExit` now gates travel until the current campaign chapter is complete.
- `DefaultEngine.ini` boots the game into the Bodycam VHS prologue after the startup movie.
- Non-final chapter completion queues a short auto-travel to the next map.
- Campaign audio now has default mappings for objective completion, chapter completion, ambush start, boss weak points, and boss attacks.
- Campaign runtime text and interaction prompts were cleaned to readable Chinese for playtesting.

## Next Polish Passes

- Move generated objectives to hand-authored landmarks in each map after playtesting the imported geometry.
- Add more mini mechanics: scan hold, generator delay, carried relic return, trap bypass.
- Expand the Stone Golem BOSS with authored attack tells, camera shake, footstep/roar audio, and avoidance windows.
- Insert transition videos between chapter exits.
- Add final video playback after the Deep Water Station temporary ending.
