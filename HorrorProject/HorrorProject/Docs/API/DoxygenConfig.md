# Doxygen Configuration

Configuration for generating API documentation using Doxygen.

## Installation

### Windows
```bash
# Using Chocolatey
choco install doxygen.install

# Or download from: https://www.doxygen.nl/download.html
```

### Linux
```bash
sudo apt-get install doxygen graphviz
```

## Configuration File

The `Doxyfile` is located at: `Docs/API/Doxyfile`

### Key Settings

```ini
PROJECT_NAME           = "HorrorProject"
PROJECT_BRIEF          = "Horror Game Framework API Documentation"
OUTPUT_DIRECTORY       = ./Generated
INPUT                  = ../../Source/HorrorProject
RECURSIVE              = YES
EXTRACT_ALL            = YES
EXTRACT_PRIVATE        = NO
EXTRACT_STATIC         = YES
GENERATE_HTML          = YES
GENERATE_LATEX         = NO
HTML_OUTPUT            = html
```

## Generating Documentation

### Command Line
```bash
cd D:/gptzuo/HorrorProject/HorrorProject/Docs/API
doxygen Doxyfile
```

### Output Location
Generated documentation: `Docs/API/Generated/html/index.html`

## Documentation Comments

### Class Documentation
```cpp
/**
 * @brief Manages evidence collection for the player
 * 
 * This component handles collecting, storing, and managing evidence items
 * that the player finds throughout the game.
 * 
 * @see AEvidenceActor
 * @see UArchiveSubsystem
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HORRORPROJECT_API UEvidenceCollectionComponent : public UActorComponent
{
    GENERATED_BODY()
};
```

### Function Documentation
```cpp
/**
 * @brief Collects an evidence item
 * 
 * Attempts to add the specified evidence to the player's collection.
 * Will fail if the collection is full or the evidence is invalid.
 * 
 * @param Evidence The evidence actor to collect
 * @return true if collection succeeded, false otherwise
 * 
 * @code
 * bool bSuccess = EvidenceComponent->CollectEvidence(EvidenceActor);
 * if (bSuccess)
 * {
 *     UE_LOG(LogTemp, Log, TEXT("Evidence collected!"));
 * }
 * @endcode
 */
UFUNCTION(BlueprintCallable, Category = "Evidence")
bool CollectEvidence(AEvidenceActor* Evidence);
```

### Property Documentation
```cpp
/**
 * @brief Maximum number of evidence items that can be collected
 * 
 * When this limit is reached, no more evidence can be collected
 * until some items are removed.
 */
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evidence Settings")
int32 MaxEvidenceCapacity = 100;
```

## Doxygen Tags Reference

### Common Tags
- `@brief` - Short description
- `@param` - Parameter description
- `@return` - Return value description
- `@code` - Code example
- `@see` - Cross-reference
- `@note` - Important note
- `@warning` - Warning message
- `@deprecated` - Deprecated feature

### Example with Multiple Tags
```cpp
/**
 * @brief Plays an ambient sound with fade-in
 * 
 * @param Sound The sound asset to play
 * @param FadeInTime Fade-in duration in seconds
 * 
 * @note This will stop any currently playing ambient sound
 * @warning Sound must not be nullptr
 * 
 * @see StopAmbientSound
 * @see SetMusicIntensity
 */
UFUNCTION(BlueprintCallable, Category = "Horror Audio")
void PlayAmbientSound(USoundBase* Sound, float FadeInTime = 2.0f);
```

## Customizing Output

### Themes
Edit `HTML_EXTRA_STYLESHEET` in Doxyfile:
```ini
HTML_EXTRA_STYLESHEET  = custom.css
```

### Logo
```ini
PROJECT_LOGO           = ../../Content/UI/Logo.png
```

### Search
```ini
SEARCHENGINE           = YES
SERVER_BASED_SEARCH    = NO
```

## Integration with CI/CD

### GitHub Actions
```yaml
name: Generate Docs

on:
  push:
    branches: [ main ]

jobs:
  docs:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Install Doxygen
        run: sudo apt-get install doxygen graphviz
      - name: Generate Documentation
        run: |
          cd Docs/API
          doxygen Doxyfile
      - name: Deploy to GitHub Pages
        uses: peaceiris/actions-gh-pages@v3
        with:
          github_token: ${{ secrets.GITHUB_TOKEN }}
          publish_dir: ./Docs/API/Generated/html
```

## Troubleshooting

### Issue: No output generated
**Solution**: Check that INPUT path is correct and files exist

### Issue: Missing class documentation
**Solution**: Ensure EXTRACT_ALL = YES in Doxyfile

### Issue: Graphs not generated
**Solution**: Install graphviz package

## Related Documentation
- [API Reference](README.md)
- [Documentation Standards](../BestPractices/DocumentationStandards.md)
- [Contributing Guide](../Developer/ContributingGuide.md)
