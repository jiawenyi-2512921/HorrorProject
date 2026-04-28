# Validates that the Day1 vertical slice can survive a clean checkout.

param(
    [switch]$AllowUntracked,
    [switch]$NoExitCode
)

$ErrorActionPreference = "Stop"

. (Join-Path $PSScriptRoot "Common.ps1")

$ProjectRoot = Get-HorrorProjectRoot -StartPath $PSScriptRoot
$Issues = New-Object System.Collections.Generic.List[object]
$Warnings = New-Object System.Collections.Generic.List[object]
$script:GitLfsCoverageSeen = @{}
$script:GitLfsCoverageChecked = 0
$script:GitLfsCoverageCovered = 0
$script:GitLfsCoverageMissing = 0
$script:GitLfsCoverageSkippedMissing = 0

function Invoke-Day1Git {
    param(
        [Parameter(Mandatory = $true)]
        [string[]]$Arguments
    )

    $PreviousErrorActionPreference = $ErrorActionPreference
    $ErrorActionPreference = "Continue"
    try {
        $Output = & git -C $ProjectRoot @Arguments 2>&1
        return [PSCustomObject]@{
            ExitCode = $LASTEXITCODE
            Output = @($Output)
        }
    }
    finally {
        $ErrorActionPreference = $PreviousErrorActionPreference
    }
}

function ConvertTo-GitPath {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path
    )

    return ($Path -replace "\\", "/")
}

function Add-Day1Result {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Severity,

        [Parameter(Mandatory = $true)]
        [string]$Category,

        [Parameter(Mandatory = $true)]
        [string]$Path,

        [Parameter(Mandatory = $true)]
        [string]$Message,

        [string]$Advice = ""
    )

    $Result = [PSCustomObject]@{
        Severity = $Severity
        Category = $Category
        Path = $Path
        Message = $Message
        Advice = $Advice
    }

    if ($Severity -eq "WARN") {
        $Warnings.Add($Result) | Out-Null
    }
    else {
        $Issues.Add($Result) | Out-Null
    }
}

function Test-GitTracked {
    param(
        [Parameter(Mandatory = $true)]
        [string]$RelativePath
    )

    $GitPath = ConvertTo-GitPath -Path $RelativePath
    $Result = Invoke-Day1Git -Arguments @("ls-files", "--error-unmatch", "--", $GitPath)
    return ($Result.ExitCode -eq 0)
}

function Test-GitIgnored {
    param(
        [Parameter(Mandatory = $true)]
        [string]$RelativePath
    )

    $GitPath = ConvertTo-GitPath -Path $RelativePath
    $Result = Invoke-Day1Git -Arguments @("check-ignore", "-q", "--", $GitPath)
    return ($Result.ExitCode -eq 0)
}

function Get-Day1GitAttributes {
    param(
        [Parameter(Mandatory = $true)]
        [string]$RelativePath
    )

    $GitPath = ConvertTo-GitPath -Path $RelativePath
    foreach ($CandidatePath in @($GitPath, "./$GitPath")) {
        $Result = Invoke-Day1Git -Arguments @("check-attr", "filter", "diff", "merge", "text", "--", $CandidatePath)
        $Attributes = @{}
        foreach ($Line in $Result.Output) {
            $Text = [string]$Line
            if ($Text -match "^.+?:\s+([^:]+):\s+(.+)$") {
                $Attributes[$Matches[1]] = $Matches[2]
            }
        }

        if ($Attributes.Count -gt 0) {
            return $Attributes
        }

        if ($Result.ExitCode -eq 0) {
            return @{}
        }
    }

    return $null
}

function Format-Day1GitAttributes {
    param(
        [hashtable]$Attributes
    )

    if ($null -eq $Attributes) {
        return "unavailable"
    }

    $Parts = @()
    foreach ($Name in @("filter", "diff", "merge", "text")) {
        $Value = "unspecified"
        if ($Attributes.ContainsKey($Name)) {
            $Value = $Attributes[$Name]
        }
        $Parts += ("{0}={1}" -f $Name, $Value)
    }

    return ($Parts -join ", ")
}

