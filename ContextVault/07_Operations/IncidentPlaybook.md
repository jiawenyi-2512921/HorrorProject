# Incident Playbook

Date: 2026-04-26
Scope: SM-13 agent-company failure handling

## Build Break

1. Stop new write packets in affected territory.
2. REAPER captures command, exit code, first actionable compiler/test error.
3. DISPATCHER assigns one owner to fix; others become read-only.
4. Fix root cause, not hooks/check bypasses.
5. Rebuild same target before broader validation.
6. Record failure and pass evidence if milestone-impacting.

## Automation Regression

1. Classify by domain: Game, Player, UI, Save, AI, EventBus, Editor-only.
2. Run only the failing focused suite first.
3. If multiple independent failures exist, dispatch one read-only investigator per domain.
4. Integrate fixes one at a time.
5. Rerun focused suite, then full regression if shared contracts changed.

## Binary Asset / Map Risk

1. Do not modify `.uasset` or `.umap` through text/shell tools.
2. Record exact asset paths and suspected references.
3. Schedule UE Editor action: Fix Up Redirectors, open map/Blueprint, reassign references, save.
4. After Editor save, run build and relevant smoke/automation.
5. If corruption suspected, stop and preserve current state before further edits.

## Agent Conflict

1. Identify overlapping file territories.
2. Freeze lower-priority writer.
3. Let highest-priority owner finish or revert only its own unneeded work.
4. Rebase/sync by reading actual current files, not relying on agent summaries.
5. Update ConcurrencyBoard.

## Scope Creep

1. Compare request against NorthStar and milestone.
2. If outside core loop, mark `deferred` unless user explicitly overrides.
3. If useful but too large, split into smallest deliverable packet.
4. Record ADR only for persistent direction changes.

## Context Drift

1. Prefer current files, git status, and latest validation over memory or summaries.
2. Remove stale ContextVault entries only when superseded by verified state.
3. Keep EventBus concise; long rationale belongs in operations/ADR docs.
4. Ask for user help only when irreversible/editor/external/shared-state action is required.

## User Tool Interruption

1. Stop the exact rejected tool action.
2. Mark related validation/action as blocked, not failed.
3. Continue with safe non-overlapping work if available.
4. Do not retry the same command unless user reauthorizes or changes instruction.

## Emergency Stop Conditions

- destructive git/file operation requested without explicit confirmation;
- force push/main branch rewrite;
- binary asset corruption suspicion;
- repeated build failures with unclear ownership;
- package/release step that affects shared external state.
