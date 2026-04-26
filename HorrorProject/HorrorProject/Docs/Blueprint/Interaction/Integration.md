# Interactive Object System Integration Guide

## Overview
Complete guide for integrating interactive objects with game systems: Evidence, Inventory, Objectives, EventBus, and SaveGame.

## Evidence System Integration

### Overview
Evidence system tracks collectible evidence items for investigation gameplay. Integrates with Pickup, Document, and Recorder interactables.

### C++ Evidence System Components
**Location:** `Source/HorrorProject/Player/Components/`
- `InventoryComponent.h/cpp` - Manages evidence collection
- Evidence metadata structure: `FHorrorEvidenceMetadata`

### Evidence Metadata Structure
```cpp
struct FHorrorEvidenceMetadata
{
    FName EvidenceId;           // Unique identifier
    FText EvidenceName;         // Display name
    FText Description;          // Evidence description
    FString Category;           // Evidence category
    bool bIsOptional;           // Optional evidence flag
    // Additional fields as needed
};
```

### Pickup → Evidence Integration

**Blueprint Implementation:**
```
Event OnInteract (Override)
├─ Call Parent OnInteract
├─ Branch (PickupType == Evidence?)
│  ├─ True:
│  │  ├─ Get Player Character
│  │  ├─ Get Inventory Component
│  │  ├─ Call AddEvidence(EvidenceMetadata)
│  │  ├─ Branch (Success?)
│  │  │  ├─ True:
│  │  │     ├─ Show Evidence Notification
│  │  │     ├─ Update Evidence Counter UI
│  │  │     ├─ Play Collection Sound
│  │  │     └─ Publish Event "Evidence.Collected"
│  │  │  └─ False:
│  │  │     ├─ Show Error Message
│  │  │     └─ Don't Destroy Pickup
│  └─ False → Continue
```

**Automatic C++ Behavior:**
- `PickupInteractable::OnInteract()` checks if `PickupType == Evidence`
- Automatically calls `InventoryComponent::AddEvidence()`
- Evidence metadata passed to inventory system
- Pickup destroyed on successful collection

**Configuration Checklist:**
- [ ] Set `PickupType` to `Evidence`
- [ ] Fill `EvidenceMetadata` completely
- [ ] Set unique `EvidenceId`
- [ ] Set `bIsOptional` correctly
- [ ] Assign `Category` for filtering

### Document → Evidence Integration

**Blueprint Implementation:**
```
Event OnDocumentRead
├─ Branch (bIsEvidence AND bAutoCollectAsEvidence?)
│  ├─ True:
│  │  ├─ Get Player Character
│  │  ├─ Get Inventory Component
│  │  ├─ Call AddEvidence(EvidenceMetadata)
│  │  ├─ Show Evidence Notification
│  │  ├─ Update Evidence Log UI
│  │  └─ Publish Event "Evidence.Document.Collected"
│  └─ False → Continue
```

**Automatic C++ Behavior:**
- `DocumentInteractable::OnInteract()` checks `bIsEvidence`
- If `bAutoCollectAsEvidence` is true, adds to evidence
- Document content stored in evidence log
- Player can review document later

**Configuration Checklist:**
- [ ] Set `bIsEvidence` to `true`
- [ ] Set `bAutoCollectAsEvidence` to `true`
- [ ] Fill `EvidenceMetadata` completely
- [ ] Ensure `DocumentContent` is complete
- [ ] Set `DocumentTitle` for evidence log

### Recorder → Evidence Integration

**Blueprint Implementation:**
```
Event OnAudioLogStarted
├─ Branch (bIsEvidence AND bAutoCollectAsEvidence?)
│  ├─ True:
│  │  ├─ Get Player Character
│  │  ├─ Get Inventory Component
│  │  ├─ Call AddEvidence(EvidenceMetadata)
│  │  ├─ Add Audio Transcript to Evidence
│  │  ├─ Show Evidence Notification
│  │  └─ Publish Event "Evidence.AudioLog.Collected"
│  └─ False → Continue
```

