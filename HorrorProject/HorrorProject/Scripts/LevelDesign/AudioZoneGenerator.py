"""
AudioZoneGenerator.py - 音频区域生成器
用于HorrorProject的音频区域自动化创建和配置

功能：
- 根据房间大小自动创建HorrorAudioZoneActor
- 批量配置音频参数
- 分层音频系统（环境+紧张+线索）
- 自动检测房间边界并创建触发区域

使用方法：
1. 在Unreal Editor中打开Python控制台
2. 运行: import AudioZoneGenerator
3. 选择房间Actor，运行: AudioZoneGenerator.create_audio_zone_for_selected_room()

兼容性：Unreal Engine 5.6
"""

import unreal
import math

# ============================================================================
# 音频层级配置
# ============================================================================

class AudioLayerPresets:
    """音频层级预设"""

    # 环境音效层（持续播放）
    AMBIENT_LAYER = {
        'layer_name': 'Ambient',
        'volume': 0.4,
        'fade_in': 2.0,
        'fade_out': 1.5,
        'loop': True,
        'priority': 1.0
    }

    # 紧张音效层（根据玩家状态触发）
    TENSION_LAYER = {
        'layer_name': 'Tension',
        'volume': 0.6,
        'fade_in': 1.0,
        'fade_out': 2.0,
        'loop': True,
        'priority': 2.0
    }

    # 线索音效层（一次性触发）
    CLUE_LAYER = {
        'layer_name': 'Clue',
        'volume': 0.8,
        'fade_in': 0.5,
        'fade_out': 1.0,
        'loop': False,
        'priority': 3.0
    }

    # 恐怖事件层（高优先级）
    SCARE_EVENT_LAYER = {
        'layer_name': 'ScareEvent',
        'volume': 1.0,
        'fade_in': 0.1,
        'fade_out': 0.5,
        'loop': False,
        'priority': 10.0
    }


class HorrorAudioPresets:
    """恐怖音效预设路径（需要根据实际资产路径调整）"""

    # 环境音效
    AMBIENT_SOUNDS = {
        'corridor': '/Game/Audio/Ambient/Amb_Corridor_Loop',
        'room_empty': '/Game/Audio/Ambient/Amb_Room_Empty_Loop',
        'basement': '/Game/Audio/Ambient/Amb_Basement_Loop',
        'outdoor': '/Game/Audio/Ambient/Amb_Outdoor_Loop',
    }

    # 紧张音效
    TENSION_SOUNDS = {
        'low': '/Game/Audio/Tension/Tension_Low_Loop',
        'medium': '/Game/Audio/Tension/Tension_Medium_Loop',
        'high': '/Game/Audio/Tension/Tension_High_Loop',
    }

    # 线索音效
    CLUE_SOUNDS = {
        'whisper': '/Game/Audio/Clues/Clue_Whisper',
        'footsteps': '/Game/Audio/Clues/Clue_Footsteps',
        'breathing': '/Game/Audio/Clues/Clue_Breathing',
    }


# ============================================================================
# 核心音频区域操作函数
# ============================================================================

def create_audio_zone_actor(location, zone_id, zone_size=None):
    """
    创建HorrorAudioZoneActor

    Args:
        location: unreal.Vector - 区域中心位置
        zone_id: str - 区域ID
        zone_size: unreal.Vector - 区域大小（可选，用于创建BoxComponent）

    Returns:
        unreal.Actor - 创建的HorrorAudioZoneActor
    """
    editor_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    world = editor_subsystem.get_editor_world()

    # 加载HorrorAudioZoneActor类
    audio_zone_class = unreal.load_class(None, '/Script/HorrorProject.HorrorAudioZoneActor')

    if not audio_zone_class:
        unreal.log_error("无法加载HorrorAudioZoneActor类，请确保项目已编译")
        return None

    # 创建Actor
    audio_zone = unreal.EditorLevelLibrary.spawn_actor_from_class(
        audio_zone_class,
        location,
        unreal.Rotator(0, 0, 0)
    )

    if audio_zone:
        # 设置区域ID
        audio_zone.set_editor_property('zone_id', unreal.Name(zone_id))
        audio_zone.set_actor_label(f"AudioZone_{zone_id}")

        # 如果提供了区域大小，添加BoxComponent作为触发器
        if zone_size:
            box_component = audio_zone.add_component_by_class(
                unreal.BoxComponent,
                False,
                unreal.Transform(),
                False
            )

            if box_component:
                box_component.set_editor_property('box_extent', zone_size / 2)
                box_component.set_editor_property('generate_overlap_events', True)
                box_component.set_collision_profile_name('OverlapAllDynamic')

        unreal.log(f"创建音频区域: {zone_id} at {location}")
        return audio_zone
    else:
        unreal.log_error(f"无法创建音频区域: {zone_id}")
        return None


