"""
NavMeshHelper.py - 导航网格助手
用于HorrorProject的导航网格检测、生成和验证

功能：
- 检测缺失导航网格的区域
- 自动生成Nav Mesh Bounds Volume
- 验证AI可达性
- 导航网格质量分析

使用方法：
1. 在Unreal Editor中打开Python控制台
2. 运行: import NavMeshHelper
3. 检测问题: NavMeshHelper.detect_navmesh_issues()
4. 自动修复: NavMeshHelper.auto_fix_navmesh_for_level()

兼容性：Unreal Engine 5.6
"""

import unreal
import math

# ============================================================================
# 导航网格配置
# ============================================================================

class NavMeshConfig:
    """导航网格配置参数"""

    # 默认Nav Mesh Bounds Volume尺寸
    DEFAULT_BOUNDS_SIZE = unreal.Vector(2000.0, 2000.0, 500.0)

    # 最小可行走区域尺寸
    MIN_WALKABLE_AREA = 100.0

    # AI角色高度（用于可达性检测）
    AI_CHARACTER_HEIGHT = 180.0

    # AI角色半径
    AI_CHARACTER_RADIUS = 40.0

    # 导航网格采样间隔
    SAMPLE_INTERVAL = 200.0


# ============================================================================
# 核心导航网格操作函数
# ============================================================================

def create_nav_mesh_bounds_volume(location, bounds_size=None):
    """
    创建Nav Mesh Bounds Volume

    Args:
        location: unreal.Vector - 位置
        bounds_size: unreal.Vector - 边界尺寸

    Returns:
        unreal.NavMeshBoundsVolume - 创建的导航网格边界体积
    """
    if bounds_size is None:
        bounds_size = NavMeshConfig.DEFAULT_BOUNDS_SIZE

    editor_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)

    # 创建NavMeshBoundsVolume
    nav_bounds = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.NavMeshBoundsVolume,
        location,
        unreal.Rotator(0, 0, 0)
    )

    if nav_bounds:
        # 获取BrushComponent并设置尺寸
        brush_component = nav_bounds.get_component_by_class(unreal.BrushComponent)

        if brush_component:
            # 设置边界尺寸
            nav_bounds.set_actor_scale3d(bounds_size / 200.0)  # 默认Box是200x200x200

        nav_bounds.set_actor_label(f"NavMeshBounds_{int(location.x)}_{int(location.y)}")

        unreal.log(f"创建导航网格边界: {nav_bounds.get_actor_label()} at {location}")
        return nav_bounds
    else:
        unreal.log_error("无法创建NavMeshBoundsVolume")
        return None


def get_all_nav_mesh_bounds_volumes():
    """
    获取关卡中所有的NavMeshBoundsVolume

    Returns:
        list - 所有NavMeshBoundsVolume列表
    """
    all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
    nav_bounds = [actor for actor in all_actors if isinstance(actor, unreal.NavMeshBoundsVolume)]

    unreal.log(f"关卡中共有 {len(nav_bounds)} 个NavMeshBoundsVolume")
    return nav_bounds


def get_all_walkable_surfaces():
    """
    获取关卡中所有可行走的表面（StaticMeshActor和Landscape）

    Returns:
        list - 可行走表面Actor列表
    """
    all_actors = unreal.EditorLevelLibrary.get_all_level_actors()

    walkable_actors = []

    for actor in all_actors:
        # 检查是否是StaticMeshActor
        if isinstance(actor, unreal.StaticMeshActor):
            # 检查是否启用了碰撞
            static_mesh_component = actor.get_component_by_class(unreal.StaticMeshComponent)

            if static_mesh_component:
                collision_enabled = static_mesh_component.get_collision_enabled()

                if collision_enabled != unreal.CollisionEnabled.NO_COLLISION:
                    walkable_actors.append(actor)

        # 检查是否是Landscape
        elif actor.get_class().get_name() == 'Landscape':
            walkable_actors.append(actor)

    unreal.log(f"找到 {len(walkable_actors)} 个可行走表面")
    return walkable_actors


