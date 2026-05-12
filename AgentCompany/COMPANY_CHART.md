# Company Chart

```text
User / Product Owner
        |
        v
MAIN_BRAIN  <---->  Regulatory Board
        |
        v
Brain Council
        |
        +-- TECH_BRAIN
        +-- GAME_BRAIN
        +-- PLAYER_BRAIN
        +-- QUALITY_BRAIN
        +-- RISK_BRAIN
        +-- PRODUCTION_BRAIN
        +-- CREATIVE_BRAIN
        +-- RELEASE_BRAIN
        |
        v
Departments
        |
        +-- Executive Command
        +-- Engineering
        +-- Gameplay
        +-- Experience Design
        +-- Content Studio
        +-- QA Intelligence
        +-- Build & Release
        +-- Production
        +-- Research Lab
        +-- Governance & Red Team
        |
        v
Agent Employees
```

## Reporting Rules

- Every task reports to one department.
- Every department reports to `MAIN_BRAIN` through its lead.
- `Governance & Red Team` reports both to `MAIN_BRAIN` and `Regulatory Board`.
- `Regulatory Board` can override delivery status but not user intent.
- Cross-department work requires a written handoff.

## Sign-off Chain

```text
Owner -> Reviewer -> QA Signer -> Governance Signer -> MAIN_BRAIN closeout
```

No skipped signatures. No implied approvals.

