# Decisions

## Why `GameModeBase` instead of `GameMode`

This project is a single-player horror game, so `GameModeBase` is the better fit. It is lighter than `GameMode` and fully sufficient because the game does not need team logic, voting systems, round management, or other match-style rules.

`GameModeBase` is the better foundation because it is lighter, clearer, and keeps the main rule surface aligned with the real product scope.

Defense soundbite:

`GameModeBase` gives us exactly the lifecycle and authority we need for a single-player horror game, without carrying competitive or round-based baggage we will not use.

## Why player interaction uses a native interface first

The player interaction loop uses `UInteractableInterface` as the primary contract instead of relying on ad-hoc Blueprint function names. Horror interactions need context: who initiated the action, what was hit, and whether the target can currently be used. A native interface gives doors, items, notes, recorders, and anomaly props one stable entry point while still letting designers implement behavior in Blueprint.

Legacy fallbacks are intentionally narrow. The current prototype door asset has no explicit interaction function, so `UInteractionComponent` can play its `Door Control` timeline as a temporary compatibility path. New interactables should implement the interface instead of depending on timeline names or reflection.
