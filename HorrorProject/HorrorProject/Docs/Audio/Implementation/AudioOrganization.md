# Audio Organization Guide

## Overview

Complete organizational structure for the HorrorProject audio system, including folder hierarchy, naming conventions, and asset management strategies.

## Directory Structure

### Complete Folder Hierarchy

```
Content/Audio/
├── Ambient/
│   ├── Station/
│   │   ├── ambient_station_base_01
│   │   ├── ambient_station_base_02
│   │   ├── ambient_machinery_01
│   │   ├── ambient_machinery_02
│   │   ├── ambient_water_drip_01
│   │   └── ambient_water_flow_01
│   ├── Underwater/
│   │   └── ambient_underwater_01
│   └── Tension/
│       └── ambient_tension_01
├── Footsteps/
│   ├── Concrete/
│   │   ├── footstep_concrete_01
│   │   ├── footstep_concrete_02
│   │   └── footstep_concrete_03
│   ├── Metal/
│   │   ├── footstep_metal_01
│   │   ├── footstep_metal_02
│   │   └── footstep_metal_03
│   ├── Water/
│   │   ├── footstep_water_01
│   │   ├── footstep_water_02
│   │   └── footstep_water_03
│   ├── Grate/
│   │   ├── footstep_grate_01
│   │   ├── footstep_grate_02
│   │   └── footstep_grate_03
│   └── Carpet/
│       ├── footstep_carpet_01
│       ├── footstep_carpet_02
│       └── footstep_carpet_03
├── Breathing/
│   ├── Normal/
│   │   └── breathing_normal_loop
│   ├── Heavy/
│   │   └── breathing_heavy_loop
│   ├── Panic/
│   │   └── breathing_panic_loop
│   ├── LowOxygen/
│   │   └── breathing_low_oxygen_loop
│   └── breathing_hold_breath
├── Underwater/
│   ├── Ambience/
│   │   └── underwater_ambience_loop
│   ├── Bubbles/
│   │   └── underwater_bubbles_01
│   ├── Movement/
│   │   └── underwater_movement_loop
│   ├── underwater_enter
│   └── underwater_exit
├── SoundCues/
│   ├── Ambient/
│   │   ├── SC_Ambient_Station_Base
│   │   ├── SC_Ambient_Station_Machinery
│   │   ├── SC_Ambient_Station_Water
│   │   ├── SC_Ambient_Underwater
│   │   └── SC_Ambient_Tension
│   ├── Footsteps/
│   │   ├── SC_Footstep_Concrete
│   │   ├── SC_Footstep_Metal
│   │   ├── SC_Footstep_Water
│   │   ├── SC_Footstep_Grate
│   │   └── SC_Footstep_Carpet
│   ├── Breathing/
│   │   ├── SC_Breathing_Normal
│   │   ├── SC_Breathing_Heavy
│   │   ├── SC_Breathing_Panic
│   │   ├── SC_Breathing_LowOxygen
│   │   └── SC_Breathing_HoldBreath
│   └── Underwater/
│       ├── SC_Underwater_Ambience
│       ├── SC_Underwater_Bubbles
│       ├── SC_Underwater_Movement
│       ├── SC_Underwater_Enter
│       └── SC_Underwater_Exit
├── Attenuation/
│   ├── ATT_Ambient_Large
│   ├── ATT_Ambient_Medium
│   ├── ATT_Footsteps
│   ├── ATT_Breathing
│   └── ATT_Underwater
├── Mixes/
│   ├── Mix_Underwater
│   ├── Mix_Tension
│   └── Mix_Pause
├── Concurrency/
│   ├── Concurrency_Footsteps
│   ├── Concurrency_Ambient
│   └── Concurrency_Bubbles
└── Submixes/
    ├── Submix_Ambient
    ├── Submix_SFX
    ├── Submix_Underwater
    └── Submix_Voice
```

## Naming Conventions

### Sound Wave Assets

**Format:** `[category]_[type]_[variation]`

