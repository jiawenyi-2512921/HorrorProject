# Validates high-priority gameplay review fixes that are easy to regress in source.

$ErrorActionPreference = "Stop"

. (Join-Path $PSScriptRoot "Common.ps1")

$ProjectRoot = Get-HorrorProjectRoot

Write-Host "=== Gameplay Review Fix Validation ===" -ForegroundColor Cyan
Write-Host "Project: $ProjectRoot" -ForegroundColor Yellow

$Checks = New-Object System.Collections.Generic.List[object]

function Add-Check {
    param(
        [string]$Name,
        [bool]$Passed,
        [string]$Details
    )

    $Checks.Add([pscustomobject]@{
        Name = $Name
        Passed = $Passed
        Details = $Details
    })
}

function Get-ProjectText {
    param([string]$RelativePath)
    return Get-Content -Raw -LiteralPath (Join-Path $ProjectRoot $RelativePath)
}

$GolemCpp = Get-ProjectText "Source\HorrorProject\AI\HorrorGolemBehaviorComponent.cpp"
$GolemHeader = Get-ProjectText "Source\HorrorProject\AI\HorrorGolemBehaviorComponent.h"
$NetworkHeader = Get-ProjectText "Source\HorrorProject\Network\NetworkReplicationComponent.h"
$AudioCpp = Get-ProjectText "Source\HorrorProject\Audio\HorrorAudioSubsystem.cpp"
$FearCpp = Get-ProjectText "Source\HorrorProject\Player\Components\FearComponent.cpp"
$PostProcessCpp = Get-ProjectText "Source\HorrorProject\VFX\PostProcessController.cpp"
$PostProcessHeader = Get-ProjectText "Source\HorrorProject\VFX\PostProcessController.h"

Add-Check `
    -Name "Golem movement uses NavMesh pathfinding" `
    -Passed ($GolemCpp -match "FindPathToLocationSynchronously" -and $GolemCpp -match "MoveToLocation" -and $GolemCpp -notmatch "SetActorLocation\(NewLocation") `
    -Details "Movement must route through NavigationSystem + AIController, not direct SetActorLocation."

Add-Check `
    -Name "Golem chase has lost-target fallback" `
    -Passed ($GolemCpp -match "HandleLostTarget" -and $GolemHeader -match "ChaseLostTargetDistance") `
    -Details "Full chase should downgrade when the target remains beyond the lost-target range."

Add-Check `
    -Name "Client transform RPC is unreliable" `
    -Passed ($NetworkHeader -match "UFUNCTION\(\s*Server\s*,\s*Unreliable\s*,\s*WithValidation\s*\)") `
    -Details "High-frequency location updates must not use Reliable RPC."

Add-Check `
    -Name "Audio occlusion is smoothed" `
    -Passed ($AudioCpp -match "FInterpTo" -and $AudioCpp -notmatch "VolumeMultiplier\s*\*\s*OcclusionVolumeMultiplier") `
    -Details "Occlusion volume should interpolate instead of multiplying repeatedly."

Add-Check `
    -Name "Preloaded audio roots are released" `
    -Passed ($AudioCpp -match "PreloadedSounds" -and $AudioCpp -match "RemoveFromRoot" -and $AudioCpp -match "Deinitialize\(\)[\s\S]*PreloadedSounds\.Empty") `
    -Details "Sounds rooted by PreloadSound must be unrooted during unload/deinitialize."

Add-Check `
    -Name "Fear decay timer is demand-driven" `
    -Passed ($FearCpp -match "StartFearDecayTimer" -and $FearCpp -match "StopFearDecayTimer" -and $FearCpp -notmatch "void UFearComponent::BeginPlay\(\)[\s\S]{0,500}SetTimer") `
    -Details "Fear decay should not keep a timer active while fear is zero."

Add-Check `
    -Name "Post-process material updates report null failures" `
    -Passed ($PostProcessCpp -match "bool UPostProcessController::CreateDynamicMaterial" -and $PostProcessCpp -match "bool UPostProcessController::UpdateMaterialParameters" -and $PostProcessHeader -match "bool CreateDynamicMaterial") `
    -Details "Missing dynamic materials must prevent active effect registration."

$Failed = @($Checks | Where-Object { -not $_.Passed })

foreach ($Check in $Checks) {
    if ($Check.Passed) {
        Write-Host "[PASS] $($Check.Name)" -ForegroundColor Green
    } else {
        Write-Host "[FAIL] $($Check.Name) - $($Check.Details)" -ForegroundColor Red
    }
}

if ($Failed.Count -gt 0) {
    Write-Host "`n[ERROR] Gameplay review validation failed: $($Failed.Count) issue(s)" -ForegroundColor Red
    exit 1
}

Write-Host "`n[OK] Gameplay review validation passed" -ForegroundColor Green
