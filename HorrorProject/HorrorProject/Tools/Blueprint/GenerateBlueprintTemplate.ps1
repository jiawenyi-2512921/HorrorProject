<#
.SYNOPSIS
    Generate Blueprint template files for Unreal Engine
.DESCRIPTION
    Creates Blueprint template files with proper structure and metadata
.PARAMETER BlueprintName
    Name of the Blueprint to create
.PARAMETER BlueprintType
    Type of Blueprint (Actor, Component, Widget, GameMode, etc.)
.PARAMETER OutputPath
    Output directory for the Blueprint
.PARAMETER ParentClass
    Parent class for the Blueprint
.EXAMPLE
    .\GenerateBlueprintTemplate.ps1 -BlueprintName "BP_MyActor" -BlueprintType "Actor" -OutputPath "D:/gptzuo/HorrorProject/HorrorProject/Content/Blueprints"
#>

param(
    [Parameter(Mandatory=$true)]
    [string]$BlueprintName,

    [Parameter(Mandatory=$true)]
    [ValidateSet("Actor", "Component", "Widget", "GameMode", "Character", "Pawn", "Interface", "Struct", "Enum", "MacroLibrary", "FunctionLibrary")]
    [string]$BlueprintType,

    [Parameter(Mandatory=$true)]
    [string]$OutputPath,

    [Parameter(Mandatory=$false)]
    [string]$ParentClass = "",

    [Parameter(Mandatory=$false)]
    [switch]$Verbose
)

$ErrorActionPreference = "Stop"
$LogFile = Join-Path $PSScriptRoot "Logs/GenerateBlueprintTemplate_$(Get-Date -Format 'yyyyMMdd_HHmmss').log"

function Write-Log {
    param([string]$Message, [string]$Level = "INFO")
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    $logMessage = "[$timestamp] [$Level] $Message"
    Add-Content -Path $LogFile -Value $logMessage
    if ($Verbose -or $Level -eq "ERROR") {
        Write-Host $logMessage -ForegroundColor $(if($Level -eq "ERROR"){"Red"}elseif($Level -eq "WARN"){"Yellow"}else{"White"})
    }
}

function Get-ParentClassForType {
    param([string]$Type)

    $defaultParents = @{
        "Actor" = "/Script/Engine.Actor"
        "Component" = "/Script/Engine.ActorComponent"
        "Widget" = "/Script/UMG.UserWidget"
        "GameMode" = "/Script/Engine.GameModeBase"
        "Character" = "/Script/Engine.Character"
        "Pawn" = "/Script/Engine.Pawn"
        "Interface" = "/Script/CoreUObject.Interface"
        "Struct" = "/Script/CoreUObject.ScriptStruct"
        "Enum" = "/Script/CoreUObject.Enum"
        "MacroLibrary" = "/Script/Engine.BlueprintMacroLibrary"
        "FunctionLibrary" = "/Script/Engine.BlueprintFunctionLibrary"
    }

    return $defaultParents[$Type]
}

function New-BlueprintTemplate {
    param(
        [string]$Name,
        [string]$Type,
        [string]$Parent,
        [string]$Path
    )

    if ([string]::IsNullOrEmpty($Parent)) {
        $Parent = Get-ParentClassForType -Type $Type
    }

    $guid = [guid]::NewGuid().ToString("N").ToUpper()
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"

    $template = @"
{
    "Type": "Blueprint",
    "BlueprintType": "$Type",
    "Name": "$Name",
    "ParentClass": "$Parent",
    "GeneratedBy": "GenerateBlueprintTemplate.ps1",
    "GeneratedDate": "$timestamp",
    "GUID": "$guid",
    "Properties": {
        "bReplicates": false,
        "bNetLoadOnClient": true,
        "bNetLoadOnServer": true
    },
    "Components": [],
    "Variables": [],
    "Functions": [],
    "EventGraph": {
        "Nodes": [],
        "Connections": []
    },
    "Metadata": {
        "Category": "Generated",
        "Description": "Auto-generated Blueprint template for $Name",
        "Keywords": "$Type, Blueprint, Generated"
    }
}
"@

    $outputFile = Join-Path $Path "$Name.json"

    if (-not (Test-Path $Path)) {
        New-Item -ItemType Directory -Path $Path -Force | Out-Null
        Write-Log "Created directory: $Path"
    }

    $template | Out-File -FilePath $outputFile -Encoding UTF8
    Write-Log "Generated Blueprint template: $outputFile"

    return $outputFile
}

function New-BlueprintMetadata {
    param(
        [string]$Name,
        [string]$Type,
        [string]$Path
    )

    $metadataFile = Join-Path $Path "$Name.metadata.json"

    $metadata = @{
        "BlueprintName" = $Name
        "BlueprintType" = $Type
        "CreatedDate" = Get-Date -Format "o"
        "LastModified" = Get-Date -Format "o"
        "Version" = "1.0.0"
        "Author" = $env:USERNAME
        "Tags" = @($Type, "Generated", "Template")
        "Dependencies" = @()
        "Notes" = ""
    } | ConvertTo-Json -Depth 10

    $metadata | Out-File -FilePath $metadataFile -Encoding UTF8
    Write-Log "Generated metadata file: $metadataFile"

    return $metadataFile
}

# Main execution
try {
    Write-Log "Starting Blueprint template generation"
    Write-Log "Blueprint Name: $BlueprintName"
    Write-Log "Blueprint Type: $BlueprintType"
    Write-Log "Output Path: $OutputPath"

    # Create logs directory
    $logsDir = Join-Path $PSScriptRoot "Logs"
    if (-not (Test-Path $logsDir)) {
        New-Item -ItemType Directory -Path $logsDir -Force | Out-Null
    }

    # Validate Blueprint name
    if ($BlueprintName -notmatch '^[A-Za-z_][A-Za-z0-9_]*$') {
        throw "Invalid Blueprint name. Must start with letter or underscore and contain only alphanumeric characters and underscores."
    }

    # Generate Blueprint template
    $templateFile = New-BlueprintTemplate -Name $BlueprintName -Type $BlueprintType -Parent $ParentClass -Path $OutputPath

    # Generate metadata
    $metadataFile = New-BlueprintMetadata -Name $BlueprintName -Type $BlueprintType -Path $OutputPath

    Write-Log "Blueprint template generation completed successfully" -Level "INFO"
    Write-Host "`nGenerated files:" -ForegroundColor Green
    Write-Host "  Template: $templateFile" -ForegroundColor Cyan
    Write-Host "  Metadata: $metadataFile" -ForegroundColor Cyan

    return @{
        Success = $true
        TemplateFile = $templateFile
        MetadataFile = $metadataFile
    }

} catch {
    Write-Log "Error: $($_.Exception.Message)" -Level "ERROR"
    Write-Log "Stack trace: $($_.ScriptStackTrace)" -Level "ERROR"
    throw
}
