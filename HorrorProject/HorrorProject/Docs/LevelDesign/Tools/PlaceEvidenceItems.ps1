# Place Evidence Items Tool
# Places 28 evidence items across the level

param(
    [Parameter(Mandatory=$false)]
    [string]$LevelName = "SM13",

    [Parameter(Mandatory=$false)]
    [int]$RequiredCount = 8,

    [Parameter(Mandatory=$false)]
    [int]$OptionalCount = 16,

    [Parameter(Mandatory=$false)]
    [int]$SecretCount = 4,

    [Parameter(Mandatory=$false)]
    [string]$OutputPath = "D:/gptzuo/HorrorProject/HorrorProject/Content/Maps"
)

$ErrorActionPreference = "Stop"

Write-Host "=== Place Evidence Items ===" -ForegroundColor Cyan
Write-Host "Level: $LevelName"
Write-Host "Required: $RequiredCount | Optional: $OptionalCount | Secret: $SecretCount"
Write-Host ""

# Evidence definitions
$evidence = @(
    # Required Evidence (8)
    @{ ID=1; Name="Welcome Letter"; Type="Document"; Zone="EntryHall"; Position=@{X=500;Y=200;Z=130}; Category="Required"; Visibility="Tier1" },
    @{ ID=2; Name="Incident Report"; Type="Document"; Zone="MainCorridor"; Position=@{X=1200;Y=100;Z=150}; Category="Required"; Visibility="Tier1" },
    @{ ID=3; Name="Employee Journal"; Type="Document"; Zone="WestWing"; Position=@{X=700;Y=700;Z=130}; Category="Required"; Visibility="Tier1" },
    @{ ID=4; Name="Security Footage"; Type="AudioLog"; Zone="WestWing"; Position=@{X=900;Y=900;Z=130}; Category="Required"; Visibility="Tier2" },
    @{ ID=5; Name="Medical Records"; Type="Document"; Zone="EastWing"; Position=@{X=1700;Y=-700;Z=130}; Category="Required"; Visibility="Tier1" },
    @{ ID=6; Name="Specimen Tag"; Type="Object"; Zone="EastWing"; Position=@{X=1800;Y=-800;Z=130}; Category="Required"; Visibility="Tier1" },
    @{ ID=7; Name="Ritual Instructions"; Type="Document"; Zone="Basement"; Position=@{X=1000;Y=0;Z=-170}; Category="Required"; Visibility="Tier1" },
    @{ ID=8; Name="Final Message"; Type="AudioLog"; Zone="UpperFloor"; Position=@{X=1000;Y=600;Z=430}; Category="Required"; Visibility="Tier2" },

    # Optional Evidence (16)
    @{ ID=9; Name="Staff Photo"; Type="Object"; Zone="WestWing"; Position=@{X=650;Y=750;Z=200}; Category="Optional"; Visibility="Tier2" },
    @{ ID=10; Name="Maintenance Log"; Type="Document"; Zone="WestWing"; Position=@{X=600;Y=650;Z=130}; Category="Optional"; Visibility="Tier2" },
    @{ ID=11; Name="Personal Effects"; Type="Object"; Zone="WestWing"; Position=@{X=750;Y=800;Z=130}; Category="Optional"; Visibility="Tier2" },
    @{ ID=12; Name="Email Printout"; Type="Document"; Zone="WestWing"; Position=@{X=1200;Y=1000;Z=430}; Category="Optional"; Visibility="Tier2" },
    @{ ID=13; Name="Patient Chart"; Type="Document"; Zone="EastWing"; Position=@{X=1600;Y=-700;Z=130}; Category="Optional"; Visibility="Tier2" },
    @{ ID=14; Name="Doctor's Notes"; Type="AudioLog"; Zone="EastWing"; Position=@{X=1750;Y=-750;Z=130}; Category="Optional"; Visibility="Tier2" },
    @{ ID=15; Name="Prescription Bottle"; Type="Object"; Zone="EastWing"; Position=@{X=1650;Y=-650;Z=150}; Category="Optional"; Visibility="Tier2" },
    @{ ID=16; Name="Cult Symbol"; Type="Environmental"; Zone="Basement"; Position=@{X=800;Y=-200;Z=-100}; Category="Optional"; Visibility="Tier2" },
    @{ ID=17; Name="Sacrifice Record"; Type="Document"; Zone="Basement"; Position=@{X=900;Y=200;Z=-170}; Category="Optional"; Visibility="Tier2" },
    @{ ID=18; Name="Architect Plans"; Type="Document"; Zone="UpperFloor"; Position=@{X=950;Y=550;Z=430}; Category="Optional"; Visibility="Tier2" },
    @{ ID=19; Name="Founder's Diary"; Type="Document"; Zone="UpperFloor"; Position=@{X=1200;Y=1000;Z=430}; Category="Optional"; Visibility="Tier2" },
    @{ ID=20; Name="Old Photograph"; Type="Object"; Zone="UpperFloor"; Position=@{X=1050;Y=650;Z=430}; Category="Optional"; Visibility="Tier2" },
    @{ ID=21; Name="Evacuation Notice"; Type="Document"; Zone="MainCorridor"; Position=@{X=1300;Y=50;Z=150}; Category="Optional"; Visibility="Tier2" },
    @{ ID=22; Name="Visitor Log"; Type="Document"; Zone="EntryHall"; Position=@{X=450;Y=150;Z=130}; Category="Optional"; Visibility="Tier2" },
    @{ ID=23; Name="Security Badge"; Type="Object"; Zone="EntryHall"; Position=@{X=550;Y=250;Z=110}; Category="Optional"; Visibility="Tier2" },
    @{ ID=24; Name="Survivor's Note"; Type="Document"; Zone="ExitSequence"; Position=@{X=2100;Y=0;Z=130}; Category="Optional"; Visibility="Tier2" },

    # Secret Evidence (4)
    @{ ID=25; Name="Developer Room Key"; Type="Object"; Zone="WestWing"; Position=@{X=580;Y=720;Z=250}; Category="Secret"; Visibility="Tier3" },
    @{ ID=26; Name="Encrypted File"; Type="Document"; Zone="EastWing"; Position=@{X=1900;Y=-900;Z=130}; Category="Secret"; Visibility="Tier3" },
    @{ ID=27; Name="Ancient Artifact"; Type="Object"; Zone="Basement"; Position=@{X=1100;Y=-100;Z=-250}; Category="Secret"; Visibility="Tier3" },
    @{ ID=28; Name="Time Capsule"; Type="Object"; Zone="UpperFloor"; Position=@{X=1150;Y=700;Z=520}; Category="Secret"; Visibility="Tier3" }
)

