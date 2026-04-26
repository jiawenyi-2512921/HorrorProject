# Blueprint Document Implementation Guide

## Overview
Complete guide for creating document interactables using Blueprint, inheriting from `ADocumentInteractable` C++ class.

## 1. Blueprint Creation Steps

### 1.1 Create Blueprint Class
1. Content Browser → Right-click → Blueprint Class
2. Search for `DocumentInteractable` parent class
3. Name: `BP_Document_[Type]` (e.g., `BP_Document_Note`, `BP_Document_Letter`)
4. Location: `Content/Blueprints/Interaction/Documents/`

### 1.2 Component Structure
Default components from C++ class:
- **DocumentMesh** (StaticMeshComponent) - Visual representation
- **InteractionVolume** (BoxComponent) - Interaction trigger

### 1.3 Collision Configuration
**InteractionVolume Settings:**
- Collision Preset: `OverlapAllDynamic`
- Generate Overlap Events: `true`
- Box Extent: `X=50, Y=50, Z=20` (adjust per document size)

**DocumentMesh Collision:**
- Collision Preset: `BlockAll` or `OverlapAll`
- Simulate Physics: `false`

## 2. Mesh and Material Configuration

### 2.1 Document Mesh Setup
1. Select `DocumentMesh` component
2. Static Mesh: Choose document type
3. Materials: Apply paper/parchment material
4. Transform:
   - Location: `(0,0,0)`
   - Rotation: Flat or slightly angled
   - Scale: Readable size

### 2.2 Recommended Assets by Type

**Paper Documents:**
- Mesh: `SM_Paper_Single`, `SM_Paper_Folded`, `SM_Letter`
- Material: `M_Paper_Old`, `M_Paper_Clean`, `M_Parchment`

**Notebooks:**
- Mesh: `SM_Notebook_Closed`, `SM_Journal`
- Material: `M_Leather_Book`, `M_Notebook_Cover`

**Photos:**
- Mesh: `SM_Photo_Polaroid`, `SM_Photo_Standard`
- Material: `M_Photo_Glossy`, `M_Photo_Matte`

**Newspapers:**
- Mesh: `SM_Newspaper_Folded`, `SM_Newspaper_Page`
- Material: `M_Newsprint`

### 2.3 Material Setup
**Paper Material Properties:**
- Base Color: Aged paper texture
- Roughness: 0.6-0.8 (matte finish)
- Normal Map: Paper fiber detail
- Emissive: None (unless special effect)

## 3. Interaction Parameters

### 3.1 Base Interaction Settings
**Category: Interaction**
- `InteractableId`: Unique name (e.g., `Document_Letter_Victim`)
- `InteractionPrompt`: "Read [Document Name]"
- `bCanInteractMultipleTimes`: `true`
- `bRequireLineOfSight`: `true`

### 3.2 Content Settings
**Category: Document | Content**
- `DocumentType`: Select from enum
  - `Text` - Text-only document
  - `Image` - Image-only document
  - `Mixed` - Text and images combined
- `DocumentTitle`: Display title (e.g., "Victim's Letter")
- `DocumentContent`: Main text content (multiline)
  - Use rich text formatting if supported
  - Keep paragraphs readable
  - Limit to 500-1000 words
- `DocumentImage`: Primary image texture (for Image/Mixed types)
- `AdditionalImages`: Array of extra images (for galleries)

### 3.3 Evidence Settings
**Category: Document | Evidence**
- `bIsEvidence`: `true` if collectible evidence
- `EvidenceMetadata`: (if bIsEvidence = true)
  - `EvidenceId`: Unique evidence ID (e.g., `Evidence_Letter_01`)
  - `EvidenceName`: Display name
  - `Description`: Evidence description
  - `Category`: "Documents" or specific category
  - `bIsOptional`: Optional evidence flag

### 3.4 Audio Configuration
**Category: Document | Audio**
- `OpenSound`: Sound when document opens
  - Paper: `SFX_Paper_Rustle`
  - Book: `SFX_Book_Open`
  - Photo: `SFX_Photo_Pickup`
- `CloseSound`: Sound when document closes
  - Paper: `SFX_Paper_Fold`
  - Book: `SFX_Book_Close`
  - Photo: `SFX_Photo_Putdown`

### 3.5 Behavior Settings
**Category: Document | Behavior**
- `bAutoCollectAsEvidence`: `true` (auto-add to evidence)
- `bDisablePlayerMovement`: `true` (freeze player while reading)
- `bPauseGame`: `false` (pause game time)
  - Use `true` for critical story documents
  - Use `false` for optional reading

## 4. Specific Functionality

