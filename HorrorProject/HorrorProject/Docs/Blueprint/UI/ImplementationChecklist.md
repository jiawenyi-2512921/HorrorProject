# UI Blueprint Implementation Checklist

## Overview
Complete checklist for implementing all UI Widget Blueprints in the HorrorProject.

---

## Pre-Implementation Setup

### Asset Preparation
- [ ] Import F_HomeVideo font (VHS-style monospace)
- [ ] Create icon textures (32x32): Evidence, Note, Photo, Audio
- [ ] Create UI element textures: CheckMark, CloseButton, RecDot
- [ ] Create noise textures: FilmGrain, NoisePattern
- [ ] Import all UI sound effects (see AssetRequirements.md)
- [ ] Verify all assets in correct folders

### Material Creation
- [ ] Create M_VHS_Overlay material (User Interface domain)
- [ ] Create M_Scanlines material (User Interface domain)
- [ ] Create M_FilmGrain material (User Interface domain)
- [ ] Create M_Vignette material (User Interface domain)
- [ ] Create M_BackgroundBlur material (User Interface domain)
- [ ] Create M_ToastBackground material (User Interface domain)
- [ ] Test all materials in UI preview
- [ ] Verify shader complexity < 100 instructions

### C++ Verification
- [ ] Verify all Widget C++ classes compile
- [ ] Verify UIManagerSubsystem exists
- [ ] Verify UIEventManager exists
- [ ] Verify UIAnimationLibrary exists
- [ ] Verify Evidence system integration
- [ ] Run UI unit tests (if available)

---

## WBP_BodycamOverlay Implementation

### Widget Creation
- [ ] Create WBP_BodycamOverlay blueprint
- [ ] Set parent class to UBodycamOverlayWidget
- [ ] Configure canvas panel root

### Layout Construction
- [ ] Add Image_VHSOverlay (full screen, M_VHS_Overlay material)
- [ ] Add Image_Scanlines (full screen, M_Scanlines material)
- [ ] Add Image_NoiseOverlay (full screen, M_FilmGrain material)
- [ ] Add Overlay_TopBar (top stretch anchor)
- [ ] Add HorizontalBox_RecordingInfo (left side)
  - [ ] Add Image_RecDot (16x16, red, initially collapsed)
  - [ ] Add TextBlock_RecLabel ("REC", red, initially collapsed)
  - [ ] Add TextBlock_Timestamp ("00:00:00", white)
- [ ] Add HorizontalBox_BatteryInfo (right side)
  - [ ] Add ProgressBar_Battery (120x20, green fill)
  - [ ] Add TextBlock_BatteryPercent ("100%", white)
- [ ] Add Image_VignetteEdge (full screen, M_Vignette material)

### Animation Setup
- [ ] Create RecordingBlink animation (1.0s, loop)
- [ ] Create GlitchEffect animation (0.5s, no loop)
- [ ] Create LowBatteryPulse animation (1.0s, loop)
- [ ] Test all animations in preview

### Event Graph Implementation
- [ ] Implement Event Construct (EventBus subscription)
- [ ] Implement BP_RecordingStateChanged
- [ ] Implement BP_BatteryLevelChanged
- [ ] Implement BP_TimestampUpdated
- [ ] Implement BP_VHSIntensityChanged
- [ ] Implement BP_GlitchEffectTriggered
- [ ] Implement Event Tick (for scanline animation, optional)

### Material Parameter Setup
- [ ] Cache dynamic material instances in Event Construct
- [ ] Set up VHS intensity parameter updates
- [ ] Set up scanline time offset updates
- [ ] Set up noise time offset updates

### Testing
- [ ] Test at 1920x1080 resolution
- [ ] Test at 2560x1440 resolution
- [ ] Test at 3840x2160 resolution
- [ ] Test REC indicator blink
- [ ] Test battery level updates (100%, 50%, 20%, 5%)
- [ ] Test battery color changes (green → yellow → red)
- [ ] Test timestamp updates
- [ ] Test VHS glitch effect
- [ ] Test performance (< 0.1ms per frame)
- [ ] Test with different VHS intensity values

