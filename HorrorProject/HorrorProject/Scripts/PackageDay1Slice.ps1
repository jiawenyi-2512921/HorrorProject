# HorrorProject Day1 vertical slice packager.

param(
    [string]$Configuration = "Development",
    [string]$ArchiveDirectory = "",
    [switch]$SkipBuild = $false,
    [switch]$ValidateExisting = $false,
    [switch]$Smoke = $false,
    [switch]$SourceControlGate = $false,
    [switch]$AllowUntrackedSourceControlGate = $false
)

$ErrorActionPreference = "Stop"

$ValidationCommon = Join-Path $PSScriptRoot "Validation\Common.ps1"
. $ValidationCommon

$ProjectRoot = Get-HorrorProjectRoot -StartPath $PSScriptRoot
$ProjectFile = Get-HorrorProjectFile -ProjectRoot $ProjectRoot
$UE5Root = Get-HorrorUERoot
$UAT = Get-HorrorBuildScript -UERoot $UE5Root -ScriptName "RunUAT.bat"
$Day1Map = "/Game/DeepWaterStation/Maps/DemoMap_VerticalSlice_Day1"
$Day1MapFile = "Content\DeepWaterStation\Maps\DemoMap_VerticalSlice_Day1.umap"
$RequiredDay1Assets = @(
    $Day1MapFile,
    "Content\Horror\BP_HorrorGameModeBase.uasset",
    "Content\Bodycam_VHS_Effect\Materials\Instances\PostProcess\MI_OldVHS.uasset",
    "Content\Bodycam_VHS_Effect\Sounds\S_CamZoomOut.uasset",
    "Content\SoundsOfHorror\Clues\CUE\CUE_SOH_Clue_01.uasset",
    "Content\SoundsOfHorror\Clues\CUE\CUE_SOH_Clue_02.uasset",
    "Content\SoundsOfHorror\BuildUps\CUE\CUE_SOH_BU_01.uasset",
    "Content\SoundsOfHorror\Impacts\CUE\CUE_SOH_IP_01.uasset",
    "Content\SoundsOfHorror\Impacts\CUE\CUE_SOH_IP_02.uasset",
    "Content\SoundsOfHorror\Puzzles\CUE\CUE_SOH_PZ_01.uasset",
    "Content\SoundsOfHorror\Tension\CUE\CUE_SOH_TS_01.uasset",
    "Content\SoundsOfHorror\Tension\CUE\CUE_SOH_TS_02.uasset",
    "Content\SoundsOfHorror\Jumpscares\CUE\CUE_SOH_JS_01.uasset",
    "Content\SoundsOfHorror\Atmosphere\CUE\CUE_SOH_ATM_01.uasset",
    "Content\SoundsOfHorror\XMelodies\CUE\CUE_SOH_MD_01.uasset",
    "Content\Stone_Golem\mesh\SKM_Stone_Golem.uasset"
)
$RequiredCookDirectories = @(
    "/Game/DeepWaterStation",
    "/Game/Horror",
    "/Game/SoundsOfHorror",
    "/Game/Bodycam_VHS_Effect/Sounds",
    "/Game/Bodycam_VHS_Effect/Materials/Instances/PostProcess",
    "/Game/Stone_Golem"
)

function Test-Day1Readiness {
    param([string]$Root)

    $DefaultGamePath = Join-Path $Root "Config\DefaultGame.ini"
    $DefaultGameText = Get-Content -LiteralPath $DefaultGamePath -Raw

    if ($DefaultGameText -notmatch [regex]::Escape("MapsToCook=(FilePath=`"$Day1Map`")")) {
        throw "DefaultGame.ini does not cook Day1 map: $Day1Map"
    }

    foreach ($CookDirectory in $RequiredCookDirectories) {
        $CookEntry = "DirectoriesToAlwaysCook=(Path=`"$CookDirectory`")"
        if ($DefaultGameText -notmatch [regex]::Escape($CookEntry)) {
            throw "DefaultGame.ini missing Day1 cook directory: $CookDirectory"
        }
    }

    foreach ($RelativeAssetPath in $RequiredDay1Assets) {
        $AssetPath = Join-Path $Root $RelativeAssetPath
        if (-not (Test-Path -LiteralPath $AssetPath)) {
            throw "Day1 required asset missing before cook: $RelativeAssetPath"
        }
    }
}

