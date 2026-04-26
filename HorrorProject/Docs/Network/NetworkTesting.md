# Network Testing Guide

## Overview
Comprehensive testing guide for HorrorProject's multiplayer networking, covering unit tests, integration tests, and performance testing.

## Test Categories

### 1. Unit Tests
Located in: `Source/HorrorProject/Tests/`

#### Running Unit Tests
```bash
# In Unreal Editor
Window → Test Automation → Session Frontend

# Command line
UnrealEditor.exe HorrorProject.uproject -ExecCmds="Automation RunTests HorrorProject.Network" -unattended -nopause -testexit="Automation Test Queue Empty"
```

#### Test Coverage

**TestNetworkReplication.cpp**
- NetworkReplicationComponent functionality
- ReplicatedPlayerState synchronization
- ReplicatedGameState management
- Transform replication
- Variable synchronization

**TestMultiplayerSync.cpp**
- HorrorGameModeMultiplayer logic
- HorrorPlayerControllerMultiplayer features
- MultiplayerSessionSubsystem operations
- Player interaction
- Game flow (lobby → playing → end)

**TestNetworkPerformance.cpp**
- Replication rate performance
- Bandwidth optimization
- Sync interval efficiency
- Lag compensation
- Variable sync stress tests

### 2. Integration Tests

#### Local Multiplayer Testing

**Setup PIE Testing**
1. Edit → Editor Preferences → Play
2. Set "Number of Players" to 2-4
3. Set "Net Mode" to "Play As Listen Server"
4. Enable "Run Under One Process"

**Test Scenarios**

**Scenario 1: Session Creation & Join**
```
1. Player 1 creates session
2. Player 2 finds sessions
3. Player 2 joins session
4. Verify both players in lobby
5. Start game
6. Verify game starts for both
```

**Scenario 2: Player State Sync**
```
1. Start multiplayer game
2. Player 1 takes damage
3. Verify Player 2 sees health change
4. Player 1 picks up item
5. Verify Player 2 sees inventory update
6. Player 1 hides
7. Verify Player 2 sees hiding state
```

**Scenario 3: Game State Sync**
```
1. Start multiplayer game
2. Complete objective on Player 1
3. Verify objective count updates for all
4. Timer counts down
5. Verify timer synced across clients
6. Game ends
7. Verify end state for all players
```

**Scenario 4: Disconnection Handling**
```
1. Start game with 3 players
2. Player 2 disconnects
3. Verify game continues
4. Verify player list updates
5. Player 2 reconnects
6. Verify rejoin works
```

#### Network Condition Testing

**Latency Testing**
```
# In PIE, use console command
Net PktLag=100  // 100ms latency
Net PktLagVariance=20  // ±20ms variance
```

**Packet Loss Testing**
```
Net PktLoss=5  // 5% packet loss
Net PktOrder=1  // Enable packet reordering
```

**Bandwidth Limiting**
```
Net PktIncomingLagMax=100
Net PktIncomingLossMax=5
```

**Test Matrix**
| Latency | Packet Loss | Expected Result |
|---------|-------------|-----------------|
| 0ms     | 0%          | Perfect sync    |
| 50ms    | 0%          | Smooth gameplay |
| 100ms   | 2%          | Acceptable      |
| 150ms   | 5%          | Playable        |
| 200ms   | 10%         | Degraded        |
| 300ms+  | 15%+        | Unplayable      |

### 3. Performance Testing

#### Bandwidth Monitoring

**Enable Network Stats**
```cpp
// In console
stat net
stat netplayermovement
```

**Custom Bandwidth Logging**
```cpp
void LogNetworkStats()
{
    if (UNetworkSyncComponent* SyncComp = GetComponentByClass<UNetworkSyncComponent>())
    {
        float Bandwidth = SyncComp->GetBandwidthUsage();
        UE_LOG(LogTemp, Log, TEXT("Bandwidth: %.2f KB/s"), Bandwidth);
    }
}
```

**Performance Targets**
- Bandwidth per client: < 50 KB/s
- Server CPU usage: < 50%
- Client CPU usage: < 40%
- Memory usage: < 2 GB
- Frame rate: > 60 FPS

#### Stress Testing

**Max Players Test**
```
1. Start server
2. Connect 4 clients
3. All players move simultaneously
4. Monitor bandwidth and CPU
5. Verify smooth gameplay
6. Check for desync
```

**Long Duration Test**
```
1. Start 30-minute game
2. Monitor memory leaks
3. Check for performance degradation
4. Verify no crashes
5. Log any warnings/errors
```

**Rapid Action Test**
```
1. All players spam actions
2. Pick up/drop items rapidly
3. Toggle hiding repeatedly
4. Monitor replication
5. Check for packet overflow
```

### 4. Automated Testing

#### CI/CD Integration

**GitHub Actions Example**
```yaml
name: Network Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v2
      - name: Run Network Tests
        run: |
          UnrealEditor.exe HorrorProject.uproject -ExecCmds="Automation RunTests HorrorProject.Network" -unattended -nopause
```

#### Nightly Tests
```bash
# Run comprehensive tests overnight
./RunNetworkTests.bat --full --report=NetworkTestReport.html
```

### 5. Manual Test Cases

#### Test Case 1: Basic Connectivity
**Steps:**
1. Launch server
2. Launch client
3. Client connects to server
4. Verify connection established