---

## WBP_EvidenceToast Implementation

### Widget Creation
- [ ] Create WBP_EvidenceToast blueprint
- [ ] Set parent class to UEvidenceToastWidget
- [ ] Configure canvas panel root (initially collapsed)

### Layout Construction
- [ ] Add Border_ToastContainer (450x140, bottom-left anchor)
  - [ ] Set brush color: Black (0, 0, 0, 0.85)
  - [ ] Set outline: 2px, Yellow (1, 0.8, 0, 0.8)
  - [ ] Set rounded corners
- [ ] Add Image_Background (optional, M_FilmGrain)
- [ ] Add VerticalBox_Content
- [ ] Add HorizontalBox_Header
  - [ ] Add Image_Icon (24x24, yellow)
  - [ ] Add TextBlock_Header ("EVIDENCE COLLECTED", yellow)
- [ ] Add TextBlock_EvidenceName (size 20, white, bold)
- [ ] Add TextBlock_Description (size 14, light gray)

### Animation Setup
- [ ] Create FadeIn animation (0.3s, slide up + fade)
- [ ] Create FadeOut animation (0.5s, slide down + fade)
- [ ] Create IconPulse animation (1.0s, loop)
- [ ] Test all animations

### Event Graph Implementation
- [ ] Implement Event Construct
- [ ] Implement BP_ShowToast
- [ ] Implement BP_DismissToast
- [ ] Implement BP_PlayFadeIn
- [ ] Implement BP_PlayFadeOut
- [ ] Implement auto-dismiss timer (DisplayDuration)
- [ ] Implement sound effects (SFX_EvidenceCollected)

### Advanced Features (Optional)
- [ ] Implement typewriter effect for description
- [ ] Implement toast queue system
- [ ] Implement SetToastStyle (different colors per type)

### Testing
- [ ] Test ShowEvidenceToast with full metadata
- [ ] Test ShowSimpleToast with minimal data
- [ ] Test DismissToast immediate hide
- [ ] Test auto-dismiss after DisplayDuration
- [ ] Test multiple toasts in quick succession
- [ ] Test long evidence names (wrapping)
- [ ] Test long descriptions (wrapping)
- [ ] Test animation smoothness
- [ ] Test sound playback

---

## WBP_ObjectiveToast Implementation

### Widget Creation
- [ ] Create WBP_ObjectiveToast blueprint
- [ ] Set parent class to UObjectiveToastWidget
- [ ] Configure canvas panel root (initially collapsed)

### Layout Construction
- [ ] Add Border_ToastContainer (500x150, top-left anchor)
  - [ ] Set brush color: Dark Blue (0.05, 0.1, 0.2, 0.9)
  - [ ] Set outline: 2px, Cyan (0, 0.8, 1, 0.9)
  - [ ] Set initial position: X=-550 (off-screen)
- [ ] Add Image_Background (M_ScanlineSubtle)
- [ ] Add VerticalBox_Content
- [ ] Add HorizontalBox_Header
  - [ ] Add TextBlock_Arrow (">>", cyan)
  - [ ] Add TextBlock_Header ("NEW OBJECTIVE", cyan)
- [ ] Add TextBlock_ObjectiveText (size 22, white, bold)
- [ ] Add TextBlock_HintText (size 14, light cyan, initially collapsed)

### Animation Setup
- [ ] Create SlideIn animation (0.4s, slide from left with bounce)
- [ ] Create SlideOut animation (0.3s, slide to left)
- [ ] Create ArrowBlink animation (0.8s, loop)
- [ ] Create HeaderPulse animation (2.0s, loop)
- [ ] Test all animations

### Event Graph Implementation
- [ ] Implement Event Construct (EventBus subscription)
- [ ] Implement BP_ShowToast
- [ ] Implement BP_DismissToast
- [ ] Implement BP_PlaySlideIn
- [ ] Implement BP_PlaySlideOut
- [ ] Implement BP_PlayTypewriterEffect
- [ ] Implement auto-dismiss timer
- [ ] Implement sound effects

