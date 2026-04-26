# Interaction System Blueprint Implementation Guide

## Overview

The Interaction system provides 6 types of interactable objects:
- **ABaseInteractable** - Base class for all interactables
- **ADocumentInteractable** - Readable documents
- **ADoorInteractable** - Doors and hatches
- **AExaminableInteractable** - Examinable objects
- **APickupInteractable** - Pickup items
- **ARecorderInteractable** - Audio recorders
- **ASwitchInteractable** - Switches and buttons

All classes implement `IInteractableInterface` and support Blueprint extension.

## C++ Foundation

### BaseInteractable

**Purpose**: Base class providing common interaction functionality.

**Key Features**:
- IInteractableInterface implementation
- Sound playback support
- Event bus integration
- State management (interacted/not interacted)
- Save/load support
- Line of sight checking
- Multiple interaction support

### Interaction Types

Each specialized interactable adds specific functionality:
- **Document**: Display text content
- **Door**: Open/close with optional locking
- **Examinable**: Rotate and inspect objects
- **Pickup**: Add items to inventory
- **Recorder**: Play audio recordings
- **Switch**: Toggle states, trigger events

## Blueprint Implementation

### Step 1: Create Base Interactable Blueprint

**Create**: `BP_BaseInteractable`

**Parent Class**: BaseInteractable

**Purpose**: Base class for all custom interactables.

**Components**:
- StaticMeshComponent (inherited)
- SphereComponent (name: `InteractionRadius`)
- WidgetComponent (name: `InteractionPromptWidget`)

**Variables**:
- `bShowPromptWhenNearby` (bool, default: true)
- `InteractionDistance` (float, default: 200)
- `bHighlightWhenNearby` (bool, default: true)
- `HighlightColor` (LinearColor, default: Yellow)

**Event Graph**:
```
Event BeginPlay
  → Setup Interaction Prompt
  → Hide Interaction Prompt

On Component Begin Overlap (InteractionRadius)
  → Cast to PlayerCharacter
  → Branch (bShowPromptWhenNearby)
    True:
      → Show Interaction Prompt
      → Update Prompt Text (InteractionPrompt)
  → Branch (bHighlightWhenNearby)
    True:
      → Enable Highlight Effect

On Component End Overlap (InteractionRadius)
  → Hide Interaction Prompt
  → Disable Highlight Effect

On Interact (from IInteractableInterface)
  → Branch (Can Interact)
    True:
      → Play Interaction Sound
      → Publish Interaction Event
      → Call BP_OnInteract (Blueprint event)
    False:
      → Show Cannot Interact Message
```

**Blueprint Events**:
```
BP_OnInteract (BlueprintImplementableEvent)
  - Override in child blueprints for custom behavior
```

### Step 2: Create Document Interactable

**Create**: `BP_DocumentInteractable`

**Parent Class**: DocumentInteractable

**Purpose**: Readable documents (notes, logs, papers).

**Mesh**: Paper/document mesh

**Variables**:
- `DocumentTitle` (Text)
- `DocumentContent` (Text)
- `DocumentTexture` (Texture2D) - Optional image
- `PageCount` (int32, default: 1)
- `bAutoCollectAsEvidence` (bool, default: true)

**Event Graph**:
```
On Interact
  → Parent: On Interact
  → Get UI Manager
  → Show Document Viewer
     - Title: DocumentTitle
     - Content: DocumentContent
     - Texture: DocumentTexture
  → Branch (bAutoCollectAsEvidence)
    True:
      → Create Evidence Metadata
      → Get Evidence Collection Component
      → Collect Evidence
```

**Document Viewer Widget** (create `WBP_DocumentViewer`):

**Widget Hierarchy**:
```
Canvas Panel
├── Image (Background Blur)
└── Border (Document Panel)
    ├── VerticalBox
    │   ├── TextBlock (Document Title)
    │   ├── ScrollBox (Document Content)
    │   │   └── TextBlock (Content Text)
    │   ├── Image (Document Texture - if provided)
    │   └── HorizontalBox (Navigation - if multiple pages)
    │       ├── Button (Previous Page)
    │       ├── TextBlock (Page X/Y)
    │       └── Button (Next Page)
    └── Button (Close)
```

