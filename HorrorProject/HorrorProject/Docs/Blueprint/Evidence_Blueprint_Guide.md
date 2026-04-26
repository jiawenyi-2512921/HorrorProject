# Evidence System Blueprint Implementation Guide

## Overview

The Evidence system provides collectible evidence objects and archive viewing:
- **AEvidenceActor** - Collectible evidence objects in the world
- **UEvidenceCollectionComponent** - Player component for collecting evidence
- **UArchiveViewerWidget** - UI for viewing collected evidence
- **UArchiveSubsystem** - Backend system for evidence management

All classes support Blueprint extension and customization.

## C++ Foundation

### EvidenceActor

**Purpose**: Physical evidence objects that can be collected.

**Key Features**:
- Implements IInteractableInterface
- Stores extended evidence metadata
- Auto-registers with archive system
- Supports hide/destroy on collection
- Blueprint events for customization

**Evidence Types**:
```cpp
enum class EEvidenceCategory : uint8
{
    Document,      // Readable documents
    Photo,         // Photographs
    Audio,         // Audio recordings
    Video,         // Video recordings
    Physical,      // Physical objects
    Digital        // Digital files
};
```

### EvidenceCollectionComponent

**Purpose**: Player component for collecting and managing evidence.

**Key Features**:
- Automatic collection on interaction
- Evidence validation
- Collection events
- Save/load support
- Integration with archive system

### ArchiveSubsystem

**Purpose**: Game-wide evidence management system.

**Key Features**:
- Centralized evidence storage
- Metadata management
- Search and filtering
- Persistence support
- Event broadcasting

## Blueprint Implementation

### Step 1: Create Evidence Actor Blueprints

**Location**: `Content/Blueprints/Evidence/`

#### BP_EvidenceActor_Base

**Parent Class**: EvidenceActor

**Purpose**: Base class for all evidence actors.

**Components**:
- StaticMeshComponent (inherited, name: `MeshComponent`)
- SphereComponent (name: `InteractionRadius`)
- WidgetComponent (name: `InteractionPrompt`)

**Variables**:
- `bGlowWhenNearby` (bool, default: true)
- `GlowColor` (LinearColor, default: Yellow)
- `GlowIntensity` (float, default: 2.0)

**Event Graph**:
```
Event BeginPlay
  → Branch (bAutoRegisterMetadata)
    True:
      → Get Archive Subsystem
      → Register Evidence Metadata (EvidenceMetadata)

On Component Begin Overlap (InteractionRadius)
  → Cast to PlayerCharacter
  → Show Interaction Prompt
  → Branch (bGlowWhenNearby)
    True: Enable Glow Effect

On Component End Overlap (InteractionRadius)
  → Hide Interaction Prompt
  → Disable Glow Effect

BP_OnEvidenceCollected (Collector)
  → Play Sound 2D (Collection Sound)
  → Spawn Emitter at Location (Collection VFX)
  → Branch (bHideOnCollect)
    True: Set Actor Hidden (true)
  → Branch (bDestroyOnCollect)
    True: Destroy Actor
```

#### BP_EvidenceActor_Document

**Parent Class**: BP_EvidenceActor_Base

**Purpose**: Readable document evidence.

**Mesh**: Paper/folder mesh

**Evidence Metadata**:
- Category: Document
- Title: "Document Title"
- Description: "Document description"
- Content: "Full document text"
- Tags: ["Document", "Readable"]

**Additional Variables**:
- `DocumentTexture` (Texture2D) - Document image
- `PageCount` (int32) - Number of pages

**Event Graph**:
```
BP_OnEvidenceCollected (Collector)
  → Parent: BP_OnEvidenceCollected
  → Get UI Manager
  → Show Document Viewer (DocumentTexture, Content)
```

#### BP_EvidenceActor_Photo

**Parent Class**: BP_EvidenceActor_Base

**Purpose**: Photograph evidence.

**Mesh**: Photo/polaroid mesh

**Evidence Metadata**:
- Category: Photo
- Title: "Photo Title"
- Description: "Photo description"
- Thumbnail: Photo texture
- Tags: ["Photo", "Visual"]

**Additional Variables**:
- `PhotoTexture` (Texture2D) - Full photo
- `CaptureLocation` (Vector) - Where photo was taken
- `CaptureDate` (DateTime) - When photo was taken

#### BP_EvidenceActor_Audio

**Parent Class**: BP_EvidenceActor_Base

**Purpose**: Audio recording evidence.

**Mesh**: Tape recorder/cassette mesh