### Advanced Features
- [ ] Implement typewriter effect for objective text
- [ ] Implement typewriter effect for hint text (delayed)
- [ ] Implement SetObjectiveStyle (different colors per type)
- [ ] Implement objective queue system
- [ ] Implement progress indicator (optional)

### Testing
- [ ] Test ShowObjectiveToast (objective only)
- [ ] Test ShowObjectiveWithHint (objective + hint)
- [ ] Test DismissToast
- [ ] Test slide-in animation with bounce
- [ ] Test slide-out animation
- [ ] Test arrow blink animation
- [ ] Test typewriter effect speed
- [ ] Test hint text visibility toggle
- [ ] Test multiple objectives queuing
- [ ] Test different objective types (Main, Optional, Hidden, Failed)

---

## WBP_ArchiveMenu Implementation

### Widget Creation
- [ ] Create WBP_ArchiveMenu blueprint
- [ ] Set parent class to UArchiveMenuWidget
- [ ] Configure canvas panel root (initially collapsed)

### Layout Construction
- [ ] Add Border_ArchiveContainer (full screen)
  - [ ] Set brush color: Black (0, 0, 0, 0.95)
- [ ] Add Image_Background (M_FilmGrain)
- [ ] Add VerticalBox_Main
- [ ] Add Border_Header (height 80)
  - [ ] Add Button_Close (40x40, "X")
  - [ ] Add TextBlock_Title ("ARCHIVE", size 32, cyan)
  - [ ] Add filter buttons (All, Evidence, Notes, Photos)
- [ ] Add HorizontalBox_Content
- [ ] Add Border_ListPanel (width 400)
  - [ ] Add ScrollBox_EntryList
- [ ] Add Border_DetailPanel (fill)
  - [ ] Add ScrollBox_DetailView
  - [ ] Add Image_EvidencePreview (400 height, initially collapsed)
  - [ ] Add TextBlock_EntryTitle (size 28, white)
  - [ ] Add TextBlock_EntryType (size 16, cyan)
  - [ ] Add TextBlock_Location (size 16, light gray)
  - [ ] Add TextBlock_Description (size 18, white)
- [ ] Add Image_ScanlineOverlay (full screen, M_Scanlines)

### Sub-Widget Creation
- [ ] Create WBP_ArchiveListItem blueprint
  - [ ] Add Border_ItemContainer (400x80)
  - [ ] Add Image_Icon (32x32)
  - [ ] Add TextBlock_Title
  - [ ] Add TextBlock_Subtitle
  - [ ] Add Image_CheckMark (24x24)
  - [ ] Implement SetItemData function
  - [ ] Implement OnItemClicked event dispatcher

### Animation Setup
- [ ] Create OpenArchive animation (0.5s, fade + scale)
- [ ] Create CloseArchive animation (0.3s, fade)
- [ ] Create FilterButtonHighlight animation (0.2s)
- [ ] Test all animations

### Event Graph Implementation
- [ ] Implement Event Construct
- [ ] Implement BP_ArchiveOpened
- [ ] Implement BP_ArchiveClosed
- [ ] Implement BP_ArchiveDataRefreshed
- [ ] Implement BP_EntrySelected
- [ ] Implement BP_FilterApplied
- [ ] Implement button click events (Close, Filter buttons)
- [ ] Implement OnListItemClicked custom event
- [ ] Implement input handling (ESC to close)

### Advanced Features (Optional)
- [ ] Implement search functionality
- [ ] Implement sort options (Date, Name, Type)
- [ ] Implement entry statistics display
- [ ] Implement object pooling for list items

### Testing
- [ ] Test OpenArchive
- [ ] Test CloseArchive
- [ ] Test RefreshArchiveData with 0 items
- [ ] Test RefreshArchiveData with 50+ items
- [ ] Test SelectEntry updates detail view
- [ ] Test FilterByType (All, Evidence, Notes, Photos)
- [ ] Test ClearFilter
- [ ] Test list scrolling with many items
- [ ] Test detail view scrolling with long text
- [ ] Test preview image display
- [ ] Test preview image hidden when not available
- [ ] Test mouse cursor visibility
- [ ] Test input mode switching
- [ ] Test ESC key closes menu
- [ ] Test performance with 100+ entries

