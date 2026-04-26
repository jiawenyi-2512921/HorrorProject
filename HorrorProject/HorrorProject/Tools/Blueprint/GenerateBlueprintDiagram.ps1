<#
.SYNOPSIS
    Generate visual diagrams for Blueprint structure
.DESCRIPTION
    Creates visual representations of Blueprint architecture and relationships
.PARAMETER BlueprintPath
    Path to Blueprint file
.PARAMETER DiagramType
    Type of diagram (Structure, Hierarchy, Dependencies)
.PARAMETER OutputFormat
    Output format (DOT, Mermaid, PlantUML)
.EXAMPLE
    .\GenerateBlueprintDiagram.ps1 -BlueprintPath "D:/gptzuo/HorrorProject/HorrorProject/Content/Blueprints/BP_Actor.json" -DiagramType "Structure" -OutputFormat "Mermaid"
#>

param(
    [Parameter(Mandatory=$true)]
    [string]$BlueprintPath,

    [Parameter(Mandatory=$false)]
    [ValidateSet("Structure", "Hierarchy", "Dependencies", "Flow")]
    [string]$DiagramType = "Structure",

    [Parameter(Mandatory=$false)]
    [ValidateSet("DOT", "Mermaid", "PlantUML")]
    [string]$OutputFormat = "Mermaid",

    [Parameter(Mandatory=$false)]
    [switch]$Verbose
)

$ErrorActionPreference = "Stop"
$LogFile = Join-Path $PSScriptRoot "Logs/GenerateBlueprintDiagram_$(Get-Date -Format 'yyyyMMdd_HHmmss').log"

function Write-Log {
    param([string]$Message, [string]$Level = "INFO")
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    $logMessage = "[$timestamp] [$Level] $Message"
    Add-Content -Path $LogFile -Value $logMessage
    if ($Verbose -or $Level -eq "ERROR") {
        Write-Host $logMessage -ForegroundColor $(if($Level -eq "ERROR"){"Red"}else{"White"})
    }
}

function New-MermaidStructureDiagram {
    param([hashtable]$Blueprint)

    $diagram = @"
graph TD
    BP["$($Blueprint.Name)<br/>$($Blueprint.Type)"]

"@

    # Add parent class
    if ($Blueprint.ParentClass) {
        $diagram += "    Parent[""$($Blueprint.ParentClass)""]`n"
        $diagram += "    Parent --> BP`n`n"
    }

    # Add components
    if ($Blueprint.Components -and $Blueprint.Components.Count -gt 0) {
        $diagram += "    subgraph Components`n"
        foreach ($component in $Blueprint.Components) {
            $compId = $component.Name -replace '[^a-zA-Z0-9]', '_'
            $diagram += "        $compId[""$($component.Name)<br/>$($component.Type)""]`n"
        }
        $diagram += "    end`n"
        $diagram += "    BP --> Components`n`n"
    }

    # Add variables
    if ($Blueprint.Variables -and $Blueprint.Variables.Count -gt 0) {
        $diagram += "    subgraph Variables`n"
        foreach ($variable in $Blueprint.Variables) {
            $varId = $variable.Name -replace '[^a-zA-Z0-9]', '_'
            $diagram += "        $varId[""$($variable.Name): $($variable.Type)""]`n"
        }
        $diagram += "    end`n"
        $diagram += "    BP --> Variables`n`n"
    }

    # Add functions
    if ($Blueprint.Functions -and $Blueprint.Functions.Count -gt 0) {
        $diagram += "    subgraph Functions`n"
        foreach ($function in $Blueprint.Functions) {
            $funcId = $function.Name -replace '[^a-zA-Z0-9]', '_'
            $diagram += "        $funcId[""$($function.Name)()""]{$($function.Type)}`n"
        }
        $diagram += "    end`n"
        $diagram += "    BP --> Functions`n"
    }

    return $diagram
}

