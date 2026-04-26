# Objective System Blueprint Implementation Guide

## Overview

The Objective system provides node-based objective management:
- **AHorrorObjectiveNode** - Base objective node class
- **AHorrorNavigationNode** - Navigation objectives (reach location)
- **AHorrorInteractionNode** - Interaction objectives (interact with object)
- **AHorrorEncounterNode** - Encounter objectives (survive/defeat threat)

All nodes support prerequisite chains, event publishing, and Blueprint customization.

## C++ Foundation

### HorrorObjectiveNode

**Purpose**: Base class for all objective nodes.

**Key Features**:
- 3 node types: Navigation, Interaction, Encounter
- 4 states: Inactive, Active, Completed, Failed
- Prerequisite system
- Event bus integration
- Trigger volume support
- Save/load support

**Node Types**:
```cpp
enum class EHorrorObjectiveNodeType : uint8
{
    Navigation,   // Go to location
    Interaction,  // Interact with object
    Encounter     // Survive/defeat threat
};
```

**Node States**:
```cpp
enum class EHorrorObjectiveNodeState : uint8
{
    Inactive,   // Not yet available
    Active,     // Currently active
    Completed,  // Successfully completed
    Failed      // Failed (optional)
};
```

## Blueprint Implementation

### Step 1: Create Base Objective Node

**Create**: `BP_ObjectiveNode_Base`

**Parent Class**: HorrorObjectiveNode

**Purpose**: Base class for all objective nodes.

**Components**:
- BoxComponent (inherited, name: `TriggerVolume`)
- BillboardComponent (name: `EditorIcon`) - Editor visualization only

**Variables**:
- `bShowDebugInfo` (bool, default: false)
- `DebugColor` (LinearColor, default: Yellow)
- `CompletionReward` (int32, default: 0) - Optional score/XP

**Event Graph**:
```
Event BeginPlay
  → Branch (bAutoActivateOnBeginPlay)
    True: Activate Node
  → Branch (bShowDebugInfo)
    True: Draw Debug Info

BP_OnNodeActivated (BlueprintNativeEvent)
  → Show Objective Toast (NodeTitle, NodeDescription)
  → Publish Event (EventTag, StateTag)
  → Play Activation Sound
  → Update Objective UI

BP_OnNodeCompleted (BlueprintNativeEvent)
  → Show Objective Toast ("Objective Complete: " + NodeTitle)
  → Publish Event (EventTag, StateTag)
  → Play Completion Sound
  → Award Completion Reward
  → Activate Next Objectives

BP_OnNodeFailed (BlueprintNativeEvent)
  → Show Objective Toast ("Objective Failed: " + NodeTitle)
  → Publish Event (EventTag, StateTag)
  → Play Failure Sound
```

### Step 2: Create Navigation Objective Node

**Create**: `BP_ObjectiveNode_Navigation`

**Parent Class**: HorrorNavigationNode

**Purpose**: Objectives requiring player to reach a location.

**Variables**:
- `TargetLocation` (Vector) - Destination
- `CompletionRadius` (float, default: 200) - How close to complete
- `bShowMarker` (bool, default: true) - Show waypoint marker
- `bRequireLineOfSight` (bool, default: false) - Must see target

**Event Graph**:
```
On Node Activated
  → Parent: On Node Activated
  → Branch (bShowMarker)
    True:
      → Spawn Waypoint Marker (TargetLocation)
      → Set Marker Color (DebugColor)

On Actor Begin Overlap (TriggerVolume)
  → Cast to PlayerCharacter
  → Branch (Is Player)
    True:
      → Check Prerequisites Met
      → Branch (Prerequisites Met)
        True:
          → Complete Node (Player)
        False:
          → Show Message ("Prerequisites not met")

Event Tick (while active)
  → Get Player Location
  → Get Distance to TargetLocation
  → Branch (Distance < CompletionRadius)
    True:
      → Branch (bRequireLineOfSight)
        True:
          → Line Trace to Target
          → Branch (Has Line of Sight)
            True: Complete Node
        False:
          → Complete Node

On Node Completed
  → Parent: On Node Completed
  → Destroy Waypoint Marker
```

**Navigation Node Types** (create variants):

