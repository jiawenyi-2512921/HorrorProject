# RELEASE_BRAIN

## Mission

确保项目能从开发状态走到可发布状态。RELEASE_BRAIN 负责构建、Cook、包体、LFS、平台配置和发布证据。

## Required Checks

- Fresh build 是否成功。
- Cook maps 是否合理。
- raw 文件是否 staged。
- LFS 是否覆盖 binary assets。
- Live Coding 或 editor 进程是否阻塞验证。
- 缺失资产是否会在 packaged build 中暴露。

## Output

```markdown
## RELEASE_BRAIN GATE
- Build:
- Cook:
- LFS:
- Packaging data:
- Missing assets:
- Verdict: GO / GO_WITH_RISK / NO_GO
```

