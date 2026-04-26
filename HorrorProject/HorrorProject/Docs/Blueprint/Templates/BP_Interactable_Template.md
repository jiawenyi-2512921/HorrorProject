# Blueprint Interactable Template

## Template: BP_Interactable_Template

Use this template as a starting point for creating interactable objects.

---

## BP_BaseInteractable Template

**Parent Class**: BaseInteractable (C++)

**Purpose**: Base class for all interactable objects.

### Components
```
Root (StaticMeshComponent)
├── SphereComponent (InteractionRadius)
└── WidgetComponent (InteractionPrompt)
```

### Variables

**Configuration**:
- `InteractableId` (Name) - Unique identifier
- `InteractionPrompt` (Text, default: "Interact") - Prompt text
- `bCanInteractMultipleTimes` (bool, default: false) - Allow multiple interactions
- `bRequireLineOfSight` (bool, default: true) - Require LOS to interact

**Visual Feedback**:
- `bHighlightWhenNearby` (bool, default: true) - Highlight on approach
- `HighlightColor` (LinearColor, default: Yellow) - Highlight color
- `HighlightIntensity` (float, default: 2.0) - Highlight intensity

**Audio**:
- `InteractionSound` (SoundBase) - Sound on interaction
- `HoverSound` (SoundBase) - Sound on hover

**State**:
- `bHasBeenInteracted` (bool, default: false) - Interaction state
- `bIsPlayerNearby` (bool, default: false) - Player proximity state

### Functions

#### CanInteract (from IInteractableInterface)
```
Return: bool
  → Branch (bHasBeenInteracted AND NOT bCanInteractMultipleTimes)
    True: Return false
  → Branch (bRequireLineOfSight)
    True:
      → Line Trace to Player
      → Return (Has Line of Sight)
    False:
      → Return true
```

#### Interact (from IInteractableInterface)
```
Input: Actor Interactor
  → Branch (CanInteract)
    True:
      → Set bHasBeenInteracted = true
      → Play Interaction Sound
      → Publish Interaction Event
      → Call OnInteract (Blueprint event)
      → Return true
    False:
      → Return false
```

#### OnInteract (BlueprintNativeEvent)
```
Override in child blueprints for custom behavior
```

#### ShowInteractionPrompt
```
  → InteractionPrompt → Set Visibility (true)
  → Update Prompt Text
```

#### HideInteractionPrompt
```
  → InteractionPrompt → Set Visibility (false)
```

#### EnableHighlight
```
  → Create Dynamic Material Instance
  → Set Emissive Color (HighlightColor)
  → Set Emissive Intensity (HighlightIntensity)
```

#### DisableHighlight
```
  → Reset Material to Original
```

### Event Graph

```
Event BeginPlay
  → Setup Interaction Prompt
  → Hide Interaction Prompt
  → Store Original Material

On Component Begin Overlap (InteractionRadius)
  → Cast to PlayerCharacter
  → Branch (Is Player)
    True:
      → Set bIsPlayerNearby = true
      → ShowInteractionPrompt
      → Branch (bHighlightWhenNearby)
        True: EnableHighlight
      → Play Hover Sound

On Component End Overlap (InteractionRadius)
  → Set bIsPlayerNearby = false
  → HideInteractionPrompt
  → DisableHighlight
```

---

## BP_DocumentInteractable Template

**Parent Class**: BP_BaseInteractable

**Purpose**: Readable document interactable.

### Variables

**Content**:
- `DocumentTitle` (Text) - Document title
- `DocumentContent` (Text) - Document text content
- `DocumentTexture` (Texture2D) - Optional document image
- `PageCount` (int32, default: 1) - Number of pages
- `CurrentPage` (int32, default: 0) - Current page

**Evidence**:
- `bAutoCollectAsEvidence` (bool, default: true) - Auto-collect as evidence
- `EvidenceCategory` (EEvidenceCategory, default: Document) - Evidence type

### Functions

#### OnInteract (override)
```
  → Parent: OnInteract
  → Get UI Manager
  → Show Document Viewer (DocumentTitle, DocumentContent, DocumentTexture)
  → Branch (bAutoCollectAsEvidence)
    True:
      → Create Evidence Metadata
      → Get Evidence Collection Component
      → Collect Evidence
```

### Event Graph

```
On Document Viewer Closed
  → Mark as read
  → Update state
```

---

## BP_DoorInteractable Template

**Parent Class**: BP_BaseInteractable

**Purpose**: Door that can be opened/closed.

### Components
```
Root (StaticMeshComponent) - Door frame
├── StaticMeshComponent (DoorPanel)
├── AudioComponent (DoorAudio)
└── SphereComponent (InteractionRadius)
```

### Variables

**State**:
- `bIsOpen` (bool, default: false) - Door open state
- `bIsLocked` (bool, default: false) - Door locked state