function Test-Day1BinaryLfsCoverage {
    param(
        [Parameter(Mandatory = $true)]
        [string]$RelativePath,

        [Parameter(Mandatory = $true)]
        [string]$Label
    )

    $Extension = [System.IO.Path]::GetExtension($RelativePath).ToLowerInvariant()
    if ($Extension -notin @(".uasset", ".umap")) {
        return
    }

    $GitPath = ConvertTo-GitPath -Path $RelativePath
    $CoverageKey = $GitPath.ToLowerInvariant()
    if ($script:GitLfsCoverageSeen.ContainsKey($CoverageKey)) {
        return
    }
    $script:GitLfsCoverageSeen[$CoverageKey] = $true

    $Path = Join-Path $ProjectRoot $RelativePath
    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        $script:GitLfsCoverageSkippedMissing++
        return
    }

    $script:GitLfsCoverageChecked++
    $Attributes = Get-Day1GitAttributes -RelativePath $RelativePath
    $IsLfsCovered = (
        $null -ne $Attributes -and
        $Attributes.ContainsKey("filter") -and $Attributes["filter"] -eq "lfs" -and
        $Attributes.ContainsKey("diff") -and $Attributes["diff"] -eq "lfs" -and
        $Attributes.ContainsKey("merge") -and $Attributes["merge"] -eq "lfs" -and
        $Attributes.ContainsKey("text") -and $Attributes["text"] -eq "unset"
    )

    if ($IsLfsCovered) {
        $script:GitLfsCoverageCovered++
        return
    }

    $script:GitLfsCoverageMissing++
    $ActualAttributes = Format-Day1GitAttributes -Attributes $Attributes
    Add-Day1Result `
        -Severity "ERROR" `
        -Category "GitLfsCoverage" `
        -Path $RelativePath `
        -Message "$Label is not covered by the expected Git LFS attributes. Actual: $ActualAttributes." `
        -Advice "Add or repair a .gitattributes rule such as '*$Extension filter=lfs diff=lfs merge=lfs -text' before packaging from a clean checkout."
}

function Test-Day1File {
    param(
        [Parameter(Mandatory = $true)]
        [string]$RelativePath,

        [Parameter(Mandatory = $true)]
        [string]$Label
    )

    $Path = Join-Path $ProjectRoot $RelativePath
    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        Add-Day1Result `
            -Severity "ERROR" `
            -Category "MissingCritical" `
            -Path $RelativePath `
            -Message "$Label is missing." `
            -Advice "Restore or create this required Day1 file before local packaging or delivery."
        return
    }

    Test-Day1BinaryLfsCoverage -RelativePath $RelativePath -Label $Label

    if (Test-GitTracked -RelativePath $RelativePath) {
        return
    }

    if (Test-GitIgnored -RelativePath $RelativePath) {
        Add-Day1Result `
            -Severity "ERROR" `
            -Category "IgnoredCritical" `
            -Path $RelativePath `
            -Message "$Label exists but is ignored by git; it will be absent from clean checkouts." `
            -Advice "Remove the ignore rule or move the file into a tracked Day1 location."
        return
    }

    if ($AllowUntracked) {
        Add-Day1Result `
            -Severity "WARN" `
            -Category "LocalUntrackedAllowed" `
            -Path $RelativePath `
            -Message "$Label exists locally but is not tracked yet." `
            -Advice "Allowed only for local development with -AllowUntracked; strict delivery still requires tracking."
    }
    else {
        Add-Day1Result `
            -Severity "ERROR" `
            -Category "StrictUntrackedFailure" `
            -Path $RelativePath `
            -Message "$Label exists locally but strict mode requires it to be tracked." `
            -Advice "Track the file, and use Git LFS for binary assets, before release packaging."
    }
}

function Test-Day1Directory {
    param(
        [Parameter(Mandatory = $true)]
        [string]$RelativePath,

        [Parameter(Mandatory = $true)]
        [string]$Label
    )

    $Path = Join-Path $ProjectRoot $RelativePath
    if (-not (Test-Path -LiteralPath $Path -PathType Container)) {
        Add-Day1Result `
            -Severity "ERROR" `
            -Category "MissingCritical" `
            -Path $RelativePath `
            -Message "$Label directory is missing." `
            -Advice "Restore this required Day1 content directory before local packaging or delivery."
        return
    }

    $Files = @(Get-ChildItem -LiteralPath $Path -File -Recurse)
    if ($Files.Count -eq 0) {
        Add-Day1Result `
            -Severity "ERROR" `
            -Category "MissingCritical" `
            -Path $RelativePath `
            -Message "$Label directory exists but contains no files." `
            -Advice "Restore the required Day1 content files before local packaging or delivery."
        return
    }

    foreach ($File in $Files) {
        $RelativeFile = $File.FullName.Substring($ProjectRoot.Length + 1)
        Test-Day1File -RelativePath $RelativeFile -Label "$Label content"
    }
}

function Test-Day1DocsForSavedPython {
    $DocFiles = @(Get-ChildItem -LiteralPath (Join-Path $ProjectRoot "Docs") -Filter "*.md" -File -Recurse)
    foreach ($DocFile in $DocFiles) {
        $Matches = @(Select-String -LiteralPath $DocFile.FullName -Pattern "Saved[\\/][^`r`n`)]*\.py", "Saved[^`r`n`)]*\.py" -AllMatches)
        foreach ($Match in $Matches) {
            $RelativePath = $DocFile.FullName.Substring($ProjectRoot.Length + 1)
            Add-Day1Result `
                -Severity "ERROR" `
                -Category "TransientScriptReference" `
                -Path "${RelativePath}:$($Match.LineNumber)" `
                -Message "Docs reference Saved/*.py." `
                -Advice "Move reusable verification scripts to Scripts and update the doc reference."
        }
    }
}

