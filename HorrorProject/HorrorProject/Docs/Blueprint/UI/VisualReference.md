# UI Visual Reference Guide

## Overview
ASCII art and text-based visual references for UI Widget layouts in the HorrorProject.

---

## Screen Resolutions Reference

### Common Resolutions
```
1920x1080 (1080p, Full HD)    - Most common
2560x1440 (1440p, 2K)         - High-end gaming
3840x2160 (4K, UHD)           - Ultra high-end
1280x720  (720p, HD)          - Minimum spec
```

### Safe Zones
```
┌─────────────────────────────────────────────────────────────┐
│ ← 5% margin                                    5% margin → │
│ ┌─────────────────────────────────────────────────────────┐ │
│ │                                                         │ │
│ │                     SAFE ZONE                           │ │
│ │              (90% of screen width)                      │ │
│ │                                                         │ │
│ └─────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

---

## WBP_BodycamOverlay Layout

### Full Screen View (1920x1080)
```
┌─────────────────────────────────────────────────────────────┐
│ ● REC  00:15:32                          [████░░] 80%       │ ← Top Bar (60px height)
│                                                             │
│                                                             │
│                                                             │
│                                                             │
│                                                             │
│                    [GAME VIEW AREA]                         │
│                                                             │
│                                                             │
│                                                             │
│                                                             │
│                                                             │
│ [VHS SCANLINES + NOISE OVERLAY - FULL SCREEN]              │
└─────────────────────────────────────────────────────────────┘
  ↑                                                           ↑
  Vignette darkening at edges                    Vignette darkening
```

### Top Bar Detail
```
┌─────────────────────────────────────────────────────────────┐
│ ● REC  00:15:32                          [████░░] 80%       │
│ ↑  ↑    ↑                                 ↑       ↑         │
│ │  │    │                                 │       │         │
│ │  │    └─ Timestamp (18pt, white)       │       └─ Battery % (16pt)
│ │  └────── "REC" label (18pt, red)       │                 │
│ └───────── Red dot (16x16, blinks)       └─ Battery bar (120x20)
│                                                             │
│ Padding: 20px left, 10px top                                │
└─────────────────────────────────────────────────────────────┘
```

### Recording States
```
NOT RECORDING:
┌─────────────────────────────────────────────────────────────┐
│ 00:15:32                                     [████░░] 80%    │
│ (no dot, no "REC" label)                                    │
└─────────────────────────────────────────────────────────────┘

RECORDING:
┌─────────────────────────────────────────────────────────────┐
│ ● REC  00:15:32                              [████░░] 80%    │
│ (blinking dot and label)                                    │
└─────────────────────────────────────────────────────────────┘
```

### Battery States
```
FULL (100-50%):
[████████████] 100%  (Green)

MEDIUM (50-20%):
[██████░░░░░░] 45%   (Yellow)

LOW (20-0%):
[██░░░░░░░░░░] 15%   (Red, pulsing)
```

---

## WBP_EvidenceToast Layout

### Toast Position (Bottom-Left)
```
┌─────────────────────────────────────────────────────────────┐
│                                                             │
│                                                             │
│                                                             │
│                                                             │
│                                                             │
│                                                             │
│                                                             │
│                                                             │
│  ┌────────────────────────────────────────────────────┐    │
│  │ [!] EVIDENCE COLLECTED                             │    │ ← Toast (450x140)
│  │ Mysterious Note                                    │    │
│  │ A torn page with cryptic symbols...                │    │
│  └────────────────────────────────────────────────────┘    │
│  ↑                                                          │
│  5% from left, 15% from bottom                              │
└─────────────────────────────────────────────────────────────┘
```

### Toast Detail
```
┌────────────────────────────────────────────────────┐
│ [!] EVIDENCE COLLECTED                             │ ← Header (Yellow)
│ ↑   ↑                                              │
│ │   └─ Header text (16pt, yellow, bold)           │
│ └───── Icon (24x24, yellow, pulsing)              │
│                                                    │
│ Mysterious Note                                    │ ← Evidence name (20pt, white, bold)
│                                                    │
│ A torn page with cryptic symbols...                │ ← Description (14pt, light gray)
│                                                    │
└────────────────────────────────────────────────────┘
  ↑                                                  ↑
  Black background (0,0,0,0.85)    Yellow outline (2px)
