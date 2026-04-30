import unreal


CAMPAIGN_MAPS = [
    "/Game/Bodycam_VHS_Effect/Maps/LVL_Showcase_01",
    "/Game/ForestOfSpikes/Levels/Level_ForestOfSpikes_Demo_Night",
    "/Game/Scrapopolis/Levels/Level_Scrapopolis_Demo",
    "/Game/Fantastic_Dungeon_Pack/maps/map_dungeon_level_2_entrance",
    "/Game/Fantastic_Dungeon_Pack/maps/map_dungeon_level_1_dungeon",
    "/Game/Fantastic_Dungeon_Pack/maps/map_dungeon_level_3_hall",
    "/Game/Fantastic_Dungeon_Pack/maps/map_dungeon_level_4_temple",
    "/Game/Fantastic_Dungeon_Pack/maps/map_dungeon_level_5_bossroom",
    "/Game/DeepWaterStation/Maps/DemoMap_VerticalSlice_Day1",
]

GAME_MODE_CLASS_PATH = "/Game/Horror/BP_HorrorGameModeBase.BP_HorrorGameModeBase_C"


def main():
    game_mode_class = unreal.load_class(None, GAME_MODE_CLASS_PATH)
    if not game_mode_class:
        raise RuntimeError(f"Could not load campaign game mode: {GAME_MODE_CLASS_PATH}")

    for map_path in CAMPAIGN_MAPS:
        unreal.log(f"Loading campaign map for GameMode override: {map_path}")
        if not unreal.EditorLoadingAndSavingUtils.load_map(map_path):
            raise RuntimeError(f"Failed to load map: {map_path}")

        world = unreal.EditorLevelLibrary.get_editor_world()
        if not world:
            raise RuntimeError(f"No editor world after loading map: {map_path}")

        world_settings = world.get_world_settings()
        world_settings.set_editor_property("default_game_mode", game_mode_class)
        unreal.EditorLoadingAndSavingUtils.save_current_level()
        unreal.log(f"Saved campaign GameMode override on {map_path}")


if __name__ == "__main__":
    main()
