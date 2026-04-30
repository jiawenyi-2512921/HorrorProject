"""
EncounterSetup.py - 遭遇配置工具

配置HorrorEncounterDirector系统：
- 设置威胁生成点和触发区域
- 配置遭遇阶段和行为
- 难度曲线和动态调整
- 与HorrorEventBus集成

与HorrorProject Game模块集成。
"""

import unreal
import json
from pathlib import Path
from typing import List, Dict, Optional, Tuple
from dataclasses import dataclass
from enum import Enum


class EncounterPhase(Enum):
    """遭遇阶段"""
    DORMANT = "Dormant"  # 休眠
    PRIMED = "Primed"  # 待触发
    REVEALED = "Revealed"  # 已现身
    RESOLVED = "Resolved"  # 已解决


class DifficultyLevel(Enum):
    """难度等级"""
    EASY = "Easy"
    NORMAL = "Normal"
    HARD = "Hard"
    NIGHTMARE = "Nightmare"


@dataclass
class TriggerZoneConfig:
    """触发区域配置"""
    location: Tuple[float, float, float]
    extent: Tuple[float, float, float]  # Box extent
    trigger_once: bool = True
    require_line_of_sight: bool = False


@dataclass
class ThreatSpawnConfig:
    """威胁生成配置"""
    threat_class_path: str
    spawn_location: Tuple[float, float, float]
    spawn_rotation: Tuple[float, float, float] = (0, 0, 0)
    relative_to_player: bool = False
    spawn_distance: float = 500.0


@dataclass
class EncounterConfig:
    """遭遇配置"""
    encounter_id: str
    encounter_name: str
    trigger_zone: TriggerZoneConfig
    threat_spawn: ThreatSpawnConfig
    reveal_radius: float = 1200.0
    gate_route: bool = True
    difficulty: DifficultyLevel = DifficultyLevel.NORMAL
    audio_config: Optional[Dict] = None
    camera_shake_config: Optional[Dict] = None


