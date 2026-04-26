# UI Blueprint Asset Requirements

## Overview
Complete list of all assets needed for UI Widget Blueprint implementation.

---

## Fonts

### F_HomeVideo
**Type:** TrueType Font (TTF)  
**Purpose:** Primary UI font for VHS aesthetic  
**Characteristics:**
- Monospace
- VHS/retro style
- Readable at small sizes
- Supports ASCII characters

**Usage:**
- All UI text elements
- Bodycam overlay timestamp
- Evidence toast notifications
- Objective toast text
- Archive menu entries
- Pause menu buttons
- Settings menu labels

**Recommended Font:** "VCR OSD Mono" or similar VHS-style monospace font

**Import Settings:**
- Font Cache Type: Runtime
- Hinting: Default
- Loading Policy: Inline
- Sizes to Cache: 12, 14, 16, 18, 20, 22, 24, 28, 32, 48

**Location:** `Content/UI/Fonts/F_HomeVideo`

---

## Textures

### Icons (32x32)

#### T_IconEvidence
**Size:** 32x32 pixels  
**Format:** PNG with alpha  
**Purpose:** Evidence icon in archive list  
**Color:** Yellow/Gold tint  
**Location:** `Content/UI/Textures/Icons/T_IconEvidence`

#### T_IconNote
**Size:** 32x32 pixels  
**Format:** PNG with alpha  
**Purpose:** Note icon in archive list  
**Color:** White/Paper tint  
**Location:** `Content/UI/Textures/Icons/T_IconNote`

#### T_IconPhoto
**Size:** 32x32 pixels  
**Format:** PNG with alpha  
**Purpose:** Photo icon in archive list  
**Color:** Cyan tint  
**Location:** `Content/UI/Textures/Icons/T_IconPhoto`

#### T_IconAudio
**Size:** 32x32 pixels  
**Format:** PNG with alpha  
**Purpose:** Audio log icon in archive list  
**Color:** Green tint  
**Location:** `Content/UI/Textures/Icons/T_IconAudio`

#### T_EvidenceIcon
**Size:** 24x24 pixels  
**Format:** PNG with alpha  
**Purpose:** Evidence collected toast icon  
**Design:** Exclamation mark or document icon  
**Color:** Yellow  
**Location:** `Content/UI/Textures/Icons/T_EvidenceIcon`

### UI Elements

#### T_CheckMark
**Size:** 24x24 pixels  
**Format:** PNG with alpha  
**Purpose:** Checkmark for collected items  
**Color:** Green or Cyan  
**Location:** `Content/UI/Textures/Icons/T_CheckMark`

#### T_CloseButton
**Size:** 40x40 pixels  
**Format:** PNG with alpha  
**Purpose:** Close button icon (X)  
**Color:** White  
**Location:** `Content/UI/Textures/Icons/T_CloseButton`

#### T_RecDot
**Size:** 16x16 pixels  
**Format:** PNG with alpha  
**Purpose:** Recording indicator dot  
**Color:** Red  
**Alternative:** Can use solid color instead  
**Location:** `Content/UI/Textures/Icons/T_RecDot`

#### T_BatteryIcon
**Size:** 24x24 pixels  
**Format:** PNG with alpha  
**Purpose:** Battery icon (optional)  
**Color:** White  
**Location:** `Content/UI/Textures/Icons/T_BatteryIcon`

### Noise Textures

#### T_FilmGrain
**Size:** 512x512 pixels  
**Format:** PNG grayscale  
**Purpose:** Film grain noise overlay  
**Tiling:** Yes  
**Location:** `Content/UI/Textures/Effects/T_FilmGrain`

#### T_NoisePattern
**Size:** 256x256 pixels  
**Format:** PNG grayscale  
**Purpose:** VHS noise pattern  
**Tiling:** Yes  
**Location:** `Content/UI/Textures/Effects/T_NoisePattern`

---

## Materials

### VHS Effect Materials

#### M_VHS_Overlay
**Type:** Material (User Interface domain)  
**Purpose:** VHS distortion effect for bodycam overlay  
**Location:** `Content/UI/Materials/M_VHS_Overlay`

