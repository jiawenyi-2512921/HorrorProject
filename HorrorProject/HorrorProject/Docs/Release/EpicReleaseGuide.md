# Epic Games Store Release Guide
# Epic Games Store发布指南

## 概述

Epic Games Store (EGS) 是PC游戏的主要分销平台之一，以其对开发者友好的收入分成（88/12）和独占交易而闻名。

---

## 1. 前期准备

### 1.1 Epic开发者账户

#### 注册流程
- [ ] 访问 https://dev.epicgames.com/
- [ ] 创建Epic Games账户（如果没有）
- [ ] 申请发行商账户
- [ ] 完成公司验证
- [ ] 等待审核（通常1-2周）

#### 所需信息
- 公司法律名称
- 公司地址
- 税务信息（EIN或等效）
- 银行账户信息
- 联系人信息
- 游戏概述和截图

### 1.2 Epic开发者门户设置

- [ ] 登录Epic开发者门户
- [ ] 创建组织（Organization）
- [ ] 创建产品（Product）
- [ ] 获取产品ID
- [ ] 设置团队成员和权限
- [ ] 配置支付和税务信息

---

## 2. Epic Online Services (EOS) 集成

### 2.1 EOS SDK下载和安装

```cpp
// 下载EOS SDK
// https://dev.epicgames.com/docs/epic-online-services/eos-get-started/services-quick-start

// 在虚幻引擎中启用EOS插件
// Edit -> Plugins -> 搜索 "Epic Online Services"
// 启用插件并重启编辑器
```

### 2.2 配置EOS

**DefaultEngine.ini:**
```ini
[OnlineSubsystem]
DefaultPlatformService=EOS

[OnlineSubsystemEOS]
bEnabled=true
ProductId=YOUR_PRODUCT_ID
SandboxId=YOUR_SANDBOX_ID
DeploymentId=YOUR_DEPLOYMENT_ID
ClientId=YOUR_CLIENT_ID
ClientSecret=YOUR_CLIENT_SECRET
```

### 2.3 实现EOS功能

#### 必需功能
- [ ] 用户认证
- [ ] 成就系统
- [ ] 云存档
- [ ] 统计追踪
- [ ] 好友系统（可选）
- [ ] 语音聊天（如适用）

#### 成就系统
```cpp
// 定义成就
// 在Epic开发者门户配置成就

// 解锁成就代码示例
void UnlockAchievement(FString AchievementId)
{
    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
    if (OnlineSubsystem)
    {
        IOnlineAchievementsPtr Achievements = OnlineSubsystem->GetAchievementsInterface();
        if (Achievements.IsValid())
        {
            Achievements->UnlockAchievement(*GetLocalPlayerController()->GetLocalPlayer()->GetPreferredUniqueNetId(), AchievementId);
        }
    }
}
```

#### 云存档
```cpp
// 保存到云端
void SaveToCloud(FString FileName, TArray<uint8> Data)
{
    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
    if (OnlineSubsystem)
    {
        IOnlineUserCloudPtr UserCloud = OnlineSubsystem->GetUserCloudInterface();
        if (UserCloud.IsValid())
        {
            UserCloud->WriteUserFile(*GetLocalPlayerController()->GetLocalPlayer()->GetPreferredUniqueNetId(), FileName, Data);
        }
    }
}
```

---

## 3. 商店页面设置

### 3.1 基本信息

#### 游戏信息
- [ ] **游戏标题** - 清晰、独特、可搜索
- [ ] **开发商** - 公司或团队名称
- [ ] **发行商** - 如果不同
- [ ] **发布日期** - 精确日期
- [ ] **价格** - 基础价格（USD）
- [ ] **年龄分级** - ESRB/PEGI等

#### 描述
- [ ] **简短描述** - 1-2句话钩子
- [ ] **详细描述** - 完整游戏介绍
- [ ] **关键特性** - 列表形式，3-5个要点
- [ ] **故事概述** - 不剧透的背景
- [ ] **开发者说明** - 可选，个人化内容

### 3.2 分类和标签

#### 类型
- [ ] 主要类型（如：冒险）
- [ ] 子类型（如：恐怖）
- [ ] 游戏模式（单人/多人）
- [ ] 视角（第一人称/第三人称）

#### 特性标签
- 单人
- 故事丰富
- 大气
- 恐怖
- 探索
- 解谜
- 虚幻引擎5
- 手柄支持
- 云存档
- 成就

### 3.3 系统需求

**最低配置：**
```
操作系统: Windows 10 64-bit
处理器: Intel Core i5-8400 / AMD Ryzen 5 2600
内存: 16 GB RAM
显卡: NVIDIA GTX 1060 6GB / AMD RX 580 8GB
DirectX: Version 12
存储空间: 50 GB 可用空间
附加说明: SSD推荐
```

