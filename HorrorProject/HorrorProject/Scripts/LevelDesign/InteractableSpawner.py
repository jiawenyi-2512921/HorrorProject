"""
InteractableSpawner.py - 交互物体批量放置工具

用于快速在关卡中批量创建和配置交互物体：
- EvidenceActor（证据物品）
- DoorInteractable（门）
- SwitchInteractable（开关）
- PickupInteractable（拾取物）

与HorrorProject C++系统集成，自动配置BaseInteractable属性。
"""

import unreal
import json
from pathlib import Path
from typing import List, Dict, Optional, Tuple
from dataclasses import dataclass
from enum import Enum


class InteractableType(Enum):
    """交互物体类型"""
    EVIDENCE = "EvidenceActor"
    DOOR = "DoorInteractable"
    SWITCH = "SwitchInteractable"
    PICKUP = "PickupInteractable"


@dataclass
class SpawnConfig:
    """生成配置"""
    location: Tuple[float, float, float]
    rotation: Tuple[float, float, float] = (0.0, 0.0, 0.0)
    interactable_id: str = ""
    interaction_prompt: str = "互动"
    custom_properties: Dict = None


class InteractableSpawner:
    """交互物体批量生成器"""

    def __init__(self, level: unreal.Level = None):
        self.level = level or unreal.EditorLevelLibrary.get_editor_world()
        self.editor_actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
        self.spawned_actors: List[unreal.Actor] = []

    def spawn_evidence_batch(self, configs: List[SpawnConfig]) -> List[unreal.Actor]:
        """批量生成证据物品

        Args:
            configs: 生成配置列表

        Returns:
            生成的Actor列表
        """
        evidence_class = unreal.load_class(None, "/Script/HorrorProject.EvidenceActor")
        if not evidence_class:
            unreal.log_error("无法加载EvidenceActor类")
            return []

        actors = []
        for config in configs:
            actor = self._spawn_actor(evidence_class, config)
            if actor:
                self._configure_evidence(actor, config)
                actors.append(actor)

        self.spawned_actors.extend(actors)
        unreal.log(f"成功生成 {len(actors)} 个证据物品")
        return actors

    def spawn_doors_batch(self, configs: List[SpawnConfig]) -> List[unreal.Actor]:
        """批量生成门

        Args:
            configs: 生成配置列表

        Returns:
            生成的Actor列表
        """
        door_class = unreal.load_class(None, "/Script/HorrorProject.DoorInteractable")
        if not door_class:
            unreal.log_error("无法加载DoorInteractable类")
            return []

        actors = []
        for config in configs:
            actor = self._spawn_actor(door_class, config)
            if actor:
                self._configure_door(actor, config)
                actors.append(actor)

        self.spawned_actors.extend(actors)
        unreal.log(f"成功生成 {len(actors)} 个门")
        return actors

    def spawn_switches_batch(self, configs: List[SpawnConfig]) -> List[unreal.Actor]:
        """批量生成开关

        Args:
            configs: 生成配置列表

        Returns:
            生成的Actor列表
        """
        switch_class = unreal.load_class(None, "/Script/HorrorProject.SwitchInteractable")
        if not switch_class:
            unreal.log_error("无法加载SwitchInteractable类")
            return []

        actors = []
        for config in configs:
            actor = self._spawn_actor(switch_class, config)
            if actor:
                self._configure_switch(actor, config)
                actors.append(actor)

        self.spawned_actors.extend(actors)
        unreal.log(f"成功生成 {len(actors)} 个开关")
        return actors

    def spawn_pickups_batch(self, configs: List[SpawnConfig]) -> List[unreal.Actor]:
        """批量生成拾取物

        Args:
            configs: 生成配置列表

        Returns:
            生成的Actor列表
        """
        pickup_class = unreal.load_class(None, "/Script/HorrorProject.PickupInteractable")
        if not pickup_class:
            unreal.log_error("无法加载PickupInteractable类")
            return []

        actors = []
        for config in configs:
            actor = self._spawn_actor(pickup_class, config)
            if actor:
                self._configure_pickup(actor, config)
                actors.append(actor)

        self.spawned_actors.extend(actors)
        unreal.log(f"成功生成 {len(actors)} 个拾取物")
        return actors

    def _spawn_actor(self, actor_class: unreal.Class, config: SpawnConfig) -> Optional[unreal.Actor]:
        """生成单个Actor"""
        location = unreal.Vector(config.location[0], config.location[1], config.location[2])
        rotation = unreal.Rotator(config.rotation[0], config.rotation[1], config.rotation[2])

        actor = self.editor_actor_subsystem.spawn_actor_from_class(
            actor_class,
            location,
            rotation
        )

        if actor:
            # 配置基础交互属性
            if config.interactable_id:
                actor.set_editor_property("interactable_id", unreal.Name(config.interactable_id))
            if config.interaction_prompt:
                actor.set_editor_property("interaction_prompt", unreal.Text(config.interaction_prompt))

        return actor

    def _configure_evidence(self, actor: unreal.Actor, config: SpawnConfig):
        """配置证据物品特定属性"""
        if not config.custom_properties:
            return

        props = config.custom_properties

        # 配置证据元数据
        if "evidence_id" in props:
            metadata = unreal.ExtendedEvidenceMetadata()
            metadata.evidence_id = unreal.Name(props["evidence_id"])
            metadata.display_name = unreal.Text(props.get("display_name", "未命名证据"))
            metadata.description = unreal.Text(props.get("description", ""))

            # 证据类型
            if "evidence_type" in props:
                type_map = {
                    "physical": unreal.EvidenceType.PHYSICAL,
                    "photo": unreal.EvidenceType.PHOTO,
                    "video": unreal.EvidenceType.VIDEO,
                    "audio": unreal.EvidenceType.AUDIO,
                    "note": unreal.EvidenceType.NOTE,
                    "document": unreal.EvidenceType.DOCUMENT
                }
                metadata.type = type_map.get(props["evidence_type"].lower(), unreal.EvidenceType.PHYSICAL)

            # 证据分类
            if "category" in props:
                category_map = {
                    "clue": unreal.EvidenceCategory.CLUE,
                    "witness": unreal.EvidenceCategory.WITNESS,
                    "location": unreal.EvidenceCategory.LOCATION,
                    "temporal": unreal.EvidenceCategory.TEMPORAL,
                    "personal": unreal.EvidenceCategory.PERSONAL,
                    "environmental": unreal.EvidenceCategory.ENVIRONMENTAL
                }
                metadata.category = category_map.get(props["category"].lower(), unreal.EvidenceCategory.CLUE)

            metadata.importance_level = props.get("importance", 1)
            metadata.is_key_evidence = props.get("is_key", False)

            actor.set_evidence_metadata(metadata)

        # 配置收集行为
        if "destroy_on_collect" in props:
            actor.set_editor_property("destroy_on_collect", props["destroy_on_collect"])
        if "hide_on_collect" in props:
            actor.set_editor_property("hide_on_collect", props["hide_on_collect"])

    def _configure_door(self, actor: unreal.Actor, config: SpawnConfig):
        """配置门特定属性"""
        if not config.custom_properties:
            return

        props = config.custom_properties

        # 门动画配置
        if "open_angle" in props:
            actor.set_editor_property("open_angle", float(props["open_angle"]))
        if "open_speed" in props:
            actor.set_editor_property("open_speed", float(props["open_speed"]))

        # 门行为配置
        if "start_locked" in props:
            actor.set_editor_property("start_locked", props["start_locked"])
        if "auto_close" in props:
            actor.set_editor_property("auto_close", props["auto_close"])
        if "auto_close_delay" in props:
            actor.set_editor_property("auto_close_delay", float(props["auto_close_delay"]))

        # 密码配置
        if "requires_password" in props and props["requires_password"]:
            actor.set_editor_property("requires_password", True)
            if "password" in props:
                password = props["password"]
                hint = props.get("password_hint", "")
                actor.configure_password(password, unreal.Text(hint))

    def _configure_switch(self, actor: unreal.Actor, config: SpawnConfig):
        """配置开关特定属性"""
        if not config.custom_properties:
            return

        props = config.custom_properties

        # 开关类型
        if "switch_type" in props:
            type_map = {
                "toggle": unreal.SwitchType.TOGGLE,
                "momentary": unreal.SwitchType.MOMENTARY,
                "onetime": unreal.SwitchType.ONE_TIME
            }
            switch_type = type_map.get(props["switch_type"].lower(), unreal.SwitchType.TOGGLE)
            actor.set_editor_property("switch_type", switch_type)

        # 开关行为
        if "start_on" in props:
            actor.set_editor_property("start_on", props["start_on"])
        if "momentary_duration" in props:
            actor.set_editor_property("momentary_duration", float(props["momentary_duration"]))

        # 连接的Actor
        if "connected_actors" in props:
            connected = []
            for actor_name in props["connected_actors"]:
                target = unreal.EditorLevelLibrary.get_actor_reference(actor_name)
                if target:
                    connected.append(target)
            if connected:
                actor.set_editor_property("connected_actors", connected)

    def _configure_pickup(self, actor: unreal.Actor, config: SpawnConfig):
        """配置拾取物特定属性"""
        if not config.custom_properties:
            return

        props = config.custom_properties

        # 拾取类型
        if "pickup_type" in props:
            type_map = {
                "evidence": unreal.PickupType.EVIDENCE,
                "key": unreal.PickupType.KEY,
                "tool": unreal.PickupType.TOOL,
                "document": unreal.PickupType.DOCUMENT,
                "battery": unreal.PickupType.BATTERY,
                "generic": unreal.PickupType.GENERIC
            }
            pickup_type = type_map.get(props["pickup_type"].lower(), unreal.PickupType.GENERIC)
            actor.set_editor_property("pickup_type", pickup_type)

        # 物品ID
        if "item_id" in props:
            actor.set_editor_property("item_id", unreal.Name(props["item_id"]))

        # 视觉效果
        if "rotate_in_place" in props:
            actor.set_editor_property("rotate_in_place", props["rotate_in_place"])
        if "rotation_speed" in props:
            actor.set_editor_property("rotation_speed", float(props["rotation_speed"]))
        if "float_in_place" in props:
            actor.set_editor_property("float_in_place", props["float_in_place"])
        if "float_amplitude" in props:
            actor.set_editor_property("float_amplitude", float(props["float_amplitude"]))

        # 拾取行为
        if "destroy_on_pickup" in props:
            actor.set_editor_property("destroy_on_pickup", props["destroy_on_pickup"])

    def load_from_json(self, json_path: str, interactable_type: InteractableType) -> List[unreal.Actor]:
        """从JSON文件加载配置并批量生成

        Args:
            json_path: JSON配置文件路径
            interactable_type: 交互物体类型

        Returns:
            生成的Actor列表
        """
        try:
            with open(json_path, 'r', encoding='utf-8') as f:
                data = json.load(f)

            configs = []
            for item in data.get("interactables", []):
                config = SpawnConfig(
                    location=tuple(item["location"]),
                    rotation=tuple(item.get("rotation", [0, 0, 0])),
                    interactable_id=item.get("id", ""),
                    interaction_prompt=item.get("prompt", "互动"),
                    custom_properties=item.get("properties", {})
                )
                configs.append(config)

            # 根据类型生成
            if interactable_type == InteractableType.EVIDENCE:
                return self.spawn_evidence_batch(configs)
            elif interactable_type == InteractableType.DOOR:
                return self.spawn_doors_batch(configs)
            elif interactable_type == InteractableType.SWITCH:
                return self.spawn_switches_batch(configs)
            elif interactable_type == InteractableType.PICKUP:
                return self.spawn_pickups_batch(configs)

        except Exception as e:
            unreal.log_error(f"加载JSON配置失败: {e}")
            return []

    def clear_spawned_actors(self):
        """清除所有已生成的Actor"""
        for actor in self.spawned_actors:
            if actor:
                self.editor_actor_subsystem.destroy_actor(actor)
        self.spawned_actors.clear()
        unreal.log("已清除所有生成的交互物体")


