# Priority Policy

## P0

Immediate command attention. P0 means the project cannot be trusted until addressed.

Examples:

- dirty git threatens correctness of commits.
- player save data can be corrupted or resurrect stale progress.
- active regression tests are deleted.
- main game flow cannot be verified.

Required:

- War Room entry.
- Risk Register entry.
- Brain Council review.
- owner/reviewer/QA/governance signers.

## P1

High priority. Blocks release readiness or reliable validation.

Examples:

- build cannot run.
- Build.cs dependency missing.
- packaged runtime data missing.
- high-risk feature lacks tests.

## P2

Important quality issue. Degrades experience or maintainability.

Examples:

- missing optional UI asset with native fallback.
- stale docs.
- package bloat.

## P3

Useful cleanup or polish.

P3 cannot interrupt P0/P1 unless bundled with the same owner and no extra risk.

