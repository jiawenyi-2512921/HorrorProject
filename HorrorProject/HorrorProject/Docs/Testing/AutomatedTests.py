#!/usr/bin/env python3
"""
HorrorProject 自动化测试脚本
用于验证关卡完整性、资产引用和性能基准

使用方法:
    python AutomatedTests.py --all
    python AutomatedTests.py --level-integrity
    python AutomatedTests.py --asset-validation
    python AutomatedTests.py --performance-benchmark

要求:
    - Python 3.8+
    - Unreal Engine 5.6 安装
    - 项目路径配置正确
"""

import os
import sys
import json
import subprocess
import argparse
import re
from pathlib import Path
from datetime import datetime
from typing import List, Dict, Tuple, Optional

# ============================================================================
# 配置
# ============================================================================

# 项目路径配置
PROJECT_ROOT = Path(__file__).parent.parent.parent.absolute()
PROJECT_NAME = "HorrorProject"
UPROJECT_FILE = PROJECT_ROOT / f"{PROJECT_NAME}.uproject"
CONTENT_DIR = PROJECT_ROOT / "Content"
CONFIG_DIR = PROJECT_ROOT / "Config"
SAVED_DIR = PROJECT_ROOT / "Saved"
LOGS_DIR = SAVED_DIR / "Logs"

# Unreal Engine 路径。优先使用环境变量，默认与项目构建脚本保持一致。
UE_ROOT = Path(os.environ.get("UE_5_6_ROOT", os.environ.get("UNREAL_ENGINE_ROOT", r"D:\UnrealEngine\UE_5.6")))
UE_EDITOR_CMD = Path(os.environ.get(
    "UE_EDITOR_CMD",
    str(UE_ROOT / "Engine" / "Binaries" / "Win64" / "UnrealEditor-Cmd.exe")
))

# 测试结果输出
TEST_RESULTS_DIR = PROJECT_ROOT / "Docs" / "Testing" / "Results"
TEST_RESULTS_DIR.mkdir(parents=True, exist_ok=True)

# 性能基准
PERFORMANCE_BENCHMARKS = {
    "target_fps": 60,
    "min_fps": 30,
    "max_memory_mb": 8192,
    "max_load_time_sec": 30,
}

# ============================================================================
# 工具函数
# ============================================================================

class TestLogger:
    """测试日志记录器"""

    def __init__(self, log_file: Path):
        self.log_file = log_file
        self.results = []

    def log(self, message: str, level: str = "INFO"):
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        log_entry = f"[{timestamp}] [{level}] {message}"
        print(log_entry)

        with open(self.log_file, "a", encoding="utf-8") as f:
            f.write(log_entry + "\n")

    def success(self, message: str):
        self.log(f"✓ {message}", "SUCCESS")
        self.results.append({"status": "PASS", "message": message})

    def error(self, message: str):
        self.log(f"✗ {message}", "ERROR")
        self.results.append({"status": "FAIL", "message": message})

    def warning(self, message: str):
        self.log(f"⚠ {message}", "WARNING")
        self.results.append({"status": "WARN", "message": message})

    def get_summary(self) -> Dict:
        total = len(self.results)
        passed = sum(1 for r in self.results if r["status"] == "PASS")
        failed = sum(1 for r in self.results if r["status"] == "FAIL")
        warnings = sum(1 for r in self.results if r["status"] == "WARN")

        return {
            "total": total,
            "passed": passed,
            "failed": failed,
            "warnings": warnings,
            "pass_rate": (passed / total * 100) if total > 0 else 0
        }


def check_prerequisites() -> bool:
    """检查测试前提条件"""
    print("检查测试前提条件...")

    if not UPROJECT_FILE.exists():
        print(f"错误: 找不到项目文件 {UPROJECT_FILE}")
        return False

    if not UE_EDITOR_CMD.exists():
        print(f"错误: 找不到UE编辑器 {UE_EDITOR_CMD}")
        print("请设置 UE_EDITOR_CMD、UE_5_6_ROOT 或 UNREAL_ENGINE_ROOT 环境变量")
        return False

    if not CONTENT_DIR.exists():
        print(f"错误: 找不到Content目录 {CONTENT_DIR}")
        return False

    print("✓ 前提条件检查通过")
    return True


def find_asset_files(extensions: List[str]) -> List[Path]:
    """查找指定扩展名的资产文件"""
    assets = []
    for ext in extensions:
        assets.extend(CONTENT_DIR.rglob(f"*{ext}"))
    return assets


