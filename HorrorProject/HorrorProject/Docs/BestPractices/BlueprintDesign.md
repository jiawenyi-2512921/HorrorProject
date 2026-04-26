# Blueprint Design Best Practices

Guidelines for creating maintainable and performant Blueprints in HorrorProject.

## Organization

### Blueprint Structure

```
MyBlueprint
├── Components
│   ├── Evidence Collection
│   ├── Audio Component
│   └── Interaction Component
├── Construction Script
├── Event Graph
│   ├── Initialization
│   ├── Input Handling
│   └── Game Logic
└── Functions
    ├── Public Functions
    └── Private Functions
```

### Use Functions

```blueprint
// Bad: All logic in Event Graph
Event BeginPlay → [100 nodes of logic]

// Good: Organized into functions
Event BeginPlay → Initialize Components
              → Setup Audio
              → Load Save Data
```

### Use Macros for Reusable Logic

Create macros for commonly used node sequences:
- Input validation
- Null checks
- Common calculations

### Separate Concerns

- **Event Graph**: High-level flow and events
- **Functions**: Specific logic and calculations
- **Macros**: Reusable node sequences
- **Interfaces**: Communication between Blueprints

## Performance

### Minimize Tick Usage

```blueprint
// Bad: Using Tick for everything
Event Tick → Check Distance
         → Update UI
         → Play Audio

// Good: Use events and timers
Event BeginPlay → Set Timer by Function Name (0.5s, looping)
Custom Event "Update Logic" → Check Distance → Update UI
```

### Cache References

```blueprint
// Bad: Get component every frame
Event Tick → Get Component by Class (Evidence Collection) → Use Component

// Good: Cache in BeginPlay
Event BeginPlay → Get Component by Class → Promote to Variable "Cached Evidence"
Event Tick → Use "Cached Evidence" variable
```

### Use Pure Functions Wisely

```blueprint
// Pure functions (no execution pins) are called every time they're used
// Cache results if used multiple times

// Bad: Pure function called 5 times
Get Evidence Count → Branch
Get Evidence Count → Print
Get Evidence Count → Set Text
Get Evidence Count → Compare
Get Evidence Count → Add

// Good: Call once and cache
Get Evidence Count → Promote to Local Variable "Count"
Use "Count" variable in all places
```

### Avoid Complex Math in Tick

```blueprint
// Bad: Complex calculations every frame
Event Tick → Complex Math (sin, cos, power, etc.)

// Good: Precalculate or use timers
Event BeginPlay → Calculate Values → Store in Variables
Event Tick → Use Precalculated Values
```

## Communication

### Use Interfaces

```blueprint
// Define Interface: BPI_Interactable
Function: Interact (Actor: Interactor)

// Implement in Blueprint
Implement Interface → BPI_Interactable
Interact Event → Custom Logic

// Call from other Blueprint
Does Implement Interface (BPI_Interactable) → Interact (Message)
```

### Use Event Dispatchers

```blueprint
// In Evidence Component Blueprint
Event Dispatcher: OnEvidenceCollected (Evidence Data)

// Bind in Character Blueprint
Event BeginPlay → Get Evidence Component → Bind Event to OnEvidenceCollected
Custom Event "Handle Evidence Collected" → Update UI
```

### Avoid Casting When Possible

```blueprint
// Bad: Hard reference via cast
Get Player Character → Cast to BP_MyCharacter → Access Properties

// Good: Use interface
Get Player Character → Does Implement Interface → Call Interface Function

// Good: Use component
Get Player Character → Get Component by Class (Evidence Collection)
```

## Variables

### Use Appropriate Access Levels

```blueprint
// Public (Editable Anywhere): Designer-configurable values
Max Health (Editable Anywhere, BlueprintReadWrite)

// Protected (Blueprint Read Only): Internal state
Current Health (VisibleAnywhere, BlueprintReadOnly)

// Private: Implementation details
Internal Timer Handle (Private)
```

### Use Categories

```blueprint
Variables:
├── Settings
│   ├── Max Health
│   └── Movement Speed
├── State
│   ├── Current Health
│   └── Is Alive
└── References
    ├── Evidence Component
    └── Audio Component
```

### Initialize Variables

```blueprint
// Set default values in Class Defaults
Max Health = 100
Movement Speed = 600.0
Is Alive = true
```

### Use Descriptive Names

```blueprint
// Bad
H = 100
Spd = 600
B = true

// Good
MaxHealth = 100
MovementSpeed = 600
bIsAlive = true
```

## Error Handling

### Validate Input

```blueprint
Function: Collect Evidence
├── Input: Evidence Actor
├── Is Valid (Evidence Actor) → Branch
│   ├── True → Proceed with Collection
│   └── False → Print Warning → Return False
```