# 使用示例
def example_spawn_evidence():
    """示例：批量生成证据物品"""
    spawner = InteractableSpawner()

    configs = [
        SpawnConfig(
            location=(1000, 0, 100),
            interactable_id="evidence_note_01",
            interaction_prompt="检查笔记",
            custom_properties={
                "evidence_id": "note_01",
                "display_name": "神秘笔记",
                "description": "一张沾满血迹的笔记",
                "evidence_type": "note",
                "category": "clue",
                "importance": 3,
                "is_key": True
            }
        ),
        SpawnConfig(
            location=(1500, 0, 100),
            interactable_id="evidence_photo_01",
            interaction_prompt="拾取照片",
            custom_properties={
                "evidence_id": "photo_01",
                "display_name": "旧照片",
                "description": "一张褪色的家庭照片",
                "evidence_type": "photo",
                "category": "personal",
                "importance": 2
            }
        )
    ]

    return spawner.spawn_evidence_batch(configs)


def example_spawn_doors():
    """示例：批量生成门"""
    spawner = InteractableSpawner()

    configs = [
        SpawnConfig(
            location=(2000, 0, 0),
            rotation=(0, 90, 0),
            interactable_id="door_main_01",
            interaction_prompt="开门",
            custom_properties={
                "open_angle": 90.0,
                "open_speed": 2.0,
                "auto_close": True,
                "auto_close_delay": 5.0
            }
        ),
        SpawnConfig(
            location=(3000, 0, 0),
            rotation=(0, 90, 0),
            interactable_id="door_locked_01",
            interaction_prompt="需要密码",
            custom_properties={
                "start_locked": True,
                "requires_password": True,
                "password": "1984",
                "password_hint": "书架上的年份"
            }
        )
    ]

    return spawner.spawn_doors_batch(configs)