**Automatic C++ Behavior:**
- `RecorderInteractable::OnInteract()` checks `bIsEvidence`
- If `bAutoCollectAsEvidence` is true, adds to evidence
- Audio transcript stored for reference
- Player can replay audio from evidence log

**Configuration Checklist:**
- [ ] Set `bIsEvidence` to `true`
- [ ] Set `bAutoCollectAsEvidence` to `true`
- [ ] Fill `EvidenceMetadata` completely
- [ ] Provide complete `AudioLogTranscript`
- [ ] Set `AudioLogTitle` for evidence log

### Evidence Query Functions

**Blueprint Functions:**
```
// Check if evidence collected
Function: HasEvidence
├─ Input: EvidenceId (Name)
├─ Get Inventory Component
├─ Call HasEvidence(EvidenceId)
└─ Return: Boolean

// Get evidence count
Function: GetEvidenceCount
├─ Get Inventory Component
├─ Call GetEvidenceCount()
└─ Return: Integer

// Get evidence by category
Function: GetEvidenceByCategory
├─ Input: Category (String)
├─ Get Inventory Component
├─ Call GetEvidenceByCategory(Category)
└─ Return: Array of Evidence
```

### Evidence UI Integration

**Evidence Log Widget:**
```
Widget: EvidenceLog
├─ Evidence List (Scroll Box)
│  ├─ Evidence Entry Widget (per item)
│  │  ├─ Evidence Name
│  │  ├─ Evidence Description
│  │  ├─ Category Badge
│  │  ├─ Optional Flag
│  │  └─ View Details Button
├─ Category Filter (Dropdown)
├─ Search Bar
└─ Evidence Counter (X/Y Collected)

Event OnEvidenceSelected
├─ Get Evidence Details
├─ Show Evidence Detail View
├─ Display Content (Text/Image/Audio)
└─ Enable Review/Replay
```

## Inventory System Integration

### Overview
Inventory system manages player items: keys, tools, batteries, and evidence. Integrates primarily with Pickup interactables.

### C++ Inventory Component
**Location:** `Source/HorrorProject/Player/Components/InventoryComponent.h`

**Key Functions:**
- `AddItem(FName ItemId, int32 Count)` - Add item to inventory
- `RemoveItem(FName ItemId, int32 Count)` - Remove item
- `HasItem(FName ItemId)` - Check if item exists
- `GetItemCount(FName ItemId)` - Get item quantity
- `AddEvidence(FHorrorEvidenceMetadata)` - Add evidence

### Pickup → Inventory Integration

**Blueprint Implementation:**
```
Event OnInteract (Override)
├─ Call Parent OnInteract
├─ Get Player Character
├─ Get Inventory Component
├─ Branch (PickupType)
│  ├─ Key:
│  │  ├─ Call AddItem(ItemId, 1)
│  │  ├─ Show "Key Acquired" Message
│  │  └─ Highlight Locked Doors
│  ├─ Tool:
│  │  ├─ Call AddItem(ItemId, 1)
│  │  ├─ Show "Tool Acquired" Message
│  │  └─ Enable Tool Usage
│  ├─ Battery:
│  │  ├─ Call AddItem(ItemId, 1)
│  │  ├─ Add Battery Charge
│  │  └─ Update Battery UI
│  └─ Generic:
│     ├─ Call AddItem(ItemId, 1)
│     └─ Show Pickup Message
```

**Automatic C++ Behavior:**
- `PickupInteractable::TryAddToInventory()` handles inventory addition
- Returns true if successful, false if inventory full
- Pickup only destroyed if successfully added

**Configuration Checklist:**
- [ ] Set correct `PickupType`
- [ ] Set unique `ItemId` matching inventory system
- [ ] Verify `ItemId` exists in inventory data table
- [ ] Test inventory full scenario

### Key Item Usage

