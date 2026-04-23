# EQS Queries

| Query | Purpose | Key tests |
|---|---|---|
| `EQS_Golem_LastKnownLocation` | reacquire suspicious target | distance, path length |
| `EQS_Golem_SearchSpots` | search behavior around clue location | visibility, navigability |
| `EQS_Golem_ChasePosition` | optimize chase approach | path length, line of sight |
| `EQS_Spawn_AnomalyAnchor` | choose procedural anomaly points | anchor proximity, exclusion radius |

## Review rules

- every query must have a plain-language intent sentence
- every nontrivial score must be documented