**Parameters:**
- Intensity (Scalar): 0.0 - 1.0, Default: 0.3
- ChromaticAberration (Scalar): 0.0 - 1.0, Default: 0.5
- DistortionAmount (Scalar): 0.0 - 1.0, Default: 0.2
- TimeOffset (Scalar): 0.0+, Default: 0.0

**Material Graph:**
```
TexCoord
├→ Distortion (Sine wave based on TimeOffset)
├→ Chromatic Aberration (RGB channel offset)
└→ Multiply by Intensity
   └→ Output to Emissive Color
```

**Shader Instructions:** < 50  
**Texture Samples:** 1-2

#### M_Scanlines
**Type:** Material (User Interface domain)  
**Purpose:** CRT scanline effect  
**Location:** `Content/UI/Materials/M_Scanlines`

**Parameters:**
- ScanlineCount (Scalar): 100 - 1000, Default: 480
- ScanlineIntensity (Scalar): 0.0 - 1.0, Default: 0.2
- ScanlineSpeed (Scalar): 0.0 - 2.0, Default: 0.5
- TimeOffset (Scalar): 0.0+, Default: 0.0

**Material Graph:**
```
TexCoord Y
├→ Multiply by ScanlineCount
├→ Add TimeOffset * ScanlineSpeed
├→ Frac
├→ Step (0.5)
└→ Multiply by ScanlineIntensity
   └→ Output to Opacity
```

**Shader Instructions:** < 30  
**Texture Samples:** 0

#### M_FilmGrain
**Type:** Material (User Interface domain)  
**Purpose:** Film grain noise overlay  
**Location:** `Content/UI/Materials/M_FilmGrain`

**Parameters:**
- NoiseAmount (Scalar): 0.0 - 1.0, Default: 0.15
- NoiseScale (Scalar): 0.5 - 2.0, Default: 1.0
- TimeOffset (Scalar): 0.0+, Default: 0.0

**Material Graph:**
```
TexCoord
├→ Multiply by NoiseScale
├→ Add TimeOffset
├→ Noise Node (or Texture Sample T_FilmGrain)
└→ Multiply by NoiseAmount
   ├→ Output to Emissive Color
   └→ Output to Opacity
```

**Shader Instructions:** < 40  
**Texture Samples:** 1

#### M_Vignette
**Type:** Material (User Interface domain)  
**Purpose:** Vignette darkening at screen edges  
**Location:** `Content/UI/Materials/M_Vignette`

**Parameters:**
- VignetteIntensity (Scalar): 0.0 - 1.0, Default: 0.4
- VignetteRadius (Scalar): 0.0 - 1.0, Default: 0.7

**Material Graph:**
```
TexCoord
├→ Subtract (0.5, 0.5) [Center]
├→ Length [Distance from center]
├→ Smoothstep (VignetteRadius, 1.0)
└→ Multiply by VignetteIntensity
   └→ Output to Opacity
```

**Shader Instructions:** < 25  
**Texture Samples:** 0

### Background Materials

#### M_BackgroundBlur
**Type:** Material (User Interface domain)  
**Purpose:** Blur effect for pause menu background  
**Location:** `Content/UI/Materials/M_BackgroundBlur`

**Parameters:**
- BlurAmount (Scalar): 0.0 - 10.0, Default: 5.0

**Material Graph:**
```
Scene Texture (Post Process Input 0)
├→ Blur (Gaussian or Box)
└→ Output to Emissive Color
```

**Shader Instructions:** < 60  
**Texture Samples:** 5-9 (depending on blur quality)

#### M_ToastBackground
**Type:** Material (User Interface domain)  
**Purpose:** Subtle noise for toast backgrounds  
**Location:** `Content/UI/Materials/M_ToastBackground`

**Parameters:**
- NoiseAmount (Scalar): 0.0 - 0.2, Default: 0.05
- ScanlineIntensity (Scalar): 0.0 - 0.5, Default: 0.1
- TimeOffset (Scalar): 0.0+, Default: 0.0

**Material Graph:**
```
Combine:
├→ Noise (NoiseAmount)
└→ Scanlines (ScanlineIntensity)
   └→ Output to Emissive Color + Opacity (0.3)
```