### Step 3: Create Door Interactable

**Create**: `BP_DoorInteractable`

**Parent Class**: DoorInteractable

**Purpose**: Doors, hatches, and other openable barriers.

**Components**:
- StaticMeshComponent (door frame)
- StaticMeshComponent (door panel, name: `DoorPanel`)
- AudioComponent (name: `DoorAudio`)

**Variables**:
- `bIsOpen` (bool, default: false)
- `bIsLocked` (bool, default: false)
- `RequiredKeyId` (FName) - Key required to unlock
- `OpenAngle` (float, default: 90.0)
- `OpenSpeed` (float, default: 2.0)
- `OpenSound` (SoundBase)
- `CloseSound` (SoundBase)
- `LockedSound` (SoundBase)

**Event Graph**:
```
On Interact
  → Branch (bIsLocked)
    True:
      → Check Player Has Key (RequiredKeyId)
      → Branch (Has Key)
        True:
          → Set bIsLocked = false
          → Play Sound (UnlockSound)
          → Open Door
        False:
          → Play Sound (LockedSound)
          → Show Message ("Door is locked")
    False:
      → Branch (bIsOpen)
        True: Close Door
        False: Open Door

Open Door:
  → Set bIsOpen = true
  → Play Sound (OpenSound)
  → Timeline (0 to 1 over OpenSpeed)
    → Lerp Rotation (0 to OpenAngle)
    → Set DoorPanel Rotation
  → Publish Event (Door.Opened)

Close Door:
  → Set bIsOpen = false
  → Play Sound (CloseSound)
  → Timeline (1 to 0 over OpenSpeed)
    → Lerp Rotation (OpenAngle to 0)
    → Set DoorPanel Rotation
  → Publish Event (Door.Closed)
```

**Door Types** (create variants):

1. **BP_Door_Standard** - Standard hinged door
2. **BP_Door_Sliding** - Sliding door (use location instead of rotation)
3. **BP_Door_Hatch** - Floor/ceiling hatch
4. **BP_Door_Airlock** - Double door with pressure seal

### Step 4: Create Examinable Interactable

**Create**: `BP_ExaminableInteractable`

**Parent Class**: ExaminableInteractable

**Purpose**: Objects that can be rotated and inspected.

**Variables**:
- `ExamineDistance` (float, default: 100) - Distance from camera
- `bCanRotate` (bool, default: true)
- `RotationSpeed` (float, default: 50.0)
- `bCanZoom` (bool, default: true)
- `ZoomMin` (float, default: 50)
- `ZoomMax` (float, default: 200)
- `ExamineText` (Text) - Description shown during examination

**Event Graph**:
```
On Interact
  → Enter Examine Mode
  → Store Original Transform
  → Attach to Camera
  → Set Location (Camera Forward * ExamineDistance)
  → Disable Player Movement
  → Show Examine UI

Event Tick (while examining)
  → Branch (bCanRotate)
    True:
      → Get Mouse Delta
      → Rotate Object (Delta * RotationSpeed)
  → Branch (bCanZoom)
    True:
      → Get Mouse Wheel
      → Adjust Distance (Clamp between ZoomMin and ZoomMax)

On Exit Examine (Input)
  → Exit Examine Mode
  → Detach from Camera
  → Restore Original Transform
  → Enable Player Movement
  → Hide Examine UI
```

**Examine UI Widget** (create `WBP_ExamineUI`):

**Widget Hierarchy**:
```
Canvas Panel
├── TextBlock (Examine Text)
├── TextBlock (Controls: "Mouse to rotate, Wheel to zoom, E to exit")
└── Image (Crosshair)
```

### Step 5: Create Pickup Interactable

**Create**: `BP_PickupInteractable`

**Parent Class**: PickupInteractable

**Purpose**: Items that can be picked up and added to inventory.

**Variables**:
- `ItemId` (FName) - Unique item identifier
- `ItemName` (Text)
- `ItemDescription` (Text)
- `ItemIcon` (Texture2D)
- `ItemCategory` (Enum: Tool, Key, Battery, Evidence)
- `bAutoUse` (bool, default: false) - Auto-use on pickup
- `bDestroyOnPickup` (bool, default: true)

