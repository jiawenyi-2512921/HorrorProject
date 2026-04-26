# Blueprint Examinable Implementation Guide

## Overview
Complete guide for creating examinable interactables using Blueprint, inheriting from `AExaminableInteractable` C++ class.

## 1. Blueprint Creation Steps

### 1.1 Create Blueprint Class
1. Content Browser → Right-click → Blueprint Class
2. Search for `ExaminableInteractable` parent class
3. Name: `BP_Examinable_[Type]` (e.g., `BP_Examinable_Statue`, `BP_Examinable_Artifact`)
4. Location: `Content/Blueprints/Interaction/Examinables/`

### 1.2 Component Structure
Default components from C++ class:
- **ExamineMesh** (StaticMeshComponent) - Object to examine
- **InteractionVolume** (BoxComponent) - Interaction trigger

### 1.3 Collision Configuration
**InteractionVolume Settings:**
- Collision Preset: `OverlapAllDynamic`
- Generate Overlap Events: `true`
- Box Extent: `X=100, Y=100, Z=100` (adjust per object size)

**ExamineMesh Collision:**
- Collision Preset: `NoCollision` (during examine mode)
- Simulate Physics: `false`

## 2. Mesh and Material Configuration

### 2.1 Examine Mesh Setup
1. Select `ExamineMesh` component
2. Static Mesh: Choose object to examine
3. Materials: Apply high-quality materials
4. Transform:
   - Location: `(0,0,0)`
   - Rotation: `(0,0,0)`
   - Scale: Appropriate size for examination

### 2.2 Recommended Assets by Type

**Small Objects (Artifacts, Tools):**
- Mesh: `SM_Artifact_Idol`, `SM_Tool_Knife`, `SM_Jewelry_Ring`
- Material: High-detail materials with normal maps
- Scale: 1.0-2.0 for close inspection

**Medium Objects (Statues, Boxes):**
- Mesh: `SM_Statue_Small`, `SM_Box_Ornate`, `SM_Skull`
- Material: Detailed materials with roughness variation
- Scale: 0.5-1.0 for comfortable viewing

**Documents/Photos:**
- Mesh: `SM_Photo_Frame`, `SM_Painting_Small`
- Material: Materials with readable textures
- Scale: 1.0-1.5 for text readability

### 2.3 Material Quality
**Examination Materials Should Have:**
- High-resolution textures (2K minimum)
- Normal maps for surface detail
- Roughness maps for material variation
- Emissive for special effects (optional)
- No LOD transitions during examine

## 3. Interaction Parameters

### 3.1 Base Interaction Settings
**Category: Interaction**
- `InteractableId`: Unique name (e.g., `Examinable_Idol_01`)
- `InteractionPrompt`: "Examine [Object Name]"
- `bCanInteractMultipleTimes`: `true`
- `bRequireLineOfSight`: `true`

### 3.2 Camera Settings
**Category: Examine | Camera**
- `ExamineDistance`: `100.0` units from camera
  - Small objects: `50.0-80.0`
  - Medium objects: `100.0-150.0`
  - Large objects: `150.0-250.0`
- `MinZoomDistance`: `50.0` units
  - Close inspection: `30.0`
  - Standard: `50.0`
  - Limited zoom: `80.0`
- `MaxZoomDistance`: `200.0` units
  - Standard: `200.0`
  - Extended: `300.0`
  - Limited: `150.0`
- `ZoomSpeed`: `10.0` units/second
  - Slow: `5.0`
  - Normal: `10.0`
  - Fast: `20.0`

### 3.3 Rotation Settings
**Category: Examine | Rotation**
- `RotationSpeed`: `100.0` degrees/second
  - Slow/Precise: `50.0`
  - Normal: `100.0`
  - Fast: `200.0`
- `bAllowRotation`: `true`
  - Enable for 3D objects
  - Disable for flat items (photos)
- `bAllowZoom`: `true`
  - Enable for detailed inspection
  - Disable for fixed-distance viewing