```

### Animation States
```
HIDDEN (Initial):
  Position: Y = normal
  Opacity: 0.0
  Scale: 0.95

FADE IN (0.3s):
  Position: Y = normal - 30 → normal
  Opacity: 0.0 → 1.0
  Scale: 0.95 → 1.0

VISIBLE (3.0s):
  Position: Y = normal
  Opacity: 1.0
  Scale: 1.0
  Icon: Pulsing (1.0 → 1.2 → 1.0)

FADE OUT (0.5s):
  Position: Y = normal → normal + 20
  Opacity: 1.0 → 0.0
```

---

## WBP_ObjectiveToast Layout

### Toast Position (Top-Left)
```
┌─────────────────────────────────────────────────────────────┐
│                                                             │
│  ┌────────────────────────────────────────────────────┐    │
│  │ >> NEW OBJECTIVE                                   │    │ ← Toast (500x150)
│  │                                                    │    │
│  │ Find the basement key                              │    │
│  │ [Hint: Check the kitchen drawers]                  │    │
│  └────────────────────────────────────────────────────┘    │
│  ↑                                                          │
│  5% from left, 10% from top                                 │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### Toast Detail
```
┌────────────────────────────────────────────────────┐
│ >> NEW OBJECTIVE                                   │ ← Header (Cyan)
│ ↑  ↑                                               │
│ │  └─ Header text (18pt, cyan, bold)              │
│ └──── Arrow (20pt, cyan, blinking + moving)       │
│                                                    │
│ Find the basement key                              │ ← Objective (22pt, white, bold)
│                                                    │
│ [Hint: Check the kitchen drawers]                  │ ← Hint (14pt, light cyan, italic)
│                                                    │
└────────────────────────────────────────────────────┘
  ↑                                                  ↑
  Dark blue background (0.05,0.1,0.2,0.9)  Cyan outline (2px)
```

### Slide Animation
```
OFF-SCREEN (Initial):
  Position: X = -550 (left of screen)
  Opacity: 0.0

SLIDE IN (0.4s with bounce):
  Position: X = -550 → 0 (with overshoot)
  Opacity: 0.0 → 1.0
  
  Timeline:
  0.0s: X = -550
  0.2s: X = 20 (overshoot)
  0.4s: X = 0 (settle)

VISIBLE:
  Position: X = 0
  Arrow: Blinking (opacity 1.0 → 0.3 → 1.0)
  Arrow: Moving (X offset 0 → 5 → 0)

SLIDE OUT (0.3s):
  Position: X = 0 → -550
  Opacity: 1.0 → 0.0
```

### Objective Types
```
MAIN OBJECTIVE (Cyan):
┌────────────────────────────────────────────────────┐
│ >> NEW OBJECTIVE                                   │ (Cyan)
│ Find the basement key                              │
└────────────────────────────────────────────────────┘

OPTIONAL OBJECTIVE (Yellow):
┌────────────────────────────────────────────────────┐
│ >> OPTIONAL OBJECTIVE                              │ (Yellow)
│ Collect all notes                                  │
└────────────────────────────────────────────────────┘

FAILED OBJECTIVE (Red):
┌────────────────────────────────────────────────────┐
│ >> OBJECTIVE FAILED                                │ (Red)
│ Time ran out                                       │
└────────────────────────────────────────────────────┘
```

---

## WBP_ArchiveMenu Layout

### Full Screen View (1920x1080)
```
┌─────────────────────────────────────────────────────────────┐
│ [X] ARCHIVE                    [ALL][EVIDENCE][NOTES][PHOTOS]│ ← Header (80px)
├─────────────────┬───────────────────────────────────────────┤
│                 │                                           │
│  Evidence List  │         Detail View                       │
│  (400px wide)   │         (Fill remaining)                  │
│                 │                                           │
│ [√] Note #1     │  ┌─────────────────────────────────┐     │
│ [ ] Photo #2    │  │                                 │     │
│ [√] Document    │  │     [Evidence Image/Content]    │     │
│ [ ] Audio Log   │  │         (400px height)          │     │
│ [√] Note #2     │  │                                 │     │
│ [ ] Photo #3    │  └─────────────────────────────────┘     │
│ [√] Document #2 │                                           │
│ [ ] Audio #2    │  Title: Mysterious Note                   │
│                 │  Type: Document                           │
│ (scrollable)    │  Location: Kitchen                        │
│                 │                                           │
│                 │  Description text here...                 │
│                 │  (scrollable)                             │
│                 │                                           │
└─────────────────┴───────────────────────────────────────────┘
```

