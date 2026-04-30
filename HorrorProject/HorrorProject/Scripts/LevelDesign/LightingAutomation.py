"""
LightingAutomation.py - 自动灯光布置工具
用于HorrorProject的恐怖氛围灯光自动化布置

功能：
- 沿走廊自动放置点光源
- 批量调整灯光强度、颜色、衰减
- 创建闪烁灯光效果
- 恐怖氛围灯光预设（冷色调、低强度）

使用方法：
1. 在Unreal Editor中打开Python控制台（Window > Developer Tools > Python）
2. 运行: import LightingAutomation
3. 使用预设: LightingAutomation.apply_horror_corridor_lighting()

兼容性：Unreal Engine 5.6
"""

import unreal
import math

# ============================================================================
# 灯光预设配置
# ============================================================================

class HorrorLightingPresets:
    """恐怖氛围灯光预设"""

    # 冷色调低强度（默认恐怖氛围）
    COLD_DIM = {
        'color': unreal.LinearColor(0.6, 0.7, 1.0, 1.0),  # 冷蓝色
        'intensity': 500.0,
        'attenuation_radius': 800.0,
        'use_inverse_squared_falloff': True
    }

    # 暖色调闪烁（故障灯光）
    WARM_FLICKER = {
        'color': unreal.LinearColor(1.0, 0.8, 0.5, 1.0),  # 暖黄色
        'intensity': 800.0,
        'attenuation_radius': 1000.0,
        'use_inverse_squared_falloff': True
    }

    # 红色警报
    RED_ALERT = {
        'color': unreal.LinearColor(1.0, 0.1, 0.1, 1.0),  # 深红色
        'intensity': 1200.0,
        'attenuation_radius': 1500.0,
        'use_inverse_squared_falloff': False
    }

    # 绿色阴森
    GREEN_EERIE = {
        'color': unreal.LinearColor(0.2, 1.0, 0.3, 1.0),  # 阴森绿
        'intensity': 600.0,
        'attenuation_radius': 900.0,
        'use_inverse_squared_falloff': True
    }


# ============================================================================
# 核心灯光操作函数
# ============================================================================

def create_point_light(location, preset_dict=None, light_name="PointLight"):
    """
    创建点光源

    Args:
        location: unreal.Vector - 灯光位置
        preset_dict: dict - 灯光预设参数
        light_name: str - 灯光名称

    Returns:
        unreal.PointLight - 创建的灯光Actor
    """
    if preset_dict is None:
        preset_dict = HorrorLightingPresets.COLD_DIM

    # 获取当前关卡
    editor_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    world = editor_subsystem.get_editor_world()

    # 创建PointLight Actor
    light_actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.PointLight,
        location,
        unreal.Rotator(0, 0, 0)
    )

    if light_actor:
        # 设置灯光名称
        light_actor.set_actor_label(light_name)

        # 获取PointLightComponent
        light_component = light_actor.get_component_by_class(unreal.PointLightComponent)

        if light_component:
            # 应用预设参数
            light_component.set_editor_property('light_color', preset_dict['color'])
            light_component.set_editor_property('intensity', preset_dict['intensity'])
            light_component.set_editor_property('attenuation_radius', preset_dict['attenuation_radius'])
            light_component.set_editor_property('use_inverse_squared_falloff', preset_dict['use_inverse_squared_falloff'])

            # 启用阴影（恐怖氛围需要）
            light_component.set_editor_property('cast_shadows', True)
            light_component.set_editor_property('cast_dynamic_shadows', True)

        unreal.log(f"创建灯光: {light_name} at {location}")
        return light_actor
    else:
        unreal.log_error(f"无法创建灯光: {light_name}")
        return None


def place_lights_along_corridor(start_location, end_location, spacing=400.0, height_offset=250.0, preset_name='COLD_DIM'):
    """
    沿走廊自动放置点光源

    Args:
        start_location: unreal.Vector - 起始位置
        end_location: unreal.Vector - 结束位置
        spacing: float - 灯光间距（厘米）
        height_offset: float - 高度偏移（厘米）
        preset_name: str - 预设名称

    Returns:
        list - 创建的灯光Actor列表
    """
    # 获取预设
    preset_dict = getattr(HorrorLightingPresets, preset_name, HorrorLightingPresets.COLD_DIM)

    # 计算方向和距离
    direction = end_location - start_location
    distance = direction.length()
    direction = direction.normal()

    # 计算需要放置的灯光数量
    num_lights = max(2, int(distance / spacing) + 1)

    created_lights = []

    for i in range(num_lights):
        # 计算当前灯光位置
        t = i / (num_lights - 1) if num_lights > 1 else 0
        current_location = start_location + (direction * distance * t)
        current_location.z += height_offset

        # 创建灯光
        light_name = f"CorridorLight_{i+1}"
        light = create_point_light(current_location, preset_dict, light_name)

        if light:
            created_lights.append(light)

    unreal.log(f"沿走廊创建了 {len(created_lights)} 个灯光")
    return created_lights


