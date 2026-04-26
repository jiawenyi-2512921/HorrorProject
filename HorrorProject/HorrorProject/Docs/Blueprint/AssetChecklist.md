# Blueprint Asset Checklist

## Overview

This checklist covers all assets that need to be created in the Unreal Engine 5 editor to support the C++ systems. Check off items as you create them.

## VFX System Assets

### Post-Process Materials
- [ ] M_VHSEffect - VHS tape distortion effect
- [ ] M_UnderwaterEffect - Underwater color grading
- [ ] M_PressureVignette - Depth pressure vignette
- [ ] M_LensDroplets - Water droplets on lens
- [ ] M_ChromaticAberration - Color separation effect
- [ ] M_Scanlines - CRT scanline effect
- [ ] M_NoiseOverlay - Film grain noise
- [ ] M_DeathEffect - Death screen effect
- [ ] M_FearEffect - Fear/panic effect

### Niagara Particle Systems
- [ ] NS_UnderwaterDebris - Floating debris particles
- [ ] NS_DustParticles - Dust motes in air
- [ ] NS_BloodSplatter - Blood impact effects
- [ ] NS_WaterDrips - Dripping water
- [ ] NS_SteamVents - Steam from vents
- [ ] NS_EmergencyLights - Emergency light particles
- [ ] NS_BubbleTrail - Underwater bubbles
- [ ] NS_SparkShower - Electrical sparks

### Camera Shake Classes
- [ ] CS_Impact - Physical impact shake
- [ ] CS_Explosion - Explosion shake
- [ ] CS_Pressure - Pressure shake
- [ ] CS_Fear - Fear trembling shake
- [ ] CS_Drowning - Drowning shake
- [ ] CS_Death - Death shake

### VFX Blueprints
- [ ] BP_VFXManager - Central VFX management actor
- [ ] BP_CameraFlash - Camera flash effect actor
- [ ] BP_PostProcessVolume - Configured post-process volume

## UI System Assets

### Widget Blueprints
- [ ] WBP_BodycamOverlay - Main HUD with recording indicator
- [ ] WBP_EvidenceToast - Evidence collection notification
- [ ] WBP_ObjectiveToast - Objective update notification
- [ ] WBP_ArchiveMenu - Evidence archive viewer
- [ ] WBP_PauseMenu - Game pause menu
- [ ] WBP_SettingsMenu - Settings interface
- [ ] WBP_EvidenceListItem - Evidence list item widget
- [ ] WBP_EvidenceDetailViewer - Evidence detail viewer
- [ ] WBP_DocumentViewer - Document reading interface
- [ ] WBP_RecordingPlayer - Audio recording player
- [ ] WBP_PhotoGallery - Photo gallery viewer
- [ ] WBP_PhotoThumbnail - Photo thumbnail widget
- [ ] WBP_BatteryIndicator - Battery level indicator
- [ ] WBP_InteractionPrompt - Interaction prompt widget
- [ ] WBP_ExamineUI - Examine mode UI
- [ ] WBP_ObjectiveList - Active objectives list
- [ ] WBP_ObjectiveListItem - Objective list item
- [ ] WBP_WaypointMarker - Navigation waypoint marker

### UI Materials
- [ ] M_UI_Scanlines - UI scanline overlay
- [ ] M_UI_Noise - UI noise overlay
- [ ] M_UI_Vignette - UI vignette effect
- [ ] M_UI_Glitch - UI glitch effect

### UI Textures
- [ ] T_UI_RecordingDot - Recording indicator icon
- [ ] T_UI_BatteryIcon - Battery icon
- [ ] T_UI_EvidenceIcon - Evidence category icons (6 types)
- [ ] T_UI_ObjectiveIcon - Objective status icons
- [ ] T_UI_InteractionIcon - Interaction prompt icons
- [ ] T_UI_ButtonPrompts - Input button prompts (keyboard + gamepad)

### UI Fonts
- [ ] Font_VCR_OSD_Mono - Primary monospace font
- [ ] Font_Roboto_Mono - Secondary monospace font

### UI Blueprints
- [ ] BP_UIManager - Central UI management actor

## Audio System Assets

