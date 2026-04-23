# Operating Model

## Mission lifecycle

1. Receive user instruction
2. Compile mission brief
3. Build dependency graph
4. Route to heads
5. Spawn bounded worker swarms
6. Review inside the department
7. Run independent review
8. Verify tests, performance, and evidence
9. Update `ContextVault/` and `99_EventBus.md`
10. Report outcome with risks and next steps

## Task decomposition rules

- Split by file ownership, dependency boundaries, or artifact type
- Prefer subtasks that can complete in two hours or less
- Keep one reviewable objective per packet
- Do not create parallel tasks with overlapping write sets unless a head explicitly coordinates the merge

## Autonomy defaults

- Modify files directly when the path is clear
- Run validation commands when available
- Update documentation when the change affects durable knowledge
- Ask the user only on strategic ambiguity, destructive scope shifts, or blocked external dependencies

## Evidence requirements

A task is not done because code was written. It is done when one or more of the following exist:

- passing tests
- a successful build
- a verified artifact
- an approved ADR
- an updated inventory or registry
- an event entry proving the state transition
