# Console Commands Reference

## Overview

Complete reference for all debug console commands in HorrorProject.

## Accessing the Console

Press the **`** (backtick/tilde) key to open the console.

## Command Categories

### Visualization Commands

#### DebugShowCollision
```
DebugShowCollision <true|false>
```
Shows or hides collision shapes in the world.

**Examples:**
```
DebugShowCollision true
DebugShowCollision false
```

#### DebugShowNavMesh
```
DebugShowNavMesh <true|false>
```
Shows or hides the navigation mesh.

**Examples:**
```
DebugShowNavMesh true
```

#### DebugShowAI
```
DebugShowAI <true|false>
```
Shows or hides AI debug information.

**Examples:**
```
DebugShowAI true
```

#### DebugShowAudio
```
DebugShowAudio <true|false>
```
Shows or hides audio source debug information.

**Examples:**
```
DebugShowAudio true
```

#### DebugShowPerformance
```
DebugShowPerformance <true|false>
```
Shows or hides performance statistics overlay.

**Examples:**
```
DebugShowPerformance true
```

### Performance Commands

#### DebugFPS
```
DebugFPS <true|false>
```
Toggles FPS display.

**Examples:**
```
DebugFPS true
```

#### DebugMemory
```
DebugMemory
```
Displays memory statistics and generates memory report.

**Examples:**
```
DebugMemory
```

#### DebugGPU
```
DebugGPU
```
Displays GPU statistics and captures GPU profile.

**Examples:**
```
DebugGPU
```

#### DebugDrawCalls
```
DebugDrawCalls
```
Displays draw call statistics and rendering information.

**Examples:**
```
DebugDrawCalls
```

### Gameplay Commands

#### DebugGodMode
```
DebugGodMode <true|false>
```
Enables or disables god mode (invincibility).

**Examples:**
```
DebugGodMode true
DebugGodMode false
```

**Warning:** This is a cheat command and should only be used for testing.

#### DebugNoClip
```
DebugNoClip <true|false>
```
Enables or disables no-clip mode (fly through walls).

**Examples:**
```
DebugNoClip true
DebugNoClip false
```

**Warning:** This is a cheat command and should only be used for testing.

#### DebugTeleport
```
DebugTeleport <X> <Y> <Z>
```
Teleports the player to the specified coordinates.

**Parameters:**
- X: X coordinate
- Y: Y coordinate
- Z: Z coordinate

**Examples:**
```
DebugTeleport 0 0 100
DebugTeleport 1000 2000 500
```

#### DebugSpawnEnemy
```
DebugSpawnEnemy <EnemyType>
```
Spawns an enemy of the specified type in front of the player.

**Parameters:**
- EnemyType: Type of enemy to spawn (e.g., "Ghost", "Monster")

**Examples:**
```
DebugSpawnEnemy Ghost
DebugSpawnEnemy Monster
```

#### DebugKillAllEnemies
```
DebugKillAllEnemies
```
Kills all enemies in the level.

**Examples:**
```
DebugKillAllEnemies
```

### Horror System Commands

#### DebugSetSanity
```
DebugSetSanity <Value>
```
Sets the player's sanity level.

**Parameters:**
- Value: Sanity value (0-100)

**Examples:**
```
DebugSetSanity 100
DebugSetSanity 50
DebugSetSanity 0
```

#### DebugSetFear
```
DebugSetFear <Value>
```
Sets the player's fear level.

**Parameters:**
- Value: Fear value (0-100)

**Examples:**
```
DebugSetFear 0
DebugSetFear 50
DebugSetFear 100
```

#### DebugTriggerHallucination
```
DebugTriggerHallucination
```
Manually triggers a hallucination event.

**Examples:**
```
DebugTriggerHallucination
```

#### DebugShowSanityInfo
```
DebugShowSanityInfo
```
Displays detailed sanity system information.

**Examples:**
```
DebugShowSanityInfo
```

### Evidence Commands

#### DebugShowAllEvidence
```
DebugShowAllEvidence
```
Shows the locations of all evidence in the level.

**Examples:**
```
DebugShowAllEvidence
```

#### DebugCollectAllEvidence
```
DebugCollectAllEvidence
```
Automatically collects all evidence in the level.

**Examples:**
```
DebugCollectAllEvidence
```

#### DebugResetEvidence
```
DebugResetEvidence
```
Resets all evidence collection progress.

**Examples:**
```
DebugResetEvidence
```

### Save/Load Commands

#### DebugSaveGame
```
DebugSaveGame <SlotName>
```
Saves the game to the specified slot.

**Parameters:**
- SlotName: Name of the save slot

**Examples:**
```
DebugSaveGame QuickSave
DebugSaveGame TestSave1
```

#### DebugLoadGame
```
DebugLoadGame <SlotName>
```
Loads the game from the specified slot.

**Parameters:**
- SlotName: Name of the save slot

**Examples:**
```
DebugLoadGame QuickSave
DebugLoadGame TestSave1
```

#### DebugDeleteSave
```
DebugDeleteSave <SlotName>
```
Deletes the specified save slot.

**Parameters:**
- SlotName: Name of the save slot

**Examples:**
```
DebugDeleteSave TestSave1
```

**Warning:** This permanently deletes the save file.

### Logging Commands

#### DebugSetLogLevel
```
DebugSetLogLevel <Category> <Level>
```
Sets the log level for a specific category.

**Parameters:**
- Category: Log category name
- Level: Log level (Verbose, Log, Warning, Error, Fatal)

**Examples:**
```
DebugSetLogLevel LogTemp Verbose
DebugSetLogLevel LogAI Warning
DebugSetLogLevel LogAudio Error
```

#### DebugDumpGameState
```
DebugDumpGameState
```
Dumps current game state to the log.

**Examples:**
```
DebugDumpGameState
```

#### DebugDumpActors
```
DebugDumpActors
```
Dumps all actors in the level to the log.

**Examples:**
```
DebugDumpActors
```

### Profiling Commands

#### DebugStartProfiling
```
DebugStartProfiling
```
Starts CPU profiling session.

**Examples:**
```
DebugStartProfiling
```

**Note:** Profiling data is saved to Saved/Profiling directory.

#### DebugStopProfiling
```
DebugStopProfiling
```
Stops CPU profiling session and saves results.

**Examples:**
```
DebugStopProfiling
```

#### DebugCaptureFrame
```
DebugCaptureFrame
```
Captures a single frame profile for detailed analysis.

**Examples:**
```
DebugCaptureFrame
```

## Built-in Unreal Engine Commands

### Performance

```
stat fps              - Show FPS
stat unit             - Show frame time breakdown
stat game             - Show game thread stats
stat gpu              - Show GPU stats
stat memory           - Show memory stats
stat streaming        - Show streaming stats
stat scenerendering   - Show rendering stats
stat rhi              - Show RHI stats
```

### Profiling

```
profilegpu            - Capture GPU profile
stat startfile        - Start CPU profiling
stat stopfile         - Stop CPU profiling
memreport             - Generate memory report
memreport -full       - Generate detailed memory report
obj list              - List all objects
obj gc                - Force garbage collection
```

### Rendering

```
show collision        - Toggle collision display
show bounds           - Toggle bounds display
show bsp              - Toggle BSP display
show fog              - Toggle fog
show postprocessing   - Toggle post-processing
r.SetRes <width>x<height> - Set resolution
r.ScreenPercentage <value> - Set screen percentage
```

### Camera

```
toggledebugcamera     - Toggle debug camera
fov <value>           - Set field of view
```

### Time

```
slomo <value>         - Set time dilation (1.0 = normal)
pause                 - Pause game
```

## Command Aliases

You can create command aliases in DefaultInput.ini:

```ini
[/Script/Engine.InputSettings]
ConsoleKeys=Grave
+ConsoleKeys=Tilde

