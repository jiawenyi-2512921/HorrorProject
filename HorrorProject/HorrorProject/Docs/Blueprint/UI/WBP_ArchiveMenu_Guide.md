# WBP_ArchiveMenu Blueprint Implementation Guide

## Overview
Full-screen archive menu for viewing collected evidence and notes with filtering and detail view.

## C++ Base Class
**Parent Class:** `UArchiveMenuWidget` (inherits from `UUserWidget`)

### Available C++ Functions
```cpp
// Menu Control
void OpenArchive()
void CloseArchive()
void RefreshArchiveData()
void SelectEntry(FName EntryId)
void FilterByType(EArchiveEntryType Type)
void ClearFilter()

// State Query
bool IsOpen() const
```

### Blueprint Implementable Events
```cpp
BP_ArchiveOpened()
BP_ArchiveClosed()
BP_ArchiveDataRefreshed(const TArray<FHorrorEvidenceMetadata>& Evidence, const TArray<FHorrorNoteMetadata>& Notes)
BP_EntrySelected(FName EntryId)
BP_FilterApplied(EArchiveEntryType Type)
```

### Exposed Properties
- `bIsOpen` (bool, ReadOnly)
- `SelectedEntryId` (FName, ReadOnly)
- `CurrentFilter` (EArchiveEntryType, ReadOnly)
- `OpenAnimationDuration` (float, EditDefaultsOnly) - Default: 0.5
- `CloseAnimationDuration` (float, EditDefaultsOnly) - Default: 0.3

### Enums
```cpp
EArchiveEntryType:
- All
- Evidence
- Notes
- Photos
- Audio
- Documents
```

---

## Canvas Layout Design

### Screen Layout (1920x1080 reference)
```
┌─────────────────────────────────────────────────────────────┐
│ [X] ARCHIVE                                    [FILTERS]    │ ← Header
├─────────────────┬───────────────────────────────────────────┤
│                 │                                           │
│  Evidence List  │         Detail View                       │
│                 │                                           │
│ [√] Note #1     │  ┌─────────────────────────────────┐     │
│ [ ] Photo #2    │  │                                 │     │
│ [√] Document    │  │     [Evidence Image/Content]    │     │
│ [ ] Audio Log   │  │                                 │     │
│ [√] Note #2     │  └─────────────────────────────────┘     │
│                 │                                           │
│                 │  Title: Mysterious Note                   │
│                 │  Type: Document                           │
│                 │  Location: Kitchen                        │
│                 │                                           │
│                 │  Description text here...                 │
│                 │                                           │
└─────────────────┴───────────────────────────────────────────┘
```

### Widget Hierarchy
```
Canvas Panel (Root)
└── Border_ArchiveContainer (Full Screen)
    ├── Image_Background (VHS effect)
    ├── VerticalBox_Main
    │   ├── Border_Header (Height: 80)
    │   │   ├── HorizontalBox_HeaderContent
    │   │   │   ├── Button_Close
    │   │   │   ├── TextBlock_Title
    │   │   │   ├── Spacer (Fill)
    │   │   │   └── HorizontalBox_Filters
    │   │   │       ├── Button_FilterAll
    │   │   │       ├── Button_FilterEvidence
    │   │   │       ├── Button_FilterNotes
    │   │   │       └── Button_FilterPhotos
    │   └── HorizontalBox_Content (Fill)
    │       ├── Border_ListPanel (Width: 400)
    │       │   └── ScrollBox_EntryList
    │       │       └── [WBP_ArchiveListItem] (multiple)
    │       └── Border_DetailPanel (Fill)
    │           └── ScrollBox_DetailView
    │               ├── Image_EvidencePreview
    │               ├── TextBlock_EntryTitle
    │               ├── TextBlock_EntryType
    │               ├── TextBlock_Location
    │               └── TextBlock_Description
    └── Image_ScanlineOverlay
```

---

## Step-by-Step Implementation

### Step 1: Create Widget Blueprint
1. Content Browser → Right-click → User Interface → Widget Blueprint
2. Name: `WBP_ArchiveMenu`
3. Open the blueprint
4. Class Settings → Parent Class → Search "ArchiveMenuWidget"
5. Compile and Save

