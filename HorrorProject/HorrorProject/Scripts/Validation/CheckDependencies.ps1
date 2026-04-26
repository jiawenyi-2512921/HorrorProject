# Check module dependencies for HorrorProject
# Validates Build.cs files have all required modules

$ErrorActionPreference = "Stop"
$ProjectRoot = "D:\gptzuo\HorrorProject\HorrorProject"

Write-Host "=== Dependency Check ===" -ForegroundColor Cyan

# Required modules for different systems
$RequiredModules = @{
    "VFX" = @("Niagara")
    "Audio" = @("AudioMixer", "AudioExtensions")
    "UI" = @("UMG", "SlateCore", "Slate")
    "Editor" = @("UnrealEd", "EditorSubsystem", "AssetTools")
}

# Check HorrorProject.Build.cs
$BuildFile = Join-Path $ProjectRoot "Source\HorrorProject\HorrorProject.Build.cs"
$BuildContent = Get-Content $BuildFile -Raw

Write-Host "`nChecking HorrorProject.Build.cs..." -ForegroundColor Yellow

$MissingModules = @()

# Check for Niagara (required by ParticleSpawner)
if ($BuildContent -notmatch "Niagara") {
    $MissingModules += "Niagara"
    Write-Host "  [MISSING] Niagara module (required by VFX/ParticleSpawner)" -ForegroundColor Red
}

# Check for Audio modules
if ($BuildContent -notmatch "AudioMixer") {
    Write-Host "  [INFO] AudioMixer module not found (optional for advanced audio)" -ForegroundColor Yellow
}

# Check for UMG (should be present)
if ($BuildContent -match "UMG") {
    Write-Host "  [OK] UMG module found" -ForegroundColor Green
} else {
    Write-Host "  [ERROR] UMG module missing" -ForegroundColor Red
    $MissingModules += "UMG"
}

# Check HorrorProjectEditor.Build.cs
$EditorBuildFile = Join-Path $ProjectRoot "Source\HorrorProjectEditor\HorrorProjectEditor.Build.cs"
$EditorBuildContent = Get-Content $EditorBuildFile -Raw

Write-Host "`nChecking HorrorProjectEditor.Build.cs..." -ForegroundColor Yellow

if ($EditorBuildContent -match "UnrealEd") {
    Write-Host "  [OK] UnrealEd module found" -ForegroundColor Green
} else {
    Write-Host "  [ERROR] UnrealEd module missing" -ForegroundColor Red
}

# Summary
Write-Host "`n=== Dependency Check Summary ===" -ForegroundColor Cyan
if ($MissingModules.Count -eq 0) {
    Write-Host "All critical modules present" -ForegroundColor Green
    exit 0
} else {
    Write-Host "Missing modules: $($MissingModules -join ', ')" -ForegroundColor Red
    Write-Host "`nTo fix, add these to PublicDependencyModuleNames in Build.cs" -ForegroundColor Yellow
    exit 1
}
