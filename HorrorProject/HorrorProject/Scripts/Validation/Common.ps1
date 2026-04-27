# Shared helpers for HorrorProject validation scripts.

function Get-HorrorProjectRoot {
    param(
        [string]$StartPath = $PSScriptRoot
    )

    $CurrentPath = (Resolve-Path -LiteralPath $StartPath).Path
    if ((Get-Item -LiteralPath $CurrentPath) -is [System.IO.FileInfo]) {
        $CurrentPath = Split-Path -Parent $CurrentPath
    }

    while ($CurrentPath) {
        if (Test-Path -LiteralPath (Join-Path $CurrentPath "HorrorProject.uproject")) {
            return $CurrentPath
        }

        $ParentPath = Split-Path -Parent $CurrentPath
        if ($ParentPath -eq $CurrentPath) {
            break
        }
        $CurrentPath = $ParentPath
    }

    if ($env:HORRORPROJECT_ROOT -and (Test-Path -LiteralPath (Join-Path $env:HORRORPROJECT_ROOT "HorrorProject.uproject"))) {
        return (Resolve-Path -LiteralPath $env:HORRORPROJECT_ROOT).Path
    }

    throw "Unable to locate HorrorProject.uproject from '$StartPath'. Set HORRORPROJECT_ROOT to the project directory."
}

function Get-HorrorProjectFile {
    param(
        [Parameter(Mandatory = $true)]
        [string]$ProjectRoot
    )

    $ProjectFile = Join-Path $ProjectRoot "HorrorProject.uproject"
    if (-not (Test-Path -LiteralPath $ProjectFile)) {
        throw "Project file not found: $ProjectFile"
    }

    return $ProjectFile
}

function Get-HorrorUERoot {
    $Candidates = @(
        $env:UE5_ROOT,
        $env:UE_5_6_ROOT,
        $env:UNREAL_ENGINE_ROOT,
        "D:\UnrealEngine\UE_5.6",
        "C:\Program Files\Epic Games\UE_5.6"
    ) | Where-Object { -not [string]::IsNullOrWhiteSpace($_) }

    foreach ($Candidate in $Candidates) {
        $BuildScript = Join-Path $Candidate "Engine\Build\BatchFiles\Build.bat"
        if (Test-Path -LiteralPath $BuildScript) {
            return (Resolve-Path -LiteralPath $Candidate).Path
        }
    }

    throw "Unreal Engine 5.6 root not found. Set UE5_ROOT or UE_5_6_ROOT to your engine directory."
}

function Get-HorrorBuildScript {
    param(
        [Parameter(Mandatory = $true)]
        [string]$UERoot,

        [string]$ScriptName = "Build.bat"
    )

    $ScriptPath = Join-Path $UERoot "Engine\Build\BatchFiles\$ScriptName"
    if (-not (Test-Path -LiteralPath $ScriptPath)) {
        throw "Unreal build script not found: $ScriptPath"
    }

    return $ScriptPath
}

function Get-HorrorEditorCmd {
    param(
        [Parameter(Mandatory = $true)]
        [string]$UERoot
    )

    $EditorPath = Join-Path $UERoot "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
    if (-not (Test-Path -LiteralPath $EditorPath)) {
        throw "UnrealEditor-Cmd.exe not found: $EditorPath"
    }

    return $EditorPath
}
