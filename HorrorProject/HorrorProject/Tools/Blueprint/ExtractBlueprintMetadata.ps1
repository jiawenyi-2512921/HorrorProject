<#
.SYNOPSIS
    Extract metadata from Blueprint files
.DESCRIPTION
    Extracts and exports Blueprint metadata for analysis and reporting
.PARAMETER BlueprintPath
    Path to Blueprint file or directory
.PARAMETER ExportFormat
    Export format (JSON, CSV, XML)
.EXAMPLE
    .\ExtractBlueprintMetadata.ps1 -BlueprintPath "D:/gptzuo/HorrorProject/HorrorProject/Content/Blueprints" -ExportFormat "CSV"
#>

param(
    [Parameter(Mandatory=$true)]
    [string]$BlueprintPath,

    [Parameter(Mandatory=$false)]
    [ValidateSet("JSON", "CSV", "XML")]
    [string]$ExportFormat = "JSON",

    [Parameter(Mandatory=$false)]
    [switch]$Verbose
)

$ErrorActionPreference = "Stop"
$LogFile = Join-Path $PSScriptRoot "Logs/ExtractBlueprintMetadata_$(Get-Date -Format 'yyyyMMdd_HHmmss').log"

function Write-Log {
    param([string]$Message, [string]$Level = "INFO")
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    $logMessage = "[$timestamp] [$Level] $Message"
    Add-Content -Path $LogFile -Value $logMessage
    if ($Verbose -or $Level -eq "ERROR") {
        Write-Host $logMessage -ForegroundColor $(if($Level -eq "ERROR"){"Red"}else{"White"})
    }
}

function Get-BlueprintMetadata {
    param([string]$FilePath)

    $metadata = @{
        FileName = (Get-Item $FilePath).Name
        FilePath = $FilePath
        FileSize = (Get-Item $FilePath).Length
        LastModified = (Get-Item $FilePath).LastWriteTime.ToString("o")
        Name = ""
        Type = ""
        ParentClass = ""
        GUID = ""
        GeneratedBy = ""
        GeneratedDate = ""
        ComponentCount = 0
        VariableCount = 0
        FunctionCount = 0
        EventCount = 0
        Category = ""
        Description = ""
        Keywords = ""
        Author = ""
        Version = ""
    }

    try {
        if ($FilePath -match '\.json$') {
            $content = Get-Content $FilePath -Raw | ConvertFrom-Json

            $metadata.Name = $content.Name
            $metadata.Type = $content.Type
            $metadata.ParentClass = $content.ParentClass
            $metadata.GUID = $content.GUID
            $metadata.GeneratedBy = $content.GeneratedBy
            $metadata.GeneratedDate = $content.GeneratedDate

            if ($content.Components) {
                $metadata.ComponentCount = $content.Components.Count
            }
            if ($content.WidgetTree -and $content.WidgetTree.Components) {
                $metadata.ComponentCount += $content.WidgetTree.Components.Count
            }

            if ($content.Variables) {
                $metadata.VariableCount = $content.Variables.Count
            }

            if ($content.Functions) {
                $metadata.FunctionCount = ($content.Functions | Where-Object { $_.Type -ne "Event" }).Count
                $metadata.EventCount = ($content.Functions | Where-Object { $_.Type -eq "Event" }).Count
            }

            if ($content.Metadata) {
                $metadata.Category = $content.Metadata.Category
                $metadata.Description = $content.Metadata.Description
                $metadata.Keywords = $content.Metadata.Keywords
            }

            # Check for separate metadata file
            $metadataFile = "$FilePath.metadata.json"
            if (Test-Path $metadataFile) {
                $metaContent = Get-Content $metadataFile -Raw | ConvertFrom-Json
                $metadata.Author = $metaContent.Author
                $metadata.Version = $metaContent.Version
            }
        }

    } catch {
        Write-Log "Failed to extract metadata from $FilePath : $($_.Exception.Message)" -Level "ERROR"
    }

    return $metadata
}

function Export-ToCSV {
    param([array]$Metadata, [string]$OutputPath)

    $csvFile = Join-Path $OutputPath "BlueprintMetadata_$(Get-Date -Format 'yyyyMMdd_HHmmss').csv"

    $Metadata | Export-Csv -Path $csvFile -NoTypeInformation -Encoding UTF8
    Write-Log "CSV export saved to: $csvFile"

    return $csvFile
}

