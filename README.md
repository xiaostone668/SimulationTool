# Simulation Tool - Qt & OpenCASCADE 仿真工具

基于Qt Widgets和OpenCASCADE的3D仿真工具框架。

## 功能特性

### 主要功能
- **STEP文件加载**: 支持读取和显示STEP格式的3D模型
- **3D可视化**: 基于OpenCASCADE的AIS_Viewer实现3D渲染
- **仿真引擎**: 独立线程运行的物理仿真计算核心
- **参数控制**: 可调节时间步长、总时长、阻尼系数、刚度系数等参数
- **进度监控**: 实时显示仿真进度和状态
- **共享内存**: 支持通过共享内存与其他进程通信

### 用户界面
- **菜单栏**: 文件操作（打开STEP、保存结果）
- **工具栏**: 仿真控制按钮（开始、暂停、停止）
- **中心区域**: 3D视图显示区域
- **右侧面板**: 仿真参数设置面板
- **底部状态栏**: 显示仿真进度和状态信息

## 项目结构

```
SimulationTool/
├── CMakeLists.txt              # CMake构建配置
├── README.md                   # 项目说明文档
├── include/                    # 头文件目录
│   ├── SimulatorMainWindow.h   # 主窗口类
│   ├── SimulationEngine.h      # 仿真引擎类
│   ├── STEPReader.h           # STEP文件读取类
│   └── SharedMemorySender.h   # 共享内存发送类
└── src/                        # 源文件目录
    ├── main.cpp               # 程序入口
    ├── SimulatorMainWindow.cpp
    ├── SimulationEngine.cpp
    ├── STEPReader.cpp
    └── SharedMemorySender.cpp
```

## 依赖库

### 必需依赖
- **Qt 5.12+**: Qt Widgets, Qt OpenGL
- **OpenCASCADE 7.5+**: 3D建模和可视化库
  - TKernel, TKMath, TKBRep
  - TKGeomBase, TKGeomAlgo
  - TKG3d, TKG2d
  - TKTopAlgo, TKPrim
  - TKSTEP, TKSTEPAttr, TKSTEPBase, TKSTEP209
  - TKXSBase, TKIGES
  - TKV3d, TKService, TKOpenGl

### 编译器要求
- C++17 或更高版本
- MSVC 2019+ (Windows)
- GCC 9+ (Linux)
- Clang 10+ (macOS)

## 构建说明

### Windows (使用CMake和Visual Studio)

1. **安装依赖**
   ```powershell
   # 安装Qt 5 (开源版)
   # 下载并安装 OpenCASCADE 7.5+
   ```

2. **配置环境变量**
   ```powershell
   # 设置Qt路径
   set Qt5_DIR=C:/Qt/5.15.2/msvc2019_64/lib/cmake/Qt5
   
   # 设置OpenCASCADE路径
   set OpenCASCADE_DIR=C:/OpenCASCADE-7.5.0/cmake
   ```

3. **生成项目**
   ```powershell
   mkdir build
   cd build
   cmake .. -G "Visual Studio 16 2019" -A x64
   ```

4. **编译**
   ```powershell
   cmake --build . --config Release
   ```

### Linux

1. **安装依赖**
   ```bash
   # Ubuntu/Debian
   sudo apt-get install qt5-default libqt5opengl5-dev
   sudo apt-get install liboce-foundation-dev liboce-modeling-dev liboce-visualization-dev
   
   # 或者从源码编译OpenCASCADE
   ```

2. **构建**
   ```bash
   mkdir build && cd build
   cmake ..
   make -j$(nproc)
   ```

### macOS

1. **安装依赖**
   ```bash
   brew install qt@5
   brew install opencascade
   ```

2. **构建**
   ```bash
   mkdir build && cd build
   cmake .. -DQt5_DIR=/usr/local/opt/qt@5/lib/cmake/Qt5
   make -j$(sysctl -n hw.ncpu)
   ```

## 使用说明

### 启动程序
```bash
# Windows
.\build\Release\SimulationTool.exe

# Linux/macOS
./build/SimulationTool
```

### 基本操作流程

1. **加载模型**
   - 点击菜单 "文件" -> "打开STEP文件"
   - 选择.step或.stp格式的3D模型文件
   - 模型将在3D视图中显示

2. **设置参数**
   - 在右侧参数面板中调整仿真参数
   - 时间步长: 控制仿真精度（越小越精确但越慢）
   - 总时长: 仿真的总时间
   - 阻尼系数: 系统阻尼
   - 刚度系数: 系统刚度

3. **运行仿真**
   - 点击工具栏的"开始仿真"按钮
   - 观察底部进度条和状态信息
   - 可以使用"暂停"和"停止"按钮控制仿真

4. **保存结果**
   - 点击菜单 "文件" -> "保存结果"
   - 选择保存位置和文件名

## 核心类说明

### SimulatorMainWindow
主窗口类，负责整个用户界面的创建和管理。

**主要功能:**
- 创建菜单栏、工具栏、参数面板
- 初始化OpenCASCADE 3D视图
- 管理仿真引擎和STEP读取器
- 处理用户交互事件

### SimulationEngine
仿真计算引擎，在独立线程中运行。

**主要功能:**
- 时间步进仿真计算
- 力的计算和运动积分
- 进度报告和状态更新
- 支持暂停/继续/停止控制

**仿真算法:**
- 简单的弹簧-阻尼系统
- 欧拉积分法（可扩展为更高级的方法）
- 支持多自由度系统

### STEPReader
STEP文件读取和几何处理类。

**主要功能:**
- 读取STEP格式文件
- 提取几何信息（面、边、顶点数量）
- 计算几何属性（体积、表面积、包围盒）
- 在AIS上下文中显示形状

### SharedMemorySender
共享内存通信类，用于进程间数据传输。

**主要功能:**
- 创建和管理共享内存段
- 发送结构化数据包
- 发送向量数据
- 线程安全操作

## 扩展开发

### 添加新的仿真算法
在`SimulationEngine`类中修改以下方法:
- `computeForces()`: 计算作用力
- `integrateMotion()`: 运动积分
- `checkConvergence()`: 收敛性检查

### 添加新的文件格式支持
参考`STEPReader`类，创建新的读取器类:
- 继承自`QObject`
- 实现文件读取和几何提取
- 使用OpenCASCADE的相应模块

### 自定义3D视图交互
在`SimulatorMainWindow::initializeOCC()`中:
- 配置视图参数
- 添加交互模式
- 自定义渲染选项

## 常见问题

### Q: 编译时找不到Qt或OpenCASCADE
A: 确保正确设置了`Qt5_DIR`和`OpenCASCADE_DIR`环境变量，指向相应的CMake配置目录。

### Q: 运行时找不到DLL/SO文件
A: 将Qt和OpenCASCADE的bin目录添加到系统PATH环境变量中。

### Q: 3D视图显示黑屏
A: 检查OpenGL驱动是否正确安装，确保支持OpenGL 3.3+。

### Q: STEP文件加载失败
A: 确认文件格式正确，检查文件路径中是否包含中文或特殊字符。

## 许可证

本项目使用Qt开源版（LGPL）和OpenCASCADE（LGPL）。

## 作者

MyAICAD Team

## 版本历史

- **v1.0.0** (2026-02-17)
  - 初始版本
  - 基础框架实现
  - STEP文件加载
  - 简单仿真引擎
  - 共享内存通信

## 贡献

欢迎提交Issue和Pull Request！

## 联系方式

如有问题或建议，请通过GitHub Issues联系。
