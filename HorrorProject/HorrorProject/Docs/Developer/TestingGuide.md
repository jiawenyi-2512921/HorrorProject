# Testing Guide

## Testing Philosophy

HorrorProject uses a comprehensive testing strategy:
- **Unit Tests** - Test individual components in isolation
- **Integration Tests** - Test system interactions
- **Functional Tests** - Test gameplay features end-to-end
- **Performance Tests** - Validate performance targets

## Test Framework

### Unreal Automation Framework

All tests use Unreal's built-in automation framework:
- Fast execution
- Editor and runtime support
- CI/CD integration
- Detailed reporting

### Test Categories

```cpp
// Unit test
EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter

// Smoke test (quick validation)
EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter

// Performance test
EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::PerfFilter
```

## Writing Unit Tests

### Basic Test Structure

```cpp
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FMyTest,
    "HorrorProject.Module.Component.TestName",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

bool FMyTest::RunTest(const FString& Parameters)
{
    // Arrange - Set up test conditions
    int32 Value = 42;
    
    // Act - Perform the action
    int32 Result = Value * 2;
    
    // Assert - Verify the result
    TestEqual("Value doubled correctly", Result, 84);
    
    return true;
}
```

### Test Assertions

```cpp
// Equality
TestEqual("Description", ActualValue, ExpectedValue);
TestNotEqual("Description", ActualValue, UnexpectedValue);

// Boolean
TestTrue("Description", bCondition);
TestFalse("Description", bCondition);

// Null checks
TestNull("Description", Pointer);
TestNotNull("Description", Pointer);

// Floating point
TestEqual("Description", ActualFloat, ExpectedFloat, 0.001f); // With tolerance

// Strings
TestEqual("Description", ActualString, ExpectedString);

// Custom validation
if (!CustomCondition) {
    AddError("Custom error message");
}
```

### Complex Test Example

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FHorrorEventBusPublishTest,
    "HorrorProject.Game.EventBus.Publish",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

bool FHorrorEventBusPublishTest::RunTest(const FString& Parameters)
{
    // Arrange
    UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
    UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
    TestNotNull("Event Bus created", EventBus);
    
    bool bEventReceived = false;
    FHorrorEventMessage ReceivedMessage;
    
    EventBus->GetOnEventPublishedNative().AddLambda([&](const FHorrorEventMessage& Message) {
        bEventReceived = true;
        ReceivedMessage = Message;
    });
    
    // Act
    FGameplayTag TestTag = FGameplayTag::RequestGameplayTag("Event.Test");
    bool bPublished = EventBus->Publish(TestTag, TEXT("TestSource"), FGameplayTag(), nullptr);
    
    // Assert
    TestTrue("Event published successfully", bPublished);
    TestTrue("Event received by subscriber", bEventReceived);
    TestEqual("Event tag matches", ReceivedMessage.EventTag, TestTag);
    TestEqual("Source ID matches", ReceivedMessage.SourceId, FName("TestSource"));
    
    // Verify history
    const TArray<FHorrorEventMessage>& History = EventBus->GetHistory();
    TestEqual("History contains one event", History.Num(), 1);
    
    return true;
}
```

## Testing Subsystems

### World Subsystem Tests

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FArchiveSubsystemTest,
    "HorrorProject.Evidence.ArchiveSubsystem.AddEvidence",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

bool FArchiveSubsystemTest::RunTest(const FString& Parameters)
{
    // Create test world
    UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
    UArchiveSubsystem* Archive = World->GetSubsystem<UArchiveSubsystem>();
    
    // Create test evidence
    FEvidenceData Evidence;
    Evidence.EvidenceId = TEXT("Test_Evidence");
    Evidence.DisplayName = FText::FromString("Test Evidence");
    Evidence.Type = EEvidenceType::Document;
    
    // Add evidence
    bool bAdded = Archive->AddEvidence(Evidence);
    TestTrue("Evidence added", bAdded);
    
    // Verify retrieval
    FEvidenceData Retrieved;
    bool bFound = Archive->GetEvidence(TEXT("Test_Evidence"), Retrieved);
    TestTrue("Evidence found", bFound);
    TestEqual("Evidence ID matches", Retrieved.EvidenceId, Evidence.EvidenceId);
    
    return true;
}
```

### Actor Tests

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FEncounterDirectorTest,
    "HorrorProject.Game.EncounterDirector.StateTransitions",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

