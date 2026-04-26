# Blueprint Door Implementation Guide

## Overview
Complete guide for creating door interactables using Blueprint, inheriting from `ADoorInteractable` C++ class.

## 1. Blueprint Creation Steps

### 1.1 Create Blueprint Class
1. Content Browser → Right-click → Blueprint Class
2. Search for `DoorInteractable` parent class
3. Name: `BP_Door_[Type]` (e.g., `BP_Door_Wooden`, `BP_Door_Metal`)
4. Location: `Content/Blueprints/Interaction/Doors/`

### 1.2 Component Structure
Default components from C++ class:
- **DoorRoot** (SceneComponent) - Root pivot point
- **DoorFrame** (StaticMeshComponent) - Door frame mesh
- **DoorMesh** (StaticMeshComponent) - Rotating door panel
- **InteractionVolume** (BoxComponent) - Interaction trigger

### 1.3 Collision Configuration
**InteractionVolume Settings:**
- Collision Preset: `OverlapAllDynamic`
- Generate Overlap Events: `true`
- Box Extent: `X=100, Y=100, Z=100` (adjust per door size)

**DoorFrame Collision:**
- Collision Preset: `BlockAll`
- Simulate Physics: `false`

**DoorMesh Collision:**
- Collision Preset: `BlockAll`
- Simulate Physics: `false`

## 2. Mesh and Material Configuration

### 2.1 Door Frame Setup
1. Select `DoorFrame` component
2. Static Mesh: Choose frame mesh (e.g., `SM_DoorFrame_01`)
3. Materials: Apply frame material
4. Transform: Position at origin `(0,0,0)`

### 2.2 Door Panel Setup
1. Select `DoorMesh` component
2. Static Mesh: Choose door panel (e.g., `SM_Door_Wooden_01`)
3. Materials: Apply door material
4. Transform:
   - Location: Offset from hinge point
   - Rotation: `(0,0,0)` for closed state
   - **CRITICAL**: Pivot must be at hinge edge

### 2.3 Recommended Assets
**Wooden Door:**
- Frame: `SM_DoorFrame_Wood`
- Panel: `SM_Door_Panel_Wood`
- Material: `M_Wood_Door_Inst`

**Metal Door:**
- Frame: `SM_DoorFrame_Metal`
- Panel: `SM_Door_Panel_Metal`
- Material: `M_Metal_Door_Inst`

**Prison Cell:**
- Frame: `SM_DoorFrame_Prison`
- Panel: `SM_Door_Bars`
- Material: `M_Metal_Rusty_Inst`

## 3. Interaction Parameters

### 3.1 Base Interaction Settings
**Category: Interaction**
- `InteractableId`: Unique name (e.g., `Door_Basement_01`)
- `InteractionPrompt`: "Open Door" / "Close Door"
- `bCanInteractMultipleTimes`: `true`
- `bRequireLineOfSight`: `true`

### 3.2 Door Animation Settings
**Category: Door | Animation**
- `OpenAngle`: `90.0` (degrees, positive = clockwise)
  - Standard door: `90.0`
  - Wide door: `120.0`
  - Narrow opening: `60.0`
- `OpenSpeed`: `2.0` (multiplier)
  - Fast: `3.0`
  - Normal: `2.0`
  - Slow/Heavy: `1.0`
- `OpenCurve`: Optional curve asset for smooth motion
  - Use `Curve_DoorOpen_Smooth` for realistic acceleration

### 3.3 Audio Configuration
**Category: Door | Audio**
- `OpenSound`: Door opening sound
  - Wooden: `SFX_Door_Wood_Open`
  - Metal: `SFX_Door_Metal_Open`
  - Creaky: `SFX_Door_Creaky_Open`
- `CloseSound`: Door closing sound
  - Wooden: `SFX_Door_Wood_Close`
  - Metal: `SFX_Door_Metal_Close`
- `LockedSound`: Locked attempt sound
  - Generic: `SFX_Door_Locked`
  - Rattle: `SFX_Door_Rattle`

### 3.4 Behavior Settings
**Category: Door | Behavior**
- `bAutoClose`: `false` (auto-close after delay)
- `AutoCloseDelay`: `3.0` seconds (if auto-close enabled)
- `bStartLocked`: `false` (locked at game start)

## 4. Specific Functionality

### 4.1 Lock/Unlock System
**Blueprint Event: Set Locked State**
```
Event BeginPlay
├─ Branch (Should Start Locked?)
│  ├─ True → SetLocked(true)
│  └─ False → SetLocked(false)

Custom Event: UnlockDoor
├─ SetLocked(false)
├─ Play Sound (SFX_Door_Unlock)
└─ Publish Event (EventTag: "Door.Unlocked")
```

**C++ Functions Available:**
- `SetLocked(bool bLocked)` - Lock/unlock door
- `IsLocked()` - Check lock state
- `IsOpen()` - Check if door is open
- `GetDoorState()` - Get current state enum

### 4.2 Key Requirement
**Blueprint Logic:**
```
Override: CanInteract
├─ Parent CanInteract → Result
├─ Branch (Is Locked?)
│  ├─ True:
│  │  ├─ Get Inventory Component
│  │  ├─ HasItem("Key_Basement")
│  │  ├─ Branch (Has Key?)
│  │  │  ├─ True → Return True
│  │  │  └─ False → Return False
│  └─ False → Return Result
```

### 4.3 Auto-Close Implementation
**Settings:**
- `bAutoClose`: `true`
- `AutoCloseDelay`: `3.0`

**Behavior:**
- Door opens normally
- After 3 seconds, automatically closes
- Player can still manually close before timer

