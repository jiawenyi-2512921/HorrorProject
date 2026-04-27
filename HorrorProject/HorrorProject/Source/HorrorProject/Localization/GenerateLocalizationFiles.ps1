# Generate localization JSON files from extracted text keys.

param(
    [string]$InputPath = "",
    [string]$OutputPath = ""
)

$ErrorActionPreference = "Stop"

$ProjectRoot = Split-Path -Parent (Split-Path -Parent (Split-Path -Parent $PSScriptRoot))
if ([string]::IsNullOrWhiteSpace($InputPath)) {
    $InputPath = Join-Path $ProjectRoot "Content\Localization\ExtractedTexts.csv"
}
if ([string]::IsNullOrWhiteSpace($OutputPath)) {
    $OutputPath = Join-Path $ProjectRoot "Content\Localization"
}

Write-Host "Generating localization files from: $InputPath" -ForegroundColor Cyan

$Languages = [ordered]@{
    "en" = "English"
    "zh-Hans" = "Chinese Simplified"
    "ja" = "Japanese"
    "ko" = "Korean"
    "es" = "Spanish"
}

foreach ($LangCode in $Languages.Keys) {
    $LangDir = Join-Path $OutputPath $LangCode
    New-Item -ItemType Directory -Path $LangDir -Force | Out-Null
}

if (Test-Path -LiteralPath $InputPath) {
    $ExtractedTexts = @(Import-Csv -Path $InputPath -Encoding UTF8)
    Write-Host "Loaded $($ExtractedTexts.Count) text entries"
} else {
    Write-Host "Input file not found. Creating starter localization data." -ForegroundColor Yellow
    $ExtractedTexts = @(
        [PSCustomObject]@{ Key = "UI.MainMenu.Start"; Text = "Start Game" },
        [PSCustomObject]@{ Key = "UI.MainMenu.Options"; Text = "Options" },
        [PSCustomObject]@{ Key = "UI.MainMenu.Quit"; Text = "Quit" },
        [PSCustomObject]@{ Key = "UI.Gameplay.Health"; Text = "Health" },
        [PSCustomObject]@{ Key = "UI.Gameplay.Stamina"; Text = "Stamina" },
        [PSCustomObject]@{ Key = "UI.Gameplay.Objective"; Text = "Objective" },
        [PSCustomObject]@{ Key = "Game.Tutorial.Movement"; Text = "Use WASD to move" },
        [PSCustomObject]@{ Key = "Game.Tutorial.Interact"; Text = "Press E to interact" },
        [PSCustomObject]@{ Key = "Game.Message.ItemFound"; Text = "Item Found" },
        [PSCustomObject]@{ Key = "Game.Message.SaveComplete"; Text = "Game Saved" }
    )
}

$SpanishTranslations = @{
    "UI.MainMenu.Start" = "Iniciar juego"
    "UI.MainMenu.Options" = "Opciones"
    "UI.MainMenu.Quit" = "Salir"
    "UI.Gameplay.Health" = "Salud"
    "UI.Gameplay.Stamina" = "Resistencia"
    "UI.Gameplay.Objective" = "Objetivo"
    "Game.Tutorial.Movement" = "Usa WASD para moverte"
    "Game.Tutorial.Interact" = "Presiona E para interactuar"
    "Game.Message.ItemFound" = "Objeto encontrado"
    "Game.Message.SaveComplete" = "Juego guardado"
}

foreach ($LangCode in $Languages.Keys) {
    $LangName = $Languages[$LangCode]
    $LocalizationData = [ordered]@{}

    foreach ($Entry in $ExtractedTexts) {
        if ([string]::IsNullOrWhiteSpace($Entry.Key)) {
            continue
        }

        if ($LangCode -eq "en") {
            $LocalizationData[$Entry.Key] = $Entry.Text
        } elseif ($LangCode -eq "es" -and $SpanishTranslations.ContainsKey($Entry.Key)) {
            $LocalizationData[$Entry.Key] = $SpanishTranslations[$Entry.Key]
        } else {
            $LocalizationData[$Entry.Key] = "[PENDING_TRANSLATION: $LangName] $($Entry.Text)"
        }
    }

    $JsonPath = Join-Path (Join-Path $OutputPath $LangCode) "Game.json"
    $LocalizationData | ConvertTo-Json -Depth 10 | Set-Content -Path $JsonPath -Encoding UTF8
    Write-Host "Generated: $JsonPath ($($LocalizationData.Count) entries)"
}

Write-Host "Localization generation complete." -ForegroundColor Green
