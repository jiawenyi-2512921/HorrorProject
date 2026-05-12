# Quality Gates

## Universal Completion Gate

A task cannot be Done unless all are true:

- Owner is named.
- Reviewer is named.
- QA signer is named.
- Governance signer is named.
- Changed files are listed.
- Verification commands and outputs are recorded.
- Residual risks are listed or explicitly “none found”.
- Git status impact is explained.

## Build Gate

Blocked if:

- Build not run and no reason recorded.
- Live Coding blocks build.
- UBT/UHT errors exist.
- module dependency risk is unresolved.

## Test Gate

Blocked if:

- active tests were deleted without replacement or formal retirement.
- relevant tests were skipped without risk registration.
- no regression path exists for high-risk changes.

## Runtime Gate

Blocked if high-risk gameplay paths lack verification:

- New Game / Continue.
- Save / load / checkpoint.
- input mode / pause.
- map travel.
- asset load fallback.

## Governance Verdict

- `APPROVED`: may mark done.
- `APPROVED_WITH_REGISTERED_RISK`: may mark done only with risk id.
- `REJECTED`: cannot mark done.

