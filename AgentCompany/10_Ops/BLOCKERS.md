# Blockers

| ID | Severity | Blocker | Evidence | Owner | Unblock Action | Status |
| --- | --- | --- | --- | --- | --- | --- |
| B-001 | P1 | Command-line UE build blocked by active Live Coding | UBT reported active Live Coding mutex | Build & Release | Close editor or disable Live Coding, rerun build | Open |
| B-002 | P0 | Git index/worktree split prevents trustworthy commit | `git status` shows staged and unstaged changes in same files | Production | Audit staged vs worktree hunks | Open |
| B-003 | P0 | Active regression coverage deleted | deleted active test suites | QA Intelligence | Restore or replace tests | Open |