### Step 2: Canvas Root Setup
1. Designer tab → Hierarchy panel
2. Root should be Canvas Panel (default)
3. Canvas Panel settings:
   - Is Variable: False
   - Visibility: Collapsed (initially hidden)

### Step 3: Archive Container

#### Border_ArchiveContainer
1. Add Border widget to Canvas Panel
2. Rename to `Border_ArchiveContainer`
3. Anchors: Fill screen (0,0 to 1,1)
4. Position: X=0, Y=0
5. Size: X=0, Y=0
6. Z-Order: 1000
7. Is Variable: True
8. Appearance:
   - Brush Color: Black (0, 0, 0, 0.95)
   - Brush → Draw As: Rounded Box
9. Padding: 0 (all sides)

### Step 4: Background Effects

#### Image_Background
1. Add Image widget to Border_ArchiveContainer (first child)
2. Rename to `Image_Background`
3. Anchors: Fill (0,0 to 1,1)
4. Position: X=0, Y=0
5. Size: X=0, Y=0
6. Z-Order: 0
7. Material: M_FilmGrain
8. Render Opacity: 0.1
9. Hit Test Invisible: True

### Step 5: Main Layout

#### VerticalBox_Main
1. Add Vertical Box to Border_ArchiveContainer
2. Rename to `VerticalBox_Main`
3. Anchors: Fill (0,0 to 1,1)
4. Position: X=0, Y=0
5. Size: X=0, Y=0

### Step 6: Header Section

#### Border_Header
1. Add Border to VerticalBox_Main
2. Rename to `Border_Header`
3. Size: Fill Horizontal, Fixed Height: 80
4. Appearance:
   - Brush Color: Dark Gray (0.05, 0.05, 0.05, 1)
   - Brush → Draw As: Box
   - Brush → Outline Settings → Width: 0, 0, 2, 0 (bottom only)
   - Brush → Outline Settings → Color: Cyan (0, 0.8, 1, 0.5)
5. Padding: Left=20, Top=20, Right=20, Bottom=20

#### HorizontalBox_HeaderContent
1. Add Horizontal Box to Border_Header
2. Rename to `HorizontalBox_HeaderContent`
3. Anchors: Fill

**Button_Close:**
- Type: Button
- Size: 40x40 (Fixed)
- Style:
  - Normal: Dark Gray (0.1, 0.1, 0.1, 1)
  - Hovered: Gray (0.2, 0.2, 0.2, 1)
  - Pressed: Light Gray (0.3, 0.3, 0.3, 1)
- Content: Text Block "X" (Size: 24, Color: White)
- Is Variable: True
- On Clicked: Bind to OnCloseClicked

**TextBlock_Title:**
- Text: "ARCHIVE"
- Font: F_HomeVideo
- Size: 32
- Color: Cyan (0, 0.8, 1, 1)
- Font → Typeface: Bold
- Padding: Left=20

**Spacer:** Fill Horizontal

**HorizontalBox_Filters:**
- Add Horizontal Box
- Spacing: 8

**Button_FilterAll:**
- Type: Button
- Size: 80x40
- Style: Normal (0.2, 0.2, 0.2, 1), Hovered (0.3, 0.3, 0.3, 1)
- Content: Text "ALL" (Size: 14, Color: White)
- Is Variable: True
- On Clicked: Bind to OnFilterAllClicked

**Button_FilterEvidence:**
- Type: Button
- Size: 100x40
- Content: Text "EVIDENCE" (Size: 14)
- Is Variable: True
- On Clicked: Bind to OnFilterEvidenceClicked

**Button_FilterNotes:**
- Type: Button
- Size: 80x40
- Content: Text "NOTES" (Size: 14)
- Is Variable: True
- On Clicked: Bind to OnFilterNotesClicked

**Button_FilterPhotos:**
- Type: Button
- Size: 90x40
- Content: Text "PHOTOS" (Size: 14)
- Is Variable: True
- On Clicked: Bind to OnFilterPhotosClicked

### Step 7: Content Section

