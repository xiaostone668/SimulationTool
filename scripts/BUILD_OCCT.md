# 从源码编译 OCCT（含 STEP 支持）

按以下步骤可获得带完整 STEP 的 OCCT，供 SimulationTool 使用。

## 1. 准备环境

- **Visual Studio 2022**：64 位（本脚本默认使用 VS2022；若使用 VS2019，需修改脚本参数）
- **CMake**：3.16+，已加入 PATH
- **Git**：用于拉取 OCCT 源码（可选，也可手动下载源码包）

> **注意**：OCCT 安装目录中可能仍显示为 `win64/vc14`，这是 OCCT 的命名约定（VS2017/2019/2022 均使用 vc14 子目录名），不影响使用。

## 2. 下载 OCCT 源码

**方式 A：Git 克隆（推荐）**

```powershell
mkdir D:\MyAICADCode\occt-src -Force
cd D:\MyAICADCode\occt-src
git clone --depth 1 --branch V7_9_0 https://github.com/Open-Cascade-SAS/OCCT.git .
# 或指定 7.9.3 标签: git clone --depth 1 --branch V7_9_3 https://github.com/Open-Cascade-SAS/OCCT.git .
```

**方式 B：手动下载**

- 打开 https://github.com/Open-Cascade-SAS/OCCT/releases
- 下载 7.9.0 或 7.9.3 的 Source code (zip)，解压到例如 `D:\MyAICADCode\occt-src`。

## 3. 下载第三方库（3rd party）

OCCT 可视化需要 **FreeType**；若不需要 Draw 测试工具，可不装 Tcl/Tk。

1. 打开 https://dev.opencascade.org/resources/download/3rd-party-components  
2. 下载 **vc14 / 64 bit** 对应包：
   - **Freetype 2.5.5**：[freetype-2.5.5-vc14-64.zip](https://dev.opencascade.org/system/files/occt/3rdparty/freetype-2.5.5-vc14-64.zip)  
   - （可选）**Tcl/Tk 8.6**：[tcltk-86-64.zip](https://dev.opencascade.org/system/files/occt/3rdparty/tcltk-86-64.zip)（仅在使用 Draw 时需要）
3. 新建目录，例如 `D:\SDK\occt-3rdparty-vc14-64`。
4. 将上述 zip 解压到该目录，保证结构类似：
   ```
   D:\SDK\occt-3rdparty-vc14-64\
     freetype-2.5.5-vc14-64\   (或解压后的 freetype 目录名)
       include\
       lib\
       bin\
     tcltk-86-64\              (可选)
       ...
   ```
5. 记下该路径，下面作为 `3RDPARTY_DIR` 使用。

## 4. 配置与编译

在 PowerShell 中执行（请按本机路径修改前几行变量）：

```powershell
# 路径设置（按需修改）
$OCC_SRC    = "D:\MyAICADCode\occt-src"           # OCCT 源码目录
$OCC_BUILD  = "D:\MyAICADCode\occt-build"         # 构建目录
$OCC_INSTALL= "D:\SDK\occt-7.9.3-vc14-64-full"   # 安装目录（SimulationTool 将用此作 OCC_ROOT）
$3RDPARTY   = "D:\SDK\occt-3rdparty-vc14-64"      # 第三方库根目录

# 生成器：默认使用 VS2022（Visual Studio 17 2022），64 位
# 若使用 VS2019，改为: $Generator = "Visual Studio 16 2019 -A x64"
$Generator  = "Visual Studio 17 2022 -A x64"

New-Item -ItemType Directory -Force -Path $OCC_BUILD | Out-Null
cd $OCC_BUILD

cmake -S $OCC_SRC -B $OCC_BUILD `
  -G "Visual Studio 17 2022" -A x64 `
  -DCMAKE_INSTALL_PREFIX="$OCC_INSTALL" `
  -D3RDPARTY_DIR="$3RDPARTY" `
  -DBUILD_MODULE_Draw=OFF `
  -DBUILD_DOC_Overview=OFF `
  -DBUILD_SAMPLES_MFC=OFF `
  -DBUILD_SAMPLES_QT=OFF `
  -DUSE_FREETYPE=ON

# 编译（约 10–30 分钟）
cmake --build $OCC_BUILD --config Release -j

# 安装到 $OCC_INSTALL
cmake --install $OCC_BUILD --config Release
```

- `BUILD_MODULE_Draw=OFF`：不编译 Draw，可不装 Tcl/Tk。  
- 若已装 Tcl/Tk 且希望用 Draw，可改为 `ON` 并确保 `3RDPARTY_DIR` 下有 tcltk。  
- 安装完成后，目录结构通常为：`<OCC_INSTALL>/win64/vc14/bin`、`<OCC_INSTALL>/win64/vc14/lib`、`<OCC_INSTALL>/inc`。

## 5. 用新 OCCT 编译 SimulationTool

将 SimulationTool 的 `OCC_ROOT` 指向刚安装的目录，并重新配置、编译：

```powershell
cd D:\MyAICADCode\SimulationTool\build
Remove-Item CMakeCache.txt -ErrorAction SilentlyContinue
cmake .. -DOCC_ROOT=D:/SDK/occt-7.9.3-vc14-64-full
cmake --build . --config Release
```

若安装目录不同，将上面 `OCC_ROOT` 改为你的 `$OCC_INSTALL` 路径（使用正斜杠 `/`）。

## 6. 故障排查

- **CMake 报错找不到 FreeType**  
  检查 `3RDPARTY_DIR` 下是否有 freetype 子目录，且内含 `include`、`lib`（及可选 `bin`）。

- **链接错误缺少 TKSTEP**  
  确认 OCCT 配置时未关闭 DataExchange 模块（默认开启），且安装后 `win64/vc14/lib` 下存在 `TKSTEP.lib` 等。

- **Visual Studio 版本**  
  脚本默认使用 **VS2022**（Visual Studio 17 2022）。若需使用 VS2019，运行脚本时添加参数：`-Generator "Visual Studio 16 2019 -A x64"`。  
  安装目录中可能仍显示为 `win64/vc14`，这是 OCCT 的命名约定（VS2017/2019/2022 均用 vc14），不影响使用。