**Shader Instructions:** < 35  
**Texture Samples:** 1

#### M_ScanlineSubtle
**Type:** Material (User Interface domain)  
**Purpose:** Very subtle scanline effect for archive menu  
**Location:** `Content/UI/Materials/M_ScanlineSubtle`

**Parameters:**
- ScanlineIntensity (Scalar): 0.0 - 0.3, Default: 0.1

**Material Graph:**
```
Same as M_Scanlines but with lower default intensity
```

**Shader Instructions:** < 30  
**Texture Samples:** 0

---

## Sounds

### Menu Sounds

#### SFX_MenuOpen
**Format:** WAV, 16-bit, 44.1kHz  
**Duration:** 0.2 - 0.5 seconds  
**Purpose:** Menu open sound  
**Description:** Whoosh or electronic beep  
**Volume:** -6dB  
**Location:** `Content/Audio/UI/SFX_MenuOpen`

#### SFX_MenuClose
**Format:** WAV, 16-bit, 44.1kHz  
**Duration:** 0.2 - 0.4 seconds  
**Purpose:** Menu close sound  
**Description:** Reverse whoosh or click  
**Volume:** -6dB  
**Location:** `Content/Audio/UI/SFX_MenuClose`

#### SFX_PauseOpen
**Format:** WAV, 16-bit, 44.1kHz  
**Duration:** 0.2 - 0.3 seconds  
**Purpose:** Pause menu open sound  
**Description:** Time stop effect or beep  
**Volume:** -6dB  
**Location:** `Content/Audio/UI/SFX_PauseOpen`

#### SFX_PauseClose
**Format:** WAV, 16-bit, 44.1kHz  
**Duration:** 0.2 - 0.3 seconds  
**Purpose:** Pause menu close sound  
**Description:** Time resume effect  
**Volume:** -6dB  
**Location:** `Content/Audio/UI/SFX_PauseClose`

### Button Sounds

#### SFX_ButtonClick
**Format:** WAV, 16-bit, 44.1kHz  
**Duration:** 0.1 - 0.2 seconds  
**Purpose:** Button click feedback  
**Description:** Click or beep  
**Volume:** -9dB  
**Location:** `Content/Audio/UI/SFX_ButtonClick`

#### SFX_ButtonHover
**Format:** WAV, 16-bit, 44.1kHz  
**Duration:** 0.05 - 0.1 seconds  
**Purpose:** Button hover feedback  
**Description:** Subtle tick or beep  
**Volume:** -12dB  
**Location:** `Content/Audio/UI/SFX_ButtonHover`

### Notification Sounds

#### SFX_EvidenceCollected
**Format:** WAV, 16-bit, 44.1kHz  
**Duration:** 0.5 - 1.0 seconds  
**Purpose:** Evidence collection notification  
**Description:** Success chime or jingle  
**Volume:** -6dB  
**Location:** `Content/Audio/UI/SFX_EvidenceCollected`

#### SFX_ObjectiveUpdate
**Format:** WAV, 16-bit, 44.1kHz  
**Duration:** 0.5 - 1.0 seconds  
**Purpose:** Objective update notification  
**Description:** Alert tone or beep  
**Volume:** -6dB  
**Location:** `Content/Audio/UI/SFX_ObjectiveUpdate`

#### SFX_UINotification
**Format:** WAV, 16-bit, 44.1kHz  
**Duration:** 0.3 - 0.5 seconds  
**Purpose:** General UI notification  
**Description:** Soft beep or chime  
**Volume:** -9dB  
**Location:** `Content/Audio/UI/SFX_UINotification`

#### SFX_UIDismiss
**Format:** WAV, 16-bit, 44.1kHz  
**Duration:** 0.2 - 0.4 seconds  
**Purpose:** Toast dismiss sound  
**Description:** Fade out whoosh  
**Volume:** -9dB  
**Location:** `Content/Audio/UI/SFX_UIDismiss`

### Special Effect Sounds

