# Blueprint Interaction Templates

## Overview
Standard configurations and templates for all interactive object types. Use these as starting points for consistent implementation.

## Door Templates

### Template 1: Standard Wooden Door
**Use Case:** Interior residential doors, bedroom doors, office doors
```
Class: BP_Door_Wooden
OpenAngle: 90.0
OpenSpeed: 2.0
bAutoClose: false
bStartLocked: false
OpenSound: SFX_Door_Wood_Open
CloseSound: SFX_Door_Wood_Close
LockedSound: SFX_Door_Locked
Mesh: SM_Door_Panel_Wood
Material: M_Wood_Door_Inst
```

### Template 2: Heavy Metal Door
**Use Case:** Security doors, vault doors, industrial doors
```
Class: BP_Door_Metal_Heavy
OpenAngle: 90.0
OpenSpeed: 1.0
bAutoClose: false
bStartLocked: false
OpenSound: SFX_Door_Metal_Heavy_Open
CloseSound: SFX_Door_Metal_Heavy_Close
LockedSound: SFX_Door_Metal_Locked
OpenCurve: Curve_DoorOpen_Heavy
Mesh: SM_Door_Panel_Metal_Heavy
Material: M_Metal_Door_Industrial
```

### Template 3: Auto-Closing Security Door
**Use Case:** Hospital doors, lab doors, automatic doors
```
Class: BP_Door_Security
OpenAngle: 90.0
OpenSpeed: 2.5
bAutoClose: true
AutoCloseDelay: 2.0
bStartLocked: false
OpenSound: SFX_Door_Security_Open
CloseSound: SFX_Door_Security_Close
Mesh: SM_Door_Panel_Security
Material: M_Metal_Door_Clean
```

## Pickup Templates

### Template 1: Evidence Item
**Use Case:** Photos, notes, physical evidence
```
Class: BP_Pickup_Evidence
PickupType: Evidence
ItemId: "Evidence_[Type]_[Number]"
EvidenceMetadata:
  EvidenceId: "Evidence_[Type]_[Number]"
  EvidenceName: "[Display Name]"
  Description: "[Evidence Description]"
  Category: "Physical Evidence"
  bIsOptional: false
bRotateInPlace: true
RotationSpeed: 90.0
bFloatInPlace: true
FloatAmplitude: 10.0
FloatSpeed: 2.0
PickupSound: SFX_Pickup_Evidence
bDestroyOnPickup: true
Mesh: SM_Evidence_[Type]
Material: M_Evidence_Glow
```

### Template 2: Key Item
**Use Case:** Door keys, access cards, key items
```
Class: BP_Pickup_Key
PickupType: Key
ItemId: "Key_ion]"
InteractionPrompt: "Pick Up [Key Name]"
bRotateInPlace: true
RotationSpeed: 120.0
bFloatInPlace: true
FloatAmplitude: 8.0
FloatSpeed: 2.0
PickupSound: SFX_Pickup_Key
bDestroyOnPickup: true
Mesh: SM_Key_[Type]
Material: M_Metal_Key_Inst
```

### Template 3: Tool Item
**Use Case:** Flashlight, crowbar, usable tools
```
Class: BP_Pickup_Tool
PickupType: Tool
ItemId: "Tool_[Name]"
InteractionPrompt: "Pick Up [Tool Name]"
bRotateInPlace: false
bFloatInPlace: true
FloatAmplitude: 5.0
FloatSpeed: 1.5
PickupSound: SFX_Pickup_Tool
bDestroyOnPickup: true
Mesh: SM_Tool_[Type]
Material: M_Tool_Metal_Inst
```

## Examinable Templates

### Template 1: Small Artifact
**Use Case:** Jewelry, small objects, collectibles
```
Class: BP_Examinable_Artifact
ExamineDistance: 60.0
MinZoomDistance: 30.0
MaxZoomDistance: 120.0
ZoomSpeed: 8.0
RotationSpeed: 80.0
bAllowRotation: true
bAllowZoom: true
bDisablePlayerMovement: true
bHideOriginalMesh: true
ExamineStartSound: SFX_Examine_Pickup
ExamineEndSound: SFX_Examine_Putdown
Mesh: SM_Artifact_[Type]
Material: M_[Material]_HighDetail
```