**Door Integration:**
```
// On Door Blueprint
Override: CanInteract
├─ Parent CanInteract → Result
├─ Branch (Is Locked?)
│  ├─ True:
│  │  ├─ Get Player Inventory Component
│  │  ├─ Call HasItem(RequiredKeyId)
│  │  ├─ Branch (Has Key?)
│  │  │  ├─ True:
│  │  │     ├─ Return True (Can Interact)
│  │  │     └─ Update Prompt "Unlock Door"
│  │  │  └─ False:
│  │  │     ├─ Return False
│  │  │     └─ Update Prompt "Locked (Need Key)"
│  └─ False → Return Result

Override: OnInteract
├─ Branch (Is Locked AND Has Key?)
│  ├─ True:
│  │  ├─ Get Inventory Component
│  │  ├─ Call RemoveItem(RequiredKeyId, 1)
│  │  ├─ SetLocked(false)
│  │  ├─ Show "Door Unlocked" Message
│  │  ├─ Play Unlock Sound
│  │  └─ Publish Event "Door.Unlocked"
│  └─ False → Continue
├─ Call Parent OnInteract
```

**Key Configuration:**
```
// On Pickup
PickupType: Key
ItemId: "Key_Basement"

// On Door
RequiredKeyId: "Key_Basement"
bStartLocked: true
```

### Tool Item Usage

**Flashlight Integration:**
```
// On Player Character
Variable: FlashlightComponent (Component Reference)
Variable: bHasFlashlight (Boolean) = false

Event OnToolAcquired
├─ Input: ItemId (Name)
├─ Branch (ItemId == "Tool_Flashlight"?)
│  ├─ True:
│  │  ├─ Set HasFlashlight (true)
│  │  ├─ Enable Flashlight Component
│  │  ├─ Show Tutorial "Press F to toggle flashlight"
│  │  └─ Bind Flashlight Input
│  └─ False → Check Other Tools
```

### Battery System Integration

**Battery Consumption:**
```
// On Flashlight Component
Variable: BatteryLevel (Float) = 100.0
Variable: BatteryDrainRate (Float) = 5.0

Event Tick (When Flashlight On)
├─ Drain Battery (DeltaTime * DrainRate)
├─ Update Battery UI
├─ Branch (Battery <= 0?)
│  ├─ True:
│  │  ├─ Turn Off Flashlight
│  │  ├─ Show "Battery Dead" Message
│  │  └─ Disable Flashlight
│  └─ False → Continue

Custom Event: UseBattery
├─ Get Inventory Component
├─ Call HasItem("Battery_AA")
├─ Branch (Has Battery?)
│  ├─ True:
│  │  ├─ Remove Item ("Battery_AA", 1)
│  │  ├─ Add Battery Level (50.0)
│  │  ├─ Show "+50% Battery" Message
│  │  └─ Play Battery Insert Sound
│  └─ False:
│     └─ Show "No Batteries" Message
```

### Inventory UI Integration

**Inventory Widget:**
```
Widget: InventoryUI
├─ Item Grid (Wrap Box)
│  ├─ Item Slot Widget (per item)
│  │  ├─ Item Icon
│  │  ├─ Item Name
│  │  ├─ Item Count
│  │  └─ Use/Equip Button
├─ Evidence Tab
├─ Tools Tab
├─ Keys Tab
└─ Weight/Capacity Display

Event OnItemUsed
├─ Get Item Type
├─ Execute Item Function
└─ Update UI
```

## Objective System Integration

### Overview
Objective system tracks player progress through story and side quests. Integrates with all interactable types.

### C++ Objective System
**Location:** `Source/HorrorProject/Game/` (assumed)

**Key Functions:**
- `UpdateObjective(FName ObjectiveId, int32 Progress)`
- `CompleteObjective(FName ObjectiveId)`
- `IsObjectiveActive(FName ObjectiveId)`
- `GetObjectiveProgress(FName ObjectiveId)`

### Door → Objective Integration

**Blueprint Implementation:**
```
Event OnDoorOpened
├─ Branch (Has Objective Tag?)
│  ├─ True:
│  │  ├─ Get Objective System
│  │  ├─ Update Objective (ObjectiveId, +1)
│  │  ├─ Check Objective Complete
│  │  └─ Publish Event "Objective.Updated"
│  └─ False → Continue

Example Objectives:
- "Escape the Basement" → Open basement door
- "Explore All Rooms" → Open all doors in area
- "Find the Exit" → Open final door
```