#### HorizontalBox_Content
1. Add Horizontal Box to VerticalBox_Main
2. Rename to `HorizontalBox_Content`
3. Fill: Vertical (expand to fill remaining space)

### Step 8: List Panel

#### Border_ListPanel
1. Add Border to HorizontalBox_Content
2. Rename to `Border_ListPanel`
3. Size: Fixed Width: 400, Fill Vertical
4. Appearance:
   - Brush Color: Dark Gray (0.08, 0.08, 0.08, 1)
   - Brush → Outline Settings → Width: 0, 2, 0, 0 (right only)
   - Brush → Outline Settings → Color: Cyan (0, 0.8, 1, 0.3)
5. Padding: 10 (all sides)

#### ScrollBox_EntryList
1. Add Scroll Box to Border_ListPanel
2. Rename to `ScrollBox_EntryList`
3. Anchors: Fill
4. Scroll Bar Style:
   - Thickness: 8
   - Color: Cyan (0, 0.8, 1, 0.5)
5. Always Show Scrollbar: False
6. Is Variable: True

### Step 9: Detail Panel

#### Border_DetailPanel
1. Add Border to HorizontalBox_Content
2. Rename to `Border_DetailPanel`
3. Size: Fill Horizontal, Fill Vertical
4. Appearance:
   - Brush Color: Very Dark Gray (0.05, 0.05, 0.05, 1)
5. Padding: 40 (all sides)

#### ScrollBox_DetailView
1. Add Scroll Box to Border_DetailPanel
2. Rename to `ScrollBox_DetailView`
3. Anchors: Fill
4. Is Variable: True

**Image_EvidencePreview:**
- Type: Image
- Size: Fill Horizontal, Fixed Height: 400
- Brush → Tint: White
- Brush → Draw As: Image
- Stretch: Uniform to Fill
- Is Variable: True
- Visibility: Collapsed (initially)

**Spacer:** Height=20

**TextBlock_EntryTitle:**
- Text: "Select an entry"
- Font: F_HomeVideo
- Size: 28
- Color: White (1, 1, 1, 1)
- Font → Typeface: Bold
- Is Variable: True
- Auto Wrap: True

**Spacer:** Height=16

**TextBlock_EntryType:**
- Text: "Type: -"
- Font: F_HomeVideo
- Size: 16
- Color: Cyan (0, 0.8, 1, 1)
- Is Variable: True

**Spacer:** Height=8

**TextBlock_Location:**
- Text: "Location: -"
- Font: F_HomeVideo
- Size: 16
- Color: Light Gray (0.7, 0.7, 0.7, 1)
- Is Variable: True

**Spacer:** Height=24

**TextBlock_Description:**
- Text: "Description will appear here..."
- Font: F_HomeVideo
- Size: 18
- Color: White (0.9, 0.9, 0.9, 1)
- Is Variable: True
- Auto Wrap: True
- Justification: Left

### Step 10: Scanline Overlay

#### Image_ScanlineOverlay
1. Add Image widget to Border_ArchiveContainer (last child)
2. Rename to `Image_ScanlineOverlay`
3. Anchors: Fill (0,0 to 1,1)
4. Position: X=0, Y=0
5. Size: X=0, Y=0
6. Z-Order: 100
7. Material: M_Scanlines
8. Render Opacity: 0.1
9. Hit Test Invisible: True

---

## Animation Setup

### Animation 1: OpenArchive
**Purpose:** Fade in and scale up

1. Animations tab → + Animation → Name: "OpenArchive"
2. Length: 0.5 seconds
3. Loop: False
4. Tracks:
   - Border_ArchiveContainer → Render Opacity
     - 0.0s: 0.0
     - 0.5s: 1.0 (Ease Out curve)
   - Border_ArchiveContainer → Render Transform → Scale
     - 0.0s: (0.95, 0.95)
     - 0.5s: (1.0, 1.0) (Ease Out curve)

### Animation 2: CloseArchive
**Purpose:** Fade out and scale down