### Template 2: Medium Object
**Use Case:** Statues, boxes, standard objects
```
Class: BP_Examinable_Standard
ExamineDistance: 100.0
MinZoomDistance: 50.0
MaxZoomDistance: 200.0
ZoomSpeed: 10.0
RotationSpeed: 100.0
bAllowRotation: true
bAllowZoom: true
bDisablePlayerMovement: true
bHideOriginalMesh: true
ExamineStartSound: SFX_Examine_Start
ExamineEndSound: SFX_Examine_End
Mesh: SM_[Object]
Material: M_[Material]_Detailed
```

### Template 3: Flat Photo/Document
**Use Case:** Photos, paintings, flat items
```
Class: BP_Examinable_Photo
ExamineDistance: 80.0
MinZoomDistance: 50.0
MaxZoomDistance: 150.0
ZoomSpeed: 8.0
RotationSpeed: 0.0
bAllowRotation: false
bAllowZoom: true
bDisablePlayerMovement: true
bHideOriginalMesh: true
ExamineStartSound: SFX_Examine_Pickup
ExamineEndSound: SFX_Examine_Putdown
Mesh: SM_Photo_[Type]
Material: M_Photo_[Type]
```

## Document Templates

### Template 1: Text Note
**Use Case:** Handwritten notes, letters, short documents
```
Class: BP_Document_Note
DocumentType: Text
DocumentTitle: "[Note Title]"
DocumentContent: "[Note Content]"
bIsEvidence: true
EvidenceMetadata:
  EvidenceId: "Evidence_Note_[Number]"
  EvidenceName: "[Note Name]"
  Category: "Documents"
  bIsOptional: false
bAutoCollectAsEvidence: true
bDisablePlayerMovement: true
bPauseGame: false
OpenSound: SFX_Paper_Rustle
CloseSound: SFX_Paper_Fold
Mesh: SM_Paper_[Type]
Material: M_Paper_Old
```

### Template 2: Photo Evidence
**Use Case:** Crime scene photos, evidence photos
```
Class: BP_Document_Photo
DocumentType: Image
DocumentTitle: "[Photo Title]"
DocumentContent: "[Brief Caption]"
DocumentImage: T_Photo_[Name]
bIsEvidence: true
EvidenceMetadata:
  EvidenceId: "Evidence_Photo_[Number]"
  EvidenceName: "[Photo Name]"
  Category: "Visual Evidence"
  bIsOptional: false
bAutoCollectAsEvidence: true
bDisablePlayerMovement: true
bPauseGame: false
OpenSound: SFX_Photo_Pickup
CloseSound: SFX_Photo_Putdown
Mesh: SM_Photo_Polaroid
Material: M_Photo_Glossy
```

### Template 3: Story Document
**Use Case:** Critical story documents, reports
```
Class: BP_Document_Story
DocumentType: Mixed
DocumentTitle: "[Document Title]"
DocumentContent: "[Long-form Content]"
DocumentImage: T_Document_[Name]
bIsEvidence: true
EvidenceMetadata:
  EvidenceId: "Evidence_Document_[Number]"
  EvidenceName: "[Document Name]"
  Category: "Critical Evidence"
  bIsOptional: false
bAutoCollectAsEvidence: true
bDisablePlayerMovement: true
bPauseGame: true
OpenSound: SFX_Paper_Rustle
CloseSound: SFX_Paper_Fold
Mesh: SM_Document_[Type]
Material: M_Paper_Official
```

## Recorder Templates

