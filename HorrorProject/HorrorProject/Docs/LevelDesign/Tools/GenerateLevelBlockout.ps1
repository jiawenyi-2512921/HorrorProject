# Generate Level Blockout Tool
# Creates basic level geometry and structure for SM13

param(
    [Parameter(Mandatory=$false)]
    [string]$LevelName = "SM13",

    [Parameter(Mandatory=$false)]
    [int]$TotalArea = 1300,

    [Parameter(Mandatory=$false)]
    [string]$OutputPath = "D:/gptzuo/HorrorProject/HorrorProject/Content/Maps",

    [Parameter(Mandatory=$false)]
    [switch]$GenerateReport
)

$ErrorActionPreference = "Stop"

Write-Host "=== Generate Level Blockout ===" -ForegroundColor Cyan
Write-Host "Level: $LevelName"
Write-Host "Total Area: ${TotalArea}m²"
Write-Host ""

# Zone definitions based on SM13 design
$zones = @(
    @{
        Name = "EntryHall"
        Area = 150
        Percentage = 11.5
        Dimensions = @{ Length = 15; Width = 10; Height = 4 }
        Connections = @("MainCorridor")
        Type = "Safe"
    },
    @{
        Name = "MainCorridor"
        Area = 200
        Percentage = 15.4
        Dimensions = @{ Length = 40; Width = 5; Height = 3.5 }
        Connections = @("EntryHall", "WestWing", "EastWing", "Basement", "UpperFloor")
        Type = "Transition"
    },
    @{
        Name = "WestWing"
        Area = 250
        Percentage = 19.2
        Dimensions = @{ Length = 25; Width = 10; Height = 3 }
        Connections = @("MainCorridor", "UpperFloor", "Basement")
        Type = "Exploration"
    },
    @{
        Name = "EastWing"
        Area = 250
        Percentage = 19.2
        Dimensions = @{ Length = 25; Width = 10; Height = 3 }
        Connections = @("MainCorridor", "Basement", "UpperFloor")
        Type = "Danger"
    },
    @{
        Name = "Basement"
        Area = 200
        Percentage = 15.4
        Dimensions = @{ Length = 20; Width = 10; Height = 2.5 }
        Connections = @("MainCorridor", "WestWing", "EastWing")
        Type = "Crisis"
    },
    @{
        Name = "UpperFloor"
        Area = 150
        Percentage = 11.5
        Dimensions = @{ Length = 15; Width = 10; Height = 3 }
        Connections = @("MainCorridor", "WestWing", "EastWing", "ExitSequence")
        Type = "Optional"
    },
    @{
        Name = "ExitSequence"
        Area = 100
        Percentage = 7.7
        Dimensions = @{ Length = 20; Width = 5; Height = 3 }
        Connections = @("Basement", "UpperFloor")
        Type = "Climax"
    }
)

# Generate blockout data
Write-Host "Generating zone blockouts..." -ForegroundColor Yellow

$blockoutData = @{
    LevelName = $LevelName
    TotalArea = $TotalArea
    Zones = @()
    Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
}

foreach ($zone in $zones) {
    Write-Host "  - $($zone.Name): $($zone.Area)m² ($($zone.Percentage)%)" -ForegroundColor Gray

    $zoneData = @{
        Name = $zone.Name
        Area = $zone.Area
        Percentage = $zone.Percentage
        Dimensions = $zone.Dimensions
        Connections = $zone.Connections
        Type = $zone.Type
        Volume = $zone.Dimensions.Length * $zone.Dimensions.Width * $zone.Dimensions.Height
        Actors = @()
    }

    # Generate basic actors for each zone
    $zoneData.Actors += @{
        Type = "Floor"
        Scale = @{ X = $zone.Dimensions.Length; Y = $zone.Dimensions.Width; Z = 0.1 }
        Position = @{ X = 0; Y = 0; Z = 0 }
    }

    $zoneData.Actors += @{
        Type = "Ceiling"
        Scale = @{ X = $zone.Dimensions.Length; Y = $zone.Dimensions.Width; Z = 0.1 }
        Position = @{ X = 0; Y = 0; Z = $zone.Dimensions.Height * 100 }
    }

    # Walls
    $zoneData.Actors += @{
        Type = "Wall_North"
        Scale = @{ X = $zone.Dimensions.Length; Y = 0.2; Z = $zone.Dimensions.Height }
        Position = @{ X = 0; Y = $zone.Dimensions.Width * 50; Z = $zone.Dimensions.Height * 50 }
    }

    $zoneData.Actors += @{
        Type = "Wall_South"
        Scale = @{ X = $zone.Dimensions.Length; Y = 0.2; Z = $zone.Dimensions.Height }
        Position = @{ X = 0; Y = -$zone.Dimensions.Width * 50; Z = $zone.Dimensions.Height * 50 }
    }

    $zoneData.Actors += @{
        Type = "Wall_East"
        Scale = @{ X = 0.2; Y = $zone.Dimensions.Width; Z = $zone.Dimensions.Height }
        Position = @{ X = $zone.Dimensions.Length * 50; Y = 0; Z = $zone.Dimensions.Height * 50 }
    }

    $zoneData.Actors += @{
        Type = "Wall_West"
        Scale = @{ X = 0.2; Y = $zone.Dimensions.Width; Z = $zone.Dimensions.Height }
        Position = @{ X = -$zone.Dimensions.Length * 50; Y = 0; Z = $zone.Dimensions.Height * 50 }
    }

    $blockoutData.Zones += $zoneData
}