def detect_uncovered_walkable_areas():
    """
    检测未被NavMeshBoundsVolume覆盖的可行走区域

    Returns:
        list - 未覆盖区域的位置列表
    """
    walkable_actors = get_all_walkable_surfaces()
    nav_bounds = get_all_nav_mesh_bounds_volumes()

    uncovered_areas = []

    for actor in walkable_actors:
        actor_location = actor.get_actor_location()
        actor_bounds_origin, actor_bounds_extent = actor.get_actor_bounds(False)

        # 检查是否被任何NavMeshBoundsVolume覆盖
        is_covered = False

        for nav_bound in nav_bounds:
            nav_location = nav_bound.get_actor_location()
            nav_scale = nav_bound.get_actor_scale3d()
            nav_extent = unreal.Vector(100.0, 100.0, 100.0) * nav_scale  # 默认Box是200x200x200，半径100

            # 简单的AABB碰撞检测
            if (abs(actor_bounds_origin.x - nav_location.x) < (actor_bounds_extent.x + nav_extent.x) and
                abs(actor_bounds_origin.y - nav_location.y) < (actor_bounds_extent.y + nav_extent.y) and
                abs(actor_bounds_origin.z - nav_location.z) < (actor_bounds_extent.z + nav_extent.z)):
                is_covered = True
                break

        if not is_covered:
            uncovered_areas.append({
                'actor': actor,
                'location': actor_bounds_origin,
                'extent': actor_bounds_extent
            })

    unreal.log(f"检测到 {len(uncovered_areas)} 个未覆盖的可行走区域")
    return uncovered_areas


def auto_create_nav_bounds_for_uncovered_areas():
    """
    自动为未覆盖的可行走区域创建NavMeshBoundsVolume

    Returns:
        list - 创建的NavMeshBoundsVolume列表
    """
    uncovered_areas = detect_uncovered_walkable_areas()

    if not uncovered_areas:
        unreal.log("所有可行走区域都已覆盖导航网格")
        return []

    created_bounds = []

    for area in uncovered_areas:
        location = area['location']
        extent = area['extent']

        # 计算合适的边界尺寸（稍微大一点以确保完全覆盖）
        bounds_size = extent * 2.2

        # 确保最小尺寸
        bounds_size.x = max(bounds_size.x, NavMeshConfig.MIN_WALKABLE_AREA)
        bounds_size.y = max(bounds_size.y, NavMeshConfig.MIN_WALKABLE_AREA)
        bounds_size.z = max(bounds_size.z, NavMeshConfig.AI_CHARACTER_HEIGHT * 2)

        nav_bound = create_nav_mesh_bounds_volume(location, bounds_size)

        if nav_bound:
            created_bounds.append(nav_bound)

    unreal.log(f"自动创建了 {len(created_bounds)} 个NavMeshBoundsVolume")
    return created_bounds


# ============================================================================
# 导航网格验证函数
# ============================================================================

def check_nav_mesh_at_location(location):
    """
    检查指定位置是否有导航网格

    Args:
        location: unreal.Vector - 检查位置

    Returns:
        bool - 是否有导航网格
    """
    # 使用NavigationSystemV1进行查询
    nav_system = unreal.NavigationSystemV1.get_navigation_system(
        unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).get_editor_world()
    )

    if not nav_system:
        unreal.log_warning("无法获取NavigationSystem")
        return False

    # 投射到导航网格
    projected_location = nav_system.project_point_to_navigation(
        unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).get_editor_world(),
        location
    )

    # 检查投射是否成功
    has_navmesh = projected_location is not None

    return has_navmesh


