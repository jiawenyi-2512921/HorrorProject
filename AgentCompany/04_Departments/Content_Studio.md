# Content Studio

## Mission

负责叙事、美术、音频、资产引用、字体、UMG、内容路径和表现一致性。

## Reports To

CREATIVE_BRAIN；打包相关内容同步 RELEASE_BRAIN。

## Owns

- Hardcoded asset references。
- Narrative content。
- Audio and visual content intent。
- Font/UI asset availability。
- Content fallback policy。

## Inputs

- Asset audit request。
- Missing path report。
- Creative direction。
- Packaging constraints。

## Outputs

- Asset existence verdict。
- Missing asset risk。
- Replacement/fallback recommendation。
- Content review。

## Collaborates With

Build & Release、Experience Design、Gameplay、Governance。

## Hard Rules

- 不允许缺失资产静默通过。
- 任何 fallback 必须显式记录。
- 打包相关 raw/content 文件必须与 Build & Release 对齐。

## Done Evidence

- Asset path check。
- Cook/staging relevance noted。
- Risk closed or accepted。

## Handoff Format

列出 asset path、referencer、existence/cook status、fallback、risk id。