**Configuration**:
- `OpenAngle` (float, default: 90.0) - Door open angle
- `OpenSpeed` (float, default: 2.0) - Door open speed
- `DoorType` (Enum: Hinged, Sliding) - Door movement type

**Lock**:
- `RequiredKeyId` (Name) - Key required to unlock
- `bConsumeKeyOnUnlock` (bool, default: false) - Remove key after use

**Audio**:
- `OpenSound` (SoundBase) - Door open sound
- `CloseSound` (SoundBase) - Door close sound
- `LockedSound` (SoundBase) - Locked door sound
- `UnlockSound` (SoundBase) - Door unlock sound

### Functions

#### OnInteract (override)
```
  → Branch (bIsLocked)
    True:
      → Check Player Has Key (RequiredKeyId)
      → Branch (Has Key)
        True:
          → UnlockDoor
          → Branch (bConsumeKeyOnUnlock)
            True: Remove Key from Inventory
          → OpenDoor
        False:
          → Play Locked Sound
          → Show Message ("Door is locked")
    False:
      → Branch (bIsOpen)
        True: CloseDoor
        False: OpenDoor
```

#### OpenDoor
```
  → Set bIsOpen = true
  → Play Open Sound
  → Branch (DoorType)
    Hinged:
      → Timeline (0 to 1 over OpenSpeed)
        → Lerp Rotation (0 to OpenAngle)
        → Set DoorPanel Rotation
    Sliding:
      → Timeline (0 to 1 over OpenSpeed)
        → Lerp Location (Closed to Open Position)
        → Set DoorPanel Location
  → Publish Event (Door.Opened)
```

#### CloseDoor
```
  → Set bIsOpen = false
  → Play Close Sound
  → Branch (DoorType)
    Hinged:
      → Timeline (1 to 0 over OpenSpeed)
        → Lerp Rotation (OpenAngle to 0)
        → Set DoorPanel Rotation
    Sliding:
      → Timeline (1 to 0 over OpenSpeed)
        → Lerp Location (Open to Closed Position)
        → Set DoorPanel Location
  → Publish Event (Door.Closed)
```

#### UnlockDoor
```
  → Set bIsLocked = false
  → Play Unlock Sound
  → Show Message ("Door unlocked")
  → Publish Event (Door.Unlocked)
```

---

## BP_PickupInteractable Template

**Parent Class**: BP_BaseInteractable

**Purpose**: Item that can be picked up.

### Variables

**Item Data**:
- `ItemId` (Name) - Unique item identifier
- `ItemName` (Text) - Display name
- `ItemDescription` (Text) - Item description
- `ItemIcon` (Texture2D) - Item icon
- `ItemCategory` (Enum) - Item category

**Behavior**:
- `bAutoUse` (bool, default: false) - Auto-use on pickup
- `bDestroyOnPickup` (bool, default: true) - Destroy after pickup
- `bAddToInventory` (bool, default: true) - Add to inventory

### Functions

#### OnInteract (override)
```
  → Get Inventory Component
  → Branch (bAddToInventory)
    True:
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
    False:
      → Use Item Immediately
      → Branch (bDestroyOnPickup)
        True: Destroy Actor
```

---

## BP_ExaminableInteractable Template

**Parent Class**: BP_BaseInteractable

**Purpose**: Object that can be examined up close.

### Variables

**Examine Settings**:
- `ExamineDistance` (float, default: 100) - Distance from camera
- `bCanRotate` (bool, default: true) - Allow rotation
- `RotationSpeed` (float, default: 50.0) - Rotation speed
- `bCanZoom` (bool, default: true) - Allow zoom
- `ZoomMin` (float, default: 50) - Min zoom distance
- `ZoomMax` (float, default: 200) - Max zoom distance

**Content**:
- `ExamineText` (Text) - Description during examination

**State**:
- `bIsBeingExamined` (bool, default: false) - Examine state
- `OriginalTransform` (Transform) - Original transform
- `CurrentZoomDistance` (float) - Current zoom distance

### Functions

#### OnInteract (override)
```
  → EnterExamineMode
```

#### EnterExamineMode
```
  → Set bIsBeingExamined = true
  → Store OriginalTransform
  → Detach from Parent
  → Attach to Camera
  → Set Relative Location (Forward * ExamineDistance)
  → Disable Player Movement
  → Show Examine UI (ExamineText)
  → Set Input Mode (UI Only)
```

#### ExitExamineMode
```
  → Set bIsBeingExamined = false
  → Detach from Camera
  → Set World Transform (OriginalTransform)
  → Enable Player Movement
  → Hide Examine UI
  → Set Input Mode (Game Only)
```

#### RotateObject
```
Input: Vector2D MouseDelta
  → Branch (bCanRotate)
    True:
      → Add Relative Rotation (MouseDelta * RotationSpeed * DeltaTime)
```

#### ZoomObject
```
Input: float MouseWheel
  → Branch (bCanZoom)
    True:
      → CurrentZoomDistance += MouseWheel * 10
      → Clamp (ZoomMin, ZoomMax)
      → Set Relative Location (Forward * CurrentZoomDistance)
```

