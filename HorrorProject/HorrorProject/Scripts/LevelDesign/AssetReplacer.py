#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
AssetReplacer.py - 白盒资产替换工具
批量替换LevelPrototyping为真实资产，保持transform信息
支持DeepWaterStation、Industrial_Architecture等资产包
"""

import unreal
import json
import os
from datetime import datetime
from pathlib import Path


class AssetReplacer:
    """资产替换器 - 将白盒资产替换为真实资产"""

    def __init__(self):
        self.editor_util = unreal.EditorLevelLibrary()
        self.editor_asset_lib = unreal.EditorAssetLibrary()
        self.replacement_map = self._load_replacement_map()
        self.report_data = {
            'timestamp': datetime.now().isoformat(),
            'replacements': [],
            'errors': [],
            'statistics': {}
        }

    def _load_replacement_map(self):
        """加载资产替换映射表"""
        return {
            # LevelPrototyping -> DeepWaterStation
            '/Engine/BasicShapes/Cube': [
                '/Game/DeepWaterStation/Meshes/SM_Concrete_Block_01',
                '/Game/DeepWaterStation/Meshes/SM_Metal_Crate_01'
            ],
            '/Engine/BasicShapes/Cylinder': [
                '/Game/DeepWaterStation/Meshes/SM_Pipe_01',
                '/Game/DeepWaterStation/Meshes/SM_Tank_01'
            ],
            '/Game/LevelPrototyping/Meshes/SM_Cube': [
                '/Game/DeepWaterStation/Meshes/SM_Concrete_Block_01',
                '/Game/Industrial_Architecture/Meshes/SM_Wall_01'
            ],
            '/Game/LevelPrototyping/Meshes/SM_ChamferedCube': [
                '/Game/Industrial_Architecture/Meshes/SM_Wall_01',
                '/Game/Industrial_Architecture/Meshes/SM_Floor_01'
            ],
            '/Game/LevelPrototyping/Meshes/SM_Ramp': [
                '/Game/Industrial_Architecture/Meshes/SM_Ramp_01',
                '/Game/DeepWaterStation/Meshes/SM_Stairs_01'
            ],
            '/Game/LevelPrototyping/Meshes/SM_Cylinder': [
                '/Game/DeepWaterStation/Meshes/SM_Pipe_01',
                '/Game/Industrial_Architecture/Meshes/SM_Column_01'
            ],
            '/Game/LevelPrototyping/Meshes/SM_Stairs': [
                '/Game/DeepWaterStation/Meshes/SM_Stairs_01',
                '/Game/Industrial_Architecture/Meshes/SM_Stairs_Metal_01'
            ],
            # 添加更多映射...
        }

    def scan_level_for_prototypes(self, level_path=None):
        """扫描关卡中的白盒资产"""
        if level_path:
            unreal.EditorLoadingAndSavingUtils.load_map(level_path)

        all_actors = self.editor_util.get_all_level_actors()
        prototype_actors = []

        for actor in all_actors:
            if isinstance(actor, unreal.StaticMeshActor):
                mesh_component = actor.static_mesh_component
                if mesh_component:
                    mesh = mesh_component.static_mesh
                    if mesh:
                        mesh_path = mesh.get_path_name()
                        if self._is_prototype_asset(mesh_path):
                            prototype_actors.append({
                                'actor': actor,
                                'mesh_path': mesh_path,
                                'transform': actor.get_actor_transform()
                            })

        unreal.log(f"找到 {len(prototype_actors)} 个白盒资产")
        return prototype_actors

    def _is_prototype_asset(self, asset_path):
        """判断是否为白盒资产"""
        prototype_keywords = [
            'LevelPrototyping',
            'BasicShapes',
            'Prototype',
            'Whitebox'
        ]
        return any(keyword in asset_path for keyword in prototype_keywords)

    def replace_assets(self, prototype_actors, strategy='smart'):
        """
        替换资产
        strategy: 'smart' (智能选择), 'first' (使用第一个), 'random' (随机)
        """
        replaced_count = 0

        for proto_data in prototype_actors:
            try:
                actor = proto_data['actor']
                mesh_path = proto_data['mesh_path']
                transform = proto_data['transform']

                # 查找替换资产
                replacement_paths = self.replacement_map.get(mesh_path, [])
                if not replacement_paths:
                    self.report_data['errors'].append({
                        'actor': actor.get_name(),
                        'mesh': mesh_path,
                        'error': '未找到替换映射'
                    })
                    continue

                # 选择替换资产
                if strategy == 'smart':
                    new_mesh_path = self._smart_select_replacement(
                        mesh_path, replacement_paths, transform
                    )
                elif strategy == 'random':
                    import random
                    new_mesh_path = random.choice(replacement_paths)
                else:
                    new_mesh_path = replacement_paths[0]

                # 加载新资产
                new_mesh = self.editor_asset_lib.load_asset(new_mesh_path)
                if not new_mesh:
                    self.report_data['errors'].append({
                        'actor': actor.get_name(),
                        'mesh': new_mesh_path,
                        'error': '无法加载资产'
                    })
                    continue

                # 执行替换
                success = self._replace_actor_mesh(actor, new_mesh, transform)

                if success:
                    replaced_count += 1
                    self.report_data['replacements'].append({
                        'actor': actor.get_name(),
                        'old_mesh': mesh_path,
                        'new_mesh': new_mesh_path,
                        'location': str(transform.translation)
                    })
                    unreal.log(f"✓ 替换: {actor.get_name()}")

            except Exception as e:
                self.report_data['errors'].append({
                    'actor': proto_data.get('actor', 'Unknown'),
                    'error': str(e)
                })
                unreal.log_error(f"替换失败: {e}")

        self.report_data['statistics']['total_replaced'] = replaced_count
        unreal.log(f"完成替换: {replaced_count}/{len(prototype_actors)}")
        return replaced_count

    def _smart_select_replacement(self, old_mesh_path, replacement_paths, transform):
        """智能选择替换资产（基于尺寸、位置等）"""
        scale = transform.scale3d
        location = transform.translation

        # 根据缩放选择合适的资产
        avg_scale = (scale.x + scale.y + scale.z) / 3.0

        if avg_scale > 5.0:
            # 大型物体 - 优先选择建筑结构
            for path in replacement_paths:
                if 'Wall' in path or 'Floor' in path or 'Block' in path:
                    return path
        elif avg_scale < 1.0:
            # 小型物体 - 优先选择细节资产
            for path in replacement_paths:
                if 'Crate' in path or 'Pipe' in path:
                    return path

        # 默认返回第一个
        return replacement_paths[0]

    def _replace_actor_mesh(self, actor, new_mesh, transform):
        """替换Actor的Mesh并保持Transform"""
        try:
            mesh_component = actor.static_mesh_component
            if mesh_component:
                mesh_component.set_static_mesh(new_mesh)
                actor.set_actor_transform(transform, False, True)
                return True
        except Exception as e:
            unreal.log_error(f"设置Mesh失败: {e}")
        return False

    def batch_replace_in_folder(self, content_folder='/Game/Maps'):
        """批量处理文件夹中的所有关卡"""
        level_paths = self._find_all_levels(content_folder)

        total_replaced = 0
        for level_path in level_paths:
            unreal.log(f"\n处理关卡: {level_path}")
            prototype_actors = self.scan_level_for_prototypes(level_path)
            replaced = self.replace_assets(prototype_actors)
            total_replaced += replaced

            # 保存关卡
            unreal.EditorLoadingAndSavingUtils.save_current_level()

        self.report_data['statistics']['total_levels'] = len(level_paths)
        self.report_data['statistics']['total_replaced_all'] = total_replaced

        return total_replaced

    def _find_all_levels(self, content_folder):
        """查找文件夹中的所有关卡"""
        all_assets = self.editor_asset_lib.list_assets(content_folder, recursive=True)
        level_paths = [
            asset for asset in all_assets
            if asset.endswith('.umap')
        ]
        return level_paths

    def generate_html_report(self, output_path=None):
        """生成HTML报告"""
        if not output_path:
            project_dir = unreal.Paths.project_dir()
            output_path = os.path.join(
                project_dir,
                'Saved',
                'Reports',
                f'AssetReplacement_{datetime.now().strftime("%Y%m%d_%H%M%S")}.html'
            )

        os.makedirs(os.path.dirname(output_path), exist_ok=True)

        html_content = self._generate_html_content()

        with open(output_path, 'w', encoding='utf-8') as f:
            f.write(html_content)

        unreal.log(f"报告已生成: {output_path}")
        return output_path

    def _generate_html_content(self):
        """生成HTML内容"""
        stats = self.report_data['statistics']
        replacements = self.report_data['replacements']
        errors = self.report_data['errors']

        html = f"""<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>资产替换报告 - HorrorProject</title>
    <style>
        * {{ margin: 0; padding: 0; box-sizing: border-box; }}
        body {{
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            padding: 20px;
            color: #333;
        }}
        .container {{
            max-width: 1400px;
            margin: 0 auto;
            background: white;
            border-radius: 12px;
            box-shadow: 0 20px 60px rgba(0,0,0,0.3);
            overflow: hidden;
        }}
        .header {{
            background: linear-gradient(135deg, #1e3c72 0%, #2a5298 100%);
            color: white;
            padding: 40px;
            text-align: center;
        }}
        .header h1 {{
            font-size: 2.5em;
            margin-bottom: 10px;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
        }}
        .header .timestamp {{
            opacity: 0.9;
            font-size: 1.1em;
        }}
        .stats {{
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 20px;
            padding: 40px;
            background: #f8f9fa;
        }}
        .stat-card {{
            background: white;
            padding: 25px;
            border-radius: 10px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
            text-align: center;
            transition: transform 0.3s;
        }}
        .stat-card:hover {{
            transform: translateY(-5px);
            box-shadow: 0 8px 12px rgba(0,0,0,0.15);
        }}
        .stat-card .number {{
            font-size: 3em;
            font-weight: bold;
            color: #667eea;
            margin-bottom: 10px;
        }}
        .stat-card .label {{
            color: #666;
            font-size: 1.1em;
        }}
        .section {{
            padding: 40px;
        }}
        .section h2 {{
            color: #1e3c72;
            margin-bottom: 20px;
            padding-bottom: 10px;
            border-bottom: 3px solid #667eea;
            font-size: 1.8em;
        }}
        .table-container {{
            overflow-x: auto;
            margin-top: 20px;
        }}
        table {{
            width: 100%;
            border-collapse: collapse;
            background: white;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }}
        th {{
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 15px;
            text-align: left;
            font-weight: 600;
        }}
        td {{
            padding: 12px 15px;
            border-bottom: 1px solid #e0e0e0;
        }}
        tr:hover {{
            background: #f5f5f5;
        }}
        .success {{ color: #28a745; font-weight: bold; }}
        .error {{ color: #dc3545; font-weight: bold; }}
        .code {{
            background: #f4f4f4;
            padding: 2px 6px;
            border-radius: 3px;
            font-family: 'Courier New', monospace;
            font-size: 0.9em;
        }}
        .footer {{
            background: #2c3e50;
            color: white;
            padding: 20px;
            text-align: center;
        }}
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🎮 资产替换报告</h1>
            <div class="timestamp">生成时间: {self.report_data['timestamp']}</div>
        </div>

        <div class="stats">
            <div class="stat-card">
                <div class="number">{stats.get('total_replaced', 0)}</div>
                <div class="label">成功替换</div>
            </div>
            <div class="stat-card">
                <div class="number">{len(errors)}</div>
                <div class="label">失败数量</div>
            </div>
            <div class="stat-card">
                <div class="number">{stats.get('total_levels', 0)}</div>
                <div class="label">处理关卡</div>
            </div>
            <div class="stat-card">
                <div class="number">{len(replacements) + len(errors)}</div>
                <div class="label">总计处理</div>
            </div>
        </div>

        <div class="section">
            <h2>✅ 成功替换列表</h2>
            <div class="table-container">
                <table>
                    <thead>
                        <tr>
                            <th>Actor名称</th>
                            <th>原始资产</th>
                            <th>新资产</th>
                            <th>位置</th>
                        </tr>
                    </thead>
                    <tbody>
"""

        for item in replacements:
            html += f"""
                        <tr>
                            <td>{item['actor']}</td>
                            <td><span class="code">{item['old_mesh']}</span></td>
                            <td><span class="code">{item['new_mesh']}</span></td>
                            <td>{item['location']}</td>
                        </tr>
"""

        html += """
                    </tbody>
                </table>
            </div>
        </div>
"""

        if errors:
            html += """
        <div class="section">
            <h2>❌ 错误列表</h2>
            <div class="table-container">
                <table>
                    <thead>
                        <tr>
                            <th>Actor/资产</th>
                            <th>错误信息</th>
                        </tr>
                    </thead>
                    <tbody>
"""
            for error in errors:
                html += f"""
                        <tr>
                            <td>{error.get('actor', 'N/A')}</td>
                            <td class="error">{error['error']}</td>
                        </tr>
"""
            html += """
                    </tbody>
                </table>
            </div>
        </div>
"""

        html += """
        <div class="footer">
            <p>HorrorProject - Asset Replacement Tool v1.0</p>
        </div>
    </div>
</body>
</html>
"""
        return html


def main():
    """主函数 - 示例用法"""
    replacer = AssetReplacer()

    # 方式1: 替换当前关卡
    prototype_actors = replacer.scan_level_for_prototypes()
    replacer.replace_assets(prototype_actors, strategy='smart')

    # 方式2: 批量替换所有关卡
    # replacer.batch_replace_in_folder('/Game/Maps')

    # 生成报告
    report_path = replacer.generate_html_report()
    unreal.log(f"完成! 报告: {report_path}")


if __name__ == '__main__':
    main()
