<#
.SYNOPSIS
    Generate documentation for Blueprint files
.DESCRIPTION
    Creates comprehensive documentation for Blueprints including structure, functions, and usage
.PARAMETER BlueprintPath
    Path to Blueprint file or directory
.PARAMETER OutputFormat
    Output format (HTML, Markdown, JSON)
.PARAMETER IncludeImages
    Include visual diagrams (requires additional tools)
.EXAMPLE
    .\GenerateBlueprintDocs.ps1 -BlueprintPath "D:/gptzuo/HorrorProject/HorrorProject/Content/Blueprints" -OutputFormat "HTML"
#>

param(
    [Parameter(Mandatory=$true)]
    [string]$BlueprintPath,

    [Parameter(Mandatory=$false)]
    [ValidateSet("HTML", "Markdown", "JSON")]
    [string]$OutputFormat = "HTML",

    [Parameter(Mandatory=$false)]
    [switch]$IncludeImages,

    [Parameter(Mandatory=$false)]
    [switch]$Verbose
)

$ErrorActionPreference = "Stop"
$LogFile = Join-Path $PSScriptRoot "Logs/GenerateBlueprintDocs_$(Get-Date -Format 'yyyyMMdd_HHmmss').log"

function Write-Log {
    param([string]$Message, [string]$Level = "INFO")
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    $logMessage = "[$timestamp] [$Level] $Message"
    Add-Content -Path $LogFile -Value $logMessage
    if ($Verbose -or $Level -eq "ERROR") {
        Write-Host $logMessage -ForegroundColor $(if($Level -eq "ERROR"){"Red"}else{"White"})
    }
}

function Get-BlueprintDocumentation {
    param([string]$FilePath)

    $doc = @{
        Name = ""
        Type = ""
        ParentClass = ""
        Description = ""
        Components = @()
        Variables = @()
        Functions = @()
        Events = @()
        Metadata = @{}
    }

    try {
        if ($FilePath -match '\.json$') {
            $content = Get-Content $FilePath -Raw | ConvertFrom-Json

            $doc.Name = $content.Name
            $doc.Type = $content.Type
            $doc.ParentClass = $content.ParentClass
            $doc.Description = if ($content.Metadata.Description) { $content.Metadata.Description } else { "No description available" }
            $doc.Metadata = $content.Metadata

            # Components
            if ($content.Components) {
                foreach ($component in $content.Components) {
                    $doc.Components += @{
                        Name = $component.Name
                        Type = $component.Type
                        AttachParent = $component.AttachParent
                    }
                }
            }

            # Variables
            if ($content.Variables) {
                foreach ($variable in $content.Variables) {
                    $doc.Variables += @{
                        Name = $variable.Name
                        Type = $variable.Type
                        Category = $variable.Category
                        DefaultValue = $variable.DefaultValue
                    }
                }
            }

            # Functions and Events
            if ($content.Functions) {
                foreach ($function in $content.Functions) {
                    $funcDoc = @{
                        Name = $function.Name
                        Type = $function.Type
                        ReturnType = $function.ReturnType
                        Parameters = $function.Parameters
                        Description = $function.Description
                        Category = $function.Category
                    }

                    if ($function.Type -eq "Event") {
                        $doc.Events += $funcDoc
                    } else {
                        $doc.Functions += $funcDoc
                    }
                }
            }
        }

    } catch {
        Write-Log "Failed to extract documentation from $FilePath : $($_.Exception.Message)" -Level "ERROR"
    }

    return $doc
}