# Calculate total volume
$totalVolume = ($blockoutData.Zones | ForEach-Object { $_.Volume } | Measure-Object -Sum).Sum
Write-Host ""
Write-Host "Total Volume: ${totalVolume}m³" -ForegroundColor Green

# Generate connections
Write-Host ""
Write-Host "Generating connections..." -ForegroundColor Yellow

$connections = @()
foreach ($zone in $zones) {
    foreach ($connection in $zone.Connections) {
        $connectionKey = @($zone.Name, $connection) | Sort-Object
        $connectionString = "$($connectionKey[0])-$($connectionKey[1])"

        if ($connections -notcontains $connectionString) {
            $connections += $connectionString
            Write-Host "  - $connectionString" -ForegroundColor Gray
        }
    }
}

$blockoutData.Connections = $connections

# Generate spawn points
Write-Host ""
Write-Host "Generating spawn points..." -ForegroundColor Yellow

$blockoutData.SpawnPoint = @{
    Zone = "EntryHall"
    Position = @{ X = 0; Y = 0; Z = 120 }
    Rotation = @{ Pitch = 0; Yaw = 0; Roll = 0 }
}

Write-Host "  - Player spawn: EntryHall (0, 0, 120)" -ForegroundColor Gray

# Generate navigation mesh bounds
Write-Host ""
Write-Host "Generating navigation bounds..." -ForegroundColor Yellow

$blockoutData.NavMeshBounds = @{
    Min = @{ X = -2500; Y = -1500; Z = -500 }
    Max = @{ X = 2500; Y = 1500; Z = 1000 }
}

Write-Host "  - Nav mesh bounds calculated" -ForegroundColor Gray

# Save blockout data
$outputFile = Join-Path $OutputPath "${LevelName}_Blockout.json"

Write-Host ""
Write-Host "Saving blockout data..." -ForegroundColor Yellow

# Ensure output directory exists
$outputDir = Split-Path $outputFile -Parent
if (-not (Test-Path $outputDir)) {
    New-Item -ItemType Directory -Path $outputDir -Force | Out-Null
}

$blockoutData | ConvertTo-Json -Depth 10 | Set-Content -Path $outputFile -Encoding UTF8

Write-Host "  - Saved to: $outputFile" -ForegroundColor Green

# Generate Unreal Engine import script
$ueScriptPath = Join-Path $OutputPath "${LevelName}_Import.py"

$ueScript = @"
# Unreal Engine Python script to import blockout
# Run this in UE Editor: py "$ueScriptPath"

import unreal
import json

# Load blockout data
with open(r'$outputFile', 'r') as f:
    data = json.load(f)

# Create level
level_name = data['LevelName']
level_path = '/Game/Maps/' + level_name

# Create new level
editor_level_lib = unreal.EditorLevelLibrary()
editor_level_lib.new_level(level_path)

print(f'Created level: {level_path}')

# Spawn blockout geometry
for zone in data['Zones']:
    zone_name = zone['Name']
    print(f'Creating zone: {zone_name}')

    for actor_data in zone['Actors']:
        # Create cube brush
        actor_location = unreal.Vector(
            actor_data['Position']['X'],
            actor_data['Position']['Y'],
            actor_data['Position']['Z']
        )

        actor_scale = unreal.Vector(
            actor_data['Scale']['X'],
            actor_data['Scale']['Y'],
            actor_data['Scale']['Z']
        )

        # Spawn static mesh actor (cube)
        actor = editor_level_lib.spawn_actor_from_class(
            unreal.StaticMeshActor,
            actor_location
        )

        if actor:
            actor.set_actor_label(f'{zone_name}_{actor_data["Type"]}')
            actor.set_actor_scale3d(actor_scale)
            print(f'  - Created {actor_data["Type"]}')

