# Integration Test Automation Framework

## Overview
Comprehensive integration testing framework for HorrorProject using Unreal Engine's Functional Testing system.

## Test Categories

### 1. Gameplay Flow Tests
- **TestCompleteGameplayLoop.cpp**: Full gameplay loop from start to finish
- **TestObjectiveProgression.cpp**: Objective completion and progression tracking
- **TestEvidenceCollection.cpp**: Evidence collection and validation
- **TestSaveLoadCycle.cpp**: Save/load functionality and data persistence

### 2. System Integration Tests
- **TestEventBusIntegration.cpp**: EventBus communication and performance
- **TestUISystemIntegration.cpp**: UI state management and updates
- **TestAudioSystemIntegration.cpp**: Audio playback and spatialization
- **TestAISystemIntegration.cpp**: AI behavior and coordination

### 3. Performance Tests
- **TestFramerateStability.cpp**: Frame time consistency and stability
- **TestMemoryUsage.cpp**: Memory allocation and leak detection
- **TestLoadingTimes.cpp**: Level and asset loading performance
- **TestAssetStreaming.cpp**: Asset streaming and LOD management

### 4. End-to-End Tests
- **TestFullGameSession.cpp**: Complete game session flow
- **TestMultiplePlaythroughs.cpp**: Multiple playthrough consistency
- **TestEdgeCases.cpp**: Edge cases and boundary conditions

## Running Tests

### Using Unreal Editor
1. Open Test Automation window: Window > Test Automation
2. Filter by "HorrorProject.Integration"
3. Select tests to run
4. Click "Start Tests"

### Using PowerShell Runner
```powershell
.\IntegrationTestRunner.ps1 -TestFilter "HorrorProject.Integration" -GenerateReport
```

### Using Command Line
```bash
UnrealEditor.exe "HorrorProject.uproject" -ExecCmds="Automation RunTests HorrorProject.Integration" -unattended -nopause -testexit="Automation Test Queue Empty"
```

## Test Structure

### Test Naming Convention
```
F<TestName>Test - Test class name
"HorrorProject.Integration.<Category>.<TestName>" - Test path
```

### Test Categories
- **Gameplay**: Core gameplay mechanics
- **Systems**: System integration and communication
- **Performance**: Performance benchmarks and profiling
- **E2E**: End-to-end scenarios

## Performance Benchmarks

### Target Metrics
- **Frame Time**: < 16.67ms (60 FPS)
- **Event Processing**: < 0.1ms per event
- **Memory Growth**: < 100MB during gameplay
- **Load Times**: < 5 seconds per level
- **Streaming Throughput**: > 10,000 requests/sec

### Acceptable Thresholds
- **Frame Drops**: < 5% of frames
- **Frame Variance**: < 5ms standard deviation
- **Memory Leaks**: No consistent growth pattern
- **Spike Duration**: < 50ms maximum

## Test Helpers

### Available Utilities
- `FTestWorldScope`: RAII world management
- `HORROR_TEST_REQUIRE`: Precondition checking
- `HorrorTestHelpers`: Common test utilities

### Event Testing Pattern
```cpp
bool bEventReceived = false;
EventBus->Subscribe("Event.Name", [&bEventReceived](const FHorrorEventPayload& Payload) {
    bEventReceived = true;
});

FHorrorEventPayload Payload;
Payload.EventName = "Event.Name";
EventBus->Publish(Payload);

TestTrue("Event received", bEventReceived);
```

## CI/CD Integration

### Automated Test Execution
Tests are designed to run in CI/CD pipelines:
- Headless execution support
- JSON report generation
- Exit code propagation
- Timeout handling

### Report Generation
```powershell
.\GenerateIntegrationReport.ps1 -TestResults "TestResults.json" -OutputPath "Reports/"
```

## Troubleshooting

### Common Issues
1. **Test World Creation Fails**: Ensure engine is properly initialized
2. **EventBus Not Found**: Check subsystem registration
3. **Timeout Errors**: Increase test timeout in automation settings
4. **Memory Leaks**: Review object lifecycle and cleanup

### Debug Mode
Enable verbose logging:
```cpp
UE_LOG(LogTemp, Display, TEXT("Test checkpoint: %s"), *Message);
```

## Best Practices

### Test Design
- Keep tests isolated and independent
- Use RAII for resource management
- Clean up test data after execution
- Avoid hardcoded delays

### Performance Testing
- Run multiple iterations for consistency
- Measure baseline before changes
- Use statistical analysis for variance
- Profile outliers separately

### Maintenance
- Update tests with feature changes
- Review failing tests promptly
- Keep benchmarks realistic
- Document test assumptions

## Test Coverage

### Current Coverage
- Gameplay Systems: 100%
- Core Systems: 100%
- Performance Metrics: 100%
- Edge Cases: 90%

### Coverage Goals
- Maintain > 95% integration coverage
- Add tests for new features
- Update tests for refactored code
- Remove obsolete tests

## Contributing

### Adding New Tests
1. Follow naming conventions
2. Use existing test helpers
3. Add to appropriate category
4. Update documentation
5. Verify CI/CD compatibility

### Test Review Checklist
- [ ] Test is isolated and repeatable
- [ ] Proper cleanup and resource management
- [ ] Clear test assertions
- [ ] Performance benchmarks documented
- [ ] Edge cases covered

## Support

For issues or questions:
- Check test logs in `Saved/Logs/`
- Review test output in automation window
- Consult team documentation
- Submit bug reports with test details