def validate_ai_reachability(start_location, end_location):
    """
    验证AI是否可以从起点到达终点

    Args:
        start_location: unreal.Vector - 起点
        end_location: unreal.Vector - 终点

    Returns:
        dict - 可达性验证结果
    """
    result = {
        'reachable': False,
        'path_length': 0.0,
        'has_start_navmesh': False,
        'has_end_navmesh': False
    }

    nav_system = unreal.NavigationSystemV1.get_navigation_system(
        unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).get_editor_world()
    )

    if not nav_system:
        unreal.log_warning("无法获取NavigationSystem")
        return result

    # 检查起点和终点是否有导航网格
    result['has_start_navmesh'] = check_nav_mesh_at_location(start_location)
    result['has_end_navmesh'] = check_nav_mesh_at_location(end_location)

    if not result['has_start_navmesh']:
        unreal.log_warning(f"起点没有导航网格: {start_location}")
        return result

    if not result['has_end_navmesh']:
        unreal.log_warning(f"终点没有导航网格: {end_location}")
        return result

    # 尝试寻路
    try:
        path_result = nav_system.find_path_to_location_synchronously(
            unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).get_editor_world(),
            start_location,
            end_location
        )

        if path_result and path_result.path:
            result['reachable'] = True
            result['path_length'] = path_result.path.get_path_length()

    except Exception as e:
        unreal.log_warning(f"寻路失败: {str(e)}")

    return result


def sample_navmesh_coverage(bounds_min, bounds_max, sample_interval=None):
    """
    采样指定区域的导航网格覆盖率

    Args:
        bounds_min: unreal.Vector - 区域最小边界
        bounds_max: unreal.Vector - 区域最大边界
        sample_interval: float - 采样间隔

    Returns:
        dict - 覆盖率统计
    """
    if sample_interval is None:
        sample_interval = NavMeshConfig.SAMPLE_INTERVAL

    stats = {
        'total_samples': 0,
        'covered_samples': 0,
        'coverage_percentage': 0.0,
        'uncovered_locations': []
    }

    # 计算采样点数量
    x_samples = max(1, int((bounds_max.x - bounds_min.x) / sample_interval))
    y_samples = max(1, int((bounds_max.y - bounds_min.y) / sample_interval))

    # 采样高度（地面附近）
    sample_z = bounds_min.z + 50.0

    for i in range(x_samples):
        for j in range(y_samples):
            sample_x = bounds_min.x + (i * sample_interval)
            sample_y = bounds_min.y + (j * sample_interval)
            sample_location = unreal.Vector(sample_x, sample_y, sample_z)

            stats['total_samples'] += 1

            if check_nav_mesh_at_location(sample_location):
                stats['covered_samples'] += 1
            else:
                stats['uncovered_locations'].append(sample_location)

    # 计算覆盖率
    if stats['total_samples'] > 0:
        stats['coverage_percentage'] = (stats['covered_samples'] / stats['total_samples']) * 100.0

    unreal.log(f"导航网格覆盖率: {stats['coverage_percentage']:.2f}% ({stats['covered_samples']}/{stats['total_samples']})")
    return stats


# ============================================================================
# 导航网格问题检测
# ============================================================================

def detect_navmesh_issues():
    """
    检测关卡中的导航网格问题

    Returns:
        dict - 问题报告
    """
    report = {
        'has_nav_bounds': False,
        'num_nav_bounds': 0,
        'uncovered_areas': [],
        'isolated_nav_islands': [],
        'warnings': [],
        'errors': []
    }

    # 检查是否有NavMeshBoundsVolume
    nav_bounds = get_all_nav_mesh_bounds_volumes()
    report['num_nav_bounds'] = len(nav_bounds)
    report['has_nav_bounds'] = len(nav_bounds) > 0

    if not report['has_nav_bounds']:
        report['errors'].append("关卡中没有NavMeshBoundsVolume")

    # 检测未覆盖的区域
    uncovered_areas = detect_uncovered_walkable_areas()
    report['uncovered_areas'] = uncovered_areas

    if uncovered_areas:
        report['warnings'].append(f"发现 {len(uncovered_areas)} 个未覆盖的可行走区域")

    # 检查NavMeshBoundsVolume尺寸
    for nav_bound in nav_bounds:
        scale = nav_bound.get_actor_scale3d()
        size = scale * 200.0  # 默认Box是200x200x200

        if size.x < NavMeshConfig.MIN_WALKABLE_AREA or size.y < NavMeshConfig.MIN_WALKABLE_AREA:
            report['warnings'].append(f"{nav_bound.get_actor_label()} 尺寸过小")

    # 生成报告摘要
    unreal.log("=== 导航网格问题检测报告 ===")
    unreal.log(f"NavMeshBoundsVolume数量: {report['num_nav_bounds']}")
    unreal.log(f"未覆盖区域: {len(report['uncovered_areas'])}")
    unreal.log(f"警告: {len(report['warnings'])}")
    unreal.log(f"错误: {len(report['errors'])}")

    for warning in report['warnings']:
        unreal.log_warning(warning)

    for error in report['errors']:
        unreal.log_error(error)

    return report