### 3.4 Audio Configuration
**Category: Examine | Audio**
- `ExamineStartSound`: Sound when examination begins
  - Generic: `SFX_Examine_Start`
  - Pickup: `SFX_Examine_Pickup`
  - Mysterious: `SFX_Examine_Mystery`
- `ExamineEndSound`: Sound when examination ends
  - Generic: `SFX_Examine_End`
  - Putdown: `SFX_Examine_Putdown`

### 3.5 Behavior Settings
**Category: Examine | Behavior**
- `bDisablePlayerMovement`: `true`
  - Standard: `true` (freeze player)
  - Free examine: `false` (allow movement)
- `bHideOriginalMesh`: `true`
  - Standard: `true` (hide world object)
  - Duplicate: `false` (keep visible)

## 4. Specific Functionality

### 4.1 Examine Mode Controls
**Default C++ Controls:**
- **Mouse Movement**: Rotate object (Yaw/Pitch)
- **Mouse Wheel**: Zoom in/out
- **E Key / Right Click**: Exit examine mode

**Blueprint Input Binding:**
```
Event BeginPlay
├─ Enable Input (Player Controller)
└─ Bind Input Actions

Input Action: ExamineRotate
├─ Get Mouse Delta
├─ Call RotateObject(Yaw, Pitch)

Input Action: ExamineZoom
├─ Get Mouse Wheel Delta
├─ Call ZoomObject(Delta)

Input Action: ExamineExit
├─ Call StopExamining()
```

### 4.2 Custom Examine Camera
**Blueprint Implementation:**
```
Override: StartExamining
├─ Call Parent StartExamining
├─ Store Original Camera Settings
├─ Set Camera FOV (60.0)
├─ Set Camera Post Process
│  ├─ Depth of Field (Enabled)
│  ├─ Vignette (Subtle)
│  └─ Bloom (Enhanced)
└─ Fade In Examine UI

Override: StopExamining
├─ Call Parent StopExamining
├─ Restore Original Camera Settings
├─ Clear Post Process
└─ Fade Out Examine UI
```

### 4.3 Rotation Constraints
**Limit rotation axes:**
```
Override: RotateObject
├─ Get Input (Yaw, Pitch)
├─ Branch (Allow Yaw Rotation?)
│  ├─ True → Apply Yaw
│  └─ False → Yaw = 0
├─ Branch (Allow Pitch Rotation?)
│  ├─ True → Apply Pitch (Clamped -80 to 80)
│  └─ False → Pitch = 0
└─ Call Parent RotateObject
```

### 4.4 Zoom Constraints
**Prevent over-zooming:**
```
Override: ZoomObject
├─ Get Current Distance
├─ Calculate New Distance
├─ Clamp (MinZoomDistance, MaxZoomDistance)
└─ Call Parent ZoomObject
```

### 4.5 Evidence Integration
**Examine reveals evidence:**
```
Event OnExamineComplete
├─ Branch (Has Hidden Evidence?)
│  ├─ True:
│  │  ├─ Add Evidence to Inventory
│  │  ├─ Show Evidence Notification
│  │  ├─ Play Discovery Sound
│  │  └─ Update Quest Objective
│  └─ False → Continue
```

### 4.6 Interactive Hotspots
**Add clickable points of interest:**
```
Custom Event: OnHotspotClicked
├─ Get Hotspot ID
├─ Branch (Hotspot Type)
│  ├─ "Inscription":
│  │  ├─ Show Text Overlay
│  │  └─ Play Narration
│  ├─ "Damage":
│  │  ├─ Zoom to Location
│  │  └─ Show Tooltip
│  └─ "Secret":
│     ├─ Reveal Hidden Detail
│     └─ Add Evidence
```

## 5. Event Logic

### 5.1 OnInteract Event
**Default C++ Behavior:**
1. Check if can interact
2. Check if already being examined
3. Store original transform
4. Disable player movement (if enabled)
5. Hide original mesh (if enabled)
6. Create examine camera view
7. Play start sound
8. Enter examine state

