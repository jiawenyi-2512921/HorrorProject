<#
.SYNOPSIS
    Generate Actor Blueprint templates
.DESCRIPTION
    Creates Actor Blueprint templates with components and functionality
.PARAMETER ActorName
    Name of the Actor Blueprint
.PARAMETER OutputPath
    Output directory
.PARAMETER ActorType
    Type of actor (Interactable, Prop, Enemy, Trigger, etc.)
.EXAMPLE
    .\GenerateActorBlueprint.ps1 -ActorName "BP_Door" -ActorType "Interactable" -OutputPath "D:/gptzuo/HorrorProject/HorrorProject/Content/Blueprints/Actors"
#>

param(
    [Parameter(Mandatory=$true)]
    [string]$ActorName,

    [Parameter(Mandatory=$true)]
    [string]$OutputPath,

    [Parameter(Mandatory=$false)]
    [ValidateSet("Interactable", "Prop", "Enemy", "Trigger", "Pickup", "Light", "Audio", "Custom")]
    [string]$ActorType = "Custom",

    [Parameter(Mandatory=$false)]
    [switch]$IncludePhysics,

    [Parameter(Mandatory=$false)]
    [switch]$Verbose
)

$ErrorActionPreference = "Stop"
$LogFile = Join-Path $PSScriptRoot "Logs/GenerateActorBlueprint_$(Get-Date -Format 'yyyyMMdd_HHmmss').log"

function Write-Log {
    param([string]$Message, [string]$Level = "INFO")
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    $logMessage = "[$timestamp] [$Level] $Message"
    Add-Content -Path $LogFile -Value $logMessage
    if ($Verbose -or $Level -eq "ERROR") {
        Write-Host $logMessage -ForegroundColor $(if($Level -eq "ERROR"){"Red"}elseif($Level -eq "WARN"){"Yellow"}else{"White"})
    }
}

function Get-ActorComponents {
    param([string]$Type, [bool]$Physics)

    $baseComponents = @("SceneComponent_Root")

    $typeComponents = @{
        "Interactable" = @("StaticMeshComponent", "BoxComponent_Interaction", "AudioComponent")
        "Prop" = @("StaticMeshComponent")
        "Enemy" = @("SkeletalMeshComponent", "CapsuleComponent", "AIController")
        "Trigger" = @("BoxComponent_Trigger")
        "Pickup" = @("StaticMeshComponent", "SphereComponent_Pickup", "RotatingMovementComponent")
        "Light" = @("PointLightComponent", "SphereComponent")
        "Audio" = @("AudioComponent", "SphereComponent_Attenuation")
        "Custom" = @("StaticMeshComponent")
    }

    $components = $baseComponents + $typeComponents[$Type]

    if ($Physics) {
        $components += "PhysicsConstraintComponent"
    }

    return $components
}

function Get-ActorFunctions {
    param([string]$Type)

    $baseFunctions = @(
        @{
            "Name" = "BeginPlay"
            "Type" = "Event"
            "Description" = "Called when the game starts or when spawned"
        }
        @{
            "Name" = "Tick"
            "Type" = "Event"
            "Description" = "Called every frame"
        }
    )

    $typeFunctions = @{
        "Interactable" = @(
            @{
                "Name" = "OnInteract"
                "Type" = "Function"
                "Parameters" = @("AActor* InteractingActor")
                "Description" = "Called when actor is interacted with"
            }
            @{
                "Name" = "CanInteract"
                "Type" = "Function"
                "ReturnType" = "bool"
                "Description" = "Check if actor can be interacted with"
            }
        )
        "Trigger" = @(
            @{
                "Name" = "OnTriggerEnter"
                "Type" = "Event"
                "Parameters" = @("AActor* OtherActor")
                "Description" = "Called when actor enters trigger"
            }
            @{
                "Name" = "OnTriggerExit"
                "Type" = "Event"
                "Parameters" = @("AActor* OtherActor")
                "Description" = "Called when actor exits trigger"
            }
        )
        "Pickup" = @(
            @{
                "Name" = "OnPickup"
                "Type" = "Function"
                "Parameters" = @("AActor* PickupActor")
                "Description" = "Called when item is picked up"
            }
        )
        "Enemy" = @(
            @{
                "Name" = "OnDeath"
                "Type" = "Event"
                "Description" = "Called when enemy dies"
            }
            @{
                "Name" = "TakeDamage"
                "Type" = "Function"
                "Parameters" = @("float Damage", "AActor* DamageCauser")
                "Description" = "Apply damage to enemy"
            }
        )
    }

    $functions = $baseFunctions
    if ($typeFunctions.ContainsKey($Type)) {
        $functions += $typeFunctions[$Type]
    }

    return $functions
}