def configure_audio_zone(audio_zone, ambient_sound_path=None, layer_preset=None):
    """
    配置音频区域参数

    Args:
        audio_zone: unreal.Actor - HorrorAudioZoneActor
        ambient_sound_path: str - 环境音效资产路径
        layer_preset: dict - 音频层级预设
    """
    if not audio_zone:
        unreal.log_warning("音频区域为空，无法配置")
        return

    if layer_preset is None:
        layer_preset = AudioLayerPresets.AMBIENT_LAYER

    # 加载音效资产
    if ambient_sound_path:
        sound_asset = unreal.load_asset(ambient_sound_path)

        if sound_asset:
            audio_zone.set_editor_property('ambient_sound', sound_asset)
        else:
            unreal.log_warning(f"无法加载音效资产: {ambient_sound_path}")

    # 配置音频参数
    audio_zone.set_editor_property('ambient_volume', layer_preset['volume'])
    audio_zone.set_editor_property('fade_in_duration', layer_preset['fade_in'])
    audio_zone.set_editor_property('fade_out_duration', layer_preset['fade_out'])
    audio_zone.set_editor_property('b_loop_ambient', layer_preset['loop'])

    unreal.log(f"配置音频区域: {audio_zone.get_actor_label()}")


def create_audio_zone_for_room(room_center, room_size, zone_id, audio_type='room_empty'):
    """
    为房间创建音频区域

    Args:
        room_center: unreal.Vector - 房间中心
        room_size: unreal.Vector - 房间尺寸
        zone_id: str - 区域ID
        audio_type: str - 音频类型（corridor, room_empty, basement, outdoor）

    Returns:
        unreal.Actor - 创建的音频区域
    """
    # 创建音频区域
    audio_zone = create_audio_zone_actor(room_center, zone_id, room_size)

    if audio_zone:
        # 获取对应的音效路径
        ambient_sound_path = HorrorAudioPresets.AMBIENT_SOUNDS.get(audio_type)

        # 配置音频区域
        configure_audio_zone(audio_zone, ambient_sound_path, AudioLayerPresets.AMBIENT_LAYER)

    return audio_zone


def create_layered_audio_system(location, zone_id, room_size=None):
    """
    创建分层音频系统（环境+紧张+线索）

    Args:
        location: unreal.Vector - 区域中心
        zone_id: str - 区域ID
        room_size: unreal.Vector - 房间尺寸

    Returns:
        dict - 包含各层音频区域的字典
    """
    audio_zones = {}

    # 创建环境音效层
    ambient_zone = create_audio_zone_actor(location, f"{zone_id}_Ambient", room_size)
    if ambient_zone:
        configure_audio_zone(ambient_zone, None, AudioLayerPresets.AMBIENT_LAYER)
        audio_zones['ambient'] = ambient_zone

    # 创建紧张音效层（稍微偏移位置避免重叠）
    tension_location = location + unreal.Vector(0, 0, 50)
    tension_zone = create_audio_zone_actor(tension_location, f"{zone_id}_Tension", room_size)
    if tension_zone:
        configure_audio_zone(tension_zone, None, AudioLayerPresets.TENSION_LAYER)
        audio_zones['tension'] = tension_zone

    # 创建线索音效层
    clue_location = location + unreal.Vector(0, 0, 100)
    clue_zone = create_audio_zone_actor(clue_location, f"{zone_id}_Clue", room_size)
    if clue_zone:
        configure_audio_zone(clue_zone, None, AudioLayerPresets.CLUE_LAYER)
        audio_zones['clue'] = clue_zone

    unreal.log(f"创建分层音频系统: {zone_id} (共 {len(audio_zones)} 层)")
    return audio_zones


# ============================================================================
# 批量操作函数
# ============================================================================

def batch_create_audio_zones_along_corridor(start_location, end_location, spacing=800.0, audio_type='corridor'):
    """
    沿走廊批量创建音频区域

    Args:
        start_location: unreal.Vector - 起始位置
        end_location: unreal.Vector - 结束位置
        spacing: float - 区域间距
        audio_type: str - 音频类型

    Returns:
        list - 创建的音频区域列表
    """
    direction = end_location - start_location
    distance = direction.length()
    direction = direction.normal()

    num_zones = max(1, int(distance / spacing))
    created_zones = []

    for i in range(num_zones):
        t = i / num_zones if num_zones > 1 else 0
        current_location = start_location + (direction * distance * t)

        zone_id = f"Corridor_Zone_{i+1}"
        zone_size = unreal.Vector(spacing * 0.8, 400.0, 300.0)

        audio_zone = create_audio_zone_for_room(current_location, zone_size, zone_id, audio_type)

        if audio_zone:
            created_zones.append(audio_zone)

    unreal.log(f"沿走廊创建了 {len(created_zones)} 个音频区域")
    return created_zones