**Blueprint Extension:**
```
Event OnInteract (Override)
├─ Call Parent OnInteract
├─ Show Examine Instructions UI
├─ Enable Examine Input Mode
├─ Start Background Blur Effect
└─ Publish Event "Examine.Started"
```

### 5.2 Examine State Events
**Blueprint Events:**
```
Event OnExamineStarted
├─ Pause Game Time (Optional)
├─ Show Examine UI
├─ Enable Examine Controls
└─ Start Ambient Sound Loop

Event OnExamineEnded
├─ Resume Game Time
├─ Hide Examine UI
├─ Disable Examine Controls
├─ Stop Ambient Sound
└─ Restore Player State

Event OnObjectRotated
├─ Update Rotation UI Indicator
└─ Check for Hidden Details

Event OnObjectZoomed
├─ Update Zoom UI Indicator
├─ Adjust Depth of Field
└─ Reveal Close-Up Details
```

### 5.3 EventBus Integration
**Automatic Events Published:**
- `Examine.Started` - When examination begins
- `Examine.Ended` - When examination ends
- `Examine.Rotated` - When object is rotated
- `Examine.Zoomed` - When zoom changes

**Custom Event Publishing:**
```
Event OnDetailDiscovered
├─ Get EventBus Subsystem
├─ Create Event Data
│  ├─ Add "ObjectId" → InteractableId
│  ├─ Add "DetailType" → Detail Type
│  └─ Add "DetailId" → Detail ID
└─ Publish Event ("Examine.DetailDiscovered")
```

### 5.4 SaveGame Integration
**Automatic Save/Load:**
- Examination count
- First examination timestamp

**Blueprint Extension:**
```
Override: SaveState
├─ Call Parent SaveState
├─ Add State ("HasBeenExamined", true)
├─ Add State ("DetailsDiscovered", DiscoveredDetails)
└─ Return State Map

Override: LoadState
├─ Call Parent LoadState
├─ Get State ("HasBeenExamined")
├─ Get State ("DetailsDiscovered")
└─ Update Object State
```

## 6. Testing Validation

### 6.1 Functional Tests
- [ ] Examine mode activates correctly
- [ ] Object rotates smoothly with mouse
- [ ] Zoom in/out works properly
- [ ] Exit examine returns to normal
- [ ] Player movement disabled during examine
- [ ] Original mesh hidden/shown correctly
- [ ] Camera distance appropriate
- [ ] Rotation speed feels responsive

### 6.2 Edge Cases
- [ ] Rapid enter/exit examine mode
- [ ] Examine during player movement
- [ ] Multiple examinables nearby
- [ ] Examine mode during level transition
- [ ] Save/load during examination
- [ ] Network replication (if multiplayer)
- [ ] Examine with controller input
- [ ] Rotation limits work correctly

### 6.3 Performance Validation
- [ ] No frame drops during rotation
- [ ] Smooth zoom transitions
- [ ] Camera updates at 60+ FPS
- [ ] No memory leaks on exit
- [ ] Post-process effects optimized
- [ ] Input response time < 16ms

### 6.4 Visual Quality
- [ ] Object lighting looks good
- [ ] Materials render correctly
- [ ] No LOD popping during examine
- [ ] Depth of field enhances focus
- [ ] UI elements clear and readable
- [ ] Rotation feels natural

## 7. Common Configurations

### 7.1 Small Artifact (Close Inspection)
```
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
```

### 7.2 Medium Object (Standard Examine)
```
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
```

### 7.3 Large Statue (Limited Rotation)
```
ExamineDistance: 180.0
MinZoomDistance: 100.0
MaxZoomDistance: 300.0
ZoomSpeed: 15.0
RotationSpeed: 60.0
bAllowRotation: true
bAllowZoom: true
bDisablePlayerMovement: true
bHideOriginalMesh: false
ExamineStartSound: SFX_Examine_Start
ExamineEndSound: SFX_Examine_End
```