### Template 1: Tape Recorder
**Use Case:** Victim messages, audio logs, recordings
```
Class: BP_Recorder_Tape
AudioLog: SW_AudioLog_[Name]
AudioLogTitle: "[Audio Log Title]"
AudioLogTranscript: "[Full Transcript]"
bUse3DAudio: true
MaxAudibleDistance: 1000.0
bIsEvidence: true
EvidenceMetadata:
  EvidenceId: "Evidence_AudioLog_[Number]"
  EvidenceName: "[Audio Log Name]"
  Category: "Audio Evidence"
  bIsOptional: false
bAutoPlayOnInteract: true
bAutoCollectAsEvidence: true
bStopOnReInteract: true
bLoopAudio: false
Mesh: SM_Recorder_Cassette
Material: M_Plastic_Recorder
```

### Template 2: Answering Machine
**Use Case:** Phone messages, looping messages
```
Class: BP_Recorder_AnsweringMachine
AudioLog: SW_AudioLog_[Name]
AudioLogTitle: "[Message Title]"
AudioLogTranscript: "[Message Transcript]"
bUse3DAudio: true
MaxAudibleDistance: 800.0
bIsEvidence: false
bAutoPlayOnInteract: true
bAutoCollectAsEvidence: false
bStopOnReInteract: true
bLoopAudio: true
Mesh: SM_AnsweringMachine
Material: M_Plastic_Beige
```

### Template 3: Digital Recorder
**Use Case:** Professional recordings, interviews
```
Class: BP_Recorder_Digital
AudioLog: SW_AudioLog_[Name]
AudioLogTitle: "[Recording Title]"
AudioLogTranscript: "[Full Transcript]"
bUse3DAudio: false
MaxAudibleDistance: 0.0
bIsEvidence: true
EvidenceMetadata:
  EvidenceId: "Evidence_Interview_[Number]"
  EvidenceName: "[Interview Name]"
  Category: "Official Records"
  bIsOptional: false
bAutoPlayOnInteract: true
bAutoCollectAsEvidence: true
bStopOnReInteract: true
bLoopAudio: false
Mesh: SM_Recorder_Digital
Material: M_Plastic_Modern
```

## Switch Templates

### Template 1: Light Switch
**Use Case:** Room lights, ceiling lights, lamps
```
Class: BP_Switch_Light
SwitchType: Toggle
bStartOn: false
ConnectedActors: [BP_Light_[Name]]
ActivationFunctionName: "OnSwitchActivated"
OnRotation: (0, 0, 45)
OffRotation: (0, 0, -45)
AnimationSpeed: 5.0
ActivateSound: SFX_Switch_On
DeactivateSound: SFX_Switch_Off
Mesh Base: SM_Switch_Plate_Wall
Mesh Handle: SM_Switch_Toggle
Material: M_Plastic_Switch
```

### Template 2: Door Button
**Use Case:** Security doors, automatic doors
```
Class: BP_Switch_DoorButton
SwitchType: Momentary
MomentaryDuration: 3.0
ConnectedActors: [BP_Door_[Name]]
ActivationFunctionName: "OnSwitchActivated"
OnRotation: (0, 0, -5)
OffRotation: (0, 0, 0)
AnimationSpeed: 10.0
ActivateSound: SFX_Button_Press
DeactivateSound: SFX_Button_Release
Mesh Base: SM_Button_Housing
Mesh Handle: SM_Button_Cap
Material: M_Plastic_Button
```

### Template 3: Emergency Lever
**Use Case:** Alarms, one-time triggers, story events
```
Class: BP_Switch_Emergency
SwitchType: OneTime
bStartOn: false
ConnectedActors: [BP_AlarmSystem, BP_EmergencyLights]
ActivationFunctionName: "OnEmergencyActivated"
OnRotation: (45, 0, 0)
OffRotation: (0, 0, 0)
AnimationSpeed: 3.0
ActivateSound: SFX_Lever_Pull_Heavy
Mesh Base: SM_Lever_Base
Mesh Handle: SM_Lever_Handle
Material: M_Metal_Industrial
```

## Naming Conventions