#### SFX_VHSGlitch
**Format:** WAV, 16-bit, 44.1kHz  
**Duration:** 0.3 - 0.5 seconds  
**Purpose:** VHS glitch effect  
**Description:** Static burst or distortion  
**Volume:** -6dB  
**Location:** `Content/Audio/UI/SFX_VHSGlitch`

#### SFX_RecordingStart
**Format:** WAV, 16-bit, 44.1kHz  
**Duration:** 0.3 - 0.5 seconds  
**Purpose:** Camera recording start  
**Description:** Camera beep or mechanical click  
**Volume:** -6dB  
**Location:** `Content/Audio/UI/SFX_RecordingStart`

#### SFX_RecordingStop
**Format:** WAV, 16-bit, 44.1kHz  
**Duration:** 0.3 - 0.5 seconds  
**Purpose:** Camera recording stop  
**Description:** Camera beep (different pitch)  
**Volume:** -6dB  
**Location:** `Content/Audio/UI/SFX_RecordingStop`

#### SFX_LowBattery
**Format:** WAV, 16-bit, 44.1kHz  
**Duration:** 0.5 - 1.0 seconds  
**Purpose:** Low battery warning  
**Description:** Warning beep or alarm  
**Volume:** -3dB (louder for warning)  
**Location:** `Content/Audio/UI/SFX_LowBattery`

#### SFX_TypewriterClick
**Format:** WAV, 16-bit, 44.1kHz  
**Duration:** 0.05 - 0.1 seconds  
**Purpose:** Typewriter character sound  
**Description:** Mechanical key click  
**Volume:** -15dB (very quiet, plays frequently)  
**Location:** `Content/Audio/UI/SFX_TypewriterClick`

### Archive Menu Sounds

#### SFX_ItemSelect
**Format:** WAV, 16-bit, 44.1kHz  
**Duration:** 0.2 - 0.3 seconds  
**Purpose:** Archive item selection  
**Description:** Click or select beep  
**Volume:** -9dB  
**Location:** `Content/Audio/UI/SFX_ItemSelect`

#### SFX_FilterChange
**Format:** WAV, 16-bit, 44.1kHz  
**Duration:** 0.2 - 0.3 seconds  
**Purpose:** Filter change sound  
**Description:** Switch or toggle sound  
**Volume:** -9dB  
**Location:** `Content/Audio/UI/SFX_FilterChange`

#### SFX_PageTurn
**Format:** WAV, 16-bit, 44.1kHz  
**Duration:** 0.3 - 0.5 seconds  
**Purpose:** Page navigation (optional)  
**Description:** Paper rustle or page flip  
**Volume:** -9dB  
**Location:** `Content/Audio/UI/SFX_PageTurn`

### Settings Menu Sounds

#### SFX_SettingsApplied
**Format:** WAV, 16-bit, 44.1kHz  
**Duration:** 0.5 - 0.8 seconds  
**Purpose:** Settings applied confirmation  
**Description:** Success chime  
**Volume:** -6dB  
**Location:** `Content/Audio/UI/SFX_SettingsApplied`

#### SFX_CategoryChange
**Format:** WAV, 16-bit, 44.1kHz  
**Duration:** 0.2 - 0.3 seconds  
**Purpose:** Settings category change  
**Description:** Tab switch sound  
**Volume:** -9dB  
**Location:** `Content/Audio/UI/SFX_CategoryChange`

#### SFX_SliderChange
**Format:** WAV, 16-bit, 44.1kHz  
**Duration:** 0.05 - 0.1 seconds  
**Purpose:** Slider value change (optional)  
**Description:** Subtle tick  
**Volume:** -15dB  
**Location:** `Content/Audio/UI/SFX_SliderChange`

#### SFX_UISlideIn
**Format:** WAV, 16-bit, 44.1kHz  
**Duration:** 0.3 - 0.5 seconds  
**Purpose:** Slide-in animation sound  
**Description:** Whoosh from left  
**Volume:** -9dB  
**Location:** `Content/Audio/UI/SFX_UISlideIn`

#### SFX_UISlideOut
**Format:** WAV, 16-bit, 44.1kHz  
**Duration:** 0.2 - 0.4 seconds  
**Purpose:** Slide-out animation sound  
**Description:** Whoosh to left  
**Volume:** -9dB  
**Location:** `Content/Audio/UI/SFX_UISlideOut`

