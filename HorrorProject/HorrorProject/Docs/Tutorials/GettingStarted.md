# Getting Started with HorrorProject

Complete beginner's guide to get up and running with HorrorProject in 10 steps.

## Prerequisites

Before you begin, ensure you have:
- **Unreal Engine 5.6** installed
- **Visual Studio 2022** with C++ game development workload
- **Git** for version control
- **16GB RAM** minimum (32GB recommended)
- **NVIDIA GTX 1060** or equivalent GPU

## Step 1: Clone the Repository

```bash
git clone https://github.com/your-org/HorrorProject.git
cd HorrorProject
```

**Time**: 2-5 minutes

## Step 2: Generate Project Files

Right-click `HorrorProject.uproject` and select:
- **Generate Visual Studio project files**

Or use command line:
```bash
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" -projectfiles -project="D:\gptzuo\HorrorProject\HorrorProject\HorrorProject.uproject" -game -engine
```

**Time**: 1-2 minutes

## Step 3: Open in Visual Studio

1. Open `HorrorProject.sln` in Visual Studio 2022
2. Set build configuration to **Development Editor**
3. Set platform to **Win64**

![Visual Studio Configuration](../_images/tutorials/vs_config.png)

**Time**: 1 minute

## Step 4: Build the Project

In Visual Studio:
1. Right-click **HorrorProject** in Solution Explorer
2. Select **Build**
3. Wait for compilation to complete

```
Build succeeded.
    0 Warning(s)
    0 Error(s)
```

**Time**: 5-15 minutes (first build)

**Troubleshooting**: See [Compilation Errors](../Troubleshooting/CompilationErrors.md) if build fails.

## Step 5: Launch the Editor

Double-click `HorrorProject.uproject` or press **F5** in Visual Studio.

The Unreal Editor will launch and compile shaders on first run.

**Time**: 3-10 minutes (first launch)

## Step 6: Explore the Content Browser

Navigate to key folders:
- **Content/Blueprints/** - Blueprint assets
- **Content/Maps/** - Level files
- **Content/Audio/** - Sound assets
- **Content/Materials/** - Material assets

![Content Browser](../_images/tutorials/content_browser.png)

**Time**: 2 minutes

## Step 7: Open the Demo Level

1. In Content Browser, navigate to `Content/Maps/`
2. Double-click `DemoLevel.umap`
3. Wait for level to load

**Time**: 1-2 minutes

## Step 8: Test Play in Editor

1. Click **Play** button (or press **Alt+P**)
2. Use **WASD** to move, **Mouse** to look
3. Press **E** to interact with objects
4. Press **Esc** to exit play mode

![Play in Editor](../_images/tutorials/play_in_editor.png)

**Time**: 2 minutes

## Step 9: Create Your First Blueprint

1. Right-click in Content Browser
2. Select **Blueprint Class**
3. Choose **Actor** as parent class
4. Name it `BP_MyFirstActor`
5. Double-click to open Blueprint Editor

![Create Blueprint](../_images/tutorials/create_blueprint.png)

**Time**: 2 minutes

## Step 10: Add Evidence Collection

1. Open `BP_MyFirstActor`
2. Click **Add Component** → **Evidence Collection**
3. In Event Graph, add **BeginPlay** event
4. Add **Print String** node: "Evidence system ready!"
5. Compile and save

![Evidence Blueprint](../_images/tutorials/evidence_blueprint.png)

**Time**: 3 minutes

## Next Steps

Congratulations! You've completed the quick start guide.

### Continue Learning
- [Audio System Tutorial](AudioSystem.md) - Learn the audio system
- [Evidence System Tutorial](EvidenceCollection.md) - Master evidence collection
- [AI System Tutorial](AISystem.md) - Create threat AI
- [Blueprint Best Practices](../BestPractices/BlueprintDesign.md) - Improve your blueprints

### Explore Systems
- [System Overview](../Architecture/SystemOverview.md) - Understand the architecture
- [API Reference](../API/README.md) - Browse the API documentation
- [Performance Guide](../Performance/OptimizationGuide.md) - Optimize your game

### Get Help
- [FAQ](../Troubleshooting/FAQ.md) - Common questions
- [Troubleshooting](../Troubleshooting/README.md) - Solve problems
- [Discord Community](https://discord.gg/horrorproject) - Ask questions

## Common Issues

### Build Fails
See [Compilation Errors](../Troubleshooting/CompilationErrors.md)

### Editor Crashes
See [Runtime Errors](../Troubleshooting/RuntimeErrors.md)

### Performance Issues
See [Performance Diagnostics](../Troubleshooting/PerformanceIssues.md)

## Estimated Total Time

**30-45 minutes** for complete setup and first blueprint.

## Video Tutorial

Watch the video walkthrough: [Getting Started Video](https://youtube.com/horrorproject-getting-started)

## Feedback

Found an issue with this tutorial? [Report it here](https://github.com/your-org/HorrorProject/issues)
