# Behavior Tree Setup Guide

## Overview
This guide covers setting up behavior trees for AI characters in the Horror Project, specifically for the Golem threat actor.

## Prerequisites
- Unreal Engine 5.6
- AI Module enabled in project
- HorrorThreatAIController configured
- HorrorGolemBehaviorComponent implemented

## Basic Setup

### 1. Create Behavior Tree Asset
1. Content Browser → Right-click → AI → Behavior Tree
2. Name: `BT_Golem`
3. Location: `Content/AI/BehaviorTrees/`

### 2. Create Blackboard Asset
1. Content Browser → Right-click → AI → Blackboard
2. Name: `BB_Golem`
3. Add keys:
   - `TargetActor` (Object, Base Class: Actor)
   - `CurrentState` (Enum: EGolemEncounterState)
   - `DistanceToTarget` (Float)
   - `StateTimer` (Float)
   - `PatrolLocation` (Vector)
   - `bIsChasing` (Bool)
   - `bCanSeeTarget` (Bool)
   - `LastKnownLocation` (Vector)

### 3. Link Blackboard to Behavior Tree
1. Open `BT_Golem`
2. Details Panel → Blackboard Asset → Select `BB_Golem`

## Behavior Tree Structure

### Root Node
- Selector (Root)
  - Sequence: Final Impact
  - Sequence: Full Chase
  - Sequence: Chase Triggered
  - Sequence: Close Stalking
  - Sequence: Distant Sighting
  - Task: Idle

### Node Configuration

#### Selector Node (Root)
- Type: Selector
- Purpose: Choose highest priority behavior
- Evaluation: From left to right

#### Sequence Nodes
Each sequence represents a state from GolemBehaviorComponent:
1. Decorator: Check conditions
2. Task: Execute behavior
3. Service: Update blackboard

## Integration with GolemBehaviorComponent

### Sync State to Blackboard
```cpp
// In HorrorGolemBehaviorComponent::TransitionToState
if (AIController && AIController->GetBlackboardComponent())
{
    AIController->GetBlackboardComponent()->SetValueAsEnum(
        "CurrentState", 
        static_cast<uint8>(NewState)
    );
}
```

### Update Distance
```cpp
// In HorrorGolemBehaviorComponent::TickComponent
if (AIController && AIController->GetBlackboardComponent())
{
    AIController->GetBlackboardComponent()->SetValueAsFloat(
        "DistanceToTarget", 
        GetDistanceToTarget()
    );
}
```

## Custom Tasks

### BTTask_UpdateGolemState
Updates the Golem's behavior state based on distance and conditions.

### BTTask_MoveToTarget
Moves the Golem towards the target at specified speed.

### BTTask_PatrolAroundTarget
Circles around the target at stalking distance.

### BTTask_TriggerFinalImpact
Triggers the final attack sequence.

## Custom Decorators

### BTDecorator_CheckGolemState
Checks if Golem is in specific state.

### BTDecorator_CheckDistance
Checks distance to target against threshold.

### BTDecorator_CheckStateTimer
Checks if enough time has passed in current state.

## Custom Services

### BTService_UpdateGolemBlackboard
Updates blackboard values every tick:
- DistanceToTarget
- StateTimer
- bCanSeeTarget
- LastKnownLocation

## Testing

### Debug Mode
Enable debug visualization:
```cpp
// In Blueprint or C++
GolemBehaviorComponent->bDrawDebugInfo = true;
```

### Console Commands
```
ai.debug [ActorName]  // Show AI debug info
showdebug ai          // Toggle AI debug display
```

## Performance Considerations

### Update Frequency
- Behavior Tree: 0.1s (10 Hz)
- Services: 0.5s (2 Hz)
- Decorators: On demand

### Optimization
- Use blackboard queries instead of direct actor queries
- Cache frequently accessed values
- Limit perception updates to necessary senses

## Common Issues

### Behavior Tree Not Running
- Check AIController is assigned
- Verify Behavior Tree is set in AIController
- Ensure Blackboard is linked

### State Not Updating
- Verify GolemBehaviorComponent is updating blackboard
- Check decorator conditions
- Enable debug visualization

### Performance Issues
- Reduce service update frequency
- Optimize decorator checks
- Use simpler movement calculations

## Next Steps
- See [GolemBehaviorTree.md](GolemBehaviorTree.md) for detailed implementation
- See [AITasksReference.md](AITasksReference.md) for custom task details
- See [PerceptionSystemSetup.md](../Perception/PerceptionSystemSetup.md) for perception integration