Write-Host "Placing evidence items..." -ForegroundColor Yellow

$placementData = @{
    LevelName = $LevelName
    TotalCount = $evidence.Count
    RequiredCount = $RequiredCount
    OptionalCount = $OptionalCount
    SecretCount = $SecretCount
    Evidence = @()
    Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
}

foreach ($item in $evidence) {
    Write-Host "  - Evidence $($item.ID): $($item.Name) [$($item.Type), $($item.Category)]" -ForegroundColor Gray

    $evidenceData = @{
        ID = $item.ID
        Name = $item.Name
        Type = $item.Type
        Zone = $item.Zone
        Position = $item.Position
        Category = $item.Category
        Visibility = $item.Visibility
        BlueprintClass = "BP_Evidence_$($item.Type)"
        TriggerRadius = 150
        HighlightEnabled = $true
    }

    $placementData.Evidence += $evidenceData
}

$outputFile = Join-Path $OutputPath "${LevelName}_Evidence.json"
$placementData | ConvertTo-Json -Depth 10 | Set-Content -Path $outputFile -Encoding UTF8

Write-Host ""
Write-Host "=== Evidence Placement Complete ===" -ForegroundColor Cyan
Write-Host "Total: $($evidence.Count)" -ForegroundColor Green
Write-Host "Required: $RequiredCount | Optional: $OptionalCount | Secret: $SecretCount" -ForegroundColor Green
Write-Host "Saved to: $outputFile" -ForegroundColor Green
