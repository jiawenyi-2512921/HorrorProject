# Event Bus System

## Overview

The `UHorrorEventBusSubsystem` is the central communication hub for HorrorProject. It implements a publish-subscribe pattern that decouples game systems and enables flexible, data-driven gameplay.

## Architecture

### Core Components

**Event Bus Subsystem**
- World-scoped subsystem (one per level)
- Manages event routing and history
- Provides both Blueprint and C++ APIs

**Event Message**
```cpp
struct FHorrorEventMessage {
    FGameplayTag EventTag;      // What happened
    FGameplayTag StateTag;      // Current state
    FName SourceId;             // Who triggered it
    UObject* SourceObject;      // Reference to source
    float WorldSeconds;         // When it happened
    FName TrailerBeatId;        // Optional: cinematic beat
    FText ObjectiveHint;        // Optional: player hint
    FText DebugLabel;           // Optional: debug info
};
```

**Objective Metadata**
```cpp
struct FHorrorObjectiveMessageMetadata {
    FName TrailerBeatId;
    FText ObjectiveHint;
    FText DebugLabel;
};
```

## Usage Patterns

### Publishing Events

**From C++:**
```cpp
UHorrorEventBusSubsystem* EventBus = GetWorld()->GetSubsystem<UHorrorEventBusSubsystem>();
EventBus->Publish(
    FGameplayTag::RequestGameplayTag("Event.Encounter.Revealed"),
    TEXT("Encounter_01"),
    FGameplayTag::RequestGameplayTag("State.Active"),
    this
);
```

**From Blueprint:**
```
Get Horror Event Bus Subsystem → Publish
    Event Tag: Event.Encounter.Revealed
    Source Id: Encounter_01
    State Tag: State.Active
    Source Object: Self
```

### Subscribing to Events

**C++ Native Delegate:**
```cpp
void AMyActor::BeginPlay() {
    Super::BeginPlay();
    
    UHorrorEventBusSubsystem* EventBus = GetWorld()->GetSubsystem<UHorrorEventBusSubsystem>();
    EventBus->GetOnEventPublishedNative().AddUObject(this, &AMyActor::OnEventReceived);
}

void AMyActor::OnEventReceived(const FHorrorEventMessage& Message) {
    if (Message.EventTag.MatchesTag(FGameplayTag::RequestGameplayTag("Event.Encounter"))) {
        // Handle encounter events
    }
}
```

**Blueprint Dynamic Delegate:**
```
Event Begin Play → Get Horror Event Bus Subsystem → Bind Event to On Event Published
    Event: Custom Event "Handle Event"
```

### Registering Objective Metadata

**Before Publishing:**
```cpp
FHorrorObjectiveMessageMetadata Metadata;
Metadata.ObjectiveHint = FText::FromString("Find the exit");
Metadata.TrailerBeatId = TEXT("Beat_ExitReveal");

EventBus->RegisterObjectiveMetadata(TEXT("Objective_Exit"), Metadata);
```

**Publishing with Metadata:**
```cpp
// Metadata is automatically attached to the event message
EventBus->Publish(
    FGameplayTag::RequestGameplayTag("Event.Objective.Started"),
    TEXT("Objective_Exit"),
    FGameplayTag::RequestGameplayTag("State.Active"),
    this
);
```

## Event Taxonomy

### Event Categories

**Encounter Events**
- `Event.Encounter.Primed` - Encounter ready to trigger
- `Event.Encounter.Revealed` - Threat revealed to player
- `Event.Encounter.Resolved` - Encounter completed

**Objective Events**
- `Event.Objective.Started` - New objective active
- `Event.Objective.Updated` - Objective progress changed
- `Event.Objective.Completed` - Objective finished

**Evidence Events**
- `Event.Evidence.Collected` - Evidence picked up
- `Event.Evidence.Examined` - Evidence viewed in archive
- `Event.Evidence.Unlocked` - New evidence available

**Audio Events**
- `Event.Audio.ZoneEntered` - Player entered audio zone
- `Event.Audio.ZoneExited` - Player left audio zone
- `Event.Audio.StingerTriggered` - Audio stinger played

