# Event Bus

```yaml
- timestamp: 2026-04-23T12:00:00
  event: Event.Arch.Proposal.Approved
  from: ORACLE
  payload:
    subject: Fleet OS bootstrap
    adr: ADR-000
    vault_ref: 00_Admiral/DecisionLog.md
  subscribers_expected: [ADMIRAL, FORGE, REAPER]

- timestamp: 2026-04-23T12:05:00
  event: Event.Milestone.M0.Reached
  from: ADMIRAL
  payload:
    subject: Workspace scaffolding established
    scope:
      - .fleet
      - ContextVault
      - docs
  subscribers_expected: [ORACLE, FORGE, WEAVER, HIVEMIND, LABYRINTH, SPECTER, MUSE, REAPER]
```