def detect_nav_mesh_islands():
    """
    检测导航网格孤岛（不连通的导航区域）

    Returns:
        list - 孤岛信息列表
    """
    nav_bounds = get_all_nav_mesh_bounds_volumes()

    if len(nav_bounds) < 2:
        unreal.log("NavMeshBoundsVolume数量少于2个，无法检测孤岛")
        return []

    islands = []

    # 简单的连通性检测：检查每个NavBounds是否可以到达其他NavBounds
    for i, nav_bound_a in enumerate(nav_bounds):
        location_a = nav_bound_a.get_actor_location()
        is_connected = False

        for j, nav_bound_b in enumerate(nav_bounds):
            if i == j:
                continue

            location_b = nav_bound_b.get_actor_location()

            # 验证可达性
            reachability = validate_ai_reachability(location_a, location_b)

            if reachability['reachable']:
                is_connected = True
                break

        if not is_connected:
            islands.append({
                'nav_bound': nav_bound_a,
                'location': location_a
            })

    if islands:
        unreal.log_warning(f"检测到 {len(islands)} 个导航网格孤岛")
    else:
        unreal.log("未检测到导航网格孤岛")

    return islands


# ============================================================================
# 自动修复函数
# ============================================================================

def auto_fix_navmesh_for_level():
    """
    自动修复关卡的导航网格问题

    Returns:
        dict - 修复结果
    """
    result = {
        'created_nav_bounds': [],
        'fixed_issues': 0
    }

    unreal.log("开始自动修复导航网格问题...")

    # 检测问题
    issues = detect_navmesh_issues()

    # 修复未覆盖的区域
    if issues['uncovered_areas']:
        created_bounds = auto_create_nav_bounds_for_uncovered_areas()
        result['created_nav_bounds'] = created_bounds
        result['fixed_issues'] += len(created_bounds)

    # 重建导航网格
    rebuild_navmesh()

    unreal.log(f"自动修复完成，修复了 {result['fixed_issues']} 个问题")
    return result


def rebuild_navmesh():
    """
    重建导航网格
    """
    nav_system = unreal.NavigationSystemV1.get_navigation_system(
        unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).get_editor_world()
    )

    if nav_system:
        # 触发导航网格重建
        nav_system.on_navigation_bounds_updated()
        unreal.log("触发导航网格重建")
    else:
        unreal.log_warning("无法获取NavigationSystem，无法重建导航网格")


# ============================================================================
# 批量操作函数
# ============================================================================

def create_nav_bounds_for_selected_area():
    """
    为选中的区域创建NavMeshBoundsVolume

    使用选中的Actor的边界来确定NavBounds尺寸
    """
    selected_actors = unreal.EditorLevelLibrary.get_selected_level_actors()

    if not selected_actors:
        unreal.log_warning("请先选择一个或多个Actor")
        return None

    # 计算所有选中Actor的总边界
    min_bounds = unreal.Vector(float('inf'), float('inf'), float('inf'))
    max_bounds = unreal.Vector(float('-inf'), float('-inf'), float('-inf'))

    for actor in selected_actors:
        origin, extent = actor.get_actor_bounds(False)

        min_bounds.x = min(min_bounds.x, origin.x - extent.x)
        min_bounds.y = min(min_bounds.y, origin.y - extent.y)
        min_bounds.z = min(min_bounds.z, origin.z - extent.z)

        max_bounds.x = max(max_bounds.x, origin.x + extent.x)
        max_bounds.y = max(max_bounds.y, origin.y + extent.y)
        max_bounds.z = max(max_bounds.z, origin.z + extent.z)

    # 计算中心和尺寸
    center = (min_bounds + max_bounds) / 2
    size = max_bounds - min_bounds

    # 稍微扩大一点
    size *= 1.1

    return create_nav_mesh_bounds_volume(center, size)