### Header Detail
```
┌─────────────────────────────────────────────────────────────┐
│ [X] ARCHIVE                    [ALL][EVIDENCE][NOTES][PHOTOS]│
│ ↑   ↑                           ↑    ↑         ↑      ↑     │
│ │   │                           │    │         │      │     │
│ │   └─ Title (32pt, cyan)       │    │         │      │     │
│ └───── Close button (40x40)     │    │         │      │     │
│                                 │    │         │      │     │
│                                 └────┴─────────┴──────┘     │
│                                 Filter buttons (80-100px)    │
│                                                             │
│ Padding: 20px all sides                                     │
└─────────────────────────────────────────────────────────────┘
```

### List Panel Detail
```
┌─────────────────┐
│  Evidence List  │ ← Panel title
├─────────────────┤
│ [√] Note #1     │ ← List item (80px height)
│ ↑   ↑           │
│ │   └─ Title    │
│ └───── Checkmark (collected)
│                 │
│ [ ] Photo #2    │ ← Uncollected item
│                 │
│ [√] Document    │
│                 │
│ [ ] Audio Log   │
│                 │
│ (scroll bar →)  │
└─────────────────┘
  400px wide
```

### List Item Detail
```
┌────────────────────────────────────────┐
│ [√] Mysterious Note                    │ ← Selected (cyan background)
│ ↑   ↑                                  │
│ │   └─ Title (18pt, white)            │
│ └───── Checkmark (24x24, green)       │
│     Found in: Kitchen                  │ ← Subtitle (14pt, gray)
└────────────────────────────────────────┘
  80px height, 400px wide

┌────────────────────────────────────────┐
│ [ ] Torn Photograph                    │ ← Not collected (dark background)
│ ↑   ↑                                  │
│ │   └─ Title (18pt, gray)             │
│ └───── Empty checkbox (24x24)         │
│     Location: ???                      │ ← Unknown location
└────────────────────────────────────────┘
```

### Detail Panel States
```
NO SELECTION:
┌───────────────────────────────────────────┐
│                                           │
│                                           │
│         Select an entry to view           │
│              details                      │
│                                           │
│                                           │
└───────────────────────────────────────────┘

WITH SELECTION:
┌───────────────────────────────────────────┐
│ ┌─────────────────────────────────┐       │
│ │                                 │       │
│ │     [Evidence Image]            │       │
│ │                                 │       │
│ └─────────────────────────────────┘       │
│                                           │
│ Mysterious Note                           │ ← Title (28pt, white)
│ Type: Document                            │ ← Metadata (16pt, cyan)
│ Location: Kitchen                         │ ← Metadata (16pt, gray)
│                                           │
│ A torn page with cryptic symbols...       │ ← Description (18pt, white)
│ The text appears to be written in an      │
│ ancient language...                       │
│                                           │
│ (scrollable if long)                      │
└───────────────────────────────────────────┘
```

---

## WBP_PauseMenu Layout

### Full Screen View (1920x1080)
```
┌─────────────────────────────────────────────────────────────┐
│                                                             │
│                                                             │
│                      ┌─────────────┐                        │
│                      │   PAUSED    │                        │ ← Title (48pt, cyan)
│                      └─────────────┘                        │
│                                                             │
│                      ┌─────────────┐                        │
│                      │   RESUME    │                        │ ← Button (400x60)
│                      └─────────────┘                        │
│                                                             │
│                      ┌─────────────┐                        │
│                      │  SETTINGS   │                        │ ← Button (400x60)
│                      └─────────────┘                        │
│                                                             │
│                      ┌─────────────┐                        │
│                      │QUIT TO MENU │                        │ ← Button (400x60, red)
│                      └─────────────┘                        │
│                                                             │
│                                                             │
└─────────────────────────────────────────────────────────────┘
  ↑                                                           ↑
  Dark background (0,0,0,0.8)                    Scanline overlay
```

