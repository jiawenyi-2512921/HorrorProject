# Compilation Issues

Generated: 2026-04-27 01:17:14

## Blocking Build Issues

- None reported by this static report generator. Run Scripts\Validation\ValidateCompilation.ps1 and Scripts\Validation\ValidateCompilation.ps1 -EditorOnly for fresh build proof.

## Known Engineering Debt

- Legacy automation tests are disabled behind HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS=0.
- PowerShell syntax scan reports 370 syntax errors across 38 files.
- Several older utility scripts still contain machine-specific paths and should be normalized to shared validation helpers or project-root-relative defaults.

## Quality Gate Direction

- Compilation must remain zero-warning for Game and Editor targets.
- Script syntax issues should trend to zero before these tools are used as release gates.
- Re-enable automation tests only after each suite is migrated and verified.
