#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
PerformanceAnalyzer.py - 性能分析工具
检测过多的动态光源、统计多边形数量、生成优化建议报告
"""

import unreal
import json
import os
from datetime import datetime
from collections import defaultdict


class PerformanceAnalyzer:
    """性能分析器 - 分析关卡性能并提供优化建议"""

    def __init__(self):
        self.editor_util = unreal.EditorLevelLibrary()
        self.editor_asset_lib = unreal.EditorAssetLibrary()
        self.analysis_data = {
            'timestamp': datetime.now().isoformat(),
            'lighting': {},
            'meshes': {},
            'materials': {},
            'particles': {},
            'audio': {},
            'warnings': [],
            'recommendations': []
        }

        # 性能阈值配置
        self.thresholds = {
            'max_dynamic_lights': 10,
            'max_point_lights': 20,
            'max_spotlights': 15,
            'max_triangles_per_mesh': 50000,
            'max_total_triangles': 5000000,
            'max_draw_calls': 2000,
            'max_texture_size': 2048,
            'max_particles_per_emitter': 100
        }

    def analyze_current_level(self):
        """分析当前关卡"""
        unreal.log("开始性能分析...")

        self._analyze_lighting()
        self._analyze_meshes()
        self._analyze_materials()
        self._analyze_particles()
        self._analyze_audio()
        self._generate_recommendations()

        unreal.log("性能分析完成!")
        return self.analysis_data

    def _analyze_lighting(self):
        """分析光照性能"""
        all_actors = self.editor_util.get_all_level_actors()

        lighting_stats = {
            'total_lights': 0,
            'dynamic_lights': 0,
            'static_lights': 0,
            'stationary_lights': 0,
            'point_lights': 0,
            'spot_lights': 0,
            'directional_lights': 0,
            'rect_lights': 0,
            'high_intensity_lights': [],
            'overlapping_lights': []
        }

        light_actors = []

        for actor in all_actors:
            if isinstance(actor, unreal.Light):
                lighting_stats['total_lights'] += 1
                light_component = actor.get_editor_property('light_component')

                if light_component:
                    # 检查光源类型
                    mobility = light_component.get_editor_property('mobility')

                    if mobility == unreal.ComponentMobility.MOVABLE:
                        lighting_stats['dynamic_lights'] += 1
                    elif mobility == unreal.ComponentMobility.STATIC:
                        lighting_stats['static_lights'] += 1
                    elif mobility == unreal.ComponentMobility.STATIONARY:
                        lighting_stats['stationary_lights'] += 1

                    # 统计光源种类
                    if isinstance(actor, unreal.PointLight):
                        lighting_stats['point_lights'] += 1
                    elif isinstance(actor, unreal.SpotLight):
                        lighting_stats['spot_lights'] += 1
                    elif isinstance(actor, unreal.DirectionalLight):
                        lighting_stats['directional_lights'] += 1
                    elif isinstance(actor, unreal.RectLight):
                        lighting_stats['rect_lights'] += 1

                    # 检查高强度光源
                    intensity = light_component.get_editor_property('intensity')
                    if intensity > 10000:
                        lighting_stats['high_intensity_lights'].append({
                            'name': actor.get_name(),
                            'intensity': intensity,
                            'location': str(actor.get_actor_location())
                        })

                    light_actors.append({
                        'actor': actor,
                        'location': actor.get_actor_location(),
                        'radius': self._get_light_radius(light_component)
                    })

        # 检测重叠光源
        lighting_stats['overlapping_lights'] = self._detect_overlapping_lights(light_actors)

        self.analysis_data['lighting'] = lighting_stats

        # 生成警告
        if lighting_stats['dynamic_lights'] > self.thresholds['max_dynamic_lights']:
            self.analysis_data['warnings'].append({
                'category': 'Lighting',
                'severity': 'High',
                'message': f"动态光源过多: {lighting_stats['dynamic_lights']} (建议 < {self.thresholds['max_dynamic_lights']})"
            })

        if lighting_stats['point_lights'] > self.thresholds['max_point_lights']:
            self.analysis_data['warnings'].append({
                'category': 'Lighting',
                'severity': 'Medium',
                'message': f"点光源过多: {lighting_stats['point_lights']} (建议 < {self.thresholds['max_point_lights']})"
            })

    def _get_light_radius(self, light_component):
        """获取光源影响半径"""
        try:
            if hasattr(light_component, 'attenuation_radius'):
                return light_component.get_editor_property('attenuation_radius')
            return 1000.0  # 默认半径
        except:
            return 1000.0

    def _detect_overlapping_lights(self, light_actors):
        """检测重叠的光源"""
        overlapping = []

        for i, light1 in enumerate(light_actors):
            for light2 in light_actors[i+1:]:
                distance = (light1['location'] - light2['location']).length()
                combined_radius = light1['radius'] + light2['radius']

                if distance < combined_radius * 0.5:  # 50%重叠
                    overlapping.append({
                        'light1': light1['actor'].get_name(),
                        'light2': light2['actor'].get_name(),
                        'distance': distance,
                        'overlap_percentage': (1 - distance / combined_radius) * 100
                    })

        return overlapping

    def _analyze_meshes(self):
        """分析网格性能"""
        all_actors = self.editor_util.get_all_level_actors()

        mesh_stats = {
            'total_static_meshes': 0,
            'total_triangles': 0,
            'total_vertices': 0,
            'high_poly_meshes': [],
            'mesh_complexity': defaultdict(int),
            'lod_status': {
                'with_lods': 0,
                'without_lods': 0
            }
        }

        for actor in all_actors:
            if isinstance(actor, unreal.StaticMeshActor):
                mesh_component = actor.static_mesh_component
                if mesh_component and mesh_component.static_mesh:
                    mesh = mesh_component.static_mesh
                    mesh_stats['total_static_meshes'] += 1

                    # 获取三角形数量
                    try:
                        render_data = mesh.get_editor_property('render_data')
                        if render_data:
                            lod0 = render_data.lod_resources[0]
                            triangles = lod0.get_num_triangles()
                            vertices = lod0.get_num_vertices()

                            mesh_stats['total_triangles'] += triangles
                            mesh_stats['total_vertices'] += vertices

                            # 检查高多边形网格
                            if triangles > self.thresholds['max_triangles_per_mesh']:
                                mesh_stats['high_poly_meshes'].append({
                                    'actor': actor.get_name(),
                                    'mesh': mesh.get_name(),
                                    'triangles': triangles,
                                    'location': str(actor.get_actor_location())
                                })

                            # 统计复杂度分布
                            if triangles < 1000:
                                mesh_stats['mesh_complexity']['low'] += 1
                            elif triangles < 10000:
                                mesh_stats['mesh_complexity']['medium'] += 1
                            else:
                                mesh_stats['mesh_complexity']['high'] += 1

                            # 检查LOD
                            num_lods = mesh.get_num_lods()
                            if num_lods > 1:
                                mesh_stats['lod_status']['with_lods'] += 1
                            else:
                                mesh_stats['lod_status']['without_lods'] += 1
                    except:
                        pass

        self.analysis_data['meshes'] = dict(mesh_stats)

        # 生成警告
        if mesh_stats['total_triangles'] > self.thresholds['max_total_triangles']:
            self.analysis_data['warnings'].append({
                'category': 'Meshes',
                'severity': 'High',
                'message': f"总三角形数过多: {mesh_stats['total_triangles']:,} (建议 < {self.thresholds['max_total_triangles']:,})"
            })

        if mesh_stats['lod_status']['without_lods'] > mesh_stats['total_static_meshes'] * 0.5:
            self.analysis_data['warnings'].append({
                'category': 'Meshes',
                'severity': 'Medium',
                'message': f"超过50%的网格缺少LOD: {mesh_stats['lod_status']['without_lods']}/{mesh_stats['total_static_meshes']}"
            })

    def _analyze_materials(self):
        """分析材质性能"""
        all_actors = self.editor_util.get_all_level_actors()

        material_stats = {
            'total_materials': 0,
            'unique_materials': set(),
            'complex_materials': [],
            'texture_stats': {
                'total_textures': 0,
                'large_textures': []
            }
        }

        for actor in all_actors:
            if isinstance(actor, unreal.StaticMeshActor):
                mesh_component = actor.static_mesh_component
                if mesh_component:
                    materials = mesh_component.get_materials()

                    for mat in materials:
                        if mat:
                            material_stats['total_materials'] += 1
                            mat_path = mat.get_path_name()
                            material_stats['unique_materials'].add(mat_path)

                            # 检查材质复杂度（通过纹理数量估算）
                            textures = self._get_material_textures(mat)
                            if len(textures) > 5:
                                material_stats['complex_materials'].append({
                                    'material': mat.get_name(),
                                    'texture_count': len(textures),
                                    'actor': actor.get_name()
                                })

        material_stats['unique_materials'] = len(material_stats['unique_materials'])
        self.analysis_data['materials'] = material_stats

    def _get_material_textures(self, material):
        """获取材质使用的纹理"""
        textures = []
        try:
            # 这里简化处理，实际需要遍历材质表达式
            # 在实际使用中需要更复杂的逻辑
            pass
        except:
            pass
        return textures

    def _analyze_particles(self):
        """分析粒子系统性能"""
        all_actors = self.editor_util.get_all_level_actors()

        particle_stats = {
            'total_emitters': 0,
            'high_count_emitters': []
        }

        for actor in all_actors:
            if isinstance(actor, unreal.Emitter):
                particle_stats['total_emitters'] += 1
                # 这里可以添加更详细的粒子分析

        self.analysis_data['particles'] = particle_stats

    def _analyze_audio(self):
        """分析音频性能"""
        all_actors = self.editor_util.get_all_level_actors()

        audio_stats = {
            'total_audio_actors': 0,
            'ambient_sounds': 0,
            'audio_volumes': 0
        }

        for actor in all_actors:
            if isinstance(actor, unreal.AmbientSound):
                audio_stats['total_audio_actors'] += 1
                audio_stats['ambient_sounds'] += 1
            elif isinstance(actor, unreal.AudioVolume):
                audio_stats['audio_volumes'] += 1

        self.analysis_data['audio'] = audio_stats

    def _generate_recommendations(self):
        """生成优化建议"""
        recommendations = []

        lighting = self.analysis_data['lighting']
        meshes = self.analysis_data['meshes']

        # 光照优化建议
        if lighting['dynamic_lights'] > 5:
            recommendations.append({
                'category': 'Lighting',
                'priority': 'High',
                'title': '减少动态光源',
                'description': f"当前有 {lighting['dynamic_lights']} 个动态光源。建议将部分转换为静态或固定光源。",
                'impact': '可提升 15-30% 性能'
            })

        if len(lighting['overlapping_lights']) > 0:
            recommendations.append({
                'category': 'Lighting',
                'priority': 'Medium',
                'title': '优化重叠光源',
                'description': f"检测到 {len(lighting['overlapping_lights'])} 组重叠光源。考虑合并或调整位置。",
                'impact': '可减少光照计算开销'
            })

        # 网格优化建议
        if len(meshes.get('high_poly_meshes', [])) > 0:
            recommendations.append({
                'category': 'Meshes',
                'priority': 'High',
                'title': '优化高多边形网格',
                'description': f"发现 {len(meshes['high_poly_meshes'])} 个高多边形网格。建议添加LOD或简化模型。",
                'impact': '可显著提升渲染性能'
            })

        if meshes.get('lod_status', {}).get('without_lods', 0) > 10:
            recommendations.append({
                'category': 'Meshes',
                'priority': 'Medium',
                'title': '添加LOD层级',
                'description': f"{meshes['lod_status']['without_lods']} 个网格缺少LOD。建议为远距离物体添加LOD。",
                'impact': '可提升远景渲染性能'
            })

        # 通用建议
        recommendations.append({
            'category': 'General',
            'priority': 'Low',
            'title': '启用遮挡剔除',
            'description': '确保场景中正确放置了遮挡体积（Occlusion Volumes）。',
            'impact': '可减少不可见物体的渲染'
        })

        recommendations.append({
            'category': 'General',
            'priority': 'Low',
            'title': '使用实例化网格',
            'description': '对于重复出现的物体，使用Instanced Static Mesh可以大幅提升性能。',
            'impact': '可减少Draw Call数量'
        })

        self.analysis_data['recommendations'] = recommendations

    def generate_html_report(self, output_path=None):
        """生成HTML性能报告"""
        if not output_path:
            project_dir = unreal.Paths.project_dir()
            output_path = os.path.join(
                project_dir,
                'Saved',
                'Reports',
                f'PerformanceAnalysis_{datetime.now().strftime("%Y%m%d_%H%M%S")}.html'
            )

        os.makedirs(os.path.dirname(output_path), exist_ok=True)

        html_content = self._generate_html_content()

        with open(output_path, 'w', encoding='utf-8') as f:
            f.write(html_content)

        unreal.log(f"性能报告已生成: {output_path}")
        return output_path

    def _generate_html_content(self):
        """生成HTML内容"""
        lighting = self.analysis_data['lighting']
        meshes = self.analysis_data['meshes']
        warnings = self.analysis_data['warnings']
        recommendations = self.analysis_data['recommendations']

        html = f"""<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>性能分析报告 - HorrorProject</title>
    <style>
        * {{ margin: 0; padding: 0; box-sizing: border-box; }}
        body {{
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #f093fb 0%, #f5576c 100%);
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
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 40px;
            text-align: center;
        }}
        .header h1 {{
            font-size: 2.5em;
            margin-bottom: 10px;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
        }}
        .stats-grid {{
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 20px;
            padding: 40px;
            background: #f8f9fa;
        }}
        .stat-card {{
            background: white;
            padding: 20px;
            border-radius: 10px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
            text-align: center;
        }}
        .stat-card .number {{
            font-size: 2.5em;
            font-weight: bold;
            color: #667eea;
        }}
        .stat-card .label {{
            color: #666;
            margin-top: 10px;
        }}
        .section {{
            padding: 40px;
        }}
        .section h2 {{
            color: #1e3c72;
            margin-bottom: 20px;
            padding-bottom: 10px;
            border-bottom: 3px solid #667eea;
        }}
        .warning-box {{
            background: #fff3cd;
            border-left: 4px solid #ffc107;
            padding: 15px;
            margin: 10px 0;
            border-radius: 4px;
        }}
        .warning-box.high {{
            background: #f8d7da;
            border-left-color: #dc3545;
        }}
        .warning-box.medium {{
            background: #fff3cd;
            border-left-color: #ffc107;
        }}
        .recommendation {{
            background: #d1ecf1;
            border-left: 4px solid #17a2b8;
            padding: 15px;
            margin: 10px 0;
            border-radius: 4px;
        }}
        .recommendation h3 {{
            color: #0c5460;
            margin-bottom: 10px;
        }}
        .recommendation .impact {{
            color: #28a745;
            font-weight: bold;
            margin-top: 10px;
        }}
        table {{
            width: 100%;
            border-collapse: collapse;
            margin-top: 20px;
        }}
        th {{
            background: #667eea;
            color: white;
            padding: 12px;
            text-align: left;
        }}
        td {{
            padding: 10px;
            border-bottom: 1px solid #ddd;
        }}
        tr:hover {{
            background: #f5f5f5;
        }}
        .chart {{
            margin: 20px 0;
            padding: 20px;
            background: #f8f9fa;
            border-radius: 8px;
        }}
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>⚡ 性能分析报告</h1>
            <div class="timestamp">生成时间: {self.analysis_data['timestamp']}</div>
        </div>

        <div class="stats-grid">
            <div class="stat-card">
                <div class="number">{lighting['total_lights']}</div>
                <div class="label">总光源数</div>
            </div>
            <div class="stat-card">
                <div class="number">{lighting['dynamic_lights']}</div>
                <div class="label">动态光源</div>
            </div>
            <div class="stat-card">
                <div class="number">{meshes.get('total_static_meshes', 0)}</div>
                <div class="label">静态网格</div>
            </div>
            <div class="stat-card">
                <div class="number">{meshes.get('total_triangles', 0):,}</div>
                <div class="label">总三角形数</div>
            </div>
            <div class="stat-card">
                <div class="number">{len(warnings)}</div>
                <div class="label">警告数量</div>
            </div>
            <div class="stat-card">
                <div class="number">{len(recommendations)}</div>
                <div class="label">优化建议</div>
            </div>
        </div>

        <div class="section">
            <h2>🔴 性能警告</h2>
