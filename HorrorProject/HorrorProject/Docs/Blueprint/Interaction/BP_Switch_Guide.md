# Blueprint Switch Implementation Guide

## Overview
Complete guide for creating switch interactables using Blueprint, inheriting from `ASwitchInteractable` C++ class.

## 1. Blueprint Creation Steps

### 1.1 Create Blueprint Class
1. Content Browser → Right-click → Blueprint Class
2. Search for `SwitchInteractable` parent class
3. Name: `BP_Switch_[Type]` (e.g., `BP_Switch_Light`, `BP_Switch_Door`)
4. Location: `Content/Blueprints/Interaction/Switches/`

### 1.2 Component Structure
Default components from C++ class:
- **SwitchBase** (StaticMeshComponent) - Base/mounting plate
- **SwitchHandle** (StaticMeshComponent) - Movable switch/button
- **InteractionVolume** (BoxComponent) - Interaction trigger

### 1.3 Collision Configuration
**InteractionVolume Settings:**
- Collision Preset: `OverlapAllDynamic`
- Generate Overlap Events: `true`
- Box Extent: `X=60, Y=60, Z=60` (adjust per switch size)

**SwitchBase/Handle Collision:**
- Collision Preset: `BlockAll`
- Simulate Physics: `false`

## 2. Mesh and Material Configuration

### 2.1 Switch Base Setup
1. Select `SwitchBase` component
2. Static Mesh: Choose base/plate mesh
3. Materials: Apply base material
4. Transform:
   - Location: `(0,0,0)`
   - Rotation: Facing interaction direction
   - Scale: Appropriate size

### 2.2 Switch Handle Setup
1. Select `SwitchHandle` component
2. Static Mesh: Choose handle/lever/button mesh
3. Materials: Apply handle material
4. Transform:
   - Location: Relative to base
   - Rotation: Off position rotation
   - **CRITICAL**: Pivot must be at rotation point

### 2.3 Recommended Assets by Type

**Wall Switch (Toggle):**
- Base: `SM_Switch_Plate_Wall`
- Handle: `SM_Switch_Toggle`
- Material: `M_Plastic_Switch`, `M_Metal_Plate`

**Lever Switch:**
- Base: `SM_Lever_Base`
- Handle: `SM_Lever_Handle`
- Material: `M_Metal_Industrial`, `M_Rubber_Grip`

**Push Button:**
- Base: `SM_Button_Housing`
- Handle: `SM_Button_Cap`
- Material: `M_Plastic_Button`, `M_Metal_Housing`

**Valve Wheel:**
- Base: `SM_Valve_Base`
- Handle: `SM_Valve_Wheel`
- Material: `M_Metal_Rusty`, `M_Metal_Industrial`

**Breaker Switch:**
- Base: `SM_Breaker_Panel`
- Handle: `SM_Breaker_Switch`
- Material: `M_Metal_Electrical`, `M_Plastic_Breaker`

### 2.4 Visual Feedback
**LED Indicator Setup:**
- Add material parameter for LED color
- Off state: Red or no glow
- On state: Green glow
- Emissive intensity: 2.0-5.0

## 3. Interaction Parameters

### 3.1 Base Interaction Settings
**Category: Interaction**
- `InteractableId`: Unique name (e.g., `Switch_Basement_Lights`)
- `InteractionPrompt`: "Flip Switch" / "Press Button"
- `bCanInteractMultipleTimes`: `true`
- `bRequireLineOfSight`: `true`

### 3.2 Switch Type Settings
**Category: Switch | Type**
- `SwitchType`: Select from enum
  - `Toggle` - Stays in position, can be switched on/off
  - `Momentary` - Returns to off after duration
  - `OneTime` - Can only be activated once

### 3.3 Behavior Settings
**Category: Switch | Behavior**
- `bStartOn`: `false` (initial state)
  - `true`: Switch starts in ON position
  - `false`: Switch starts in OFF position
- `MomentaryDuration`: `1.0` seconds (for Momentary type)
  - Short press: `0.5`
  - Normal: `1.0`
  - Long hold: `3.0`

### 3.4 Connection Settings
**Category: Switch | Connections**
- `ConnectedActors`: Array of actors to control
  - Add target actors (lights, doors, etc.)
  - Multiple actors can be controlled