### Sound Cues - Ambient
- [ ] SC_Ambient_Station_Base - Base station ambience
- [ ] SC_Ambient_Station_Machinery - Machinery sounds
- [ ] SC_Ambient_Station_Water - Water sounds
- [ ] SC_Ambient_Underwater - Underwater ambience
- [ ] SC_Ambient_Tension - Tension music/drones

### Sound Cues - Footsteps
- [ ] SC_Footstep_Concrete - Concrete footsteps (5-8 variations)
- [ ] SC_Footstep_Metal - Metal footsteps (5-8 variations)
- [ ] SC_Footstep_Water - Water footsteps (5-8 variations)
- [ ] SC_Footstep_Grate - Grate footsteps (5-8 variations)
- [ ] SC_Footstep_Carpet - Carpet footsteps (5-8 variations)

### Sound Cues - Breathing
- [ ] SC_Breathing_Normal - Calm breathing
- [ ] SC_Breathing_Heavy - Heavy breathing
- [ ] SC_Breathing_Panic - Panicked breathing
- [ ] SC_Breathing_LowOxygen - Low oxygen gasping
- [ ] SC_Breathing_HoldBreath - Hold breath sound

### Sound Cues - Underwater
- [ ] SC_Underwater_Ambience - Underwater ambience
- [ ] SC_Underwater_Bubbles - Bubble sounds (variations)
- [ ] SC_Underwater_Movement - Swimming sounds
- [ ] SC_Underwater_Enter - Water entry splash
- [ ] SC_Underwater_Exit - Water exit splash

### Sound Cues - Camera
- [ ] SC_Camera_Shutter - Camera shutter sound
- [ ] SC_Camera_FlashCharge - Flash charge sound
- [ ] SC_Camera_RecordStart - Recording start beep
- [ ] SC_Camera_RecordStop - Recording stop beep

### Sound Cues - Interactions
- [ ] SC_Interaction_Generic - Generic interaction sound
- [ ] SC_Door_Open - Door opening sound
- [ ] SC_Door_Close - Door closing sound
- [ ] SC_Door_Locked - Locked door sound
- [ ] SC_Switch_On - Switch on sound
- [ ] SC_Switch_Off - Switch off sound
- [ ] SC_Pickup_Item - Item pickup sound
- [ ] SC_Collect_Evidence - Evidence collection sound

### Sound Attenuation
- [ ] ATT_Ambient_Large - Large radius attenuation (5000 units)
- [ ] ATT_Ambient_Medium - Medium radius attenuation (3000 units)
- [ ] ATT_Footsteps - Footstep attenuation (1500 units)
- [ ] ATT_Breathing - Breathing attenuation (500 units)
- [ ] ATT_Underwater - Underwater attenuation (8000 units)

### Sound Mix Modifiers
- [ ] Mix_Underwater - Underwater audio mix
- [ ] Mix_Tension - Tension/combat audio mix
- [ ] Mix_Pause - Pause menu audio mix

### Audio Blueprints
- [ ] BP_AudioManager - Central audio management actor
- [ ] BP_AudioZone_Station - Station audio zone
- [ ] BP_AudioZone_Underwater - Underwater audio zone

## Evidence System Assets

### Evidence Actor Blueprints
- [ ] BP_EvidenceActor_Base - Base evidence actor
- [ ] BP_EvidenceActor_Document - Document evidence
- [ ] BP_EvidenceActor_Photo - Photo evidence
- [ ] BP_EvidenceActor_Audio - Audio recording evidence
- [ ] BP_EvidenceActor_Physical - Physical object evidence

### Evidence Meshes
- [ ] SM_Evidence_Paper - Paper/document mesh
- [ ] SM_Evidence_Photo - Photo/polaroid mesh
- [ ] SM_Evidence_Recorder - Tape recorder mesh
- [ ] SM_Evidence_Folder - Folder mesh
- [ ] SM_Evidence_USB - USB drive mesh

### Evidence Textures
- [ ] T_Evidence_Document_[001-020] - Document textures (20 documents)
- [ ] T_Evidence_Photo_[001-010] - Photo textures (10 photos)
- [ ] T_Evidence_Icon_Document - Document icon
- [ ] T_Evidence_Icon_Photo - Photo icon
- [ ] T_Evidence_Icon_Audio - Audio icon
- [ ] T_Evidence_Icon_Physical - Physical object icon

### Evidence Audio
- [ ] SW_Evidence_Recording_[001-005] - Audio recordings (5 recordings)

