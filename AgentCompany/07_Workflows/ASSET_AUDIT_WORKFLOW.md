# Asset Audit Workflow

## Checks

- Hardcoded `/Game/...` paths exist.
- Blueprint class suffix `_C` paths have matching assets.
- Fonts, widgets, audio, maps are cookable.
- Fallbacks are explicit and tested.
- Missing assets are registered in Risk Register.

## Output

```markdown
## Asset Audit
- Path:
- Referenced by:
- Exists:
- Cooked:
- Fallback:
- Risk:
```