### 4.1 Text Document Setup
**For DocumentType = Text:**
1. Set `DocumentTitle`: "Victim's Final Letter"
2. Set `DocumentContent`:
```
"My dearest Sarah,

If you're reading this, then I fear the worst has come to pass. The things I've seen in this place... they defy explanation.

The shadows move on their own. The walls whisper secrets that no living soul should hear. I tried to leave, but the doors... they won't let me.

Please, don't come looking for me. Forget this place exists.

- John"
```
3. Leave `DocumentImage` empty
4. Configure evidence metadata if collectible

### 4.2 Image Document Setup
**For DocumentType = Image:**
1. Set `DocumentTitle`: "Crime Scene Photo"
2. Leave `DocumentContent` empty or add brief caption
3. Set `DocumentImage`: Texture asset (e.g., `T_Photo_CrimeScene_01`)
4. Image requirements:
   - Resolution: 1024x1024 minimum
   - Format: PNG or TGA
   - Readable details when displayed

### 4.3 Mixed Content Setup
**For DocumentType = Mixed:**
1. Set `DocumentTitle`: "Investigation Report"
2. Set `DocumentContent`: Text description
3. Set `DocumentImage`: Primary image
4. Add `AdditionalImages`: Supporting images
5. UI should display text alongside images

### 4.4 Evidence Collection
**Automatic behavior when bIsEvidence = true:**
1. Player reads document
2. Document added to evidence inventory
3. Evidence notification shown
4. Quest objectives updated
5. EventBus event published

**Blueprint Extension:**
```
Event OnDocumentRead
├─ Branch (Is Evidence?)
│  ├─ True:
│  │  ├─ Show Evidence Collected UI
│  │  ├─ Play Collection Sound
│  │  ├─ Update Evidence Counter
│  │  └─ Check Quest Progress
│  └─ False → Continue
```

### 4.5 Multi-Page Documents
**Blueprint Implementation:**
```
Variable: CurrentPage (Integer) = 0
Variable: TotalPages (Integer) = 3
Variable: PageContents (Array of Text)

Custom Event: NextPage
├─ Increment CurrentPage
├─ Branch (CurrentPage < TotalPages?)
│  ├─ True:
│  │  ├─ Update Document UI
│  │  ├─ Display PageContents[CurrentPage]
│  │  └─ Play Page Turn Sound
│  └─ False:
│     ├─ CurrentPage = TotalPages - 1
│     └─ Show "Last Page" Message

Custom Event: PreviousPage
├─ Decrement CurrentPage
├─ Branch (CurrentPage >= 0?)
│  ├─ True:
│  │  ├─ Update Document UI
│  │  └─ Display PageContents[CurrentPage]
│  └─ False:
│     ├─ CurrentPage = 0
│     └─ Show "First Page" Message
```

### 4.6 Conditional Reading
**Require specific conditions:**
```
Override: CanInteract
├─ Parent CanInteract → Result
├─ Branch (Requires Translation?)
│  ├─ True:
│  │  ├─ Get Inventory Component
│  │  ├─ HasItem("Tool_Translator")
│  │  ├─ Branch (Has Translator?)
│  │  │  ├─ True → Return True
│  │  │  └─ False:
│  │  │     ├─ Show Message "Cannot read this language"
│  │  │     └─ Return False
│  └─ False → Return Result
```

## 5. Event Logic

### 5.1 OnInteract Event
**Default C++ Behavior:**
1. Check if can interact
2. Store player controller reference
3. Disable player movement (if enabled)
4. Pause game (if enabled)
5. Show document UI with content
6. Play open sound
7. Try add to evidence (if enabled)
8. Set currently open flag

**Blueprint Extension:**
```
Event OnInteract (Override)
├─ Call Parent OnInteract
├─ Fade In Document UI
├─ Show Document Content
├─ Enable Close Input
├─ Start Reading Timer
└─ Publish Event "Document.Opened"
```

### 5.2 Document UI Events
**Blueprint Events:**
```
Event OnDocumentOpened
├─ Show Document Widget
├─ Populate Title
├─ Populate Content
├─ Show Images (if any)
├─ Enable Navigation (if multi-page)
└─ Focus UI Input

Event OnDocumentClosed
├─ Hide Document Widget
├─ Re-enable Player Movement
├─ Resume Game Time
├─ Clear UI Content
└─ Publish Event "Document.Closed"

Event OnPageTurned
├─ Play Page Turn Animation
├─ Update Page Number Display
├─ Load New Page Content
└─ Play Page Turn Sound
```

### 5.3 EventBus Integration
**Automatic Events Published:**
- `Document.Opened` - When document is opened
- `Document.Closed` - When document is closed
- `Document.Read` - When document is fully read
- `Evidence.Document.Collected` - When document evidence collected

**Custom Event Publishing:**
```
Event OnDocumentFullyRead
├─ Get EventBus Subsystem
├─ Create Event Data
│  ├─ Add "DocumentId" → InteractableId
│  ├─ Add "DocumentTitle" → DocumentTitle
│  ├─ Add "ReadTime" → Time Spent Reading
│  └─ Add "IsEvidence" → bIsEvidence
└─ Publish Event ("Document.FullyRead")
```