1. Animations tab → + Animation → Name: "CloseArchive"
2. Length: 0.3 seconds
3. Loop: False
4. Tracks:
   - Border_ArchiveContainer → Render Opacity
     - 0.0s: 1.0
     - 0.3s: 0.0 (Ease In curve)
   - Border_ArchiveContainer → Render Transform → Scale
     - 0.0s: (1.0, 1.0)
     - 0.3s: (0.95, 0.95) (Ease In curve)

### Animation 3: FilterButtonHighlight
**Purpose:** Highlight active filter button

1. Animations tab → + Animation → Name: "FilterButtonHighlight"
2. Length: 0.2 seconds
3. Loop: False
4. Tracks:
   - [Target Button] → Color and Opacity
     - 0.0s: Gray (0.2, 0.2, 0.2, 1)
     - 0.2s: Cyan (0, 0.8, 1, 1)

---

## Event Graph Implementation

### Event Construct
```
Event Construct
├── Set Visibility (Collapsed)
├── Set Render Opacity (0.0)
├── Clear Entry List
├── Clear Detail View
└── Set Input Mode (UI Only when opened)
```

**Blueprint Nodes:**
1. Event Construct
2. Set Visibility (Target: Self, Visibility: Collapsed)
3. Set Render Opacity (Target: Border_ArchiveContainer, Opacity: 0.0)
4. ScrollBox_EntryList → Clear Children
5. TextBlock_EntryTitle → Set Text ("Select an entry")

### BP_ArchiveOpened Implementation
```
BP_ArchiveOpened
├── Set Visibility (Visible)
├── Play Animation (OpenArchive)
├── Set Input Mode (UI Only)
├── Show Mouse Cursor (True)
├── Call RefreshArchiveData
└── Play Sound 2D (SFX_MenuOpen)
```

**Blueprint Nodes:**
1. Event BP_ArchiveOpened
2. Set Visibility (Target: Self, Visibility: Visible)
3. Play Animation (OpenArchive)
4. Get Player Controller → Set Input Mode UI Only
5. Get Player Controller → Set Show Mouse Cursor (True)
6. Call RefreshArchiveData
7. Play Sound 2D (SFX_MenuOpen)

### BP_ArchiveClosed Implementation
```
BP_ArchiveClosed
├── Play Animation (CloseArchive)
├── Delay (CloseAnimationDuration)
├── Set Visibility (Collapsed)
├── Set Input Mode (Game Only)
├── Show Mouse Cursor (False)
└── Play Sound 2D (SFX_MenuClose)
```

**Blueprint Nodes:**
1. Event BP_ArchiveClosed
2. Play Animation (CloseArchive)
3. Delay (Duration: CloseAnimationDuration)
4. Set Visibility (Target: Self, Visibility: Collapsed)
5. Get Player Controller → Set Input Mode Game Only
6. Get Player Controller → Set Show Mouse Cursor (False)
7. Play Sound 2D (SFX_MenuClose)

### BP_ArchiveDataRefreshed Implementation
```
BP_ArchiveDataRefreshed (Evidence, Notes)
├── Clear Entry List
├── For Each Evidence Item
│   ├── Create WBP_ArchiveListItem
│   ├── Set Item Data (Evidence)
│   ├── Bind OnItemClicked
│   └── Add to ScrollBox_EntryList
├── For Each Note Item
│   ├── Create WBP_ArchiveListItem
│   ├── Set Item Data (Note)
│   ├── Bind OnItemClicked
│   └── Add to ScrollBox_EntryList
└── Sort List by Collection Date
```

**Blueprint Nodes:**
1. Event BP_ArchiveDataRefreshed (Parameters: Evidence array, Notes array)
2. ScrollBox_EntryList → Clear Children
3. For Each Loop (Evidence array)
4. Loop Body:
   - Create Widget (Class: WBP_ArchiveListItem)
   - Call SetItemData (Evidence metadata)
   - Bind Event to OnItemClicked → Custom Event: OnListItemClicked
   - ScrollBox_EntryList → Add Child (Widget)
5. Repeat for Notes array
6. Optional: Sort by date/type