"""

        if warnings:
            for warning in warnings:
                severity_class = warning['severity'].lower()
                html += f"""
            <div class="warning-box {severity_class}">
                <strong>[{warning['severity']}] {warning['category']}</strong>: {warning['message']}
            </div>
"""
        else:
            html += '<p>未发现性能问题！</p>'

        html += """
        </div>

        <div class="section">
            <h2>💡 优化建议</h2>
"""

        for rec in recommendations:
            html += f"""
            <div class="recommendation">
                <h3>[{rec['priority']}] {rec['title']}</h3>
                <p>{rec['description']}</p>
                <div class="impact">📈 {rec['impact']}</div>
            </div>
"""

        html += f"""
        </div>

        <div class="section">
            <h2>💡 光照详情</h2>
            <div class="chart">
                <p><strong>静态光源:</strong> {lighting['static_lights']}</p>
                <p><strong>固定光源:</strong> {lighting['stationary_lights']}</p>
                <p><strong>动态光源:</strong> {lighting['dynamic_lights']}</p>
                <p><strong>点光源:</strong> {lighting['point_lights']}</p>
                <p><strong>聚光灯:</strong> {lighting['spot_lights']}</p>
                <p><strong>方向光:</strong> {lighting['directional_lights']}</p>
            </div>
"""

        if lighting.get('high_intensity_lights'):
            html += """
            <h3>高强度光源</h3>
            <table>
                <tr>
                    <th>名称</th>
                    <th>强度</th>
                    <th>位置</th>
                </tr>
