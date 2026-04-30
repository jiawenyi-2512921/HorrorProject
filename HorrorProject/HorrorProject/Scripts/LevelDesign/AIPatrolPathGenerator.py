"""
AIPatrolPathGenerator.py - AI巡逻路径生成工具

为HorrorThreatAIController创建巡逻路径：
- 使用Spline组件创建平滑路径
- 使用Target Points创建离散巡逻点
- 配置石头魔像AI行为参数

与HorrorProject AI系统集成。
"""

import unreal
import math
from typing import List, Tuple, Optional
from dataclasses import dataclass
from enum import Enum


class PathType(Enum):
    """路径类型"""
    SPLINE = "Spline"  # 平滑样条路径
    TARGET_POINTS = "TargetPoints"  # 离散目标点
    CIRCULAR = "Circular"  # 圆形路径
    RECTANGULAR = "Rectangular"  # 矩形路径


@dataclass
class PatrolPoint:
    """巡逻点配置"""
    location: Tuple[float, float, float]
    wait_time: float = 2.0
    rotation: Optional[Tuple[float, float, float]] = None


@dataclass
class PatrolPathConfig:
    """巡逻路径配置"""
    path_name: str
    path_type: PathType
    points: List[PatrolPoint]
    loop: bool = True
    reverse_on_end: bool = False