function New-ActorBlueprint {
    param(
        [string]$Name,
        [string]$Type,
        [string]$Path,
        [bool]$Physics
    )

    $guid = [guid]::NewGuid().ToString("N").ToUpper()
    $components = Get-ActorComponents -Type $Type -Physics $Physics
    $functions = Get-ActorFunctions -Type $Type

    $actorTemplate = @{
        "Type" = "ActorBlueprint"
        "Name" = $Name
        "ParentClass" = "/Script/Engine.Actor"
        "ActorType" = $Type
        "GeneratedBy" = "GenerateActorBlueprint.ps1"
        "GeneratedDate" = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
        "GUID" = $guid
        "Properties" = @{
            "bReplicates" = $false
            "bNetLoadOnClient" = $true
            "bNetLoadOnServer" = $true
            "bCanBeDamaged" = ($Type -eq "Enemy")
        }
        "Components" = $components | ForEach-Object {
            @{
                "Name" = $_
                "Type" = $_.Split('_')[0]
                "AttachParent" = if ($_ -eq "SceneComponent_Root") { $null } else { "SceneComponent_Root" }
                "Properties" = @{}
            }
        }
        "Variables" = @(
            @{
                "Name" = "bIsActive"
                "Type" = "bool"
                "DefaultValue" = $true
                "Category" = "State"
            }
        )
        "Functions" = $functions
        "EventGraph" = @{
            "Nodes" = @()
            "Connections" = @()
        }
        "Metadata" = @{
            "Category" = "Actors/$Type"
            "Description" = "Actor Blueprint for $Name"
            "Keywords" = "Actor, Blueprint, $Type"
        }
    }

    $outputFile = Join-Path $Path "$Name.json"

    if (-not (Test-Path $Path)) {
        New-Item -ItemType Directory -Path $Path -Force | Out-Null
        Write-Log "Created directory: $Path"
    }

    $actorTemplate | ConvertTo-Json -Depth 10 | Out-File -FilePath $outputFile -Encoding UTF8
    Write-Log "Generated Actor Blueprint: $outputFile"

    return $outputFile
}

function New-ActorCppFiles {
    param(
        [string]$Name,
        [string]$Type,
        [string]$Path
    )

    $className = $Name -replace '^BP_', 'A'

    # Generate header
    $headerFile = Join-Path $Path "$className.h"
    $header = @"
// Auto-generated Actor Blueprint header
// Generated by GenerateActorBlueprint.ps1

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "$className.generated.h"

/**
 * Actor Blueprint: $Name
 * Type: $Type
 */
UCLASS()
class HORRORPROJECT_API $className : public AActor
{
    GENERATED_BODY()

public:
    // Constructor
    $className();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    bool bIsActive;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Actor")
    void Activate();

    UFUNCTION(BlueprintCallable, Category = "Actor")
    void Deactivate();
};
"@

    $header | Out-File -FilePath $headerFile -Encoding UTF8
    Write-Log "Generated C++ header: $headerFile"

    # Generate source
    $sourceFile = Join-Path $Path "$className.cpp"
    $source = @"
// Auto-generated Actor Blueprint source
// Generated by GenerateActorBlueprint.ps1

#include "$className.h"

$className::$className()
{
    // Set this actor to call Tick() every frame
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize properties
    bIsActive = true;
}

void $className::BeginPlay()
{
    Super::BeginPlay();
}

void $className::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void $className::Activate()
{
    bIsActive = true;
}

void $className::Deactivate()
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
    Write-Log "Starting Actor Blueprint generation"
    Write-Log "Actor Name: $ActorName"
    Write-Log "Actor Type: $ActorType"
    Write-Log "Output Path: $OutputPath"

    # Create logs directory
    $logsDir = Join-Path $PSScriptRoot "Logs"
    if (-not (Test-Path $logsDir)) {
        New-Item -ItemType Directory -Path $logsDir -Force | Out-Null
    }

    # Validate actor name
    if ($ActorName -notmatch '^(BP_)?[A-Za-z_][A-Za-z0-9_]*$') {
        throw "Invalid Actor name. Should follow BP_ naming convention."
    }

    # Generate Actor Blueprint
    $actorFile = New-ActorBlueprint -Name $ActorName -Type $ActorType -Path $OutputPath -Physics $IncludePhysics

    # Generate C++ files
    $cppFiles = New-ActorCppFiles -Name $ActorName -Type $ActorType -Path $OutputPath

    Write-Log "Actor Blueprint generation completed successfully" -Level "INFO"
    Write-Host "`nGenerated files:" -ForegroundColor Green
    Write-Host "  Blueprint: $actorFile" -ForegroundColor Cyan
    Write-Host "  Header: $($cppFiles.Header)" -ForegroundColor Cyan
    Write-Host "  Source: $($cppFiles.Source)" -ForegroundColor Cyan

    return @{
        Success = $true
        BlueprintFile = $actorFile
        HeaderFile = $cppFiles.Header
        SourceFile = $cppFiles.Source
    }

} catch {
    Write-Log "Error: $($_.Exception.Message)" -Level "ERROR"
    Write-Log "Stack trace: $($_.ScriptStackTrace)" -Level "ERROR"
    throw
}