class EncounterSetup:
    """遭遇配置工具"""

    def __init__(self, level: unreal.Level = None):
        self.level = level or unreal.EditorLevelLibrary.get_editor_world()
        self.editor_actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
        self.created_encounters: List[unreal.Actor] = []

    def create_encounter_director(self, config: EncounterConfig) -> Optional[unreal.Actor]:
        """创建遭遇导演Actor

        Args:
            config: 遭遇配置

        Returns:
            创建的EncounterDirector Actor
        """
        # 加载EncounterDirector类
        director_class = unreal.load_class(None, "/Script/HorrorProject.HorrorEncounterDirector")
        if not director_class:
            unreal.log_error("无法加载HorrorEncounterDirector类")
            return None

        # 生成Director Actor
        location = unreal.Vector(
            config.trigger_zone.location[0],
            config.trigger_zone.location[1],
            config.trigger_zone.location[2]
        )

        director = self.editor_actor_subsystem.spawn_actor_from_class(
            director_class,
            location,
            unreal.Rotator(0, 0, 0)
        )

        if not director:
            unreal.log_error("无法创建EncounterDirector")
            return None

        # 设置名称
        director.set_actor_label(config.encounter_name)

        # 配置基础属性
        director.set_editor_property("default_encounter_id", unreal.Name(config.encounter_id))
        director.set_editor_property("reveal_radius", config.reveal_radius)
        director.set_editor_property("gate_route_during_reveal", config.gate_route)

        # 配置威胁类
        if config.threat_spawn.threat_class_path:
            threat_class = unreal.load_class(None, config.threat_spawn.threat_class_path)
            if threat_class:
                director.set_editor_property("threat_class", threat_class)

        # 配置威胁生成位置（相对变换）
        spawn_loc = unreal.Vector(
            config.threat_spawn.spawn_location[0],
            config.threat_spawn.spawn_location[1],
            config.threat_spawn.spawn_location[2]
        )
        spawn_rot = unreal.Rotator(
            config.threat_spawn.spawn_rotation[0],
            config.threat_spawn.spawn_rotation[1],
            config.threat_spawn.spawn_rotation[2]
        )
        threat_transform = unreal.Transform(spawn_rot, spawn_loc, unreal.Vector(1, 1, 1))
        director.set_editor_property("threat_relative_transform", threat_transform)

        # 配置音频
        if config.audio_config:
            self._configure_audio(director, config.audio_config)

        # 配置相机震动
        if config.camera_shake_config:
            self._configure_camera_shake(director, config.camera_shake_config)

        # 添加标签
        director.tags.append("EncounterDirector")
        director.tags.append(f"Difficulty_{config.difficulty.value}")
        director.tags.append(config.encounter_id)

        self.created_encounters.append(director)
        unreal.log(f"成功创建遭遇导演: {config.encounter_name}")

        return director

    def create_trigger_volume(
        self,
        config: TriggerZoneConfig,
        encounter_director: unreal.Actor
    ) -> Optional[unreal.Actor]:
        """创建触发区域

        Args:
            config: 触发区域配置
            encounter_director: 关联的遭遇导演

        Returns:
            创建的触发体积Actor
        """
        # 加载TriggerBox类
        trigger_class = unreal.load_class(None, "/Script/Engine.TriggerBox")
        if not trigger_class:
            unreal.log_error("无法加载TriggerBox类")
            return None

        location = unreal.Vector(config.location[0], config.location[1], config.location[2])
        trigger = self.editor_actor_subsystem.spawn_actor_from_class(
            trigger_class,
            location,
            unreal.Rotator(0, 0, 0)
        )

        if not trigger:
            unreal.log_error("无法创建触发区域")
            return None

        # 设置触发体积大小
        brush_comp = trigger.get_component_by_class(unreal.BrushComponent)
        if brush_comp:
            extent = unreal.Vector(config.extent[0], config.extent[1], config.extent[2])
            # 注意：需要通过蓝图或C++来正确设置Box extent
            # 这里添加标签记录配置
            trigger.tags.append(f"Extent_{config.extent[0]}_{config.extent[1]}_{config.extent[2]}")

        # 关联到遭遇导演
        trigger.tags.append("EncounterTrigger")
        trigger.tags.append(f"Director_{encounter_director.get_actor_label()}")

        if config.trigger_once:
            trigger.tags.append("TriggerOnce")

        if config.require_line_of_sight:
            trigger.tags.append("RequireLineOfSight")

        unreal.log(f"成功创建触发区域，关联到 {encounter_director.get_actor_label()}")
        return trigger

    def setup_complete_encounter(self, config: EncounterConfig) -> Tuple[Optional[unreal.Actor], Optional[unreal.Actor]]:
        """设置完整遭遇（导演+触发区域）

        Args:
            config: 遭遇配置

        Returns:
            (EncounterDirector, TriggerVolume) 元组
        """
        # 创建遭遇导演
        director = self.create_encounter_director(config)
        if not director:
            return None, None

        # 创建触发区域
        trigger = self.create_trigger_volume(config.trigger_zone, director)

        return director, trigger

    def configure_difficulty_curve(
        self,
        encounter_director: unreal.Actor,
        difficulty: DifficultyLevel
    ) -> bool:
        """配置难度曲线

        Args:
            encounter_director: 遭遇导演Actor
            difficulty: 难度等级

        Returns:
            是否成功
        """
        if not encounter_director:
            return False

        # 根据难度调整参数
        difficulty_params = {
            DifficultyLevel.EASY: {
                "reveal_radius": 1500.0,
                "reveal_delay": 2.0,
                "camera_shake_scale": 0.5
            },
            DifficultyLevel.NORMAL: {
                "reveal_radius": 1200.0,
                "reveal_delay": 1.0,
                "camera_shake_scale": 1.0
            },
            DifficultyLevel.HARD: {
                "reveal_radius": 1000.0,
                "reveal_delay": 0.5,
                "camera_shake_scale": 1.5
            },
            DifficultyLevel.NIGHTMARE: {
                "reveal_radius": 800.0,
                "reveal_delay": 0.0,
                "camera_shake_scale": 2.0
            }
        }

        params = difficulty_params.get(difficulty)
        if not params:
            return False

        try:
            encounter_director.set_editor_property("reveal_radius", params["reveal_radius"])
            encounter_director.set_editor_property("reveal_delay_seconds", params["reveal_delay"])
            encounter_director.set_editor_property("reveal_camera_shake_scale", params["camera_shake_scale"])

            # 添加难度标签
            encounter_director.tags.append(f"Difficulty_{difficulty.value}")

            unreal.log(f"已配置难度: {difficulty.value}")
            return True

        except Exception as e:
            unreal.log_error(f"配置难度失败: {e}")
            return False

    def create_encounter_sequence(
        self,
        configs: List[EncounterConfig],
        sequential: bool = True
    ) -> List[unreal.Actor]:
        """创建遭遇序列

        Args:
            configs: 遭遇配置列表
            sequential: 是否顺序触发（否则并行）

        Returns:
            创建的EncounterDirector列表
        """
        directors = []

        for i, config in enumerate(configs):
            director, trigger = self.setup_complete_encounter(config)
            if director:
                directors.append(director)

                # 如果是顺序触发，添加依赖标签
                if sequential and i > 0:
                    prev_director = directors[i - 1]
                    director.tags.append(f"RequiresPrevious_{prev_director.get_actor_label()}")

        unreal.log(f"成功创建遭遇序列，共 {len(directors)} 个遭遇")
        return directors

    def _configure_audio(self, director: unreal.Actor, audio_config: Dict):
        """配置音频"""
        # 加载音频资源
        if "prime_sound" in audio_config:
            sound = unreal.load_asset(audio_config["prime_sound"])
            if sound:
                director.set_editor_property("prime_sound", sound)

        if "reveal_sound" in audio_config:
            sound = unreal.load_asset(audio_config["reveal_sound"])
            if sound:
                director.set_editor_property("reveal_sound", sound)

        if "resolve_sound" in audio_config:
            sound = unreal.load_asset(audio_config["resolve_sound"])
            if sound:
                director.set_editor_property("resolve_sound", sound)

    def _configure_camera_shake(self, director: unreal.Actor, shake_config: Dict):
        """配置相机震动"""
        if "shake_class" in shake_config:
            shake_class = unreal.load_class(None, shake_config["shake_class"])
            if shake_class:
                director.set_editor_property("reveal_camera_shake", shake_class)

        if "shake_scale" in shake_config:
            director.set_editor_property("reveal_camera_shake_scale", float(shake_config["shake_scale"]))

    def load_from_json(self, json_path: str) -> List[unreal.Actor]:
        """从JSON文件加载遭遇配置

        Args:
            json_path: JSON配置文件路径

        Returns:
            创建的EncounterDirector列表
        """
        try:
            with open(json_path, 'r', encoding='utf-8') as f:
                data = json.load(f)

            directors = []

            for encounter_data in data.get("encounters", []):
                # 解析触发区域
                trigger_zone = TriggerZoneConfig(
                    location=tuple(encounter_data["trigger_zone"]["location"]),
                    extent=tuple(encounter_data["trigger_zone"]["extent"]),
                    trigger_once=encounter_data["trigger_zone"].get("trigger_once", True),
                    require_line_of_sight=encounter_data["trigger_zone"].get("require_line_of_sight", False)
                )

                # 解析威胁生成
                threat_spawn = ThreatSpawnConfig(
                    threat_class_path=encounter_data["threat_spawn"]["class_path"],
                    spawn_location=tuple(encounter_data["threat_spawn"]["location"]),
                    spawn_rotation=tuple(encounter_data["threat_spawn"].get("rotation", [0, 0, 0])),
                    relative_to_player=encounter_data["threat_spawn"].get("relative_to_player", False),
                    spawn_distance=encounter_data["threat_spawn"].get("spawn_distance", 500.0)
                )

                # 解析难度
                difficulty_str = encounter_data.get("difficulty", "Normal")
                difficulty = DifficultyLevel[difficulty_str.upper()]

                # 创建配置
                config = EncounterConfig(
                    encounter_id=encounter_data["id"],
                    encounter_name=encounter_data["name"],
                    trigger_zone=trigger_zone,
                    threat_spawn=threat_spawn,
                    reveal_radius=encounter_data.get("reveal_radius", 1200.0),
                    gate_route=encounter_data.get("gate_route", True),
                    difficulty=difficulty,
                    audio_config=encounter_data.get("audio"),
                    camera_shake_config=encounter_data.get("camera_shake")
                )

                # 创建遭遇
                director, trigger = self.setup_complete_encounter(config)
                if director:
                    directors.append(director)

            unreal.log(f"从JSON加载了 {len(directors)} 个遭遇")
            return directors

        except Exception as e:
            unreal.log_error(f"加载JSON配置失败: {e}")
            return []

    def clear_created_encounters(self):
        """清除所有创建的遭遇"""
        for encounter in self.created_encounters:
            if encounter:
                self.editor_actor_subsystem.destroy_actor(encounter)
        self.created_encounters.clear()
        unreal.log("已清除所有创建的遭遇")