function Export-ToXML {
    param([array]$Metadata, [string]$OutputPath)

    $xmlFile = Join-Path $OutputPath "BlueprintMetadata_$(Get-Date -Format 'yyyyMMdd_HHmmss').xml"

    $xml = @"
<?xml version="1.0" encoding="UTF-8"?>
<BlueprintMetadata>
    <GeneratedDate>$(Get-Date -Format "o")</GeneratedDate>
    <TotalBlueprints>$($Metadata.Count)</TotalBlueprints>
    <Blueprints>
"@

    foreach ($meta in $Metadata) {
        $xml += @"

        <Blueprint>
            <FileName>$([System.Security.SecurityElement]::Escape($meta.FileName))</FileName>
            <Name>$([System.Security.SecurityElement]::Escape($meta.Name))</Name>
            <Type>$([System.Security.SecurityElement]::Escape($meta.Type))</Type>
            <ParentClass>$([System.Security.SecurityElement]::Escape($meta.ParentClass))</ParentClass>
            <GUID>$([System.Security.SecurityElement]::Escape($meta.GUID))</GUID>
            <FileSize>$($meta.FileSize)</FileSize>
            <LastModified>$($meta.LastModified)</LastModified>
            <ComponentCount>$($meta.ComponentCount)</ComponentCount>
            <VariableCount>$($meta.VariableCount)</VariableCount>
            <FunctionCount>$($meta.FunctionCount)</FunctionCount>
            <EventCount>$($meta.EventCount)</EventCount>
            <Category>$([System.Security.SecurityElement]::Escape($meta.Category))</Category>
            <Description>$([System.Security.SecurityElement]::Escape($meta.Description))</Description>
        </Blueprint>
"@
    }

    $xml += @"

    </Blueprints>
</BlueprintMetadata>
"@

    $xml | Out-File -FilePath $xmlFile -Encoding UTF8
    Write-Log "XML export saved to: $xmlFile"

    return $xmlFile
}

# Main execution
try {
    Write-Log "Starting Blueprint metadata extraction"
    Write-Log "Blueprint Path: $BlueprintPath"
    Write-Log "Export Format: $ExportFormat"

    # Create logs directory
    $logsDir = Join-Path $PSScriptRoot "Logs"
    if (-not (Test-Path $logsDir)) {
        New-Item -ItemType Directory -Path $logsDir -Force | Out-Null
    }

    # Get Blueprint files
    $blueprintFiles = if (Test-Path $BlueprintPath -PathType Leaf) {
        @(Get-Item $BlueprintPath)
    } else {
        Get-ChildItem -Path $BlueprintPath -Include "*.uasset","*.json" -Recurse |
            Where-Object { $_.Name -notmatch '\.metadata\.json$' }
    }

    Write-Log "Found $($blueprintFiles.Count) Blueprint files"

    if ($blueprintFiles.Count -eq 0) {
        Write-Log "No Blueprint files found" -Level "WARN"
        return
    }

    # Extract metadata
    $allMetadata = @()
    foreach ($file in $blueprintFiles) {
        Write-Log "Extracting metadata from: $($file.Name)"
        $metadata = Get-BlueprintMetadata -FilePath $file.FullName
        $allMetadata += $metadata
    }

    # Export metadata
    $outputFile = switch ($ExportFormat) {
        "CSV" { Export-ToCSV -Metadata $allMetadata -OutputPath $logsDir }
        "XML" { Export-ToXML -Metadata $allMetadata -OutputPath $logsDir }
        "JSON" {
            $jsonFile = Join-Path $logsDir "BlueprintMetadata_$(Get-Date -Format 'yyyyMMdd_HHmmss').json"
            @{
                GeneratedDate = Get-Date -Format "o"
                TotalBlueprints = $allMetadata.Count
                Blueprints = $allMetadata
            } | ConvertTo-Json -Depth 10 | Out-File -FilePath $jsonFile -Encoding UTF8
            Write-Log "JSON export saved to: $jsonFile"
            $jsonFile
        }
    }

    Write-Log "Metadata extraction completed successfully" -Level "SUCCESS"

    Write-Host "`n=== Metadata Extraction Complete ===" -ForegroundColor Cyan
    Write-Host "Blueprints Processed: $($allMetadata.Count)" -ForegroundColor Green
    Write-Host "Export Format: $ExportFormat" -ForegroundColor White
    Write-Host "Output File: $outputFile" -ForegroundColor Cyan

    return @{
        Success = $true
        BlueprintsProcessed = $allMetadata.Count
        ExportFormat = $ExportFormat
        OutputFile = $outputFile
    }

} catch {
    Write-Log "Error: $($_.Exception.Message)" -Level "ERROR"
    Write-Log "Stack trace: $($_.ScriptStackTrace)" -Level "ERROR"
    throw
}