def get_all_audio_zones_in_level():
    """
    获取关卡中所有的HorrorAudioZoneActor

    Returns:
        list - 所有音频区域Actor列表
    """
    all_actors = unreal.EditorLevelLibrary.get_all_level_actors()

    # 过滤出HorrorAudioZoneActor
    audio_zones = []
    for actor in all_actors:
        if actor.get_class().get_name() == 'HorrorAudioZoneActor':
            audio_zones.append(actor)

    unreal.log(f"关卡中共有 {len(audio_zones)} 个音频区域")
    return audio_zones


def batch_adjust_audio_zones(audio_zones, volume_multiplier=1.0, fade_in_multiplier=1.0):
    """
    批量调整音频区域参数

    Args:
        audio_zones: list - 音频区域列表
        volume_multiplier: float - 音量倍数
        fade_in_multiplier: float - 淡入时间倍数
    """
    adjusted_count = 0

    for zone in audio_zones:
        if not zone:
            continue

        try:
            # 调整音量
            current_volume = zone.get_editor_property('ambient_volume')
            new_volume = min(1.0, current_volume * volume_multiplier)
            zone.set_editor_property('ambient_volume', new_volume)

            # 调整淡入时间
            current_fade_in = zone.get_editor_property('fade_in_duration')
            new_fade_in = current_fade_in * fade_in_multiplier
            zone.set_editor_property('fade_in_duration', new_fade_in)

            adjusted_count += 1
        except Exception as e:
            unreal.log_warning(f"调整音频区域失败: {zone.get_actor_label()} - {str(e)}")

    unreal.log(f"批量调整了 {adjusted_count} 个音频区域")


# ============================================================================
# 自动检测和创建函数
# ============================================================================

def detect_room_bounds(room_actor):
    """
    检测房间边界

    Args:
        room_actor: unreal.Actor - 房间Actor

    Returns:
        tuple - (center: unreal.Vector, size: unreal.Vector)
    """
    if not room_actor:
        return None, None

    # 获取Actor的边界
    origin, box_extent = room_actor.get_actor_bounds(False)

    # 计算房间尺寸
    room_size = box_extent * 2

    unreal.log(f"检测到房间边界: 中心={origin}, 尺寸={room_size}")
    return origin, room_size


def create_audio_zone_for_selected_room(audio_type='room_empty', use_layered_system=False):
    """
    为选中的房间创建音频区域

    Args:
        audio_type: str - 音频类型
        use_layered_system: bool - 是否使用分层音频系统

    Returns:
        unreal.Actor or dict - 创建的音频区域
    """
    selected_actors = unreal.EditorLevelLibrary.get_selected_level_actors()

    if not selected_actors:
        unreal.log_warning("请先选择一个房间Actor")
        return None

    room_actor = selected_actors[0]
    room_center, room_size = detect_room_bounds(room_actor)

    if room_center is None:
        unreal.log_error("无法检测房间边界")
        return None

    zone_id = f"Room_{room_actor.get_actor_label()}"

    if use_layered_system:
        return create_layered_audio_system(room_center, zone_id, room_size)
    else:
        return create_audio_zone_for_room(room_center, room_size, zone_id, audio_type)


def auto_create_audio_zones_for_all_rooms(room_tag='Room', audio_type='room_empty'):
    """
    自动为所有带有指定标签的房间创建音频区域

    Args:
        room_tag: str - 房间标签
        audio_type: str - 音频类型

    Returns:
        list - 创建的音频区域列表
    """
    all_actors = unreal.EditorLevelLibrary.get_all_level_actors()

    # 过滤出带有指定标签的Actor
    room_actors = [actor for actor in all_actors if room_tag in actor.tags]

    if not room_actors:
        unreal.log_warning(f"未找到带有标签 '{room_tag}' 的房间")
        return []

    created_zones = []

    for room_actor in room_actors:
        room_center, room_size = detect_room_bounds(room_actor)

        if room_center:
            zone_id = f"Room_{room_actor.get_actor_label()}"
            audio_zone = create_audio_zone_for_room(room_center, room_size, zone_id, audio_type)

            if audio_zone:
                created_zones.append(audio_zone)

    unreal.log(f"自动为 {len(created_zones)} 个房间创建了音频区域")
    return created_zones