### 5.4 SaveGame Integration
**Automatic Save/Load:**
- Document read state
- First read timestamp
- Read count

**Blueprint Extension:**
```
Override: SaveState
├─ Call Parent SaveState
├─ Add State ("HasBeenRead", true)
├─ Add State ("CurrentPage", CurrentPage)
├─ Add State ("WasCollected", bWasCollectedAsEvidence)
└─ Return State Map

Override: LoadState
├─ Call Parent LoadState
├─ Get State ("HasBeenRead")
├─ Get State ("CurrentPage")
├─ Branch (Was Collected?)
│  ├─ True → Mark as Collected
│  └─ False → Continue
```

## 6. Testing Validation

### 6.1 Functional Tests
- [ ] Document opens and displays correctly
- [ ] Text is readable and formatted properly
- [ ] Images display at correct resolution
- [ ] Close button/input works
- [ ] Player movement disabled during reading
- [ ] Evidence collection works (if enabled)
- [ ] Multi-page navigation works (if applicable)
- [ ] Audio plays at correct times

### 6.2 Edge Cases
- [ ] Very long text content handling
- [ ] Missing image texture handling
- [ ] Rapid open/close spam
- [ ] Reading during level transition
- [ ] Save/load during reading
- [ ] Multiple documents open simultaneously
- [ ] Document with no content
- [ ] Special characters in text

### 6.3 Performance Validation
- [ ] UI opens without frame drops
- [ ] Text rendering is smooth
- [ ] Image loading doesn't stutter
- [ ] No memory leaks on close
- [ ] Multiple documents don't leak memory

### 6.4 Readability Tests
- [ ] Text size is comfortable
- [ ] Font is legible
- [ ] Contrast is sufficient
- [ ] Line spacing is appropriate
- [ ] Paragraphs are well-formatted
- [ ] Images are clear and detailed

## 7. Common Configurations

### 7.1 Basic Text Note
```
DocumentType: Text
DocumentTitle: "Handwritten Note"
DocumentContent: "Short message content here..."
bIsEvidence: true
EvidenceMetadata:
  EvidenceId: "Evidence_Note_01"
  EvidenceName: "Mysterious Note"
  Category: "Documents"
bAutoCollectAsEvidence: true
bDisablePlayerMovement: true
bPauseGame: false
OpenSound: SFX_Paper_Rustle
CloseSound: SFX_Paper_Fold
```

### 7.2 Story Letter (Critical)
```
DocumentType: Text
DocumentTitle: "Victim's Final Letter"
DocumentContent: "Long-form letter content..."
bIsEvidence: true
EvidenceMetadata:
  EvidenceId: "Evidence_Letter_Victim"
  EvidenceName: "Final Letter"
  Category: "Critical Evidence"
  bIsOptional: false
bAutoCollectAsEvidence: true
bDisablePlayerMovement: true
bPauseGame: true
OpenSound: SFX_Paper_Rustle
CloseSound: SFX_Paper_Fold
```

### 7.3 Photo Evidence
```
DocumentType: Image
DocumentTitle: "Crime Scene Photo"
DocumentContent: "Photo taken at crime scene - [Date]"
DocumentImage: T_Photo_CrimeScene_01
bIsEvidence: true
EvidenceMetadata:
  EvidenceId: "Evidence_Photo_CrimeScene"
  EvidenceName: "Crime Scene Photo"
  Category: "Visual Evidence"
bAutoCollectAsEvidence: true
bDisablePlayerMovement: true
bPauseGame: false
OpenSound: SFX_Photo_Pickup
CloseSound: SFX_Photo_Putdown
```

### 7.4 Investigation Report (Mixed)
```
DocumentType: Mixed
DocumentTitle: "Police Investigation Report"
DocumentContent: "Detailed report text..."
DocumentImage: T_Report_Header
AdditionalImages: [T_Report_Photo1, T_Report_Photo2]
bIsEvidence: true
EvidenceMetadata:
  EvidenceId: "Evidence_Report_Police"
  EvidenceName: "Investigation Report"
  Category: "Official Documents"
bAutoCollectAsEvidence: true
bDisablePlayerMovement: true
bPauseGame: false
OpenSound: SFX_Paper_Rustle
CloseSound: SFX_Paper_Fold
```

### 7.5 Optional Lore Document
```
DocumentType: Text
DocumentTitle: "Old Diary Entry"
DocumentContent: "Optional backstory content..."
bIsEvidence: true
EvidenceMetadata:
  EvidenceId: "Evidence_Diary_Optional"
  EvidenceName: "Diary Entry"
  Category: "Lore"
  bIsOptional: true
bAutoCollectAsEvidence: true
bDisablePlayerMovement: true
bPauseGame: false
OpenSound: SFX_Book_Open
CloseSound: SFX_Book_Close
```