**Examples:**
- `ambient_station_base_01`
- `footstep_concrete_02`
- `breathing_normal_loop`
- `underwater_enter`

**Rules:**
- All lowercase
- Underscores for spaces
- Numbers for variations (01, 02, 03)
- Suffix `_loop` for looping files
- No special characters

### Sound Cue Assets

**Format:** `SC_[Category]_[Type]`

**Examples:**
- `SC_Ambient_Station_Base`
- `SC_Footstep_Concrete`
- `SC_Breathing_Normal`
- `SC_Underwater_Ambience`

**Rules:**
- Prefix: `SC_` (Sound Cue)
- PascalCase for category and type
- Descriptive names
- Match source audio category

### Attenuation Assets

**Format:** `ATT_[Category]_[Size/Type]`

**Examples:**
- `ATT_Ambient_Large`
- `ATT_Ambient_Medium`
- `ATT_Footsteps`
- `ATT_Breathing`

**Rules:**
- Prefix: `ATT_` (Attenuation)
- PascalCase
- Include size or type descriptor
- Reusable across similar sounds

### Sound Mix Assets

**Format:** `Mix_[State/Context]`

**Examples:**
- `Mix_Underwater`
- `Mix_Tension`
- `Mix_Pause`

**Rules:**
- Prefix: `Mix_`
- PascalCase
- Describe game state or context
- Short, descriptive names

### Concurrency Assets

**Format:** `Concurrency_[Category]`

**Examples:**
- `Concurrency_Footsteps`
- `Concurrency_Ambient`
- `Concurrency_Bubbles`

**Rules:**
- Prefix: `Concurrency_`
- PascalCase
- Match audio category
- Describe what it limits

### Submix Assets

**Format:** `Submix_[Category]`

**Examples:**
- `Submix_Ambient`
- `Submix_SFX`
- `Submix_Underwater`

**Rules:**
- Prefix: `Submix_`
- PascalCase
- Match sound class category
- Hierarchical structure

## Folder Organization Principles

### 1. Category-Based Organization

**Primary folders by audio category:**
- Ambient
- Footsteps
- Breathing
- Underwater
- Music (future)
- Dialogue (future)
- UI (future)

**Benefits:**
- Easy to find related sounds
- Clear separation of concerns
- Scalable for future additions

### 2. Type-Based Subfolders

**Within each category, organize by type:**
- Ambient → Station, Underwater, Tension
- Footsteps → Concrete, Metal, Water, Grate, Carpet
- Breathing → Normal, Heavy, Panic, LowOxygen

**Benefits:**
- Further organization
- Group variations together
- Clear hierarchy

### 3. Asset Type Separation

**Separate folders for different asset types:**
- Raw audio → Category folders
- Sound Cues → SoundCues folder
- Attenuation → Attenuation folder
- Mixes → Mixes folder

**Benefits:**
- Clear asset type identification
- Easier to manage
- Prevents clutter

## Asset Management Strategies

### 1. Version Control

**What to Track:**
- All Sound Wave assets
- All Sound Cue assets
- All Attenuation assets
- All Mix assets
- Blueprint references

**What NOT to Track:**
- Temporary files
- Cache files
- Derived data

**Git Configuration:**
```gitignore
# Track audio assets
!/Content/Audio/**/*.uasset

# Ignore temporary files
/Content/Audio/**/*_temp.*
/Content/Audio/**/*.tmp
```

### 2. Asset References

**Track Dependencies:**
- Which Sound Cues use which Sound Waves
- Which Blueprints use which Sound Cues
- Which components use which assets

**Use Reference Viewer:**
1. Right-click asset
2. Reference Viewer
3. See all dependencies

### 3. Asset Tagging

**Tag System:**
- Category tags: `Audio.Ambient`, `Audio.Footsteps`
- Quality tags: `Quality.High`, `Quality.Medium`
- State tags: `State.Loop`, `State.OneShot`
- Platform tags: `Platform.PC`, `Platform.Console`