### Event Graph

```
Event Tick (while examining)
  → Branch (bIsBeingExamined)
    True:
      → Get Mouse Delta
      → RotateObject (Mouse Delta)
      → Get Mouse Wheel
      → ZoomObject (Mouse Wheel)

On Exit Input (E key)
  → Branch (bIsBeingExamined)
    True: ExitExamineMode
```

---

## BP_SwitchInteractable Template

**Parent Class**: BP_BaseInteractable

**Purpose**: Switch or button that toggles state.

### Components
```
Root (StaticMeshComponent) - Switch base
├── StaticMeshComponent (SwitchLever)
├── PointLightComponent (StatusLight)
└── AudioComponent (SwitchAudio)
```

### Variables

**State**:
- `bIsOn` (bool, default: false) - Switch state

**Configuration**:
- `SwitchType` (Enum: Toggle, Momentary, OneTime) - Switch behavior
- `OnAngle` (float, default: 45.0) - On position angle
- `OffAngle` (float, default: -45.0) - Off position angle
- `SwitchSpeed` (float, default: 0.2) - Animation speed

**Targets**:
- `TargetActors` (Array of Actor) - Actors to trigger

**Audio**:
- `OnSound` (SoundBase) - Switch on sound
- `OffSound` (SoundBase) - Switch off sound

### Functions

#### OnInteract (override)
```
  → Branch (SwitchType)
    Toggle:
      → ToggleSwitchState
    Momentary:
      → TurnOn
      → Delay (0.5)
      → TurnOff
    OneTime:
      → Branch (bIsOn)
        True: Do Nothing
        False: TurnOn
```

#### TurnOn
```
  → Set bIsOn = true
  → Play On Sound
  → StatusLight → Set Color (Green)
  → Timeline (0 to 1 over SwitchSpeed)
    → Lerp Rotation (OffAngle to OnAngle)
    → Set SwitchLever Rotation
  → TriggerTargetActors (true)
  → Publish Event (Switch.On)
```

#### TurnOff
```
  → Set bIsOn = false
  → Play Off Sound
  → StatusLight → Set Color (Red)
  → Timeline (1 to 0 over SwitchSpeed)
    → Lerp Rotation (OnAngle to OffAngle)
    → Set SwitchLever Rotation
  → TriggerTargetActors (false)
  → Publish Event (Switch.Off)
```

#### TriggerTargetActors
```
Input: bool bOn
  → For Each Actor in TargetActors:
      → Branch (Actor implements ITriggerable)
        True: Call Trigger (bOn)
```

---

## Best Practices

### Interactable Design
1. Use consistent interaction prompts
2. Provide clear visual feedback
3. Play audio feedback for all interactions
4. Save interaction state
5. Handle failed interactions gracefully

### Performance
1. Disable tick when not near player
2. Use simple collision for interaction
3. Limit interaction range (200-300 units)
4. Cache component references
5. Pool common interactables

### Visual Feedback
1. Highlight interactables when nearby
2. Show interaction prompt clearly
3. Animate state changes smoothly
4. Use status lights for switches
5. Provide feedback for locked/unavailable states

### Audio Feedback
1. Play sound on interaction
2. Play sound on hover
3. Use different sounds for success/failure
4. Attenuate sounds appropriately
5. Don't overlap too many sounds

### State Management
1. Save interaction state
2. Restore state on load
3. Handle multiple interactions correctly
4. Publish events for other systems
5. Use unique IDs for all interactables

---

## Testing Checklist

- [ ] Interactable is visible
- [ ] Interaction prompt shows when nearby
- [ ] Interaction works on input
- [ ] Visual feedback is clear
- [ ] Audio feedback plays
- [ ] State saves/loads correctly
- [ ] Multiple interactions work (if enabled)
- [ ] Line of sight check works (if enabled)
- [ ] Integration with other systems works
- [ ] Performance is acceptable

---

## Common Issues

### Interaction Not Working
- Check IInteractableInterface implementation
- Verify CanInteract returns true
- Ensure interaction range sufficient
- Check collision settings
- Verify input binding

### Prompt Not Showing
- Check widget component visibility
- Verify widget class is set
- Ensure widget is in screen space
- Check widget draw size
- Verify prompt update is called

### Highlight Not Working
- Check material supports emissive
- Verify dynamic material instance created
- Ensure highlight color is visible
- Check emissive intensity
- Verify material is assigned

### State Not Saving
- Check unique ID is set
- Verify save function is called
- Ensure load function is called
- Check save game object
- Verify state is serialized

---

## Example Implementation

See these files for reference:
- `/Content/Blueprints/Interactables/BP_BaseInteractable`
- `/Content/Blueprints/Interactables/BP_DocumentInteractable`
- `/Content/Blueprints/Interactables/BP_DoorInteractable`
- `/Content/Blueprints/Interactables/BP_PickupInteractable`