if ([string]::IsNullOrWhiteSpace($ArchiveDirectory)) {
    $ArchiveDirectory = Join-Path $ProjectRoot "Build\Packages\Win64-Day1"
}

function Get-Day1PackageExe {
    param([string]$ArchiveRoot)

    $ExePath = Join-Path $ArchiveRoot "HorrorProject.exe"
    if (-not (Test-Path -LiteralPath $ExePath)) {
        throw "Packaged executable not found: $ExePath"
    }

    return $ExePath
}

function Test-Day1Package {
    param([string]$ArchiveRoot)

    $ExePath = Get-Day1PackageExe -ArchiveRoot $ArchiveRoot
    $PaksDir = Join-Path $ArchiveRoot "HorrorProject\Content\Paks"
    $RequiredFiles = @(
        $ExePath,
        (Join-Path $PaksDir "HorrorProject-Windows.utoc"),
        (Join-Path $PaksDir "HorrorProject-Windows.ucas"),
        (Join-Path $PaksDir "global.utoc"),
        (Join-Path $PaksDir "global.ucas")
    )

    foreach ($RequiredFile in $RequiredFiles) {
        if (-not (Test-Path -LiteralPath $RequiredFile)) {
            throw "Required package file missing: $RequiredFile"
        }
    }

    return $ExePath
}

Write-Host "=== HorrorProject Day1 Packager ===" -ForegroundColor Cyan
Write-Host "Configuration: $Configuration"
Write-Host "Archive: $ArchiveDirectory"
Write-Host "Map: $Day1Map"
Write-Host ""

if ($SourceControlGate) {
    $SourceControlGateScript = Join-Path $PSScriptRoot "Validation\Day1SourceControlGate.ps1"
    if ($AllowUntrackedSourceControlGate) {
        Write-Host "Day1 source control gate: local development mode (-AllowUntracked)." -ForegroundColor Yellow
        Write-Host "Untracked Day1 files may pass as warnings here, but ignored/missing files and missing Git LFS coverage still fail." -ForegroundColor Yellow

        & $SourceControlGateScript -AllowUntracked
    }
    else {
        Write-Host "Day1 source control gate: strict release/package mode." -ForegroundColor Yellow
        Write-Host "All required Day1 files must be tracked and Day1 .uasset/.umap files must be covered by Git LFS before UAT packaging starts." -ForegroundColor Yellow

        & $SourceControlGateScript
    }
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Day1 source control gate failed." -ForegroundColor Red
        exit $LASTEXITCODE
    }
}

Test-Day1Readiness -Root $ProjectRoot
Write-Host "Day1 readiness checks passed." -ForegroundColor Green

if (-not $ValidateExisting) {
    $BuildArgs = @(
        "BuildCookRun",
        "-project=$ProjectFile",
        "-noP4",
        "-platform=Win64",
        "-clientconfig=$Configuration",
        "-cook",
        "-stage",
        "-pak",
        "-iostore",
        "-map=$Day1Map",
        "-archive",
        "-archivedirectory=$ArchiveDirectory",
        "-utf8output"
    )

    if (-not $SkipBuild) {
        $BuildArgs += "-build"
    }

    & $UAT @BuildArgs
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Day1 package failed." -ForegroundColor Red
        exit $LASTEXITCODE
    }
}

$PackagedExe = Test-Day1Package -ArchiveRoot $ArchiveDirectory
Write-Host "Package files verified." -ForegroundColor Green

if ($Smoke) {
    Write-Host "Running packaged smoke test..." -ForegroundColor Yellow
    $SmokeProcess = Start-Process `
        -FilePath $PackagedExe `
        -ArgumentList @("-NullRHI", "-nosound", "-unattended", "-NoSplash", "-ExecCmds=quit") `
        -Wait `
        -PassThru `
        -WindowStyle Hidden

    if ($SmokeProcess.ExitCode -ne 0) {
        Write-Host "Packaged smoke test failed." -ForegroundColor Red
        exit $SmokeProcess.ExitCode
    }
    Write-Host "Packaged smoke test passed." -ForegroundColor Green
}

Write-Host "Day1 package ready: $ArchiveDirectory" -ForegroundColor Green