def parse_uasset_references(uasset_path: Path) -> List[str]:
    """
    解析.uasset文件中的引用（简化版）
    注意: 这是一个简化实现，实际解析需要UE的资产注册表
    """
    references = []

    try:
        with open(uasset_path, "rb") as f:
            content = f.read()

        # 查找常见的资产路径模式
        # 格式: /Game/Path/To/Asset
        pattern = rb'/Game/[A-Za-z0-9_/]+'
        matches = re.findall(pattern, content)

        for match in matches:
            try:
                ref = match.decode('utf-8')
                if ref not in references:
                    references.append(ref)
            except:
                pass

    except Exception as e:
        pass

    return references


# ============================================================================
# 测试套件
# ============================================================================

class LevelIntegrityTests:
    """关卡完整性测试"""

    def __init__(self, logger: TestLogger):
        self.logger = logger

    def run_all(self):
        """运行所有关卡完整性测试"""
        self.logger.log("=" * 60)
        self.logger.log("开始关卡完整性测试")
        self.logger.log("=" * 60)

        self.test_level_files_exist()
        self.test_level_metadata()
        self.test_persistent_levels()

        self.logger.log("关卡完整性测试完成")

    def test_level_files_exist(self):
        """测试关卡文件是否存在"""
        self.logger.log("检查关卡文件...")

        level_files = list(CONTENT_DIR.rglob("*.umap"))

        if not level_files:
            self.logger.error("未找到任何关卡文件 (.umap)")
            return

        self.logger.success(f"找到 {len(level_files)} 个关卡文件")

        for level in level_files:
            level_name = level.stem
            uexp_file = level.with_suffix(".uexp")

            if uexp_file.exists():
                self.logger.success(f"关卡 {level_name}: 文件完整")
            else:
                self.logger.error(f"关卡 {level_name}: 缺少 .uexp 文件")

    def test_level_metadata(self):
        """测试关卡元数据"""
        self.logger.log("检查关卡元数据...")

        level_files = list(CONTENT_DIR.rglob("*.umap"))

        for level in level_files:
            level_name = level.stem
            file_size = level.stat().st_size

            if file_size < 1024:  # 小于1KB可能是空关卡
                self.logger.warning(f"关卡 {level_name}: 文件过小 ({file_size} bytes)")
            else:
                self.logger.success(f"关卡 {level_name}: 大小正常 ({file_size / 1024:.2f} KB)")

    def test_persistent_levels(self):
        """测试持久关卡配置"""
        self.logger.log("检查持久关卡配置...")

        # 检查DefaultEngine.ini中的关卡配置
        engine_ini = CONFIG_DIR / "DefaultEngine.ini"

        if not engine_ini.exists():
            self.logger.warning("未找到 DefaultEngine.ini")
            return

        with open(engine_ini, "r", encoding="utf-8") as f:
            content = f.read()

        if "GameDefaultMap" in content:
            self.logger.success("找到默认游戏地图配置")
        else:
            self.logger.warning("未配置默认游戏地图")

        if "EditorStartupMap" in content:
            self.logger.success("找到编辑器启动地图配置")
        else:
            self.logger.warning("未配置编辑器启动地图")