### BP_EntrySelected Implementation
```
BP_EntrySelected (EntryId)
├── Get Entry Data from Evidence System
├── Branch (Entry Found)
│   ├── True:
│   │   ├── Update Detail View
│   │   │   ├── Set Title Text
│   │   │   ├── Set Type Text
│   │   │   ├── Set Location Text
│   │   │   ├── Set Description Text
│   │   │   └── Set Preview Image (if available)
│   │   ├── Highlight Selected Item in List
│   │   └── Play Sound 2D (SFX_ItemSelect)
│   └── False:
│       └── Show "Entry not found" message
```

**Blueprint Nodes:**
1. Event BP_EntrySelected (Parameter: EntryId)
2. Get Game Instance → Get Subsystem (EvidenceSubsystem)
3. Get Evidence by ID (EntryId)
4. Branch (Is Valid)
5. True path:
   - TextBlock_EntryTitle → Set Text (Evidence.DisplayName)
   - Format Text ("Type: {0}", Evidence.Type) → Set Text (TextBlock_EntryType)
   - Format Text ("Location: {0}", Evidence.Location) → Set Text (TextBlock_Location)
   - TextBlock_Description → Set Text (Evidence.Description)
   - Branch (Has Preview Image)
     - True: Image_EvidencePreview → Set Brush from Texture → Set Visibility (Visible)
     - False: Image_EvidencePreview → Set Visibility (Collapsed)
6. Play Sound 2D (SFX_ItemSelect)

### BP_FilterApplied Implementation
```
BP_FilterApplied (Type)
├── Store CurrentFilter = Type
├── Update Filter Button States
│   ├── Reset All Buttons to Normal
│   └── Highlight Active Filter Button
├── Filter Entry List
│   ├── For Each List Item
│   │   ├── Branch (Item Type matches Filter OR Filter is All)
│   │   │   ├── True: Set Visibility (Visible)
│   │   │   └── False: Set Visibility (Collapsed)
└── Play Sound 2D (SFX_FilterChange)
```

**Blueprint Nodes:**
1. Event BP_FilterApplied (Parameter: Type)
2. Set CurrentFilter = Type
3. Reset all filter buttons to normal color
4. Switch on Type:
   - All: Highlight Button_FilterAll
   - Evidence: Highlight Button_FilterEvidence
   - Notes: Highlight Button_FilterNotes
   - Photos: Highlight Button_FilterPhotos
5. Get all children from ScrollBox_EntryList
6. For Each Loop:
   - Cast to WBP_ArchiveListItem
   - Get Item Type
   - Branch (Type == CurrentFilter OR CurrentFilter == All)
     - True: Set Visibility (Visible)
     - False: Set Visibility (Collapsed)
7. Play Sound 2D (SFX_FilterChange)

### Button Click Events

#### OnCloseClicked
```
OnCloseClicked
└── Call CloseArchive
```

#### OnFilterAllClicked
```
OnFilterAllClicked
└── Call FilterByType (EArchiveEntryType::All)
```

#### OnFilterEvidenceClicked
```
OnFilterEvidenceClicked
└── Call FilterByType (EArchiveEntryType::Evidence)
```

#### OnFilterNotesClicked
```
OnFilterNotesClicked
└── Call FilterByType (EArchiveEntryType::Notes)
```

#### OnFilterPhotosClicked
```
OnFilterPhotosClicked
└── Call FilterByType (EArchiveEntryType::Photos)
```

### Custom Event: OnListItemClicked
```
OnListItemClicked (EntryId)
└── Call SelectEntry (EntryId)
```

---

## WBP_ArchiveListItem (Sub-Widget)

### Purpose
Individual list item for evidence/note entries.

### Widget Hierarchy
```
Border_ItemContainer (400x80)
├── HorizontalBox_Content
│   ├── Image_Icon (32x32)
│   ├── Spacer (12px)
│   ├── VerticalBox_Info
│   │   ├── TextBlock_Title
│   │   └── TextBlock_Subtitle
│   └── Image_CheckMark (24x24)
```

### Implementation
1. Create new Widget Blueprint: WBP_ArchiveListItem
2. Add Border as root (400x80)
3. Style:
   - Normal: Dark Gray (0.1, 0.1, 0.1, 1)
   - Hovered: Gray (0.15, 0.15, 0.15, 1)
   - Selected: Cyan (0, 0.8, 1, 0.2)
