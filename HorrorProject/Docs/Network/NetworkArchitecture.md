# Network Architecture

## Overview
HorrorProject's multiplayer network architecture is built on Unreal Engine's robust replication system, designed to support 2-4 player cooperative gameplay with optimized bandwidth usage and lag compensation.

## Core Components

### 1. Network Replication System
- **NetworkReplicationComponent**: Handles actor transform replication with configurable rates
- **ReplicatedPlayerState**: Manages player-specific data (health, stamina, fear, inventory)
- **ReplicatedGameState**: Manages global game state (objectives, timers, player list)
- **NetworkSyncComponent**: Generic variable synchronization with bandwidth optimization

### 2. Game Mode & Controllers
- **HorrorGameModeMultiplayer**: Server-authoritative game mode managing sessions and match flow
- **HorrorPlayerControllerMultiplayer**: Client controller with input replication and lag compensation
- **MultiplayerSessionSubsystem**: Session management for creating/finding/joining games

## Network Topology

```
Server (Authority)
    ├── HorrorGameModeMultiplayer
    ├── ReplicatedGameState
    └── Connected Clients (2-4)
        ├── HorrorPlayerControllerMultiplayer
        ├── ReplicatedPlayerState
        └── NetworkReplicationComponent
```

## Replication Strategy

### Server Authority
- All gameplay logic executes on server
- Server validates client inputs
- Server broadcasts state changes to clients

### Client Prediction
- Clients predict movement locally
- Server reconciles predictions
- Interpolation smooths corrections

### Replication Rates
- **Transform**: 30 Hz (configurable)
- **Player State**: 10 Hz
- **Game State**: 5 Hz
- **Events**: Immediate (reliable RPC)

## Data Flow

### Player Input
1. Client captures input
2. Client predicts movement locally
3. Client sends input to server via `ServerSendInput`
4. Server validates and applies input
5. Server replicates authoritative state
6. Client reconciles with server state

### Game Events
1. Event occurs on server
2. Server updates replicated variables
3. `OnRep_` functions trigger on clients
4. Clients update UI and local state

### Inventory & Interactions
1. Client requests action via Server RPC
2. Server validates request
3. Server modifies state
4. State replicates to all clients
5. Clients receive `OnRep_` notifications

## Bandwidth Optimization

### Delta Compression
- Only replicate changed values
- Position/rotation tolerance thresholds
- Reduces redundant data transmission

### Variable Replication
- Conditional replication based on relevancy
- Dormancy for inactive actors
- Priority-based replication

### Network Culling
- Distance-based relevancy
- Interest management
- Adaptive update rates

## Lag Compensation

### Client-Side Prediction
- Immediate input response
- Predictive movement
- Server reconciliation

### Interpolation
- Smooth position updates
- Configurable interpolation speed
- Handles packet loss gracefully

### Time Synchronization
- Server timestamp on replicated data
- Client calculates latency
- Compensates for round-trip time

## Security

### Server Validation
- All RPCs validated with `_Validate` functions
- Input sanity checks
- Anti-cheat measures

### Authority Checks
- `HasAuthority()` guards state modifications
- Client requests, server decides
- No client-side authority

## Performance Targets

- **Max Players**: 4
- **Target Tick Rate**: 30 Hz
- **Max Latency**: 150ms
- **Bandwidth per Client**: < 50 KB/s
- **Packet Loss Tolerance**: < 5%

## Network Modes

### Standalone
- Single-player mode
- No network overhead
- Full local authority

### Listen Server
- Host player acts as server
- Reduced latency for host
- Suitable for co-op

### Dedicated Server
- Separate server process
- Equal latency for all players
- Best for competitive play

## Future Enhancements

- Voice chat integration
- Spectator mode
- Server browser
- Matchmaking system
- Cross-platform support
- Cloud save synchronization