### Button States
```
NORMAL:
┌─────────────────────────────────────────┐
│              RESUME                     │ (Dark gray background)
└─────────────────────────────────────────┘
  Outline: 2px cyan

HOVERED:
┌─────────────────────────────────────────┐
│              RESUME                     │ (Gray background, brighter)
└─────────────────────────────────────────┘
  Outline: 2px cyan (brighter)
  Scale: 1.05 (pulsing)

PRESSED:
┌─────────────────────────────────────────┐
│              RESUME                     │ (Cyan tint background)
└─────────────────────────────────────────┘
  Outline: 2px cyan (bright)
```

### Quit Button (Special Styling)
```
NORMAL:
┌─────────────────────────────────────────┐
│          QUIT TO MENU                   │ (Dark gray background)
└─────────────────────────────────────────┘
  Outline: 2px red
  Text: Light red (1.0, 0.6, 0.6)

HOVERED:
┌─────────────────────────────────────────┐
│          QUIT TO MENU                   │ (Gray background)
└─────────────────────────────────────────┘
  Outline: 2px red (brighter)
  Text: Light red
```

---

## WBP_SettingsMenu Layout

### Full Screen View (1920x1080)
```
┌─────────────────────────────────────────────────────────────┐
│ [X] SETTINGS                              *Unsaved Changes  │ ← Header (80px)
├─────────────┬───────────────────────────────────────────────┤
│             │                                               │
│  GRAPHICS   │  Graphics Settings                            │
│  AUDIO      │                                               │
│  CONTROLS   │  Resolution: [1920x1080 ▼]                    │
│  GAMEPLAY   │  Window Mode: [Fullscreen ▼]                  │
│  ACCESS.    │  Quality: [High ▼]                            │
│             │  VSync: [✓]                                   │
│ (200px)     │  Frame Limit: [60 ▼]                          │
│             │                                               │
│             │  VHS Effects: [████████░░] 80%                │
│             │  Brightness: [█████░░░░░] 50%                 │
│             │                                               │
│             │  (scrollable)                                 │
│             │                                               │
├─────────────┴───────────────────────────────────────────────┤
│                    [APPLY]  [RESET]  [CLOSE]                │ ← Footer (80px)
└─────────────────────────────────────────────────────────────┘
```

### Category Panel Detail
```
┌─────────────┐
│  GRAPHICS   │ ← Active (cyan background)
├─────────────┤
│  AUDIO      │ ← Inactive (dark gray)
├─────────────┤
│  CONTROLS   │
├─────────────┤
│  GAMEPLAY   │
├─────────────┤
│  ACCESS.    │
└─────────────┘
  200px wide
  50px height per button
```

### Settings Control Types
```
COMBO BOX:
Resolution: [1920x1080 ▼]
            ↑           ↑
            Value       Dropdown arrow

CHECKBOX:
VSync: [✓]
       ↑
       Checked state

SLIDER:
VHS Effects: [████████░░] 80%
             ↑           ↑
             Bar         Value display

TEXT INPUT (Key Binding):
Forward: [W]  [Change]
         ↑    ↑
         Current  Button to rebind
```

### Settings Page Example (Graphics)
```
┌───────────────────────────────────────────┐
│  Graphics Settings                        │
│                                           │
│  Resolution:                              │
│  [1920x1080 ▼]                            │
│                                           │
│  Window Mode:                             │
│  [Fullscreen ▼]                           │
│                                           │
│  Graphics Quality:                        │
│  [High ▼]                                 │
│                                           │
│  VSync:                                   │
│  [✓] Enable VSync                         │
│                                           │
│  Frame Rate Limit:                        │
│  [60 ▼]                                   │
│                                           │
│  VHS Effect Intensity:                    │
│  [████████░░] 80%                         │
│                                           │
│  Brightness:                              │
│  [█████░░░░░] 50%                         │
│                                           │
│  (more settings...)                       │
└───────────────────────────────────────────┘
```

---

## Responsive Layout Examples

### 1920x1080 (Full HD)
```
┌─────────────────────────────────────────────────────────────┐
│                         FULL LAYOUT                         │
│                     All elements visible                    │
│                    Optimal spacing                          │
└─────────────────────────────────────────────────────────────┘
```