1. **BP_ObjectiveNode_ReachLocation** - Simple reach location
2. **BP_ObjectiveNode_FollowPath** - Follow waypoint path
3. **BP_ObjectiveNode_Escape** - Reach exit while being chased
4. **BP_ObjectiveNode_Explore** - Explore area (multiple sub-locations)

### Step 3: Create Interaction Objective Node

**Create**: `BP_ObjectiveNode_Interaction`

**Parent Class**: HorrorInteractionNode

**Purpose**: Objectives requiring interaction with specific objects.

**Variables**:
- `TargetInteractable` (Actor reference) - Object to interact with
- `RequiredInteractionCount` (int32, default: 1) - Times to interact
- `CurrentInteractionCount` (int32, default: 0)
- `bMustBeActive` (bool, default: true) - Only count when objective active

**Event Graph**:
```
On Node Activated
  → Parent: On Node Activated
  → Branch (TargetInteractable is valid)
    True:
      → Highlight Target Interactable
      → Bind to Interactable's OnInteract event
    False:
      → Log Error ("No target interactable set")

On Target Interacted (from event binding)
  → Branch (bMustBeActive AND IsActive)
    True:
      → Increment CurrentInteractionCount
      → Update Objective Progress
      → Branch (CurrentInteractionCount >= RequiredInteractionCount)
        True:
          → Complete Node (Interactor)
        False:
          → Show Progress ("Interact {0}/{1}", Current, Required)

On Node Completed
  → Parent: On Node Completed
  → Unhighlight Target Interactable
  → Unbind from Interactable events
```

**Interaction Node Types** (create variants):

1. **BP_ObjectiveNode_OpenDoor** - Open specific door
2. **BP_ObjectiveNode_CollectItem** - Collect specific item
3. **BP_ObjectiveNode_ReadDocument** - Read specific document
4. **BP_ObjectiveNode_ActivateSwitch** - Activate switch/button
5. **BP_ObjectiveNode_UseItem** - Use specific item on target

### Step 4: Create Encounter Objective Node

**Create**: `BP_ObjectiveNode_Encounter`

**Parent Class**: HorrorEncounterNode

**Purpose**: Objectives involving threats (survive, defeat, escape).

**Variables**:
- `EncounterType` (Enum: Survive, Defeat, Escape, Hide)
- `ThreatActors` (Array of Actor) - Threat actors involved
- `EncounterDuration` (float, default: 60.0) - For survive objectives
- `RequiredKills` (int32, default: 1) - For defeat objectives
- `CurrentKills` (int32, default: 0)
- `SafeZone` (Actor reference) - For escape/hide objectives

**Event Graph**:
```
On Node Activated
  → Parent: On Node Activated
  → Branch (EncounterType)
    Survive:
      → Start Survival Timer (EncounterDuration)
      → Spawn/Activate Threat Actors
    Defeat:
      → Spawn/Activate Threat Actors
      → Bind to Threat Death Events
    Escape:
      → Spawn/Activate Threat Actors
      → Activate Safe Zone
    Hide:
      → Spawn/Activate Threat Actors
      → Start Hide Timer

On Survival Timer Complete
  → Branch (Player is Alive)
    True: Complete Node (Player)
    False: Fail Node

On Threat Killed (from event binding)
  → Increment CurrentKills
  → Update Objective Progress
  → Branch (CurrentKills >= RequiredKills)
    True: Complete Node (Player)

On Player Entered Safe Zone
  → Branch (EncounterType == Escape OR Hide)
    True: Complete Node (Player)

On Player Death
  → Branch (IsActive)
    True: Fail Node

On Node Completed
  → Parent: On Node Completed
  → Deactivate/Destroy Threat Actors
  → Deactivate Safe Zone
```

**Encounter Node Types** (create variants):

1. **BP_ObjectiveNode_SurviveTime** - Survive for duration
2. **BP_ObjectiveNode_DefeatThreat** - Defeat specific threat
3. **BP_ObjectiveNode_EscapeArea** - Escape to safe zone
4. **BP_ObjectiveNode_HideFromThreat** - Hide until threat leaves
5. **BP_ObjectiveNode_AvoidDetection** - Sneak past threat