class AIPatrolPathGenerator:
    """AI巡逻路径生成器"""

    def __init__(self, level: unreal.Level = None):
        self.level = level or unreal.EditorLevelLibrary.get_editor_world()
        self.editor_actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
        self.generated_paths: List[unreal.Actor] = []

    def create_spline_path(self, config: PatrolPathConfig) -> Optional[unreal.Actor]:
        """创建样条路径

        Args:
            config: 路径配置

        Returns:
            生成的Spline Actor
        """
        # 创建空Actor作为路径容器
        actor_class = unreal.load_class(None, "/Script/Engine.Actor")
        path_actor = self.editor_actor_subsystem.spawn_actor_from_class(
            actor_class,
            unreal.Vector(0, 0, 0),
            unreal.Rotator(0, 0, 0)
        )

        if not path_actor:
            unreal.log_error("无法创建路径Actor")
            return None

        # 设置Actor名称
        path_actor.set_actor_label(config.path_name)

        # 添加Spline组件
        spline_comp = path_actor.add_component_by_class(
            unreal.SplineComponent,
            manual_attachment=False,
            relative_transform=unreal.Transform()
        )

        if not spline_comp:
            unreal.log_error("无法添加Spline组件")
            self.editor_actor_subsystem.destroy_actor(path_actor)
            return None

        # 清除默认点
        spline_comp.clear_spline_points()

        # 添加巡逻点
        for i, point in enumerate(config.points):
            location = unreal.Vector(point.location[0], point.location[1], point.location[2])
            spline_comp.add_spline_point(
                location,
                unreal.SplineCoordinateSpace.WORLD,
                update_spline=False
            )

        # 设置循环
        spline_comp.set_closed_loop(config.loop, update_spline=True)

        # 设置样条类型为平滑曲线
        for i in range(spline_comp.get_number_of_spline_points()):
            spline_comp.set_spline_point_type(i, unreal.SplinePointType.CURVE)

        # 添加标签用于识别
        path_actor.tags.append("AIPatrolPath")
        path_actor.tags.append("SplinePath")

        self.generated_paths.append(path_actor)
        unreal.log(f"成功创建样条路径: {config.path_name}")
        return path_actor

    def create_target_points_path(self, config: PatrolPathConfig) -> List[unreal.Actor]:
        """创建目标点路径

        Args:
            config: 路径配置

        Returns:
            生成的Target Point列表
        """
        target_point_class = unreal.load_class(None, "/Script/Engine.TargetPoint")
        if not target_point_class:
            unreal.log_error("无法加载TargetPoint类")
            return []

        target_points = []

        for i, point in enumerate(config.points):
            location = unreal.Vector(point.location[0], point.location[1], point.location[2])
            rotation = unreal.Rotator(0, 0, 0)
            if point.rotation:
                rotation = unreal.Rotator(point.rotation[0], point.rotation[1], point.rotation[2])

            target = self.editor_actor_subsystem.spawn_actor_from_class(
                target_point_class,
                location,
                rotation
            )

            if target:
                # 设置名称
                target.set_actor_label(f"{config.path_name}_Point_{i:02d}")

                # 添加标签
                target.tags.append("AIPatrolPoint")
                target.tags.append(config.path_name)

                # 存储等待时间（通过Actor标签）
                target.tags.append(f"WaitTime_{point.wait_time}")

                target_points.append(target)

        self.generated_paths.extend(target_points)
        unreal.log(f"成功创建目标点路径: {config.path_name}，共 {len(target_points)} 个点")
        return target_points

    def create_circular_path(
        self,
        center: Tuple[float, float, float],
        radius: float,
        num_points: int,
        path_name: str,
        height_variation: float = 0.0,
        use_spline: bool = True
    ) -> Optional[unreal.Actor]:
        """创建圆形巡逻路径

        Args:
            center: 圆心位置
            radius: 半径
            num_points: 点数量
            path_name: 路径名称
            height_variation: 高度变化（正弦波幅度）
            use_spline: 是否使用样条（否则使用目标点）

        Returns:
            生成的路径Actor或Target Points列表
        """
        points = []
        for i in range(num_points):
            angle = (2 * math.pi * i) / num_points
            x = center[0] + radius * math.cos(angle)
            y = center[1] + radius * math.sin(angle)
            z = center[2]

            # 添加高度变化
            if height_variation > 0:
                z += height_variation * math.sin(angle * 2)

            points.append(PatrolPoint(location=(x, y, z)))

        config = PatrolPathConfig(
            path_name=path_name,
            path_type=PathType.SPLINE if use_spline else PathType.TARGET_POINTS,
            points=points,
            loop=True
        )

        if use_spline:
            return self.create_spline_path(config)
        else:
            return self.create_target_points_path(config)

    def create_rectangular_path(
        self,
        corner1: Tuple[float, float, float],
        corner2: Tuple[float, float, float],
        points_per_side: int,
        path_name: str,
        use_spline: bool = True
    ) -> Optional[unreal.Actor]:
        """创建矩形巡逻路径

        Args:
            corner1: 第一个角点
            corner2: 对角点
            points_per_side: 每边点数
            path_name: 路径名称
            use_spline: 是否使用样条

        Returns:
            生成的路径Actor
        """
        x1, y1, z = corner1
        x2, y2, _ = corner2

        points = []

        # 顶边
        for i in range(points_per_side):
            t = i / points_per_side
            x = x1 + (x2 - x1) * t
            points.append(PatrolPoint(location=(x, y1, z)))

        # 右边
        for i in range(points_per_side):
            t = i / points_per_side
            y = y1 + (y2 - y1) * t
            points.append(PatrolPoint(location=(x2, y, z)))

        # 底边
        for i in range(points_per_side):
            t = i / points_per_side
            x = x2 - (x2 - x1) * t
            points.append(PatrolPoint(location=(x, y2, z)))

        # 左边
        for i in range(points_per_side):
            t = i / points_per_side
            y = y2 - (y2 - y1) * t
            points.append(PatrolPoint(location=(x1, y, z)))

        config = PatrolPathConfig(
            path_name=path_name,
            path_type=PathType.SPLINE if use_spline else PathType.TARGET_POINTS,
            points=points,
            loop=True
        )

        if use_spline:
            return self.create_spline_path(config)
        else:
            return self.create_target_points_path(config)

    def assign_path_to_ai(
        self,
        ai_controller_actor: unreal.Actor,
        path_actor: unreal.Actor
    ) -> bool:
        """将路径分配给AI控制器

        Args:
            ai_controller_actor: AI控制器Actor
            path_actor: 路径Actor

        Returns:
            是否成功
        """
        if not ai_controller_actor or not path_actor:
            unreal.log_error("AI控制器或路径Actor无效")
            return False

        # 通过标签关联
        ai_controller_actor.tags.append(f"PatrolPath_{path_actor.get_actor_label()}")

        unreal.log(f"已将路径 {path_actor.get_actor_label()} 分配给 {ai_controller_actor.get_actor_label()}")
        return True

    def configure_golem_behavior(
        self,
        threat_actor: unreal.Actor,
        patrol_speed: float = 200.0,
        chase_speed: float = 400.0,
        detection_radius: float = 1500.0,
        attack_range: float = 200.0
    ) -> bool:
        """配置石头魔像AI行为参数

        Args:
            threat_actor: 威胁角色Actor
            patrol_speed: 巡逻速度
            chase_speed: 追逐速度
            detection_radius: 检测半径
            attack_range: 攻击范围

        Returns:
            是否成功
        """
        if not threat_actor:
            unreal.log_error("威胁Actor无效")
            return False

        try:
            # 设置移动速度
            threat_actor.set_editor_property("patrol_speed", patrol_speed)
            threat_actor.set_editor_property("chase_speed", chase_speed)

            # 设置检测参数
            threat_actor.set_editor_property("detection_radius", detection_radius)
            threat_actor.set_editor_property("attack_range", attack_range)

            # 添加行为标签
            threat_actor.tags.append("GolemAI")
            threat_actor.tags.append(f"DetectionRadius_{detection_radius}")

            unreal.log(f"已配置魔像AI行为: {threat_actor.get_actor_label()}")
            return True

        except Exception as e:
            unreal.log_error(f"配置魔像行为失败: {e}")
            return False

    def visualize_path(self, path_actor: unreal.Actor, color: Tuple[float, float, float] = (1, 0, 0)):
        """可视化路径（添加调试球体）

        Args:
            path_actor: 路径Actor
            color: 可视化颜色
        """
        if not path_actor:
            return

        # 检查是否是Spline路径
        spline_comp = path_actor.get_component_by_class(unreal.SplineComponent)
        if spline_comp:
            num_points = spline_comp.get_number_of_spline_points()
            for i in range(num_points):
                location = spline_comp.get_location_at_spline_point(i, unreal.SplineCoordinateSpace.WORLD)
                # 在编辑器中绘制调试球体
                unreal.SystemLibrary.draw_debug_sphere(
                    path_actor,
                    location,
                    50.0,
                    12,
                    unreal.LinearColor(color[0], color[1], color[2], 1.0),
                    10000.0,
                    2.0
                )

    def clear_generated_paths(self):
        """清除所有生成的路径"""
        for path in self.generated_paths:
            if path:
                self.editor_actor_subsystem.destroy_actor(path)
        self.generated_paths.clear()
        unreal.log("已清除所有生成的巡逻路径")