**Configuration:**
```
Variable: ObjectiveId (Name) = "Objective_EscapeBasement"
Variable: bUpdatesObjective (Boolean) = true

Event OnInteract
├─ Call Parent OnInteract
├─ Branch (Updates Objective?)
│  ├─ True → Update Objective System
│  └─ False → Continue
```

### Pickup → Objective Integration

**Blueprint Implementation:**
```
Event OnPickedUp
├─ Branch (PickupType == Evidence?)
│  ├─ True:
│  │  ├─ Get Objective System
│  │  ├─ Update Objective ("Collect_Evidence", +1)
│  │  ├─ Check Evidence Count
│  │  ├─ Branch (All Evidence Collected?)
│  │  │  ├─ True → Complete Objective
│  │  │  └─ False → Show Progress (5/10)
│  └─ False → Continue

Example Objectives:
- "Collect All Evidence" → Collect X evidence items
- "Find the Key" → Collect specific key
- "Gather Supplies" → Collect tools/batteries
```

### Document → Objective Integration

**Blueprint Implementation:**
```
Event OnDocumentRead
├─ Get Objective System
├─ Update Objective ("Read_Documents", +1)
├─ Branch (Specific Document?)
│  ├─ True:
│  │  ├─ Complete Objective ("Find_Victim_Letter")
│  │  ├─ Show Objective Complete UI
│  │  └─ Unlock Next Objective
│  └─ False → Continue

Example Objectives:
- "Read the Victim's Letter" → Read specific document
- "Uncover the Truth" → Read all story documents
- "Find Clues" → Read X documents
```

### Recorder → Objective Integration

**Blueprint Implementation:**
```
Event OnAudioLogCompleted
├─ Get Objective System
├─ Update Objective ("Listen_AudioLogs", +1)
├─ Branch (Critical Audio Log?)
│  ├─ True:
│  │  ├─ Complete Objective ("Hear_Victim_Message")
│  │  ├─ Trigger Story Event
│  │  └─ Show Objective Complete
│  └─ False → Continue

Example Objectives:
- "Listen to All Audio Logs" → Listen to X recordings
- "Hear the Victim's Message" → Listen to specific log
- "Gather Audio Evidence" → Collect audio evidence
```

### Switch → Objective Integration

**Blueprint Implementation:**
```
Event OnSwitchActivated
├─ Get Objective System
├─ Branch (Switch Type)
│  ├─ Puzzle Switch:
│  │  ├─ Update Objective ("Solve_Puzzle", +1)
│  │  └─ Check Puzzle Complete
│  ├─ Power Switch:
│  │  ├─ Complete Objective ("Restore_Power")
│  │  └─ Unlock New Area
│  └─ Story Switch:
│     ├─ Complete Objective ("Activate_System")
│     └─ Trigger Cutscene

Example Objectives:
- "Restore Power" → Activate breaker switches
- "Solve the Puzzle" → Activate switches in correct order
- "Open the Door" → Activate door button
```

### Objective UI Integration

**Objective Widget:**
```
Widget: ObjectiveTracker
├─ Active Objectives List
│  ├─ Objective Entry Widget
│  │  ├─ Objective Title
│  │  ├─ Objective Description
│  │  ├─ Progress Bar (X/Y)
│  │  └─ Optional Marker
├─ Objective Complete Notification
└─ Objective Updated Animation

Event OnObjectiveUpdated
├─ Update Progress Display
├─ Play Update Animation
└─ Show Progress Notification

Event OnObjectiveCompleted
├─ Play Complete Animation
├─ Show Complete Notification
├─ Play Success Sound
└─ Remove from Active List
```

## EventBus Integration

### Overview
EventBus system enables decoupled communication between game systems. All interactables publish events automatically.

### C++ EventBus System
**Location:** `Source/HorrorProject/Core/HorrorEventBusSubsystem.h`

