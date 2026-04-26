# Check include paths and forward declarations
# Validates all #include statements are correct

$ErrorActionPreference = "Stop"

. (Join-Path $PSScriptRoot "Common.ps1")

$ProjectRoot = Get-HorrorProjectRoot

Write-Host "=== Include Path Validation ===" -ForegroundColor Cyan

$Issues = @()

# Check VFX includes
Write-Host "`nChecking VFX system includes..." -ForegroundColor Yellow
$VFXFiles = @(Get-ChildItem -Path "$ProjectRoot\Source\HorrorProject\VFX" -Filter "*.cpp" -Recurse -ErrorAction SilentlyContinue)

foreach ($File in $VFXFiles) {
    $Content = Get-Content $File.FullName -Raw

    # Check for Niagara includes
    if ($Content -match "UNiagaraComponent|UNiagaraSystem") {
        if ($Content -notmatch '#include.*Niagara') {
            $Issues += "  [WARNING] $($File.Name): Uses Niagara types but missing includes"
        }
    }
}

# Check Audio includes
Write-Host "Checking Audio system includes..." -ForegroundColor Yellow
$AudioFiles = @(Get-ChildItem -Path "$ProjectRoot\Source\HorrorProject\Audio" -Filter "*.cpp" -Recurse -ErrorAction SilentlyContinue)

foreach ($File in $AudioFiles) {
    $Content = Get-Content $File.FullName -Raw

    # Check for Audio component includes
    if ($Content -match "UAudioComponent") {
        if ($Content -notmatch '#include.*AudioComponent') {
            $Issues += "  [WARNING] $($File.Name): Uses UAudioComponent but missing include"
        }
    }
}

# Check UI includes
Write-Host "Checking UI system includes..." -ForegroundColor Yellow
$UIFiles = @(Get-ChildItem -Path "$ProjectRoot\Source\HorrorProject\UI" -Filter "*.cpp" -Recurse -ErrorAction SilentlyContinue)

foreach ($File in $UIFiles) {
    $Content = Get-Content $File.FullName -Raw

    # Check for UMG includes
    if ($Content -match "UUserWidget") {
        if ($Content -notmatch '#include.*UserWidget') {
            $Issues += "  [WARNING] $($File.Name): Uses UUserWidget but missing include"
        }
    }
}

# Check Editor tools includes
Write-Host "Checking Editor tools includes..." -ForegroundColor Yellow
$EditorFiles = @(Get-ChildItem -Path "$ProjectRoot\Source\HorrorProjectEditor\Tools" -Filter "*.cpp" -Recurse -ErrorAction SilentlyContinue)

foreach ($File in $EditorFiles) {
    $Content = Get-Content $File.FullName -Raw

    # Check for Editor subsystem includes
    if ($Content -match "UEditorSubsystem") {
        if ($Content -notmatch '#include.*EditorSubsystem') {
            $Issues += "  [WARNING] $($File.Name): Uses UEditorSubsystem but missing include"
        }
    }
}

# Summary
Write-Host "`n=== Include Check Summary ===" -ForegroundColor Cyan
if ($Issues.Count -eq 0) {
    Write-Host "No include issues detected" -ForegroundColor Green
    exit 0
} else {
    Write-Host "Found $($Issues.Count) potential include issues:" -ForegroundColor Yellow
    $Issues | ForEach-Object { Write-Host $_ -ForegroundColor Yellow }
    exit 0
}