4. Add Button behavior (make entire item clickable)
5. Add icon, title, subtitle, checkmark
6. Create function: SetItemData(Metadata)
7. Create event dispatcher: OnItemClicked(EntryId)

---

## Testing Checklist

### Visual Tests
- [ ] Archive fills entire screen
- [ ] Header displays correctly with title and filters
- [ ] List panel shows entries with scrolling
- [ ] Detail panel displays entry information
- [ ] Scanline overlay subtle
- [ ] Filter buttons highlight when active

### Animation Tests
- [ ] OpenArchive animation smooth (0.5s)
- [ ] CloseArchive animation smooth (0.3s)
- [ ] Fade and scale effects work together
- [ ] No animation stuttering

### Functional Tests
- [ ] OpenArchive shows menu
- [ ] CloseArchive hides menu
- [ ] RefreshArchiveData populates list
- [ ] SelectEntry updates detail view
- [ ] FilterByType filters list correctly
- [ ] ClearFilter shows all entries
- [ ] Mouse cursor shows when open
- [ ] Input mode switches correctly

### List Tests
- [ ] All evidence items display
- [ ] All note items display
- [ ] Items clickable
- [ ] Selected item highlighted
- [ ] Scroll works with many items
- [ ] Empty state handled gracefully

### Filter Tests
- [ ] "All" shows all entries
- [ ] "Evidence" shows only evidence
- [ ] "Notes" shows only notes
- [ ] "Photos" shows only photos
- [ ] Active filter button highlighted
- [ ] Filter persists during session

### Detail View Tests
- [ ] Title displays correctly
- [ ] Type displays correctly
- [ ] Location displays correctly
- [ ] Description displays and wraps
- [ ] Preview image shows when available
- [ ] Preview image hidden when not available
- [ ] Long text scrolls properly

### Performance Tests
- [ ] Handles 100+ entries smoothly
- [ ] Filtering is instant
- [ ] Scrolling smooth at 60fps
- [ ] No memory leaks from repeated open/close

---

## Common Issues & Solutions

### Issue: Archive not opening
**Solution:** Check that OpenArchive is called from C++. Verify Set Visibility is called in BP_ArchiveOpened. Check Z-Order is high (1000+).

### Issue: List items not appearing
**Solution:** Verify BP_ArchiveDataRefreshed is called. Check that WBP_ArchiveListItem widget exists. Ensure ScrollBox_EntryList is valid.

### Issue: Detail view not updating
**Solution:** Check that BP_EntrySelected is implemented. Verify Evidence System returns valid data. Ensure text blocks are marked as Variable.

### Issue: Filter buttons not working
**Solution:** Verify button click events are bound. Check that FilterByType is called with correct enum value. Ensure BP_FilterApplied is implemented.

### Issue: Mouse cursor not showing
**Solution:** Check that Set Show Mouse Cursor is called in BP_ArchiveOpened. Verify Set Input Mode UI Only is called.

### Issue: Can't close archive
**Solution:** Verify Button_Close click event is bound. Check that CloseArchive is called. Ensure ESC key is bound to close (see Input Handling).

### Issue: Scrolling not working
**Solution:** Check that ScrollBox has enough content to scroll. Verify scroll bar style is set. Ensure content exceeds visible area.

---

## Advanced Features

### Search Functionality
**Purpose:** Search entries by name or description

#### Add to Header
```
HorizontalBox_HeaderContent
└── EditableTextBox_Search (Width: 200)
```

#### Implementation
```
OnSearchTextChanged (Text)
├── For Each List Item
│   ├── Get Item Title and Description
│   ├── Branch (Contains Search Text)
│   │   ├── True: Set Visibility (Visible)
│   │   └── False: Set Visibility (Collapsed)
```

### Sort Options
**Purpose:** Sort entries by date, name, or type

#### Add to Header
```
ComboBox_Sort (Options: Date, Name, Type)
```