**Evidence Metadata**:
- Category: Audio
- Title: "Audio Recording Title"
- Description: "Recording description"
- Duration: Audio length
- Tags: ["Audio", "Recording"]

**Additional Variables**:
- `AudioFile` (SoundWave) - Audio recording
- `Transcript` (Text) - Audio transcript

**Event Graph**:
```
BP_OnEvidenceCollected (Collector)
  → Parent: BP_OnEvidenceCollected
  → Get UI Manager
  → Show Audio Player (AudioFile, Transcript)
```

#### BP_EvidenceActor_Physical

**Parent Class**: BP_EvidenceActor_Base

**Purpose**: Physical object evidence.

**Mesh**: Various object meshes

**Evidence Metadata**:
- Category: Physical
- Title: "Object Name"
- Description: "Object description"
- Tags: ["Physical", "Object"]

**Additional Variables**:
- `bCanRotateInViewer` (bool, default: true)
- `ViewerScale` (float, default: 1.0)

### Step 2: Create Evidence Collection Component Setup

**In BP_PlayerCharacter**:

**Add Component**:
- EvidenceCollectionComponent (name: `EvidenceCollection`)

**Component Configuration**:
- Max Evidence Capacity: 100
- Auto Collect: true
- Show Collection Toast: true

**Event Graph**:
```
Event BeginPlay
  → EvidenceCollection → Load Collected Evidence

On Evidence Collected Event (EvidenceCollection)
  → Get UI Manager
  → Show Evidence Toast (Evidence Metadata)
  → Play Sound 2D (Collection Sound)
  → Apply Camera Shake (Light shake)

On Evidence Collection Failed Event (EvidenceCollection)
  → Get UI Manager
  → Show Error Toast ("Evidence storage full")
```

**Input Binding** (Interact):
```
Input Action (Interact)
  → Line Trace (from camera)
  → Branch (Hit Actor implements IInteractableInterface)
    True:
      → Call Interact (Hit Actor)
```

### Step 3: Create Archive Viewer Widget

**Already covered in UI_Blueprint_Guide.md**, but here's evidence-specific details:

**WBP_ArchiveMenu** (from UI guide):

**Evidence List Item** (create `WBP_EvidenceListItem`):

**Widget Hierarchy**:
```
Border (Background)
└── HorizontalBox
    ├── Image (Evidence Icon)
    ├── VerticalBox
    │   ├── TextBlock (Evidence Title)
    │   ├── TextBlock (Evidence Category)
    │   └── TextBlock (Collection Date)
    └── Image (New Badge - if recently collected)
```

**Variables**:
- `EvidenceMetadata` (FExtendedEvidenceMetadata)
- `bIsSelected` (bool)
- `bIsNew` (bool)

**Functions**:

#### SetEvidenceData
```
Input: FExtendedEvidenceMetadata Metadata
Set EvidenceMetadata = Metadata
  → Set Evidence Icon (based on category)
  → Set Evidence Title (Metadata.Title)
  → Set Evidence Category (Metadata.Category)
  → Set Collection Date (Metadata.CollectionTimestamp)
  → Set New Badge Visibility (bIsNew)
```

#### SetSelected
```
Input: bool bSelected
Set bIsSelected = bSelected
  → Branch (bSelected)
    True: Set Background Color (Highlight Color)
    False: Set Background Color (Normal Color)
```

### Step 4: Create Evidence Detail Viewer

**Create**: `WBP_EvidenceDetailViewer`

**Purpose**: Shows detailed view of selected evidence.

**Widget Hierarchy**:
```
VerticalBox
├── Image (Evidence Preview)
├── TextBlock (Evidence Title)
├── TextBlock (Evidence Category)
├── Border (Description Panel)
│   └── TextBlock (Evidence Description)
├── Border (Metadata Panel)
│   └── VerticalBox
│       ├── TextBlock (Collection Date)
│       ├── TextBlock (Collection Location)
│       └── TextBlock (Tags)
└── HorizontalBox (Actions)
    ├── Button (View Full)
    ├── Button (Play Audio - if audio)
    └── Button (Close)
```

**Functions**:

#### ShowEvidence
```
Input: FExtendedEvidenceMetadata Metadata
Set Evidence Preview (Metadata.Thumbnail or Icon)
Set Evidence Title (Metadata.Title)
Set Evidence Category (Metadata.Category)
Set Evidence Description (Metadata.Description)
Set Collection Date (Metadata.CollectionTimestamp)
Set Collection Location (Metadata.DiscoveryLocation)
Set Tags (Metadata.Tags)

Branch (Category == Audio)
  True: Show Play Audio Button
  False: Hide Play Audio Button

Branch (Category == Document)
  True: Show View Full Button
  False: Hide View Full Button
```