**Benefits:**
- Easy searching
- Bulk operations
- Asset management

### 4. Collections

**Create Collections for:**
- All ambient sounds
- All footstep sounds
- All breathing sounds
- All underwater sounds
- All looping sounds
- All streaming sounds

**How to Create:**
1. Content Browser → Collections
2. New Collection → "Ambient_Sounds"
3. Add assets to collection
4. Use for bulk operations

## Search and Filter Strategies

### Content Browser Filters

**By Type:**
- Sound Wave
- Sound Cue
- Sound Attenuation
- Sound Mix

**By Path:**
- `/Game/Audio/Ambient/`
- `/Game/Audio/Footsteps/`
- `/Game/Audio/SoundCues/`

**By Name:**
- Search: `ambient_`
- Search: `SC_Footstep`
- Search: `ATT_`

### Advanced Filters

**By Property:**
- Looping: Yes/No
- Streaming: Yes/No
- Compression Quality: 40-60
- Duration: > 5 seconds

**By Usage:**
- Referenced by Blueprints
- Used in Sound Cues
- Unused assets

## Maintenance Procedures

### Weekly Maintenance

**Tasks:**
1. Check for unused assets
2. Verify all references are valid
3. Update documentation
4. Review asset sizes
5. Check for duplicates

**Tools:**
- Reference Viewer
- Size Map
- Asset Audit
- Find in Content Browser

### Monthly Maintenance

**Tasks:**
1. Optimize compression settings
2. Review streaming settings
3. Clean up temporary files
4. Update collections
5. Audit memory usage

### Quarterly Maintenance

**Tasks:**
1. Major reorganization if needed
2. Archive old assets
3. Update naming conventions
4. Review folder structure
5. Performance audit

## Scalability Planning

### Future Categories

**Planned Additions:**
- Music/
- Dialogue/
- UI/
- Interactions/
- Impacts/
- Weapons/
- Creatures/

**Organization:**
- Follow same structure
- Maintain naming conventions
- Create appropriate subfolders
- Add to collections

### Growth Management

**When to Split Folders:**
- More than 20 assets in folder
- Multiple distinct types
- Different use cases
- Performance concerns

**When to Merge Folders:**
- Less than 5 assets total
- Very similar types
- Same use case
- Simplification needed

## Blueprint Integration

### Component Organization

**Player Character:**
```
BP_PlayerCharacter
├── FootstepAudioComponent
│   └── References: SC_Footstep_*
└── BreathingAudioComponent
    └── References: SC_Breathing_*
```

**Audio Zones:**
```
BP_AudioZone_Station
└── AmbientAudioComponent
    └── References: SC_Ambient_Station_*

BP_AudioZone_Underwater
└── UnderwaterAudioComponent
    └── References: SC_Underwater_*
```

### Reference Management

**Best Practices:**
- Use soft references for large assets
- Use hard references for critical assets
- Async load when possible
- Unload when not needed

## Performance Organization

### Memory Tiers

**Tier 1: Always Loaded (< 2 MB)**
- Player breathing
- Footsteps
- Critical UI sounds

**Tier 2: Load on Demand (2-5 MB)**
- Ambient sounds (when in zone)
- Underwater sounds (when underwater)
- Interaction sounds

**Tier 3: Streaming (> 5 MB)**
- Long ambient loops
- Music
- Dialogue

### Streaming Groups

**Group 1: Station Ambient**
- ambient_station_base_*
- ambient_machinery_*
- ambient_water_*

**Group 2: Underwater**
- underwater_ambience_loop
- ambient_underwater_01

**Group 3: Music**
- ambient_tension_01
- (future music tracks)

## Documentation Organization

### Per-Asset Documentation

**Include in Asset:**
- Description
- Usage notes
- Technical specs
- Dependencies
- Version history

**How to Add:**
1. Open asset
2. Asset Details → Description
3. Add comprehensive notes
4. Save

### Category Documentation

