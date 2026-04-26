# UI Blueprint Implementation FAQ

## Overview
Frequently asked questions and solutions for implementing UI Widget Blueprints in the HorrorProject.

---

## General Questions

### Q: What is the difference between the C++ Widget classes and the Blueprint Widgets?

**A:** The C++ Widget classes (e.g., `UBodycamOverlayWidget`) provide the core functionality, data management, and event handling. The Blueprint Widgets (e.g., `WBP_BodycamOverlay`) inherit from these C++ classes and implement the visual layout, animations, and UI-specific logic. Think of C++ as the "brain" and Blueprint as the "body" of the widget.

**Example:**
- C++ handles: Data updates, event subscriptions, timer management
- Blueprint handles: Visual layout, animations, material parameters, user input

---

### Q: Do I need to create the C++ classes first?

**A:** Yes, the C++ Widget classes must exist and compile before you can create Blueprint Widgets that inherit from them. The C++ classes are already implemented in the HorrorProject.

---

### Q: Can I modify the C++ Widget classes?

**A:** You can, but it's not recommended unless you need to add new functionality. The existing C++ classes provide all necessary functions and events. Most customization should be done in Blueprint.

---

### Q: What if I want to add a new UI element not covered in the guides?

**A:** Follow the same pattern:
1. Create a C++ Widget class inheriting from `UUserWidget`
2. Add necessary functions and Blueprint Implementable Events
3. Create a Blueprint Widget inheriting from your C++ class
4. Implement the visual layout and event handlers in Blueprint

---

## Event Binding Questions

### Q: How do I bind to C++ events in Blueprint?

**A:** C++ events marked as `BlueprintImplementableEvent` automatically appear in the Blueprint Event Graph. Simply right-click in the Event Graph and search for the event name (e.g., "BP_RecordingStateChanged").

**Steps:**
1. Open Widget Blueprint
2. Go to Event Graph
3. Right-click → Add Event → [Event Name]
4. Implement the event logic

---

### Q: How do I subscribe to EventBus events?

**A:** Use the UIEventManager to subscribe to gameplay events.

**Blueprint Implementation:**
```
Event Construct
├── Get Game Instance
├── Get Subsystem (UIManagerSubsystem)
├── Get Event Manager
├── Subscribe to Event
│   ├── Event Tag: Make Gameplay Tag ("Evidence.Collected")
│   └── Callback: Create Custom Event "OnEvidenceCollected"
```

**Important:** Always unsubscribe in Event Destruct to prevent memory leaks.

---

### Q: My event subscriptions aren't working. What's wrong?

**A:** Common issues:
1. **Event tag mismatch:** Ensure the tag string exactly matches the C++ event tag
2. **Subscription timing:** Subscribe in Event Construct, not BeginPlay
3. **Event Manager null:** Verify UIManagerSubsystem is initialized
4. **Callback not bound:** Ensure the custom event is created and bound

**Debug Steps:**
1. Add Print String after Subscribe to Event
2. Verify Event Manager is valid (not null)
3. Check that the event is being dispatched from C++
4. Use breakpoints in the custom event callback

---

### Q: How do I unsubscribe from events?

**A:** Call Unsubscribe in Event Destruct.

**Blueprint Implementation:**
```
Event Destruct
├── Get Event Manager
└── Unsubscribe from Event
    └── Event Tag: [Same tag used in Subscribe]
```

---

## Text and Font Questions

### Q: How do I update text dynamically?

**A:** Use the Set Text node on Text Block widgets.

**Blueprint Implementation:**
```
Update Battery Text
├── Get Battery Percent (0.0 to 1.0)
├── Multiply by 100
├── Round
├── Format Text ("{0}%", Percent)
└── TextBlock_BatteryPercent → Set Text
```

**Important:** Mark the Text Block as "Is Variable" in the Designer to access it in Blueprint.

---

### Q: My text is not wrapping. How do I fix it?

**A:** Enable Auto Wrap Text on the Text Block.

**Steps:**
1. Select Text Block in Designer
2. Details panel → Appearance → Auto Wrap Text: True
3. Set Wrap Text At: 0 (wraps at widget width)
4. Ensure parent container has defined width

---

### Q: The font looks blurry. How do I fix it?

**A:** Ensure the font is cached at the correct sizes.

**Steps:**
1. Open F_HomeVideo font asset
2. Font Cache Type: Runtime
3. Sizes to Cache: Add the sizes you're using (e.g., 14, 16, 18, 20, 24)
4. Save and test

**Alternative:** Use larger font sizes and scale down the widget instead of using small font sizes directly.

---

