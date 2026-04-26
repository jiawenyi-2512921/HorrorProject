# Place Objective Nodes Tool
# Places 8 objective nodes in the level based on design specifications

param(
    [Parameter(Mandatory=$false)]
    [string]$LevelName = "SM13",

    [Parameter(Mandatory=$false)]
    [int]$ObjectiveCount = 8,

    [Parameter(Mandatory=$false)]
    [string]$OutputPath = "D:/gptzuo/HorrorProject/HorrorProject/Content/Maps",

    [Parameter(Mandatory=$false)]
    [switch]$GenerateReport
)

$ErrorActionPreference = "Stop"

Write-Host "=== Place Objective Nodes ===" -ForegroundColor Cyan
Write-Host "Level: $LevelName"
Write-Host "Objectives: $ObjectiveCount"
Write-Host ""

# Objective definitions based on design document
$objectives = @(
    @{
        ID = 1
        Name = "Find the Keycard"
        Zone = "EntryHall"
        Position = @{ X = 500; Y = 200; Z = 120 }
        Type = "Tutorial"
        Difficulty = "Easy"
        IsOptional = $false
        TargetTime = 5
        Beat = 1
        Visibility = "High"
        Description = "Introduce interaction mechanics"
    },
    @{
        ID = 2
        Name = "Restore Power"
        Zone = "MainCorridor"
        Position = @{ X = 1500; Y = 0; Z = 150 }
        Type = "Puzzle"
        Difficulty = "Easy"
        IsOptional = $false
        TargetTime = 13
        Beat = 2
        Visibility = "Medium"
        Description = "Multi-step puzzle, unlocks lighting"
    },
    @{
        ID = 3
        Name = "Collect 3 Documents"
        Zone = "WestWing"
        Position = @{ X = 800; Y = 800; Z = 120 }
        Type = "Collection"
        Difficulty = "Medium"
        IsOptional = $false
        TargetTime = 18
        Beat = 2
        Visibility = "Medium"
        Description = "Exploration reward, narrative delivery"
    },
    @{
        ID = 4
        Name = "Unlock Safe"
        Zone = "WestWing"
        Position = @{ X = 1200; Y = 1000; Z = 420 }
        Type = "Puzzle"
        Difficulty = "Medium"
        IsOptional = $true
        TargetTime = 21
        Beat = 4
        Visibility = "Low"
        Description = "Optional code puzzle, extra resources"
    },
    @{
        ID = 5
        Name = "Find the Specimen"
        Zone = "EastWing"
        Position = @{ X = 1800; Y = -800; Z = 120 }
        Type = "Challenge"
        Difficulty = "Medium"
        IsOptional = $false
        TargetTime = 23
        Beat = 3
        Visibility = "Medium"
        Description = "Stealth/combat challenge, enemy encounter"
    },
    @{
        ID = 6
        Name = "Destroy the Source"
        Zone = "Basement"
        Position = @{ X = 1000; Y = 0; Z = -180 }
        Type = "Crisis"
        Difficulty = "Hard"
        IsOptional = $false
        TargetTime = 32
        Beat = 5
        Visibility = "Low"
        Description = "Peak tension moment, combat/puzzle hybrid"
    },
    @{
        ID = 7
        Name = "Retrieve the Key"
        Zone = "UpperFloor"
        Position = @{ X = 1000; Y = 600; Z = 420 }
        Type = "Exploration"
        Difficulty = "Medium"
        IsOptional = $true
        TargetTime = 34
        Beat = 6
        Visibility = "Medium"
        Description = "Unlocks shortcut to exit"
    },
    @{
        ID = 8
        Name = "Escape"
        Zone = "ExitSequence"
        Position = @{ X = 2200; Y = 0; Z = 120 }
        Type = "Completion"
        Difficulty = "Easy"
        IsOptional = $false
        TargetTime = 35
        Beat = 7
        Visibility = "High"
        Description = "Level completion trigger"
    }
)

# Generate objective placement data
Write-Host "Placing objective nodes..." -ForegroundColor Yellow

$placementData = @{
    LevelName = $LevelName
    ObjectiveCount = $ObjectiveCount
    Objectives = @()
    Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
}

foreach ($obj in $objectives) {
    Write-Host "  - Objective $($obj.ID): $($obj.Name) [$($obj.Zone)]" -ForegroundColor Gray

    $objectiveData = @{
        ID = $obj.ID
        Name = $obj.Name
        Zone = $obj.Zone
        Position = $obj.Position
        Type = $obj.Type
        Difficulty = $obj.Difficulty
        IsOptional = $obj.IsOptional
        TargetTime = $obj.TargetTime
        Beat = $obj.Beat
        Visibility = $obj.Visibility
        Description = $obj.Description
        BlueprintClass = "BP_Objective_$($obj.ID)"
        TriggerRadius = 200
        HighlightEnabled = $true
    }

    $placementData.Objectives += $objectiveData
}