def batch_adjust_lights(light_actors, intensity_multiplier=1.0, color_tint=None, attenuation_multiplier=1.0):
    """
    批量调整灯光参数

    Args:
        light_actors: list - 灯光Actor列表
        intensity_multiplier: float - 强度倍数
        color_tint: unreal.LinearColor - 颜色叠加
        attenuation_multiplier: float - 衰减半径倍数
    """
    adjusted_count = 0

    for light_actor in light_actors:
        if not light_actor:
            continue

        light_component = light_actor.get_component_by_class(unreal.PointLightComponent)

        if light_component:
            # 调整强度
            current_intensity = light_component.get_editor_property('intensity')
            light_component.set_editor_property('intensity', current_intensity * intensity_multiplier)

            # 调整颜色
            if color_tint:
                current_color = light_component.get_editor_property('light_color')
                new_color = unreal.LinearColor(
                    current_color.r * color_tint.r,
                    current_color.g * color_tint.g,
                    current_color.b * color_tint.b,
                    1.0
                )
                light_component.set_editor_property('light_color', new_color)

            # 调整衰减
            current_attenuation = light_component.get_editor_property('attenuation_radius')
            light_component.set_editor_property('attenuation_radius', current_attenuation * attenuation_multiplier)

            adjusted_count += 1

    unreal.log(f"批量调整了 {adjusted_count} 个灯光")


def create_flickering_light(location, preset_name='WARM_FLICKER', flicker_intensity_range=(0.3, 1.0)):
    """
    创建闪烁灯光效果（通过Timeline蓝图实现）

    Args:
        location: unreal.Vector - 灯光位置
        preset_name: str - 预设名称
        flicker_intensity_range: tuple - 闪烁强度范围（最小，最大）

    Returns:
        unreal.PointLight - 创建的闪烁灯光

    注意：需要在蓝图中添加Timeline来实现实时闪烁
    这里创建基础灯光，并设置标签以便蓝图识别
    """
    preset_dict = getattr(HorrorLightingPresets, preset_name, HorrorLightingPresets.WARM_FLICKER)

    light = create_point_light(location, preset_dict, "FlickeringLight")

    if light:
        # 添加标签以便蓝图系统识别
        light.tags.append("FlickeringLight")
        light.tags.append(f"FlickerRange_{flicker_intensity_range[0]}_{flicker_intensity_range[1]}")

        unreal.log(f"创建闪烁灯光（需要蓝图Timeline支持）: {light.get_actor_label()}")

    return light


def apply_horror_atmosphere_to_selected():
    """
    对选中的灯光应用恐怖氛围效果
    """
    selected_actors = unreal.EditorLevelLibrary.get_selected_level_actors()

    light_actors = [actor for actor in selected_actors if isinstance(actor, unreal.PointLight)]

    if not light_actors:
        unreal.log_warning("未选中任何PointLight，请先选择灯光")
        return

    # 应用冷色调低强度
    batch_adjust_lights(
        light_actors,
        intensity_multiplier=0.6,
        color_tint=unreal.LinearColor(0.7, 0.8, 1.0, 1.0),
        attenuation_multiplier=0.8
    )

    unreal.log(f"对 {len(light_actors)} 个灯光应用了恐怖氛围效果")


# ============================================================================
# 高级功能
# ============================================================================

def create_room_lighting(room_center, room_size, num_lights=4, preset_name='COLD_DIM', height_offset=200.0):
    """
    为房间创建均匀分布的灯光

    Args:
        room_center: unreal.Vector - 房间中心
        room_size: unreal.Vector - 房间尺寸（X, Y, Z）
        num_lights: int - 灯光数量（2或4）
        preset_name: str - 预设名称
        height_offset: float - 距离天花板的高度偏移

    Returns:
        list - 创建的灯光列表
    """
    preset_dict = getattr(HorrorLightingPresets, preset_name, HorrorLightingPresets.COLD_DIM)
    created_lights = []

    # 计算灯光位置
    half_x = room_size.x / 2 * 0.6  # 留出边缘空间
    half_y = room_size.y / 2 * 0.6
    ceiling_z = room_center.z + room_size.z / 2 - height_offset

    if num_lights == 4:
        positions = [
            unreal.Vector(room_center.x + half_x, room_center.y + half_y, ceiling_z),
            unreal.Vector(room_center.x + half_x, room_center.y - half_y, ceiling_z),
            unreal.Vector(room_center.x - half_x, room_center.y + half_y, ceiling_z),
            unreal.Vector(room_center.x - half_x, room_center.y - half_y, ceiling_z),
        ]
    elif num_lights == 2:
        positions = [
            unreal.Vector(room_center.x + half_x, room_center.y, ceiling_z),
            unreal.Vector(room_center.x - half_x, room_center.y, ceiling_z),
        ]
    else:
        positions = [unreal.Vector(room_center.x, room_center.y, ceiling_z)]

    for i, pos in enumerate(positions):
        light = create_point_light(pos, preset_dict, f"RoomLight_{i+1}")
        if light:
            created_lights.append(light)

    unreal.log(f"为房间创建了 {len(created_lights)} 个灯光")
    return created_lights


