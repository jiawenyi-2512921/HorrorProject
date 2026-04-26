# Frequently Asked Questions (FAQ)

## General

### Q: What engine version is required?
**A:** Unreal Engine 5.6 or higher.

### Q: What platforms are supported?
**A:** Windows, with planned support for consoles and other platforms.

### Q: Is multiplayer supported?
**A:** Yes, the game includes a full multiplayer system supporting up to 4 players.

---

## Achievements

### Q: How do I unlock an achievement?
**A:** Use `AchievementSubsystem->UnlockAchievement(FName("ACH_ID"))`.

### Q: Can achievements be reset for testing?
**A:** Yes, use `AchievementSubsystem->ResetAchievement(FName("ACH_ID"))`.

### Q: Do achievements work offline?
**A:** Yes, achievements are tracked locally and synced when platform becomes available.

### Q: How do I create a custom achievement?
**A:** Add definition to `AchievementList.md` and implement tracking logic.

### Q: What's the difference between progressive and instant achievements?
**A:** Progressive achievements track progress (e.g., collect 10 items), instant achievements unlock immediately.

---

## Analytics

### Q: Is analytics GDPR compliant?
**A:** Yes, the system requires explicit user consent and supports data deletion.

### Q: Where is analytics data stored?
**A:** Locally in `Saved/Analytics/` directory.

### Q: How do I view analytics reports?
**A:** Run `ReportGenerator.ps1` and open the generated HTML report.

### Q: Can I export user data?
**A:** Yes, use `AnalyticsSubsystem->ExportUserData()`.

### Q: How often are events flushed to disk?
**A:** Every 30 seconds or when 1000 events are queued.

### Q: Does analytics impact performance?
**A:** Minimal impact - events are queued and flushed asynchronously.

---

## Accessibility

### Q: Is the game WCAG 2.1 AA compliant?
**A:** Yes, the accessibility system meets WCAG 2.1 AA standards.

### Q: Can I customize subtitle appearance?
**A:** Yes, size, color, and background are fully customizable.

### Q: What color blind modes are supported?
**A:** Protanopia (red-blind), Deuteranopia (green-blind), and Tritanopia (blue-blind).

### Q: How do visual audio cues work?
**A:** They display directional indicators showing sound source location and type.

### Q: Can players remap controls?
**A:** Yes, full keyboard and gamepad remapping is supported.

### Q: Does the game support screen readers?
**A:** Basic support is implemented, with plans for full integration.

---

## Localization

### Q: What languages are supported?
**A:** English, Chinese (Simplified), Japanese, Korean, and Spanish.

### Q: Can I add a new language?
**A:** Yes, add translations to `MasterLocalizationTable.csv` and regenerate files.

### Q: How do I change language at runtime?
**A:** Use `LocalizationSubsystem->SetLanguage(ELanguage::Chinese)`.

### Q: Do CJK languages require special fonts?
**A:** Yes, use fonts with full CJK character support (e.g., Noto Sans CJK).

### Q: How do I localize audio?
**A:** Place localized audio files in `Content/Audio/Localization/{language}/`.

### Q: What happens if a translation is missing?
**A:** The system falls back to English.

---

## Network Multiplayer

### Q: How many players are supported?
**A:** Up to 4 players per session.

### Q: Is LAN play supported?
**A:** Yes, set `bIsLAN=true` when creating sessions.

### Q: How do I test multiplayer locally?
**A:** Launch multiple instances: host with `-log`, client with `127.0.0.1 -log`.

### Q: What online subsystems are supported?
**A:** NULL (for testing), Steam, and Epic Online Services.

### Q: Why can't clients see my session?
**A:** Check firewall settings and ensure session is started after creation.

### Q: How do I replicate custom data?
**A:** Implement `GetLifetimeReplicatedProps()` and mark properties with `UPROPERTY(Replicated)`.

---

## Performance

### Q: What's the target frame rate?
**A:** 60 FPS on recommended hardware.

### Q: How do I profile performance?
**A:** Use Unreal Insights or the built-in stat commands (`stat fps`, `stat unit`).

### Q: Why is my FPS low?
**A:** Check Performance documentation for optimization guides.

### Q: Does the game support ray tracing?
**A:** Planned for future updates.

---

## Development

### Q: How do I build the project?
**A:** Open `HorrorProject.sln` in Visual Studio and build in Development Editor configuration.

### Q: What IDE is recommended?
**A:** Visual Studio 2022 with Unreal Engine workload.

### Q: How do I run automated tests?
**A:** Session Frontend → Automation → Select test category.

### Q: Where are the source files?
**A:** `Source/HorrorProject/` directory.

### Q: How do I add a new system?
**A:** Follow the architecture patterns in existing systems and add documentation.

---

## Troubleshooting

### Q: Game won't launch
**A:** Check logs in `Saved/Logs/`, verify engine version, rebuild project.

### Q: Compilation errors
**A:** Clean solution, regenerate project files, rebuild.

### Q: Missing DLLs
**A:** Ensure all dependencies are installed, check `Binaries/` directory.

### Q: Crashes on startup
**A:** Check crash logs in `Saved/Crashes/`, verify hardware compatibility.

### Q: Assets not loading
**A:** Verify asset paths, check Content Browser, reimport if necessary.

---

## Content Creation

### Q: What asset formats are supported?
**A:** FBX for models, PNG/TGA for textures, WAV for audio.

### Q: How do I import custom assets?
**A:** Drag and drop into Content Browser or use Import button.

### Q: What's the recommended texture resolution?
**A:** 2K for main assets, 4K for hero assets, 1K for background.

### Q: Can I use Marketplace assets?
**A:** Yes, ensure license compatibility.

---

## Modding

### Q: Does the game support mods?
**A:** Planned for future updates.

### Q: Can I access the source code?
**A:** Yes, the project is open for development.

---

## Support

### Q: Where can I report bugs?
**A:** Create an issue in the project repository.

### Q: How do I contribute?
**A:** Follow contribution guidelines in project documentation.

### Q: Where can I find more documentation?
**A:** Check `Docs/` directory for comprehensive guides.

### Q: Who do I contact for help?
**A:** Refer to project maintainers or community forums.