### 4.4 Direction-Based Opening
**Advanced Blueprint:**
```
Override: OnInteract
├─ Get Player Location
├─ Get Door Forward Vector
├─ Dot Product (Player To Door, Door Forward)
├─ Branch (Dot > 0?)
│  ├─ True → Open Inward (Negative Angle)
│  └─ False → Open Outward (Positive Angle)
└─ Call Parent OnInteract
```

## 5. Event Logic

### 5.1 OnInteract Event
**Default C++ Behavior:**
1. Check if locked → Play locked sound, return
2. Check current state:
   - Closed → Start opening
   - Open → Start closing
   - Opening/Closing → Ignore
3. Play appropriate sound
4. Update door state
5. Publish EventBus event

**Blueprint Extension:**
```
Event OnInteract (Override)
├─ Branch (Is Locked AND Has Required Key?)
│  ├─ True:
│  │  ├─ Consume Key Item
│  │  ├─ SetLocked(false)
│  │  ├─ Show Message "Door Unlocked"
│  │  └─ Play Unlock Sound
│  └─ False → Continue
├─ Call Parent OnInteract
└─ Custom Logic (Optional)
```

### 5.2 State Change Events
**Blueprint Events to Implement:**
```
Event OnDoorOpened
├─ Trigger Connected Events
├─ Update Quest Objectives
└─ Spawn Enemies (if scripted)

Event OnDoorClosed
├─ Check Lock Conditions
└─ Update AI Navigation

Event OnDoorLocked
├─ Update UI Prompt
└─ Publish EventBus Event
```

### 5.3 EventBus Integration
**Automatic Events Published:**
- `Door.Opened` - When door finishes opening
- `Door.Closed` - When door finishes closing
- `Door.Locked` - When door is locked
- `Door.Unlocked` - When door is unlocked

**Subscribe to Events:**
```
Event BeginPlay
├─ Get EventBus Subsystem
├─ Subscribe to "Door.Opened"
└─ Bind to Custom Event

Custom Event: OnDoorOpenedEvent
├─ Check Event Data (Door ID)
└─ Execute Response Logic
```

### 5.4 SaveGame Integration
**Automatic Save/Load:**
- Door state (open/closed)
- Lock state
- Interaction count

**Blueprint Extension:**
```
Override: SaveState
├─ Call Parent SaveState
├─ Add Custom State ("HasBeenUnlocked", bWasUnlocked)
└─ Return State Map

Override: LoadState
├─ Call Parent LoadState
├─ Get Custom State ("HasBeenUnlocked")
└─ Apply Custom Logic
```

## 6. Testing Validation

### 6.1 Functional Tests
- [ ] Door opens smoothly to correct angle
- [ ] Door closes back to original position
- [ ] Locked door plays locked sound
- [ ] Unlocking with key works correctly
- [ ] Auto-close timer functions (if enabled)
- [ ] Multiple interactions work properly
- [ ] Collision blocks player correctly
- [ ] Interaction prompt updates correctly

### 6.2 Edge Cases
- [ ] Rapid interaction spam handling
- [ ] Player blocking door path
- [ ] Door state during level transition
- [ ] Save/load preserves door state
- [ ] Network replication (if multiplayer)
- [ ] Door animation interruption
- [ ] Key consumption on unlock
- [ ] EventBus events fire correctly

### 6.3 Performance Validation
- [ ] No tick overhead when idle
- [ ] Smooth animation at 60+ FPS
- [ ] Audio doesn't overlap/spam
- [ ] Collision checks optimized
- [ ] No memory leaks on destroy

### 6.4 Audio/Visual Polish
- [ ] Opening sound matches animation
- [ ] Closing sound plays at right time
- [ ] Locked sound is distinct
- [ ] Door mesh pivot is correct
- [ ] No clipping with frame
- [ ] Materials look correct

## 7. Common Configurations

### 7.1 Basic Wooden Door
```
OpenAngle: 90.0
OpenSpeed: 2.0
bAutoClose: false
bStartLocked: false
OpenSound: SFX_Door_Wood_Open
CloseSound: SFX_Door_Wood_Close
```

### 7.2 Heavy Metal Door
```
OpenAngle: 90.0
OpenSpeed: 1.0
bAutoClose: false
bStartLocked: false
OpenSound: SFX_Door_Metal_Heavy_Open
CloseSound: SFX_Door_Metal_Heavy_Close
OpenCurve: Curve_DoorOpen_Heavy
```

### 7.3 Auto-Closing Security Door
```
OpenAngle: 90.0
OpenSpeed: 2.5
bAutoClose: true
AutoCloseDelay: 2.0
bStartLocked: false
OpenSound: SFX_Door_Security_Open
CloseSound: SFX_Door_Security_Close
```

### 7.4 Locked Story Door
```
OpenAngle: 90.0
OpenSpeed: 2.0
bAutoClose: false
bStartLocked: true
InteractableId: "Door_Basement_Main"
LockedSound: SFX_Door_Locked_Rattle
Required Key: "Key_Basement"
```

## 8. Troubleshooting

### Issue: Door rotates from wrong pivot
**Solution:** Adjust DoorMesh location so pivot is at hinge edge

### Issue: Door opens wrong direction
**Solution:** Change OpenAngle sign (positive/negative)

### Issue: Door animation is jerky
**Solution:** Add OpenCurve asset for smooth interpolation

### Issue: Locked sound doesn't play
**Solution:** Ensure LockedSound is assigned and bStartLocked is true

### Issue: Door doesn't save state
**Solution:** Ensure InteractableId is unique and not NAME_None

### Issue: Collision blocks interaction
**Solution:** Check InteractionVolume extends beyond door frame
