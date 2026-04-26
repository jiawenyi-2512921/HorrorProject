# Events Reference

## Overview

This document provides a comprehensive reference for all analytics events tracked in the Horror Project.

## Event Categories

### Navigation Events

Events related to screen navigation and UI interaction.

#### `screen_view`

Tracks when a user views a screen.

**Parameters:**
- `screen_name` (string): Name of the screen viewed

**Example:**
```cpp
Analytics->TrackScreenView(TEXT("MainMenu"));
```

#### `menu_opened`

Tracks when a menu is opened.

**Parameters:**
- `menu_name` (string): Name of the menu
- `source` (string): Where the menu was opened from

#### `menu_closed`

Tracks when a menu is closed.

**Parameters:**
- `menu_name` (string): Name of the menu
- `duration` (float): How long the menu was open (seconds)

### Gameplay Events

Events related to core gameplay mechanics.

#### `level_start`

Tracks when a level starts.

**Parameters:**
- `level_name` (string): Name of the level
- `difficulty` (string): Selected difficulty
- `attempt_number` (int): Number of attempts on this level

**Example:**
```cpp
TMap<FString, FString> Parameters;
Parameters.Add(TEXT("level_name"), TEXT("Level_1"));
Parameters.Add(TEXT("difficulty"), TEXT("Normal"));
Parameters.Add(TEXT("attempt_number"), TEXT("1"));
Analytics->TrackEvent(TEXT("level_start"), Parameters);
```

#### `level_complete`

Tracks when a level is completed.

**Parameters:**
- `level_name` (string): Name of the level
- `completion_time` (float): Time taken to complete (seconds)
- `deaths` (int): Number of deaths during the level
- `score` (int): Final score
- `collectibles_found` (int): Number of collectibles found

**Example:**
```cpp
TMap<FString, FString> Parameters;
Parameters.Add(TEXT("level_name"), TEXT("Level_1"));
Parameters.Add(TEXT("completion_time"), FString::Printf(TEXT("%.2f"), CompletionTime));
Parameters.Add(TEXT("deaths"), FString::FromInt(DeathCount));
Analytics->TrackEvent(TEXT("level_complete"), Parameters);
```

#### `level_failed`

Tracks when a level is failed or abandoned.

**Parameters:**
- `level_name` (string): Name of the level
- `reason` (string): Reason for failure (death, quit, timeout)
- `progress` (float): Percentage of level completed (0-100)

#### `checkpoint_reached`

Tracks when a checkpoint is reached.