# 使用示例
def example_create_spline_patrol():
    """示例：创建样条巡逻路径"""
    generator = AIPatrolPathGenerator()

    config = PatrolPathConfig(
        path_name="GolemPatrol_Hallway",
        path_type=PathType.SPLINE,
        points=[
            PatrolPoint(location=(1000, 0, 100), wait_time=3.0),
            PatrolPoint(location=(2000, 500, 100), wait_time=2.0),
            PatrolPoint(location=(3000, 0, 100), wait_time=3.0),
            PatrolPoint(location=(2000, -500, 100), wait_time=2.0)
        ],
        loop=True
    )

    return generator.create_spline_path(config)


def example_create_target_points_patrol():
    """示例：创建目标点巡逻路径"""
    generator = AIPatrolPathGenerator()

    config = PatrolPathConfig(
        path_name="GolemPatrol_Room",
        path_type=PathType.TARGET_POINTS,
        points=[
            PatrolPoint(location=(500, 500, 100), wait_time=5.0, rotation=(0, 90, 0)),
            PatrolPoint(location=(500, -500, 100), wait_time=5.0, rotation=(0, 180, 0)),
            PatrolPoint(location=(-500, -500, 100), wait_time=5.0, rotation=(0, 270, 0)),
            PatrolPoint(location=(-500, 500, 100), wait_time=5.0, rotation=(0, 0, 0))
        ],
        loop=True
    )

    return generator.create_target_points_path(config)


def example_create_circular_patrol():
    """示例：创建圆形巡逻路径"""
    generator = AIPatrolPathGenerator()

    return generator.create_circular_path(
        center=(0, 0, 100),
        radius=1000.0,
        num_points=8,
        path_name="GolemPatrol_Circular",
        height_variation=50.0,
        use_spline=True
    )


def example_create_rectangular_patrol():
    """示例：创建矩形巡逻路径"""
    generator = AIPatrolPathGenerator()

    return generator.create_rectangular_path(
        corner1=(-1000, -1000, 100),
        corner2=(1000, 1000, 100),
        points_per_side=4,
        path_name="GolemPatrol_Perimeter",
        use_spline=True
    )


def example_setup_golem_with_patrol():
    """示例：完整设置魔像AI和巡逻路径"""
    generator = AIPatrolPathGenerator()

    # 1. 创建巡逻路径
    patrol_path = generator.create_circular_path(
        center=(0, 0, 100),
        radius=1500.0,
        num_points=6,
        path_name="GolemPatrol_Main",
        use_spline=True
    )

    # 2. 获取场景中的威胁Actor（假设已存在）
    threat_actor = unreal.EditorLevelLibrary.get_actor_reference("BP_GolemThreat")

    if threat_actor and patrol_path:
        # 3. 配置魔像行为
        generator.configure_golem_behavior(
            threat_actor=threat_actor,
            patrol_speed=200.0,
            chase_speed=500.0,
            detection_radius=2000.0,
            attack_range=250.0
        )

        # 4. 分配巡逻路径
        generator.assign_path_to_ai(threat_actor, patrol_path)

        # 5. 可视化路径
        generator.visualize_path(patrol_path, color=(1, 0, 0))

        unreal.log("魔像AI和巡逻路径设置完成")
        return threat_actor, patrol_path

    return None, None


if __name__ == "__main__":
    unreal.log("AIPatrolPathGenerator工具已加载")
    unreal.log("使用示例：")
    unreal.log("  generator = AIPatrolPathGenerator()")
    unreal.log("  generator.create_spline_path(config)")
    unreal.log("  generator.create_circular_path(center, radius, num_points, name)")
    unreal.log("  generator.configure_golem_behavior(threat_actor, ...)")