---

## WBP_PauseMenu Implementation

### Widget Creation
- [ ] Create WBP_PauseMenu blueprint
- [ ] Set parent class to UPauseMenuWidget
- [ ] Configure canvas panel root (initially collapsed)

### Layout Construction
- [ ] Add Border_PauseContainer (full screen)
  - [ ] Set brush color: Black (0, 0, 0, 0.8)
- [ ] Add Image_BackgroundBlur (M_BackgroundBlur, optional)
- [ ] Add VerticalBox_MenuContent (centered, 400x400)
- [ ] Add TextBlock_Title ("PAUSED", size 48, cyan)
- [ ] Add Button_Resume (fill horizontal, 60 height)
  - [ ] Set text: "RESUME"
  - [ ] Set style: Dark gray normal, gray hovered, cyan outline
- [ ] Add Button_Settings (fill horizontal, 60 height)
  - [ ] Set text: "SETTINGS"
- [ ] Add Button_Quit (fill horizontal, 60 height)
  - [ ] Set text: "QUIT TO MENU"
  - [ ] Set style: Red outline
- [ ] Add Image_ScanlineOverlay (full screen, M_Scanlines)

### Sub-Widget Creation (Optional)
- [ ] Create WBP_ConfirmationDialog blueprint
  - [ ] Add Border_DialogContainer (400x200, centered)
  - [ ] Add TextBlock_Message
  - [ ] Add Button_Confirm
  - [ ] Add Button_Cancel
  - [ ] Implement SetMessage function
  - [ ] Implement OnDialogResult event dispatcher

### Animation Setup
- [ ] Create FadeIn animation (0.2s, fade + slide down)
- [ ] Create FadeOut animation (0.2s, fade)
- [ ] Create ButtonHoverPulse animation (0.3s, scale)
- [ ] Test all animations

### Event Graph Implementation
- [ ] Implement Event Construct
- [ ] Implement BP_PauseMenuOpened
- [ ] Implement BP_PauseMenuClosed
- [ ] Implement BP_SettingsOpened
- [ ] Implement OnResumeClicked
- [ ] Implement OnSettingsClicked
- [ ] Implement OnQuitClicked (with confirmation)
- [ ] Implement button hover events
- [ ] Implement input handling (ESC to resume)
- [ ] Implement OnSettingsClosed custom event

### Advanced Features (Optional)
- [ ] Implement game state display (chapter, playtime)
- [ ] Implement quick save/load buttons

### Testing
- [ ] Test OpenPauseMenu
- [ ] Test ClosePauseMenu
- [ ] Test ResumeGame
- [ ] Test OpenSettings
- [ ] Test QuitToMainMenu with confirmation
- [ ] Test game pauses when menu opens
- [ ] Test game resumes when menu closes
- [ ] Test mouse cursor visibility
- [ ] Test input mode switching
- [ ] Test ESC key opens/closes menu
- [ ] Test button hover effects
- [ ] Test button click sounds
- [ ] Test focus set to Resume on open
- [ ] Test keyboard navigation (Up/Down arrows)
- [ ] Test gamepad navigation

---

## WBP_SettingsMenu Implementation

### Widget Creation
- [ ] Create WBP_SettingsMenu blueprint
- [ ] Set parent class to USettingsMenuWidget
- [ ] Configure canvas panel root (initially collapsed)

### Layout Construction
- [ ] Add Border_SettingsContainer (full screen)
- [ ] Add Image_Background (M_FilmGrain)
- [ ] Add VerticalBox_Main
- [ ] Add Border_Header (height 80)
  - [ ] Add Button_Close
  - [ ] Add TextBlock_Title ("SETTINGS")
  - [ ] Add TextBlock_UnsavedIndicator (initially collapsed)
- [ ] Add HorizontalBox_Content
- [ ] Add Border_CategoryPanel (width 200)
  - [ ] Add category buttons (Graphics, Audio, Controls, Gameplay, Accessibility)
