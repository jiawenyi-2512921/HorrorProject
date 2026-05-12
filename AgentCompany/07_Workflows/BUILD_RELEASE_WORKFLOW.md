# Build Release Workflow

## Build Gate

Before release or completion claims:

- UE Editor Live Coding must not block command-line build.
- Build command must be recorded.
- Exit code and relevant output must be recorded.
- Build.cs dependency changes must be reviewed by Engineering.

## Packaging Gate

- MapsToCook reviewed.
- DirectoriesToAlwaysCook reviewed for package bloat.
- NonUFS staging reviewed for raw JSON/movies/etc.
- LFS status checked for binary assets.
- Missing hardcoded assets resolved or risk-accepted.

## Release Verdict

- `GO`: all gates pass.
- `GO_WITH_RISK`: only P2/P3 accepted risks remain.
- `NO_GO`: any P0/P1 open or verification blocked.