### Step 5: Create Objective Manager

**Create**: `BP_ObjectiveManager` (Actor)

**Purpose**: Manages all objectives in the level.

**Variables**:
- `AllObjectives` (Array of HorrorObjectiveNode)
- `ActiveObjectives` (Array of HorrorObjectiveNode)
- `CompletedObjectives` (Array of HorrorObjectiveNode)
- `CurrentPrimaryObjective` (HorrorObjectiveNode)
- `bShowObjectiveList` (bool, default: true)

**Functions**:

#### RegisterObjective
```
Input: HorrorObjectiveNode Objective
Add to AllObjectives array
  → Branch (Objective.bAutoActivateOnBeginPlay)
    True: Activate Objective
```

#### ActivateObjective
```
Input: HorrorObjectiveNode Objective
Branch (Can Activate - prerequisites met)
  True:
    → Objective → Activate Node
    → Add to ActiveObjectives
    → Branch (Is Primary Objective)
      True: Set CurrentPrimaryObjective
    → Update Objective UI
  False:
    → Log Warning ("Cannot activate - prerequisites not met")
```

#### CompleteObjective
```
Input: HorrorObjectiveNode Objective
Remove from ActiveObjectives
Add to CompletedObjectives
  → Branch (Objective == CurrentPrimaryObjective)
    True:
      → Clear CurrentPrimaryObjective
      → Find Next Primary Objective
  → Check for Dependent Objectives
  → Activate Dependent Objectives
  → Update Objective UI
  → Save Progress
```

#### GetActiveObjectives
```
Return: Array<HorrorObjectiveNode>
Return ActiveObjectives array
```

#### GetObjectiveProgress
```
Return: float (0-1)
Calculate: CompletedObjectives.Num() / AllObjectives.Num()
```

**Event Graph**:
```
Event BeginPlay
  → Get All Actors of Class (HorrorObjectiveNode)
  → For Each Objective:
      → Register Objective
  → Load Objective Progress
  → Update Objective UI

On Objective Completed (from Event Bus)
  → Complete Objective
  → Check for Level Completion
  → Branch (All Objectives Complete)
    True: Trigger Level Complete Event
```

### Step 6: Create Objective UI Widget

**Create**: `WBP_ObjectiveList`

**Purpose**: Shows active objectives in HUD.

**Widget Hierarchy**:
```
VerticalBox
├── TextBlock (Header: "OBJECTIVES")
├── ScrollBox (Objective List)
│   └── WBP_ObjectiveListItem (repeated)
└── ProgressBar (Overall Progress)
```

**Variables**:
- `ObjectiveManager` (BP_ObjectiveManager reference)
- `ObjectiveItems` (Array of WBP_ObjectiveListItem)

**Functions**:

#### UpdateObjectiveList
```
Get ObjectiveManager → Get Active Objectives
Clear Objective List
For Each Objective in Active Objectives:
  → Create Widget (WBP_ObjectiveListItem)
  → Set Objective Data (Objective)
  → Add to Objective List
  → Add to ObjectiveItems array
Update Overall Progress
```

**Create**: `WBP_ObjectiveListItem`

**Widget Hierarchy**:
```
HorizontalBox
├── Image (Status Icon)
├── VerticalBox
│   ├── TextBlock (Objective Title)
│   └── TextBlock (Objective Description)
└── ProgressBar (Objective Progress - if applicable)
```

**Variables**:
- `ObjectiveData` (HorrorObjectiveNode reference)
- `ObjectiveState` (EHorrorObjectiveNodeState)

**Functions**:

#### SetObjectiveData
```
Input: HorrorObjectiveNode Objective
Set ObjectiveData = Objective
Set Objective Title (Objective.NodeTitle)
Set Objective Description (Objective.NodeDescription)
Set Status Icon (based on Objective.NodeState)
Update Progress Bar (if applicable)
```

### Step 7: Create Waypoint Marker

**Create**: `BP_WaypointMarker` (Actor)

**Purpose**: Visual marker for navigation objectives.

**Components**:
- BillboardComponent (name: `MarkerIcon`)
- WidgetComponent (name: `MarkerWidget`)
- ArrowComponent (name: `DirectionArrow`)