**Expected:** Client successfully connects, appears in player list

**Pass/Fail:** ___________

#### Test Case 2: Player Movement Sync
**Steps:**
1. Connect 2 players
2. Player 1 moves forward
3. Observe on Player 2's screen

**Expected:** Player 1's movement visible to Player 2 with < 100ms delay

**Pass/Fail:** ___________

#### Test Case 3: Inventory Replication
**Steps:**
1. Connect 2 players
2. Player 1 picks up item
3. Check Player 1's inventory
4. Check Player 2's view of Player 1

**Expected:** Item appears in Player 1's inventory, visible to Player 2

**Pass/Fail:** ___________

#### Test Case 4: Objective Sync
**Steps:**
1. Connect 2 players
2. Player 1 completes objective
3. Check objective counter on both clients

**Expected:** Objective count updates for both players

**Pass/Fail:** ___________

#### Test Case 5: Game End Sync
**Steps:**
1. Connect 2 players
2. Complete all objectives
3. Observe game end screen

**Expected:** Both players see victory screen simultaneously

**Pass/Fail:** ___________

#### Test Case 6: Chat System
**Steps:**
1. Connect 2 players
2. Player 1 sends chat message
3. Check Player 2's chat

**Expected:** Message appears on Player 2's screen

**Pass/Fail:** ___________

#### Test Case 7: Voice Chat
**Steps:**
1. Connect 2 players
2. Player 1 starts voice chat
3. Player 1 speaks
4. Player 2 listens

**Expected:** Player 2 hears Player 1's voice

**Pass/Fail:** ___________

#### Test Case 8: Lag Compensation
**Steps:**
1. Connect 2 players
2. Add 150ms latency
3. Player 1 shoots at Player 2
4. Check hit registration

**Expected:** Hits register correctly despite latency

**Pass/Fail:** ___________

### 6. Debugging Tools

#### Network Logging
```cpp
// Enable verbose network logging
LogNet: Verbose
LogNetPlayerMovement: Verbose
LogNetTraffic: Verbose
```

#### Visual Debugging
```cpp
// Show network debug info
ShowDebug Net
ShowDebug NetPlayerMovement
```

#### Packet Capture
```bash
# Use Wireshark to capture packets
# Filter: udp.port == 7777
```

### 7. Common Issues & Solutions

#### Issue: Players Desync
**Symptoms:** Players see different game states
**Debug:**
- Check replication settings
- Verify `bReplicates = true`
- Check `GetLifetimeReplicatedProps`
**Solution:** Ensure all critical variables replicated

#### Issue: High Latency
**Symptoms:** Delayed actions, rubber-banding
**Debug:**
- Check `stat net`
- Monitor bandwidth usage
- Check server tick rate
**Solution:** Optimize replication rate, enable delta compression

#### Issue: Connection Timeout
**Symptoms:** Clients disconnect randomly
**Debug:**
- Check firewall settings
- Verify port forwarding
- Check network stability
**Solution:** Increase timeout values, improve connection handling

#### Issue: Packet Loss
**Symptoms:** Choppy movement, missing updates
**Debug:**
- Use `Net PktLoss` to simulate
- Check network quality
- Monitor packet statistics
**Solution:** Implement better interpolation, use unreliable RPCs

### 8. Test Reporting

#### Test Report Template
```
Network Test Report
Date: __________
Tester: __________
Build: __________

Test Environment:
- Players: ___
- Network: LAN / Internet
- Latency: ___ ms
- Packet Loss: ___ %

Test Results:
- Unit Tests: Pass / Fail
- Integration Tests: Pass / Fail
- Performance Tests: Pass / Fail

Issues Found:
1. ___________
2. ___________

Recommendations:
1. ___________
2. ___________
```

### 9. Continuous Monitoring

#### Production Monitoring
```cpp
// Log network metrics
void LogProductionMetrics()
{
    UE_LOG(LogTemp, Log, TEXT("Active Players: %d"), GetNumPlayers());
    UE_LOG(LogTemp, Log, TEXT("Average Ping: %.2f ms"), GetAveragePing());
    UE_LOG(LogTemp, Log, TEXT("Bandwidth: %.2f KB/s"), GetBandwidth());
}
```

#### Analytics Integration
```cpp
// Send metrics to analytics service
void SendNetworkAnalytics()
{
    FAnalyticsEventAttribute Attributes[] = {
        FAnalyticsEventAttribute(TEXT("Players"), GetNumPlayers()),
        FAnalyticsEventAttribute(TEXT("Ping"), GetAveragePing()),
        FAnalyticsEventAttribute(TEXT("Bandwidth"), GetBandwidth())
    };
    
    Analytics->RecordEvent(TEXT("NetworkMetrics"), Attributes);
}
```

## Best Practices

1. **Test Early**: Test networking from day one
2. **Automate**: Use automated tests for regression
3. **Simulate Conditions**: Test with realistic network conditions
4. **Monitor Production**: Track metrics in live environment
5. **Document Issues**: Keep detailed bug reports
6. **Iterate**: Continuously improve based on findings

## Checklist

- [ ] All unit tests passing
- [ ] Integration tests completed
- [ ] Performance targets met
- [ ] Stress tests passed
- [ ] Manual test cases verified
- [ ] Network conditions tested
- [ ] Debugging tools configured
- [ ] Test reports generated
- [ ] Issues documented
- [ ] Fixes verified