**推荐配置：**
```
操作系统: Windows 11 64-bit
处理器: Intel Core i7-10700K / AMD Ryzen 7 3700X
内存: 32 GB RAM
显卡: NVIDIA RTX 3070 / AMD RX 6800
DirectX: Version 12
存储空间: 50 GB 可用空间
附加说明: SSD必需，支持光线追踪
```

### 3.4 视觉资产

#### 必需图像资产

**商店图像：**
- [ ] **主要横幅** - 2560x1440 px (16:9)
- [ ] **缩略图** - 1200x1600 px (3:4)
- [ ] **Logo** - 400x400 px (透明PNG)
- [ ] **背景图** - 1920x1080 px
- [ ] **特色图** - 2560x1440 px

**库资产：**
- [ ] **库横幅** - 1920x1080 px
- [ ] **库Logo** - 透明PNG，推荐尺寸

**社交分享：**
- [ ] **社交图** - 1200x630 px

#### 截图
- [ ] 最少5张高质量截图
- [ ] 推荐8-12张
- [ ] 分辨率：1920x1080或更高（推荐4K）
- [ ] 格式：PNG或JPG
- [ ] 展示游戏多样性和特色

#### 视频
- [ ] **发布预告片** - 必需
- [ ] **游戏玩法视频** - 强烈推荐
- [ ] **开发者访谈** - 可选
- [ ] 格式：MP4，H.264编码
- [ ] 分辨率：1080p或4K
- [ ] 时长：预告片60-90秒，玩法2-3分钟

---

## 4. 构建上传

### 4.1 使用Epic Games Launcher上传

#### 准备构建
- [ ] 创建发布版本构建
- [ ] 配置正确的Product ID
- [ ] 测试EOS集成
- [ ] 验证所有功能
- [ ] 准备发布说明

#### 上传流程
1. 打开Epic Games Launcher
2. 进入"库"标签
3. 找到你的游戏项目
4. 点击"上传构建"
5. 选择构建文件夹
6. 填写构建信息
7. 开始上传
8. 等待处理完成

### 4.2 使用BuildPatch Tool上传

**下载工具：**
```bash
# 从Epic开发者门户下载BuildPatch Tool
# 解压到本地目录
```

**配置上传：**
```bash
# 创建上传配置文件
BuildPatchTool.exe -mode=UploadBinary ^
-OrganizationId=YOUR_ORG_ID ^
-ProductId=YOUR_PRODUCT_ID ^
-ArtifactId=YOUR_ARTIFACT_ID ^
-ClientId=YOUR_CLIENT_ID ^
-ClientSecret=YOUR_CLIENT_SECRET ^
-CloudDir=YOUR_CLOUD_DIR ^
-BuildRoot=D:\GameBuild ^
-BuildVersion=1.0.0 ^
-AppLaunch=YourGame.exe ^
-AppArgs=""
```

### 4.3 构建标签和分支

**发布分支：**
- **Live** - 公开发布版本
- **Staging** - 预发布测试
- **Development** - 内部开发

**配置分支：**
- [ ] 在开发者门户创建分支
- [ ] 上传到对应分支
- [ ] 设置分支可见性
- [ ] 测试分支切换

---

## 5. 定价策略

### 5.1 基础定价

**定价建议：**
- 研究竞品定价
- 考虑游戏内容和时长
- Epic的88/12分成更有利
- 可以略低于Steam价格

**区域定价：**
- [ ] 使用Epic推荐定价
- [ ] 调整主要市场价格
- [ ] 考虑购买力平价
- [ ] 测试不同价格点

### 5.2 折扣和促销

**Epic促销活动：**
- Epic Mega Sale（5月/11月）
- 季节性促销
- 发行商促销
- 独占游戏推广

**折扣策略：**
- 首次折扣：发布后2-3个月
- 折扣幅度：10-50%
- 参与Epic促销活动
- 配合内容更新

---

## 6. 独占交易考虑

### 6.1 Epic独占优势

**好处：**
- 前期资金支持
- 保证最低销售额
- Epic营销支持
- 首页推荐位置
- 开发资金

**考虑因素：**
- 独占期限（通常6-12个月）
- 社区反应
- 长期销售影响
- 品牌形象

### 6.2 谈判要点

- 独占期限
- 保证金额
- 营销支持
- 技术支持
- 灵活条款

---

## 7. 发布前测试

### 7.1 技术测试清单

- [ ] EOS功能测试
  - [ ] 用户登录
  - [ ] 成就解锁
  - [ ] 云存档同步
  - [ ] 统计追踪
- [ ] Epic Launcher集成
  - [ ] 游戏启动
  - [ ] 更新检测
  - [ ] DLC检测