- `ActivationFunctionName`: `"OnSwitchActivated"`
  - Function name to call on connected actors
  - Must exist on target actor

### 3.5 Audio Configuration
**Category: Switch | Audio**
- `ActivateSound`: Sound when switched on
  - Toggle: `SFX_Switch_On`
  - Button: `SFX_Button_Press`
  - Lever: `SFX_Lever_Pull`
- `DeactivateSound`: Sound when switched off
  - Toggle: `SFX_Switch_Off`
  - Button: `SFX_Button_Release`
  - Lever: `SFX_Lever_Return`

### 3.6 Visual Animation Settings
**Category: Switch | Visual**
- `OnRotation`: Rotation when ON (e.g., `(0, 0, 45)`)
  - Toggle up: `(0, 0, 45)`
  - Toggle down: `(0, 0, -45)`
  - Lever pulled: `(45, 0, 0)`
- `OffRotation`: Rotation when OFF (e.g., `(0, 0, -45)`)
- `AnimationSpeed`: `5.0` (interpolation speed)
  - Slow: `2.0`
  - Normal: `5.0`
  - Fast: `10.0`

## 4. Specific Functionality

### 4.1 Toggle Switch
**For SwitchType = Toggle:**
- First interaction: Switch ON
- Second interaction: Switch OFF
- State persists until changed
- Saves state across sessions

**Configuration:**
```
SwitchType: Toggle
bStartOn: false
OnRotation: (0, 0, 45)
OffRotation: (0, 0, -45)
AnimationSpeed: 5.0
```

### 4.2 Momentary Button
**For SwitchType = Momentary:**
- Interaction: Switch ON
- After duration: Automatically switch OFF
- Used for temporary activation
- Doors, elevators, timed events

**Configuration:**
```
SwitchType: Momentary
MomentaryDuration: 2.0
OnRotation: (0, 0, -10)
OffRotation: (0, 0, 0)
AnimationSpeed: 10.0
```

### 4.3 One-Time Trigger
**For SwitchType = OneTime:**
- Can only be activated once
- Cannot be deactivated
- Used for story triggers
- Permanent state change

**Configuration:**
```
SwitchType: OneTime
bStartOn: false
OnRotation: (0, 0, 90)
OffRotation: (0, 0, 0)
AnimationSpeed: 3.0
```

### 4.4 Connected Actor Control
**Automatic behavior:**
1. Switch activated
2. For each connected actor:
   - Call function with name `ActivationFunctionName`
   - Pass switch reference and state (on/off)
3. Connected actors respond to activation

**Target Actor Implementation:**
```
// On target actor (e.g., Light)
Custom Event: OnSwitchActivated
├─ Input: Switch (Base Interactable)
├─ Input: bIsOn (Boolean)
├─ Branch (Is On?)
│  ├─ True:
│  │  ├─ Turn Light On
│  │  └─ Play Light On Sound
│  └─ False:
│     ├─ Turn Light Off
│     └─ Play Light Off Sound
```

### 4.5 Multi-Switch Logic
**Multiple switches control one target:**
```
Variable: RequiredSwitchCount (Integer) = 3
Variable: ActivatedSwitches (Array of Switch)

Custom Event: OnSwitchActivated
├─ Add Switch to Activated Array
├─ Branch (Activated Count >= Required?)
│  ├─ True:
│  │  ├─ Activate Target System
│  │  ├─ Play Success Sound
│  │  └─ Show Success Message
│  └─ False:
│     ├─ Show Progress (2/3 Switches)
│     └─ Play Partial Sound
```

### 4.6 Conditional Activation
**Require specific conditions:**
```
Override: CanInteract
├─ Parent CanInteract → Result
├─ Branch (Requires Power?)
│  ├─ True:
│  │  ├─ Check Power System Active
│  │  ├─ Branch (Has Power?)
│  │  │  ├─ True → Return True
│  │  │  └─ False:
│  │  │     ├─ Show Message "No Power"
│  │  │     └─ Return False
│  └─ False → Return Result
```

## 5. Event Logic

### 5.1 OnInteract Event
**Default C++ Behavior:**
1. Check if can interact
2. Check switch type:
   - Toggle: Switch to opposite state
   - Momentary: Switch ON, start timer
   - OneTime: Switch ON if not already
3. Update visual rotation
4. Play appropriate sound
5. Trigger connected actors
6. Publish EventBus event