**Variables**:
- `TargetLocation` (Vector)
- `MarkerColor` (LinearColor, default: Yellow)
- `bShowDistance` (bool, default: true)
- `bShowDirection` (bool, default: true)

**Event Graph**:
```
Event BeginPlay
  → Set Marker Color
  → Update Marker Widget

Event Tick
  → Get Player Location
  → Calculate Distance to Target
  → Calculate Direction to Target
  → Update Marker Widget (Distance, Direction)
  → Branch (bShowDirection)
    True:
      → Rotate Arrow to Point at Target
```

**Marker Widget** (create `WBP_WaypointMarker`):

**Widget Hierarchy**:
```
VerticalBox
├── Image (Marker Icon)
├── TextBlock (Distance)
└── Image (Direction Arrow)
```

### Step 8: Create Objective Prerequisite System

**In BP_ObjectiveNode_Base**:

**Functions**:

#### CheckPrerequisites
```
Return: bool
Branch (PrerequisiteNodeIds.Num() == 0)
  True: Return true (no prerequisites)
  False:
    → Get Objective Manager
    → For Each Prerequisite ID:
        → Get Objective by ID
        → Branch (Objective is NOT Completed)
          True: Return false
    → Return true (all prerequisites met)
```

#### GetPrerequisiteStatus
```
Return: FText
Branch (CheckPrerequisites)
  True: Return "Ready"
  False:
    → Get Incomplete Prerequisites
    → Return "Requires: {PrerequisiteNames}"
```

### Step 9: Create Objective Save/Load System

**In BP_ObjectiveManager**:

**Functions**:

#### SaveObjectiveProgress
```
Create Save Game Object
For Each Objective in AllObjectives:
  → Save Objective State (ID, State, Progress)
Save Game to Slot ("ObjectiveSave")
```

#### LoadObjectiveProgress
```
Load Game from Slot ("ObjectiveSave")
Branch (Save Game Exists)
  True:
    → For Each Saved Objective:
        → Find Objective by ID
        → Restore Objective State
        → Branch (State == Completed)
          True: Complete Objective (skip events)
        → Branch (State == Active)
          True: Activate Objective
```

### Step 10: Create Objective Debug Tools

**Create**: `BP_ObjectiveDebugger` (Actor)

**Purpose**: Debug tool for testing objectives.

**Variables**:
- `ObjectiveManager` (BP_ObjectiveManager reference)
- `bShowDebugInfo` (bool, default: true)

**Functions**:

#### ForceCompleteObjective
```
Input: FName ObjectiveID
Get Objective by ID
  → Force Complete (skip prerequisites)
```

#### ResetAllObjectives
```
Get All Objectives
For Each Objective:
  → Reset to Inactive State
  → Clear Progress
```

#### ActivateObjectiveByID
```
Input: FName ObjectiveID
Get Objective by ID
  → Force Activate (ignore prerequisites)
```

**Event Graph**:
```
Event Tick (if bShowDebugInfo)
  → Draw Debug Info for All Objectives
  → Show Active Objectives
  → Show Completed Objectives
  → Show Objective Progress
```

## Blueprint Usage Examples

### Example 1: Simple Navigation Objective

**BP_Objective_ReachLab**:
```
Node Type: Navigation
Node ID: "Obj_ReachLab"
Node Title: "Reach the Research Lab"
Node Description: "Find your way to the research laboratory"
Target Location: (X, Y, Z)
Completion Radius: 200
bShowMarker: true
```

### Example 2: Sequential Objectives

**BP_Objective_FindKeycard**:
```
Node ID: "Obj_FindKeycard"
Prerequisite Node IDs: []

On Completed:
  → Get Objective Manager
  → Activate Objective ("Obj_UnlockDoor")
```

**BP_Objective_UnlockDoor**:
```
Node ID: "Obj_UnlockDoor"
Prerequisite Node IDs: ["Obj_FindKeycard"]
Target Interactable: Door_MainLab
```

### Example 3: Branching Objectives

**BP_Objective_ChoosePath**:
```
On Completed:
  → Branch (Player chose Path A)
    True: Activate Objective ("Obj_PathA")
    False: Activate Objective ("Obj_PathB")
```