function Write-Day1ResultGroup {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Title,

        [object[]]$Results = @(),

        [ConsoleColor]$Color = [ConsoleColor]::White
    )

    if ($Results.Count -eq 0) {
        return
    }

    Write-Host $Title -ForegroundColor $Color
    foreach ($Result in @($Results | Sort-Object Category, Path)) {
        Write-Host ("[{0}] [{1}] {2} - {3}" -f $Result.Severity, $Result.Category, $Result.Path, $Result.Message) -ForegroundColor $Color
        if (-not [string]::IsNullOrWhiteSpace($Result.Advice)) {
            Write-Host ("    Advice: {0}" -f $Result.Advice) -ForegroundColor $Color
        }
    }
}

Write-Host "=== Day1 Source Control Gate ===" -ForegroundColor Cyan
Write-Host "Project: $ProjectRoot"
if ($AllowUntracked) {
    Write-Host "Mode: local development (-AllowUntracked)" -ForegroundColor Yellow
    Write-Host "Untracked-but-present Day1 files are WARN only. Missing or ignored critical files still fail." -ForegroundColor Yellow
}
else {
    Write-Host "Mode: strict release/package gate" -ForegroundColor Yellow
    Write-Host "Every required Day1 file must be tracked; binary assets must be covered by Git LFS attributes." -ForegroundColor Yellow
}
Write-Host ""

$InsideWorkTree = Invoke-Day1Git -Arguments @("rev-parse", "--is-inside-work-tree")
if ($InsideWorkTree.ExitCode -ne 0 -or ($InsideWorkTree.Output -join "") -ne "true") {
    throw "Project root is not inside a git work tree: $ProjectRoot"
}

$RequiredFiles = @(
    @{ Path = "Scripts\Validation\Day1SourceControlGate.ps1"; Label = "Day1 source control gate script" },
    @{ Path = "Scripts\Validation\VerifyDay1PasswordDoors.py"; Label = "Day1 password door validation script" },
    @{ Path = "Scripts\PackageDay1Slice.ps1"; Label = "Day1 package script" },
    @{ Path = "Source\HorrorProject\UI\Day1SliceHUD.cpp"; Label = "Day1 HUD implementation" },
    @{ Path = "Source\HorrorProject\UI\Day1SliceHUD.h"; Label = "Day1 HUD header" },
    @{ Path = "Content\DeepWaterStation\Maps\DemoMap_VerticalSlice_Day1.umap"; Label = "Day1 demo map" },
    @{ Path = "Source\HorrorProject\AI\Tests\HorrorThreatDay1RuntimeTests.cpp"; Label = "Day1 threat runtime test" },
    @{ Path = "Source\HorrorProject\Audio\Tests\HorrorAudioDay1RuntimeTests.cpp"; Label = "Day1 audio runtime test" },
    @{ Path = "Source\HorrorProject\Game\Tests\Day1PackagingConfigTests.cpp"; Label = "Day1 packaging config test" },
    @{ Path = "Source\HorrorProject\Game\Tests\DeepWaterStationRouteKitRuntimeTests.cpp"; Label = "Day1 route kit runtime test" },
    @{ Path = "Source\HorrorProject\Game\Tests\FoundFootageObjectivePromptRuntimeTests.cpp"; Label = "Day1 objective prompt runtime test" },
    @{ Path = "Source\HorrorProject\Game\Tests\HorrorDay1CompleteTests.cpp"; Label = "Day1 completion test" },
    @{ Path = "Source\HorrorProject\Game\Tests\HorrorEncounterRuntimeFeedbackTests.cpp"; Label = "Day1 encounter feedback test" },
    @{ Path = "Source\HorrorProject\Game\Tests\HorrorGameModeBaseBootstrapTests.cpp"; Label = "Day1 game mode bootstrap test" },
    @{ Path = "Source\HorrorProject\Game\Tests\HorrorPlayerFailureRecoveryTests.cpp"; Label = "Day1 failure recovery test" },
    @{ Path = "Source\HorrorProject\Interaction\Tests\DoorInteractableRuntimeTests.cpp"; Label = "Day1 door runtime test" },
    @{ Path = "Source\HorrorProject\Player\Tests\HorrorPlayerControllerPauseTests.cpp"; Label = "Day1 pause runtime test" },
    @{ Path = "Source\HorrorProject\Player\Tests\HorrorPlayerJumpTests.cpp"; Label = "Day1 player jump test" },
    @{ Path = "Source\HorrorProject\Save\Tests\HorrorSaveSubsystemDay1AutosaveTests.cpp"; Label = "Day1 autosave test" },
    @{ Path = "Source\HorrorProject\UI\Tests\Day1SliceHUDRuntimeTests.cpp"; Label = "Day1 HUD runtime test" }
)