## Camera System Assets

### Camera Blueprints
- [ ] BP_BatteryChargingStation - Battery charging station

### Camera Meshes
- [ ] SM_ChargingStation - Charging station mesh

### Camera Materials
- [ ] M_ChargingStation_Status - Status light material

## Interaction System Assets

### Interactable Blueprints
- [ ] BP_BaseInteractable - Base interactable
- [ ] BP_DocumentInteractable - Document interactable
- [ ] BP_DoorInteractable - Door interactable
- [ ] BP_Door_Standard - Standard hinged door
- [ ] BP_Door_Sliding - Sliding door
- [ ] BP_Door_Hatch - Floor/ceiling hatch
- [ ] BP_Door_Airlock - Airlock double door
- [ ] BP_ExaminableInteractable - Examinable object
- [ ] BP_PickupInteractable - Pickup item
- [ ] BP_Pickup_Battery - Battery pickup
- [ ] BP_Pickup_Key - Key pickup
- [ ] BP_Pickup_Flashlight - Flashlight pickup
- [ ] BP_Pickup_MedKit - Med kit pickup
- [ ] BP_RecorderInteractable - Audio recorder
- [ ] BP_SwitchInteractable - Switch/button

### Interactable Meshes
- [ ] SM_Door_Frame - Door frame mesh
- [ ] SM_Door_Panel - Door panel mesh
- [ ] SM_Hatch - Hatch mesh
- [ ] SM_Switch_Base - Switch base mesh
- [ ] SM_Switch_Lever - Switch lever mesh
- [ ] SM_Recorder - Recorder mesh
- [ ] SM_Battery - Battery mesh
- [ ] SM_Key - Key mesh
- [ ] SM_Flashlight - Flashlight mesh

### Interactable Materials
- [ ] M_Door_Metal - Metal door material
- [ ] M_Switch_Status - Switch status light material
- [ ] M_Highlight - Interactable highlight material

## Objective System Assets

### Objective Blueprints
- [ ] BP_ObjectiveNode_Base - Base objective node
- [ ] BP_ObjectiveNode_Navigation - Navigation objective
- [ ] BP_ObjectiveNode_ReachLocation - Reach location objective
- [ ] BP_ObjectiveNode_FollowPath - Follow path objective
- [ ] BP_ObjectiveNode_Escape - Escape objective
- [ ] BP_ObjectiveNode_Explore - Explore area objective
- [ ] BP_ObjectiveNode_Interaction - Interaction objective
- [ ] BP_ObjectiveNode_OpenDoor - Open door objective
- [ ] BP_ObjectiveNode_CollectItem - Collect item objective
- [ ] BP_ObjectiveNode_ReadDocument - Read document objective
- [ ] BP_ObjectiveNode_ActivateSwitch - Activate switch objective
- [ ] BP_ObjectiveNode_Encounter - Encounter objective
- [ ] BP_ObjectiveNode_SurviveTime - Survive time objective
- [ ] BP_ObjectiveNode_DefeatThreat - Defeat threat objective
- [ ] BP_ObjectiveNode_EscapeArea - Escape area objective
- [ ] BP_ObjectiveNode_HideFromThreat - Hide from threat objective
- [ ] BP_ObjectiveManager - Objective manager actor
- [ ] BP_WaypointMarker - Waypoint marker actor
- [ ] BP_ObjectiveDebugger - Objective debug tool

### Objective Textures
- [ ] T_Objective_Icon_Navigation - Navigation icon
- [ ] T_Objective_Icon_Interaction - Interaction icon
- [ ] T_Objective_Icon_Encounter - Encounter icon
- [ ] T_Objective_Icon_Complete - Complete checkmark
- [ ] T_Objective_Icon_Failed - Failed X mark
- [ ] T_Waypoint_Marker - Waypoint marker icon

## Character/Player Assets

### Player Blueprint
- [ ] BP_PlayerCharacter - Main player character (inherits from HorrorProjectCharacter)

### Player Components (added to BP_PlayerCharacter)
- [ ] CameraPhotoComponent
- [ ] CameraRecordingComponent
- [ ] CameraBatteryComponent
- [ ] EvidenceCollectionComponent
- [ ] FootstepAudioComponent
- [ ] BreathingAudioComponent
- [ ] InteractionManager component

## Level Assets