### Example 4: Timed Survival Objective

**BP_Objective_SurviveLockdown**:
```
Node Type: Encounter
Encounter Type: Survive
Encounter Duration: 120.0 (2 minutes)
Threat Actors: [Threat1, Threat2, Threat3]

On Activated:
  → Show Timer UI (120 seconds)
  → Spawn Threats
  → Lock Doors

On Completed:
  → Hide Timer UI
  → Unlock Doors
  → Despawn Threats
```

### Example 5: Multi-Stage Objective

**BP_Objective_RestorePower**:
```
Node ID: "Obj_RestorePower"
Sub-Objectives:
  - "Obj_FindFuseBox"
  - "Obj_RepairFuse"
  - "Obj_FlipSwitch"

On Sub-Objective Completed:
  → Increment Progress
  → Update UI ("Power Restoration: {0}/3", Progress)
  → Branch (All Sub-Objectives Complete)
    True: Complete Main Objective
```

## Event Binding

### HorrorObjectiveNode Events

**OnNodeActivated** (BlueprintNativeEvent):
- Called when node becomes active
- Override for custom activation logic

**OnNodeCompleted** (BlueprintNativeEvent):
- Called when node is completed
- Override for custom completion logic

**OnNodeFailed** (BlueprintNativeEvent):
- Called when node fails
- Override for custom failure logic

### Objective Manager Events

**OnObjectiveActivated** (Multicast Delegate):
- Broadcasts when objective activated
- Use for UI updates

**OnObjectiveCompleted** (Multicast Delegate):
- Broadcasts when objective completed
- Use for progression tracking

**OnObjectiveProgressChanged** (Multicast Delegate):
- Broadcasts when progress changes
- Use for UI updates

## Performance Optimization

### Objective System Optimization
- Disable tick for inactive objectives
- Use event-driven updates instead of tick
- Cache objective references
- Limit active objectives (max 5-10)
- Use simple collision for trigger volumes

### UI Optimization
- Update objective UI on events only (not tick)
- Cache objective widgets
- Limit objective list size
- Use invalidation for static content

## Troubleshooting

### Objective Not Activating
- Check prerequisites are met
- Verify objective ID is unique
- Ensure objective is registered
- Check auto-activate setting
- Verify event bus is working

### Objective Not Completing
- Check completion conditions
- Verify trigger volume overlap
- Ensure CanComplete returns true
- Check for blocking prerequisites
- Verify event bindings

### Waypoint Not Showing
- Check bShowMarker is true
- Verify waypoint spawn location
- Ensure widget component is visible
- Check Z-order and visibility
- Verify marker widget is set

### Progress Not Saving
- Check save game slot name
- Verify save game object created
- Ensure objective IDs are saved
- Check load is called on start
- Verify save directory permissions

## Best Practices

1. **Unique IDs**: Always use unique objective IDs
2. **Clear Descriptions**: Write clear objective descriptions
3. **Visual Feedback**: Show objective progress clearly
4. **Prerequisites**: Use prerequisite system for sequencing
5. **Save Progress**: Always save objective progress
6. **Event-Driven**: Use events instead of tick
7. **Testing**: Test all objective paths
8. **Debug Tools**: Use debug tools for testing
9. **UI Updates**: Update UI on objective changes
10. **Performance**: Optimize for many objectives

## Testing Checklist

- [ ] All objective nodes created
- [ ] Objective manager placed in level
- [ ] Objectives activate correctly
- [ ] Objectives complete correctly
- [ ] Prerequisites work correctly
- [ ] Waypoint markers show correctly
- [ ] Objective UI updates correctly
- [ ] Progress saves/loads correctly
- [ ] Sequential objectives work
- [ ] Branching objectives work
- [ ] Timed objectives work
- [ ] Multi-stage objectives work
- [ ] Debug tools work
- [ ] Performance acceptable
- [ ] No memory leaks

## Next Steps

1. Create all objective node blueprints
2. Place objectives in level
3. Configure objective chains
4. Test objective progression
5. Implement objective UI
6. Add waypoint markers
7. Test save/load system
8. Create debug tools
9. Optimize performance
10. Playtest objective flow
