# Blueprint Pickup Implementation Guide

## Overview
Complete guide for creating pickup interactables using Blueprint, inheriting from `APickupInteractable` C++ class.

## 1. Blueprint Creation Steps

### 1.1 Create Blueprint Class
1. Content Browser → Right-click → Blueprint Class
2. Search for `PickupInteractable` parent class
3. Name: `BP_Pickup_[Type]` (e.g., `BP_Pickup_Key`, `BP_Pickup_Evidence`)
4. Location: `Content/Blueprints/Interaction/Pickups/`

### 1.2 Component Structure
Default components from C++ class:
- **PickupMesh** (StaticMeshComponent) - Visual representation
- **InteractionVolume** (SphereComponent) - Interaction trigger

### 1.3 Collision Configuration
**InteractionVolume Settings:**
- Collision Preset: `OverlapAllDynamic`
- Generate Overlap Events: `true`
- Sphere Radius: `100.0` (adjust per pickup size)

**PickupMesh Collision:**
- Collision Preset: `NoCollision` or `OverlapAll`
- Simulate Physics: `false`

## 2. Mesh and Material Configuration

### 2.1 Pickup Mesh Setup
1. Select `PickupMesh` component
2. Static Mesh: Choose appropriate mesh
3. Materials: Apply material with emissive glow (optional)
4. Transform:
   - Location: `(0,0,0)`
   - Rotation: `(0,0,0)`
   - Scale: Adjust to appropriate size

### 2.2 Recommended Assets by Type

**Evidence Items:**
- Mesh: `SM_Evidence_Photo`, `SM_Evidence_Note`, `SM_Evidence_Tape`
- Material: `M_Evidence_Glow` (subtle emissive)

**Keys:**
- Mesh: `SM_Key_Old`, `SM_Key_Modern`, `SM_Keycard`
- Material: `M_Metal_Key_Inst`

**Tools:**
- Mesh: `SM_Flashlight`, `SM_Crowbar`, `SM_Wrench`
- Material: `M_Tool_Metal_Inst`

**Batteries:**
- Mesh: `SM_Battery_AA`, `SM_Battery_D`
- Material: `M_Battery_Inst`

**Documents:**
- Mesh: `SM_Paper_Folded`, `SM_Notebook`
- Material: `M_Paper_Inst`

### 2.3 Visual Effects
**Glow Material Setup:**
- Base Color: Item color
- Emissive: Subtle glow (0.5-1.0 intensity)
- Roughness: 0.4-0.6

**Particle Effects (Optional):**
- Add `ParticleSystemComponent`
- Template: `PS_Pickup_Sparkle`
- Auto Activate: `true`

## 3. Interaction Parameters

### 3.1 Base Interaction Settings
**Category: Interaction**
- `InteractableId`: Unique name (e.g., `Pickup_Key_Basement`)
- `eractionPrompt`: "Pick Up [Item Name]"
- `bCanInteractMultipleTimes`: `false` (single pickup)
- `bRequireLineOfSight`: `true`

### 3.2 Pickup Item Settings
**Category: Pickup | Item**
- `PickupType`: Select from enum
  - `Evidence` - Collectible evidence
  - `Key` - Unlocks doors/containers
  - `Tool` - Usable tools (flashlight, etc.)
  - `Document` - Readable documents
  - `Battery` - Flashlight power
  - `Generic` - Other items
- `ItemId`: Inventory identifier (e.g., `Key_Basement`)
  - **CRITICAL**: Must match inventory system IDs
- `EvidenceMetadata`: (if PickupType = Evidence)
  - `EvidenceId`: Unique evidence ID
  - `EvidenceName`: Display name
  - `Description`: Evidence description
  - `Category`: Evidence category
  - `bIsOptional`: Optional evidence flag

### 3.3 Visual Animation Settings
**Category: Pickup | Visual**
- `bRotateInPlace`: `true` (rotate while idle)
- `RotationSpeed`: `90.0` degrees/second
  - Slow: `45.0`
  - Normal: `90.0`
  - Fast: `180.0`
- `bFloatInPlace`: `true` (bob up/down)
- `FloatAmplitude`: `10.0` units
  - Subtle: `5.0`
  - Normal: `10.0`
  - Pronounced: `20.0`
- `FloatSpeed`: `2.0` cycles/second
  - Slow: `1.0`
  - Normal: `2.0`
  - Fast: `3.0`

### 3.4 Audio Configuration
**Category: Pickup | Audio**
- `PickupSound`: Sound played on pickup
  - Evidence: `SFX_Pickup_Evidence`
  - Key: `SFX_Pickup_Key`
  - Tool: `SFX_Pickup_Tool`
  - Battery: `SFX_Pickup_Battery`
  - Generic: `SFX_Pickup_Item`

### 3.5 Behavior Settings
**Category: Pickup | Behavior**
- `bDestroyOnPickup`: `true` (remove from world)
- `DestroyDelay`: `0.0` seconds
  - Immediate: `0.0`
  - With fade: `0.5-1.0`

## 4. Specific Functionality

