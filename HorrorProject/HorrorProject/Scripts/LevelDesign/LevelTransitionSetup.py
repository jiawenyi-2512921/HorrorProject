#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
关卡传送系统配置工具

功能：
1. 批量创建传送触发器配置
2. 配置地图传送链
3. 验证传送点配置
4. 生成关卡流程图

使用方法：
    python LevelTransitionSetup.py --config ../Config/LevelFlow.json
    python LevelTransitionSetup.py --validate
    python LevelTransitionSetup.py --generate-graph
"""

import json
import os
import sys
from pathlib import Path
from typing import Dict, List, Optional, Tuple
from dataclasses import dataclass, asdict
from enum import Enum

# 设置控制台编码为UTF-8（Windows兼容）
if sys.platform == 'win32':
    try:
        import io
        sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')
        sys.stderr = io.TextIOWrapper(sys.stderr.buffer, encoding='utf-8')
    except:
        pass


class TriggerType(Enum):
    """触发器类型"""
    ON_OVERLAP = "OnOverlap"
    ON_OBJECTIVE_COMPLETE = "OnObjectiveComplete"
    MANUAL = "Manual"


class AnimationType(Enum):
    """过渡动画类型"""
    FADE_TO_BLACK = "FadeToBlack"
    WHITE_FLASH = "WhiteFlash"
    STATIC_NOISE = "StaticNoise"
    GLITCH = "Glitch"
    CUSTOM = "Custom"


@dataclass
class SpawnPoint:
    """出生点配置"""
    tag: str
    description: str
    location: Tuple[float, float, float]
    rotation: Tuple[float, float, float]
    camera_fade_in: bool = True
    fade_duration: float = 1.0


@dataclass
class TransitionTrigger:
    """传送触发器配置"""
    name: str
    trigger_type: str
    target_level: str
    spawn_point_tag: str
    location: Tuple[float, float, float]
    trigger_volume_extent: Tuple[float, float, float]
    required_objectives: List[str]
    required_tags: List[str]
    animation_type: str
    transition_delay: float
    only_trigger_once: bool


@dataclass
class LevelObjective:
    """关卡目标配置"""
    node_id: str
    title: str
    description: str
    objective_type: str
    required_for_transition: bool


@dataclass
class LevelConfig:
    """关卡配置"""
    level_name: str
    display_name: str
    description: str
    objectives: List[LevelObjective]
    spawn_points: List[SpawnPoint]
    transitions: List[TransitionTrigger]


class LevelTransitionSetup:
    """关卡传送系统配置工具"""

    def __init__(self, config_path: str):
        self.config_path = Path(config_path)
        self.config_data: Dict = {}
        self.levels: Dict[str, LevelConfig] = {}

    def load_config(self) -> bool:
        """加载配置文件"""
        try:
            if not self.config_path.exists():
                print(f"❌ 配置文件不存在: {self.config_path}")
                return False

            with open(self.config_path, 'r', encoding='utf-8') as f:
                self.config_data = json.load(f)

            print(f"✅ 成功加载配置: {self.config_path}")
            return True
        except Exception as e:
            print(f"❌ 加载配置失败: {e}")
            return False

    def parse_levels(self) -> bool:
        """解析关卡配置"""
        try:
            for level_data in self.config_data.get('levels', []):
                # 解析目标
                objectives = [
                    LevelObjective(**obj) for obj in level_data.get('objectives', [])
                ]

                # 解析出生点
                spawn_points = [
                    SpawnPoint(**sp) for sp in level_data.get('spawn_points', [])
                ]

                # 解析传送触发器
                transitions = [
                    TransitionTrigger(**tr) for tr in level_data.get('transitions', [])
                ]

                # 创建关卡配置
                level_config = LevelConfig(
                    level_name=level_data['level_name'],
                    display_name=level_data['display_name'],
                    description=level_data['description'],
                    objectives=objectives,
                    spawn_points=spawn_points,
                    transitions=transitions
                )

                self.levels[level_config.level_name] = level_config

            print(f"✅ 成功解析 {len(self.levels)} 个关卡配置")
            return True
        except Exception as e:
            print(f"❌ 解析关卡配置失败: {e}")
            return False

    def validate_config(self) -> bool:
        """验证配置完整性"""
        print("\n🔍 开始验证配置...")
        errors = []
        warnings = []

        # 验证每个关卡
        for level_name, level_config in self.levels.items():
            print(f"\n检查关卡: {level_name}")

            # 验证出生点
            if not level_config.spawn_points:
                errors.append(f"{level_name}: 没有配置出生点")
            else:
                spawn_tags = [sp.tag for sp in level_config.spawn_points]
                print(f"  ✓ 出生点: {', '.join(spawn_tags)}")

            # 验证传送触发器
            for trigger in level_config.transitions:
                # 检查目标关卡是否存在
                if trigger.target_level not in self.levels:
                    errors.append(
                        f"{level_name}: 传送触发器 '{trigger.name}' "
                        f"的目标关卡 '{trigger.target_level}' 不存在"
                    )
                else:
                    # 检查目标出生点是否存在
                    target_level = self.levels[trigger.target_level]
                    target_spawn_tags = [sp.tag for sp in target_level.spawn_points]

                    if trigger.spawn_point_tag not in target_spawn_tags:
                        errors.append(
                            f"{level_name}: 传送触发器 '{trigger.name}' "
                            f"的目标出生点 '{trigger.spawn_point_tag}' "
                            f"在关卡 '{trigger.target_level}' 中不存在"
                        )

                # 检查必需目标是否存在
                level_objective_ids = [obj.node_id for obj in level_config.objectives]
                for req_obj in trigger.required_objectives:
                    if req_obj not in level_objective_ids:
                        warnings.append(
                            f"{level_name}: 传送触发器 '{trigger.name}' "
                            f"需要的目标 '{req_obj}' 在当前关卡中未定义"
                        )

            # 验证目标配置
            if not level_config.objectives:
                warnings.append(f"{level_name}: 没有配置目标")
            else:
                print(f"  ✓ 目标数量: {len(level_config.objectives)}")

        # 输出验证结果
        print("\n" + "="*60)
        if errors:
            print(f"\n❌ 发现 {len(errors)} 个错误:")
            for error in errors:
                print(f"  • {error}")

        if warnings:
            print(f"\n⚠️  发现 {len(warnings)} 个警告:")
            for warning in warnings:
                print(f"  • {warning}")

        if not errors and not warnings:
            print("\n✅ 配置验证通过，没有发现问题！")
        elif not errors:
            print("\n✅ 配置验证通过（有警告）")

        return len(errors) == 0

    def generate_unreal_config(self, output_path: str) -> bool:
        """生成Unreal Engine配置文件"""
        try:
            output_file = Path(output_path)
            output_file.parent.mkdir(parents=True, exist_ok=True)

            # 生成配置数据
            unreal_config = {
                "LevelTransitionSystem": {
                    "Levels": []
                }
            }

            for level_name, level_config in self.levels.items():
                level_data = {
                    "LevelName": level_config.level_name,
                    "DisplayName": level_config.display_name,
                    "SpawnPoints": [
                        {
                            "Tag": sp.tag,
                            "Location": {"X": sp.location[0], "Y": sp.location[1], "Z": sp.location[2]},
                            "Rotation": {"Pitch": sp.rotation[0], "Roll": sp.rotation[1], "Yaw": sp.rotation[2]}
                        }
                        for sp in level_config.spawn_points
                    ],
                    "Transitions": [
                        {
                            "Name": tr.name,
                            "TargetLevel": tr.target_level,
                            "SpawnPointTag": tr.spawn_point_tag,
                            "RequiredObjectives": tr.required_objectives
                        }
                        for tr in level_config.transitions
                    ]
                }
                unreal_config["LevelTransitionSystem"]["Levels"].append(level_data)

            # 写入文件
            with open(output_file, 'w', encoding='utf-8') as f:
                json.dump(unreal_config, f, indent=2, ensure_ascii=False)

            print(f"✅ 成功生成Unreal配置: {output_file}")
            return True
        except Exception as e:
            print(f"❌ 生成Unreal配置失败: {e}")
            return False

    def generate_flow_graph(self, output_path: str) -> bool:
        """生成关卡流程图（Mermaid格式）"""
        try:
            output_file = Path(output_path)
            output_file.parent.mkdir(parents=True, exist_ok=True)

            # 生成Mermaid图表
            lines = [
                "# 关卡流程图",
                "",
                "```mermaid",
                "graph TD"
            ]

            # 添加关卡节点
            for level_name, level_config in self.levels.items():
                node_id = level_name.replace("_", "")
                objectives_text = "<br/>".join([
                    f"• {obj.title}" for obj in level_config.objectives[:3]
                ])
                lines.append(
                    f'    {node_id}["{level_config.display_name}<br/>'
                    f'{objectives_text}"]'
                )

            lines.append("")

            # 添加传送连接
            for level_name, level_config in self.levels.items():
                source_id = level_name.replace("_", "")
                for trigger in level_config.transitions:
                    target_id = trigger.target_level.replace("_", "")
                    req_text = ", ".join(trigger.required_objectives) if trigger.required_objectives else "无条件"
                    lines.append(
                        f'    {source_id} -->|"{trigger.name}<br/>{req_text}"| {target_id}'
                    )

            lines.append("```")
            lines.append("")

            # 添加详细信息
            lines.append("## 关卡详情")
            lines.append("")

            for level_name, level_config in self.levels.items():
                lines.append(f"### {level_config.display_name}")
                lines.append(f"**关卡名称**: `{level_config.level_name}`")
                lines.append(f"**描述**: {level_config.description}")
                lines.append("")

                lines.append("**目标**:")
                for obj in level_config.objectives:
                    req_mark = "🔑" if obj.required_for_transition else "📋"
                    lines.append(f"- {req_mark} {obj.title}: {obj.description}")
                lines.append("")

                lines.append("**出生点**:")
                for sp in level_config.spawn_points:
                    lines.append(f"- `{sp.tag}`: {sp.description}")
                lines.append("")

                if level_config.transitions:
                    lines.append("**传送点**:")
                    for tr in level_config.transitions:
                        lines.append(f"- {tr.name} → {tr.target_level}")
                        if tr.required_objectives:
                            lines.append(f"  - 需要完成: {', '.join(tr.required_objectives)}")
                    lines.append("")

            # 写入文件
            with open(output_file, 'w', encoding='utf-8') as f:
                f.write('\n'.join(lines))

            print(f"✅ 成功生成流程图: {output_file}")
            return True
        except Exception as e:
            print(f"❌ 生成流程图失败: {e}")
            return False

    def generate_blueprint_data(self, output_path: str) -> bool:
        """生成蓝图数据表CSV"""
        try:
            output_file = Path(output_path)
            output_file.parent.mkdir(parents=True, exist_ok=True)

            # 生成传送触发器数据表
            trigger_csv_lines = [
                "Name,TriggerType,TargetLevel,SpawnPointTag,LocationX,LocationY,LocationZ,"
                "ExtentX,ExtentY,ExtentZ,RequiredObjectives,AnimationType,TransitionDelay"
            ]

            for level_name, level_config in self.levels.items():
                for trigger in level_config.transitions:
                    trigger_csv_lines.append(
                        f"{trigger.name},"
                        f"{trigger.trigger_type},"
                        f"{trigger.target_level},"
                        f"{trigger.spawn_point_tag},"
                        f"{trigger.location[0]},{trigger.location[1]},{trigger.location[2]},"
                        f"{trigger.trigger_volume_extent[0]},{trigger.trigger_volume_extent[1]},{trigger.trigger_volume_extent[2]},"
                        f"\"{';'.join(trigger.required_objectives)}\","
                        f"{trigger.animation_type},"
                        f"{trigger.transition_delay}"
                    )

            trigger_file = output_file.parent / "TransitionTriggers.csv"
            with open(trigger_file, 'w', encoding='utf-8') as f:
                f.write('\n'.join(trigger_csv_lines))

            print(f"✅ 成功生成触发器数据表: {trigger_file}")

            # 生成出生点数据表
            spawn_csv_lines = [
                "LevelName,Tag,Description,LocationX,LocationY,LocationZ,"
                "Pitch,Roll,Yaw,CameraFadeIn,FadeDuration"
            ]

            for level_name, level_config in self.levels.items():
                for sp in level_config.spawn_points:
                    spawn_csv_lines.append(
                        f"{level_name},"
                        f"{sp.tag},"
                        f"\"{sp.description}\","
                        f"{sp.location[0]},{sp.location[1]},{sp.location[2]},"
                        f"{sp.rotation[0]},{sp.rotation[1]},{sp.rotation[2]},"
                        f"{sp.camera_fade_in},"
                        f"{sp.fade_duration}"
                    )

            spawn_file = output_file.parent / "SpawnPoints.csv"
            with open(spawn_file, 'w', encoding='utf-8') as f:
                f.write('\n'.join(spawn_csv_lines))

            print(f"✅ 成功生成出生点数据表: {spawn_file}")
            return True
        except Exception as e:
            print(f"❌ 生成蓝图数据表失败: {e}")
            return False

    def create_template_config(self, output_path: str) -> bool:
        """创建配置模板"""
        try:
            template = {
                "version": "1.0",
                "project": "HorrorProject",
                "levels": [
                    {
                        "level_name": "Level_01_ForestPath",
                        "display_name": "森林小径",
                        "description": "游戏开始的森林区域",
                        "objectives": [
                            {
                                "node_id": "FindCabin",
                                "title": "找到小屋",
                                "description": "在森林中找到废弃的小屋",
                                "objective_type": "Navigation",
                                "required_for_transition": True
                            },
                            {
                                "node_id": "InvestigateNoise",
                                "title": "调查声音",
                                "description": "调查小屋内的奇怪声音",
                                "objective_type": "Interaction",
                                "required_for_transition": True
                            }
                        ],
                        "spawn_points": [
                            {
                                "tag": "ForestStart",
                                "description": "森林入口",
                                "location": [0.0, 0.0, 100.0],
                                "rotation": [0.0, 0.0, 0.0],
                                "camera_fade_in": True,
                                "fade_duration": 1.5
                            }
                        ],
                        "transitions": [
                            {
                                "name": "Trigger_Forest_To_Hospital",
                                "trigger_type": "OnObjectiveComplete",
                                "target_level": "Level_02_Hospital",
                                "spawn_point_tag": "HospitalMainEntrance",
                                "location": [1000.0, 500.0, 100.0],
                                "trigger_volume_extent": [100.0, 100.0, 200.0],
                                "required_objectives": ["FindCabin", "InvestigateNoise"],
                                "required_tags": [],
                                "animation_type": "FadeToBlack",
                                "transition_delay": 1.0,
                                "only_trigger_once": True
                            }
                        ]
                    },
                    {
                        "level_name": "Level_02_Hospital",
                        "display_name": "废弃医院",
                        "description": "阴森的废弃医院",
                        "objectives": [
                            {
                                "node_id": "FindMedicalRecords",
                                "title": "找到病历",
                                "description": "在医院中找到关键病历",
                                "objective_type": "Interaction",
                                "required_for_transition": True
                            }
                        ],
                        "spawn_points": [
                            {
                                "tag": "HospitalMainEntrance",
                                "description": "医院主入口",
                                "location": [0.0, 0.0, 100.0],
                                "rotation": [0.0, 0.0, 90.0],
                                "camera_fade_in": True,
                                "fade_duration": 2.0
                            }
                        ],
                        "transitions": []
                    }
                ]
            }

            output_file = Path(output_path)
            output_file.parent.mkdir(parents=True, exist_ok=True)

            with open(output_file, 'w', encoding='utf-8') as f:
                json.dump(template, f, indent=2, ensure_ascii=False)

            print(f"✅ 成功创建配置模板: {output_file}")
            return True
        except Exception as e:
            print(f"❌ 创建配置模板失败: {e}")
            return False


def main():
    """主函数"""
    import argparse

    parser = argparse.ArgumentParser(
        description='关卡传送系统配置工具',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
示例:
  创建配置模板:
    python LevelTransitionSetup.py --create-template Config/LevelFlow.json

  验证配置:
    python LevelTransitionSetup.py --config Config/LevelFlow.json --validate

  生成流程图:
    python LevelTransitionSetup.py --config Config/LevelFlow.json --generate-graph Docs/LevelFlow.md

  生成所有输出:
    python LevelTransitionSetup.py --config Config/LevelFlow.json --all
        """
    )

    parser.add_argument('--config', type=str, help='配置文件路径')
    parser.add_argument('--create-template', type=str, help='创建配置模板')
    parser.add_argument('--validate', action='store_true', help='验证配置')
    parser.add_argument('--generate-graph', type=str, help='生成流程图')
    parser.add_argument('--generate-unreal', type=str, help='生成Unreal配置')
    parser.add_argument('--generate-blueprint', type=str, help='生成蓝图数据表')
    parser.add_argument('--all', action='store_true', help='生成所有输出')

    args = parser.parse_args()

    # 创建模板
    if args.create_template:
        setup = LevelTransitionSetup("")
        if setup.create_template_config(args.create_template):
            print("\n✅ 配置模板创建完成！")
            print(f"请编辑 {args.create_template} 并运行验证命令")
        return

    # 需要配置文件的操作
    if not args.config:
        parser.print_help()
        return

    setup = LevelTransitionSetup(args.config)

    # 加载配置
    if not setup.load_config():
        return

    # 解析配置
    if not setup.parse_levels():
        return

    # 验证配置
    if args.validate or args.all:
        if not setup.validate_config():
            print("\n❌ 配置验证失败，请修复错误后重试")
            return

    # 生成流程图
    if args.generate_graph or args.all:
        graph_path = args.generate_graph or "Docs/LevelFlow.md"
        setup.generate_flow_graph(graph_path)

    # 生成Unreal配置
    if args.generate_unreal or args.all:
        unreal_path = args.generate_unreal or "Config/LevelTransition.json"
        setup.generate_unreal_config(unreal_path)

    # 生成蓝图数据表
    if args.generate_blueprint or args.all:
        blueprint_path = args.generate_blueprint or "Config/BlueprintData.csv"
        setup.generate_blueprint_data(blueprint_path)

    print("\n✅ 所有操作完成！")


if __name__ == "__main__":
    main()