**Event Graph**:
```
On Interact
  → Get Inventory Component
  → Branch (Can Add Item)
    True:
      → Add Item to Inventory
      → Play Pickup Sound
      → Show Pickup Toast (ItemName, ItemIcon)
      → Branch (bAutoUse)
        True: Use Item
      → Branch (bDestroyOnPickup)
        True: Destroy Actor
        False: Hide Actor
    False:
      → Show Message ("Inventory full")
```

**Pickup Types** (create variants):

1. **BP_Pickup_Battery** - Camera battery
2. **BP_Pickup_Key** - Door key
3. **BP_Pickup_Flashlight** - Flashlight item
4. **BP_Pickup_MedKit** - Health restoration
5. **BP_Pickup_Tool** - Puzzle tool

### Step 6: Create Recorder Interactable

**Create**: `BP_RecorderInteractable`

**Parent Class**: RecorderInteractable

**Purpose**: Audio recorders that play recordings.

**Components**:
- StaticMeshComponent (recorder mesh)
- AudioComponent (name: `RecorderAudio`)
- PointLightComponent (name: `StatusLight`)

**Variables**:
- `RecordingTitle` (Text)
- `RecordingAudio` (SoundWave)
- `RecordingDuration` (float)
- `RecordingTranscript` (Text)
- `bAutoPlayOnInteract` (bool, default: true)
- `bLoopRecording` (bool, default: false)
- `bCollectAsEvidence` (bool, default: true)

**Event Graph**:
```
On Interact
  → Branch (bAutoPlayOnInteract)
    True:
      → Play Recording
    False:
      → Show Recording UI (with play button)
  → Branch (bCollectAsEvidence)
    True:
      → Create Audio Evidence
      → Collect Evidence

Play Recording:
  → RecorderAudio → Set Sound (RecordingAudio)
  → RecorderAudio → Play
  → StatusLight → Set Color (Green)
  → Show Subtitle UI (RecordingTranscript)
  → Branch (bLoopRecording)
    True: RecorderAudio → Set Looping (true)
    False: 
      → Delay (RecordingDuration)
      → Stop Recording

Stop Recording:
  → RecorderAudio → Stop
  → StatusLight → Set Color (Red)
  → Hide Subtitle UI
```

**Recording UI Widget** (create `WBP_RecordingPlayer`):

**Widget Hierarchy**:
```
Border (Player Panel)
├── VerticalBox
│   ├── TextBlock (Recording Title)
│   ├── HorizontalBox (Controls)
│   │   ├── Button (Play/Pause)
│   │   ├── Slider (Timeline)
│   │   └── TextBlock (Time: 00:00 / 00:00)
│   └── ScrollBox (Transcript)
│       └── TextBlock (Transcript Text)
```

### Step 7: Create Switch Interactable

**Create**: `BP_SwitchInteractable`

**Parent Class**: SwitchInteractable

**Purpose**: Switches, buttons, levers that toggle states.

**Components**:
- StaticMeshComponent (switch base)
- StaticMeshComponent (switch lever, name: `SwitchLever`)
- PointLightComponent (name: `StatusLight`)
- AudioComponent (name: `SwitchAudio`)

**Variables**:
- `bIsOn` (bool, default: false)
- `SwitchType` (Enum: Toggle, Momentary, OneTime)
- `OnAngle` (float, default: 45.0)
- `OffAngle` (float, default: -45.0)
- `SwitchSpeed` (float, default: 0.2)
- `OnSound` (SoundBase)
- `OffSound` (SoundBase)
- `TargetActors` (Array of Actor) - Actors to trigger

