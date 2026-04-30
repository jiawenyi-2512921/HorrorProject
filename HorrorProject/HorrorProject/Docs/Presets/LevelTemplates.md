# 关卡模板文档

本文档定义了HorrorProject中可复用的关卡模板，每个模板包含推荐的资产清单和设计指南。

---

## 1. 走廊模板（工业风格）

### 设计特点
- 狭窄压迫的空间设计
- 工业金属材质为主
- 管道和电缆沿墙布置
- 昏暗照明，营造紧张感

### 尺寸规格
- **标准走廊**: 300cm宽 × 400cm高 × 1000-2000cm长
- **狭窄走廊**: 200cm宽 × 350cm高 × 800-1500cm长
- **宽阔走廊**: 400-500cm宽 × 450cm高 × 1500-3000cm长

### 资产清单

#### 结构资产
- `SM_Industrial_Wall_Straight` - 直墙段
- `SM_Industrial_Wall_Corner` - 转角墙
- `SM_Industrial_Floor_Grate` - 金属格栅地板
- `SM_Industrial_Ceiling_Panel` - 天花板面板
- `SM_Industrial_Door_Frame` - 门框
- `SM_Industrial_Door_Sliding` - 滑动门

#### 装饰资产
- `SM_Pipe_Horizontal_Large` - 大型水平管道
- `SM_Pipe_Vertical_Small` - 小型垂直管道
- `SM_Cable_Tray` - 电缆桥架
- `SM_Vent_Wall` - 墙面通风口
- `SM_Warning_Sign` - 警告标识
- `SM_Fire_Extinguisher` - 灭火器
- `SM_Emergency_Light` - 应急灯

#### 灯光资产
- `BP_Flickering_TubeLight` - 闪烁日光灯
- `BP_Emergency_RedLight` - 红色应急灯
- `BP_Spotlight_Dim` - 昏暗聚光灯

#### 音效
- `AMB_Industrial_Hum` - 工业嗡嗡声
- `SFX_Pipe_Drip` - 管道滴水声
- `SFX_Metal_Creak` - 金属吱嘎声
- `SFX_Distant_Machinery` - 远处机械声

### 布局建议
1. 每10-15米放置一个转角或岔路口
2. 在视线尽头放置视觉焦点（门、光源、装饰物）
3. 使用不对称布局增加不安感
4. 管道和电缆应有逻辑走向

---

## 2. 房间模板

### 2.1 小型房间（储藏室/办公室）

#### 尺寸规格
- 400cm × 400cm × 300cm高

#### 资产清单
- `SM_Room_Wall_Small` - 小房间墙体
- `SM_Door_Single` - 单扇门
- `SM_Shelf_Metal` - 金属货架
- `SM_Desk_Office` - 办公桌
- `SM_Chair_Office` - 办公椅
- `SM_Filing_Cabinet` - 文件柜
- `SM_Cardboard_Box` - 纸箱
- `SM_Ceiling_Light_Single` - 单盏吊灯

#### 用途
- 物资存放点
- 文档/日志发现点
- 小型遭遇战场景
- 藏身点

### 2.2 中型房间（实验室/控制室）

#### 尺寸规格
- 600-800cm × 600-800cm × 350cm高

#### 资产清单
- `SM_Room_Wall_Medium` - 中型房间墙体
- `SM_Lab_Table` - 实验台
- `SM_Computer_Terminal` - 计算机终端
- `SM_Monitor_Array` - 监视器阵列
- `SM_Equipment_Rack` - 设备机架
- `SM_Glass_Partition` - 玻璃隔断
- `SM_Hazard_Container` - 危险品容器
- `BP_Interactive_Terminal` - 可交互终端

#### 用途
- 主要剧情点
- 解谜场景
- Boss战前置区域
- 关键物品获取点

### 2.3 大型房间（大厅/仓库）

#### 尺寸规格
- 1000-1500cm × 1000-1500cm × 500-600cm高

