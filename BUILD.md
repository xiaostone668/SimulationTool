# 构建指南

## 快速开始

### Windows + Visual Studio 2022

1. **准备环境**
   ```powershell
   # 确保已安装:
   # - Visual Studio 2022 (带C++工作负载)
   # - CMake 3.16+
   # - Qt 5.15+ (MSVC版本)
   # - OpenCASCADE 7.5+
   ```

2. **配置CMake**
   ```powershell
   # 在项目根目录创建build目录
   mkdir build
   cd build
   
   # 生成Visual Studio解决方案
   cmake .. -G "Visual Studio 17 2022" -A x64 ^
     -DQt5_DIR="C:/Qt/5.15.2/msvc2019_64/lib/cmake/Qt5" ^
     -DOpenCASCADE_DIR="C:/OpenCASCADE-7.5.0/cmake"
   ```

3. **编译项目**
   ```powershell
   # 使用CMake编译
   cmake --build . --config Release
   
   # 或者打开生成的.sln文件在Visual Studio中编译
   start SimulationTool.sln
   ```

4. **运行程序**
   ```powershell
   # 确保Qt和OpenCASCADE的DLL在PATH中
   set PATH=C:\Qt\5.15.2\msvc2019_64\bin;C:\OpenCASCADE-7.5.0\win64\vc14\bin;%PATH%
   
   # 运行
   .\Release\SimulationTool.exe
   ```

### Linux (Ubuntu/Debian)

1. **安装依赖**
   ```bash
   # 更新包列表
   sudo apt-get update
   
   # 安装Qt5
   sudo apt-get install -y \
     qt5-default \
     libqt5opengl5-dev \
     qtbase5-dev
   
   # 安装OpenCASCADE (方法1: 使用包管理器)
   sudo apt-get install -y \
     liboce-foundation-dev \
     liboce-modeling-dev \
     liboce-ocaf-dev \
     liboce-visualization-dev
   
   # 或者从源码编译OpenCASCADE (方法2)
   # 参见OpenCASCADE官方文档
   ```

2. **构建项目**
   ```bash
   mkdir build && cd build
   cmake ..
   make -j$(nproc)
   ```

3. **运行**
   ```bash
   ./SimulationTool
   ```

### macOS

1. **安装依赖**
   ```bash
   # 使用Homebrew安装
   brew install qt@5
   brew install opencascade
   brew install cmake
   ```

2. **构建**
   ```bash
   mkdir build && cd build
   cmake .. \
     -DQt5_DIR=/usr/local/opt/qt@5/lib/cmake/Qt5 \
     -DOpenCASCADE_DIR=/usr/local/opt/opencascade/lib/cmake/opencascade
   make -j$(sysctl -n hw.ncpu)
   ```

3. **运行**
   ```bash
   ./SimulationTool
   ```

## 详细配置

### CMake选项

```bash
# 指定构建类型
cmake .. -DCMAKE_BUILD_TYPE=Release  # 或 Debug

# 指定安装路径
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local

# 启用详细输出
cmake .. -DCMAKE_VERBOSE_MAKEFILE=ON
```

### Qt配置

如果CMake找不到Qt，手动指定:

```bash
# Windows
cmake .. -DQt5_DIR="C:/Qt/5.15.2/msvc2019_64/lib/cmake/Qt5"

# Linux
cmake .. -DQt5_DIR="/usr/lib/x86_64-linux-gnu/cmake/Qt5"

# macOS
cmake .. -DQt5_DIR="/usr/local/opt/qt@5/lib/cmake/Qt5"
```

### OpenCASCADE配置

如果CMake找不到OpenCASCADE:

```bash
# Windows
cmake .. -DOpenCASCADE_DIR="C:/OpenCASCADE-7.5.0/cmake"

# Linux (从源码编译的)
cmake .. -DOpenCASCADE_DIR="/usr/local/lib/cmake/opencascade"

# macOS
cmake .. -DOpenCASCADE_DIR="/usr/local/opt/opencascade/lib/cmake/opencascade"
```

## 常见编译问题

### 问题1: 找不到Qt5
```
CMake Error: Could not find a package configuration file provided by "Qt5"
```

**解决方案:**
```bash
# 设置Qt5_DIR环境变量
export Qt5_DIR=/path/to/qt5/lib/cmake/Qt5  # Linux/macOS
set Qt5_DIR=C:\path\to\qt5\lib\cmake\Qt5   # Windows
```

### 问题2: 找不到OpenCASCADE
```
CMake Error: Could not find a package configuration file provided by "OpenCASCADE"
```

**解决方案:**
```bash
# 设置OpenCASCADE_DIR环境变量
export OpenCASCADE_DIR=/path/to/opencascade/cmake  # Linux/macOS
set OpenCASCADE_DIR=C:\path\to\opencascade\cmake   # Windows
```

### 问题3: 链接错误
```
undefined reference to `vtable for SimulatorMainWindow'
```

**解决方案:**
- 确保CMake的AUTOMOC选项已启用
- 清理build目录重新构建: `rm -rf build && mkdir build`

### 问题4: OpenGL相关错误
```
error: 'WNT_Window' was not declared in this scope
```

**解决方案:**
- 确保链接了TKOpenGl和TKV3d库
- 检查OpenCASCADE安装是否完整

## 性能优化

### Release构建
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

### 启用编译器优化
```bash
# GCC/Clang
cmake .. -DCMAKE_CXX_FLAGS="-O3 -march=native"

# MSVC
cmake .. -DCMAKE_CXX_FLAGS="/O2 /arch:AVX2"
```

## 调试构建

### Debug模式
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --config Debug
```

### 使用调试器
```bash
# GDB (Linux)
gdb ./SimulationTool

# LLDB (macOS)
lldb ./SimulationTool

# Visual Studio (Windows)
# 直接在VS中打开.sln文件并按F5
```

## 安装

```bash
# 编译后安装到系统
sudo cmake --install . --config Release

# 或指定安装路径
cmake --install . --prefix /opt/SimulationTool
```

## 打包发布

### Windows
```powershell
# 使用windeployqt复制Qt依赖
windeployqt.exe Release\SimulationTool.exe

# 手动复制OpenCASCADE DLL
copy C:\OpenCASCADE-7.5.0\win64\vc14\bin\*.dll Release\
```

### Linux
```bash
# 使用linuxdeployqt
linuxdeployqt SimulationTool -appimage
```

### macOS
```bash
# 使用macdeployqt
macdeployqt SimulationTool.app -dmg
```

## 持续集成

### GitHub Actions示例
```yaml
name: Build

on: [push, pull_request]

jobs:
  build:
    runs-on: ${{ matrix.os }}
    strategy:
      matrix:
        os: [ubuntu-latest, windows-latest, macos-latest]
    
    steps:
    - uses: actions/checkout@v2
    - name: Install Qt
      uses: jurplel/install-qt-action@v2
    - name: Build
      run: |
        mkdir build && cd build
        cmake ..
        cmake --build .
```

## 技术支持

如遇到构建问题，请提供以下信息:
- 操作系统和版本
- CMake版本 (`cmake --version`)
- Qt版本
- OpenCASCADE版本
- 完整的错误日志

在GitHub Issues中提交问题。