**Parameters:**
- `level_name` (string the level
- `checkpoint_id` (string): Checkpoint identifier
- `time_elapsed` (float): Time since level start

### Combat Events

Events related to combat and enemy interactions.

#### `enemy_encountered`

Tracks when an enemy is first encountered.

**Parameters:**
- `enemy_type` (string): Type of enemy
- `level_name` (string): Current level
- `player_health` (float): Player health percentage

#### `enemy_defeated`

Tracks when an enemy is defeated.

**Parameters:**
- `enemy_type` (string): Type of enemy
- `weapon_used` (string): Weapon used to defeat
- `combat_duration` (float): Duration of combat (seconds)

**Example:**
```cpp
GameplayTelemetry->RecordEnemyDefeated(TEXT("Zombie"));
```

#### `player_death`

Tracks when the player dies.

**Parameters:**
- `cause_of_death` (string): What killed the player
- `level_name` (string): Current level
- `location` (string): Death location
- `time_alive` (float): Time alive in current life

**Example:**
```cpp
GameplayTelemetry->RecordDeath(TEXT("Zombie"));
```

### Progression Events

Events related to player progression and achievements.

#### `achievement_unlocked`

Tracks when an achievement is unlocked.

**Parameters:**
- `achievement_id` (string): Achievement identifier
- `achievement_name` (string): Achievement display name
- `rarity` (string): Achievement rarity (common, rare, epic, legendary)

**Example:**
```cpp
GameplayTelemetry->RecordAchievement(TEXT("ACH_FIRST_LEVEL"));
```

#### `item_collected`

Tracks when an item is collected.

**Parameters:**
- `item_id` (string): Item identifier
- `item_type` (string): Type of item (weapon, health, key, collectible)
- `level_name` (string): Current level

**Example:**
```cpp
GameplayTelemetry->RecordItemCollected(TEXT("Key_Red"));
```

#### `item_used`

Tracks when an item is used.

**Parameters:**
- `item_id` (string): Item identifier
- `item_type` (string): Type of item
- `context` (string): Usage context

#### `upgrade_purchased`

Tracks when an upgrade is purchased.

**Parameters:**
- `upgrade_id` (string): Upgrade identifier
- `upgrade_type` (string): Type of upgrade
- `cost` (int): Cost of upgrade
- `currency_type` (string): Currency used

### Social Events

Events related to social features.

#### `share_initiated`

Tracks when a share action is initiated.

**Parameters:**
- `content_type` (string): Type of content shared
- `platform` (string): Social platform
- `source` (string): Where share was initiated

#### `multiplayer_joined`

Tracks when a multiplayer session is joined.

**Parameters:**
- `session_type` (string): Type of session
- `player_count` (int): Number of players
- `is_host` (bool): Whether player is host

### Configuration Events

Events related to settings and configuration.

#### `settings_changed`

Tracks when settings are changed.

**Parameters:**
- `setting_name` (string): Name of setting
- `old_value` (string): Previous value
- `new_value` (string): New value
- `category` (string): Settings category

#### `graphics_preset_changed`

Tracks when graphics preset is changed.

**Parameters:**
- `preset` (string): Graphics preset (low, medium, high, ultra)
- `custom` (bool): Whether using custom settings

#### `audio_settings_changed`

Tracks when audio settings are changed.

**Parameters:**
- `master_volume` (float): Master volume (0-1)
- `music_volume` (float): Music volume (0-1)
- `sfx_volume` (float): SFX volume (0-1)

### Error Events

Events related to errors and issues.

#### `error`

Tracks general errors.

**Parameters:**
- `error_message` (string): Error message
- `error_code` (string): Error code
- `severity` (string): Error severity

**Example:**
```cpp
Analytics->TrackError(TEXT("Failed to load save file"), TEXT("ERR_SAVE_001"));
```

#### `crash`

Tracks application crashes.

**Parameters:**
- `crash_reason` (string): Reason for crash
- `stack_trace` (string): Stack trace
- `memory_usage` (float): Memory usage at crash

#### `performance_warning`

Tracks performance issues.

**Parameters:**
- `warning_type` (string): Type of warning (low_fps, high_memory, etc.)
- `value` (float): Measured value
- `threshold` (float): Warning threshold

### Monetization Events

Events related to in-app purchases and monetization.

#### `store_opened`

Tracks when the store is opened.

**Parameters:**
- `source` (string): Where store was opened from

#### `purchase_initiated`

Tracks when a purchase is initiated.

**Parameters:**
- `product_id` (string): Product identifier
- `product_type` (string): Type of product
- `price` (float): Product price
- `currency` (string): Currency code

#### `purchase_completed`

Tracks when a purchase is completed.

**Parameters:**
- `product_id` (string): Product identifier
- `transaction_id` (string): Transaction identifier
- `revenue` (float): Revenue amount
- `currency` (string): Currency code

#### `purchase_failed`

Tracks when a purchase fails.

**Parameters:**
- `product_id` (string): Product identifier
- `error_code` (string): Error code
- `error_message` (string): Error message

### Tutorial Events

Events related to tutorial and onboarding.

#### `tutorial_started`

Tracks when tutorial starts.

**Parameters:**
- `tutorial_id` (string): Tutorial identifier
- `is_first_time` (bool): Whether first time playing

#### `tutorial_step_completed`

Tracks when a tutorial step is completed.

**Parameters:**
- `tutorial_id` (string): Tutorial identifier
- `step_number` (int): Step number
- `step_name` (string): Step name
- `time_taken` (float): Time taken for step

#### `tutorial_skipped`

Tracks when tutorial is skipped.

**Parameters:**
- `tutorial_id` (string): Tutorial identifier
- `step_number` (int): Step where skipped

## Event Naming Conventions

1. Use lowercase with underscores: `level_complete`
2. Use verb-noun format: `enemy_defeated`, `item_collected`
3. Be specific but concise: `graphics_preset_changed` not `settings_changed`
4. Group related events: `purchase_initiated`, `purchase_completed`, `purchase_failed`

## Parameter Guidelines

1. **Consistent Types**: Use consistent data types for parameters
2. **Descriptive Names**: Use clear, descriptive parameter names
3. **Standard Values**: Use standard values where possible (e.g., difficulty: "Easy", "Normal", "Hard")
4. **No PII**: Never include personally identifiable information
5. **Reasonable Size**: Keep parameter values reasonably sized

## Custom Events

To add custom events:

```cpp
TMap<FString, FString> Parameters;
Parameters.Add(TEXT("param1"), TEXT("value1"));
Parameters.Add(TEXT("param2"), TEXT("value2"));

Analytics->TrackEvent(TEXT("custom_event_name"), Parameters);
```

## Event Frequency Guidelines

- **High Frequency** (multiple per second): Performance metrics
- **Medium Frequency** (multiple per minute): Gameplay events
- **Low Frequency** (few per session): Progression events
- **Very Low Frequency** (once per session): Configuration events

## Privacy Considerations

1. **No Personal Data**: Never track names, emails, or other PII
2. **Anonymized IDs**: Use anonymized user IDs
3. **Consent Required**: Only track events with user consent
4. **Data Minimization**: Only collect necessary data
5. **Secure Storage**: Store data securely

## Testing Events

Use the Analytics Subsystem test suite to verify events:

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCustomEventTest, "HorrorProject.Analytics.CustomEvent", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCustomEventTest::RunTest(const FString& Parameters)
{
    UAnalyticsSubsystem* Analytics = NewObject<UAnalyticsSubsystem>();
    
    FUserConsent Consent;
    Consent.bAnalyticsEnabled = true;
    Analytics->SetUserConsent(Consent);
    
    TMap<FString, FString> Params;
    Params.Add(TEXT("test_param"), TEXT("test_value"));
    Analytics->TrackEvent(TEXT("test_event"), Params);
    
    return true;
}
```

## Event Validation

Before deploying new events:

1. ✓ Event name follows naming conventions
2. ✓ Parameters are well-defined
3. ✓ No PII in parameters
4. ✓ Event is documented
5. ✓ Event is tested
6. ✓ Event frequency is appropriate
7. ✓ Event provides actionable insights

## Common Patterns

### Timed Events

```cpp
// Start timing
float StartTime = GetWorld()->GetTimeSeconds();

// ... action occurs ...

// Track with duration
TMap<FString, FString> Parameters;
Parameters.Add(TEXT("duration"), FString::Printf(TEXT("%.2f"), 
    GetWorld()->GetTimeSeconds() - StartTime));
Analytics->TrackEvent(TEXT("action_completed"), Parameters);
```

### Funnel Tracking

```cpp
// Step 1
Analytics->TrackEvent(TEXT("funnel_step_1"), Parameters);

// Step 2
Analytics->TrackEvent(TEXT("funnel_step_2"), Parameters);

// Step 3 (conversion)
Analytics->TrackEvent(TEXT("funnel_step_3_complete"), Parameters);
```

### A/B Testing

```cpp
TMap<FString, FString> Parameters;
Parameters.Add(TEXT("variant"), bIsVariantA ? TEXT("A") : TEXT("B"));
Analytics->TrackEvent(TEXT("feature_used"), Parameters);
```