**Blueprint Extension:**
```
Event OnInteract (Override)
├─ Call Parent OnInteract
├─ Branch (Switch Type)
│  ├─ Toggle:
│  │  ├─ Update LED Color
│  │  └─ Show State Change Effect
│  ├─ Momentary:
│  │  ├─ Show Timer UI
│  │  └─ Start Countdown Effect
│  └─ OneTime:
│     ├─ Lock Switch
│     └─ Show Permanent Activation
└─ Publish Custom Event
```

### 5.2 State Change Events
**Blueprint Events:**
```
Event OnSwitchActivated
├─ Set Switch State (On)
├─ Update Visual Feedback
├─ Trigger Connected Actors
├─ Update Quest Objectives
└─ Publish Event "Switch.Activated"

Event OnSwitchDeactivated
├─ Set Switch State (Off)
├─ Update Visual Feedback
├─ Trigger Connected Actors
└─ Publish Event "Switch.Deactivated"

Event OnMomentaryTimerExpired
├─ Automatically Deactivate
├─ Play Return Animation
├─ Play Deactivate Sound
└─ Notify Connected Actors
```

### 5.3 Connected Actor Events
**Delegate: OnSwitchActivated**
```
// Bind to delegate in connected actor
Event BeginPlay
├─ Get Switch Reference
├─ Bind to OnSwitchActivated Delegate
└─ Set Initial State

Event OnSwitchActivated (Delegate)
├─ Input: Switch (Base Interactable)
├─ Input: bIsOn (Boolean)
├─ Execute Response Logic
└─ Update Actor State
```

### 5.4 EventBus Integration
**Automatic Events Published:**
- `Switch.Activated` - When switch turns ON
- `Switch.Deactivated` - When switch turns OFF
- `Switch.Toggled` - When switch state changes
- `Switch.OneTimeTriggered` - When one-time switch activated

**Custom Event Publishing:**
```
Event OnSwitchStateChanged
├─ Get EventBus Subsystem
├─ Create Event Data
│  ├─ Add "SwitchId" → InteractableId
│  ├─ Add "IsOn" → bIsSwitchOn
│  ├─ Add "SwitchType" → SwitchType
│  └─ Add "ConnectedActorCount" → ConnectedActors.Length
└─ Publish Event ("Switch.StateChanged")
```

### 5.5 SaveGame Integration
**Automatic Save/Load:**
- Switch state (on/off)
- One-time activation state
- Interaction count

**Blueprint Extension:**
```
Override: SaveState
├─ Call Parent SaveState
├─ Add State ("IsOn", bIsSwitchOn)
├─ Add State ("HasBeenActivated", bHasBeenActivated)
└─ Return State Map

Override: LoadState
├─ Call Parent LoadState
├─ Get State ("IsOn")
├─ Get State ("HasBeenActivated")
├─ Apply Switch State
└─ Update Connected Actors
```

## 6. Testing Validation

### 6.1 Functional Tests
- [ ] Switch activates correctly
- [ ] Visual animation smooth
- [ ] Audio plays at right time
- [ ] Connected actors respond
- [ ] Toggle switches on/off properly
- [ ] Momentary timer works correctly
- [ ] One-time switch locks after use
- [ ] State saves/loads correctly

### 6.2 Edge Cases
- [ ] Rapid interaction spam
- [ ] Multiple switches activated simultaneously
- [ ] Connected actor is null/destroyed
- [ ] Switch during level transition
- [ ] Save/load during momentary timer
- [ ] Network replication (if multiplayer)
- [ ] Switch with no connected actors
- [ ] Activation function doesn't exist on target

### 6.3 Performance Validation
- [ ] Animation interpolation smooth
- [ ] No tick overhead when idle
- [ ] Audio doesn't overlap
- [ ] Connected actor calls efficient
- [ ] No memory leaks

### 6.4 Integration Tests
- [ ] EventBus events fire correctly
- [ ] Quest objectives update
- [ ] Connected lights turn on/off
- [ ] Connected doors open/close
- [ ] Multiple switches work together
- [ ] Power system integration

## 7. Common Configurations