**Player Events**
- `Event.Player.Damaged` - Player took damage
- `Event.Player.Died` - Player death
- `Event.Player.Interacted` - Player interacted with object

### State Tags

- `State.Inactive` - System dormant
- `State.Active` - System running
- `State.Completed` - System finished
- `State.Failed` - System failed

## Event History

The Event Bus maintains a circular buffer of recent events:

```cpp
const TArray<FHorrorEventMessage>& History = EventBus->GetHistory();
for (const FHorrorEventMessage& Event : History) {
    UE_LOG(LogTemp, Log, TEXT("Event: %s at %.2f"), 
        *Event.EventTag.ToString(), Event.WorldSeconds);
}
```

**History Capacity:** 128 events (configurable)

## Performance Considerations

### Optimization Strategies

**Delegate Management**
- Unbind delegates when actors are destroyed
- Use weak object pointers for long-lived subscriptions
- Avoid heavy processing in event handlers

**Event Filtering**
- Filter by tag hierarchy: `Event.Encounter` matches all encounter events
- Early-out in handlers for irrelevant events
- Use native delegates for performance-critical systems

**History Management**
- History is circular buffer (no dynamic allocation)
- Old events are automatically overwritten
- Disable history in shipping builds if not needed

### Profiling

```cpp
TRACE_CPUPROFILER_EVENT_SCOPE(HorrorEventBus_Publish);
```

## Testing

### Unit Tests

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHorrorEventBusTest, "HorrorProject.EventBus.Publish")

bool FHorrorEventBusTest::RunTest(const FString& Parameters) {
    UWorld* World = CreateTestWorld();
    UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
    
    bool bReceived = false;
    EventBus->GetOnEventPublishedNative().AddLambda([&](const FHorrorEventMessage& Msg) {
        bReceived = true;
    });
    
    EventBus->Publish(FGameplayTag::RequestGameplayTag("Event.Test"), TEXT("Test"), FGameplayTag(), nullptr);
    
    TestTrue("Event received", bReceived);
    return true;
}
```

### Debug Visualization

Enable debug logging:
```cpp
EventBus->SetDebugLogging(true);
```

Console command:
```
HorrorEventBus.ShowHistory
```

## Best Practices

### Do's
- Use descriptive SourceIds (e.g., "Encounter_Hallway_01")
- Register metadata before publishing
- Unbind delegates in EndPlay/Destroyed
- Use gameplay tags for event types
- Keep event handlers lightweight

### Don'ts
- Don't publish events every frame
- Don't store raw pointers to SourceObject
- Don't perform expensive operations in handlers
- Don't create circular event dependencies
- Don't use events for time-critical systems

## Integration Examples

### Encounter Director Integration

```cpp
void AHorrorEncounterDirector::TriggerReveal(AActor* PlayerActor) {
    if (UHorrorEventBusSubsystem* EventBus = GetWorld()->GetSubsystem<UHorrorEventBusSubsystem>()) {
        EventBus->Publish(
            FGameplayTag::RequestGameplayTag("Event.Encounter.Revealed"),
            ActiveEncounterId,
            FGameplayTag::RequestGameplayTag("State.Active"),
            this
        );
    }
}
```

### UI Integration

```cpp
void UObjectiveWidget::NativeConstruct() {
    Super::NativeConstruct();
    
    if (UHorrorEventBusSubsystem* EventBus = GetWorld()->GetSubsystem<UHorrorEventBusSubsystem>()) {
        EventBus->OnEventPublished.AddDynamic(this, &UObjectiveWidget::OnEventReceived);
    }
}

void UObjectiveWidget::OnEventReceived(const FHorrorEventMessage& Message) {
    if (Message.EventTag.MatchesTag(FGameplayTag::RequestGameplayTag("Event.Objective"))) {
        UpdateObjectiveDisplay(Message.ObjectiveHint);
    }
}
```

## Troubleshooting

**Events not received:**
- Check delegate is bound
- Verify gameplay tags are registered
- Ensure subsystem is initialized

**Performance issues:**
- Profile event handlers
- Reduce event frequency
- Use native delegates instead of dynamic

**Memory leaks:**
- Unbind delegates properly
- Don't capture strong references in lambdas
- Clear metadata when no longer needed

---
Last Updated: 2026-04-26