# Calculate spacing
Write-Host ""
Write-Host "Calculating objective spacing..." -ForegroundColor Yellow

for ($i = 0; $i -lt $objectives.Count - 1; $i++) {
    $obj1 = $objectives[$i]
    $obj2 = $objectives[$i + 1]

    $dx = $obj2.Position.X - $obj1.Position.X
    $dy = $obj2.Position.Y - $obj1.Position.Y
    $dz = $obj2.Position.Z - $obj1.Position.Z

    $distance = [Math]::Sqrt($dx * $dx + $dy * $dy + $dz * $dz) / 100

    Write-Host "  - Obj $($obj1.ID) to Obj $($obj2.ID): $([Math]::Round($distance, 1))m" -ForegroundColor Gray
}

# Save placement data
$outputFile = Join-Path $OutputPath "${LevelName}_Objectives.json"

Write-Host ""
Write-Host "Saving objective data..." -ForegroundColor Yellow

$outputDir = Split-Path $outputFile -Parent
if (-not (Test-Path $outputDir)) {
    New-Item -ItemType Directory -Path $outputDir -Force | Out-Null
}

$placementData | ConvertTo-Json -Depth 10 | Set-Content -Path $outputFile -Encoding UTF8

Write-Host "  - Saved to: $outputFile" -ForegroundColor Green

# Generate Unreal Engine import script
$ueScriptPath = Join-Path $OutputPath "${LevelName}_Objectives_Import.py"

$ueScript = @"
# Unreal Engine Python script to place objectives
import unreal
import json

with open(r'$outputFile', 'r') as f:
    data = json.load(f)

editor_level_lib = unreal.EditorLevelLibrary()

for obj in data['Objectives']:
    location = unreal.Vector(
        obj['Position']['X'],
        obj['Position']['Y'],
        obj['Position']['Z']
    )

    # Spawn objective actor
    actor = editor_level_lib.spawn_actor_from_class(
        unreal.Actor,
        location
    )

    if actor:
        label = f'Objective_{obj["ID"]}_{obj["Name"].replace(" ", "")}'
        actor.set_actor_label(label)
        print(f'Placed: {label} at {obj["Zone"]}')

print(f'Placed {len(data["Objectives"])} objectives')
"@

$ueScript | Set-Content -Path $ueScriptPath -Encoding UTF8

Write-Host "  - UE import script: $ueScriptPath" -ForegroundColor Green

# Generate report
if ($GenerateReport) {
    Write-Host ""
    Write-Host "Generating report..." -ForegroundColor Yellow

    $reportPath = Join-Path $OutputPath "${LevelName}_Objectives_Report.md"

    $report = @"
# $LevelName Objectives Report

**Generated**: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")
**Total Objectives**: $ObjectiveCount
**Required**: $($objectives.Where({-not $_.IsOptional}).Count)
**Optional**: $($objectives.Where({$_.IsOptional}).Count)

## Objective List

| ID | Name | Zone | Type | Difficulty | Optional | Time | Beat |
|----|------|------|------|------------|----------|------|------|
"@

    foreach ($obj in $objectives) {
        $optional = if ($obj.IsOptional) { "Yes" } else { "No" }
        $report += "`n| $($obj.ID) | $($obj.Name) | $($obj.Zone) | $($obj.Type) | $($obj.Difficulty) | $optional | $($obj.TargetTime)m | $($obj.Beat) |"
    }

    $report += @"


## Detailed Specifications

"@

    foreach ($obj in $objectives) {
        $report += @"

### Objective $($obj.ID): $($obj.Name)

- **Zone**: $($obj.Zone)
- **Position**: ($($obj.Position.X), $($obj.Position.Y), $($obj.Position.Z))
- **Type**: $($obj.Type)
- **Difficulty**: $($obj.Difficulty)
- **Optional**: $(if ($obj.IsOptional) { "Yes" } else { "No" })
- **Target Time**: $($obj.TargetTime) minutes
- **Beat**: $($obj.Beat)
- **Visibility**: $($obj.Visibility)
- **Description**: $($obj.Description)

"@
    }

    $report += @"

## Import Instructions

1. Run in UE Editor: ``py "$ueScriptPath"``
2. Configure objective blueprints
3. Test objective flow: ``.\TestObjectiveFlow.ps1``
4. Validate layout: ``.\ValidateLevelLayout.ps1``
"@

    $report | Set-Content -Path $reportPath -Encoding UTF8

    Write-Host "  - Report saved: $reportPath" -ForegroundColor Green
}

Write-Host ""
Write-Host "=== Objective Placement Complete ===" -ForegroundColor Cyan
Write-Host "Total: $ObjectiveCount" -ForegroundColor Green
Write-Host "Required: $($objectives.Where({-not $_.IsOptional}).Count)" -ForegroundColor Green
Write-Host "Optional: $($objectives.Where({$_.IsOptional}).Count)" -ForegroundColor Green