### Check for Null

```blueprint
// Always check references before use
Get Evidence Component → Is Valid → Branch
├── True → Use Component
└── False → Log Error
```

### Use Return Nodes

```blueprint
Function: Process Data
├── Validate Input → Branch
│   └── False → Print Error → Return (early exit)
├── Process Data
└── Return Success
```

## Debugging

### Use Print String

```blueprint
// Add debug prints during development
Collect Evidence → Print String ("Collecting: " + Evidence Name)
```

### Use Debug Categories

```blueprint
// Organize debug output
Print String (Category: "Evidence", Text: "Collected")
Print String (Category: "Audio", Text: "Playing Sound")
```

### Use Breakpoints

- Right-click node → Add Breakpoint
- Game pauses when breakpoint hit
- Inspect variable values
- Step through execution

### Use Watch Values

- Right-click variable → Watch This Value
- Monitor variable changes in real-time

## Blueprint Types

### Actor Blueprint

```blueprint
BP_EvidenceActor (inherits from Actor)
├── Static Mesh Component
├── Collision Component
├── Evidence Data (struct)
└── Interaction Logic
```

### Component Blueprint

```blueprint
BP_EvidenceCollectionComponent (inherits from ActorComponent)
├── Evidence Array
├── Collection Logic
└── Save/Load Functions
```

### Widget Blueprint

```blueprint
WBP_EvidenceUI (inherits from UserWidget)
├── Text Blocks
├── Images
├── Update Functions
└── Event Bindings
```

### Function Library

```blueprint
BPL_EvidenceLibrary (inherits from BlueprintFunctionLibrary)
├── Static Function: Get Evidence Count
├── Static Function: Has Evidence
└── Static Function: Format Evidence Text
```

## Common Patterns

### Singleton Pattern

```blueprint
// Game Instance Blueprint
BP_GameInstance
├── Evidence Manager (variable)
└── Get Evidence Manager (function)
    ├── Is Valid (Evidence Manager) → Branch
    │   ├── True → Return Evidence Manager
    │   └── False → Create Manager → Return
```

### Observer Pattern

```blueprint
// Subject
Event Dispatcher: OnStateChanged

// Observer
Bind Event to OnStateChanged → Custom Event "Handle State Change"
```

### State Machine

```blueprint
// Enum: ECharacterState (Idle, Walking, Running, Jumping)

// State Variable
Current State (ECharacterState)

// State Logic
Switch on ECharacterState
├── Idle → Idle Logic
├── Walking → Walking Logic
├── Running → Running Logic
└── Jumping → Jumping Logic
```

## Testing

### Create Test Functions

```blueprint
Function: Test Evidence Collection
├── Create Test Evidence
├── Call Collect Evidence
├── Assert Evidence Count = 1
└── Print Test Result
```

### Use Automation

```blueprint
// Functional Test Actor
BP_EvidenceTest (inherits from FunctionalTest)
├── Prepare Test → Spawn Evidence
├── Start Test → Collect Evidence
└── Finish Test → Assert Success
```

## Documentation

### Add Comments

```blueprint
// Comment boxes for major sections
[Initialize Components]
[Setup Audio System]
[Load Save Data]

// Node comments for complex logic
"Calculate distance to nearest threat"
"Apply damage with falloff"
```

### Use Descriptive Names

```blueprint
// Bad
Function: DoStuff
Variable: Temp
Event: E1

// Good
Function: CollectEvidence
Variable: EvidenceCount
Event: OnEvidenceCollected
```

## Common Pitfalls

### Circular Dependencies

```blueprint
// Bad: BP_A references BP_B, BP_B references BP_A
// Solution: Use interfaces or event dispatchers
```

### Too Many Casts

```blueprint
// Bad: Casting everywhere creates hard dependencies
// Solution: Use interfaces, components, or event dispatchers
```

### Tick Overuse

```blueprint
// Bad: Everything in Tick
// Solution: Use events, timers, and delegates
```

### No Error Handling

```blueprint
// Bad: Assuming everything works
// Solution: Validate input, check for null, handle errors
```

### Spaghetti Code

```blueprint
// Bad: Execution wires crossing everywhere
// Solution: Use functions, reroute nodes, organize layout
```

## Blueprint Nativization

For shipping builds, enable Blueprint nativization:

```
Project Settings → Packaging
Blueprint Nativization Method: Inclusive
```

Benefits:
- Improved performance
- Reduced memory usage
- Faster execution

## Related Documentation
- [C++ Best Practices](CppCoding.md)
- [Performance Optimization](PerformanceOptimization.md)
- [Blueprint Quick Start](../Tutorials/BlueprintQuickStart.md)
- [API Reference](../API/README.md)