# 使用示例
def example_create_simple_encounter():
    """示例：创建简单遭遇"""
    setup = EncounterSetup()

    config = EncounterConfig(
        encounter_id="encounter.golem_reveal_01",
        encounter_name="GolemEncounter_Hallway",
        trigger_zone=TriggerZoneConfig(
            location=(2000, 0, 100),
            extent=(200, 200, 100),
            trigger_once=True
        ),
        threat_spawn=ThreatSpawnConfig(
            threat_class_path="/Script/HorrorProject.HorrorThreatCharacter",
            spawn_location=(100, 0, 0),
            spawn_rotation=(0, 180, 0)
        ),
        reveal_radius=1200.0,
        gate_route=True,
        difficulty=DifficultyLevel.NORMAL
    )

    return setup.setup_complete_encounter(config)


def example_create_encounter_with_audio():
    """示例：创建带音频的遭遇"""
    setup = EncounterSetup()

    config = EncounterConfig(
        encounter_id="encounter.golem_reveal_02",
        encounter_name="GolemEncounter_Room",
        trigger_zone=TriggerZoneConfig(
            location=(5000, 0, 100),
            extent=(300, 300, 150)
        ),
        threat_spawn=ThreatSpawnConfig(
            threat_class_path="/Script/HorrorProject.HorrorThreatCharacter",
            spawn_location=(200, 0, 0)
        ),
        audio_config={
            "prime_sound": "/Game/Audio/Encounters/SFX_Encounter_Prime",
            "reveal_sound": "/Game/Audio/Encounters/SFX_Golem_Roar",
            "resolve_sound": "/Game/Audio/Encounters/SFX_Encounter_Resolve"
        },
        camera_shake_config={
            "shake_class": "/Script/Engine.CameraShakeBase",
            "shake_scale": 1.5
        }
    )

    return setup.setup_complete_encounter(config)