bool FEncounterDirectorTest::RunTest(const FString& Parameters)
{
    UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
    
    // Spawn encounter director
    AHorrorEncounterDirector* Director = World->SpawnActor<AHorrorEncounterDirector>();
    TestNotNull("Director spawned", Director);
    
    // Test state transitions
    TestEqual("Initial state is Dormant", 
        Director->GetEncounterPhase(), EHorrorEncounterPhase::Dormant);
    
    bool bPrimed = Director->PrimeEncounter(TEXT("Test_Encounter"));
    TestTrue("Encounter primed", bPrimed);
    TestEqual("State is Primed", 
        Director->GetEncounterPhase(), EHorrorEncounterPhase::Primed);
    
    // Spawn player for reveal test
    ACharacter* Player = World->SpawnActor<ACharacter>(
        Director->GetActorLocation() + FVector(100, 0, 0),
        FRotator::ZeroRotator
    );
    
    bool bRevealed = Director->TriggerReveal(Player);
    TestTrue("Encounter revealed", bRevealed);
    TestEqual("State is Revealed", 
        Director->GetEncounterPhase(), EHorrorEncounterPhase::Revealed);
    
    return true;
}
```

## Integration Tests

### System Interaction Tests

```cpp
IMPLEMENT_COMPLEX_AUTOMATION_TEST(
    FEvidenceCollectionIntegrationTest,
    "HorrorProject.Integration.EvidenceCollection",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

void FEvidenceCollectionIntegrationTest::GetTests(TArray<FString>& OutBeautifiedNames, TArray<FString>& OutTestCommands) const
{
    OutBeautifiedNames.Add("Collect Evidence");
    OutTestCommands.Add("");
}

bool FEvidenceCollectionIntegrationTest::RunTest(const FString& Parameters)
{
    UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
    
    // Set up systems
    UArchiveSubsystem* Archive = World->GetSubsystem<UArchiveSubsystem>();
    UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
    
    // Spawn player with evidence collection
    AHorrorProjectCharacter* Player = World->SpawnActor<AHorrorProjectCharacter>();
    UEvidenceCollectionComponent* Collection = Player->FindComponentByClass<UEvidenceCollectionComponent>();
    TestNotNull("Collection component exists", Collection);
    
    // Spawn evidence actor
    AEvidenceActor* EvidenceActor = World->SpawnActor<AEvidenceActor>(
        Player->GetActorLocation() + FVector(100, 0, 0),
        FRotator::ZeroRotator
    );
    
    // Subscribe to events
    bool bEventReceived = false;
    EventBus->GetOnEventPublishedNative().AddLambda([&](const FHorrorEventMessage& Message) {
        if (Message.EventTag.MatchesTag(FGameplayTag::RequestGameplayTag("Event.Evidence.Collected"))) {
            bEventReceived = true;
        }
    });
    
    // Collect evidence
    Collection->CollectEvidence(EvidenceActor);
    
    // Verify results
    TestTrue("Event published", bEventReceived);
    TestTrue("Evidence in archive", Archive->HasEvidence(EvidenceActor->GetEvidenceId()));
    
    return true;
}
```

## Performance Tests

### Frame Time Tests

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FEventBusPerformanceTest,
    "HorrorProject.Performance.EventBus.PublishRate",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::PerfFilter
)

bool FEventBusPerformanceTest::RunTest(const FString& Parameters)
{
    UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
    UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
    
    const int32 NumEvents = 1000;
    const double StartTime = FPlatformTime::Seconds();
    
    // Publish many events
    for (int32 i = 0; i < NumEvents; ++i) {
        EventBus->Publish(
            FGameplayTag::RequestGameplayTag("Event.Test"),
            FName(*FString::Printf(TEXT("Source_%d"), i)),
            FGameplayTag(),
            nullptr
        );
    }
    
    const double EndTime = FPlatformTime::Seconds();
    const double Duration = EndTime - StartTime;
    const double EventsPerSecond = NumEvents / Duration;
    
    AddInfo(FString::Printf(TEXT("Published %d events in %.3f seconds (%.0f events/sec)"),
        NumEvents, Duration, EventsPerSecond));
    
    // Verify performance target
    TestTrue("Event bus performance acceptable", EventsPerSecond > 10000);
    
    return true;
}
```

### Memory Tests

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FEventBusMemoryTest,
    "HorrorProject.Performance.EventBus.MemoryUsage",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::PerfFilter
)