### Test Maps
- [ ] TestMap_VFX - VFX system test map
- [ ] TestMap_UI - UI system test map
- [ ] TestMap_Audio - Audio system test map
- [ ] TestMap_Evidence - Evidence system test map
- [ ] TestMap_Camera - Camera system test map
- [ ] TestMap_Interaction - Interaction system test map
- [ ] TestMap_Objective - Objective system test map

### Main Maps
- [ ] MainMenu - Main menu level
- [ ] DeepWaterStation - Main game level

## Material Assets

### Master Materials
- [ ] M_Master_Opaque - Master opaque material
- [ ] M_Master_Transparent - Master transparent material
- [ ] M_Master_PostProcess - Master post-process material

### Material Instances
- [ ] MI_Metal_Rusty - Rusty metal material
- [ ] MI_Concrete_Wet - Wet concrete material
- [ ] MI_Glass_Dirty - Dirty glass material
- [ ] MI_Water_Murky - Murky water material

## Texture Assets

### Environment Textures
- [ ] T_Metal_Albedo - Metal base color
- [ ] T_Metal_Normal - Metal normal map
- [ ] T_Metal_Roughness - Metal roughness
- [ ] T_Concrete_Albedo - Concrete base color
- [ ] T_Concrete_Normal - Concrete normal map
- [ ] T_Water_Normal - Water normal map

### Effect Textures
- [ ] T_Noise_Grain - Film grain noise texture
- [ ] T_Scanlines - Scanline texture
- [ ] T_Droplets - Water droplet texture
- [ ] T_VHS_Distortion - VHS distortion texture

## Physical Materials

### Surface Types
- [ ] PM_Concrete - Concrete physical material
- [ ] PM_Metal - Metal physical material
- [ ] PM_Water - Water physical material
- [ ] PM_Grate - Grate physical material
- [ ] PM_Carpet - Carpet physical material

## Input Assets

### Input Mapping Context
- [ ] IMC_Player - Player input mapping context

### Input Actions
- [ ] IA_Move - Movement input action
- [ ] IA_Look - Look input action
- [ ] IA_Interact - Interact input action
- [ ] IA_TakePhoto - Take photo input action
- [ ] IA_ToggleRecording - Toggle recording input action
- [ ] IA_ToggleFlash - Toggle flash input action
- [ ] IA_ViewPhotos - View photos input action
- [ ] IA_Pause - Pause game input action
- [ ] IA_Archive - Open archive input action

## Data Assets

### Data Tables
- [ ] DT_EvidenceMetadata - Evidence metadata table
- [ ] DT_ObjectiveData - Objective data table
- [ ] DT_InteractableData - Interactable data table

## Editor Utility Assets

### Editor Utilities
- [ ] EUW_EvidencePlacementTool - Evidence placement tool
- [ ] EUW_ObjectivePlacementTool - Objective placement tool

## Summary

**Total Assets to Create**: ~200+

**By Category**:
- VFX: 20 assets
- UI: 30 assets
- Audio: 40 assets
- Evidence: 40 assets
- Camera: 5 assets
- Interaction: 25 assets
- Objective: 25 assets
- Character: 10 assets
- Level: 10 assets
- Materials: 15 assets
- Textures: 20 assets
- Input: 10 assets
- Data: 5 assets
- Editor: 2 assets

## Progress Tracking

**Overall Progress**: 0 / 200+ assets created

**By System**:
- [ ] VFX System: 0 / 20
- [ ] UI System: 0 / 30
- [ ] Audio System: 0 / 40
- [ ] Evidence System: 0 / 40
- [ ] Camera System: 0 / 5
- [ ] Interaction System: 0 / 25
- [ ] Objective System: 0 / 25
- [ ] Character/Player: 0 / 10
- [ ] Level Assets: 0 / 10
- [ ] Materials: 0 / 15
- [ ] Textures: 0 / 20
- [ ] Input: 0 / 10
- [ ] Data Assets: 0 / 5
- [ ] Editor Utilities: 0 / 2

## Notes

- Assets marked with [001-020] indicate multiple variations needed
- All textures should be compressed appropriately (BC1/BC3/BC5)
- All audio should be in OGG Vorbis format
- All materials should use material instances where possible
- Test maps should be created before main level implementation
- Editor utilities are optional but highly recommended for workflow