# ============================================================================
# 音频区域验证和调试
# ============================================================================

def validate_audio_zone(audio_zone):
    """
    验证音频区域配置

    Args:
        audio_zone: unreal.Actor - 音频区域

    Returns:
        dict - 验证结果
    """
    validation_result = {
        'valid': True,
        'warnings': [],
        'errors': []
    }

    if not audio_zone:
        validation_result['valid'] = False
        validation_result['errors'].append("音频区域为空")
        return validation_result

    try:
        # 检查ZoneId
        zone_id = audio_zone.get_editor_property('zone_id')
        if not zone_id or str(zone_id) == 'Zone.Default':
            validation_result['warnings'].append("使用默认ZoneId")

        # 检查音效资产
        ambient_sound = audio_zone.get_editor_property('ambient_sound')
        if not ambient_sound:
            validation_result['warnings'].append("未设置环境音效")

        # 检查音量
        volume = audio_zone.get_editor_property('ambient_volume')
        if volume <= 0:
            validation_result['errors'].append("音量为0或负数")
            validation_result['valid'] = False

        # 检查淡入淡出时间
        fade_in = audio_zone.get_editor_property('fade_in_duration')
        fade_out = audio_zone.get_editor_property('fade_out_duration')

        if fade_in < 0 or fade_out < 0:
            validation_result['errors'].append("淡入/淡出时间为负数")
            validation_result['valid'] = False

    except Exception as e:
        validation_result['valid'] = False
        validation_result['errors'].append(f"验证过程出错: {str(e)}")

    return validation_result


def validate_all_audio_zones():
    """
    验证关卡中所有音频区域

    Returns:
        dict - 验证统计结果
    """
    audio_zones = get_all_audio_zones_in_level()

    stats = {
        'total': len(audio_zones),
        'valid': 0,
        'warnings': 0,
        'errors': 0
    }

    for zone in audio_zones:
        result = validate_audio_zone(zone)

        if result['valid']:
            stats['valid'] += 1

        if result['warnings']:
            stats['warnings'] += len(result['warnings'])
            unreal.log_warning(f"{zone.get_actor_label()}: {', '.join(result['warnings'])}")

        if result['errors']:
            stats['errors'] += len(result['errors'])
            unreal.log_error(f"{zone.get_actor_label()}: {', '.join(result['errors'])}")

    unreal.log(f"音频区域验证完成: 总数={stats['total']}, 有效={stats['valid']}, 警告={stats['warnings']}, 错误={stats['errors']}")
    return stats


# ============================================================================
# 便捷使用函数
# ============================================================================

def quick_corridor_audio():
    """
    快速为走廊创建音频区域（使用选中的两个Actor作为起点和终点）
    """
    selected_actors = unreal.EditorLevelLibrary.get_selected_level_actors()

    if len(selected_actors) < 2:
        unreal.log_warning("请选择两个Actor作为走廊的起点和终点")
        return None

    start_pos = selected_actors[0].get_actor_location()
    end_pos = selected_actors[1].get_actor_location()

    return batch_create_audio_zones_along_corridor(start_pos, end_pos, spacing=800.0, audio_type='corridor')


def quick_room_audio(use_layered=False):
    """
    快速为选中的房间创建音频区域
    """
    return create_audio_zone_for_selected_room(audio_type='room_empty', use_layered_system=use_layered)


# ============================================================================
# 使用示例
# ============================================================================

def example_usage():
    """
    使用示例代码
    """
    unreal.log("=== AudioZoneGenerator 使用示例 ===")

    # 示例1：为选中的房间创建音频区域
    # audio_zone = create_audio_zone_for_selected_room(audio_type='room_empty')

    # 示例2：创建分层音频系统
    # layered_system = create_audio_zone_for_selected_room(use_layered_system=True)

    # 示例3：沿走廊批量创建音频区域
    # quick_corridor_audio()

    # 示例4：自动为所有房间创建音频区域
    # auto_create_audio_zones_for_all_rooms(room_tag='Room', audio_type='room_empty')

    # 示例5：验证所有音频区域
    # validate_all_audio_zones()

    unreal.log("请参考代码中的示例函数")


if __name__ == "__main__":
    unreal.log("AudioZoneGenerator.py 已加载")
    unreal.log("使用 AudioZoneGenerator.example_usage() 查看示例")