### Q: How do I implement a typewriter effect?

**A:** Use a loop with delays to reveal characters one by one.

**Blueprint Implementation:**
```
PlayTypewriterEffect (FullText)
├── Convert Text to String
├── Get String Length
├── Set Text (Empty)
├── For Loop (0 to Length)
│   ├── Get Substring (0 to Index+1)
│   ├── Convert to Text
│   ├── Set Text (Substring)
│   └── Delay (TypewriterSpeed)
```

**Optimization:** Use a timer instead of delays in a loop for better performance.

---

## Material and VHS Effect Questions

### Q: How do I apply VHS effects to my widget?

**A:** Use Image widgets with VHS effect materials.

**Steps:**
1. Add Image widget (full screen)
2. Set Material to M_VHS_Overlay, M_Scanlines, or M_FilmGrain
3. Set Render Opacity to control intensity (0.1 to 0.5)
4. Set Hit Test Invisible to True (so it doesn't block input)

---

### Q: How do I update material parameters at runtime?

**A:** Get the dynamic material instance and set scalar parameters.

**Blueprint Implementation:**
```
Update VHS Intensity
├── Image_VHSOverlay → Get Dynamic Material Instance
├── Set Scalar Parameter Value
│   ├── Parameter Name: "Intensity"
│   └── Value: [New Intensity]
```

**Important:** Cache the dynamic material instance in Event Construct for better performance.

---

### Q: My VHS effects are too strong/weak. How do I adjust them?

**A:** Adjust the Render Opacity of the Image widget or the material parameters.

**Recommended Values:**
- VHS Overlay: Opacity 0.3, Intensity 0.3
- Scanlines: Opacity 0.2, Intensity 0.2
- Film Grain: Opacity 0.15, NoiseAmount 0.15
- Vignette: Opacity 0.4, Intensity 0.4

**Tip:** Create a settings slider to let players adjust VHS intensity.

---

### Q: The scanlines aren't animating. What's wrong?

**A:** You need to update the TimeOffset parameter in Event Tick.

**Blueprint Implementation:**
```
Event Tick (Delta Time)
├── Add (CurrentTime + Delta Time)
├── Image_Scanlines → Get Dynamic Material Instance
└── Set Scalar Parameter Value ("TimeOffset", CurrentTime)
```

**Alternative:** Use a Timeline for more control over the animation.

---

### Q: Can I use post-process effects instead of material overlays?

**A:** Yes, but material overlays are recommended for UI because:
1. They only affect the UI, not the game world
2. They're more performant for UI-specific effects
3. They're easier to control per-widget

Post-process effects are better for full-screen camera effects.

---

## Animation Questions

### Q: How do I create a fade-in animation?

**A:** Use the Animation tab to create a timeline animation.

**Steps:**
1. Animations tab → + Animation → Name: "FadeIn"
2. Set Length: 0.3 seconds
3. Add Track: [Widget] → Render Opacity
4. Add keyframe at 0.0s: Value 0.0
5. Add keyframe at 0.3s: Value 1.0
6. Right-click keyframe → Interpolation → Ease Out

---

### Q: How do I play an animation from Blueprint?

**A:** Use the Play Animation node.

**Blueprint Implementation:**
```
Show Widget
├── Set Visibility (Visible)
└── Play Animation
    ├── Animation: FadeIn
    ├── Start Time: 0.0
    ├── Num Loops: 1
    └── Play Mode: Forward
```

---

### Q: How do I loop an animation?

**A:** Set Num Loops to 0 (infinite) or use the Loop parameter.

**Blueprint Implementation:**
```
Play Animation
├── Animation: RecordingBlink
├── Num Loops: 0 (infinite loop)
└── Play Mode: Forward
```

**To stop:** Use Stop Animation node.

---

### Q: My animation is stuttering. How do I fix it?

**A:** Common causes and solutions:

1. **Wrong interpolation:** Use Ease In/Out curves instead of Linear
2. **Too many keyframes:** Simplify the animation
3. **Heavy tick logic:** Move logic out of Event Tick
4. **Material complexity:** Reduce shader instructions

**Debug:** Use Unreal Insights to profile animation performance.

---

### Q: How do I animate a slide-in from off-screen?

**A:** Animate the Slot → Position property, not Render Transform.

**Steps:**
1. Widget must be in a Canvas Panel
2. Animation track: [Widget] → Slot (Canvas Panel Slot) → Position X
3. Keyframe at 0.0s: -550 (off-screen left)
4. Keyframe at 0.4s: 0 (on-screen)
5. Use Ease Out curve

**Important:** Set initial position to -550 in Designer or Event Construct.

---

## Layout and Positioning Questions

### Q: How do I center a widget on screen?

**A:** Use anchors and alignment.

**Steps:**
1. Select widget in Designer
2. Anchors: Click center anchor preset (0.5, 0.5 to 0.5, 0.5)
3. Position: X=0, Y=0
4. Alignment: X=0.5, Y=0.5

**Result:** Widget is centered regardless of screen resolution.

---

### Q: My widget is not scaling correctly at different resolutions. What's wrong?

**A:** Use proper anchors.

**Anchor Guide:**
- **Full screen:** Anchors (0,0 to 1,1), Position (0,0), Size (0,0)
- **Top-left corner:** Anchors (0,0 to 0,0), Position (X, Y), Size (W, H)
- **Top stretch:** Anchors (0,0 to 1,0), Position (0, Y), Size (0, H)
- **Center:** Anchors (0.5,0.5 to 0.5,0.5), Position (0,0), Alignment (0.5,0.5)

**Test:** Use different resolutions in the Designer dropdown (1920x1080, 2560x1440, 3840x2160).

---

### Q: How do I make a widget fill its parent?

**A:** Set anchors to fill (0,0 to 1,1) and size to (0,0).

**Steps:**
1. Select widget
2. Anchors: Fill preset (0,0 to 1,1)
3. Position: X=0, Y=0
4. Size: X=0, Y=0

**Result:** Widget automatically fills parent container.

---

### Q: What's the difference between Position and Render Transform?

**A:** 
- **Position (Slot):** Actual layout position, affects other widgets
- **Render Transform:** Visual-only offset, doesn't affect layout

**Use Position for:** Permanent layout changes, slide-in animations  
**Use Render Transform for:** Temporary visual effects, shake animations, scale effects

---

## Input and Interaction Questions

### Q: How do I handle button clicks?

**A:** Bind to the On Clicked event.

**Steps:**
1. Select Button in Designer
2. Details panel → Events → On Clicked: +
3. Implement logic in Event Graph

**Blueprint Implementation:**
```
OnClicked (Button_Resume)
├── Play Sound 2D (SFX_ButtonClick)
└── Call ResumeGame
```

---

### Q: How do I detect mouse hover on a button?

**A:** Bind to On Hovered and On Unhovered events.

**Blueprint Implementation:**
```
OnHovered (Button_Resume)
├── Play Sound 2D (SFX_ButtonHover)
└── Play Animation (ButtonHoverPulse)

OnUnhovered (Button_Resume)
└── Stop Animation (ButtonHoverPulse)
```

---

### Q: How do I handle keyboard input in a widget?

**A:** Override OnKeyDown in the Widget Blueprint.

**Steps:**
1. Event Graph → Override → OnKeyDown
2. Switch on Key
3. Implement logic for each key

**Blueprint Implementation:**
```
OnKeyDown (Key)
├── Switch on Key
│   ├── Escape: Call CloseMenu
│   ├── Up Arrow: Focus Previous Button
│   └── Down Arrow: Focus Next Button
```

**Important:** Call Enable Input in Event Construct.

---

### Q: How do I show/hide the mouse cursor?

**A:** Use Set Show Mouse Cursor on Player Controller.

**Blueprint Implementation:**
```
Show Menu
├── Get Player Controller
└── Set Show Mouse Cursor (True)

Hide Menu
├── Get Player Controller
└── Set Show Mouse Cursor (False)
```

---

### Q: How do I set input mode to UI only?

**A:** Use Set Input Mode UI Only on Player Controller.

**Blueprint Implementation:**
```
Open Menu
├── Get Player Controller
├── Set Input Mode UI Only
│   └── Widget to Focus: Self
└── Set Show Mouse Cursor (True)

Close Menu
├── Get Player Controller
├── Set Input Mode Game Only
└── Set Show Mouse Cursor (False)
```

---

## Performance Questions

### Q: My UI is causing frame drops. How do I optimize it?

**A:** Common optimization techniques:

1. **Use Invalidation Boxes:** Wrap static content in Invalidation Boxes
2. **Disable Tick:** Set Update Mode to "Tick" only if needed
3. **Simplify Materials:** Reduce shader instructions, minimize texture samples
4. **Object Pooling:** Reuse widgets instead of creating/destroying
5. **Lazy Loading:** Load heavy assets (images) on demand
6. **Batch Updates:** Use Batch Update Begin/End for multiple changes

**Profile:** Use Unreal Insights → Widget Reflector to identify bottlenecks.

---

### Q: How do I use Invalidation Boxes?

**A:** Wrap static content in an Invalidation Box.

**Steps:**
1. Add Invalidation Box to hierarchy
2. Move static widgets inside it
3. Set Invalidation Mode: Manual or Auto

**When to use:** Headers, static text, backgrounds that don't change every frame.

**When NOT to use:** Animated widgets, frequently updating text.

---

### Q: Should I disable tick on my widgets?

**A:** Yes, if the widget doesn't need per-frame updates.

**Steps:**
1. Class Settings → Tick
2. Uncheck "Enable Tick"

**Enable tick only if:**
- Animating materials (updating TimeOffset)
- Updating text every frame (e.g., timestamp)
- Polling for state changes

**Alternative:** Use timers instead of tick for periodic updates.

---

### Q: How do I implement object pooling for widgets?

**A:** Create a pool of reusable widgets.

**Blueprint Implementation:**
```
Variables:
- WidgetPool (Array of UserWidget)
- PoolSize (int) = 20

GetOrCreateWidget
├── Branch (Pool has available widget)
│   ├── True:
│   │   ├── Get from Pool
│   │   └── Set Visibility (Visible)
│   └── False:
│       └── Create Widget
└── Return Widget

ReturnWidgetToPool (Widget)
├── Set Visibility (Collapsed)
├── Remove from Parent
└── Add to Pool
```

**Use for:** List items, toast notifications, repeated UI elements.

---

## Debugging Questions

### Q: My widget is not appearing. How do I debug it?

**A:** Check these common issues:

1. **Visibility:** Is it set to Visible?
2. **Z-Order:** Is it behind other widgets?
3. **Size:** Does it have non-zero size?
4. **Parent:** Is it added to viewport?
5. **Opacity:** Is Render Opacity > 0?

**Debug Steps:**
1. Add Print String after Set Visibility
2. Check Widget Reflector (Tools → Debug → Widget Reflector)
3. Use "Show Collision" console command to see widget bounds
4. Verify parent widget is visible

---

### Q: How do I use the Widget Reflector?

**A:** Widget Reflector shows all active widgets and their properties.

**Steps:**
1. Tools → Debug → Widget Reflector
2. Click "Pick Live Widget"
3. Click on your widget in the game
4. View hierarchy, properties, and performance stats

**Useful for:** Finding invisible widgets, checking Z-order, profiling performance.

---

### Q: My button is not clickable. What's wrong?

**A:** Common issues:

1. **Is Enabled:** Check button Is Enabled is True
2. **Hit Test Invisible:** Parent widgets blocking input
3. **Z-Order:** Another widget is on top
4. **Visibility:** Button is Collapsed or Hidden
5. **Size:** Button has zero size

**Debug:** Use Widget Reflector to check hit test visibility.

---

### Q: How do I add debug print statements?

**A:** Use Print String node.

**Blueprint Implementation:**
```
Debug Event
├── Print String
│   ├── In String: "Event Triggered"
│   ├── Text Color: Yellow
│   └── Duration: 5.0
```

**Tip:** Use different colors for different widgets (Red for errors, Yellow for warnings, Green for success).

---

## Integration Questions

### Q: How do I create widgets from C++?

**A:** Use UIManagerSubsystem.

**C++ Implementation:**
```cpp
UUIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UUIManagerSubsystem>();
UUserWidget* Widget = UIManager->CreateWidget(WidgetClass, "WidgetName");
UIManager->ShowWidget("WidgetName");
```

**Blueprint Implementation:**
```
Create Widget
├── Get Game Instance
├── Get Subsystem (UIManagerSubsystem)
├── Create Widget
│   ├── Class: WBP_BodycamOverlay
│   └── Name: "BodycamOverlay"
└── Show Widget ("BodycamOverlay")
```

---

### Q: How do I access widgets from other Blueprints?

**A:** Use UIManagerSubsystem to get widgets by name.

**Blueprint Implementation:**
```
Get Widget
├── Get Game Instance
├── Get Subsystem (UIManagerSubsystem)
├── Get Widget ("BodycamOverlay")
└── Cast to BodycamOverlayWidget
```

**Important:** Widgets must be registered with UIManagerSubsystem first.

---

### Q: How do I pass data between widgets?

**A:** Use function calls or event dispatchers.

**Method 1: Direct Function Call**
```
Widget A
├── Get Widget ("WidgetB")
├── Cast to WidgetB
└── Call Function (SetData)
```

**Method 2: Event Dispatcher**
```
Widget A
├── Call Event Dispatcher (OnDataChanged)

Widget B (Event Construct)
├── Get Widget ("WidgetA")
├── Cast to WidgetA
└── Bind Event to OnDataChanged
```

**Method 3: EventBus (Recommended)**
```
Widget A
├── Get Event Manager
└── Dispatch Event ("Data.Changed", EventData)

Widget B (Event Construct)
├── Get Event Manager
└── Subscribe to Event ("Data.Changed")
```

---

### Q: How do I pause the game when a menu is open?

**A:** Use Set Game Paused.

**Blueprint Implementation:**
```
Open Menu
├── Set Game Paused (True)
├── Set Input Mode UI Only
└── Show Mouse Cursor (True)

Close Menu
├── Set Game Paused (False)
├── Set Input Mode Game Only
└── Show Mouse Cursor (False)
```

**Important:** Ensure game mode allows pausing (Can Pause = True).

---

## Troubleshooting

### Problem: "Blueprint could not be loaded because it derives from an invalid class"

**Solution:** The parent C++ class is missing or not compiled.
1. Verify C++ class exists in source code
2. Compile C++ project
3. Restart Unreal Editor
4. Recreate Blueprint if necessary

---

### Problem: "Accessed None trying to read property"

**Solution:** A variable is null (not initialized).
1. Add null checks before accessing variables
2. Verify widgets are marked "Is Variable"
3. Check that Get Widget returns valid result
4. Use Branch (Is Valid) before accessing

---

### Problem: Widget appears but is not interactive

**Solution:** Input is blocked or not enabled.
1. Check Hit Test Invisible on parent widgets
2. Verify Set Input Mode UI Only is called
3. Check button Is Enabled is True
4. Verify Z-Order (not behind other widgets)

---

### Problem: Animations not playing

**Solution:** Animation not found or widget not visible.
1. Verify animation name matches exactly
2. Check widget is Visible (not Collapsed)
3. Ensure animation length > 0
4. Check that Play Animation is actually called

---

### Problem: Text not updating

**Solution:** Text block not marked as variable or wrong reference.
1. Mark Text Block "Is Variable" in Designer
2. Verify variable name matches in Blueprint
3. Check that Set Text is called
4. Use Print String to debug text value

---

### Problem: Memory leak / widgets not being destroyed

**Solution:** Event subscriptions not cleaned up.
1. Unsubscribe from events in Event Destruct
2. Clear timer handles
3. Unbind event dispatchers
4. Remove from parent before destroying

---

## Best Practices

### Do's
✓ Use Invalidation Boxes for static content  
✓ Mark widgets "Is Variable" only if needed in Blueprint  
✓ Use proper anchors for responsive design  
✓ Cache material instances in Event Construct  
✓ Unsubscribe from events in Event Destruct  
✓ Use object pooling for repeated widgets  
✓ Profile performance with Unreal Insights  
✓ Test at multiple resolutions  
✓ Use Event Dispatchers for widget communication  
✓ Follow naming conventions (WBP_ prefix)  

### Don'ts
✗ Don't use Event Tick unless necessary  
✗ Don't create/destroy widgets every frame  
✗ Don't forget to unsubscribe from events  
✗ Don't use absolute positions (use anchors)  
✗ Don't make materials too complex (< 100 instructions)  
✗ Don't block input with Hit Test Invisible parents  
✗ Don't forget to test at different resolutions  
✗ Don't hardcode text (use localization)  
✗ Don't access null references (use Is Valid checks)  
✗ Don't forget to set Input Mode when opening menus  

---

## Additional Resources

### Unreal Engine Documentation
- UMG UI Designer: https://docs.unrealengine.com/en-US/umg-ui-designer/
- Widget Blueprints: https://docs.unrealengine.com/en-US/widget-blueprints/
- UMG Best Practices: https://docs.unrealengine.com/en-US/umg-best-practices/

### Project-Specific Documentation
- Implementation Guides: See individual WBP_*_Guide.md files
- Asset Requirements: See AssetRequirements.md
- Implementation Checklist: See ImplementationChecklist.md

### Tools
- Widget Reflector: Tools → Debug → Widget Reflector
- Unreal Insights: Tools → Session Frontend → Profiler
- Blueprint Debugger: Debug → Start Debugging

---

## Getting Help

If you encounter issues not covered in this FAQ:

1. Check the implementation guides for your specific widget
2. Use Widget Reflector to inspect the widget hierarchy
3. Profile with Unreal Insights to identify performance issues
4. Review the C++ source code for the parent class
5. Check Unreal Engine documentation for UMG
6. Ask the team for assistance

**Common Support Channels:**
- Team Slack: #ui-development
- Code Reviews: Submit PR for review
- Documentation: Update this FAQ with new findings
