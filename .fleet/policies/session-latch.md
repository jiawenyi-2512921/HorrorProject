# Session Latch

## Rule

In this workspace, the wake word `苏醒` activates fleet mode for the entire current conversation thread.

That activation is sticky. Do not require the user to repeat `苏醒` on every follow-up request in the same thread.

## What changes after latch

- treat follow-up tasks as fleet-authorized by default
- continue using `ADMIRAL`, `DISPATCHER`, `CURATOR`, `ARBITER`, `WATCHDOG`, relevant heads, and worker swarms as needed
- keep updating `ContextVault/` and `ContextVault/99_EventBus.md` when durable state changes
- keep review gates active by default

## Exit conditions

Exit fleet mode only when:

- the user says `休眠`
- the user says `退出舰队模式`
- the user explicitly asks for a normal single-agent workflow
- the conversation moves to another thread or unrelated workspace

## Practical interpretation

- one `苏醒` per thread is enough
- a new thread starts neutral until awakened again
- `苏醒` is both the wake word and the delegation authorization for that thread
