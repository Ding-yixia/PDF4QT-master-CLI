# PDF4QT Simplify (CLI)

命令行版 PDF4QT，专注于 PDF 文档优化、图像压缩、文档净化、对象统计与检查。

> 本项目提取自 [PDF4QT](https://github.com/Ding-yixia/PDF4QT-master) 的底层实现，去除 GUI 界面，保留核心功能。

## 功能

| 命令 | 说明 |
|------|------|
| `sanitize` | 文档净化：移除文档信息、元数据、大纲、文件附件、内嵌搜索索引、批注、页面缩略图、页面标签、不可见文本（OCR 层） |
| `optimize` | 文档优化：嵌入简单对象、移除空对象、移除未使用对象、合并相同对象、缩小对象存储、重新压缩 Flate 流 |
| `image-optimize` | 图像优化：重采样、颜色模式转换（彩色/灰度/黑白）、压缩算法选择（JPEG/JPEG2000/Flate）、目标 DPI 设置、JPEG 质量调整 |
| `statistics` | 对象统计：按对象类别或类型统计数量和大小 |
| `inspect` | 对象检查：查看 PDF 内部对象结构（文档、页面、图像、字体、批注等） |

## 依赖

### 核心依赖

| 依赖 | 最低版本 | 说明 |
|------|----------|------|
| **CMake** | 3.16+ | 构建系统 |
| **C++** | C++20 | 编译器需支持 C++20 |
| **Qt6** | 6.0+ | Core, Gui, Svg, Xml |

### 第三方库

| 库 | 说明 |
|----|------|
| OpenSSL | 加密与证书处理 |
| ZLIB | 数据压缩 |
| Freetype | 字体渲染 |
| OpenJPEG | JPEG2000 编解码 |
| libjpeg | JPEG 编解码 |
| libpng | PNG 编解码 |
| blend2d | 2D 图形渲染 |
| lcms2 | 颜色管理系统 |

### 推荐安装方式（vcpkg）

```bash
# 安装 vcpkg
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# 安装所有依赖
.\vcpkg install qt6[core,gui,svg,xml]:x64-windows openssl:x64-windows zlib:x64-windows freetype:x64-windows openjpeg:x64-windows libjpeg-turbo:x64-windows libpng:x64-windows blend2d:x64-windows lcms:x64-windows
```

## 构建

### Windows (MSVC + vcpkg)

```bash
cd pdf_simplify
mkdir build && cd build

# 配置（替换 vcpkg 路径）
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=C:/Code/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows

# 编译（/MP 已启用，自动多核并行编译）
cmake --build . --config Release
```

编译产物位于 `build/bin/Release/pdf_simplify.exe`。

### Linux (GCC/Clang)

```bash
cd pdf_simplify
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
```

## 使用

### 基本用法

```
pdf_simplify <command> [options] <input.pdf> [output.pdf]
```

### 示例

#### 1. 文档净化（最大净化）

```bash
pdf_simplify sanitize \
    --remove-document-info \
    --remove-metadata \
    --remove-outline \
    --remove-file-attachments \
    --remove-embedded-search-index \
    --remove-markup-annotations \
    --remove-page-thumbnails \
    --remove-page-labels \
    --remove-invisible-text \
    input.pdf output.pdf
```

#### 2. 文档优化（最大优化）

```bash
pdf_simplify optimize \
    --dereference-simple-objects \
    --remove-null-objects \
    --remove-unused-objects \
    --merge-identical-objects \
    --shrink-object-storage \
    --recompress-flate-streams \
    input.pdf output.pdf
```

#### 3. 图像优化（最小文件大小）

```bash
pdf_simplify image-optimize \
    --goal minimum-size \
    --keep-original \
    --color-mode auto \
    --color-algorithm auto \
    --color-target-dpi 72 \
    --gray-target-dpi 72 \
    --bitonal-target-dpi 150 \
    --png-predictor \
    --resample-filter lanczos \
    input.pdf output.pdf
```

#### 4. 完整优化流程（净化 → 优化 → 图像优化）

```bash
pdf_simplify sanitize --remove-document-info --remove-metadata --remove-outline --remove-file-attachments --remove-embedded-search-index --remove-markup-annotations --remove-page-thumbnails --remove-page-labels --remove-invisible-text input.pdf step1.pdf
pdf_simplify optimize --dereference-simple-objects --remove-null-objects --remove-unused-objects --merge-identical-objects --shrink-object-storage --recompress-flate-streams step1.pdf step2.pdf
pdf_simplify image-optimize --goal minimum-size --keep-original --color-mode auto --color-algorithm auto --color-target-dpi 72 --gray-target-dpi 72 --bitonal-target-dpi 150 --png-predictor --resample-filter lanczos step2.pdf output.pdf
```

#### 5. 查看文档对象统计

```bash
pdf_simplify statistics --by-class input.pdf
pdf_simplify statistics --by-type input.pdf
```

#### 6. 检查文档内部结构

```bash
pdf_simplify inspect -m document input.pdf
pdf_simplify inspect -m images input.pdf
pdf_simplify inspect -m fonts input.pdf
```

## 压缩效果

典型压缩效果（基于 5-10 MB 测试文件）：

| 文件 | 原始大小 | 压缩后 | 压缩率 |
|------|----------|--------|--------|
| 70_5MB-10MB.pdf | 5.26 MB | 922 KB | 82.5% |
| 102_5MB-10MB.pdf | 10.38 MB | 1.80 MB | 82.6% |

## 许可证

MIT License（继承自原始 PDF4QT 项目）

Copyright (c) 2018-2025 Jakub Melka and Contributors
