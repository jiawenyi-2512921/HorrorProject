# Visualize AI Perception Script

# Visualize AI perception ranges and states in Horror Project
# Usage: .\VisualizeAIPerception.ps1 [-Enable] [-Disable] [-AIName <name>]

param(
    [switch]$Enable = $false,
    [switch]$Disable = $false,
    [string]$AIName = "",
    [switch]$ShowSight = $true,
    [switch]$ShowHearing = $true,
    [switch]$ShowDamage = $false
)

$ProjectPath = "D:\gptzuo\HorrorProject\HorrorProject"
$ConfigPath = "$ProjectPath\Config\DefaultEngine.ini"

Write-Host "=== AI Perception Visualization Tool ===" -ForegroundColor Cyan
Write-Host ""

function Enable-AIDebugVisualization {
    Write-Host "Enabling AI debug visualization..." -ForegroundColor Yellow

    # Create console commands file
    $commandsFile = "$ProjectPath\Saved\Config\WindowsEditor\EditorPerProjectUserSettings.ini"
    $commands = @"
[/Script/UnrealEd.EditorPerProjectUserSettings]
ConsoleCommands=(Command="showdebug ai")
ConsoleCommands=(Command="ai.debug.DrawPerception 1")
ConsoleCommands=(Command="ai.debug.DrawBehaviorTree 1")
ConsoleCommands=(Command="ai.debug.DrawBlackboard 1")
"@

    # Ensure directory exists
    $configDir = Split-Path $commandsFile -Parent
    if (-not (Test-Path $configDir)) {
        New-Item -ItemType Directory -Path $configDir -Force | Out-Null
    }

    # Write commands
    $commands | Out-File -FilePath $commandsFile -Encoding UTF8 -Append

    Write-Host "Debug visualization enabled" -ForegroundColor Green
    Write-Host "Restart the editor to apply changes" -ForegroundColor Yellow
}

function Disable-AIDebugVisualization {
    Write-Host "Disabling AI debug visualization..." -ForegroundColor Yellow

    $commandsFile = "$ProjectPath\Saved\Config\WindowsEditor\EditorPerProjectUserSettings.ini"
    $commands = @"
[/Script/UnrealEd.EditorPerProjectUserSettings]
ConsoleCommands=(Command="showdebug ai 0")
ConsoleCommands=(Command="ai.debug.DrawPerception 0")
ConsoleCommands=(Command="ai.debug.DrawBehaviorTree 0")
ConsoleCommands=(Command="ai.debug.DrawBlackboard 0")
"@

    $configDir = Split-Path $commandsFile -Parent
    if (-not (Test-Path $configDir)) {
        New-Item -ItemType Directory -Path $configDir -Force | Out-Null
    }

    $commands | Out-File -FilePath $commandsFile -Encoding UTF8 -Append

    Write-Host "Debug visualization disabled" -ForegroundColor Green
    Write-Host "Restart the editor to apply changes" -ForegroundColor Yellow
}

function Show-PerceptionInfo {
    Write-Host "AI Perception Visualization Info:" -ForegroundColor Yellow
    Write-Host "==================================" -ForegroundColor Yellow
    Write-Host ""

    Write-Host "Color Legend:" -ForegroundColor Cyan
    Write-Host "  Blue Sphere   - Sight Range" -ForegroundColor Blue
    Write-Host "  Green Cone    - Field of View" -ForegroundColor Green
    Write-Host "  Yellow Sphere - Hearing Range" -ForegroundColor Yellow
    Write-Host "  Red Line      - Line of Sight (blocked)" -ForegroundColor Red
    Write-Host "  Green Line    - Line of Sight (clear)" -ForegroundColor Green
    Write-Host ""

    Write-Host "State Colors:" -ForegroundColor Cyan
    Write-Host "  Black   - Dormant" -ForegroundColor Gray
    Write-Host "  Blue    - Distant Sighting" -ForegroundColor Blue
    Write-Host "  Yellow  - Close Stalking" -ForegroundColor Yellow
    Write-Host "  Orange  - Chase Triggered" -ForegroundColor DarkYellow
    Write-Host "  Red     - Full Chase" -ForegroundColor Red
    Write-Host "  Magenta - Final Impact" -ForegroundColor Magenta
    Write-Host ""

    Write-Host "Console Commands:" -ForegroundColor Cyan
    Write-Host "  showdebug ai                    - Toggle AI debug display"
    Write-Host "  ai.debug [AIName]               - Debug specific AI"
    Write-Host "  ai.debug.DrawPerception 1       - Enable perception visualization"
    Write-Host "  ai.debug.DrawBehaviorTree 1     - Enable behavior tree visualization"
    Write-Host "  ai.debug.DrawBlackboard 1       - Enable blackboard visualization"
    Write-Host "  ai.showblackboard               - Show blackboard values"
    Write-Host ""

    Write-Host "In-Game Controls:" -ForegroundColor Cyan
    Write-Host "  Press ~ to open console"
    Write-Host "  Type commands above"
    Write-Host "  Press F1 to toggle AI debug overlay"
    Write-Host ""
}

