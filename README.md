# Agent Fleet OS

This workspace is configured for a single-window, high-autonomy delivery model.

## Operating model

- You give one instruction in the main thread.
- The fleet compiles it into a mission brief, routes it across heads and swarms, applies independent review, and writes evidence back to the workspace.
- Architecture, memory hygiene, and delivery gates are enforced through `.fleet/` and `ContextVault/`.

## Source of truth

- Fleet contract: `.fleet/manifest.md`
- Fleet bootstrap: `.fleet/START-HERE.md`
- Fleet prompts: `.fleet/prompts/`
- Fleet policies: `.fleet/policies/`
- Shared templates: `.fleet/templates/`
- Project knowledge base: `ContextVault/`
- Design spec: `docs/superpowers/specs/2026-04-23-agent-fleet-orchestrator-design.md`

## How to use

In future tasks, ask for the fleet explicitly, for example:

- `Use our fleet OS to implement this feature.`
- `Run the agent team on this task in full autonomous mode.`
- `Use the fleet with architecture review and independent QA.`

That wording is intentional. It makes the request unambiguous, which allows aggressive parallel delegation and review.