class AssetValidationTests:
    """资产引用验证测试"""

    def __init__(self, logger: TestLogger):
        self.logger = logger

    def run_all(self):
        """运行所有资产验证测试"""
        self.logger.log("=" * 60)
        self.logger.log("开始资产引用验证测试")
        self.logger.log("=" * 60)

        self.test_asset_count()
        self.test_texture_formats()
        self.test_audio_formats()
        self.test_blueprint_files()

        selog("资产引用验证测试完成")

    def test_asset_count(self):
        """统计资产数量"""
        self.logger.log("统计资产数量...")

        asset_types = {
            "Blueprints": [".uasset"],
            "Maps": [".umap"],
            "Textures": [".uasset"],  # 纹理也是.uasset
            "Materials": [".uasset"],
            "Sounds": [".uasset"],
        }

        for asset_type, extensions in asset_types.items():
            files = find_asset_files(extensions)

            # 根据路径过滤
            if asset_type == "Blueprints":
                files = [f for f in files if "Blueprints" in str(f)]
            elif asset_type == "Maps":
                pass  # .umap已经是地图
            elif asset_type == "Textures":
                files = [f for f in files if "Textures" in str(f)]
            elif asset_type == "Materials":
                files = [f for f in files if "Materials" in str(f)]
            elif asset_type == "Sounds":
                files = [f for f in files if "Sounds" in str(f) or "Audio" in str(f)]

            count = len(files)
            self.logger.success(f"{asset_type}: {count} 个文件")

    def test_texture_formats(self):
        """检查纹理格式"""
        self.logger.log("检查纹理格式...")

        texture_files = [f for f in find_asset_files([".uasset"]) if "Textures" in str(f)]

        if not texture_files:
            self.logger.warning("未找到纹理文件")
            return

        self.logger.success(f"找到 {len(texture_files)} 个纹理资产")

        # 检查是否有对应的源文件（可选）
        source_extensions = [".png", ".jpg", ".tga", ".exr"]
        source_files = []
        for ext in source_extensions:
            source_files.extend(CONTENT_DIR.rglob(f"*{ext}"))

        if source_files:
            self.logger.warning(f"发现 {len(source_files)} 个未导入的源纹理文件")

    def test_audio_formats(self):
        """检查音频格式"""
        self.logger.log("检查音频格式...")

        audio_files = [f for f in find_asset_files([".uasset"]) if "Sounds" in str(f) or "Audio" in str(f)]

        if not audio_files:
            self.logger.warning("未找到音频文件")
            return

        self.logger.success(f"找到 {len(audio_files)} 个音频资产")

        # 检查源音频文件
        source_audio = list(CONTENT_DIR.rglob("*.wav")) + list(CONTENT_DIR.rglob("*.mp3"))

        if source_audio:
            self.logger.warning(f"发现 {len(source_audio)} 个未导入的源音频文件")

    def test_blueprint_files(self):
        """检查蓝图文件"""
        self.logger.log("检查蓝图文件...")

        blueprint_files = [f for f in find_asset_files([".uasset"]) if "Blueprints" in str(f)]

        if not blueprint_files:
            self.logger.warning("未找到蓝图文件")
            return

        self.logger.success(f"找到 {len(blueprint_files)} 个蓝图资产")

        # 检查蓝图完整性（需要.uexp文件）
        incomplete = []
        for bp in blueprint_files:
            uexp_file = bp.with_suffix(".uexp")
            if not uexp_file.exists():
                incomplete.append(bp.stem)

        if incomplete:
            self.logger.error(f"{len(incomplete)} 个蓝图缺少 .uexp 文件")
        else:
            self.logger.success("所有蓝图文件完整")


class PerformanceBenchmarkTests:
    """性能基准测试"""

    def __init__(self, logger: TestLogger):
        self.logger = logger

    def run_all(self):
        """运行所有性能基准测试"""
        self.logger.log("=" * 60)
        self.logger.log("开始性能基准测试")
        self.logger.log("=" * 60)

        self.test_project_size()
        self.test_asset_count_performance()
        self.test_log_analysis()

        self.logger.log("性能基准测试完成")

    def test_project_size(self):
        """测试项目大小"""
        self.logger.log("检查项目大小...")

        def get_dir_size(path: Path) -> int:
            total = 0
            try:
                for entry in path.rglob("*"):
                    if entry.is_file():
                        total += entry.stat().st_size
            except:
                pass
            return total

        content_size = get_dir_size(CONTENT_DIR)
        saved_size = get_dir_size(SAVED_DIR) if SAVED_DIR.exists() else 0

        self.logger.success(f"Content 目录大小: {content_size / (1024**3):.2f} GB")
        self.logger.success(f"Saved 目录大小: {saved_size / (1024**3):.2f} GB")

        total_size = content_size + saved_size
        if total_size > 50 * (1024**3):  # 50GB
            self.logger.warning("项目总大小超过 50GB，建议清理")

    def test_asset_count_performance(self):
        """测试资产数量对性能的影响"""
        self.logger.log("分析资产数量...")

        all_assets = find_asset_files([".uasset", ".umap"])
        total_count = len(all_assets)

        self.logger.success(f"总资产数量: {total_count}")

        if total_count > 10000:
            self.logger.warning("资产数量较多，可能影响编辑器性能")
        elif total_count > 5000:
            self.logger.log("资产数量适中")
        else:
            self.logger.success("资产数量合理")

    def test_log_analysis(self):
        """分析日志文件"""
        self.logger.log("分析最近的日志文件...")

        if not LOGS_DIR.exists():
            self.logger.warning("未找到日志目录")
            return

        log_files = list(LOGS_DIR.glob("*.log"))

        if not log_files:
            self.logger.warning("未找到日志文件")
            return

        # 获取最新的日志文件
        latest_log = max(log_files, key=lambda f: f.stat().st_mtime)

        self.logger.log(f"分析日志: {latest_log.name}")

        try:
            with open(latest_log, "r", encoding="utf-8", errors="ignore") as f:
                content = f.read()

            # 统计错误和警告
            errors = content.count("Error:")
            warnings = content.count("Warning:")

            if errors > 0:
                self.logger.error(f"发现 {errors} 个错误")
            else:
                self.logger.success("未发现错误")

            if warnings > 10:
                self.logger.warning(f"发现 {warnings} 个警告")
            elif warnings > 0:
                self.logger.log(f"发现 {warnings} 个警告（正常范围）")
            else:
                self.logger.success("未发现警告")

        except Exception as e:
            self.logger.error(f"无法读取日志文件: {e}")