### Step 5: Create Evidence Placement Tool (Editor Utility)

**Purpose**: Editor tool for placing evidence in levels.

**Create**: `EUW_EvidencePlacementTool` (Editor Utility Widget)

**Widget Hierarchy**:
```
VerticalBox
├── TextBlock (Title: "Evidence Placement Tool")
├── ComboBox (Evidence Type)
├── EditableTextBox (Evidence Title)
├── EditableTextBox (Evidence Description)
├── ComboBox (Evidence Category)
├── Button (Place Evidence)
└── Button (Refresh Evidence List)
```

**Functions**:

#### PlaceEvidence
```
Get Player Camera Location/Rotation
  → Spawn Actor (Selected Evidence Type)
  → Set Evidence Metadata (Title, Description, Category)
  → Place at Camera Forward (500 units)
```

### Step 6: Create Evidence Save/Load System

**In BP_GameMode**:

**Functions**:

#### SaveEvidenceProgress
```
Get Archive Subsystem
  → Get All Collected Evidence
  → Create Save Game Object
  → For Each Evidence:
      → Add Evidence ID to Save Data
  → Save Game to Slot ("EvidenceSave")
```

#### LoadEvidenceProgress
```
Load Game from Slot ("EvidenceSave")
  → Branch (Save Game Exists)
    True:
      → Get Archive Subsystem
      → For Each Evidence ID in Save Data:
          → Mark Evidence as Collected
```

**Event Graph**:
```
Event BeginPlay
  → LoadEvidenceProgress

On Game Saved Event
  → SaveEvidenceProgress
```

## Evidence Metadata Configuration

### FExtendedEvidenceMetadata Structure

**Required Fields**:
- `EvidenceId` (FName) - Unique identifier
- `Title` (FText) - Display name
- `Description` (FText) - Detailed description
- `Category` (EEvidenceCategory) - Evidence type
- `Thumbnail` (Texture2D) - Preview image
- `Tags` (Array<FGameplayTag>) - Search tags

**Optional Fields**:
- `Content` (FText) - Full content (for documents)
- `AudioFile` (SoundWave) - Audio recording
- `VideoFile` (MediaSource) - Video recording
- `RelatedEvidenceIds` (Array<FName>) - Connected evidence
- `DiscoveryLocation` (FVector) - Where found
- `CollectionTimestamp` (FDateTime) - When collected
- `bIsStoryCritical` (bool) - Required for progression
- `UnlockRequirements` (Array<FName>) - Prerequisites

### Evidence Configuration Examples

#### Document Evidence
```
Evidence ID: "Doc_StationLog_001"
Title: "Station Log - Day 47"
Description: "Personal log entry from the station commander"
Category: Document
Content: "Full log text here..."
Tags: ["Document", "Log", "Commander", "Story"]
bIsStoryCritical: true
```

#### Photo Evidence
```
Evidence ID: "Photo_Creature_001"
Title: "Creature Sighting"
Description: "Blurry photo of unknown entity"
Category: Photo
Thumbnail: T_Photo_Creature_001
Tags: ["Photo", "Creature", "Threat"]
Related Evidence: ["Audio_Scream_001", "Doc_Incident_Report"]
```

#### Audio Evidence
```
Evidence ID: "Audio_Distress_001"
Title: "Distress Call"
Description: "Emergency broadcast from lower decks"
Category: Audio
Audio File: SW_Distress_Call_001
Duration: 45.0 seconds
Tags: ["Audio", "Distress", "Emergency"]
Transcript: "This is... [static] ...need help... [static]"
```

## Blueprint Usage Examples

### Example 1: Place Evidence in Level

**In Level Blueprint**:
```
Event BeginPlay
  → Spawn Actor (BP_EvidenceActor_Document)
    - Location: (X, Y, Z)
    - Rotation: (0, 0, 0)
  → Set Evidence Metadata
    - Evidence ID: "Doc_Welcome_001"
    - Title: "Welcome Letter"
    - Description: "Welcome to Deep Water Station"
    - Category: Document
```

### Example 2: Conditional Evidence Unlock

**In BP_ObjectiveNode**:
```
On Objective Completed
  → Get Archive Subsystem
  → Unlock Evidence ("Photo_SecretRoom_001")
  → Get UI Manager
  → Show Evidence Toast ("New evidence unlocked!")
```