**Event Graph**:
```
On Interact
  → Branch (SwitchType)
    Toggle:
      → Toggle Switch State
    Momentary:
      → Turn On
      → Delay (0.5)
      → Turn Off
    OneTime:
      → Branch (bIsOn)
        True: Do Nothing
        False: Turn On

Toggle Switch State:
  → Set bIsOn = !bIsOn
  → Branch (bIsOn)
    True: Turn On
    False: Turn Off

Turn On:
  → Set bIsOn = true
  → Play Sound (OnSound)
  → StatusLight → Set Color (Green)
  → Timeline (0 to 1 over SwitchSpeed)
    → Lerp Rotation (OffAngle to OnAngle)
    → Set SwitchLever Rotation
  → Trigger Target Actors (On)
  → Publish Event (Switch.On)

Turn Off:
  → Set bIsOn = false
  → Play Sound (OffSound)
  → StatusLight → Set Color (Red)
  → Timeline (1 to 0 over SwitchSpeed)
    → Lerp Rotation (OnAngle to OffAngle)
    → Set SwitchLever Rotation
  → Trigger Target Actors (Off)
  → Publish Event (Switch.Off)

Trigger Target Actors:
  Input: bool bOn
  For Each Actor in TargetActors:
    → Branch (Actor implements ITriggerable)
      True:
        → Call Trigger (bOn)
```

### Step 8: Create Interaction Prompt Widget

**Create**: `WBP_InteractionPrompt`

**Purpose**: Shows interaction prompt above interactable objects.

**Widget Hierarchy**:
```
Border (Prompt Background)
└── HorizontalBox
    ├── Image (Input Icon - E key or gamepad button)
    └── TextBlock (Interaction Text)
```

**Variables**:
- `InteractionText` (Text)
- `InputIcon` (Texture2D)

**Functions**:

#### UpdatePrompt
```
Input: FText Text
Set InteractionText = Text
Update Input Icon (based on input device)
```

**Attach to Interactables**:
```
In BP_BaseInteractable:
  Add WidgetComponent
  Set Widget Class: WBP_InteractionPrompt
  Set Widget Space: Screen
  Set Draw Size: (200, 50)
  Set Pivot: (0.5, 1.0) - Bottom center
```

### Step 9: Create Interaction Manager

**Create**: `BP_InteractionManager` (Actor Component)

**Purpose**: Manages player interaction with objects.

**Variables**:
- `InteractionRange` (float, default: 200)
- `InteractionTraceChannel` (ECollisionChannel)
- `CurrentInteractable` (IInteractableInterface)
- `bIsInteracting` (bool)

**Functions**:

#### CheckForInteractables (called on Tick)
```
Line Trace from Camera
  → Branch (Hit Actor implements IInteractableInterface)
    True:
      → Cast to IInteractableInterface
      → Branch (Can Interact)
        True:
          → Set CurrentInteractable
          → Show Interaction Prompt
        False:
          → Clear CurrentInteractable
          → Hide Interaction Prompt
    False:
      → Clear CurrentInteractable
      → Hide Interaction Prompt
```

#### Interact (called on Input)
```
Branch (CurrentInteractable is valid)
  True:
    → Call Interact on CurrentInteractable
    → Set bIsInteracting = true
  False:
    → Do Nothing
```

**Add to BP_PlayerCharacter**:
```
Add Component: BP_InteractionManager

Input Action (Interact)
  → InteractionManager → Interact
```

### Step 10: Create Interaction Tutorial

**Create**: `BP_InteractionTutorial` (Actor)

**Purpose**: Tutorial for teaching interaction mechanics.

**Variables**:
- `TutorialSteps` (Array of FTutorialStep)
- `CurrentStep` (int32)

**Struct**: `FTutorialStep`
- `StepText` (Text)
- `RequiredInteractable` (Actor reference)
- `bCompleted` (bool)

**Event Graph**:
```
Event BeginPlay
  → Show Tutorial Step (0)

On Interactable Interacted (from Event Bus)
  → Check if Interactable matches CurrentStep
  → Branch (Matches)
    True:
      → Mark Step Complete
      → Increment CurrentStep
      → Branch (More Steps)
        True: Show Next Tutorial Step
        False: Complete Tutorial
```

## Blueprint Usage Examples

### Example 1: Locked Door with Key

**BP_Door_Locked**:
```
Variables:
  bIsLocked = true
  RequiredKeyId = "Key_MainHall"

On Interact
  → Get Inventory Component
  → Has Item (RequiredKeyId)
  → Branch (Has Item)
    True:
      → Unlock Door
      → Remove Key from Inventory
      → Open Door
    False:
      → Show Message ("You need the Main Hall Key")
```

