<#
.SYNOPSIS
    Convert C++ classes to Blueprint assistance tool
.DESCRIPTION
    Analyzes C++ classes and generates Blueprint template scaffolding
.PARAMETER CppHeaderPath
    Path to C++ header file
.PARAMETER OutputPath
    Output directory for Blueprint template
.PARAMETER GenerateStubs
    Generate Blueprint function stubs
.EXAMPLE
    .\ConvertToBlueprint.ps1 -CppHeaderPath "D:/gptzuo/HorrorProject/HorrorProject/Source/HorrorProject/MyActor.h" -OutputPath "D:/gptzuo/HorrorProject/HorrorProject/Content/Blueprints" -GenerateStubs
#>

param(
    [Parameter(Mandatory=$true)]
    [string]$CppHeaderPath,

    [Parameter(Mandatory=$true)]
    [string]$OutputPath,

    [Parameter(Mandatory=$false)]
    [switch]$GenerateStubs,

    [Parameter(Mandatory=$false)]
    [switch]$Verbose
)

$ErrorActionPreference = "Stop"
$LogFile = Join-Path $PSScriptRoot "Logs/ConvertToBlueprint_$(Get-Date -Format 'yyyyMMdd_HHmmss').log"

function Write-Log {
    param([string]$Message, [string]$Level = "INFO")
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    $logMessage = "[$timestamp] [$Level] $Message"
    Add-Content -Path $LogFile -Value $logMessage
    if ($Verbose -or $Level -eq "ERROR" -or $Level -eq "WARN") {
        $color = switch ($Level) {
            "ERROR" { "Red" }
            "WARN" { "Yellow" }
            "SUCCESS" { "Green" }
            default { "White" }
        }
        Write-Host $logMessage -ForegroundColor $color
    }
}

function Parse-CppHeader {
    param([string]$FilePath)

    $content = Get-Content $FilePath -Raw

    $classInfo = @{
        ClassName = ""
        ParentClass = ""
        Properties = @()
        Functions = @()
        Components = @()
        IsActor = $false
        IsComponent = $false
        IsWidget = $false
    }

    # Extract class name and parent
    if ($content -match 'class\s+\w+\s+(\w+)\s*:\s*public\s+(\w+)') {
        $classInfo.ClassName = $matches[1]
        $classInfo.ParentClass = $matches[2]

        # Determine type
        $classInfo.IsActor = $classInfo.ParentClass -match 'Actor|Character|Pawn'
        $classInfo.IsComponent = $classInfo.ParentClass -match 'Component'
        $classInfo.IsWidget = $classInfo.ParentClass -match 'UserWidget'
    }

    # Extract UPROPERTY declarations
    $propertyPattern = 'UPROPERTY\((.*?)\)\s+(\w+[\w\s\*<>]+?)\s+(\w+);'
    $propertyMatches = [regex]::Matches($content, $propertyPattern, [System.Text.RegularExpressions.RegexOptions]::Singleline)

    foreach ($match in $propertyMatches) {
        $specifiers = $match.Groups[1].Value
        $type = $match.Groups[2].Value.Trim()
        $name = $match.Groups[3].Value

        $property = @{
            Name = $name
            Type = $type
            Specifiers = $specifiers
            IsEditAnywhere = $specifiers -match 'EditAnywhere'
            IsBlueprintReadWrite = $specifiers -match 'BlueprintReadWrite'
            IsVisibleAnywhere = $specifiers -match 'VisibleAnywhere'
            Category = if ($specifiers -match 'Category\s*=\s*"([^"]+)"') { $matches[1] } else { "Default" }
        }

        # Check if it's a component
        if ($type -match 'Component\*$') {
            $classInfo.Components += $property
        } else {
            $classInfo.Properties += $property
        }
    }

    # Extract UFUNCTION declarations
    $functionPattern = 'UFUNCTION\((.*?)\)\s+(?:virtual\s+)?(\w+[\w\s\*<>]*?)\s+(\w+)\s*\((.*?)\)'
    $functionMatches = [regex]::Matches($content, $functionPattern, [System.Text.RegularExpressions.RegexOptions]::Singleline)

    foreach ($match in $functionMatches) {
        $specifiers = $match.Groups[1].Value
        $returnType = $match.Groups[2].Value.Trim()
        $name = $match.Groups[3].Value
        $params = $match.Groups[4].Value

        $function = @{
            Name = $name
            ReturnType = $returnType
            Parameters = $params
            Specifiers = $specifiers
            IsBlueprintCallable = $specifiers -match 'BlueprintCallable'
            IsBlueprintImplementableEvent = $specifiers -match 'BlueprintImplementableEvent'
            IsBlueprintNativeEvent = $specifiers -match 'BlueprintNativeEvent'
            Category = if ($specifiers -match 'Category\s*=\s*"([^"]+)"') { $matches[1] } else { "Default" }
        }

        $classInfo.Functions += $function
    }

    return $classInfo
}