def get_all_lights_in_level():
    """
    获取关卡中所有的PointLight

    Returns:
        list - 所有PointLight Actor列表
    """
    all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
    lights = [actor for actor in all_actors if isinstance(actor, unreal.PointLight)]

    unreal.log(f"关卡中共有 {len(lights)} 个PointLight")
    return lights


def create_light_intensity_gradient(light_actors, start_intensity, end_intensity):
    """
    为一组灯光创建强度渐变

    Args:
        light_actors: list - 灯光Actor列表
        start_intensity: float - 起始强度
        end_intensity: float - 结束强度
    """
    num_lights = len(light_actors)

    for i, light_actor in enumerate(light_actors):
        if not light_actor:
            continue

        light_component = light_actor.get_component_by_class(unreal.PointLightComponent)

        if light_component:
            # 计算当前灯光的强度
            t = i / (num_lights - 1) if num_lights > 1 else 0
            intensity = start_intensity + (end_intensity - start_intensity) * t

            light_component.set_editor_property('intensity', intensity)

    unreal.log(f"为 {num_lights} 个灯光创建了强度渐变 ({start_intensity} -> {end_intensity})")


# ============================================================================
# 便捷使用函数
# ============================================================================

def apply_horror_corridor_lighting(start_pos=None, end_pos=None):
    """
    快速应用恐怖走廊灯光（使用默认参数）

    如果不提供参数，将使用选中的两个Actor作为起点和终点
    """
    if start_pos is None or end_pos is None:
        selected_actors = unreal.EditorLevelLibrary.get_selected_level_actors()

        if len(selected_actors) < 2:
            unreal.log_warning("请选择两个Actor作为走廊的起点和终点，或提供start_pos和end_pos参数")
            return None

        start_pos = selected_actors[0].get_actor_location()
        end_pos = selected_actors[1].get_actor_location()

    return place_lights_along_corridor(start_pos, end_pos, spacing=400.0, height_offset=250.0, preset_name='COLD_DIM')


def quick_horror_room_lighting():
    """
    快速为选中的房间创建恐怖氛围灯光

    使用选中的Actor作为房间中心，需要手动指定房间大小
    """
    selected_actors = unreal.EditorLevelLibrary.get_selected_level_actors()

    if not selected_actors:
        unreal.log_warning("请先选择一个Actor作为房间中心")
        return None

    room_center = selected_actors[0].get_actor_location()
    room_size = unreal.Vector(800.0, 800.0, 300.0)  # 默认房间尺寸

    return create_room_lighting(room_center, room_size, num_lights=4, preset_name='COLD_DIM')


# ============================================================================
# 使用示例
# ============================================================================

def example_usage():
    """
    使用示例代码
    """
    unreal.log("=== LightingAutomation 使用示例 ===")

    # 示例1：创建单个灯光
    # light = create_point_light(unreal.Vector(0, 0, 200), HorrorLightingPresets.COLD_DIM, "TestLight")

    # 示例2：沿走廊放置灯光
    # start = unreal.Vector(0, 0, 0)
    # end = unreal.Vector(2000, 0, 0)
    # lights = place_lights_along_corridor(start, end, spacing=400.0, preset_name='COLD_DIM')

    # 示例3：批量调整选中的灯光
    # apply_horror_atmosphere_to_selected()

    # 示例4：创建闪烁灯光
    # flicker_light = create_flickering_light(unreal.Vector(500, 500, 200), preset_name='WARM_FLICKER')

    # 示例5：为房间创建灯光
    # room_lights = create_room_lighting(unreal.Vector(0, 0, 0), unreal.Vector(1000, 1000, 300), num_lights=4)

    unreal.log("请参考代码中的示例函数")


if __name__ == "__main__":
    unreal.log("LightingAutomation.py 已加载")
    unreal.log("使用 LightingAutomation.example_usage() 查看示例")
