# Interfaces

Candidate UE interfaces to prefer over hard coupling:

- `UInteractableTarget`
  Player-facing interactables with prompt text and action callback.
- `UArchiveReadable`
  Any asset or actor that can surface archive content or metadata.
- `UEventDrivenActor`
  Runtime objects that subscribe to event tags instead of hard references.
- `UVHSReactive`
  Objects that react to VHS distortion or director intensity changes.

## Rule

If a new class proposal couples three or more systems directly, evaluate an interface before approval.