# Spawn player start
spawn_data = data['SpawnPoint']
spawn_location = unreal.Vector(
    spawn_data['Position']['X'],
    spawn_data['Position']['Y'],
    spawn_data['Position']['Z']
)

player_start = editor_level_lib.spawn_actor_from_class(
    unreal.PlayerStart,
    spawn_location
)

if player_start:
    player_start.set_actor_label('PlayerStart')
    print('Created player start')

# Create nav mesh bounds volume
nav_bounds = editor_level_lib.spawn_actor_from_class(
    unreal.NavMeshBoundsVolume,
    unreal.Vector(0, 0, 0)
)

if nav_bounds:
    nav_bounds.set_actor_label('NavMeshBounds')
    print('Created nav mesh bounds')

print('Blockout import complete!')
"@

$ueScript | Set-Content -Path $ueScriptPath -Encoding UTF8

Write-Host "  - UE import script: $ueScriptPath" -ForegroundColor Green

# Generate report
if ($GenerateReport) {
    Write-Host ""
    Write-Host "Generating report..." -ForegroundColor Yellow

    $reportPath = Join-Path $OutputPath "${LevelName}_Blockout_Report.md"

    $report = @"
# $LevelName Blockout Report

**Generated**: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")
**Total Area**: ${TotalArea}m²
**Total Volume**: ${totalVolume}m³

## Zones

| Zone | Area (m²) | Percentage | Dimensions (L×W×H) | Volume (m³) | Type |
|------|-----------|------------|-------------------|-------------|------|
"@

    foreach ($zone in $blockoutData.Zones) {
        $dims = "$($zone.Dimensions.Length)×$($zone.Dimensions.Width)×$($zone.Dimensions.Height)"
        $report += "`n| $($zone.Name) | $($zone.Area) | $($zone.Percentage)% | $dims | $($zone.Volume) | $($zone.Type) |"
    }

    $report += @"


## Connections

"@

    foreach ($connection in $connections) {
        $report += "- $connection`n"
    }

    $report += @"

## Spawn Point

- **Zone**: $($blockoutData.SpawnPoint.Zone)
- **Position**: ($($blockoutData.SpawnPoint.Position.X), $($blockoutData.SpawnPoint.Position.Y), $($blockoutData.SpawnPoint.Position.Z))

## Navigation

- **Nav Mesh Bounds**: Configured
- **Min**: ($($blockoutData.NavMeshBounds.Min.X), $($blockoutData.NavMeshBounds.Min.Y), $($blockoutData.NavMeshBounds.Min.Z))
- **Max**: ($($blockoutData.NavMeshBounds.Max.X), $($blockoutData.NavMeshBounds.Max.Y), $($blockoutData.NavMeshBounds.Max.Z))

## Next Steps

1. Import blockout into Unreal Engine using: ``py "$ueScriptPath"``
2. Place objective nodes using: ``.\PlaceObjectiveNodes.ps1``
3. Place evidence items using: ``.\PlaceEvidenceItems.ps1``
4. Test navigation using: ``.\TestLevelNavigation.ps1``
5. Validate layout using: ``.\ValidateLevelLayout.ps1``

## Files Generated

- Blockout Data: ``$outputFile``
- UE Import Script: ``$ueScriptPath``
- This Report: ``$reportPath``
"@

    $report | Set-Content -Path $reportPath -Encoding UTF8

    Write-Host "  - Report saved: $reportPath" -ForegroundColor Green
}

# Summary
Write-Host ""
Write-Host "=== Blockout Generation Complete ===" -ForegroundColor Cyan
Write-Host "Zones: $($zones.Count)" -ForegroundColor Green
Write-Host "Connections: $($connections.Count)" -ForegroundColor Green
Write-Host "Total Area: ${TotalArea}m²" -ForegroundColor Green
Write-Host "Total Volume: ${totalVolume}m³" -ForegroundColor Green
Write-Host ""
Write-Host "Next: Import into UE using: py `"$ueScriptPath`"" -ForegroundColor Yellow
