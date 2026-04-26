<#
.SYNOPSIS
    Generate Component Blueprint templates
.DESCRIPTION
    Creates Component Blueprint templates for reusable actor components
.PARAMETER ComponentName
    Name of the Component Blueprint
.PARAMETER OutputPath
    Output directory
.PARAMETER ComponentType
    Type of component (Scene, Actor, Movement, Audio, etc.)
.EXAMPLE
    .\GenerateComponentBlueprint.ps1 -ComponentName "BP_HealthComponent" -ComponentType "Actor" -OutputPath "D:/gptzuo/HorrorProject/HorrorProject/Content/Blueprints/Components"
#>

param(
    [Parameter(Mandatory=$true)]
    [string]$ComponentName,

    [Parameter(Mandatory=$true)]
    [string]$OutputPath,

    [Parameter(Mandatory=$false)]
    [ValidateSet("Scene", "Actor", "Movement", "Audio", "Particle", "Light", "Camera", "Custom")]
    [string]$ComponentType = "Actor",

    [Parameter(Mandatory=$false)]
    [switch]$Verbose
)

$ErrorActionPreference = "Stop"
$LogFile = Join-Path $PSScriptRoot "Logs/GenerateComponentBlueprint_$(Get-Date -Format 'yyyyMMdd_HHmmss').log"

function Write-Log {
    param([string]$Message, [string]$Level = "INFO")
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    $logMessage = "[$timestamp] [$Level] $Message"
    Add-Content -Path $LogFile -Value $logMessage
    if ($Verbose -or $Level -eq "ERROR") {
        Write-Host $logMessage -ForegroundColor $(if($Level -eq "ERROR"){"Red"}elseif($Level -eq "WARN"){"Yellow"}else{"White"})
    }
}

function Get-ComponentParentClass {
    param([string]$Type)

    $parentClasses = @{
        "Scene" = "/Script/Engine.SceneComponent"
        "Actor" = "/Script/Engine.ActorComponent"
        "Movement" = "/Script/Engine.MovementComponent"
        "Audio" = "/Script/Engine.AudioComponent"
        "Particle" = "/Script/Engine.ParticleSystemComponent"
        "Light" = "/Script/Engine.LightComponent"
        "Camera" = "/Script/Engine.CameraComponent"
        "Custom" = "/Script/Engine.ActorComponent"
    }

    return $parentClasses[$Type]
}

function Get-ComponentFunctions {
    param([string]$Type)

    $baseFunctions = @(
        @{
            "Name" = "BeginPlay"
            "Type" = "Event"
            "Description" = "Called when component begins play"
        }
        @{
            "Name" = "TickComponent"
            "Type" = "Event"
            "Description" = "Called every frame"
        }
    )

    $typeFunctions = @{
        "Actor" = @(
            @{
                "Name" = "Initialize"
                "Type" = "Function"
                "Description" = "Initialize component"
            }
            @{
                "Name" = "Cleanup"
                "Type" = "Function"
                "Description" = "Cleanup component resources"
            }
        )
        "Movement" = @(
            @{
                "Name" = "UpdateMovement"
                "Type" = "Function"
                "Parameters" = @("float DeltaTime")
                "Description" = "Update movement logic"
            }
        )
        "Audio" = @(
            @{
                "Name" = "PlaySound"
                "Type" = "Function"
                "Parameters" = @("USoundBase* Sound")
                "Description" = "Play audio"
            }
            @{
                "Name" = "StopSound"
                "Type" = "Function"
                "Description" = "Stop audio playback"
            }
        )
    }

    $functions = $baseFunctions
    if ($typeFunctions.ContainsKey($Type)) {
        $functions += $typeFunctions[$Type]
    }

    return $functions
}

function New-ComponentBlueprint {
    param(
        [string]$Name,
        [string]$Type,
        [string]$Path
    )

    $guid = [guid]::NewGuid().ToString("N").ToUpper()
    $parentClass = Get-ComponentParentClass -Type $Type
    $functions = Get-ComponentFunctions -Type $Type

    $componentTemplate = @{
        "Type" = "ComponentBlueprint"
        "Name" = $Name
        "ParentClass" = $parentClass
        "ComponentType" = $Type
        "GeneratedBy" = "GenerateComponentBlueprint.ps1"
        "GeneratedDate" = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
        "GUID" = $guid
        "Properties" = @{
            "bCanEverTick" = $true
            "bTickInEditor" = $false
            "bAutoActivate" = $true
        }
        "Variables" = @(
            @{
                "Name" = "bIsInitialized"
                "Type" = "bool"
                "DefaultValue" = $false
                "Category" = "State"
            }
        )
        "Functions" = $functions
        "EventGraph" = @{
            "Nodes" = @()
            "Connections" = @()
        }
        "Metadata" = @{
            "Category" = "Components/$Type"
            "Description" = "Component Blueprint for $Name"
            "Keywords" = "Component, Blueprint, $Type"
        }
    }

    $outputFile = Join-Path $Path "$Name.json"

    if (-not (Test-Path $Path)) {
        New-Item -ItemType Directory -Path $Path -Force | Out-Null
        Write-Log "Created directory: $Path"
    }

    $componentTemplate | ConvertTo-Json -Depth 10 | Out-File -FilePath $outputFile -Encoding UTF8
    Write-Log "Generated Component Blueprint: $outputFile"

    return $outputFile
}