---

## Color Palette

### Primary Colors

#### Cyan (Primary UI Color)
- RGB: (0, 204, 255)
- Hex: #00CCFF
- Linear: (0.0, 0.8, 1.0, 1.0)
- Usage: Headers, outlines, highlights, active states

#### Yellow (Evidence/Warning)
- RGB: (255, 204, 0)
- Hex: #FFCC00
- Linear: (1.0, 0.8, 0.0, 1.0)
- Usage: Evidence icons, warnings, unsaved indicators

#### Red (Danger/Recording)
- RGB: (255, 51, 51)
- Hex: #FF3333
- Linear: (1.0, 0.2, 0.2, 1.0)
- Usage: Recording indicator, quit button, errors

#### Green (Success/Battery)
- RGB: (0, 255, 102)
- Hex: #00FF66
- Linear: (0.0, 1.0, 0.4, 1.0)
- Usage: Battery full, success states, checkmarks

### Background Colors

#### Black (Primary Background)
- RGB: (0, 0, 0)
- Hex: #000000
- Linear: (0.0, 0.0, 0.0, 0.95)
- Usage: Full-screen menu backgrounds

#### Very Dark Gray (Panel Background)
- RGB: (13, 13, 13)
- Hex: #0D0D0D
- Linear: (0.05, 0.05, 0.05, 1.0)
- Usage: Detail panels, settings panels

#### Dark Gray (Secondary Background)
- RGB: (20, 20, 20)
- Hex: #141414
- Linear: (0.08, 0.08, 0.08, 1.0)
- Usage: List panels, category panels

#### Medium Gray (Button Normal)
- RGB: (38, 38, 38)
- Hex: #262626
- Linear: (0.15, 0.15, 0.15, 1.0)
- Usage: Button normal state

#### Light Gray (Button Hovered)
- RGB: (64, 64, 64)
- Hex: #404040
- Linear: (0.25, 0.25, 0.25, 1.0)
- Usage: Button hovered state

### Text Colors

#### White (Primary Text)
- RGB: (255, 255, 255)
- Hex: #FFFFFF
- Linear: (1.0, 1.0, 1.0, 1.0)
- Usage: Primary text, titles, button labels

#### Light Gray (Secondary Text)
- RGB: (179, 179, 179)
- Hex: #B3B3B3
- Linear: (0.7, 0.7, 0.7, 1.0)
- Usage: Descriptions, subtitles, metadata

#### Light Cyan (Hint Text)
- RGB: (153, 230, 255)
- Hex: #99E6FF
- Linear: (0.6, 0.9, 1.0, 0.8)
- Usage: Hints, help text, optional information

---

## Widget Blueprints (Sub-Widgets)

### WBP_ArchiveListItem
**Purpose:** Individual list item for archive menu  
**Location:** `Content/UI/Widgets/WBP_ArchiveListItem`

**Required Elements:**
- Border container (400x80)
- Icon image (32x32)
- Title text block
- Subtitle text block
- Checkmark image (24x24)

**Functions:**
- SetItemData(Metadata)
- SetSelected(bool)

**Event Dispatchers:**
- OnItemClicked(EntryId)

### WBP_ConfirmationDialog
**Purpose:** Confirmation dialog for destructive actions  
**Location:** `Content/UI/Widgets/WBP_ConfirmationDialog`

**Required Elements:**
- Border container (400x200)
- Message text block
- Confirm button
- Cancel button

**Functions:**
- SetMessage(Text)
- Show()
- Hide()

**Event Dispatchers:**
- OnDialogResult(bool Confirmed)

### WBP_SettingSlider
**Purpose:** Reusable slider with label and value display  
**Location:** `Content/UI/Widgets/WBP_SettingSlider`

**Required Elements:**
- Label text block
- Slider control
- Value text block

**Functions:**
- SetLabel(Text)
- SetValue(float)
- SetRange(Min, Max, Step)

**Event Dispatchers:**
- OnValueChanged(float Value)