## 8. Advanced Features

### 8.1 Encrypted Documents
**Require decryption before reading:**
```
Variable: bIsEncrypted (Boolean) = true
Variable: DecryptionKey (Name) = "Code_Safe_01"

Override: OnInteract
├─ Branch (Is Encrypted?)
│  ├─ True:
│  │  ├─ Get Inventory Component
│  │  ├─ HasItem(DecryptionKey)
│  │  ├─ Branch (Has Key?)
│  │  │  ├─ True:
│  │  │     ├─ Set Encrypted (false)
│  │  │     ├─ Show Decryption Animation
│  │  │     ├─ Call Parent OnInteract
│  │  │     └─ Play Success Sound
│  │  │  └─ False:
│  │  │     ├─ Show "Encrypted" Message
│  │  │     └─ Play Error Sound
│  └─ False → Call Parent OnInteract
```

### 8.2 Narrated Documents
**Audio narration while reading:**
```
Variable: NarrationAudio (Sound Base)
Variable: bAutoPlayNarration (Boolean) = true

Event OnDocumentOpened
├─ Call Parent Event
├─ Branch (Has Narration?)
│  ├─ True:
│  │  ├─ Branch (Auto Play?)
│  │  │  ├─ True → Play Narration
│  │  │  └─ False → Show Play Button
│  └─ False → Continue

Custom Event: PlayNarration
├─ Play Sound 2D (NarrationAudio)
├─ Show Narration UI
├─ Highlight Current Text
└─ Sync Text with Audio
```

### 8.3 Interactive Documents
**Clickable elements in document:**
```
Custom Event: OnDocumentElementClicked
├─ Get Element ID
├─ Branch (Element Type)
│  ├─ "Hyperlink":
│  │  ├─ Open Referenced Document
│  │  └─ Play Click Sound
│  ├─ "Image":
│  │  ├─ Zoom to Full Screen
│  │  └─ Enable Image Examine
│  └─ "Annotation":
│     ├─ Show Tooltip
│     └─ Add to Notes
```

### 8.4 Translation System
**Documents in foreign languages:**
```
Variable: OriginalLanguage (String) = "Latin"
Variable: TranslatedContent (Text)
Variable: bIsTranslated (Boolean) = false

Custom Event: TranslateDocument
├─ Branch (Has Translator Tool?)
│  ├─ True:
│  │  ├─ Set Content (TranslatedContent)
│  │  ├─ Set Translated (true)
│  │  ├─ Show Translation Effect
│  │  └─ Play Translation Sound
│  └─ False:
│     ├─ Show Original Language
│     └─ Show "Cannot Translate" Message
```

### 8.5 Document Comparison
**Compare multiple documents:**
```
Custom Event: StartComparisonMode
├─ Get Selected Documents (Array)
├─ Create Comparison UI
├─ Display Documents Side-by-Side
├─ Highlight Common Elements
└─ Enable Cross-Reference Navigation
```

## 9. UI Design Guidelines

### 9.1 Document Widget Layout
**Recommended structure:**
- Background: Semi-transparent dark overlay
- Document panel: Centered, paper texture background
- Title: Top center, large readable font
- Content: Scrollable text area, comfortable margins
- Images: Embedded or side panel
- Close button: Top-right corner
- Page navigation: Bottom center (if multi-page)

### 9.2 Typography
**Text settings:**
- Font: Serif for immersion (e.g., Garamond, Times)
- Size: 16-20pt for body text
- Line height: 1.5-1.8 for readability
- Color: Dark text on light background (high contrast)
- Margins: 40-60px on all sides

### 9.3 Image Display
**Image presentation:**
- Max size: 80% of screen width/height
- Maintain aspect ratio
- Click to enlarge functionality
- Zoom controls for detail inspection
- Caption below image

## 10. Troubleshooting

### Issue: Text is unreadable
**Solution:** Increase font size, improve contrast, check text color

### Issue: Document doesn't open
**Solution:** Verify DocumentContent is not empty, check UI widget binding

### Issue: Player can still move while reading
**Solution:** Ensure bDisablePlayerMovement is true

### Issue: Evidence not collecting
**Solution:** Check bIsEvidence is true and EvidenceMetadata is filled

### Issue: Images not displaying
**Solution:** Verify texture assets are valid and not null

### Issue: Can't close document
**Solution:** Check close input binding and CloseDocument function

### Issue: Multi-page navigation broken
**Solution:** Verify page array is populated and indices are valid

### Issue: Audio doesn't play
**Solution:** Check OpenSound/CloseSound assets are assigned

### Issue: Document UI overlaps other UI
**Solution:** Adjust UI layer z-order, ensure proper widget hierarchy