- [ ] Add Border_SettingsPanel (fill)
  - [ ] Add WidgetSwitcher_SettingsPages
- [ ] Add Border_Footer (height 80)
  - [ ] Add Button_Apply
  - [ ] Add Button_Reset
  - [ ] Add Button_Close

### Settings Pages Construction
- [ ] Create ScrollBox_GraphicsSettings (index 0)
  - [ ] Add Resolution combo box
  - [ ] Add Window Mode combo box
  - [ ] Add Quality combo box
  - [ ] Add VSync checkbox
  - [ ] Add Frame Limit combo box
  - [ ] Add VHS Intensity slider
  - [ ] Add Brightness slider
- [ ] Create ScrollBox_AudioSettings (index 1)
  - [ ] Add Master Volume slider
  - [ ] Add Music Volume slider
  - [ ] Add SFX Volume slider
  - [ ] Add Ambience Volume slider
  - [ ] Add Subtitles checkbox
  - [ ] Add Subtitle Size combo box
- [ ] Create ScrollBox_ControlsSettings (index 2)
  - [ ] Add Mouse Sensitivity slider
  - [ ] Add Invert Y checkbox
  - [ ] Add Invert X checkbox
  - [ ] Add Gamepad Sensitivity slider
  - [ ] Add key binding list
- [ ] Create ScrollBox_GameplaySettings (index 3)
  - [ ] Add Difficulty combo box
  - [ ] Add Auto Save checkbox
  - [ ] Add Auto Save Interval slider
  - [ ] Add Show Hints checkbox
  - [ ] Add Show Crosshair checkbox
- [ ] Create ScrollBox_AccessibilitySettings (index 4)
  - [ ] Add Colorblind Mode checkbox
  - [ ] Add Colorblind Type combo box
  - [ ] Add UI Scale slider
  - [ ] Add Reduce Motion checkbox
  - [ ] Add High Contrast checkbox
  - [ ] Add Text Size slider

### Sub-Widget Creation (Optional)
- [ ] Create WBP_SettingSlider (reusable slider with label)
- [ ] Create WBP_SettingCheckbox (reusable checkbox with label)
- [ ] Create WBP_SettingComboBox (reusable combo box with label)

### Animation Setup
- [ ] Create OpenSettings animation (0.3s)
- [ ] Create CloseSettings animation (0.2s)
- [ ] Create CategoryHighlight animation (0.2s)
- [ ] Test all animations

### Event Graph Implementation
- [ ] Implement Event Construct
- [ ] Implement BP_SettingsOpened
- [ ] Implement BP_SettingsClosed
- [ ] Implement BP_SettingsApplied
- [ ] Implement BP_CategorySelected
- [ ] Implement MarkAsUnsaved function
- [ ] Implement LoadCurrentSettings function
- [ ] Implement SaveAllSettings function
- [ ] Implement button click events
- [ ] Implement category button click events
- [ ] Bind all setting controls to MarkAsUnsaved

### Testing
- [ ] Test OpenSettings
- [ ] Test CloseSettings
- [ ] Test ApplySettings saves changes
- [ ] Test ResetToDefaults restores defaults
- [ ] Test SelectCategory switches pages
- [ ] Test all Graphics settings
- [ ] Test all Audio settings
- [ ] Test all Controls settings
- [ ] Test all Gameplay settings
- [ ] Test all Accessibility settings
- [ ] Test unsaved changes indicator
- [ ] Test Apply button enabled/disabled state
- [ ] Test confirmation dialog on close with unsaved changes
- [ ] Test settings persist after closing
- [ ] Test settings load correctly on open

---

## Integration & System Testing

### UIManagerSubsystem Integration
- [ ] Create all widgets in Game Mode BeginPlay
- [ ] Register widgets with UIManagerSubsystem
- [ ] Test CreateWidget function
- [ ] Test ShowWidget function
- [ ] Test HideWidget function
- [ ] Test RemoveWidget function
- [ ] Test GetWidget function