### 7.4 Flat Photo (No Rotation)
```
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
```

### 7.5 Free Examine (No Movement Lock)
```
ExamineDistance: 120.0
MinZoomDistance: 60.0
MaxZoomDistance: 250.0
ZoomSpeed: 12.0
RotationSpeed: 120.0
bAllowRotation: true
bAllowZoom: true
bDisablePlayerMovement: false
bHideOriginalMesh: false
ExamineStartSound: SFX_Examine_Start
ExamineEndSound: SFX_Examine_End
```

## 8. Advanced Features

### 8.1 Multi-Part Examination
**Examine object with multiple components:**
```
Custom Event: SwitchExaminePart
├─ Get Current Part Index
├─ Increment Index
├─ Branch (Index < Part Count?)
│  ├─ True:
│  │  ├─ Hide Current Part Mesh
│  │  ├─ Show Next Part Mesh
│  │  ├─ Reset Rotation
│  │  └─ Play Transition Sound
│  └─ False:
│     ├─ Reset to First Part
│     └─ Loop
```

### 8.2 X-Ray Vision Mode
**Toggle between normal and x-ray view:**
```
Custom Event: ToggleXRayMode
├─ Branch (X-Ray Active?)
│  ├─ True:
│  │  ├─ Set Material (Normal Material)
│  │  └─ Set X-Ray Active (false)
│  └─ False:
│     ├─ Set Material (X-Ray Material)
│     ├─ Set X-Ray Active (true)
│     └─ Reveal Hidden Components
```

### 8.3 Time-Based Reveals
**Details appear after examining for duration:**
```
Event Tick (During Examine)
├─ Increment Examine Time
├─ Branch (Examine Time > 3.0?)
│  ├─ True:
│  │  ├─ Reveal Hidden Detail
│  │  ├─ Play Discovery Effect
│  │  └─ Add Evidence
│  └─ False → Continue
```

### 8.4 Comparison Mode
**Compare two objects side-by-side:**
```
Custom Event: StartComparisonMode
├─ Get Second Object
├─ Position Objects Side-by-Side
├─ Enable Synchronized Rotation
├─ Show Comparison UI
└─ Highlight Differences
```

### 8.5 Annotation System
**Add notes to examined objects:**
```
Custom Event: AddAnnotation
├─ Get Click Location on Mesh
├─ Create Annotation Widget
├─ Attach to Mesh Surface
├─ Save Annotation Data
└─ Show in Future Examinations
```

## 9. UI Integration

### 9.1 Examine HUD
**Display during examination:**
- Object name/title
- Rotation indicator
- Zoom level indicator
- Control hints (Mouse to rotate, Wheel to zoom, E to exit)
- Discovery progress (optional)

### 9.2 Detail Callouts
**Highlight points of interest:**
```
Widget: ExamineCallout
├─ Icon (Magnifying glass)
├─ Text (Detail description)
├─ Progress (Discovery percentage)
└─ Interaction prompt
```

## 10. Troubleshooting

### Issue: Object rotates incorrectly
**Solution:** Check RotationSpeed value and ensure bAllowRotation is true

### Issue: Camera too close/far
**Solution:** Adjust ExamineDistance to appropriate value for object size

### Issue: Player can still move during examine
**Solution:** Ensure bDisablePlayerMovement is true

### Issue: Original object still visible
**Solution:** Set bHideOriginalMesh to true

### Issue: Zoom doesn't work
**Solution:** Check bAllowZoom is true and Min/Max distances are valid

### Issue: Can't exit examine mode
**Solution:** Verify exit input binding is configured correctly

### Issue: Rotation feels sluggish
**Solution:** Increase RotationSpeed value (try 150-200)

### Issue: Object lighting looks wrong
**Solution:** Add dedicated examine light source or adjust post-process settings