function Generate-VisualizationBlueprint {
    Write-Host "Generating visualization helper blueprint..." -ForegroundColor Yellow

    $blueprintContent = @"
# AI Perception Visualization Helper
# This blueprint can be placed in the level to visualize AI perception

Blueprint: BP_AIPerceptionVisualizer
Parent: Actor

Components:
  - SceneComponent (Root)
  - TextRenderComponent (InfoDisplay)

Properties:
  - bShowSightRange: Boolean = true
  - bShowHearingRange: Boolean = true
  - bShowFOV: Boolean = true
  - bShowLineOfSight: Boolean = true
  - UpdateFrequency: Float = 0.1

Functions:
  - DrawSightRange(AI: Actor, Radius: Float, Color: LinearColor)
  - DrawHearingRange(AI: Actor, Radius: Float, Color: LinearColor)
  - DrawFieldOfView(AI: Actor, Angle: Float, Range: Float, Color: LinearColor)
  - DrawLineOfSight(AI: Actor, Target: Actor, bHasLOS: Boolean)
  - UpdateVisualization()

Events:
  - BeginPlay: Start visualization timer
  - Tick: Update visualization if enabled
  - EndPlay: Clean up debug draws

Usage:
  1. Place BP_AIPerceptionVisualizer in level
  2. Set properties in Details panel
  3. Play in editor to see visualization
"@

    $blueprintPath = "$ProjectPath\Docs\AI\Debug\BP_AIPerceptionVisualizer_Info.txt"
    $blueprintContent | Out-File -FilePath $blueprintPath -Encoding UTF8

    Write-Host "Blueprint info generated: $blueprintPath" -ForegroundColor Green
}