function New-BlueprintFromCpp {
    param([hashtable]$ClassInfo, [string]$OutputPath)

    $blueprintName = "BP_$($ClassInfo.ClassName)"
    $guid = [guid]::NewGuid().ToString("N").ToUpper()

    # Determine Blueprint type
    $blueprintType = if ($ClassInfo.IsActor) { "ActorBlueprint" }
                     elseif ($ClassInfo.IsComponent) { "ComponentBlueprint" }
                     elseif ($ClassInfo.IsWidget) { "WidgetBlueprint" }
                     else { "Blueprint" }

    # Map C++ parent class to UE path
    $parentClassPath = "/Script/Engine.$($ClassInfo.ParentClass)"

    $blueprint = @{
        Type = $blueprintType
        Name = $blueprintName
        ParentClass = $parentClassPath
        SourceCppClass = $ClassInfo.ClassName
        GeneratedBy = "ConvertToBlueprint.ps1"
        GeneratedDate = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
        GUID = $guid
        Components = @()
        Variables = @()
        Functions = @()
        Metadata = @{
            Category = "Generated/FromCpp"
            Description = "Blueprint generated from C++ class: $($ClassInfo.ClassName)"
            Keywords = "Blueprint, Generated, C++, $($ClassInfo.ClassName)"
        }
    }

    # Add components
    foreach ($component in $ClassInfo.Components) {
        $blueprint.Components += @{
            Name = $component.Name
            Type = $component.Type -replace '\*$', ''
            Category = $component.Category
            Properties = @{}
        }
    }

    # Add variables
    foreach ($property in $ClassInfo.Properties) {
        $blueprint.Variables += @{
            Name = $property.Name
            Type = $property.Type
            Category = $property.Category
            IsEditAnywhere = $property.IsEditAnywhere
            IsBlueprintReadWrite = $property.IsBlueprintReadWrite
            DefaultValue = $null
        }
    }

    # Add functions
    foreach ($function in $ClassInfo.Functions) {
        $blueprint.Functions += @{
            Name = $function.Name
            Type = if ($function.IsBlueprintImplementableEvent -or $function.IsBlueprintNativeEvent) { "Event" } else { "Function" }
            ReturnType = $function.ReturnType
            Parameters = $function.Parameters
            Category = $function.Category
            IsBlueprintCallable = $function.IsBlueprintCallable
            Description = "Exposed from C++ class"
        }
    }

    $outputFile = Join-Path $OutputPath "$blueprintName.json"

    if (-not (Test-Path $OutputPath)) {
        New-Item -ItemType Directory -Path $OutputPath -Force | Out-Null
        Write-Log "Created directory: $OutputPath"
    }

    $blueprint | ConvertTo-Json -Depth 10 | Out-File -FilePath $outputFile -Encoding UTF8
    Write-Log "Generated Blueprint template: $outputFile" -Level "SUCCESS"

    return $outputFile
}

