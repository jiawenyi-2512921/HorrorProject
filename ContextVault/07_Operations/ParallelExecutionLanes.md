# Parallel Execution Lanes

Date: 2026-04-26
Scope: SM-13 agent-team coordination

## Lane Map

| Lane | Owner Role | File / Asset Territory | Can Run In Parallel With | Cannot Overlap With |
|---|---|---|---|---|
| Native Gameplay | FORGE + WEAVER | `Source/HorrorProject/Game/**`, gameplay tests | read-only architecture, validation planning | another gameplay writer |
| Player / Camera / VHS | FORGE + MUSE | `Source/HorrorProject/Player/**`, VHS component tests | UI read-only audit, asset recon | route/objective writer if shared tests are changing |
| UI / Archive Bridge | MUSE + FORGE | `Source/HorrorProject/Variant_Horror/UI/**` | gameplay read-only audit | player/VHS writer when delegate contracts change |
| Encounter / Threat | SPECTER + FORGE | `Source/HorrorProject/AI/**`, `HorrorEncounterDirector*` | route-kit read-only audit | route-kit writer when encounter API changes |
| Asset / Level Recon | LABYRINTH + MUSE | `Content/**` read-only, Editor action list | all C++ lanes | direct binary modification outside UE Editor |
| Validation | REAPER | build/test commands, logs, regression plan | all read-only audits | uncoordinated build spam during active compile |
| Operations / Evidence | CURATOR + ADMIRAL | `ContextVault/**`, EventBus, status docs | all implementation lanes | another ContextVault writer |

## Dispatch Packet Format

```yaml
packet_id: SM13-YYYYMMDD-NN
lane: Native Gameplay | UI | Encounter | Asset Recon | Validation | Operations
owner: ROLE
mode: write | read-only | validation
scope:
  include:
    - path/or/topic
  exclude:
    - path/or/topic
success_criteria:
  - concrete observable result
validation:
  required:
    - build or focused test
  optional:
    - full regression
handoff:
  expected_output: summary + touched files + evidence
  eventbus_required: true|false
```

## Parallel Scheduling Rules

1. Start with one write lane and two to five read-only lanes.
2. Increase parallelism only by splitting independent file territories.
3. Do not let validation consume all machine resources while compile is already memory-limited.
4. Prefer agents for broad audits; prefer main thread for synthesis and edits.
5. Integrate worktree outputs one lane at a time, then rerun relevant validation.

## Conflict Handling

- If two lanes need the same file, mark one lane `blocked-by-owner`.
- If a validation failure touches multiple lanes, REAPER groups by failure domain before dispatch.
- If an agent discovers binary asset work, convert it into an Editor action packet, not a shell edit.
- If a packet expands beyond its scope, ADMIRAL either splits it or rejects the expansion.

## Throughput Targets

| Period | Target |
|---|---|
| Active coding loop | 1 main implementation packet + 1 validation packet + 2 read-only audits |
| Burst recon loop | 4-8 read-only agents, no overlapping writes |
| Release hardening | 0 speculative agents, validation-first |

## Output Contract

Every lane report must include:

- what was checked;
- what changed, if anything;
- exact files touched or inspected;
- validation evidence or blocker;
- next recommended packet.