function New-ComponentCppFiles {
    param(
        [string]$Name,
        [string]$Type,
        [string]$Path
    )

    $className = $Name -replace '^BP_', 'U'
    $parentClass = switch ($Type) {
        "Scene" { "USceneComponent" }
        "Movement" { "UMovementComponent" }
        "Audio" { "UAudioComponent" }
        "Particle" { "UParticleSystemComponent" }
        "Light" { "ULightComponent" }
        "Camera" { "UCameraComponent" }
        default { "UActorComponent" }
    }

    # Generate header
    $headerFile = Join-Path $Path "$className.h"
    $header = @"
// Auto-generated Component Blueprint header
// Generated by GenerateComponentBlueprint.ps1

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "$className.generated.h"

/**
 * Component Blueprint: $Name
 * Type: $Type
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HORRORPROJECT_API $className : public $parentClass
{
    GENERATED_BODY()

public:
    // Constructor
    $className();

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    bool bIsInitialized;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Component")
    void Initialize();

    UFUNCTION(BlueprintCallable, Category = "Component")
    void Cleanup();

protected:
    // Internal state
    bool bIsActive;
};
"@

    $header | Out-File -FilePath $headerFile -Encoding UTF8
    Write-Log "Generated C++ header: $headerFile"

    # Generate source
    $sourceFile = Join-Path $Path "$className.cpp"
    $source = @"
// Auto-generated Component Blueprint source
// Generated by GenerateComponentBlueprint.ps1

#include "$className.h"

$className::$className()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame
    PrimaryComponentTick.bCanEverTick = true;
    bAutoActivate = true;

    // Initialize properties
    bIsInitialized = false;
    bIsActive = false;
}

void $className::BeginPlay()
{
    Super::BeginPlay();

    // Initialize component
    Initialize();
}

void $className::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Component tick logic
}

void $className::Initialize()
{
    if (!bIsInitialized)
    {
        bIsInitialized = true;
        bIsActive = true;
    }
}

void $className::Cleanup()
{
    bIsActive = false;
}
"@

    $source | Out-File -FilePath $sourceFile -Encoding UTF8
    Write-Log "Generated C++ source: $sourceFile"

    return @{
        Header = $headerFile
        Source = $sourceFile
    }
}

# Main execution
try {
    Write-Log "Starting Component Blueprint generation"
    Write-Log "Component Name: $ComponentName"
    Write-Log "Component Type: $ComponentType"
    Write-Log "Output Path: $OutputPath"

    # Create logs directory
    $logsDir = Join-Path $PSScriptRoot "Logs"
    if (-not (Test-Path $logsDir)) {
        New-Item -ItemType Directory -Path $logsDir -Force | Out-Null
    }

    # Validate component name
    if ($ComponentName -notmatch '^(BP_)?[A-Za-z_][A-Za-z0-9_]*$') {
        throw "Invalid Component name. Should follow BP_ naming convention."
    }

    # Generate Component Blueprint
    $componentFile = New-ComponentBlueprint -Name $ComponentName -Type $ComponentType -Path $OutputPath

    # Generate C++ files
    $cppFiles = New-ComponentCppFiles -Name $ComponentName -Type $ComponentType -Path $OutputPath

    Write-Log "Component Blueprint generation completed successfully" -Level "INFO"
    Write-Host "`nGenerated files:" -ForegroundColor Green
    Write-Host "  Blueprint: $componentFile" -ForegroundColor Cyan
    Write-Host "  Header: $($cppFiles.Header)" -ForegroundColor Cyan
    Write-Host "  Source: $($cppFiles.Source)" -ForegroundColor Cyan

    return @{
        Success = $true
        BlueprintFile = $componentFile
        HeaderFile = $cppFiles.Header
        SourceFile = $cppFiles.Source
    }

} catch {
    Write-Log "Error: $($_.Exception.Message)" -Level "ERROR"
    Write-Log "Stack trace: $($_.ScriptStackTrace)" -Level "ERROR"
    throw
}
