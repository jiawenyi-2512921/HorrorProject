# Director Formula

## Working formula

`Intensity = clamp((SanityLoss * 0.30) + (DeathsLastMinute * 0.25) + (EncounterRecency * 0.25) + (PlayerVelocityStress * 0.20), 0, 1)`

## Purpose

Control pacing, audiovisual pressure, NPC emotional tone, and encounter escalation.

## Signals

- `SanityLoss`: how unstable the player state is becoming
- `DeathsLastMinute`: repeated failure pressure
- `EncounterRecency`: how recently a threat was confirmed
- `PlayerVelocityStress`: panic movement and escape behavior

## Outputs

- director tag
- VHS intensity
- audio pressure
- NPC prompt tone bias
