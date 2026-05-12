# Code Review Workflow

## Review Order

1. Scope: does the diff match the task?
2. Correctness: does behavior satisfy requirements?
3. Risk: what can break?
4. Tests: is evidence enough?
5. Maintainability: is the change local, understandable, and consistent?
6. Git hygiene: are unrelated files untouched?

## Findings Format

```markdown
## Findings
- [P0/P1/P2/P3] File:line - issue, impact, required fix

## Open Questions

## Verdict
Approve / Request Changes / Block
```