- [ ] 性能测试
  - [ ] 启动时间
  - [ ] 加载时间
  - [ ] 内存使用
  - [ ] 崩溃测试

### 7.2 商店页面测试

- [ ] 所有图像正确显示
- [ ] 视频正常播放
- [ ] 文本无错误
- [ ] 链接有效
- [ ] 购买流程测试
- [ ] 移动端显示

---

## 8. 营销和推广

### 8.1 Epic营销支持

**可用资源：**
- 首页特色位置
- 社交媒体推广
- Epic Games博客报道
- 新闻稿支持
- 社区活动

**申请流程：**
- [ ] 联系Epic营销团队
- [ ] 提交营销材料
- [ ] 协调发布时间
- [ ] 参与Epic活动

### 8.2 社区建设

- [ ] 创建Discord服务器
- [ ] 建立社交媒体存在
- [ ] 准备FAQ
- [ ] 设置支持渠道
- [ ] 培养核心社区

---

## 9. 发布流程

### 9.1 发布前检查（T-1周）

- [ ] 最终构建上传并测试
- [ ] 商店页面完整
- [ ] 所有资产就位
- [ ] 定价确认
- [ ] 发布日期锁定
- [ ] 营销材料准备
- [ ] 团队就位

### 9.2 发布日执行

**发布时间建议：**
- 太平洋时间上午10点
- 避开大作发布日
- 考虑全球时区

**发布步骤：**
1. [ ] 将构建设置为Live
2. [ ] 确认游戏可见
3. [ ] 测试购买流程
4. [ ] 发布公告
5. [ ] 监控反馈
6. [ ] 准备快速响应

---

## 10. 发布后管理

### 10.1 更新和补丁

**更新流程：**
1. 开发和测试补丁
2. 上传到Staging分支
3. 内部测试
4. 推送到Live分支
5. 发布更新说明
6. 监控玩家反馈

### 10.2 社区管理

- [ ] 监控Epic评论
- [ ] 回复玩家问题
- [ ] 收集反馈
- [ ] 定期更新
- [ ] 举办社区活动

### 10.3 数据分析

**关键指标：**
- 销售数量和收入
- 玩家留存率
- 平均游戏时长
- 成就完成率
- 退款率

---

## 11. DLC和扩展内容

### 11.1 DLC规划

- [ ] 规划DLC内容
- [ ] 创建DLC产品
- [ ] 设置DLC页面
- [ ] 配置定价
- [ ] 实现DLC检测
- [ ] 测试DLC

### 11.2 季票

- [ ] 规划季票内容
- [ ] 设置季票产品
- [ ] 配置自动授予
- [ ] 推广季票

---

## 12. 最佳实践

### 做 ✓
- 充分利用EOS功能
- 优化商店页面
- 积极与Epic合作
- 快速响应反馈
- 定期更新内容
- 建立社区
- 监控数据
- 参与Epic活动

### 不要 ✗
- 忽视EOS集成
- 低质量商店页面
- 忽视社区反馈
- 延迟更新
- 误导性营销
- 忽视技术问题
- 放弃发布后支持

---

## 13. 常见问题

### Q: Epic和Steam可以同时发布吗？
A: 可以，除非签署了独占协议。

### Q: Epic的收入分成是多少？
A: 88%给开发者，12%给Epic（比Steam的70/30更优惠）。

### Q: 如何获得Epic的营销支持？
A: 联系Epic营销团队，提交游戏信息和营销材料。

### Q: Epic支持哪些支付方式？
A: 信用卡、PayPal、Epic钱包等多种方式。

### Q: 如何处理退款？
A: Epic处理退款，遵循其退款政策（14天内且游戏时间<2小时）。

---

## 14. 资源链接

**官方文档：**
- Epic开发者门户：https://dev.epicgames.com/
- EOS文档：https://dev.epicgames.com/docs/epic-online-services
- 发布指南：https://dev.epicgames.com/docs/epic-games-store

**工具：**
- Epic Games Launcher
- BuildPatch Tool
- EOS SDK

**支持：**
- 开发者支持：通过开发者门户
- 社区论坛：Epic开发者社区

---

## 15. 检查清单总结

### 准备阶段
- [ ] Epic开发者账户创建
- [ ] EOS集成完成
- [ ] 商店页面设置
- [ ] 构建准备

### 发布阶段
- [ ] 构建上传
- [ ] 商店页面完成
- [ ] 测试通过
- [ ] 营销准备

### 发布后
- [ ] 监控和响应
- [ ] 定期更新
- [ ] 社区管理
- [ ] 数据分析

---

**祝Epic Games Store发布成功！**

**最后更新：** 2026-04-26
