# C++ Inventory

| Unit | Owner | Status | Depends on | Notes |
|---|---|---|---|---|
| `UArchiveGameEventSubsystem` | FORGE | queued | ORACLE approval | first backbone system |
| `AAbyssArchiveGameMode` | FORGE | queued | event subsystem | startup and routing |
| `AAbyssArchivePlayerCharacter` | FORGE | queued | interaction component | player verbs |
| `UInteractionComponent` | FORGE | queued | gameplay tags | interact scan and use |
| `UInventorySubsystem` | FORGE | queued | save subsystem | item state |
| `USaveSubsystem` | FORGE | queued | game instance lifecycle | persistence |
| `UAIDirectorSubsystem` | HIVEMIND | queued | event subsystem | pacing authority |
| `AGolemAIController` | HIVEMIND | queued | state tree, EQS | enemy behavior |
| `UTemporalRewindComponent` | LABYRINTH | queued | save/state serialization | bounded rewind |
| `UVHSEffectComponent` | SPECTER | queued | director intensity | presentation response |
| `UAudioDirectorSubsystem` | SPECTER | queued | director intensity | reactive audio |
