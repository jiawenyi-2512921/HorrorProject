# Blueprint Asset Migration Checklist

## Pre-Migration Planning

- [ ] Identify all assets to migrate
- [ ] Document current asset locations
- [ ] Map destination structure
- [ ] Identify dependencies
- [ ] Create backup of source assets
- [ ] Plan migration order (dependencies first)
- [ ] Schedule migration window
- [ ] Notify team members

## Dependency Analysis

- [ ] Run CheckBlueprintReferences.ps1
- [ ] List all Blueprint dependencies
- [ ] Identify external asset references
  - [ ] Materials
  - [ ] Textures
  - [ ] Meshes
  - [ ] Sounds
  - [ ] Animations
  - [ ] Particle systems
- [ ] Document circular dependencies
- [ ] Create dependency graph
- [ ] Resolve dependency conflicts

## Source Validation

- [ ] Run ValidateBlueprints.ps1 on source
- [ ] Fix any validation errors
- [ ] Run FindBrokenBlueprints.ps1
- [ ] Repair broken Blueprints
- [ ] Verify all references are valid
- [ ] Check for missing assets
- [ ] Ensure no compilation errors
- [ ] Test source Blueprints functionality

## Destination Preparation

- [ ] Create destination directory structure
- [ ] Verify write permissions
- [ ] Check available disk space
- [ ] Ensure destination is in version control
- [ ] Create migration branch (if using Git)
- [ ] Document new folder structure

## Migration Execution

- [ ] Run MigrateBlueprintAssets.ps1 with -DryRun
- [ ] Review dry run results
- [ ] Verify migration plan
- [ ] Execute actual migration
- [ ] Verify all files copied successfully
- [ ] Check file integrity
- [ ] Verify metadata files migrated

## Reference Updates

- [ ] Run UpdateBlueprintReferences.ps1
- [ ] Update all Blueprint references
- [ ] Update C++ references (if applicable)
- [ ] Update config files
- [ ] Update level references
- [ ] Update widget references
- [ ] Update data table references
- [ ] Update animation Blueprint references

## Post-Migration Validation

- [ ] Run ValidateBlueprints.ps1 on destination
- [ ] Verify no broken references
- [ ] Check all GUIDs are intact
- [ ] Verify parent class references
- [ ] Test Blueprint compilation
- [ ] Run FindBrokenBlueprints.ps1
- [ ] Fix any issues found

## Testing

### Functional Testing
- [ ] Test each migrated Blueprint
- [ ] Verify all functions work
- [ ] Test all events fire correctly
- [ ] Verify component behavior
- [ ] Test variable values
- [ ] Check animation playback
- [ ] Verify audio playback
- [ ] Test particle effects

### Integration Testing
- [ ] Test Blueprint interactions
- [ ] Verify level integration
- [ ] Test UI integration
- [ ] Check gameplay systems
- [ ] Test save/load functionality
- [ ] Verify multiplayer (if applicable)

### Platform Testing
- [ ] Test on Windows
- [ ] Test on target consoles
- [ ] Test on mobile (if applicable)
- [ ] Verify performance
- [ ] Check memory usage

## Content Browser

- [ ] Refresh Content Browser
- [ ] Verify assets appear correctly
- [ ] Check asset thumbnails
- [ ] Verify asset categories
- [ ] Update asset tags
- [ ] Fix any display issues

## Version Control

- [ ] Stage all migrated files
- [ ] Stage all updated references
- [ ] Review changes in diff
- [ ] Write descriptive commit message
- [ ] Commit migration
- [ ] Push to remote repository
- [ ] Tag migration commit
- [ ] Update documentation

## Cleanup

- [ ] Remove old asset locations (after verification)
- [ ] Clean up backup files
- [ ] Remove temporary files
- [ ] Update redirectors
- [ ] Fix up redirectors in UE
- [ ] Delete empty folders
- [ ] Clean up logs

## Documentation

- [ ] Update asset location documentation
- [ ] Document migration process
- [ ] Update team wiki
- [ ] Update README files
- [ ] Document any issues encountered
- [ ] Document workarounds applied
- [ ] Create migration report
- [ ] Update asset registry

## Communication

- [ ] Notify team of migration completion
- [ ] Share migration report
- [ ] Document breaking changes
- [ ] Update project documentation
- [ ] Send migration summary email
- [ ] Update project board/tracker

## Rollback Plan

- [ ] Document rollback procedure
- [ ] Keep backup until verified
- [ ] Test rollback process
- [ ] Document rollback triggers
- [ ] Assign rollback responsibility

## Special Cases

### Migrating Between Projects
- [ ] Export assets properly
- [ ] Handle different engine versions
- [ ] Resolve plugin dependencies
- [ ] Update project settings
- [ ] Rebuild lighting
- [ ] Recompile shaders

### Migrating with Localization
- [ ] Migrate localization files
- [ ] Update string tables
- [ ] Verify translations
- [ ] Test all languages

### Migrating Marketplace Assets
- [ ] Check license compatibility
- [ ] Verify marketplace updates
- [ ] Document marketplace versions
- [ ] Update attribution

## Performance Verification

- [ ] Profile Blueprint performance
- [ ] Check load times
- [ ] Verify memory usage
- [ ] Check package size
- [ ] Optimize if needed

## Quality Assurance

- [ ] Run full QA pass
- [ ] Test all affected features
- [ ] Verify no regressions
- [ ] Check for new bugs
- [ ] Validate user experience
- [ ] Performance testing

## Final Verification

- [ ] All Blueprints compile
- [ ] No broken references
- [ ] All tests pass
- [ ] Performance acceptable
- [ ] Documentation updated
- [ ] Team notified
- [ ] Backup verified
- [ ] Rollback plan ready

## Post-Migration Monitoring

- [ ] Monitor for issues (first 24 hours)
- [ ] Track bug reports
- [ ] Monitor performance metrics
- [ ] Gather team feedback
- [ ] Address issues promptly

## Lessons Learned

- [ ] Document what went well
- [ ] Document challenges
- [ ] Document solutions
- [ ] Update migration procedures
- [ ] Share knowledge with team

## Tools Used

- MigrateBlueprintAssets.ps1
- UpdateBlueprintReferences.ps1
- ValidateBlueprints.ps1
- FindBrokenBlueprints.ps1
- CheckBlueprintReferences.ps1

## Common Issues & Solutions

### Broken References
- Run UpdateBlueprintReferences.ps1
- Manually fix complex references
- Use Find in Blueprints tool

### Missing Dependencies
- Identify with CheckBlueprintReferences.ps1
- Migrate dependencies first
- Update dependency paths

### Compilation Errors
- Check parent class references
- Verify component types
- Update function signatures

### Performance Issues
- Profile migrated Blueprints
- Check for reference loops
- Optimize complex graphs

## Notes

- Always backup before migration
- Test thoroughly before committing
- Migrate during low-activity periods
- Keep team informed throughout process
- Document everything
- Have rollback plan ready
- Use dry run mode first
- Verify on multiple machines
- Test packaged builds
- Monitor post-migration