"""
            for light in lighting['high_intensity_lights']:
                html += f"""
                <tr>
                    <td>{light['name']}</td>
                    <td>{light['intensity']}</td>
                    <td>{light['location']}</td>
                </tr>
"""
            html += "</table>"

        html += """
        </div>

        <div class="section">
            <h2>🎨 网格详情</h2>
            <div class="chart">
"""

        mesh_complexity = meshes.get('mesh_complexity', {})
        html += f"""
                <p><strong>低复杂度网格:</strong> {mesh_complexity.get('low', 0)}</p>
                <p><strong>中复杂度网格:</strong> {mesh_complexity.get('medium', 0)}</p>
                <p><strong>高复杂度网格:</strong> {mesh_complexity.get('high', 0)}</p>
                <p><strong>带LOD的网格:</strong> {meshes.get('lod_status', {}).get('with_lods', 0)}</p>
                <p><strong>无LOD的网格:</strong> {meshes.get('lod_status', {}).get('without_lods', 0)}</p>
            </div>
"""

        if meshes.get('high_poly_meshes'):
            html += """
            <h3>高多边形网格</h3>
            <table>
                <tr>
                    <th>Actor</th>
                    <th>网格</th>
                    <th>三角形数</th>
                    <th>位置</th>
                </tr>
"""
            for mesh in meshes['high_poly_meshes'][:20]:  # 只显示前20个
                html += f"""
                <tr>
                    <td>{mesh['actor']}</td>
                    <td>{mesh['mesh']}</td>
                    <td>{mesh['triangles']:,}</td>
                    <td>{mesh['location']}</td>
                </tr>
"""
            html += "</table>"

        html += """
        </div>

        <div class="footer" style="background: #2c3e50; color: white; padding: 20px; text-align: center;">
            <p>HorrorProject - Performance Analyzer v1.0</p>
        </div>
    </div>
</body>
</html>
"""
        return html


def main():
    """主函数"""
    analyzer = PerformanceAnalyzer()
    analyzer.analyze_current_level()
    report_path = analyzer.generate_html_report()
    unreal.log(f"分析完成! 报告: {report_path}")


if __name__ == '__main__':
    main()