function New-BlueprintStubs {
    param([hashtable]$ClassInfo, [string]$OutputPath)

    $blueprintName = "BP_$($ClassInfo.ClassName)"
    $stubFile = Join-Path $OutputPath "$blueprintName`_Stubs.txt"

    $stubs = @"
Blueprint Function Stubs for: $($ClassInfo.ClassName)
Generated: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")

=== Blueprint Implementable Events ===
"@

    $implementableEvents = $ClassInfo.Functions | Where-Object { $_.IsBlueprintImplementableEvent -or $_.IsBlueprintNativeEvent }
    if ($implementableEvents.Count -gt 0) {
        foreach ($func in $implementableEvents) {
            $stubs += @"

Event: $($func.Name)
Category: $($func.Category)
Return Type: $($func.ReturnType)
Parameters: $($func.Parameters)
Description: Override this event in Blueprint to implement custom behavior
"@
        }
    } else {
        $stubs += "`nNo implementable events found.`n"
    }

    $stubs += @"

=== Blueprint Callable Functions ===
"@

    $callableFunctions = $ClassInfo.Functions | Where-Object { $_.IsBlueprintCallable }
    if ($callableFunctions.Count -gt 0) {
        foreach ($func in $callableFunctions) {
            $stubs += @"

Function: $($func.Name)
Category: $($func.Category)
Return Type: $($func.ReturnType)
Parameters: $($func.Parameters)
Description: Call this function from Blueprint
"@
        }
    } else {
        $stubs += "`nNo callable functions found.`n"
    }

    $stubs += @"

=== Blueprint Editable Properties ===
"@

    $editableProps = $ClassInfo.Properties | Where-Object { $_.IsEditAnywhere -or $_.IsBlueprintReadWrite }
    if ($editableProps.Count -gt 0) {
        foreach ($prop in $editableProps) {
            $stubs += @"

Property: $($prop.Name)
Type: $($prop.Type)
Category: $($prop.Category)
Editable: $($prop.IsEditAnywhere)
Blueprint Read/Write: $($prop.IsBlueprintReadWrite)
"@
        }
    } else {
        $stubs += "`nNo editable properties found.`n"
    }

    $stubs | Out-File -FilePath $stubFile -Encoding UTF8
    Write-Log "Generated Blueprint stubs: $stubFile"

    return $stubFile
}

# Main execution
try {
    Write-Log "Starting C++ to Blueprint conversion"
    Write-Log "C++ Header: $CppHeaderPath"
    Write-Log "Output Path: $OutputPath"

    # Create logs directory
    $logsDir = Join-Path $PSScriptRoot "Logs"
    if (-not (Test-Path $logsDir)) {
        New-Item -ItemType Directory -Path $logsDir -Force | Out-Null
    }

    # Validate header file
    if (-not (Test-Path $CppHeaderPath)) {
        throw "C++ header file not found: $CppHeaderPath"
    }

    if ($CppHeaderPath -notmatch '\.h$') {
        throw "File must be a C++ header (.h): $CppHeaderPath"
    }

    # Parse C++ header
    Write-Log "Parsing C++ header..."
    $classInfo = Parse-CppHeader -FilePath $CppHeaderPath

    if ([string]::IsNullOrEmpty($classInfo.ClassName)) {
        throw "Could not extract class information from header file"
    }

    Write-Log "Found class: $($classInfo.ClassName) (Parent: $($classInfo.ParentClass))"
    Write-Log "  Components: $($classInfo.Components.Count)"
    Write-Log "  Properties: $($classInfo.Properties.Count)"
    Write-Log "  Functions: $($classInfo.Functions.Count)"

    # Generate Blueprint template
    Write-Log "Generating Blueprint template..."
    $blueprintFile = New-BlueprintFromCpp -ClassInfo $classInfo -OutputPath $OutputPath

    # Generate stubs if requested
    $stubFile = $null
    if ($GenerateStubs) {
        Write-Log "Generating Blueprint stubs..."
        $stubFile = New-BlueprintStubs -ClassInfo $classInfo -OutputPath $OutputPath
    }

    Write-Log "Conversion completed successfully" -Level "SUCCESS"

    Write-Host "`n=== C++ to Blueprint Conversion Complete ===" -ForegroundColor Cyan
    Write-Host "Class: $($classInfo.ClassName)" -ForegroundColor White
    Write-Host "Blueprint Type: $(if($classInfo.IsActor){'Actor'}elseif($classInfo.IsComponent){'Component'}elseif($classInfo.IsWidget){'Widget'}else{'Generic'})" -ForegroundColor White
    Write-Host "Components: $($classInfo.Components.Count)" -ForegroundColor White
    Write-Host "Properties: $($classInfo.Properties.Count)" -ForegroundColor White
    Write-Host "Functions: $($classInfo.Functions.Count)" -ForegroundColor White
    Write-Host "`nGenerated files:" -ForegroundColor Green
    Write-Host "  Blueprint: $blueprintFile" -ForegroundColor Cyan
    if ($stubFile) {
        Write-Host "  Stubs: $stubFile" -ForegroundColor Cyan
    }

    return @{
        Success = $true
        ClassName = $classInfo.ClassName
        BlueprintFile = $blueprintFile
        StubFile = $stubFile
        ComponentCount = $classInfo.Components.Count
        PropertyCount = $classInfo.Properties.Count
        FunctionCount = $classInfo.Functions.Count
    }

} catch {
    Write-Log "Error: $($_.Exception.Message)" -Level "ERROR"
    Write-Log "Stack trace: $($_.ScriptStackTrace)" -Level "ERROR"
    throw
}