### 1280x720 (HD)
```
┌───────────────────────────────────────────────┐
│              COMPACT LAYOUT                   │
│          Reduced spacing                      │
│          Smaller fonts                        │
└───────────────────────────────────────────────┘
```

### 3840x2160 (4K)
```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              EXPANDED LAYOUT                                │
│                          Increased spacing                                  │
│                          Larger fonts                                       │
│                          More detail visible                                │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## Z-Order Reference

### Layer Stack (Bottom to Top)
```
Layer 0:    Game World
Layer 100:  Bodycam Overlay (VHS effects)
Layer 200:  Evidence Toast
Layer 300:  Objective Toast
Layer 1000: Archive Menu
Layer 2000: Pause Menu
Layer 2500: Settings Menu
Layer 3000: Confirmation Dialogs
Layer 9999: Debug Overlays
```

### Widget Internal Z-Order
```
Bodycam Overlay:
  0: VHS Overlay
  1: Scanlines
  2: Noise
  10: UI Elements (text, bars)
  20: Vignette

Archive Menu:
  0: Background
  10: Content (list, detail)
  100: Scanline overlay

Pause Menu:
  0: Background blur
  10: Menu content
  100: Scanline overlay
```

---

## Color Coding Reference

### UI Element Colors
```
PRIMARY (Cyan):
████ (0, 0.8, 1.0) - Headers, outlines, highlights

EVIDENCE (Yellow):
████ (1.0, 0.8, 0.0) - Evidence icons, warnings

DANGER (Red):
████ (1.0, 0.2, 0.2) - Recording, quit, errors

SUCCESS (Green):
████ (0.0, 1.0, 0.4) - Battery full, checkmarks

TEXT PRIMARY (White):
████ (1.0, 1.0, 1.0) - Main text

TEXT SECONDARY (Gray):
████ (0.7, 0.7, 0.7) - Descriptions, metadata

BACKGROUND (Black):
████ (0.0, 0.0, 0.0, 0.95) - Menu backgrounds
```

---

## Animation Timing Reference

### Standard Durations
```
INSTANT:        0.0s  - State changes
VERY FAST:      0.1s  - Button feedback
FAST:           0.2s  - Fade in/out
NORMAL:         0.3s  - Slide animations
SLOW:           0.5s  - Complex animations
VERY SLOW:      1.0s  - Looping effects
```

### Easing Curves
```
LINEAR:         ────────  Constant speed
EASE IN:        ╭───────  Slow start, fast end
EASE OUT:       ───────╮  Fast start, slow end
EASE IN-OUT:    ╭─────╮  Slow start and end
BOUNCE:         ╭─╮─╮──  Overshoot and settle
```

---

## Spacing and Padding Reference

### Standard Spacing
```
TIGHT:      4px   - Between related elements
NORMAL:     8px   - Between UI elements
MEDIUM:     16px  - Between sections
LARGE:      20px  - Panel padding
EXTRA:      40px  - Major sections
```

### Container Padding
```
┌─────────────────────────────────────────┐
│ ← 20px                         20px →  │ ← Header/Footer
│                                         │
│ ← 40px                         40px →  │ ← Content panels
│                                         │
│ ← 10px                         10px →  │ ← List panels
└─────────────────────────────────────────┘
```

---

## Font Size Reference

### Text Hierarchy
```
TITLE:          48pt  - Menu titles (PAUSED, ARCHIVE)
LARGE HEADER:   32pt  - Section headers
HEADER:         28pt  - Entry titles
SUBHEADER:      22pt  - Objective text
BODY LARGE:     20pt  - Evidence names
BODY:           18pt  - Descriptions, settings labels
BODY SMALL:     16pt  - Metadata, hints
CAPTION:        14pt  - Subtitles, help text
TINY:           12pt  - Timestamps, fine print
```

---

## Icon Size Reference

### Standard Icon Sizes
```
TINY:       16x16   - Recording dot
SMALL:      24x24   - Checkmarks, close buttons
MEDIUM:     32x32   - List item icons
LARGE:      48x48   - Feature icons
HUGE:       64x64   - Main menu icons
```

---

This visual reference guide provides ASCII art representations of all UI layouts. Use these as a quick reference when implementing widgets in Unreal Engine.
