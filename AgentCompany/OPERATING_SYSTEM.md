# AgentCompany Operating System

## Operating Doctrine

AgentCompany runs as a high-concurrency, evidence-first 3A studio. It assumes abundant agents and tokens, so the constraint is not effort. The constraint is coordination quality.

## Work Lifecycle

1. Intake: User request or project risk enters `TASK_BOARD.md`.
2. Triage: `MAIN_BRAIN` assigns priority and required brains.
3. Council: P0/P1 tasks go through `BRAIN_COUNCIL.md`.
4. Dispatch: Owner, reviewer, QA signer, governance signer are named.
5. Execution: Department agent works inside assigned scope.
6. Handoff: Any context transfer uses the handoff template.
7. Review: Reviewer checks scope and correctness.
8. QA: QA checks evidence and regression coverage.
9. Governance: Regulatory board checks gates and risks.
10. Closeout: MAIN_BRAIN closes only after signatures.

## Priority Classes

- P0: Blocks trust, player data, build credibility, or core game flow.
- P1: Blocks release readiness or high-risk systems.
- P2: Degrades content, UX, maintainability, or packaged quality.
- P3: Cleanup, polish, docs, opportunistic improvements.

## Evidence Levels

- E0: No evidence. Cannot complete.
- E1: Static inspection only. Useful for planning, not final delivery.
- E2: Targeted command or test. Acceptable for narrow changes.
- E3: Relevant build/test suite plus review. Required for P1.
- E4: Full release gate evidence. Required for release decisions.

## Company Memory

Important facts must be promoted from conversation into files:

- decisions -> `06_CommunicationBus/DECISION_RECORD_TEMPLATE.md`
- active risks -> `08_Governance/RISK_REGISTER.md`
- current blockers -> `10_Ops/BLOCKERS.md`
- known project facts -> `09_KnowledgeBase/PROJECT_FACTS.md`
- current work -> `10_Ops/TASK_BOARD.md`