[ConsoleVariables]
; Add your aliases here
```

## Tips

1. **Tab Completion**: Press Tab to auto-complete commands
2. **Command History**: Use Up/Down arrows to navigate command history
3. **Clear Console**: Type `clear` to clear console output
4. **Help**: Type `help` to see available commands
5. **Batch Commands**: Separate multiple commands with `|`

## Common Command Combinations

### Performance Testing
```
stat fps | stat unit | stat gpu
```

### Debug Visualization
```
DebugShowCollision true | DebugShowNavMesh true | DebugShowAI true
```

### Quick Testing Setup
```
DebugGodMode true | DebugNoClip true | DebugSetSanity 100
```

### Performance Profiling
```
DebugStartProfiling
; ... play game ...
DebugStopProfiling
```

## Troubleshooting

### Command Not Found
- Check spelling
- Ensure CheatManager is enabled
- Verify you're in the correct game mode

### Command Has No Effect
- Check if the system is initialized
- Verify parameters are correct
- Check logs for error messages

### Console Not Opening
- Check keybindings in Input settings
- Try alternative console key (Tilde ~)
- Verify console is not disabled in project settings

## Security Note

Many of these commands are cheats and should be disabled in shipping builds. Use preprocessor directives to exclude debug code:

```cpp
#if !UE_BUILD_SHIPPING
    // Debug code here
#endif
```