function New-MermaidHierarchyDiagram {
    param([hashtable]$Blueprint)

    $diagram = @"
classDiagram
    class $($Blueprint.ParentClass) {
        <<Parent>>
    }

    class $($Blueprint.Name) {
        <<$($Blueprint.Type)>>
"@

    # Add variables
    foreach ($variable in $Blueprint.Variables) {
        $diagram += "        +$($variable.Type) $($variable.Name)`n"
    }

    # Add functions
    foreach ($function in $Blueprint.Functions) {
        $returnType = if ($function.ReturnType) { $function.ReturnType } else { "void" }
        $diagram += "        +$returnType $($function.Name)()`n"
    }

    $diagram += "    }`n`n"
    $diagram += "    $($Blueprint.ParentClass) <|-- $($Blueprint.Name)`n"

    return $diagram
}

function New-DOTStructureDiagram {
    param([hashtable]$Blueprint)

    $diagram = @"
digraph Blueprint {
    rankdir=TB;
    node [shape=box, style=rounded];

    BP [label="$($Blueprint.Name)\n$($Blueprint.Type)", shape=box, style="rounded,filled", fillcolor=lightblue];

"@

    # Parent class
    if ($Blueprint.ParentClass) {
        $diagram += "    Parent [label=""$($Blueprint.ParentClass)"", fillcolor=lightgray]`n"
        $diagram += "    Parent -> BP [label=""inherits""]`n"
    }

    # Components
    if ($Blueprint.Components -and $Blueprint.Components.Count -gt 0) {
        $diagram += "`n    // Components`n"
        foreach ($component in $Blueprint.Components) {
            $compId = $component.Name -replace '[^a-zA-Z0-9]', '_'
            $diagram += "    $compId [label=""$($component.Name)\n$($component.Type)"", fillcolor=lightgreen]`n"
            $diagram += "    BP -> $compId`n"
        }
    }

    # Variables
    if ($Blueprint.Variables -and $Blueprint.Variables.Count -gt 0) {
        $diagram += "`n    // Variables`n"
        foreach ($variable in $Blueprint.Variables) {
            $varId = $variable.Name -replace '[^a-zA-Z0-9]', '_'
            $diagram += "    $varId [label=""$($variable.Name): $($variable.Type)"", fillcolor=lightyellow]`n"
            $diagram += "    BP -> $varId`n"
        }
    }

    # Functions
    if ($Blueprint.Functions -and $Blueprint.Functions.Count -gt 0) {
        $diagram += "`n    // Functions`n"
        foreach ($function in $Blueprint.Functions) {
            $funcId = $function.Name -replace '[^a-zA-Z0-9]', '_'
            $diagram += "    $funcId [label=""$($function.Name)()"", fillcolor=lightpink]`n"
            $diagram += "    BP -> $funcId`n"
        }
    }

    $diagram += "}`n"

    return $diagram
}

function New-PlantUMLDiagram {
    param([hashtable]$Blueprint)

    $diagram = @"
@startuml
!theme plain

class "$($Blueprint.ParentClass)" <<Parent>> {
}

class "$($Blueprint.Name)" <<$($Blueprint.Type)>> {
"@

    # Add variables
    foreach ($variable in $Blueprint.Variables) {
        $diagram += "    +$($variable.Type) $($variable.Name)`n"
    }

    $diagram += "    --`n"

    # Add functions
    foreach ($function in $Blueprint.Functions) {
        $returnType = if ($function.ReturnType) { $function.ReturnType } else { "void" }
        $diagram += "    +$returnType $($function.Name)()`n"
    }

    $diagram += "}`n`n"
    $diagram += """$($Blueprint.ParentClass)"" <|-- ""$($Blueprint.Name)""`n"

    # Add components as associations
    foreach ($component in $Blueprint.Components) {
        $diagram += """$($Blueprint.Name)"" *-- ""$($component.Type)"" : $($component.Name)`n"
    }

    $diagram += "@enduml`n"

    return $diagram
}

# Main execution
try {
    Write-Log "Starting Blueprint diagram generation"
    Write-Log "Blueprint Path: $BlueprintPath"
    Write-Log "Diagram Type: $DiagramType"
    Write-Log "Output Format: $OutputFormat"

    # Create logs directory
    $logsDir = Join-Path $PSScriptRoot "Logs"
    if (-not (Test-Path $logsDir)) {
        New-Item -ItemType Directory -Path $logsDir -Force | Out-Null
    }

    # Validate Blueprint file
    if (-not (Test-Path $BlueprintPath)) {
        throw "Blueprint file not found: $BlueprintPath"
    }

    # Load Blueprint
    $blueprint = @{
        Name = ""
        Type = ""
        ParentClass = ""
        Components = @()
        Variables = @()
        Functions = @()
    }

    if ($BlueprintPath -match '\.json$') {
        $content = Get-Content $BlueprintPath -Raw | ConvertFrom-Json
        $blueprint.Name = $content.Name
        $blueprint.Type = $content.Type
        $blueprint.ParentClass = $content.ParentClass
        $blueprint.Components = $content.Components
        $blueprint.Variables = $content.Variables
        $blueprint.Functions = $content.Functions
    } else {
        throw "Only JSON Blueprint files are supported for diagram generation"
    }

    Write-Log "Loaded Blueprint: $($blueprint.Name)"

    # Generate diagram
    $diagram = switch ($OutputFormat) {
        "Mermaid" {
            switch ($DiagramType) {
                "Structure" { New-MermaidStructureDiagram -Blueprint $blueprint }
                "Hierarchy" { New-MermaidHierarchyDiagram -Blueprint $blueprint }
                default { New-MermaidStructureDiagram -Blueprint $blueprint }
            }
        }
        "DOT" {
            New-DOTStructureDiagram -Blueprint $blueprint
        }
        "PlantUML" {
            New-PlantUMLDiagram -Blueprint $blueprint
        }
    }

    # Save diagram
    $extension = switch ($OutputFormat) {
        "Mermaid" { "mmd" }
        "DOT" { "dot" }
        "PlantUML" { "puml" }
    }

    $outputFile = Join-Path $logsDir "$($blueprint.Name)_$DiagramType.$extension"
    $diagram | Out-File -FilePath $outputFile -Encoding UTF8

    Write-Log "Diagram saved to: $outputFile" -Level "SUCCESS"

    # Generate HTML preview for Mermaid
    if ($OutputFormat -eq "Mermaid") {
        $htmlFile = Join-Path $logsDir "$($blueprint.Name)_$DiagramType.html"
        $html = @"
<!DOCTYPE html>
<html>
<head>
    <title>$($blueprint.Name) - $DiagramType Diagram</title>
    <script src="https://cdn.jsdelivr.net/npm/mermaid/dist/mermaid.min.js"></script>
    <script>mermaid.initialize({startOnLoad:true});</script>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background: #f5f5f5; }
        .container { max-width: 1200px; margin: 0 auto; background: white; padding: 30px; border-radius: 8px; }
        h1 { color: #333; }
        .mermaid { background: white; padding: 20px; border-radius: 4px; }
    </style>
</head>
<body>
    <div class="container">
        <h1>$($blueprint.Name) - $DiagramType Diagram</h1>
        <p>Generated: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")</p>
        <div class="mermaid">
$diagram
        </div>
    </div>
</body>
</html>
"@
        $html | Out-File -FilePath $htmlFile -Encoding UTF8
        Write-Log "HTML preview saved to: $htmlFile"
    }

    Write-Host "`n=== Diagram Generation Complete ===" -ForegroundColor Cyan
    Write-Host "Blueprint: $($blueprint.Name)" -ForegroundColor White
    Write-Host "Diagram Type: $DiagramType" -ForegroundColor White
    Write-Host "Output Format: $OutputFormat" -ForegroundColor White
    Write-Host "Output File: $outputFile" -ForegroundColor Cyan

    return @{
        Success = $true
        BlueprintName = $blueprint.Name
        DiagramType = $DiagramType
        OutputFormat = $OutputFormat
        OutputFile = $outputFile
    }

} catch {
    Write-Log "Error: $($_.Exception.Message)" -Level "ERROR"
    Write-Log "Stack trace: $($_.ScriptStackTrace)" -Level "ERROR"
    throw
}
