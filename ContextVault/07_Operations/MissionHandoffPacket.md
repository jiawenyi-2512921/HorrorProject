# Mission Handoff Packet

Date: 2026-04-26
Scope: agent-to-agent and agent-to-main-thread handoffs

## Mission Brief Template

```yaml
mission_id: SM13-YYYYMMDD-NN
created_by: DISPATCHER
assigned_to: ROLE_OR_AGENT
mode: read-only | write | validation | editor-action
priority: P0 | P1 | P2
northstar_link: short reason this matters
context:
  current_state:
    - fact with source path if possible
  recent_validation:
    - command/test/result
scope:
  include:
    - exact file/path/topic
  exclude:
    - exact file/path/topic
success_criteria:
  - observable completion condition
constraints:
  - no binary asset edits outside UE Editor
  - preserve unrelated changes
  - one writer per file set
expected_return:
  - root cause or implementation summary
  - touched/inspected files
  - validation evidence or blocker
```

## Completion Report Template

```yaml
mission_id: SM13-YYYYMMDD-NN
status: passed | failed | blocked | no-op
summary: one sentence
files_touched:
  - path
files_inspected:
  - path
validation:
  build: command + exit code | not run + reason
  tests:
    - name + exit code | not run + reason
findings:
  - concise technical fact
risks:
  - new or changed risk
next_recommendation:
  - exact next packet
```

## EventBus Entry Template

```yaml
- timestamp: YYYY-MM-DDTHH:MM:SS
  event: Event.Packet.SM13.Name.Started|Passed|Failed|Blocked
  from: ROLE
  payload:
    subject: concise packet summary
    scope:
      - item
    validation:
      - command/result
    blockers:
      - item if any
  subscribers_expected: [ADMIRAL, DISPATCHER, REAPER, WATCHDOG, CURATOR]
```

## Handoff Rules

1. A handoff must be understandable without reading chat history.
2. File paths must be exact and relative to repo/workspace when possible.
3. If validation was interrupted by user, record it as `blocked:user-interrupted-tool-use`.
4. If an agent only audited, do not imply implementation happened.
5. If a report recommends Editor work, include the exact asset path and safe Editor action.

## Blocker Labels

| Label | Meaning |
|---|---|
| `blocked:user-decision` | requires aesthetic/scope decision |
| `blocked:editor-binary` | requires UE Editor resave/fix-up |
| `blocked:validation` | build/test failed or was interrupted |
| `blocked:file-ownership` | another active lane owns the file set |
| `blocked:external-resource` | requires Marketplace/Fab/import/system access |

## Minimum Useful Handoff

If time is short, provide:

1. packet name;
2. changed/inspected files;
3. validation status;
4. next exact action.
