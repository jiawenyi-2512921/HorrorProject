# Coding Standard

## Core rules

- Follow Unreal naming and macro conventions
- Prefer forward declarations in headers when possible
- Keep public surface area minimal
- Use `const` consistently
- Avoid `using namespace std`
- Favor engine-managed lifecycle patterns over raw global state
- Add concise logs only at meaningful diagnostic points

## Review focus

- ownership and lifetime
- thread safety
- subsystem boundaries
- Blueprint exposure discipline
- testability