function New-HtmlDocumentation {
    param([array]$Docs, [string]$OutputPath)

    $htmlFile = Join-Path $OutputPath "BlueprintDocumentation_$(Get-Date -Format 'yyyyMMdd_HHmmss').html"

    $html = @"
<!DOCTYPE html>
<html>
<head>
    <title>Blueprint Documentation</title>
    <style>
        body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; margin: 0; padding: 20px; background: #f5f5f5; }
        .container { max-width: 1400px; margin: 0 auto; }
        .header { background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; padding: 30px; border-radius: 8px; margin-bottom: 30px; }
        .header h1 { margin: 0; font-size: 36px; }
        .header p { margin: 10px 0 0 0; opacity: 0.9; }
        .toc { background: white; padding: 20px; border-radius: 8px; margin-bottom: 30px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        .toc h2 { margin-top: 0; color: #333; }
        .toc ul { list-style: none; padding: 0; }
        .toc li { padding: 8px 0; border-bottom: 1px solid #eee; }
        .toc a { color: #667eea; text-decoration: none; font-weight: 500; }
        .toc a:hover { text-decoration: underline; }
        .blueprint { background: white; padding: 30px; border-radius: 8px; margin-bottom: 30px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        .blueprint h2 { color: #667eea; margin-top: 0; border-bottom: 3px solid #667eea; padding-bottom: 10px; }
        .blueprint-meta { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 15px; margin: 20px 0; }
        .meta-item { background: #f8f9fa; padding: 15px; border-radius: 6px; border-left: 4px solid #667eea; }
        .meta-label { font-size: 12px; color: #666; text-transform: uppercase; font-weight: 600; }
        .meta-value { font-size: 16px; color: #333; margin-top: 5px; }
        .section { margin: 30px 0; }
        .section h3 { color: #555; border-bottom: 2px solid #eee; padding-bottom: 8px; }
        table { width: 100%; border-collapse: collapse; margin: 15px 0; }
        th { background: #667eea; color: white; padding: 12px; text-align: left; font-weight: 600; }
        td { padding: 10px 12px; border-bottom: 1px solid #eee; }
        tr:hover { background: #f8f9fa; }
        .type { background: #e3f2fd; color: #1976d2; padding: 4px 8px; border-radius: 4px; font-family: 'Courier New', monospace; font-size: 13px; }
        .category { background: #f3e5f5; color: #7b1fa2; padding: 4px 8px; border-radius: 4px; font-size: 12px; }
        .description { color: #666; font-style: italic; }
        .footer { text-align: center; padding: 20px; color: #999; font-size: 14px; }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>Blueprint Documentation</h1>
            <p>Generated on $(Get-Date -Format "MMMM dd, yyyy 'at' HH:mm:ss")</p>
            <p>Total Blueprints: $($Docs.Count)</p>
        </div>

        <div class="toc">
            <h2>Table of Contents</h2>
            <ul>
"@

    foreach ($doc in $Docs) {
        $html += "                <li><a href=`"#$($doc.Name)`">$($doc.Name)</a> - <span class=`"type`">$($doc.Type)</span></li>`n"
    }

    $html += @"
            </ul>
        </div>
"@

    foreach ($doc in $Docs) {
        $html += @"
        <div class="blueprint" id="$($doc.Name)">
            <h2>$($doc.Name)</h2>
            <p class="description">$($doc.Description)</p>

            <div class="blueprint-meta">
                <div class="meta-item">
                    <div class="meta-label">Type</div>
                    <div class="meta-value">$($doc.Type)</div>
                </div>
                <div class="meta-item">
                    <div class="meta-label">Parent Class</div>
                    <div class="meta-value">$($doc.ParentClass)</div>
                </div>
                <div class="meta-item">
                    <div class="meta-label">Components</div>
                    <div class="meta-value">$($doc.Components.Count)</div>
                </div>
                <div class="meta-item">
                    <div class="meta-label">Variables</div>
                    <div class="meta-value">$($doc.Variables.Count)</div>
                </div>
                <div class="meta-item">
                    <div class="meta-label">Functions</div>
                    <div class="meta-value">$($doc.Functions.Count)</div>
                </div>
                <div class="meta-item">
                    <div class="meta-label">Events</div>
                    <div class="meta-value">$($doc.Events.Count)</div>
                </div>
            </div>
"@

        if ($doc.Components.Count -gt 0) {
            $html += @"
            <div class="section">
                <h3>Components</h3>
                <table>
                    <tr>
                        <th>Name</th>
                        <th>Type</th>
                        <th>Attach Parent</th>
                    </tr>
"@
            foreach ($component in $doc.Components) {
                $html += @"
                    <tr>
                        <td>$($component.Name)</td>
                        <td><span class="type">$($component.Type)</span></td>
                        <td>$($component.AttachParent)</td>
                    </tr>
"@
            }
            $html += @"
                </table>
            </div>
"@
        }

        if ($doc.Variables.Count -gt 0) {
            $html += @"
            <div class="section">
                <h3>Variables</h3>
                <table>
                    <tr>
                        <th>Name</th>
                        <th>Type</th>
                        <th>Category</th>
                        <th>Default Value</th>
                    </tr>
"@
            foreach ($variable in $doc.Variables) {
                $html += @"
                    <tr>
                        <td>$($variable.Name)</td>
                        <td><span class="type">$($variable.Type)</span></td>
                        <td><span class="category">$($variable.Category)</span></td>
                        <td>$($variable.DefaultValue)</td>
                    </tr>
"@
            }
            $html += @"
                </table>
            </div>
"@
        }

        if ($doc.Functions.Count -gt 0) {
            $html += @"
            <div class="section">
                <h3>Functions</h3>
                <table>
                    <tr>
                        <th>Name</th>
                        <th>Return Type</th>
                        <th>Parameters</th>
                        <th>Category</th>
                    </tr>
"@
            foreach ($function in $doc.Functions) {
                $html += @"
                    <tr>
                        <td>$($function.Name)</td>
                        <td><span class="type">$($function.ReturnType)</span></td>
                        <td>$($function.Parameters)</td>
                        <td><span class="category">$($function.Category)</span></td>
                    </tr>
"@
            }
            $html += @"
                </table>
            </div>
"@
        }

        if ($doc.Events.Count -gt 0) {
            $html += @"
            <div class="section">
                <h3>Events</h3>
                <table>
                    <tr>
                        <th>Name</th>
                        <th>Parameters</th>
                        <th>Description</th>
                    </tr>
"@
            foreach ($event in $doc.Events) {
                $html += @"
                    <tr>
                        <td>$($event.Name)</td>
                        <td>$($event.Parameters)</td>
                        <td class="description">$($event.Description)</td>
                    </tr>
"@
            }
            $html += @"
                </table>
            </div>
"@
        }

        $html += @"
        </div>
"@
    }

    $html += @"
        <div class="footer">
            <p>Generated by GenerateBlueprintDocs.ps1</p>
        </div>
    </div>
</body>
</html>
"@

    $html | Out-File -FilePath $htmlFile -Encoding UTF8
    Write-Log "HTML documentation saved to: $htmlFile"

    return $htmlFile
}

function New-MarkdownDocumentation {
    param([array]$Docs, [string]$OutputPath)

    $mdFile = Join-Path $OutputPath "BlueprintDocumentation_$(Get-Date -Format 'yyyyMMdd_HHmmss').md"

    $markdown = @"
# Blueprint Documentation

Generated on $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")

Total Blueprints: $($Docs.Count)

## Table of Contents

"@

    foreach ($doc in $Docs) {
        $markdown += "- [$($doc.Name)](#$($doc.Name.ToLower() -replace '[^a-z0-9-]', '-')) - $($doc.Type)`n"
    }

    $markdown += "`n---`n`n"

    foreach ($doc in $Docs) {
        $markdown += @"
## $($doc.Name)

**Type:** $($doc.Type)
**Parent Class:** $($doc.ParentClass)
**Description:** $($doc.Description)

### Overview

- Components: $($doc.Components.Count)
- Variables: $($doc.Variables.Count)
- Functions: $($doc.Functions.Count)
- Events: $($doc.Events.Count)

"@

        if ($doc.Components.Count -gt 0) {
            $markdown += @"
### Components

| Name | Type | Attach Parent |
|------|------|---------------|
"@
            foreach ($component in $doc.Components) {
                $markdown += "| $($component.Name) | $($component.Type) | $($component.AttachParent) |`n"
            }
            $markdown += "`n"
        }

        if ($doc.Variables.Count -gt 0) {
            $markdown += @"
### Variables

| Name | Type | Category | Default Value |
|------|------|----------|---------------|
"@
            foreach ($variable in $doc.Variables) {
                $markdown += "| $($variable.Name) | $($variable.Type) | $($variable.Category) | $($variable.DefaultValue) |`n"
            }
            $markdown += "`n"
        }

        if ($doc.Functions.Count -gt 0) {
            $markdown += @"
### Functions

| Name | Return Type | Parameters | Category |
|------|-------------|------------|----------|
"@
            foreach ($function in $doc.Functions) {
                $markdown += "| $($function.Name) | $($function.ReturnType) | $($function.Parameters) | $($function.Category) |`n"
            }
            $markdown += "`n"
        }

        if ($doc.Events.Count -gt 0) {
            $markdown += @"
### Events

| Name | Parameters | Description |
|------|------------|-------------|
"@
            foreach ($event in $doc.Events) {
                $markdown += "| $($event.Name) | $($event.Parameters) | $($event.Description) |`n"
            }
            $markdown += "`n"
        }

        $markdown += "---`n`n"
    }

    $markdown | Out-File -FilePath $mdFile -Encoding UTF8
    Write-Log "Markdown documentation saved to: $mdFile"

    return $mdFile
}

# Main execution
try {
    Write-Log "Starting Blueprint documentation generation"
    Write-Log "Blueprint Path: $BlueprintPath"
    Write-Log "Output Format: $OutputFormat"

    # Create logs directory
    $logsDir = Join-Path $PSScriptRoot "Logs"
    if (-not (Test-Path $logsDir)) {
        New-Item -ItemType Directory -Path $logsDir -Force | Out-Null
    }

    # Get Blueprint files
    $blueprintFiles = if (Test-Path $BlueprintPath -PathType Leaf) {
        @(Get-Item $BlueprintPath)
    } else {
        Get-ChildItem -Path $BlueprintPath -Filter "*.json" -Recurse |
            Where-Object { $_.Name -notmatch '\.metadata\.json$' }
    }

    Write-Log "Found $($blueprintFiles.Count) Blueprint files"

    if ($blueprintFiles.Count -eq 0) {
        Write-Log "No Blueprint files found" -Level "WARN"
        return
    }

    # Extract documentation
    $docs = @()
    foreach ($file in $blueprintFiles) {
        Write-Log "Extracting documentation from: $($file.Name)"
        $doc = Get-BlueprintDocumentation -FilePath $file.FullName
        $docs += $doc
    }

    # Generate documentation
    $outputFile = switch ($OutputFormat) {
        "HTML" { New-HtmlDocumentation -Docs $docs -OutputPath $logsDir }
        "Markdown" { New-MarkdownDocumentation -Docs $docs -OutputPath $logsDir }
        "JSON" {
            $jsonFile = Join-Path $logsDir "BlueprintDocumentation_$(Get-Date -Format 'yyyyMMdd_HHmmss').json"
            $docs | ConvertTo-Json -Depth 10 | Out-File -FilePath $jsonFile -Encoding UTF8
            Write-Log "JSON documentation saved to: $jsonFile"
            $jsonFile
        }
    }

    Write-Log "Documentation generation completed successfully" -Level "SUCCESS"

    Write-Host "`n=== Documentation Generation Complete ===" -ForegroundColor Cyan
    Write-Host "Blueprints Documented: $($docs.Count)" -ForegroundColor Green
    Write-Host "Output Format: $OutputFormat" -ForegroundColor White
    Write-Host "Output File: $outputFile" -ForegroundColor Cyan

    return @{
        Success = $true
        BlueprintsDocumented = $docs.Count
        OutputFormat = $OutputFormat
        OutputFile = $outputFile
    }

} catch {
    Write-Log "Error: $($_.Exception.Message)" -Level "ERROR"
    Write-Log "Stack trace: $($_.ScriptStackTrace)" -Level "ERROR"
    throw
}