### 7.1 Basic Light Switch
```
SwitchType: Toggle
bStartOn: false
ConnectedActors: [BP_Light_Ceiling_01, BP_Light_Ceiling_02]
ActivationFunctionName: "OnSwitchActivated"
OnRotation: (0, 0, 45)
OffRotation: (0, 0, -45)
AnimationSpeed: 5.0
ActivateSound: SFX_Switch_On
DeactivateSound: SFX_Switch_Off
```

### 7.2 Door Button (Momentary)
```
SwitchType: Momentary
MomentaryDuration: 3.0
ConnectedActors: [BP_Door_Security_01]
ActivationFunctionName: "OnSwitchActivated"
OnRotation: (0, 0, -5)
OffRotation: (0, 0, 0)
AnimationSpeed: 10.0
ActivateSound: SFX_Button_Press
DeactivateSound: SFX_Button_Release
```

### 7.3 Emergency Lever (One-Time)
```
SwitchType: OneTime
bStartOn: false
ConnectedActors: [BP_AlarmSystem, BP_EmergencyLights]
ActivationFunctionName: "OnEmergencyActivated"
OnRotation: (45, 0, 0)
OffRotation: (0, 0, 0)
AnimationSpeed: 3.0
ActivateSound: SFX_Lever_Pull_Heavy
```

### 7.4 Breaker Switch
```
SwitchType: Toggle
bStartOn: true
ConnectedActors: [BP_PowerSystem_Basement]
ActivationFunctionName: "OnPowerToggled"
OnRotation: (0, 0, 30)
OffRotation: (0, 0, -30)
AnimationSpeed: 4.0
ActivateSound: SFX_Breaker_On
DeactivateSound: SFX_Breaker_Off
```

### 7.5 Puzzle Switch (Multi-Switch)
```
SwitchType: Toggle
bStartOn: false
ConnectedActors: [BP_PuzzleController]
ActivationFunctionName: "OnPuzzleSwitchToggled"
OnRotation: (0, 0, 90)
OffRotation: (0, 0, 0)
AnimationSpeed: 5.0
ActivateSound: SFX_Switch_Puzzle_On
DeactivateSound: SFX_Switch_Puzzle_Off
```

## 8. Advanced Features

### 8.1 Timed Sequence Switch
**Activate multiple targets in sequence:**
```
Variable: SequenceTargets (Array of Actor)
Variable: SequenceDelays (Array of Float)
Variable: CurrentSequenceIndex (Integer) = 0

Event OnSwitchActivated
├─ Start Sequence Timer
└─ Activate First Target

Custom Event: ActivateNextInSequence
├─ Branch (Index < Target Count?)
│  ├─ True:
│  │  ├─ Activate Target[Index]
│  │  ├─ Increment Index
│  │  ├─ Set Timer (Delay[Index])
│  │  └─ Call Self (Recursive)
│  └─ False:
│     ├─ Reset Index
│     └─ Sequence Complete
```

### 8.2 Power Requirement
**Switch requires power to function:**
```
Variable: RequiresPower (Boolean) = true
Variable: PowerSource (Actor Reference)

Override: CanInteract
├─ Parent CanInteract → Result
├─ Branch (Requires Power?)
│  ├─ True:
│  │  ├─ Get Power Source
│  │  ├─ Check Power Active
│  │  ├─ Branch (Has Power?)
│  │  │  ├─ True → Return True
│  │  │  └─ False:
│  │  │     ├─ Show "No Power" Message
│  │  │     ├─ Play Error Sound
│  │  │     └─ Return False
│  └─ False → Return Result
```

### 8.3 Combination Lock Switch
**Multiple switches must be in correct positions:**
```
Variable: CorrectCombination (Array of Boolean)
Variable: CurrentCombination (Array of Boolean)

Event OnAnySwitchToggled
├─ Update Current Combination
├─ Check Against Correct Combination
├─ Branch (Matches?)
│  ├─ True:
│  │  ├─ Unlock Target
│  │  ├─ Play Success Sound
│  │  ├─ Show Success Effect
│  │  └─ Disable All Switches
│  └─ False:
│     ├─ Show Progress Indicator
│     └─ Continue
```

### 8.4 Delayed Activation
**Switch activates after delay:**
```
Variable: ActivationDelay (Float) = 3.0
Variable: bIsCharging (Boolean) = false

Event OnInteract
├─ Branch (Is Charging?)
│  ├─ True → Ignore
│  └─ False:
│     ├─ Set Charging (true)
│     ├─ Show Charging Effect
│     ├─ Play Charging Sound
│     ├─ Set Timer (ActivationDelay)
│     └─ Start Progress Bar

Custom Event: OnDelayComplete
├─ Set Charging (false)
├─ Activate Switch
├─ Trigger Connected Actors
└─ Play Activation Sound
```

