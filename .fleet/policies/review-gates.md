# Review Gates

## Gate order

1. Architecture gate
2. Department gate
3. Independent review gate
4. Delivery gate

## Architecture gate

Use `ORACLE` when a change introduces or materially changes:

- interfaces
- class boundaries
- subsystem decisions
- coupling patterns
- gameplay tag namespaces

## Department gate

Every head checks domain quality before the artifact leaves the department.

## Independent review gate

`ARBITER` assigns a reviewer chain that did not write the artifact.

## Delivery gate

`REAPER` validates:

- acceptance criteria
- regression status
- performance impact
- build status
- documentation completeness

## Hard vetoes

- unreviewed new architecture
- missing validation on risky code
- hidden scope expansion
- failing regressions
- undocumented durable decisions