**Key Functions:**
- `PublishEvent(FGameplayTag EventTag, TMap<FString, FString> EventData)`
- `SubscribeToEvent(FGameplayTag EventTag, Delegate Callback)`
- `UnsubscribeFromEvent(FGameplayTag EventTag, Delegate Callback)`

### Automatic Event Publishing

**All Interactables Publish:**
```cpp
// In BaseInteractable::OnInteract()
PublishInteractionEvent(InteractionEventTag, InteractionStateTag);

// Event data includes:
- InteractableId
- InteractableType
- InstigatorActor
- Timestamp
- Custom state data
```

### Event Tag Hierarchy

**Standard Event Tags:**
```
Interaction.Door.Opened
Interaction.Door.Closed
Interaction.Door.Locked
Interaction.Door.Unlocked

Interaction.Pickup.Collected
Interaction.Evidence.Collected
Interaction.Key.Acquired
Interaction.Tool.Acquired

Interaction.Examine.Started
Interaction.Examine.Ended
Interaction.Examine.DetailDiscovered

Interaction.Document.Opened
Interaction.Document.Closed
Interaction.Document.Read

Interaction.AudioLog.Started
Interaction.AudioLog.Stopped
Interaction.AudioLog.Completed

Interaction.Switch.Activated
Interaction.Switch.Deactivated
Interaction.Switch.Toggled
```

### Subscribing to Events

**Blueprint Implementation:**
```
Event BeginPlay
├─ Get EventBus Subsystem
├─ Subscribe to Event ("Interaction.Door.Opened")
├─ Bind to Custom Event (OnDoorOpenedEvent)

Custom Event: OnDoorOpenedEvent
├─ Input: EventData (Map of String to String)
├─ Get "InteractableId" from EventData
├─ Branch (Is Target Door?)
│  ├─ True:
│  │  ├─ Execute Response Logic
│  │  ├─ Spawn Enemies
│  │  ├─ Play Sound
│  │  └─ Update Quest
│  └─ False → Ignore
```

### Cross-System Communication

**Example: Door Opens → Spawn Enemies**
```
// On Enemy Spawner
Event BeginPlay
├─ Get EventBus
├─ Subscribe to "Interaction.Door.Opened"
└─ Bind to OnDoorOpened

Event OnDoorOpened
├─ Get EventData
├─ Get "InteractableId"
├─ Branch (InteractableId == TriggerDoorId?)
│  ├─ True:
│  │  ├─ Spawn Enemies
│  │  ├─ Play Alarm Sound
│  │  └─ Start Chase Sequence
│  └─ False → Ignore
```

**Example: Evidence Collected → Update UI**
```
// On Evidence UI Widget
Event Construct
├─ Get EventBus
├─ Subscribe to "Interaction.Evidence.Collected"
└─ Bind to OnEvidenceCollected

Event OnEvidenceCollected
├─ Get EventData
├─ Get "EvidenceId"
├─ Update Evidence Counter
├─ Show Collection Notification
└─ Play Collection Animation
```

### Publishing Custom Events

**Blueprint Implementation:**
```
Custom Event: PublishCustomEvent
├─ Get EventBus Subsystem
├─ Create Event Data Map
│  ├─ Add "CustomKey1" → Value1
│  ├─ Add "CustomKey2" → Value2
│  └─ Add "Timestamp" → Current Time
├─ Create Event Tag ("Game.Custom.Event")
└─ Call PublishEvent(EventTag, EventData)
```

## SaveGame Integration

### Overview
SaveGame system persists interactable states across sessions. All interactables support save/load automatically.

### C++ SaveGame System
**Location:** `Source/HorrorProject/Save/` (assumed)

**Key Functions:**
- `SaveInteractableState(FName InteractableId, TMap<FName, bool> State)`
- `LoadInteractableState(FName InteractableId)` → TMap<FName, bool>
- `SaveGame()` - Save all game state
- `LoadGame()` - Load all game state

### Automatic State Saving

**All Interactables Save:**
```cpp
// In BaseInteractable::SaveState()
OutStateMap.Add("HasBeenInteracted", bHasBeenInteracted);
OutStateMap.Add("LastInteractionTime", LastInteractionTime);

// Subclasses extend:
// Door: Door state, lock state
// Pickup: Collected state
// Document: Read state
// Recorder: Listened state
// Switch: Switch state
```

