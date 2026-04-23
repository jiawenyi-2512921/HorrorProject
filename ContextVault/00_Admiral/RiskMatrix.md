# Risk Matrix

| ID | Risk | Probability | Impact | Score | Owner | Mitigation | Trigger |
|---|---|---:|---:|---:|---|---|---|
| R-01 | Scope grows faster than implementation capacity | 3 | 3 | 9 | ADMIRAL | freeze scope at NorthStar, reject non-core work | new feature request outside core loop |
| R-02 | Architecture drift from rapid implementation | 2 | 3 | 6 | ORACLE | mandatory interface gate, ADR for boundary changes | new classes or subsystems appear without review |
| R-03 | AI feature quality slips due to hidden prompt/runtime state | 2 | 3 | 6 | HIVEMIND | debug HUD, prompt versioning, runtime log | inconsistent NPC or director output |
| R-04 | Performance collapse from VHS, lighting, or PCG | 3 | 3 | 9 | REAPER | perf budgets, scoped profiling, parameter caps | GPU or GameThread exceeds budget |
| R-05 | Content freeze happens too late | 2 | 2 | 4 | MUSE | lock story and puzzle scope by M1 | new narrative branches after Day 7 |
| R-06 | Packaging or regression failures discovered too late | 2 | 3 | 6 | REAPER | regular package cadence, automated smoke checks | failed build or broken regression after Day 14 |

## Escalation rule

Any row with `Score > 6` requires immediate review and a new EventBus entry.
