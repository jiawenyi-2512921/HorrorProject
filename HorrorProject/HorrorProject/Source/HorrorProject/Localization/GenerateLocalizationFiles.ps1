# Generate Localization Files for All Languages
# Usage: .\GenerateLocalizationFiles.ps1

param(
    [string]$InputPath = "D:\gptzuo\HorrorProject\HorrorProject\Content\Localization\ExtractedTexts.csv",
    [string]$OutputPath = "D:\gptzuo\HorrorProject\HorrorProject\Content\Localization"
)

Write-Host "Generating localization files from: $InputPath"

$Languages = @{
    "en" = "English"
    "zh-Hans" = "Chinese"
    "ja" = "Japanese"
    "ko" = "Korean"
    "es" = "Spanish"
}

# Create language directories
foreach ($LangCode in $Languages.Keys) {
    $LangDir = Join-Path $OutputPath $LangCode
    if (-not (Test-Path $LangDir)) {
        New-Item -ItemType Directory -Path $LangDir -Force | Out-Null
    }
}

# Load extracted texts
if (Test-Path $InputPath) {
    $ExtractedTexts = Import-Csv -Path $InputPath -Encoding UTF8
    Write-Host "Loaded $($ExtractedTexts.Count) text entries"
} else {
    Write-Host "Input file not found. Creating sample data..."
    $ExtractedTexts = @(
        [PSCustomObject]@{ Key = "UI.MainMenu.Start"; Text = "Start Game" }
        [PSCustomObject]@{ Key = "UI.MainMenu.Options"; Text = "Options" }
        [PSCustomObject]@{ Key = "UI.MainMenu.Quit"; Text = "Quit" }
        [PSCustomObject]@{ Key = "UI.Gameplay.Health"; Text = "Health" }
        [PSCustomObject]@{ Key = "UI.Gameplay.Stamina"; Text = "Stamina" }
        [PSCustomObject]@{ Key = "UI.Gameplay.Objective"; Text = "Objective" }
        [PSCustomObject]@{ Key = "Game.Tutorial.Movement"; Text = "Use WASD to move" }
        [PSCustomObject]@{ Key = "Game.Tutorial.Interact"; Text = "Press E to interact" }
        [PSCustomObject]@{ Key = "Game.Message.ItemFound"; Text = "Item Found" }
        [PSCustomObject]@{ Key = "Game.Message.SaveComplete"; Text = "Game Saved" }
    )
}

# Sample translations (in production, these would come from translators)
$Translations = @{
    "zh-Hans" = @{
        "UI.MainMenu.Start" = "开始游戏"
        "UI.MainMenu.Options" = "选项"
        "UI.MainMenu.Quit" = "退出"
        "UI.Gameplay.Health" = "生命值"
        "UI.Gameplay.Stamina" = "耐力"
        "UI.Gameplay.Objective" = "目标"
        "Game.Tutorial.Movement" = "使用WASD移动"
        "Game.Tutorial.Interact" = "按E键交互"
        "Game.Message.ItemFound" = "发现物品"
        "Game.Message.SaveComplete" = "游戏已保存"
    }
    "ja" = @{
        "UI.MainMenu.Start" = "ゲーム開始"
        "UI.MainMenu.Options" = "オプション"
        "UI.MainMenu.Quit" = "終了"
        "UI.Gameplay.Health" = "体力"
        "UI.Gameplay.Stamina" = "スタミナ"
        "UI.Gameplay.Objective" = "目標"
        "Game.Tutorial.Movement" = "WASDで移動"
        "Game.Tutorial.Interact" = "Eキーで操作"
        "Game.Message.ItemFound" = "アイテム発見"
        "Game.Message.SaveComplete" = "セーブ完了"
    }
    "ko" = @{
        "UI.MainMenu.Start" = "게임 시작"
        "UI.MainMenu.Options" = "옵션"
        "UI.MainMenu.Quit" = "종료"
        "UI.Gameplay.Health" = "체력"
        "UI.Gameplay.Stamina" = "스태미나"
        "UI.Gameplay.Objective" = "목표"
        "Game.Tutorial.Movement" = "WASD로 이동"
        "Game.Tutorial.Interact" = "E키로 상호작용"
        "Game.Message.ItemFound" = "아이템 발견"
        "Game.Message.SaveComplete" = "게임 저장됨"
    }
    "es" = @{
        "UI.MainMenu.Start" = "Iniciar Juego"
        "UI.MainMenu.Options" = "Opciones"
        "UI.MainMenu.Quit" = "Salir"
        "UI.Gameplay.Health" = "Salud"
        "UI.Gameplay.Stamina" = "Resistencia"
        "UI.Gameplay.Objective" = "Objetivo"
        "Game.Tutorial.Movement" = "Usa WASD para moverte"
        "Game.Tutorial.Interact" = "Presiona E para interactuar"
        "Game.Message.ItemFound" = "Objeto Encontrado"
        "Game.Message.SaveComplete" = "Juego Guardado"
    }
}

# Generate JSON files for each language
foreach ($LangCode in $Languages.Keys) {
    $LangName = $Languages[$LangCode]
    $LocalizationData = @{}

    foreach ($Entry in $ExtractedTexts) {
        $Key = $Entry.Key

        if ($LangCode -eq "en") {
            $LocalizationData[$Key] = $Entry.Text
        } elseif ($Translations[$LangCode] -and $Translations[$LangCode][$Key]) {
            $LocalizationData[$Key] = $Translations[$LangCode][$Key]
        } else {
            # Placeholder for missing translations
            $LocalizationData[$Key] = "[TODO: $LangName] $($Entry.Text)"
        }
    }

    # Save as JSON
    $JsonPath = Join-Path (Join-Path $OutputPath $LangCode) "Game.json"
    $LocalizationData | ConvertTo-Json -Depth 10 | Out-File -FilePath $JsonPath -Encoding UTF8
    Write-Host "Generated: $JsonPath ($($LocalizationData.Count) entries)"

    # Also save as CSV for translators
    $CsvPath = Join-Path (Join-Path $OutputPath $LangCode) "Game.csv"
    $CsvData = $LocalizationData.GetEnumerator() | ForEach-Object {
        [PSCustomObject]@{
            Key = $_.Key
            Translation = $_.Value
            Status = if ($_.Value -like "[TODO:*") { "Pending" } else { "Complete" }
        }
    }
    $CsvData | Export-Csv -Path $CsvPath -NoTypeInformation -Encoding UTF8
    Write-Host "Generated: $CsvPath"
}

# Generate master localization table
$MasterTable = @()
foreach ($Entry in $ExtractedTexts) {
    $Row = [PSCustomObject]@{
        Key = $Entry.Key
        English = $Entry.Text
        Chinese = if ($Translations["zh-Hans"][$Entry.Key]) { $Translations["zh-Hans"][$Entry.Key] } else { "" }
        Japanese = if ($Translations["ja"][$Entry.Key]) { $Translations["ja"][$Entry.Key] } else { "" }
        Korean = if ($Translations["ko"][$Entry.Key]) { $Translations["ko"][$Entry.Key] } else { "" }
        Spanish = if ($Translations["es"][$Entry.Key]) { $Translations["es"][$Entry.Key] } else { "" }
    }
    $MasterTable += $Row
}

$MasterPath = Join-Path $OutputPath "MasterLocalizationTable.csv"
$MasterTable | Export-Csv -Path $MasterPath -NoTypeInformation -Encoding UTF8
Write-Host "Generated master table: $MasterPath"

Write-Host "`nLocalization files generated successfully!"
Write-Host "Next step: Run ValidateTranslations.ps1 to check for issues"