foreach ($Spec in $RequiredFiles) {
    Test-Day1File -RelativePath $Spec.Path -Label $Spec.Label
}

$Day1Docs = @(Get-ChildItem -LiteralPath (Join-Path $ProjectRoot "Docs") -Filter "VerticalSlice_Day1_*.md" -File)
if ($Day1Docs.Count -eq 0) {
    Add-Day1Result `
        -Severity "ERROR" `
        -Category "MissingCritical" `
        -Path "Docs\VerticalSlice_Day1_*.md" `
        -Message "No Day1 vertical slice docs found." `
        -Advice "Restore at least one Day1 readiness/audit doc before delivery."
}
else {
    foreach ($Doc in $Day1Docs) {
        $RelativeDoc = $Doc.FullName.Substring($ProjectRoot.Length + 1)
        Test-Day1File -RelativePath $RelativeDoc -Label "Day1 vertical slice doc"
    }
}

$RequiredDirectories = @(
    @{ Path = "Content\DeepWaterStation"; Label = "DeepWaterStation Day1 content" },
    @{ Path = "Content\SoundsOfHorror"; Label = "SoundsOfHorror Day1 audio content" },
    @{ Path = "Content\Stone_Golem"; Label = "Stone Golem Day1 content" },
    @{ Path = "Content\Bodycam_VHS_Effect"; Label = "Bodycam VHS Day1 content" }
)

foreach ($Spec in $RequiredDirectories) {
    Test-Day1Directory -RelativePath $Spec.Path -Label $Spec.Label
}

Test-Day1DocsForSavedPython

Write-Host "Git LFS coverage:" -ForegroundColor Cyan
Write-Host ("  Unreal binary assets checked (.uasset/.umap): {0}" -f $script:GitLfsCoverageChecked)
Write-Host ("  Covered by LFS attributes: {0}" -f $script:GitLfsCoverageCovered) -ForegroundColor Green
if ($script:GitLfsCoverageSkippedMissing -gt 0) {
    Write-Host ("  Missing binary paths skipped for LFS attribute lookup: {0}" -f $script:GitLfsCoverageSkippedMissing) -ForegroundColor Yellow
}
if ($script:GitLfsCoverageMissing -gt 0) {
    Write-Host ("  Missing expected LFS attributes: {0}" -f $script:GitLfsCoverageMissing) -ForegroundColor Red
    Write-Host "  Expected attributes: filter=lfs, diff=lfs, merge=lfs, text unset (-text)." -ForegroundColor Red
}
else {
    Write-Host "  Expected attributes present for every checked Day1 .uasset/.umap file." -ForegroundColor Green
}
Write-Host ""

$AllResults = @()
foreach ($Warning in $Warnings) {
    $AllResults += $Warning
}
foreach ($Issue in $Issues) {
    $AllResults += $Issue
}
$SummaryRows = @($AllResults | Group-Object Category | Sort-Object Name | ForEach-Object {
    [PSCustomObject]@{
        Category = $_.Name
        Count = $_.Count
    }
})

if ($SummaryRows.Count -gt 0) {
    Write-Host "Summary by category:" -ForegroundColor Cyan
    $SummaryRows | Format-Table Category, Count -AutoSize
}

Write-Day1ResultGroup `
    -Title "Local development warnings allowed by -AllowUntracked:" `
    -Results @($Warnings.ToArray() | Where-Object { $_.Category -eq "LocalUntrackedAllowed" }) `
    -Color Yellow

Write-Day1ResultGroup `
    -Title "Warnings:" `
    -Results @($Warnings.ToArray() | Where-Object { $_.Category -ne "LocalUntrackedAllowed" }) `
    -Color Yellow

if ($Issues.Count -gt 0) {
    Write-Day1ResultGroup `
        -Title "Errors that block the Day1 gate:" `
        -Results ($Issues.ToArray()) `
        -Color Red
    Write-Host "[FAIL] Day1 source control gate found $($Issues.Count) error(s) and $($Warnings.Count) warning(s)." -ForegroundColor Red
    if ((@($Issues | Where-Object { $_.Category -eq "StrictUntrackedFailure" }).Count -gt 0) -and -not $AllowUntracked) {
        Write-Host "Strict mode failed because required Day1 files exist locally but are not tracked. Use -AllowUntracked only for local development checks." -ForegroundColor Red
    }
    if ($NoExitCode) {
        exit 0
    }
    exit 1
}

Write-Host "[OK] Day1 source control gate passed with $($Warnings.Count) warning(s)." -ForegroundColor Green
exit 0