#### Implementation
```
OnSortChanged (SortType)
├── Get All List Items
├── Sort Array by SortType
└── Rebuild List in Sorted Order
```

### Entry Statistics
**Purpose:** Show collection progress

#### Add to Header
```
TextBlock_Stats ("Collected: 15/30")
```

#### Update in BP_ArchiveDataRefreshed
```
├── Count Total Entries
├── Count Collected Entries
├── Format Text ("Collected: {0}/{1}")
└── Set Text (TextBlock_Stats)
```

---

## Input Handling

### Keyboard Shortcuts

#### Event Graph
```
Event Construct
├── Get Player Controller
└── Enable Input (Priority: 1)

Override: OnKeyDown
├── Switch on Key
│   ├── Escape:
│   │   └── Call CloseArchive
│   ├── F (Filter):
│   │   └── Cycle through filters
│   ├── Up Arrow:
│   │   └── Select previous entry
│   └── Down Arrow:
│       └── Select next entry
```

### Gamepad Support
```
Override: OnAnalogValueChanged
├── Left Stick Y:
│   └── Scroll List
├── D-Pad Up/Down:
│   └── Select Entry
└── B Button:
    └── Close Archive
```

---

## Performance Optimization

### Best Practices
1. Use object pooling for list items (reuse widgets)
2. Virtualize scroll box for large lists
3. Load preview images asynchronously
4. Cache filtered results
5. Limit description text length
6. Use Invalidation Box for static elements

### Object Pooling Implementation
```
Variables:
- ListItemPool (Array of WBP_ArchiveListItem)
- PoolSize (int) = 20

GetOrCreateListItem
├── Branch (Pool has available item)
│   ├── True: Get from pool
│   └── False: Create new widget
└── Return widget

ReturnListItemToPool (Widget)
├── Remove from ScrollBox
├── Set Visibility (Collapsed)
└── Add to Pool
```

---

## Integration with Game

### Opening from Gameplay
```cpp
// In player controller or input handler
UUIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UUIManagerSubsystem>();
UArchiveMenuWidget* Archive = Cast<UArchiveMenuWidget>(
    UIManager->GetWidget("ArchiveMenu")
);
if (Archive)
{
    Archive->OpenArchive();
}
```

### Blueprint Integration
```
Input Action "OpenArchive" (Tab key)
├── Get Game Instance
├── Get Subsystem (UIManagerSubsystem)
├── Get Widget ("ArchiveMenu")
├── Cast to ArchiveMenuWidget
└── Call OpenArchive
```

### Pause Game When Open
```
BP_ArchiveOpened (modified)
├── ... (existing code)
└── Set Game Paused (True)

BP_ArchiveClosed (modified)
├── ... (existing code)
└── Set Game Paused (False)
```

---

## Asset Requirements

### Fonts
- F_HomeVideo (VHS-style monospace font)

### Materials
- M_FilmGrain (background noise)
- M_Scanlines (scanline overlay)

### Textures
- T_IconEvidence (32x32)
- T_IconNote (32x32)
- T_IconPhoto (32x32)
- T_IconAudio (32x32)
- T_CheckMark (24x24)
- T_CloseButton (40x40)

### Sounds
- SFX_MenuOpen (archive open sound)
- SFX_MenuClose (archive close sound)
- SFX_ItemSelect (item click sound)
- SFX_FilterChange (filter change sound)
- SFX_PageTurn (optional, for navigation)

### Colors
- Primary: Cyan (0.0, 0.8, 1.0, 1.0)
- Background: Black (0.0, 0.0, 0.0, 0.95)
- Panel Dark: Very Dark Gray (0.05, 0.05, 0.05, 1.0)
- Panel Medium: Dark Gray (0.08, 0.08, 0.08, 1.0)
- Text Primary: White (1.0, 1.0, 1.0, 1.0)
- Text Secondary: Light Gray (0.7, 0.7, 0.7, 1.0)
- Button Normal: Gray (0.2, 0.2, 0.2, 1.0)
- Button Hovered: Light Gray (0.3, 0.3, 0.3, 1.0)
- Button Active: Cyan (0.0, 0.8, 1.0, 1.0)