def batch_adjust_nav_bounds_height(nav_bounds_list, new_height):
    """
    批量调整NavMeshBoundsVolume的高度

    Args:
        nav_bounds_list: list - NavMeshBoundsVolume列表
        new_height: float - 新高度
    """
    adjusted_count = 0

    for nav_bound in nav_bounds_list:
        if not nav_bound:
            continue

        current_scale = nav_bound.get_actor_scale3d()
        new_scale = unreal.Vector(current_scale.x, current_scale.y, new_height / 200.0)

        nav_bound.set_actor_scale3d(new_scale)
        adjusted_count += 1

    unreal.log(f"批量调整了 {adjusted_count} 个NavMeshBoundsVolume的高度")


# ============================================================================
# 可视化和调试
# ============================================================================

def visualize_navmesh_coverage(bounds_min, bounds_max):
    """
    可视化导航网格覆盖情况（通过创建调试Actor）

    Args:
        bounds_min: unreal.Vector - 区域最小边界
        bounds_max: unreal.Vector - 区域最大边界
    """
    stats = sample_navmesh_coverage(bounds_min, bounds_max)

    # 在未覆盖的位置创建可视化标记
    for location in stats['uncovered_locations'][:50]:  # 限制数量避免过多
        # 创建一个小的StaticMeshActor作为标记
        marker = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.StaticMeshActor,
            location,
            unreal.Rotator(0, 0, 0)
        )

        if marker:
            marker.set_actor_label("NavMesh_Missing_Marker")
            marker.tags.append("DebugMarker")

    unreal.log(f"创建了 {min(len(stats['uncovered_locations']), 50)} 个可视化标记")


def clear_debug_markers():
    """
    清除所有调试标记
    """
    all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
    markers = [actor for actor in all_actors if "DebugMarker" in actor.tags]

    for marker in markers:
        unreal.EditorLevelLibrary.destroy_actor(marker)

    unreal.log(f"清除了 {len(markers)} 个调试标记")


# ============================================================================
# 便捷使用函数
# ============================================================================

def quick_fix_navmesh():
    """
    快速修复导航网格（一键操作）
    """
    unreal.log("=== 快速修复导航网格 ===")

    # 检测问题
    issues = detect_navmesh_issues()

    # 自动修复
    result = auto_fix_navmesh_for_level()

    # 重新检测
    unreal.log("\n重新检测问题...")
    detect_navmesh_issues()

    return result


def quick_create_nav_bounds():
    """
    快速为选中区域创建NavMeshBoundsVolume
    """
    return create_nav_bounds_for_selected_area()


# ============================================================================
# 使用示例
# ============================================================================

def example_usage():
    """
    使用示例代码
    """
    unreal.log("=== NavMeshHelper 使用示例 ===")

    # 示例1：检测导航网格问题
    # issues = detect_navmesh_issues()

    # 示例2：自动修复导航网格
    # result = auto_fix_navmesh_for_level()

    # 示例3：为选中区域创建NavMeshBoundsVolume
    # nav_bound = create_nav_bounds_for_selected_area()

    # 示例4：验证AI可达性
    # start = unreal.Vector(0, 0, 0)
    # end = unreal.Vector(1000, 1000, 0)
    # reachability = validate_ai_reachability(start, end)

    # 示例5：检测导航网格孤岛
    # islands = detect_nav_mesh_islands()

    # 示例6：快速修复（一键操作）
    # quick_fix_navmesh()

    unreal.log("请参考代码中的示例函数")


if __name__ == "__main__":
    unreal.log("NavMeshHelper.py 已加载")
    unreal.log("使用 NavMeshHelper.example_usage() 查看示例")