bool FEventBusMemoryTest::RunTest(const FString& Parameters)
{
    UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
    UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
    
    // Measure initial memory
    FPlatformMemoryStats InitialStats = FPlatformMemory::GetStats();
    
    // Fill history buffer
    for (int32 i = 0; i < 128; ++i) {
        EventBus->Publish(
            FGameplayTag::RequestGameplayTag("Event.Test"),
            TEXT("TestSource"),
            FGameplayTag(),
            nullptr
        );
    }
    
    // Measure final memory
    FPlatformMemoryStats FinalStats = FPlatformMemory::GetStats();
    
    const int64 MemoryUsed = FinalStats.UsedPhysical - InitialStats.UsedPhysical;
    AddInfo(FString::Printf(TEXT("Memory used: %lld bytes"), MemoryUsed));
    
    // Verify memory usage is reasonable (< 100KB for 128 events)
    TestTrue("Memory usage acceptable", MemoryUsed < 100 * 1024);
    
    return true;
}
```

## Running Tests

### From Editor

1. Window → Test Automation
2. Select test category or individual tests
3. Click "Start Tests"
4. View results in test window

### From Command Line

```powershell
# Run all tests
.\Scripts\RunTests.ps1

# Run specific category
.\Scripts\RunTests.ps1 -Filter "HorrorProject.Game"

# Run with detailed output
.\Scripts\RunTests.ps1 -Verbose
```

### Automated Testing

```powershell
# CI/CD pipeline
& "C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" `
    "D:\gptzuo\HorrorProject\HorrorProject\HorrorProject.uproject" `
    -ExecCmds="Automation RunTests HorrorProject; Quit" `
    -unattended -nopause -nosplash -nullrhi -log
```

## Test Organization

### Directory Structure

```
Source/HorrorProject/
├── AI/
│   └── Tests/
│       └── HorrorThreatAITests.cpp
├── Audio/
│   └── Tests/
│       └── HorrorAudioSubsystemTests.cpp
├── Evidence/
│   └── Tests/
│       ├── ArchiveSubsystemTests.cpp
│       └── EvidenceCollectionTests.cpp
└── Tests/
    ├── IntegrationTests.cpp
    └── PerformanceTests.cpp
```

### Naming Conventions

**Test Files:**
- `[Component]Tests.cpp`
- Located in `Tests/` subfolder

**Test Names:**
- `F[Component][Feature]Test`
- Hierarchical: `"HorrorProject.Module.Component.Feature"`

## Test Best Practices

### Do's
- Test one thing per test
- Use descriptive test names
- Clean up resources after tests
- Use appropriate assertions
- Test edge cases and error conditions
- Keep tests fast (< 1 second each)

### Don'ts
- Don't test Unreal Engine functionality
- Don't depend on external resources
- Don't use hardcoded file paths
- Don't leave test actors in world
- Don't skip cleanup

## Mocking and Stubbing

### Interface Mocking

```cpp
class FMockInteractable : public IHorrorInteractable
{
public:
    bool bInteractCalled = false;
    AActor* LastInteractor = nullptr;
    
    virtual void Interact_Implementation(AActor* Interactor) override
    {
        bInteractCalled = true;
        LastInteractor = Interactor;
    }
};
```

### Subsystem Stubbing

```cpp
// Use test-specific subsystem configuration
EventBus->SetHistoryCapacityForTests(10);
EventBus->ResetForTests();
```

## Debugging Tests

### Visual Studio Debugging

1. Set breakpoint in test code
2. Debug → Start Debugging (F5)
3. In editor: Window → Test Automation
4. Run specific test
5. Breakpoint hits

### Logging in Tests

```cpp
bool FMyTest::RunTest(const FString& Parameters)
{
    AddInfo("Starting test");
    AddWarning("This is a warning");
    AddError("This is an error");
    
    UE_LOG(LogTemp, Log, TEXT("Test log message"));
    
    return true;
}
```

## Test Coverage

### Measuring Coverage

Use Visual Studio Code Coverage:
1. Test → Analyze Code Coverage
2. Run tests
3. View coverage results

**Target Coverage:**
- Core systems: > 80%
- Gameplay code: > 60%
- UI code: > 40%

### Coverage Reports

```powershell
# Generate coverage report
.\Scripts\GenerateCoverageReport.ps1
```

## Continuous Integration

### CI Pipeline

```yaml
test:
  stage: test
  script:
    - .\Scripts\BuildProject.ps1 -Configuration Development
    - .\Scripts\RunTests.ps1 -CI
  artifacts:
    reports:
      junit: TestResults.xml
```

### Test Reporting

Tests generate JUnit XML for CI integration:
- Test results
- Execution time
- Failure details

---
Last Updated: 2026-04-26