### Example 3: Evidence-Based Progression

**In BP_Door**:
```
On Interact
  → Get Archive Subsystem
  → Has Evidence ("Keycard_001")
  → Branch (Has Evidence)
    True: Open Door
    False: Show Message ("Keycard required")
```

### Example 4: Evidence Collection Counter

**In WBP_HUD**:
```
Event Tick
  → Get Archive Subsystem
  → Get Collected Evidence Count
  → Get Total Evidence Count
  → Set Text (Format: "{0}/{1} Evidence", Collected, Total)
```

### Example 5: Evidence Search

**In WBP_ArchiveMenu**:
```
On Search Text Changed
  → Get Archive Subsystem
  → Search Evidence (Search Text)
  → Clear Evidence List
  → For Each Result:
      → Add Evidence List Item (Result)
```

## Event Binding

### EvidenceActor Events

**BP_OnEvidenceCollected** (BlueprintImplementableEvent):
- Called when evidence is collected
- Use for custom collection effects
- Receives collector actor reference

### EvidenceCollectionComponent Events

**OnEvidenceCollected** (Multicast Delegate):
- Broadcasts when evidence collected
- Provides evidence metadata
- Use for UI updates, achievements

**OnEvidenceCollectionFailed** (Multicast Delegate):
- Broadcasts when collection fails
- Provides failure reason
- Use for error messages

### ArchiveSubsystem Events

**OnEvidenceRegistered** (Multicast Delegate):
- Broadcasts when new evidence registered
- Use for tracking total evidence count

**OnEvidenceUnlocked** (Multicast Delegate):
- Broadcasts when evidence unlocked
- Use for progression tracking

## Performance Optimization

### Evidence Actor Optimization
- Use instanced static meshes for multiple evidence
- Disable tick when not near player
- Use simple collision for interaction
- Unload evidence actors when collected
- Pool evidence actors for reuse

### Archive System Optimization
- Cache search results
- Use indexed lookups for evidence IDs
- Lazy-load evidence content
- Compress evidence textures
- Stream large evidence files

### UI Optimization
- Virtualize evidence list (only render visible items)
- Lazy-load evidence thumbnails
- Cache evidence widgets
- Limit evidence detail updates
- Use texture atlases for icons

## Troubleshooting

### Evidence Not Collecting
- Check IInteractableInterface implementation
- Verify EvidenceCollectionComponent is active
- Ensure evidence ID is unique
- Check interaction radius overlap
- Verify CanInteract returns true

### Evidence Not Appearing in Archive
- Check bAutoRegisterMetadata is true
- Verify ArchiveSubsystem is initialized
- Ensure evidence metadata is valid
- Check evidence ID is set
- Verify RegisterEvidence called

### Evidence Thumbnails Not Loading
- Check texture is assigned
- Verify texture is not corrupted
- Ensure texture is compressed correctly
- Check texture streaming settings
- Verify texture is in correct format

### Save/Load Not Working
- Check save game slot name matches
- Verify save game object is created
- Ensure evidence IDs are saved
- Check load is called on game start
- Verify save directory permissions

## Best Practices

1. **Unique IDs**: Always use unique evidence IDs
2. **Metadata Complete**: Fill all required metadata fields
3. **Thumbnails**: Provide clear, recognizable thumbnails
4. **Descriptions**: Write clear, concise descriptions
5. **Tags**: Use consistent tagging system
6. **Story Integration**: Link evidence to story progression
7. **Visual Feedback**: Provide clear collection feedback
8. **Accessibility**: Support text-to-speech for evidence
9. **Organization**: Group related evidence with tags
10. **Testing**: Test all evidence can be collected

## Testing Checklist

- [ ] All evidence actors placed in level
- [ ] Evidence metadata configured correctly
- [ ] Evidence collection works
- [ ] Evidence appears in archive
- [ ] Evidence thumbnails load correctly
- [ ] Evidence search works
- [ ] Evidence filtering works
- [ ] Evidence detail viewer works
- [ ] Save/load preserves evidence
- [ ] Evidence-based progression works
- [ ] Collection feedback is clear
- [ ] Performance acceptable
- [ ] No duplicate evidence IDs
- [ ] All evidence is reachable
- [ ] Story-critical evidence is obvious

## Next Steps

1. Create all evidence actor blueprints
2. Configure evidence metadata
3. Place evidence in levels
4. Test evidence collection
5. Implement archive viewer
6. Add evidence-based progression
7. Test save/load system
8. Optimize performance
9. Add achievements for evidence collection
10. Playtest evidence discovery flow