### EventBus Integration
- [ ] Test Bodycam overlay subscribes to camera events
- [ ] Test Evidence toast subscribes to evidence events
- [ ] Test Objective toast subscribes to objective events
- [ ] Test event dispatching from gameplay code
- [ ] Test event unsubscription on widget destroy

### Input Handling
- [ ] Bind ESC key to pause menu toggle
- [ ] Bind Tab key to archive menu (optional)
- [ ] Test keyboard navigation in all menus
- [ ] Test gamepad navigation in all menus
- [ ] Test mouse input in all menus
- [ ] Test input mode switching (Game ↔ UI)
- [ ] Test input priority (pause menu > archive menu > game)

### Cross-Widget Testing
- [ ] Test pause menu → settings menu → pause menu flow
- [ ] Test multiple toasts appearing simultaneously
- [ ] Test archive menu while bodycam overlay active
- [ ] Test pause menu while toasts visible
- [ ] Test all widgets at different resolutions
- [ ] Test all widgets with different UI scales

### Performance Testing
- [ ] Profile all widgets with Unreal Insights
- [ ] Verify total UI update time < 1ms per frame
- [ ] Verify total draw calls < 50
- [ ] Verify texture memory < 20 MB
- [ ] Test with 100+ archive entries
- [ ] Test with multiple toasts queued
- [ ] Test on low-end hardware

### Localization Testing (If Applicable)
- [ ] Test with different languages
- [ ] Test text wrapping with long translations
- [ ] Test UI layout with different text lengths
- [ ] Test font rendering with special characters

---

## Final Verification

### Visual Quality
- [ ] All text readable at 1080p
- [ ] All text readable at 4K
- [ ] VHS effects subtle and not overwhelming
- [ ] Colors match design specifications
- [ ] Animations smooth at 60fps
- [ ] No visual glitches or artifacts

### Functionality
- [ ] All buttons clickable and responsive
- [ ] All sliders smooth and accurate
- [ ] All checkboxes toggle correctly
- [ ] All combo boxes display options
- [ ] All animations play correctly
- [ ] All sounds play at correct volume

### Accessibility
- [ ] Keyboard navigation works in all menus
- [ ] Gamepad navigation works in all menus
- [ ] Focus indicators visible
- [ ] Text contrast sufficient (4.5:1 minimum)
- [ ] UI scales correctly (0.5x to 2x)
- [ ] Colorblind mode functional

### Performance
- [ ] No frame drops when opening menus
- [ ] No memory leaks from repeated open/close
- [ ] No excessive garbage collection
- [ ] Acceptable performance on minimum spec hardware

### Documentation
- [ ] All implementation guides reviewed
- [ ] All asset requirements documented
- [ ] All common issues documented
- [ ] FAQ updated with findings
- [ ] Code comments added where needed

---

## Post-Implementation

### Code Review
- [ ] Review all Blueprint graphs for optimization
- [ ] Review all event bindings for memory leaks
- [ ] Review all animations for performance
- [ ] Review all materials for shader complexity

### Bug Fixing
- [ ] Fix all critical bugs
- [ ] Fix all major bugs
- [ ] Document known minor bugs
- [ ] Create bug tracking tickets

### Optimization
- [ ] Implement object pooling where beneficial
- [ ] Optimize material shaders
- [ ] Reduce texture memory usage
- [ ] Minimize widget complexity

### Polish
- [ ] Fine-tune animation timings
- [ ] Adjust sound volumes
- [ ] Refine VHS effect intensities
- [ ] Improve visual feedback

### Handoff
- [ ] Create video walkthrough of implementation
- [ ] Document any deviations from guides
- [ ] Provide troubleshooting tips
- [ ] Train team members on usage

---

## Completion Criteria

All widgets implemented: ☐  
All tests passing: ☐  
Performance targets met: ☐  
Documentation complete: ☐  
Code review approved: ☐  
Ready for production: ☐

**Implementation Start Date:** ___________  
**Target Completion Date:** ___________  
**Actual Completion Date:** ___________  
**Implemented By:** ___________  
**Reviewed By:** ___________