function Create-DebugMacros {
    Write-Host "Creating debug macros..." -ForegroundColor Yellow

    $macrosContent = @"
// AI Debug Macros for HorrorProject
// Add to HorrorProjectMacros.h

#pragma once

// Enable AI debug drawing
#ifndef ENABLE_AI_DEBUG_DRAW
    #define ENABLE_AI_DEBUG_DRAW (1 && !UE_BUILD_SHIPPING)
#endif

// AI Debug Draw Macros
#if ENABLE_AI_DEBUG_DRAW

    #define AI_DEBUG_DRAW_SPHERE(World, Location, Radius, Color) \
        DrawDebugSphere(World, Location, Radius, 32, Color, false, -1.0f, 0, 2.0f)

    #define AI_DEBUG_DRAW_LINE(World, Start, End, Color) \
        DrawDebugLine(World, Start, End, Color, false, -1.0f, 0, 2.0f)

    #define AI_DEBUG_DRAW_CONE(World, Origin, Direction, Length, Angle, Color) \
        DrawDebugCone(World, Origin, Direction, Length, Angle, Angle, 32, Color, false, -1.0f, 0, 2.0f)

    #define AI_DEBUG_DRAW_STRING(World, Location, Text, Color) \
        DrawDebugString(World, Location, Text, nullptr, Color, 0.0f, true)

    #define AI_DEBUG_LOG(Format, ...) \
        UE_LOG(LogTemp, Log, TEXT("AI Debug: " Format), ##__VA_ARGS__)

    #define AI_DEBUG_WARNING(Format, ...) \
        UE_LOG(LogTemp, Warning, TEXT("AI Warning: " Format), ##__VA_ARGS__)

    #define AI_DEBUG_ERROR(Format, ...) \
        UE_LOG(LogTemp, Error, TEXT("AI Error: " Format), ##__VA_ARGS__)

#else

    #define AI_DEBUG_DRAW_SPHERE(World, Location, Radius, Color)
    #define AI_DEBUG_DRAW_LINE(World, Start, End, Color)
    #define AI_DEBUG_DRAW_CONE(World, Origin, Direction, Length, Angle, Color)
    #define AI_DEBUG_DRAW_STRING(World, Location, Text, Color)
    #define AI_DEBUG_LOG(Format, ...)
    #define AI_DEBUG_WARNING(Format, ...)
    #define AI_DEBUG_ERROR(Format, ...)

#endif

// AI Debug Categories
DECLARE_LOG_CATEGORY_EXTERN(LogAIBehavior, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogAIPerception, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogAINavigation, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogAIPerformance, Log, All);
"@

    $macrosPath = "$ProjectPath\Docs\AI\Debug\AIDebugMacros.h"
    $macrosContent | Out-File -FilePath $macrosPath -Encoding UTF8

    Write-Host "Debug macros created: $macrosPath" -ForegroundColor Green
}

function Show-VisualizationExamples {
    Write-Host "Visualization Code Examples:" -ForegroundColor Yellow
    Write-Host "============================" -ForegroundColor Yellow
    Write-Host ""

    Write-Host "Example 1: Draw Sight Range" -ForegroundColor Cyan
    Write-Host @"
void AHorrorThreatAIController::DrawSightDebug() const
{
    #if ENABLE_AI_DEBUG_DRAW
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return;

    FVector Location = ControlledPawn->GetActorLocation();
    AI_DEBUG_DRAW_SPHERE(GetWorld(), Location, 2000.0f, FColor::Blue);
    #endif
}
"@
    Write-Host ""

    Write-Host "Example 2: Draw Field of View" -ForegroundColor Cyan
    Write-Host @"
void AHorrorThreatAIController::DrawFOVDebug() const
{
    #if ENABLE_AI_DEBUG_DRAW
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return;

    FVector Location = ControlledPawn->GetActorLocation();
    FVector Forward = ControlledPawn->GetActorForwardVector();

    AI_DEBUG_DRAW_CONE(
        GetWorld(),
        Location,
        Forward,
        2000.0f,
        FMath::DegreesToRadians(45.0f),
        FColor::Green
    );
    #endif
}
"@
    Write-Host ""

    Write-Host "Example 3: Draw Line of Sight" -ForegroundColor Cyan
    Write-Host @"
void AHorrorThreatAIController::DrawLOSDebug(AActor* Target) const
{
    #if ENABLE_AI_DEBUG_DRAW
    if (!Target) return;

    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return;

    FVector Start = ControlledPawn->GetActorLocation();
    FVector End = Target->GetActorLocation();

    bool bHasLOS = HasLineOfSightTo(Target);
    FColor LineColor = bHasLOS ? FColor::Green : FColor::Red;

    AI_DEBUG_DRAW_LINE(GetWorld(), Start, End, LineColor);
    #endif
}
"@
    Write-Host ""
}

# Main execution
if ($Enable) {
    Enable-AIDebugVisualization
} elseif ($Disable) {
    Disable-AIDebugVisualization
} else {
    Show-PerceptionInfo
    Write-Host ""
    Generate-VisualizationBlueprint
    Create-DebugMacros
    Write-Host ""
    Show-VisualizationExamples
    Write-Host ""
    Write-Host "To enable visualization, run:" -ForegroundColor Yellow
    Write-Host "  .\VisualizeAIPerception.ps1 -Enable" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "To disable visualization, run:" -ForegroundColor Yellow
    Write-Host "  .\VisualizeAIPerception.ps1 -Disable" -ForegroundColor Cyan
}

Write-Host "`nVisualization tool complete." -ForegroundColor Green