# ============================================================================
# 主程序
# ============================================================================

def generate_report(logger: TestLogger, output_file: Path):
    """生成测试报告"""
    summary = logger.get_summary()
    generated_at = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    total = summary["total"]
    passed = summary["passed"]
    failed = summary["failed"]
    warnings = summary["warnings"]
    pass_rate = summary["pass_rate"]

    report = f"""
# HorrorProject 自动化测试报告

**生成时间**: {generated_at}

## 测试摘要

- **总测试数**: {total}
- **通过**: {passed}
- **失败**: {failed}
- **警告**: {warnings}
- **通过率**: {pass_rate:.2f}%

## 详细结果

"""

    for result in logger.results:
        status_icon = {
            "PASS": "✓",
            "FAIL": "✗",
            "WARN": "⚠"
        }.get(result["status"], "?")

        report += f"{status_icon} [{result['status']}] {result['message']}\n"

    report += f"""
---

## 性能基准

- **目标FPS**: {PERFORMANCE_BENCHMARKS['target_fps']}
- **最低FPS**: {PERFORMANCE_BENCHMARKS['min_fps']}
- **最大内存**: {PERFORMANCE_BENCHMARKS['max_memory_mb']} MB
- **最大加载时间**: {PERFORMANCE_BENCHMARKS['max_load_time_sec']} 秒

## 建议

"""

    if summary['failed'] > 0:
        report += "- 请修复失败的测试项\n"

    if summary['warnings'] > 5:
        report += "- 建议处理警告项以提升项目质量\n"

    if summary['pass_rate'] == 100:
        report += "- 所有测试通过，项目状态良好！\n"

    with open(output_file, "w", encoding="utf-8") as f:
        f.write(report)

    print(f"\n测试报告已生成: {output_file}")


def main():
    parser = argparse.ArgumentParser(description="HorrorProject 自动化测试")
    parser.add_argument("--all", action="store_true", help="运行所有测试")
    parser.add_argument("--level-integrity", action="store_true", help="运行关卡完整性测试")
    parser.add_argument("--asset-validation", action="store_true", help="运行资产验证测试")
    parser.add_argument("--performance-benchmark", action="store_true", help="运行性能基准测试")

    args = parser.parse_args()

    # 如果没有指定任何测试，默认运行所有测试
    if not any([args.all, args.level_integrity, args.asset_validation, args.performance_benchmark]):
        args.all = True

    # 检查前提条件
    if not check_prerequisites():
        sys.exit(1)

    # 创建日志记录器
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    log_file = TEST_RESULTS_DIR / f"test_log_{timestamp}.txt"
    logger = TestLogger(log_file)

    logger.log("HorrorProject 自动化测试开始")
    logger.log(f"项目路径: {PROJECT_ROOT}")
    logger.log(f"测试时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")

    # 运行测试
    try:
        if args.all or args.level_integrity:
            LevelIntegrityTests(logger).run_all()

        if args.all or args.asset_validation:
            AssetValidationTests(logger).run_all()

        if args.all or args.performance_benchmark:
            PerformanceBenchmarkTests(logger).run_all()

    except Exception as e:
        logger.error(f"测试过程中发生错误: {e}")
        import traceback
        traceback.print_exc()

    # 生成报告
    report_file = TEST_RESULTS_DIR / f"test_report_{timestamp}.md"
    generate_report(logger, report_file)

    # 打印摘要
    summary = logger.get_summary()
    print("\n" + "=" * 60)
    print("测试完成")
    print("=" * 60)
    print(f"总测试数: {summary['total']}")
    print(f"通过: {summary['passed']}")
    print(f"失败: {summary['failed']}")
    print(f"警告: {summary['warnings']}")
    print(f"通过率: {summary['pass_rate']:.2f}%")
    print("=" * 60)

    # 返回退出码
    sys.exit(0 if summary['failed'] == 0 else 1)


if __name__ == "__main__":
    main()
