# Risk Matrix

| ID | Risk | Probability | Impact | Score | Owner | Mitigation | Trigger |
|---|---|---:|---:|---:|---|---|---|
| R-01 | Scope grows faster than implementation capacity | 3 | 3 | 9 | ADMIRAL | freeze scope at NorthStar, reject non-core work | new feature request outside core loop |
| R-02 | Architecture drift from rapid implementation | 2 | 3 | 6 | ORACLE | mandatory interface gate, ADR for boundary changes | new classes or subsystems appear without review |
| R-03 | AI feature quality slips due to hidden prompt/runtime state | 2 | 3 | 6 | HIVEMIND | debug HUD, prompt versioning, runtime log | inconsistent NPC or director output |
| R-04 | Performance collapse from VHS, lighting, or PCG | 3 | 3 | 9 | REAPER | perf budgets, scoped profiling, parameter caps | GPU or GameThread exceeds budget |
| R-05 | Content freeze happens too late | 2 | 2 | 4 | MUSE | lock story and puzzle scope by M1 | new narrative branches after Day 7 |
| R-06 | Packaging or regression failures discovered too late | 2 | 3 | 6 | REAPER | regular package cadence, automated smoke checks | failed build or broken regression after Day 14 |

| R-07 | High-concurrency agents overwrite or invalidate each other’s work | 2 | 3 | 6 | DISPATCHER | enforce one writer per file set, lane ownership, and integration sequencing via `07_Operations/ParallelExecutionLanes.md` | two active packets touch the same source path |
| R-08 | Speed pressure weakens validation evidence | 2 | 3 | 6 | REAPER | enforce G0-G5 quality gates and EventBus evidence entries via `07_Operations/QualityGateProtocol.md` | packet marked done without build/test or explicit blocker |

## Escalation rule

Any row with `Score > 6` requires immediate review and a new EventBus entry.
