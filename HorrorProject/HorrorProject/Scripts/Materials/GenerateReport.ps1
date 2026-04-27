# GenerateReport.ps1 - 生成完整的材质分析报告
# 整合分析和优化建议，生成HTML报告

param(
    [string]$AnalysisPath = "",
    [string]$OutputPath = ""
)

$ErrorActionPreference = "Continue"

if ([string]::IsNullOrWhiteSpace($AnalysisPath)) {
    $AnalysisPath = Join-Path $PSScriptRoot "ShaderAnalysis.json"
}
if ([string]::IsNullOrWhiteSpace($OutputPath)) {
    $OutputPath = Join-Path $PSScriptRoot "MaterialReport.html"
}

Write-Host "=== 材质分析报告生成器 ===" -ForegroundColor Cyan

# 读取分析结果
if (-not (Test-Path $AnalysisPath)) {
    Write-Host "错误: 找不到分析文件 $AnalysisPath" -ForegroundColor Red
    exit 1
}

$analysis = Get-Content $AnalysisPath -Raw | ConvertFrom-Json

# 生成HTML报告
$html = @"
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>材质系统分析报告 - HorrorProject</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: #0a0a0a;
            color: #e0e0e0;
            padding: 20px;
            line-height: 1.6;
        }
        .container { max-width: 1400px; margin: 0 auto; }
        h1 {
            color: #00ff88;
            border-bottom: 3px solid #00ff88;
            padding-bottom: 10px;
            margin-bottom: 30px;
            font-size: 2.5em;
        }
        h2 {
            color: #00ccff;
            margin-top: 40px;
            margin-bottom: 20px;
            font-size: 1.8em;
        }
        h3 {
            color: #ffaa00;
            margin-top: 25px;
            margin-bottom: 15px;
            font-size: 1.3em;
        }
        .summary {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 20px;
            margin-bottom: 40px;
        }
        .card {
            background: linear-gradient(135deg, #1a1a2e 0%, #16213e 100%);
            border: 1px solid #00ff88;
            border-radius: 10px;
            padding: 20px;
            box-shadow: 0 4px 6px rgba(0, 255, 136, 0.1);
        }
        .card h3 { margin-top: 0; color: #00ff88; }
        .card .value {
            font-size: 2.5em;
            font-weight: bold;
            color: #00ccff;
            margin: 10px 0;
        }
        .chart {
            background: #1a1a2e;
            border: 1px solid #333;
            border-radius: 10px;
            padding: 20px;
            margin: 20px 0;
        }
        .bar-chart {
            display: flex;
            flex-direction: column;
            gap: 15px;
        }
        .bar-item {
            display: flex;
            align-items: center;
            gap: 15px;
        }
        .bar-label {
            min-width: 150px;
            font-weight: bold;
            color: #00ccff;
        }
        .bar-container {
            flex: 1;
            background: #0a0a0a;
            border-radius: 5px;
            height: 30px;
            position: relative;
            overflow: hidden;
        }
        .bar-fill {
            height: 100%;
            border-radius: 5px;
            display: flex;
            align-items: center;
            padding: 0 10px;
            color: white;
            font-weight: bold;
            transition: width 0.3s ease;
        }
        .bar-fill.very-high { background: linear-gradient(90deg, #ff0000, #cc0000); }
        .bar-fill.high { background: linear-gradient(90deg, #ff6600, #cc5200); }
        .bar-fill.medium { background: linear-gradient(90deg, #ffaa00, #cc8800); }
        .bar-fill.low { background: linear-gradient(90deg, #00ff88, #00cc66); }
        .table-container {
            overflow-x: auto;
            margin: 20px 0;
        }
        table {
            width: 100%;
            border-collapse: collapse;
            background: #1a1a2e;
            border-radius: 10px;
            overflow: hidden;
        }
        th {
            background: #00ff88;
            color: #0a0a0a;
            padding: 15px;
            text-align: left;
            font-weight: bold;
        }
        td {
            padding: 12px 15px;
            border-bottom: 1px solid #333;
        }
        tr:hover { background: #252540; }
        .badge {
            display: inline-block;
            padding: 5px 12px;
            border-radius: 15px;
            font-size: 0.85em;
            font-weight: bold;
        }
        .badge.very-high { background: #ff0000; color: white; }
        .badge.high { background: #ff6600; color: white; }
        .badge.medium { background: #ffaa00; color: black; }
        .badge.low { background: #00ff88; color: black; }
        .alert {
            background: #2a1a1a;
            border-left: 4px solid #ff0000;
            padding: 15px;
            margin: 20px 0;
            border-radius: 5px;
        }
        .alert.warning {
            background: #2a2a1a;
            border-left-color: #ffaa00;
        }
        .alert.info {
            background: #1a2a2a;
            border-left-color: #00ccff;
        }
        .alert.success {
            background: #1a2a1a;
            border-left-color: #00ff88;
        }
        .timestamp {
            color: #888;
            font-size: 0.9em;
            margin-top: 40px;
            text-align: center;
        }
        code {
            background: #0a0a0a;
            padding: 2px 6px;
            border-radius: 3px;
            color: #00ff88;
            font-family: 'Courier New', monospace;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🎨 材质系统分析报告</h1>

        <div class="alert info">
            <strong>📊 分析时间:</strong> $($analysis.Timestamp)<br>
            <strong>📁 项目路径:</strong> <code>$($analysis.ProjectPath)</code>
        </div>

        <h2>📈 执行摘要</h2>
        <div class="summary">
            <div class="card">
                <h3>总材质数</h3>
                <div class="value">$($analysis.Summary.TotalMaterials)</div>
                <p>项目中的所有材质文件</p>
            </div>
            <div class="card">
                <h3>主材质</h3>
                <div class="value">$($analysis.Summary.MasterMaterials)</div>
                <p>可复用的主材质模板</p>
            </div>
            <div class="card">
                <h3>材质实例</h3>
                <div class="value">$($analysis.Summary.MaterialInstances)</div>
                <p>基于主材质的实例</p>
            </div>
            <div class="card">
                <h3>材质函数</h3>
                <div class="value">$($analysis.Summary.MaterialFunctions)</div>
                <p>可复用的材质函数</p>
            </div>
            <div class="card">
                <h3>后处理材质</h3>
                <div class="value">$($analysis.Summary.PostProcessMaterials)</div>
                <p>性能关键的后处理</p>
            </div>
        </div>

        <h2>🔥 复杂度分布</h2>
        <div class="chart">
            <div class="bar-chart">
                <div class="bar-item">
                    <div class="bar-label">极高复杂度 (&gt;100KB)</div>
                    <div class="bar-container">
                        <div class="bar-fill very-high" style="width: $(($analysis.ComplexityDistribution.VeryHigh / $analysis.Summary.TotalMaterials * 100))%">
                            $($analysis.ComplexityDistribution.VeryHigh)
                        </div>
                    </div>
                </div>
                <div class="bar-item">
                    <div class="bar-label">高复杂度 (50-100KB)</div>
                    <div class="bar-container">
                        <div class="bar-fill high" style="width: $(($analysis.ComplexityDistribution.High / $analysis.Summary.TotalMaterials * 100))%">
                            $($analysis.ComplexityDistribution.High)
                        </div>
                    </div>
                </div>
                <div class="bar-item">
                    <div class="bar-label">中等复杂度 (20-50KB)</div>
                    <div class="bar-container">
                        <div class="bar-fill medium" style="width: $(($analysis.ComplexityDistribution.Medium / $analysis.Summary.TotalMaterials * 100))%">
                            $($analysis.ComplexityDistribution.Medium)
                        </div>
                    </div>
                </div>
                <div class="bar-item">
                    <div class="bar-label">低复杂度 (&lt;20KB)</div>
                    <div class="bar-container">
                        <div class="bar-fill low" style="width: $(($analysis.ComplexityDistribution.Low / $analysis.Summary.TotalMaterials * 100))%">
                            $($analysis.ComplexityDistribution.Low)
                        </div>
                    </div>
                </div>
            </div>
        </div>

        <div class="alert warning">
            <strong>⚠️ 优化建议:</strong> 发现 <strong>$($analysis.ComplexityDistribution.VeryHigh + $analysis.ComplexityDistribution.High)</strong> 个高复杂度材质需要优化。
            目标是将所有材质控制在 &lt;300 指令数和 &lt;16 纹理采样。
        </div>

        <h2>📂 类别分布</h2>
        <div class="chart">
            <div class="bar-chart">
                <div class="bar-item">
                    <div class="bar-label">后处理</div>
                    <div class="bar-container">
                        <div class="bar-fill high" style="width: $(($analysis.CategoryDistribution.PostProcess / $analysis.Summary.TotalMaterials * 100))%">
                            $($analysis.CategoryDistribution.PostProcess)
                        </div>
                    </div>
                </div>
                <div class="bar-item">
                    <div class="bar-label">建筑</div>
                    <div class="bar-container">
                        <div class="bar-fill medium" style="width: $(($analysis.CategoryDistribution.Architecture / $analysis.Summary.TotalMaterials * 100))%">
                            $($analysis.CategoryDistribution.Architecture)
                        </div>
                    </div>
                </div>
                <div class="bar-item">
                    <div class="bar-label">光照</div>
                    <div class="bar-container">
                        <div class="bar-fill medium" style="width: $(($analysis.CategoryDistribution.Lighting / $analysis.Summary.TotalMaterials * 100))%">
                            $($analysis.CategoryDistribution.Lighting)
                        </div>
                    </div>
                </div>
                <div class="bar-item">
                    <div class="bar-label">半透明</div>
                    <div class="bar-container">
                        <div class="bar-fill high" style="width: $(($analysis.CategoryDistribution.Translucent / $analysis.Summary.TotalMaterials * 100))%">
                            $($analysis.CategoryDistribution.Translucent)
                        </div>
                    </div>
                </div>
                <div class="bar-item">
                    <div class="bar-label">道具</div>
                    <div class="bar-container">
                        <div class="bar-fill low" style="width: $(($analysis.CategoryDistribution.Props / $analysis.Summary.TotalMaterials * 100))%">
                            $($analysis.CategoryDistribution.Props)
                        </div>
                    </div>
                </div>
                <div class="bar-item">
                    <div class="bar-label">角色</div>
                    <div class="bar-container">
                        <div class="bar-fill low" style="width: $(($analysis.CategoryDistribution.Character / $analysis.Summary.TotalMaterials * 100))%">
                            $($analysis.CategoryDistribution.Character)
                        </div>
                    </div>
                </div>
            </div>
        </div>

        <h2>🎯 优化优先级</h2>

        <div class="alert">
            <h3>🔴 高优先级 - 后处理材质</h3>
            <p>后处理材质对性能影响最大，需要立即优化。</p>
            <ul>
                <li>减少SceneTexture采样</li>
                <li>避免复杂循环和分支</li>
                <li>使用LUT替代复杂计算</li>
                <li>实现质量分级</li>
            </ul>
        </div>

        <div class="alert warning">
            <h3>🟡 中优先级 - 半透明材质</h3>
            <p>半透明材质会导致overdraw，需要谨慎优化。</p>
            <ul>
                <li>尽可能使用Masked代替Translucent</li>
                <li>减少半透明层数</li>
                <li>优化排序和渲染顺序</li>
            </ul>
        </div>

        <div class="alert success">
            <h3>🟢 标准化 - 主材质系统</h3>
            <p>创建统一的主材质系统，提高复用性和可维护性。</p>
            <ul>
                <li>M_Master_Opaque - 不透明主材质</li>
                <li>M_Master_Masked - 遮罩主材质</li>
                <li>M_Master_Translucent - 半透明主材质</li>
                <li>M_Master_Water - 水体主材质</li>
            </ul>
        </div>

        <h2>📋 性能目标</h2>
        <div class="table-container">
            <table>
                <thead>
                    <tr>
                        <th>材质类型</th>
                        <th>指令数目标</th>
                        <th>纹理采样目标</th>
                        <th>质量级别</th>
                    </tr>
                </thead>
                <tbody>
                    <tr>
                        <td>不透明材质</td>
                        <td>&lt; 200</td>
                        <td>&lt; 12</td>
                        <td>Low/Medium/High/Epic</td>
                    </tr>
                    <tr>
                        <td>遮罩材质</td>
                        <td>&lt; 220</td>
                        <td>&lt; 14</td>
                        <td>Low/Medium/High/Epic</td>
                    </tr>
                    <tr>
                        <td>半透明材质</td>
                        <td>&lt; 250</td>
                        <td>&lt; 14</td>
                        <td>Low/Medium/High/Epic</td>
                    </tr>
                    <tr>
                        <td>后处理材质</td>
                        <td>&lt; 300</td>
                        <td>&lt; 16</td>
                        <td>Low/Medium/High/Epic</td>
                    </tr>
                </tbody>
            </table>
        </div>

        <h2>🛠️ 下一步行动</h2>
        <div class="alert info">
            <h3>立即执行</h3>
            <ol>
                <li>创建主材质模板系统 (M_Master_*)</li>
                <li>创建材质函数库 (MF_*)</li>
                <li>优化前10个最复杂的材质</li>
            </ol>
        </div>

        <div class="alert info">
            <h3>短期目标 (1-2天)</h3>
            <ol>
                <li>迁移所有材质到新系统</li>
                <li>实现质量分级系统</li>
                <li>性能测试和验证</li>
            </ol>
        </div>

        <div class="alert info">
            <h3>长期目标 (1周)</h3>
            <ol>
                <li>完整的材质文档</li>
                <li>材质创建指南</li>
                <li>自动化优化工具</li>
                <li>持续性能监控</li>
            </ol>
        </div>

        <div class="timestamp">
            报告生成时间: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")<br>
            HorrorProject - 材质系统优化 - SM13团队
        </div>
    </div>
</body>
</html>
"@

# 保存HTML报告
$html | Out-File -FilePath $OutputPath -Encoding UTF8

Write-Host "HTML报告已生成: $OutputPath" -ForegroundColor Green
Write-Host "在浏览器中打开查看完整报告" -ForegroundColor Cyan