### 4.1 Evidence Collection
**For PickupType = Evidence:**
1. Set `EvidenceMetadata`:
   - `EvidenceId`: `"Evidence_Crime_Photo_01"`
   - `EvidenceName`: `"Crime Scene Photo"`
   - `Description`: `"A disturbing photograph..."`
   - `Category`: `"Visual Evidence"`
   - `bIsOptional`: `false`

2. Automatic behavior:
   - Adds to inventory
   - Registers with evidence system
   - Updates objectives
   - Publishes EventBus event

### 4.2 Key Items
**For PickupType = Key:**
1. Set `ItemId`: `"Key_Basement"`
2. Ensure matching door has same key requirement
3. Key is consumed when used on door

**Blueprint Extension:**
```
Event OnPickedUp
├─ Show Message "Basement Key Acquired"
├─ Update Quest Objective
└─ Publish Event "Key.Acquired.Basement"
```

### 4.3 Tool Items
**For PickupType = Tool:**
1. Set `ItemId`: `"Tool_Flashlight"`
2. Tool remains in inventory
3. Can be equipped/used

**Example Tools:**
- `Tool_Flashlight` - Light source
- `Tool_Crowbar` - Opens crates
- `Tool_Lockpick` - Unlocks doors

### 4.4 Battery System
**For PickupType = Battery:**
1. Set `ItemId`: `"Battery_AA"`
2. Adds charge to flashlight
3. Can stack in inventory

**Blueprint Logic:**
```
Override: OnInteract
├─ Get Flashlight Component
├─ Add Battery Charge (50%)
├─ Play Pickup Sound
├─ Show Message "+50% Battery"
└─ Destroy Pickup
```

### 4.5 Conditional Pickup
**Blueprint Implementation:**
```
Override: CanInteract
├─ Parent CanInteract → Result
├─ Branch (Inventory Full?)
│  ├─ True:
│  │  ├─ Show Message "Inventory Full"
│  │  └─ Return False
│  └─ False → Return Result
```

## 5. Event Logic

### 5.1 OnInteract Event
**Default C++ Behavior:**
1. Check if can interact
2. Try add to inventory
3. If successful:
   - Play pickup sound
   - Publish EventBus event
   - Destroy pickup (if enabled)
4. If failed:
   - Show error message

**Blueprint Extension:**
```
Event OnInteract (Override)
├─ Call Parent OnInteract
├─ Branch (Pickup Type)
│  ├─ Evidence:
│  │  ├─ Show Evidence Notification
│  │  └─ Update Evidence Counter
│  ├─ Key:
│  │  ├─ Show Key Acquired Message
│  │  └─ Highlight Locked Doors
│  ├─ Tool:
│  │  ├─ Show Tool Tutorial
│  │  └─ Enable Tool Usage
│  └─ Battery:
│     ├─ Show Battery Charge
│     └─ Update Flashlight UI
```

### 5.2 Visual Feedback Events
**Blueprint Events:**
```
Event OnPickupHighlighted
├─ Set Material Parameter (Glow Intensity, 2.0)
├─ Play Particle Effect
└─ Show Interaction Prompt

Event OnPickupUnhighlighted
├─ Set Material Parameter (Glow Intensity, 1.0)
└─ Stop Particle Effect
```

### 5.3 EventBus Integration
**Automatic Events Published:**
- `Pickup.Collected` - When item is picked up
- `Evidence.Collected` - When evidence is collected
- `Key.Acquired` - When key is picked up
- `Tool.Acquired` - When tool is picked up

**Custom Event Publishing:**
```
Event OnPickedUp
├─ Get EventBus Subsystem
├─ Create Event Data
│  ├─ Add "ItemId" → ItemId
│  ├─ Add "PickupType" → PickupType
│  └─ Add "Location" → Actor Location
└─ Publish Event ("Pickup.Custom.Event")
```

### 5.4 SaveGame Integration
**Automatic Save/Load:**
- Pickup collected state
- Prevents re-spawning collected items

**Blueprint Extension:**
```
Override: SaveState
├─ Call Parent SaveState
├─ Add State ("WasCollected", true)
└─ Return State Map

Override: LoadState
├─ Call Parent LoadState
├─ Get State ("WasCollected")
├─ Branch (Was Collected?)
│  ├─ True → Destroy Self
│  └─ False → Continue
```

## 6. Testing Validation

### 6.1 Functional Tests
- [ ] Pickup adds to inventory correctly
- [ ] Correct item ID is registered
- [ ] Pickup sound plays on collection
- [ ] Visual effects (rotation/float) work
- [ ] Pickup destroys after collection
- [ ] Interaction prompt displays correctly
- [ ] Evidence metadata is complete
- [ ] Key unlocks correct doors

### 6.2 Edge Cases
- [ ] Inventory full handling
- [ ] Duplicate pickup prevention
- [ ] Rapid interaction spam
- [ ] Pickup during level transition
- [ ] Save/load prevents re-collection
- [ ] Network replication (if multiplayer)
- [ ] Pickup in unreachable locations
- [ ] Multiple players pickup simultaneously

