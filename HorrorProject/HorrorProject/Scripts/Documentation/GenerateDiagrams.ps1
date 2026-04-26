# GenerateDiagrams.ps1 - Architecture Diagram Generator
# Generates PlantUML diagrams for system architecture

param(
    [string]$OutputPath = "$PSScriptRoot\..\..\Docs\Architecture\Diagrams",
    [string]$SourcePath = "$PSScriptRoot\..\..\Source\HorrorProject",
    [switch]$GeneratePNG
)

$ErrorActionPreference = "Stop"

Write-Host "=== Architecture Diagram Generator ===" -ForegroundColor Cyan

New-Item -ItemType Directory -Path $OutputPath -Force | Out-Null

# System Component Diagram
$componentDiagram = @"
@startuml HorrorProject_Components
!theme plain
skinparam componentStyle rectangle

package "Core Systems" {
    [Event Bus Subsystem] as EventBus
    [Audio Subsystem] as Audio
    [Save System] as Save
}

package "Game Logic" {
    [Encounter Director] as Encounter
    [Anomaly Director] as Anomaly
    [Game Mode] as GameMode
}

package "Player Systems" {
    [Player Character] as Player
    [Player Controller] as Controller
    [Camera Manager] as Camera
}

package "AI Systems" {
    [Threat AI Controller] as ThreatAI
    [Threat Character] as Threat
    [Golem Behavior] as Golem
}

package "Evidence System" {
    [Archive Subsystem] as Archive
    [Evidence Collection] as Evidence
    [Evidence Actor] as EvidenceActor
}

package "Interaction" {
    [Interaction Component] as Interaction
    [Interactable Objects] as Interactables
}

package "UI" {
    [HUD Widget] as HUD
    [Archive Viewer] as ArchiveUI
    [Objective Widget] as Objective
}

' Relationships
GameMode --> EventBus
Encounter --> EventBus
Encounter --> Audio
Encounter --> Threat
Player --> Controller
Controller --> Camera
Controller --> Interaction
ThreatAI --> Threat
Golem --> Threat
Evidence --> Archive
Evidence --> EventBus
EvidenceActor --> Evidence
Interaction --> Interactables
HUD --> EventBus
ArchiveUI --> Archive
Objective --> EventBus

@enduml
"@

Set-Content -Path (Join-Path $OutputPath "ComponentDiagram.puml") -Value $componentDiagram

# Event Bus Flow Diagram
$eventBusFlow = @"
@startuml EventBus_Flow
!theme plain

participant "Game System" as System
participant "Event Bus" as Bus
participant "Listener A" as ListenerA
participant "Listener B" as ListenerB

System -> Bus: Publish(EventTag, SourceId, StateTag)
activate Bus

Bus -> Bus: Create FHorrorEventMessage
Bus -> Bus: Add to History
Bus -> Bus: Lookup Objective Metadata

Bus -> ListenerA: OnEventPublished(Message)
activate ListenerA
ListenerA --> Bus:
deactivate ListenerA

Bus -> ListenerB: OnEventPublished(Message)
activate ListenerB
ListenerB --> Bus:
deactivate ListenerB

Bus --> System: Success
deactivate Bus

@enduml
"@

Set-Content -Path (Join-Path $OutputPath "EventBusFlow.puml") -Value $eventBusFlow

# Encounter State Machine
$encounterStateMachine = @"
@startuml Encounter_StateMachine
!theme plain

[*] --> Dormant

Dormant --> Primed : PrimeEncounter()
Primed --> Revealed : TriggerReveal()
Revealed --> Resolved : ResolveEncounter()
Resolved --> Dormant : ResetEncounter()

Primed --> Dormant : ResetEncounter()
Revealed --> Dormant : ResetEncounter()

state Dormant {
    [*] --> Idle
    Idle : No active encounter
}

state Primed {
    [*] --> Ready
    Ready : Waiting for trigger
    Ready : Route may be gated
}

state Revealed {
    [*] --> Playing
    Playing : Threat spawned
    Playing : Audio/VFX active
    Playing : Events published
}