def example_create_encounter_sequence():
    """示例：创建遭遇序列"""
    setup = EncounterSetup()

    configs = [
        EncounterConfig(
            encounter_id="encounter.sequence_01",
            encounter_name="Encounter_Phase1",
            trigger_zone=TriggerZoneConfig(
                location=(1000, 0, 100),
                extent=(200, 200, 100)
            ),
            threat_spawn=ThreatSpawnConfig(
                threat_class_path="/Script/HorrorProject.HorrorThreatCharacter",
                spawn_location=(100, 0, 0)
            ),
            difficulty=DifficultyLevel.EASY
        ),
        EncounterConfig(
            encounter_id="encounter.sequence_02",
            encounter_name="Encounter_Phase2",
            trigger_zone=TriggerZoneConfig(
                location=(3000, 0, 100),
                extent=(200, 200, 100)
            ),
            threat_spawn=ThreatSpawnConfig(
                threat_class_path="/Script/HorrorProject.HorrorThreatCharacter",
                spawn_location=(100, 0, 0)
            ),
            difficulty=DifficultyLevel.NORMAL
        ),
        EncounterConfig(
            encounter_id="encounter.sequence_03",
            encounter_name="Encounter_Phase3",
            trigger_zone=TriggerZoneConfig(
                location=(5000, 0, 100),
                extent=(200, 200, 100)
            ),
            threat_spawn=ThreatSpawnConfig(
                threat_class_path="/Script/HorrorProject.HorrorThreatCharacter",
                spawn_location=(100, 0, 0)
            ),
            difficulty=DifficultyLevel.HARD
        )
    ]

    return setup.create_encounter_sequence(configs, sequential=True)


def example_load_from_json():
    """示例：从JSON加载遭遇配置"""
    setup = EncounterSetup()

    # JSON格式示例：
    # {
    #   "encounters": [
    #     {
    #       "id": "encounter.golem_01",
    #       "name": "GolemEncounter_Main",
    #       "trigger_zone": {
    #         "location": [2000, 0, 100],
    #         "extent": [200, 200, 100],
    #         "trigger_once": true
    #       },
    #       "threat_spawn": {
    #         "class_path": "/Script/HorrorProject.HorrorThreatCharacter",
    #         "location": [100, 0, 0],
    #         "rotation": [0, 180, 0]
    #       },
    #       "reveal_radius": 1200.0,
    #       "difficulty": "Normal"
    #     }
    #   ]
    # }

    project_root = Path(__file__).resolve().parents[2]
    json_path = project_root / "Content" / "LevelDesign" / "Encounters.json"
    return setup.load_from_json(str(json_path))


if __name__ == "__main__":
    unreal.log("EncounterSetup工具已加载")
    unreal.log("使用示例：")
    unreal.log("  setup = EncounterSetup()")
    unreal.log("  setup.setup_complete_encounter(config)")
    unreal.log("  setup.create_encounter_sequence(configs)")
    unreal.log("  setup.load_from_json(path)")