### 8.5 Overload Protection
**Switch trips if overloaded:**
```
Variable: MaxConnectedLoad (Float) = 100.0
Variable: CurrentLoad (Float) = 0.0
Variable: bIsTripped (Boolean) = false

Event OnSwitchActivated
├─ Calculate Total Load
├─ Branch (Load > Max Load?)
│  ├─ True:
│  │  ├─ Trip Switch
│  │  ├─ Play Spark Effect
│  │  ├─ Play Trip Sound
│  │  ├─ Disable Switch
│  │  └─ Show "Overload" Message
│  └─ False:
│     ├─ Normal Activation
│     └─ Update Load Display

Custom Event: ResetSwitch
├─ Set Tripped (false)
├─ Enable Switch
└─ Reset Visual State
```

### 8.6 Remote Switch Control
**Switch can be controlled remotely:**
```
Variable: bAllowRemoteControl (Boolean) = true
Variable: RemoteControlId (Name)

Custom Event: RemoteActivate
├─ Input: ControlId (Name)
├─ Branch (Allow Remote AND ID Matches?)
│  ├─ True:
│  │  ├─ Activate Switch
│  │  ├─ Play Remote Activation Effect
│  │  └─ Publish Event "Switch.RemoteActivated"
│  └─ False → Ignore

Custom Event: RemoteDeactivate
├─ Similar logic for deactivation
```

## 9. Connected Actor Examples

### 9.1 Light Actor Response
```
// On BP_Light actor
Custom Event: OnSwitchActivated
├─ Input: Switch (Base Interactable)
├─ Input: bIsOn (Boolean)
├─ Branch (Is On?)
│  ├─ True:
│  │  ├─ Set Light Visible (true)
│  │  ├─ Set Light Intensity (1000.0)
│  │  └─ Play Light Flicker On
│  └─ False:
│     ├─ Set Light Visible (false)
│     ├─ Set Light Intensity (0.0)
│     └─ Play Light Fade Out
```

### 9.2 Door Actor Response
```
// On BP_Door actor
Custom Event: OnSwitchActivated
├─ Input: Switch (Base Interactable)
├─ Input: bIsOn (Boolean)
├─ Branch (Is On?)
│  ├─ True:
│  │  ├─ Unlock Door
│  │  ├─ Open Door
│  │  └─ Play Unlock Sound
│  └─ False:
│     ├─ Close Door
│     ├─ Lock Door
│     └─ Play Lock Sound
```

### 9.3 Alarm System Response
```
// On BP_AlarmSystem actor
Custom Event: OnSwitchActivated
├─ Input: Switch (Base Interactable)
├─ Input: bIsOn (Boolean)
├─ Branch (Is On?)
│  ├─ True:
│  │  ├─ Activate Alarm
│  │  ├─ Flash Lights
│  │  ├─ Play Siren
│  │  └─ Spawn Enemies
│  └─ False:
│     ├─ Deactivate Alarm
│     ├─ Stop Lights
│     └─ Stop Siren
```

## 10. Troubleshooting

### Issue: Switch doesn't animate
**Solution:** Check OnRotation/OffRotation are different and AnimationSpeed > 0

### Issue: Connected actors don't respond
**Solution:** Verify actors are in ConnectedActors array and have the activation function

### Issue: Momentary timer doesn't work
**Solution:** Ensure SwitchType is Momentary and MomentaryDuration > 0

### Issue: Switch state doesn't save
**Solution:** Verify InteractableId is unique and not NAME_None

### Issue: Audio plays multiple times
**Solution:** Check for duplicate switch activations, add cooldown

### Issue: One-time switch can be reactivated
**Solution:** Ensure SwitchType is OneTime, not Toggle

### Issue: Visual rotation wrong
**Solution:** Adjust SwitchHandle pivot point and rotation values

### Issue: Switch activates through walls
**Solution:** Check InteractionVolume size and bRequireLineOfSight setting

### Issue: Multiple switches conflict
**Solution:** Ensure each switch has unique InteractableId and separate connected actors