def example_spawn_switches():
    """示例：批量生成开关"""
    spawner = InteractableSpawner()

    configs = [
        SpawnConfig(
            location=(1000, 500, 150),
            interactable_id="switch_light_01",
            interaction_prompt="开关灯",
            custom_properties={
                "switch_type": "toggle",
                "start_on": False
            }
        ),
        SpawnConfig(
            location=(1000, 1000, 150),
            interactable_id="switch_door_01",
            interaction_prompt="按下按钮",
            custom_properties={
                "switch_type": "momentary",
                "momentary_duration": 3.0,
                "connected_actors": ["door_main_01"]
            }
        )
    ]

    return spawner.spawn_switches_batch(configs)


def example_load_from_json():
    """示例：从JSON文件加载配置"""
    spawner = InteractableSpawner()

    # JSON格式示例：
    # {
    #   "interactables": [
    #     {
    #       "location": [1000, 0, 100],
    #       "rotation": [0, 0, 0],
    #       "id": "evidence_01",
    #       "prompt": "检查证据",
    #       "properties": {
    #         "evidence_id": "clue_01",
    #         "display_name": "血迹",
    #         "evidence_type": "physical"
    #       }
    #     }
    #   ]
    # }

    project_root = Path(__file__).resolve().parents[2]
    json_path = project_root / "Content" / "LevelDesign" / "EvidenceSpawns.json"
    return spawner.load_from_json(str(json_path), InteractableType.EVIDENCE)


if __name__ == "__main__":
    unreal.log("InteractableSpawner工具已加载")
    unreal.log("使用示例：")
    unreal.log("  spawner = InteractableSpawner()")
    unreal.log("  spawner.spawn_evidence_batch(configs)")
    unreal.log("  spawner.spawn_doors_batch(configs)")
    unreal.log("  spawner.load_from_json(path, InteractableType.EVIDENCE)")
