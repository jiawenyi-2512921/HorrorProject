# Class Registry

| Class | Layer | Owner | Status | Notes |
|---|---|---|---|---|
| `UArchiveGameEventSubsystem` | Systems | ORACLE / FORGE | planned | central event publication and subscription |
| `AAbyssArchiveGameMode` | Gameplay | FORGE | planned | scenario bootstrap and milestone slice wiring |
| `AAbyssArchivePlayerCharacter` | Gameplay | FORGE | planned | player verbs, camera, interaction entrypoints |
| `UInteractionComponent` | Gameplay | FORGE | planned | common interaction scan, focus, use |
| `UInventorySubsystem` | Systems | FORGE | planned | archive items, keys, puzzle evidence |
| `USaveSubsystem` | Infrastructure | FORGE | planned | checkpoints, state persistence, settings |
| `UAIDirectorSubsystem` | Systems | HIVEMIND | planned | intensity, pacing, encounter pressure |
| `AGolemAIController` | Systems | HIVEMIND | planned | enemy controller and behavior coordination |
| `UTemporalRewindComponent` | Systems | LABYRINTH | planned | bounded rewind buffer for temporal mechanics |
| `UVHSEffectComponent` | Presentation | SPECTER | planned | per-state VHS effect control |
| `UAudioDirectorSubsystem` | Presentation | SPECTER | planned | global tension and reactive audio cues |

## Review rule

No class enters this table without an owner, a purpose statement, and a one-line defense.