#### 资产清单
- `SM_Warehouse_Wall` - 仓库墙体
- `SM_Support_Column` - 支撑柱
- `SM_Catwalk` - 高架走道
- `SM_Stairs_Industrial` - 工业楼梯
- `SM_Cargo_Container` - 货物集装箱
- `SM_Forklift` - 叉车
- `SM_Pallet_Stack` - 托盘堆
- `SM_Overhead_Crane` - 天车
- `BP_Hanging_Chain` - 悬挂链条

#### 用途
- 多敌人遭遇战
- 垂直空间探索
- 追逐场景
- 关键Boss战

---

## 3. 开放区域模板

### 3.1 室外庭院

#### 尺寸规格
- 2000cm × 2000cm（可扩展）

#### 资产清单
- `SM_Concrete_Ground` - 混凝土地面
- `SM_Chain_Fence` - 铁丝网围栏
- `SM_Shipping_Container` - 集装箱
- `SM_Industrial_Tank` - 工业储罐
- `SM_Rusted_Vehicle` - 生锈车辆
- `SM_Debris_Pile` - 碎片堆
- `BP_Fog_Volume` - 雾气体积
- `BP_Rain_Emitter` - 雨效发射器

#### 灯光
- `BP_Streetlight_Broken` - 损坏路灯
- `BP_Searchlight` - 探照灯
- `BP_Lightning_Flash` - 闪电效果

### 3.2 地下停车场

#### 尺寸规格
- 1500cm × 2000cm × 300cm高

#### 资产清单
- `SM_Parking_Floor` - 停车场地面
- `SM_Parking_Pillar` - 停车场柱子
- `SM_Car_Abandoned` - 废弃汽车
- `SM_Parking_Sign` - 停车标识
- `SM_Oil_Stain_Decal` - 油渍贴花
- `SM_Tire_Marks_Decal` - 轮胎痕迹
- `BP_Fluorescent_Light_Row` - 荧光灯排

#### 用途
- 追逐场景
- 掩体战斗
- 迷宫式探索

---

## 4. 特殊区域模板

### 4.1 通风管道系统

#### 尺寸规格
- 150cm × 150cm（爬行空间）
- 200cm × 200cm（蹲伏空间）

#### 资产清单
- `SM_Duct_Straight` - 直管道段
- `SM_Duct_Corner` - 转角管道
- `SM_Duct_Junction` - 管道交叉口
- `SM_Vent_Grate` - 通风格栅
- `SM_Fan_Industrial` - 工业风扇
- `BP_Duct_Ambience` - 管道环境音

### 4.2 电梯井

#### 尺寸规格
- 300cm × 300cm × 多层高度

#### 资产清单
- `SM_Elevator_Shaft_Wall` - 电梯井墙体
- `SM_Elevator_Cable` - 电梯缆绳
- `SM_Elevator_Car` - 电梯轿厢
- `SM_Ladder_Vertical` - 垂直梯子
- `BP_Elevator_Mechanism` - 电梯机制

---

## 使用指南

### 模板组合原则
1. **节奏控制**: 狭窄走廊 → 小房间 → 走廊 → 大房间（战斗）
2. **视觉变化**: 每3-4个区域改变材质/颜色主题
3. **高度变化**: 适当加入楼梯、电梯、坑洞
4. **循环设计**: 提供快捷通道连接已探索区域

### 性能优化
- 使用LOD系统
- 合理设置遮挡剔除体积
- 限制单屏幕动态光源数量（≤3）
- 复用材质实例

### 恐怖氛围营造
- **声音分层**: 环境音 + 随机事件音 + 敌人音
- **光影对比**: 明暗交替，避免全黑或全亮
- **视觉引导**: 用光线和装饰引导玩家视线
- **假象惊吓**: 放置无害但突然的视觉/听觉元素

---

## 版本历史
- v1.0 (2026-04-28) - 初始版本