### Door State Persistence

**Saved State:**
- Door open/closed
- Door locked/unlocked
- Interaction count

**Blueprint Extension:**
```
Override: SaveState
├─ Call Parent SaveState
├─ Add State ("IsOpen", IsOpen())
├─ Add State ("IsLocked", IsLocked())
├─ Add State ("OpenProgress", CurrentRotationAlpha)
└─ Return State Map

Override: LoadState
├─ Call Parent LoadState
├─ Get State ("IsOpen")
├─ Get State ("IsLocked")
├─ Branch (Was Open?)
│  ├─ True → Set Door to Open Position
│  └─ False → Set Door to Closed Position
├─ Branch (Was Locked?)
│  ├─ True → SetLocked(true)
│  └─ False → SetLocked(false)
```

### Pickup State Persistence

**Saved State:**
- Collected flag (prevents respawn)

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
│  ├─ True:
│  │  ├─ Destroy Self
│  │  └─ Prevent Respawn
│  └─ False → Continue Normal Behavior
```

### Document/Recorder State Persistence

**Saved State:**
- Read/listened flag
- Read/listen count

**Blueprint Extension:**
```
Override: SaveState
├─ Call Parent SaveState
├─ Add State ("HasBeenRead", bHasBeenRead)
├─ Add State ("ReadCount", ReadCount)
└─ Return State Map

Override: LoadState
├─ Call Parent LoadState
├─ Get State ("HasBeenRead")
├─ Get State ("ReadCount")
├─ Update Visual State (if needed)
```

### Switch State Persistence

**Saved State:**
- Switch on/off state
- One-time activation flag

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
├─ SetSwitchState(IsOn)
├─ Update Connected Actors
└─ Update Visual State
```

### SaveGame Triggers

**Auto-Save Points:**
- After collecting evidence
- After completing objectives
- After opening critical doors
- On level transition
- On checkpoint reached

**Manual Save:**
```
Custom Event: TriggerSave
├─ Get SaveGame Subsystem
├─ Call SaveGame()
├─ Show "Game Saved" Message
└─ Play Save Sound
```

## Integration Checklist

### For Each Interactable Type

**Evidence Integration:**
- [ ] Set `bIsEvidence` if collectible
- [ ] Fill `EvidenceMetadata` completely
- [ ] Test evidence collection
- [ ] Verify evidence appears in log
- [ ] Check evidence counter updates

**Inventory Integration:**
- [ ] Set correct `ItemId`
- [ ] Verify item exists in inventory data
- [ ] Test inventory addition
- [ ] Check inventory UI updates
- [ ] Test inventory full scenario

**Objective Integration:**
- [ ] Identify related objectives
- [ ] Implement objective update logic
- [ ] Test objective progress
- [ ] Verify objective completion
- [ ] Check objective UI updates

**EventBus Integration:**
- [ ] Configure `InteractionEventTag`
- [ ] Test event publishing
- [ ] Verify event data correct
- [ ] Test event subscribers
- [ ] Check cross-system communication

**SaveGame Integration:**
- [ ] Set unique `InteractableId`
- [ ] Test state saving
- [ ] Test state loading
- [ ] Verify state persistence
- [ ] Check respawn prevention

## Troubleshooting

### Evidence not collecting
**Solution:** Check `bIsEvidence` is true, `EvidenceMetadata` is filled, and `InventoryComponent` exists

### Inventory item not adding
**Solution:** Verify `ItemId` matches inventory data table exactly

### Objective not updating
**Solution:** Check objective system is active and `ObjectiveId` is correct

### Events not firing
**Solution:** Verify `InteractionEventTag` is set and EventBus subsystem exists

### State not saving
**Solution:** Ensure `InteractableId` is unique and not NAME_None

### Pickup respawns after load
**Solution:** Check `SaveState` adds "WasCollected" and `LoadState` destroys actor

### Door state incorrect after load
**Solution:** Verify `LoadState` applies both position and lock state correctly