### 6.3 Performance Validation
- [ ] Rotation/float animation smooth
- [ ] No tick overhead when not visible
- [ ] Particle effects optimized
- [ ] Audio doesn't overlap
- [ ] Collision checks efficient
- [ ] No memory leaks on destroy

### 6.4 Integration Tests
- [ ] Evidence system receives data
- [ ] Inventory UI updates correctly
- [ ] Quest objectives update
- [ ] EventBus events fire correctly
- [ ] Key works with doors
- [ ] Tool becomes usable
- [ ] Battery charges flashlight

## 7. Common Configurations

### 7.1 Basic Evidence Item
```
PickupType: Evidence
ItemId: "Evidence_Photo_01"
EvidenceMetadata:
  EvidenceId: "Evidence_Photo_01"
  EvidenceName: "Crime Scene Photo"
  Description: "A disturbing photograph of the crime scene"
  Category: "Visual Evidence"
  bIsOptional: false
bRotateInPlace: true
RotationSpeed: 90.0
bFloatInPlace: true
FloatAmplitude: 10.0
PickupSound: SFX_Pickup_Evidence
bDestroyOnPickup: true
```

### 7.2 Standard Key
```
PickupType: Key
ItemId: "Key_Basement"
InteractionPrompt: "Pick Up Basement Key"
bRotateInPlace: true
RotationSpeed: 120.0
bFloatInPlace: true
FloatAmplitude: 8.0
PickupSound: SFX_Pickup_Key
bDestroyOnPickup: true
```

### 7.3 Flashlight Tool
```
PickupType: Tool
ItemId: "Tool_Flashlight"
InteractionPrompt: "Pick Up Flashlight"
bRotateInPlace: false
bFloatInPlace: true
FloatAmplitude: 5.0
FloatSpeed: 1.5
PickupSound: SFX_Pickup_Tool
bDestroyOnPickup: true
```

### 7.4 Battery Pickup
```
PickupType: Battery
ItemId: "Battery_AA"
InteractionPrompt: "Pick Up Battery"
bRotateInPlace: true
RotationSpeed: 180.0
bFloatInPlace: true
FloatAmplitude: 6.0
FloatSpeed: 2.5
PickupSound: SFX_Pickup_Battery
bDestroyOnPickup: true
```

### 7.5 Optional Evidence
```
PickupType: Evidence
ItemId: "Evidence_Optional_Note"
EvidenceMetadata:
  EvidenceId: "Evidence_Optional_Note"
  EvidenceName: "Hidden Note"
  Description: "An optional clue"
  Category: "Documents"
  bIsOptional: true
bRotateInPlace: true
RotationSpeed: 60.0
bFloatInPlace: true
FloatAmplitude: 12.0
PickupSound: SFX_Pickup_Evidence
bDestroyOnPickup: true
```

## 8. Advanced Features

### 8.1 Delayed Spawn
**Blueprint Implementation:**
```
Event BeginPlay
├─ Set Actor Hidden (true)
├─ Set Collision Enabled (No Collision)
├─ Delay (5.0 seconds)
├─ Set Actor Hidden (false)
├─ Set Collision Enabled (Query and Physics)
└─ Play Spawn Effect
```

### 8.2 Respawning Pickup
**For consumables that respawn:**
```
Override: OnInteract
├─ Call Parent OnInteract
├─ Set Actor Hidden (true)
├─ Set Collision Enabled (No Collision)
├─ Delay (30.0 seconds)
├─ Set Actor Hidden (false)
└─ Set Collision Enabled (Query and Physics)

Set: bDestroyOnPickup = false
```

### 8.3 Proximity Highlight
**Blueprint Implementation:**
```
Event Tick
├─ Get Player Character
├─ Get Distance to Player
├─ Branch (Distance < 200?)
│  ├─ True:
│  │  ├─ Set Material Glow (2.0)
│  │  └─ Show Interaction Widget
│  └─ False:
│     ├─ Set Material Glow (1.0)
│     └─ Hide Interaction Widget
```

### 8.4 Pickup Chain
**Trigger next pickup on collection:**
```
Event OnPickedUp
├─ Get Next Pickup Actor
├─ Branch (Is Valid?)
│  ├─ True:
│  │  ├─ Set Next Pickup Visible
│  │  └─ Play Spawn Effect
│  └─ False → Continue
```

## 9. Troubleshooting

### Issue: Pickup doesn't add to inventory
**Solution:** Verify ItemId matches inventory system IDs exactly

### Issue: Rotation/float not working
**Solution:** Ensure bRotateInPlace/bFloatInPlace are true and speeds > 0

### Issue: Pickup sound doesn't play
**Solution:** Check PickupSound is assigned and not null

### Issue: Evidence not registering
**Solution:** Verify EvidenceMetadata is fully filled out with unique IDs

### Issue: Pickup respawns after save/load
**Solution:** Ensure InteractableId is unique and not NAME_None

### Issue: Interaction prompt not showing
**Solution:** Check InteractionVolume radius and collision settings

### Issue: Key doesn't unlock door
**Solution:** Verify ItemId matches door's required key ID exactly

### Issue: Pickup falls through floor
**Solution:** Disable physics simulation on PickupMesh component