### Example 2: Examine Object for Clue

**BP_Examinable_Statue**:
```
On Examine
  → Branch (Player rotates to specific angle)
    True:
      → Reveal Hidden Compartment
      → Spawn Pickup (Key)
      → Show Message ("You found a hidden key!")
```

### Example 3: Switch Activates Multiple Targets

**BP_Switch_PowerRestore**:
```
Variables:
  TargetActors = [Light1, Light2, Light3, Door1]

On Switch On
  → For Each Actor in TargetActors:
      → Branch (Actor is Light)
        True: Turn On Light
      → Branch (Actor is Door)
        True: Unlock Door
```

### Example 4: Document Triggers Objective

**BP_Document_MissionBrief**:
```
On Interact
  → Parent: On Interact
  → Get Objective Manager
  → Activate Objective ("Find the Research Lab")
  → Show Objective Toast
```

### Example 5: Recorder Triggers Event

**BP_Recorder_DistressCall**:
```
On Recording Finished
  → Get Event Bus
  → Publish Event (Audio.DistressCall.Heard)
  → Spawn Threat Actor
  → Show Subtitle ("Did you hear that?")
```

## Event Binding

### BaseInteractable Events

**OnInteract** (BlueprintNativeEvent):
- Called when interaction occurs
- Override for custom behavior

### IInteractableInterface Functions

**CanInteract** (BlueprintNativeEvent):
- Returns whether interaction is allowed
- Override for custom conditions

**Interact** (BlueprintNativeEvent):
- Performs the interaction
- Override for custom actions

**GetInteractionPrompt** (BlueprintNativeEvent):
- Returns prompt text
- Override for dynamic prompts

## Performance Optimization

### Interaction System Optimization
- Use sphere overlap for interaction detection
- Limit interaction trace frequency (0.1s)
- Cache interactable references
- Disable tick when not near player
- Use simple collision for interaction

### Interactable Optimization
- Disable tick when not interacting
- Use LOD for distant interactables
- Pool common interactables
- Unload unused interactables
- Use instanced static meshes

## Troubleshooting

### Interaction Not Working
- Check IInteractableInterface implementation
- Verify CanInteract returns true
- Ensure interaction range sufficient
- Check collision settings
- Verify input binding

### Prompt Not Showing
- Check widget component is visible
- Verify widget class is set
- Ensure widget is in screen space
- Check widget draw size
- Verify prompt update is called

### Door Not Opening
- Check door panel component exists
- Verify timeline is playing
- Ensure rotation values are correct
- Check door is not locked
- Verify sound is playing

### Examine Mode Stuck
- Check exit input is bound
- Verify player movement is re-enabled
- Ensure object is detached
- Check original transform is restored

## Best Practices

1. **Consistent Prompts**: Use consistent interaction prompts
2. **Visual Feedback**: Highlight interactable objects
3. **Audio Feedback**: Play sounds for all interactions
4. **State Management**: Save interaction states
5. **Error Handling**: Handle failed interactions gracefully
6. **Accessibility**: Support keyboard and gamepad
7. **Performance**: Optimize interaction detection
8. **Testing**: Test all interaction types
9. **Documentation**: Document custom interactables
10. **Reusability**: Create reusable interactable templates

## Testing Checklist

- [ ] All interactable types created
- [ ] Interaction prompts show correctly
- [ ] Interaction range appropriate
- [ ] All interactions work
- [ ] Doors open/close smoothly
- [ ] Documents display correctly
- [ ] Examine mode works
- [ ] Pickups add to inventory
- [ ] Recorders play audio
- [ ] Switches trigger targets
- [ ] Save/load preserves states
- [ ] Performance acceptable
- [ ] Gamepad support works
- [ ] Accessibility features work

## Next Steps

1. Create all interactable blueprints
2. Place interactables in level
3. Configure interaction prompts
4. Test each interaction type
5. Implement save/load system
6. Add audio feedback
7. Create interaction tutorial
8. Optimize performance
9. Add accessibility options
10. Playtest interaction flow
