#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
LevelValidator.py - 关卡验证工具
检查必需元素（Player Start、Nav Mesh、目标）
验证音频配置、检测常见错误（重叠物体、缺失引用）
"""

import unreal
import json
import os
from datetime import datetime
from collections import defaultdict


class LevelValidator:
    """关卡验证器 - 检查关卡完整性和常见错误"""

    def __init__(self):
        self.editor_util = unreal.EditorLevelLibrary()
        self.editor_asset_lib = unreal.EditorAssetLibrary()
        self.validation_data = {
            'timestamp': datetime.now().isoformat(),
            'level_name': '',
            'required_elements': {},
            'audio_validation': {},
            'collision_issues': [],
            'missing_references': [],
            'errors': [],
            'warnings': [],
            'info': [],
            'validation_passed': False
        }

    def validate_current_level(self):
        """验证当前关卡"""
        level_name = unreal.EditorLevelLibrary.get_editor_world().get_name()
        self.validation_data['level_name'] = level_name
        unreal.log(f"开始验证关卡: {level_name}")

        # 执行各项验证
        self._validate_required_elements()
        self._validate_navigation()
        self._validate_audio()
        self._validate_lighting()
        self._validate_collision()
        self._validate_references()
        self._validate_gameplay_elements()

        # 判断是否通过验证
        self.validation_data['validation_passed'] = len(self.validation_data['errors']) == 0

        unreal.log(f"验证完成! 错误: {len(self.validation_data['errors'])}, "
                   f"警告: {len(self.validation_data['warnings'])}")

        return self.validation_data

    def _validate_required_elements(self):
        """验证必需元素"""
        all_actors = self.editor_util.get_all_level_actors()

        required = {
            'player_start': False,
            'player_start_count': 0,
            'game_mode': False,
            'post_process_volume': False,
            'light_source': False,
            'audio_volume': False
        }

        for actor in all_actors:
            if isinstance(actor, unreal.PlayerStart):
                required['player_start'] = True
                required['player_start_count'] += 1
            elif isinstance(actor, unreal.GameModeBase):
                required['game_mode'] = True
            elif isinstance(actor, unreal.PostProcessVolume):
                required['post_process_volume'] = True
            elif isinstance(actor, unreal.Light):
                required['light_source'] = True
            elif isinstance(actor, unreal.AudioVolume):
                required['audio_volume'] = True

        self.validation_data['required_elements'] = required

        # 生成错误和警告
        if not required['player_start']:
            self.validation_data['errors'].append({
                'category': 'Required Elements',
                'severity': 'Critical',
                'message': '缺少 Player Start！玩家无法生成。'
            })
        elif required['player_start_count'] > 1:
            self.validation_data['warnings'].append({
                'category': 'Required Elements',
                'message': f'发现 {required["player_start_count"]} 个 Player Start，确保这是预期的。'
            })

        if not required['light_source']:
            self.validation_data['warnings'].append({
                'category': 'Required Elements',
                'message': '关卡中没有光源，场景可能完全黑暗。'
            })

        if not required['post_process_volume']:
            self.validation_data['info'].append({
                'category': 'Required Elements',
                'message': '建议添加 Post Process Volume 以增强视觉效果。'
            })

    def _validate_navigation(self):
        """验证导航网格"""
        all_actors = self.editor_util.get_all_level_actors()

        nav_mesh_found = False
        nav_mesh_bounds_found = False

        for actor in all_actors:
            if isinstance(actor, unreal.NavMeshBoundsVolume):
                nav_mesh_found = True
                nav_mesh_bounds_found = True
                break

        if not nav_mesh_found:
            self.validation_data['warnings'].append({
                'category': 'Navigation',
                'message': '未找到 Nav Mesh Bounds Volume。AI 导航可能无法正常工作。'
            })
        else:
            self.validation_data['info'].append({
                'category': 'Navigation',
                'message': '✓ 导航网格配置正确'
            })

    def _validate_audio(self):
        """验证音频配置"""
        all_actors = self.editor_util.get_all_level_actors()

        audio_stats = {
            'ambient_sounds': 0,
            'audio_volumes': 0,
            'sound_cues': 0,
            'missing_sound_assets': []
        }

        for actor in all_actors:
            if isinstance(actor, unreal.AmbientSound):
                audio_stats['ambient_sounds'] += 1

                # 检查音频组件
                audio_component = actor.get_editor_property('audio_component')
                if audio_component:
                    sound = audio_component.get_editor_property('sound')
                    if not sound:
                        audio_stats['missing_sound_assets'].append({
                            'actor': actor.get_name(),
                            'location': str(actor.get_actor_location())
                        })
                else:
                    audio_stats['missing_sound_assets'].append({
                        'actor': actor.get_name(),
                        'location': str(actor.get_actor_location())
                    })

            elif isinstance(actor, unreal.AudioVolume):
                audio_stats['audio_volumes'] += 1

        self.validation_data['audio_validation'] = audio_stats

        # 生成警告
        if audio_stats['missing_sound_assets']:
            self.validation_data['warnings'].append({
                'category': 'Audio',
                'message': f'{len(audio_stats["missing_sound_assets"])} 个音频Actor缺少声音资产'
            })

        if audio_stats['ambient_sounds'] == 0:
            self.validation_data['info'].append({
                'category': 'Audio',
                'message': '关卡中没有环境音效，考虑添加以增强氛围。'
            })

    def _validate_lighting(self):
        """验证光照设置"""
        all_actors = self.editor_util.get_all_level_actors()

        lighting_issues = []
        directional_light_count = 0

        for actor in all_actors:
            if isinstance(actor, unreal.DirectionalLight):
                directional_light_count += 1

                light_component = actor.get_editor_property('light_component')
                if light_component:
                    # 检查是否启用了阴影
                    cast_shadows = light_component.get_editor_property('cast_shadows')
                    if not cast_shadows:
                        lighting_issues.append({
                            'actor': actor.get_name(),
                            'issue': '方向光未启用阴影'
                        })

        if directional_light_count == 0:
            self.validation_data['warnings'].append({
                'category': 'Lighting',
                'message': '没有方向光（太阳光），室外场景可能需要添加。'
            })
        elif directional_light_count > 1:
            self.validation_data['warnings'].append({
                'category': 'Lighting',
                'message': f'发现 {directional_light_count} 个方向光，通常只需要一个。'
            })

        if lighting_issues:
            for issue in lighting_issues:
                self.validation_data['warnings'].append({
                    'category': 'Lighting',
                    'message': f"{issue['actor']}: {issue['issue']}"
                })

    def _validate_collision(self):
        """验证碰撞和重叠问题"""
        all_actors = self.editor_util.get_all_level_actors()

        collision_issues = []
        static_mesh_actors = []

        # 收集所有静态网格Actor
        for actor in all_actors:
            if isinstance(actor, unreal.StaticMeshActor):
                mesh_component = actor.static_mesh_component
                if mesh_component and mesh_component.static_mesh:
                    static_mesh_actors.append({
                        'actor': actor,
                        'location': actor.get_actor_location(),
                        'bounds': actor.get_actor_bounds(False)
                    })

        # 检测重叠（简化版本 - 只检查位置非常接近的物体）
        for i, actor1 in enumerate(static_mesh_actors):
            for actor2 in static_mesh_actors[i+1:]:
                distance = (actor1['location'] - actor2['location']).length()

                if distance < 10.0:  # 10cm内认为可能重叠
                    collision_issues.append({
                        'actor1': actor1['actor'].get_name(),
                        'actor2': actor2['actor'].get_name(),
                        'distance': distance,
                        'location': str(actor1['location'])
                    })

        self.validation_data['collision_issues'] = collision_issues

        if len(collision_issues) > 0:
            self.validation_data['warnings'].append({
                'category': 'Collision',
                'message': f'发现 {len(collision_issues)} 组可能重叠的物体'
            })

    def _validate_references(self):
        """验证资产引用"""
        all_actors = self.editor_util.get_all_level_actors()

        missing_refs = []

        for actor in all_actors:
            # 检查静态网格引用
            if isinstance(actor, unreal.StaticMeshActor):
                mesh_component = actor.static_mesh_component
                if mesh_component:
                    mesh = mesh_component.static_mesh
                    if not mesh:
                        missing_refs.append({
                            'actor': actor.get_name(),
                            'type': 'Static Mesh',
                            'location': str(actor.get_actor_location())
                        })

                    # 检查材质引用
                    materials = mesh_component.get_materials()
                    for i, mat in enumerate(materials):
                        if not mat:
                            missing_refs.append({
                                'actor': actor.get_name(),
                                'type': f'Material Slot {i}',
                                'location': str(actor.get_actor_location())
                            })

            # 检查蓝图引用
            elif actor.get_class().get_name().startswith('BP_'):
                # 这里可以添加更详细的蓝图验证
                pass

        self.validation_data['missing_references'] = missing_refs

        if missing_refs:
            self.validation_data['errors'].append({
                'category': 'References',
                'severity': 'High',
                'message': f'发现 {len(missing_refs)} 个缺失的资产引用'
            })

    def _validate_gameplay_elements(self):
        """验证游戏玩法元素"""
        all_actors = self.editor_util.get_all_level_actors()

        gameplay_elements = {
            'triggers': 0,
            'volumes': 0,
            'objectives': 0,
            'spawn_points': 0
        }

        for actor in all_actors:
            actor_name = actor.get_name().lower()

            if isinstance(actor, unreal.TriggerBox) or isinstance(actor, unreal.TriggerVolume):
                gameplay_elements['triggers'] += 1
            elif isinstance(actor, unreal.Volume):
                gameplay_elements['volumes'] += 1
            elif 'objective' in actor_name or 'goal' in actor_name:
                gameplay_elements['objectives'] += 1
            elif 'spawn' in actor_name:
                gameplay_elements['spawn_points'] += 1

        # 对于恐怖游戏，检查特定元素
        horror_elements = {
            'jump_scares': 0,
            'enemy_spawns': 0,
            'safe_zones': 0
        }

        for actor in all_actors:
            actor_name = actor.get_name().lower()
            if 'scare' in actor_name or 'jumpscare' in actor_name:
                horror_elements['jump_scares'] += 1
            elif 'enemy' in actor_name or 'monster' in actor_name:
                horror_elements['enemy_spawns'] += 1
            elif 'safe' in actor_name:
                horror_elements['safe_zones'] += 1

        if gameplay_elements['triggers'] == 0:
            self.validation_data['info'].append({
                'category': 'Gameplay',
                'message': '没有触发器，考虑添加以实现交互和事件。'
            })

        if horror_elements['jump_scares'] == 0 and horror_elements['enemy_spawns'] == 0:
            self.validation_data['info'].append({
                'category': 'Gameplay',
                'message': '未检测到恐怖元素（惊吓点或敌人生成点）。'
            })

    def generate_html_report(self, output_path=None):
        """生成HTML验证报告"""
        if not output_path:
            project_dir = unreal.Paths.project_dir()
            output_path = os.path.join(
                project_dir,
                'Saved',
                'Reports',
                f'LevelValidation_{datetime.now().strftime("%Y%m%d_%H%M%S")}.html'
            )

        os.makedirs(os.path.dirname(output_path), exist_ok=True)

        html_content = self._generate_html_content()

        with open(output_path, 'w', encoding='utf-8') as f:
            f.write(html_content)

        unreal.log(f"验证报告已生成: {output_path}")
        return output_path

    def _generate_html_content(self):
        """生成HTML内容"""
        required = self.validation_data['required_elements']
        audio = self.validation_data['audio_validation']
        errors = self.validation_data['errors']
        warnings = self.validation_data['warnings']
        info = self.validation_data['info']
        collision_issues = self.validation_data['collision_issues']
        missing_refs = self.validation_data['missing_references']

        validation_status = "✅ 通过" if self.validation_data['validation_passed'] else "❌ 未通过"
        status_color = "#28a745" if self.validation_data['validation_passed'] else "#dc3545"

        html = f"""<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>关卡验证报告 - HorrorProject</title>
    <style>
        * {{ margin: 0; padding: 0; box-sizing: border-box; }}
        body {{
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #4facfe 0%, #00f2fe 100%);
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
        .validation-status {{
            font-size: 2em;
            margin-top: 20px;
            padding: 20px;
            background: rgba(255,255,255,0.2);
            border-radius: 10px;
            color: {status_color};
            font-weight: bold;
        }}
        .summary {{
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 20px;
            padding: 40px;
            background: #f8f9fa;
        }}
        .summary-card {{
            background: white;
            padding: 20px;
            border-radius: 10px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
            text-align: center;
        }}
        .summary-card .number {{
            font-size: 2.5em;
            font-weight: bold;
            margin-bottom: 10px;
        }}
        .summary-card.error .number {{ color: #dc3545; }}
        .summary-card.warning .number {{ color: #ffc107; }}
        .summary-card.info .number {{ color: #17a2b8; }}
        .summary-card.success .number {{ color: #28a745; }}
        .section {{
            padding: 40px;
        }}
        .section h2 {{
            color: #1e3c72;
            margin-bottom: 20px;
            padding-bottom: 10px;
            border-bottom: 3px solid #667eea;
        }}
        .issue-box {{
            padding: 15px;
            margin: 10px 0;
            border-radius: 8px;
            border-left: 4px solid;
        }}
        .issue-box.error {{
            background: #f8d7da;
            border-left-color: #dc3545;
        }}
        .issue-box.warning {{
            background: #fff3cd;
            border-left-color: #ffc107;
        }}
        .issue-box.info {{
            background: #d1ecf1;
            border-left-color: #17a2b8;
        }}
        .issue-box.success {{
            background: #d4edda;
            border-left-color: #28a745;
        }}
        .checklist {{
            list-style: none;
            padding: 20px;
        }}
        .checklist li {{
            padding: 10px;
            margin: 5px 0;
            background: #f8f9fa;
            border-radius: 5px;
        }}
        .checklist li.pass::before {{
            content: "✅ ";
            margin-right: 10px;
        }}
        .checklist li.fail::before {{
            content: "❌ ";
            margin-right: 10px;
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
            <h1>🎯 关卡验证报告</h1>
            <div>关卡: {self.validation_data['level_name']}</div>
            <div class="timestamp">生成时间: {self.validation_data['timestamp']}</div>
            <div class="validation-status">{validation_status}</div>
        </div>

        <div class="summary">
            <div class="summary-card error">
                <div class="number">{len(errors)}</div>
                <div class="label">错误</div>
            </div>
            <div class="summary-card warning">
                <div class="number">{len(warnings)}</div>
                <div class="label">警告</div>
            </div>
            <div class="summary-card info">
                <div class="number">{len(info)}</div>
                <div class="label">提示</div>
            </div>
            <div class="summary-card success">
                <div class="number">{len(collision_issues)}</div>
                <div class="label">碰撞问题</div>
            </div>
        </div>

        <div class="section">
            <h2>📋 必需元素检查</h2>
            <ul class="checklist">
                <li class="{'pass' if required.get('player_start') else 'fail'}">
                    Player Start ({required.get('player_start_count', 0)} 个)
                </li>
                <li class="{'pass' if required.get('light_source') else 'fail'}">
                    光源
                </li>
                <li class="{'pass' if required.get('post_process_volume') else 'fail'}">
                    Post Process Volume
                </li>
                <li class="{'pass' if required.get('audio_volume') else 'fail'}">
                    Audio Volume
                </li>
            </ul>
        </div>
"""

        # 错误部分
        if errors:
            html += """
        <div class="section">
            <h2>🔴 错误</h2>
"""
            for error in errors:
                html += f"""
            <div class="issue-box error">
                <strong>[{error.get('severity', 'Error')}] {error['category']}</strong>: {error['message']}
            </div>
"""
            html += "</div>"

        # 警告部分
        if warnings:
            html += """
        <div class="section">
            <h2>⚠️ 警告</h2>
"""
            for warning in warnings:
                html += f"""
            <div class="issue-box warning">
                <strong>{warning['category']}</strong>: {warning['message']}
            </div>
"""
            html += "</div>"

        # 音频验证
        html += f"""
        <div class="section">
            <h2>🔊 音频验证</h2>
            <div class="issue-box info">
                <p><strong>环境音效:</strong> {audio.get('ambient_sounds', 0)}</p>
                <p><strong>音频体积:</strong> {audio.get('audio_volumes', 0)}</p>
                <p><strong>缺失音频资产:</strong> {len(audio.get('missing_sound_assets', []))}</p>
            </div>
"""

        if audio.get('missing_sound_assets'):
            html += """
            <h3>缺失音频资产列表</h3>
            <table>
                <tr>
                    <th>Actor名称</th>
                    <th>位置</th>
                </tr>
"""
            for item in audio['missing_sound_assets']:
                html += f"""
                <tr>
                    <td>{item['actor']}</td>
                    <td>{item['location']}</td>
                </tr>
"""
            html += "</table>"

        html += "</div>"

        # 碰撞问题
        if collision_issues:
            html += f"""
        <div class="section">
            <h2>💥 碰撞问题 ({len(collision_issues)})</h2>
            <table>
                <tr>
                    <th>Actor 1</th>
                    <th>Actor 2</th>
                    <th>距离</th>
                    <th>位置</th>
                </tr>
"""
            for issue in collision_issues[:50]:  # 只显示前50个
                html += f"""
                <tr>
                    <td>{issue['actor1']}</td>
                    <td>{issue['actor2']}</td>
                    <td>{issue['distance']:.2f} cm</td>
                    <td>{issue['location']}</td>
                </tr>
"""
            html += """
            </table>
        </div>
"""

        # 缺失引用
        if missing_refs:
            html += f"""
        <div class="section">
            <h2>🔗 缺失引用 ({len(missing_refs)})</h2>
            <table>
                <tr>
                    <th>Actor</th>
                    <th>类型</th>
                    <th>位置</th>
                </tr>
"""
            for ref in missing_refs:
                html += f"""
                <tr>
                    <td>{ref['actor']}</td>
                    <td>{ref['type']}</td>
                    <td>{ref['location']}</td>
                </tr>
"""
            html += """
            </table>
        </div>
"""

        # 提示信息
        if info:
            html += """
        <div class="section">
            <h2>💡 建议</h2>
"""
            for item in info:
                html += f"""
            <div class="issue-box info">
                <strong>{item['category']}</strong>: {item['message']}
            </div>
"""
            html += "</div>"

        html += """
        <div class="footer">
            <p>HorrorProject - Level Validator v1.0</p>
        </div>
    </div>
</body>
</html>
"""
        return html


def main():
    """主函数"""
    validator = LevelValidator()
    validator.validate_current_level()
    report_path = validator.generate_html_report()
    unreal.log(f"验证完成! 报告: {report_path}")


if __name__ == '__main__':
    main()