state Resolved {
    [*] --> Complete
    Complete : Encounter finished
    Complete : Route ungated
}

@enduml
"@

Set-Content -Path (Join-Path $OutputPath "EncounterStateMachine.puml") -Value $encounterStateMachine

# Class Hierarchy
$classHierarchy = @"
@startuml Class_Hierarchy
!theme plain

class UWorldSubsystem
class AActor
class ACharacter
class APlayerController
class AAIController
class UActorComponent

UWorldSubsystem <|-- UHorrorEventBusSubsystem
UWorldSubsystem <|-- UHorrorAudioSubsystem
UWorldSubsystem <|-- UArchiveSubsystem

AActor <|-- AHorrorEncounterDirector
AActor <|-- AHorrorAnomalyDirector
AActor <|-- AEvidenceActor
AActor <|-- AHorrorAudioZoneActor

ACharacter <|-- AHorrorProjectCharacter
ACharacter <|-- AHorrorThreatCharacter

APlayerController <|-- AHorrorProjectPlayerController

AAIController <|-- AHorrorThreatAIController

UActorComponent <|-- UHorrorGolemBehaviorComponent
UActorComponent <|-- UEvidenceCollectionComponent
UActorComponent <|-- UHorrorInteractionComponent

class UHorrorEventBusSubsystem {
    +Publish()
    +GetHistory()
    +RegisterObjectiveMetadata()
}

class AHorrorEncounterDirector {
    +PrimeEncounter()
    +TriggerReveal()
    +ResolveEncounter()
    +SpawnThreatActor()
}

class AHorrorThreatCharacter {
    +SetThreatState()
    +GetThreatState()
}

class UArchiveSubsystem {
    +AddEvidence()
    +GetAllEvidence()
    +SaveArchive()
}

@enduml
"@

Set-Content -Path (Join-Path $OutputPath "ClassHierarchy.puml") -Value $classHierarchy

# Data Flow Diagram
$dataFlow = @"
@startuml Data_Flow
!theme plain

database "Save Game" as SaveDB
database "Event History" as EventDB

rectangle "Player Input" as Input
rectangle "Game Logic" as Logic
rectangle "Event Bus" as EventBus
rectangle "Audio System" as Audio
rectangle "UI System" as UI

Input --> Logic : Player Actions
Logic --> EventBus : Publish Events
EventBus --> EventDB : Store History
EventBus --> Audio : Trigger Sounds
EventBus --> UI : Update Display
Logic --> SaveDB : Save Progress
SaveDB --> Logic : Load Progress

@enduml
"@

Set-Content -Path (Join-Path $OutputPath "DataFlow.puml") -Value $dataFlow

Write-Host "Generated PlantUML diagrams:" -ForegroundColor Green
Write-Host "  - ComponentDiagram.puml" -ForegroundColor Gray
Write-Host "  - EventBusFlow.puml" -ForegroundColor Gray
Write-Host "  - EncounterStateMachine.puml" -ForegroundColor Gray
Write-Host "  - ClassHierarchy.puml" -ForegroundColor Gray
Write-Host "  - DataFlow.puml" -ForegroundColor Gray

# Check for PlantUML
$plantUMLJar = Get-ChildItem -Path $env:USERPROFILE -Filter "plantuml*.jar" -Recurse -ErrorAction SilentlyContinue | Select-Object -First 1

if ($GeneratePNG -and $plantUMLJar) {
    Write-Host "`nGenerating PNG images..." -ForegroundColor Cyan
    $pumlFiles = Get-ChildItem -Path $OutputPath -Filter "*.puml"
    foreach ($file in $pumlFiles) {
        java -jar $plantUMLJar.FullName $file.FullName
        Write-Host "  Generated: $($file.BaseName).png" -ForegroundColor Green
    }
} elseif ($GeneratePNG) {
    Write-Warning "PlantUML JAR not found. Install from https://plantuml.com/download"
    Write-Host "You can view .puml files at: https://www.plantuml.com/plantuml/uml/" -ForegroundColor Yellow
}

Write-Host "`n=== Diagram Generation Complete ===" -ForegroundColor Cyan