### WBP_SettingCheckbox
**Purpose:** Reusable checkbox with label  
**Location:** `Content/UI/Widgets/WBP_SettingCheckbox`

**Required Elements:**
- Label text block
- Checkbox control

**Functions:**
- SetLabel(Text)
- SetChecked(bool)

**Event Dispatchers:**
- OnCheckStateChanged(bool IsChecked)

### WBP_SettingComboBox
**Purpose:** Reusable combo box with label  
**Location:** `Content/UI/Widgets/WBP_SettingComboBox`

**Required Elements:**
- Label text block
- Combo box control

**Functions:**
- SetLabel(Text)
- SetOptions(Array<String>)
- SetSelectedOption(String)

**Event Dispatchers:**
- OnSelectionChanged(String SelectedOption)

---

## Asset Creation Guidelines

### Texture Guidelines
1. Use PNG format with alpha channel
2. Power-of-two dimensions when possible (256, 512, 1024)
3. Compress using BC7 for UI textures
4. Use BC4 for grayscale textures
5. Enable mipmaps for textures > 256x256
6. Disable mipmaps for small icons

### Material Guidelines
1. Set Material Domain to "User Interface"
2. Set Blend Mode to "Translucent" or "Masked"
3. Keep shader instructions < 100
4. Minimize texture samples (< 5)
5. Use scalar parameters for runtime control
6. Test on low-end hardware

### Sound Guidelines
1. Use WAV format for UI sounds
2. 16-bit, 44.1kHz sample rate
3. Mono for most UI sounds
4. Keep duration < 1 second for feedback sounds
5. Normalize to -6dB to -12dB
6. Add slight fade-in/out to prevent clicks
7. Use Sound Cue for variations

### Font Guidelines
1. Import at multiple sizes (12-48)
2. Enable anti-aliasing
3. Test readability at 1080p and 4K
4. Ensure ASCII character support
5. Consider localization character sets

---

## Asset Import Checklist

### Textures
- [ ] Correct dimensions (power-of-two if needed)
- [ ] Alpha channel preserved
- [ ] Compression settings correct (BC7 for UI)
- [ ] sRGB enabled for color textures
- [ ] Mipmaps configured correctly
- [ ] Texture group set to "UI"

### Materials
- [ ] Material domain set to "User Interface"
- [ ] Blend mode correct
- [ ] Parameters exposed
- [ ] Default values set
- [ ] Shader complexity acceptable
- [ ] Mobile preview checked

### Sounds
- [ ] Format correct (WAV, 16-bit, 44.1kHz)
- [ ] Volume normalized
- [ ] No clipping
- [ ] Fade-in/out applied
- [ ] Sound class assigned
- [ ] Attenuation settings (if needed)

### Fonts
- [ ] Font imported successfully
- [ ] Sizes cached correctly
- [ ] Hinting settings appropriate
- [ ] Characters render correctly
- [ ] Performance acceptable

---

## Performance Budget

### Per Widget
- Draw calls: < 10
- Texture memory: < 5 MB
- Material instructions: < 100 per material
- Update time: < 0.1 ms per frame

### Total UI
- Draw calls: < 50
- Texture memory: < 20 MB
- Total update time: < 1 ms per frame
- Memory allocations: Minimal (use object pooling)

---

## Asset Optimization Tips

1. **Texture Atlasing:** Combine small icons into texture atlases
2. **Material Instances:** Use material instances instead of unique materials
3. **Sound Cues:** Use sound cues for variations instead of multiple files
4. **Widget Pooling:** Reuse widgets instead of creating/destroying
5. **Lazy Loading:** Load heavy assets (preview images) on demand
6. **Compression:** Use appropriate compression for each asset type
7. **LOD:** Consider lower quality assets for low-end hardware

---

## Missing Asset Fallbacks

If assets are missing, use these fallbacks:

### Fonts
- Fallback to engine default font (Roboto)
- Adjust sizes to maintain readability

### Textures
- Use solid colors for icons
- Use procedural noise for grain effects
- Use simple shapes for UI elements

### Materials
- Use simple color materials
- Disable effects if materials missing

### Sounds
- UI can function without sounds
- Use silent sound cues as placeholders