**Create README files:**
- `Audio/Ambient/README.md`
- `Audio/Footsteps/README.md`
- `Audio/Breathing/README.md`

**Include:**
- Category overview
- Asset list
- Usage guidelines
- Technical requirements

## Quality Assurance

### Organization Checklist

**Folder Structure:**
- [ ] All folders follow naming conventions
- [ ] No orphaned assets
- [ ] Logical hierarchy
- [ ] Consistent depth

**Asset Naming:**
- [ ] All assets follow conventions
- [ ] No duplicate names
- [ ] Descriptive names
- [ ] Consistent formatting

**References:**
- [ ] All references valid
- [ ] No circular dependencies
- [ ] Proper soft/hard references
- [ ] Documented dependencies

**Documentation:**
- [ ] All assets documented
- [ ] README files present
- [ ] Usage notes clear
- [ ] Technical specs accurate

## Migration and Refactoring

### When to Refactor

**Triggers:**
- More than 100 audio assets
- Folder depth > 4 levels
- Naming inconsistencies
- Performance issues
- Team feedback

### Refactoring Process

**Steps:**
1. Audit current organization
2. Plan new structure
3. Create new folders
4. Move assets (use redirectors)
5. Update references
6. Test thoroughly
7. Remove old folders
8. Update documentation

**Tools:**
- Asset Renamer
- Folder Mover
- Reference Updater
- Redirector Fixer

## Team Collaboration

### Folder Ownership

**Assign Owners:**
- Ambient: Audio Designer A
- Footsteps: Audio Designer B
- Breathing: Audio Designer A
- Underwater: Audio Designer B

**Responsibilities:**
- Maintain organization
- Review new assets
- Update documentation
- Quality assurance

### Workflow

**Adding New Assets:**
1. Create in appropriate folder
2. Follow naming conventions
3. Add to collections
4. Tag appropriately
5. Document
6. Notify team
7. Commit to version control

**Modifying Assets:**
1. Check out from version control
2. Make changes
3. Test thoroughly
4. Update documentation
5. Notify team
6. Commit changes

## Troubleshooting

### Can't Find Asset

**Solutions:**
1. Use Content Browser search
2. Check collections
3. Use Reference Viewer
4. Check recently modified
5. Search by type

### Broken References

**Solutions:**
1. Use Reference Viewer
2. Find redirectors
3. Fix redirectors
4. Update references
5. Test in-game

### Duplicate Assets

**Solutions:**
1. Use Size Map to find duplicates
2. Compare assets
3. Consolidate to one
4. Update all references
5. Delete duplicate

### Disorganized Folders

**Solutions:**
1. Create proper structure
2. Move assets systematically
3. Use redirectors
4. Update documentation
5. Test thoroughly

## Best Practices Summary

1. **Consistent Naming**: Always follow conventions
2. **Logical Hierarchy**: Keep folder depth reasonable
3. **Clear Separation**: Separate by category and type
4. **Documentation**: Document everything
5. **Collections**: Use for organization
6. **Tags**: Tag for easy searching
7. **References**: Track dependencies
8. **Maintenance**: Regular cleanup
9. **Version Control**: Track all assets
10. **Team Communication**: Keep team informed

## Quick Reference

### Common Paths

```
Ambient Sounds: /Game/Audio/Ambient/
Footstep Sounds: /Game/Audio/Footsteps/
Breathing Sounds: /Game/Audio/Breathing/
Underwater Sounds: /Game/Audio/Underwater/
Sound Cues: /Game/Audio/SoundCues/
Attenuation: /Game/Audio/Attenuation/
Mixes: /Game/Audio/Mixes/
```

### Common Prefixes

```
SC_   - Sound Cue
ATT_  - Attenuation
Mix_  - Sound Mix
Concurrency_ - Concurrency Settings
Submix_ - Submix
```

### Asset Counts

```
Total Sound Waves: 33
Total Sound Cues: 20
Total Attenuation: 5
Total Mixes: 3
Total Folders: 25+
```
