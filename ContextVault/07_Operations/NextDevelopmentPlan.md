# Next Development Plan

Date: 2026-04-26
Project: SM-13 / HorrorProject
Stage: Phase 0 stabilization closeout, development day 3 after D0 baseline
Deadline: 2026-05-14 hard delivery, 2026-05-15 to 2026-05-17 contingency

## Current Baseline

- Code systems are in a strong prototype-to-alpha state: native gameplay loops, save/import, event metadata, archive UI contracts, route validation, VHS/camera feedback, threat lifecycle, and interaction guardrails have automation coverage.
- The project is not yet content-complete: binary UE assets/maps still need editor-owned cleanup, reference fixing, map placement, lighting/audio polish, and packaged smoke testing.
- The main plan deviation is asset/editor integration lag. Code-only packets are ahead of a normal Phase 0 baseline, but map/Blueprint hygiene is behind because `/Game/FirstPerson` was deleted while active assets still reference it.

## P0 Stabilization Queue

1. Keep governance files live: `.fleet/**` must stay present because README uses it as the project operating source.
2. Keep local/runtime state out of git: `.claude/**`, UE `Binaries`, `Intermediate`, `Saved`, `.vs`, and `DerivedDataCache` are generated or local-only.
3. Maintain dashboard safety: LiveStatus must render status events with DOM/textContent only and whitelist event level classes.
4. Lock rotated encounter placement: authored threat transforms must preserve director rotation/scale and remain covered by automation.

## P1 Editor Integration Queue

1. Open the UE project in the editor.
2. Enable Show Redirectors and run Fix Up Redirectors on `/Game`.
3. Resolve every remaining `/Game/FirstPerson/*` reference in Reference Viewer.
4. Resave active maps and Blueprints, especially `Variant_Horror/Lvl_Horror` and `BP_HorrorPlayerCharacter`.
5. Place and verify the DeepWaterStation route kit in the playable map.
6. Run PIE smoke: start, acquire camera/VHS, record first anomaly, trigger encounter, resolve route gate, reach exit.

## P2 Content And Release Queue

1. Replace placeholder FirstPerson-era materials/animation dependencies with SM-13-owned assets.
2. Add authored lighting/audio beats for the first loop and encounter reveal.
3. Run packaging and startup smoke on a clean build output.
4. Add a release checklist: map load, no missing package warnings, no automation errors, packaged executable smoke, archive/save round trip.

## Quality Gates

- G0: `git status --short` has no accidental deleted governance files or generated runtime directories.
- G1: Python dashboard tests and JSON/py compile checks pass.
- G2: `HorrorProjectEditor Win64 Development` build passes.
- G3: focused automation for touched systems passes.
- G4: full `Automation RunTests HorrorProject` regression passes.
- G5: editor reference cleanup and packaged smoke pass before claiming alpha-ready.

## Next Recommended Packet

After this stabilization packet is validated, schedule `SM13-20260426-EDITORREFS` as the next blocking packet. Its owner must use the Unreal Editor, not only source search, because the remaining FirstPerson dependency evidence is inside binary assets/maps.