### Blueprint Naming
```
BP_[Type]_[Variant]_[Number]

Examples:
- BP_Door_Wooden_01
- BP_Pickup_Key_Basement
- BP_Examinable_Statue_Ancient
- BP_Document_Letter_Victim
- BP_Recorder_Tape_Evidence
- BP_Switch_Light_Ceiling
```

### Asset Naming
```
Static Meshes: SM_[Type]_[Variant]
Materials: M_[Type]_[Variant]_Inst
Textures: T_[Type]_[Name]
Sounds: SFX_[Type]_[Action]
Audio: SW_AudioLog_[Name]

Examples:
- SM_Door_Panel_Wood
- M_Wood_Door_Inst
- T_Photo_CrimeScene_01
- SFX_Door_Wood_Open
- SW_AudioLog_Victim_01
```

### ID Naming
```
InteractableId: [Type]_[Location]_[Identifier]
ItemId: [Type]_[Name]
EvidenceId: Evidence_[Type]_[Number]

Examples:
- Door_Basement_Main
- Key_Basement
- Evidence_Photo_01
- Tool_Flashlight
- Switch_Basement_Lights
```

## Parameter Ranges

### Common Value Ranges
```
Distances:
- Interaction Range: 100-200 units
- Audio Range: 500-2000 units
- Examine Distance: 50-250 units

Speeds:
- Door Open Speed: 1.0-3.0
- Rotation Speed: 50.0-200.0 deg/s
- Float Speed: 1.0-3.0 cycles/s
- Animation Speed: 2.0-10.0

Angles:
- Door Open Angle: 60.0-120.0 degrees
- Switch Rotation: 30.0-90.0 degrees

Timers:
- Auto Close Delay: 2.0-5.0 seconds
- Momentary Duration: 0.5-3.0 seconds
- Destroy Delay: 0.0-1.0 seconds

Visual:
- Float Amplitude: 5.0-20.0 units
- Glow Intensity: 1.0-5.0
- LED Brightness: 2.0-10.0
```

## Material Parameters

### Standard Material Parameters
```
Evidence Glow:
- EmissiveColor: (0.5, 0.8, 1.0)
- EmissiveIntensity: 1.5
- BaseColor: Item color
- Roughness: 0.5

LED Indicator:
- Off: (0.5, 0.0, 0.0), Intensity: 0.5
- On: (0.0, 1.0, 0.0), Intensity: 3.0
- Warning: (1.0, 0.5, 0.0), Intensity: 2.0

Paper/Document:
- BaseColor: Aged paper texture
- Roughness: 0.7
- Normal: Paper fiber detail

Metal:
- BaseColor: Metal texture
- Metallic: 0.9
- Roughness: 0.3-0.6
```

## Quick Reference Table

| Type | Interaction Range | Audio Range | Typical Mesh Size | Save State |
|------|------------------|-------------|-------------------|------------|
| Door | 150 units | 1000 units | 200x100x300 | Yes |
| Pickup | 100 units | 500 units | 10x10x10 | Yes (collected) |
| Examinable | 120 units | 500 units | 20x20x20 | Yes (examined) |
| Document | 100 units | 500 units | 30x20x1 | Yes (read) |
| Recorder | 120 units | 1000 units | 30x20x15 | Yes (listened) |
| Switch | 100 units | 500 units | 15x10x5 | Yes (state) |

## Best Practices

### Performance
- Use LODs for all meshes
- Optimize collision volumes (simple shapes)
- Disable tick when not needed
- Use object pooling for common pickups
- Limit particle effects

### Consistency
- Follow naming conventions strictly
- Use standard parameter ranges
- Maintain visual style consistency
- Reuse materials and meshes
- Document custom variations

### Testing
- Test all interaction states
- Verify save/load functionality
- Check audio attenuation
- Validate evidence collection
- Test edge cases

### Accessibility
- Provide clear interaction prompts
- Include audio transcripts
- Support colorblind modes
- Adjustable interaction ranges
- Subtitle support for audio logs
