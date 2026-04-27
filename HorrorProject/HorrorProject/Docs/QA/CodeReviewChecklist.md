# Code Review Checklist

**Project:** HorrorProject  
**Version:** 1.1  
**Last Updated:** 2026-04-27

## Overview

Use this checklist for every code, content, and tooling change. Mark each item as passed, not applicable, or blocked with an owner and validation command.

## Build And Validation

- [ ] Game target compiles with zero warnings.
- [ ] Editor target compiles with zero warnings.
- [ ] The closest validation script passed.
- [ ] Project health validation passed for changes that affect packaging, assets, or configuration.
- [ ] Generated reports were refreshed when the change affects their inputs.

## Correctness

- [ ] Null UObject references are guarded with `IsValid` or an equivalent lifetime check.
- [ ] `GetWorld()` results are checked before use outside guaranteed actor/component lifetimes.
- [ ] Array, map, and save-data access handles missing or invalid keys.
- [ ] External input is validated before use.
- [ ] Error paths log enough context to reproduce the failure.

## Unreal Ownership

- [ ] UObject references that must survive garbage collection use `UPROPERTY`.
- [ ] Runtime UObject references use `TObjectPtr` or `TWeakObjectPtr` as appropriate.
- [ ] Delegates are unbound during teardown when the owner can outlive the target.
- [ ] Asset renames are done through editor-safe asset APIs, not filesystem moves.

## Performance

- [ ] No expensive world searches or allocations were added to per-frame paths.
- [ ] Constants and budgets are named instead of embedded as unexplained literals.
- [ ] Asset loads use soft references when synchronous loading is not required.
- [ ] The change preserves frame-time, memory, and package-size budgets.

## Thread Safety

- [ ] Shared mutable state is protected.
- [ ] Async callbacks verify object validity before touching UObjects.
- [ ] Work scheduled from background threads returns to the game thread before using Unreal object APIs.

## Blueprint And Editor Integration

- [ ] Blueprint-callable APIs have clear categories and stable display names.
- [ ] Public properties expose only designer-safe controls.
- [ ] Editor-only code stays in editor modules or editor guards.
- [ ] Tooling scripts fail loudly on real errors and keep advisory output separate.

## Documentation

- [ ] Public contracts and workflow changes are documented.
- [ ] Markdown links validate locally.
- [ ] Screenshots or diagrams linked by documentation exist in the repository.
- [ ] Follow-up comments include an issue ID and owner.

## Security And Privacy

- [ ] File paths are normalized and constrained to expected project locations.
- [ ] Secrets, machine-local tokens, and personal data are not written to logs or reports.
- [ ] Save data and imported text are parsed with structured parsers.

## Release Readiness

- [ ] The change is reversible or has a migration note.
- [ ] Known risk is listed with impact and mitigation.
- [ ] Packaging validation is clean when release artifacts are affected.
