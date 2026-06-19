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

### Qt 依赖详细说明

本项目依赖 Qt6 框架，以下是各 Qt 模块的具体使用场景：

#### QtCore（~35 个源文件使用）

| 功能 | 涉及文件 | 说明 |
|------|----------|------|
| 文件 I/O | `pdfdocumentwriter.cpp`, `pdfdocumentreader.cpp`, `pdfcms.cpp`, `pdfcertificatemanager.cpp`, `pdfcertificatestore.cpp`, `pdfapplicationtranslator.cpp` | `QFile`/`QIODevice`/`QBuffer`/`QSaveFile` 用于 PDF 文件读写、内存缓冲、安全写入 |
| 目录与路径 | `pdfcertificatestore.cpp`, `pdfcms.cpp`, `pdfcertificatemanager.cpp`, `pdfapplicationtranslator.cpp` | `QDir`/`QFileInfo`/`QStandardPaths` 处理证书目录、配置文件路径 |
| 字符串与编码 | `pdfencoding.cpp`, `pdfencoding.h`, `pdfcertificatemanager.h` | `QString`/`QStringDecoder` 处理 PDF 文本编码转换 |
| 线程与并发 | `pdfexecutionpolicy.cpp/h`, `pdfdiff.cpp/h`, `pdfdocumentreader.h`, `pdffont.h`, `pdfcms.h`, `pdfcertificatestore.h` | `QThread`/`QThreadPool`/`QSemaphore`/`QMutex`/`QRecursiveMutex` 管理并发和线程安全 |
| 日期时间 | `pdfdocument.h`, `pdfcertificatestore.h`, `pdfencoding.h`, `pdffile.h` | `QDateTime`/`QTimeZone` 解析 PDF 日期字段 |
| 加密哈希 | `pdfdocumentreader.cpp`, `pdfcolorspaces.cpp` | `QCryptographicHash` 计算文件/数据哈希 |
| 字节序转换 | `pdfmultimedia.cpp`, `pdfstreamfilters.cpp`, `pdfsecurityhandler.cpp` | `QtEndian` 处理大端/小端字节序 |
| 数学函数 | `pdfannotation.cpp`, `pdfrenderer.cpp`, `pdfpainter.cpp`, `pdfutils.cpp`, `pdftransparencyrenderer.cpp`, `pdfxfaengine.cpp`, `pdffunction.cpp` 等 ~10 个文件 | `QtMath` 提供 `qSqrt`/`qFloor` 等函数 |
| 信号/槽对象 | `pdfannotation.h`, `pdfcms.h`, `pdfdocumentsanitizer.h`, `pdfoptimizer.h`, `pdfrenderer.h`, `pdfprogress.h`, `pdfpagenavigation.h` 等 ~15 个文件 | `QObject`/`Q_OBJECT` 宏实现事件通知机制 |
| 命令行解析 | `app/main.cpp` | `QCommandLineParser`/`QCommandLineOption` 解析 CLI 参数 |
| 翻译与本地化 | `pdfapplicationtranslator.cpp/h`, `app/main.cpp` | `QTranslator`/`QCoreApplication`/`QLocale` 国际化 |
| 智能指针 | `pdfaction.h`, `pdfcolorspaces.h`, `pdfcms.h`, `pdffont.h` | `QSharedPointer` 管理对象生命周期 |
| XML 解析 (QtXml) | `pdfcertificatestore.cpp`, `pdfxfaengine.cpp` | `QDomDocument`/`QDomElement` 解析 AATL 证书列表和 XFA 表单 |
| PDF 写入 | `pdfdocumentbuilder.cpp` | `QPdfWriter` Qt 内置 PDF 写入器 |

#### QtGui（~20 个源文件使用）

| 功能 | 涉及文件 | 说明 |
|------|----------|------|
| 图像渲染 | `pdfblpainter.h/cpp`, `pdfrenderer.cpp`, `pdfimageoptimizer.cpp` | `QImage` 作为离屏渲染后端和图像压缩输入输出 |
| 色彩空间 | `pdfcolorspaces.cpp/h`, `pdfcolorconvertor.cpp/h`, `pdfdocumentmanipulator.h` | `QColor`/`QBrush`/`QPen` 处理 RGB/CMYK 颜色转换 |
| 字体渲染 | `pdffont.cpp/h`, `pdfdocumentbuilder.cpp` | `QFont`/`QFontMetricsF` 字体度量和管理 |
| 2D 绘图 | `pdfblendfunction.h`, `pdfdocumentbuilder.cpp` | `QPainter`/`QPaintEngine` 绘制操作 |
| 坐标变换 | `pdfdocument.h`, `pdffont.h`, `pdfmeshqualitysettings.h`, `pdfimagecompressor.h` | `QTransform`/`QPointF`/`QMarginsF` 仿射变换和坐标计算 |
| 图像编解码 | `pdfimageoptimizer.cpp` | `QImageWriter` JPEG/PNG 编码 |

#### QtSvg

未使用。可从依赖中移除（当前 CMakeLists.txt 中链接了但无实际代码引用）。

### 第三方库依赖详细说明

| 库 | 涉及文件 | 功能 |
|----|----------|------|
| **OpenSSL** | `pdfsecurityhandler.cpp/h`, `pdfsignaturehandler.cpp/h`, `pdfcertificatemanager.cpp/h`, `pdfcertificatestore.cpp/h`, `pdfdocumentreader.cpp` | PDF 加密、数字签名验证、证书管理 |
| **ZLIB** | `pdfstreamfilters.cpp/h`, `pdfimagecompressor.cpp/h`, `pdfoptimizer.cpp` | Flate 压缩/解压、PDF 流过滤 |
| **Freetype** | `pdffont.cpp/h`, `pdfrenderer.cpp` | TrueType/OpenType 字体解析和光栅化 |
| **OpenJPEG** | `pdfimageoptimizer.cpp`, `pdfimagecompressor.cpp/h` | JPEG2000 图像编解码 |
| **libjpeg** | `pdfimageoptimizer.cpp`, `pdfimagecompressor.cpp/h` | JPEG 图像编解码 |
| **libpng** | `pdfimageoptimizer.cpp`, `pdfimagecompressor.cpp/h` | PNG 图像编解码 |
| **blend2d** | `pdfblpainter.cpp/h`, `pdfrenderer.cpp` | 2D 矢量图形渲染（可选，默认使用 QPainter 后端） |
| **lcms2** | `pdfcms.cpp/h`, `pdfcolorconvertor.cpp/h`, `pdfcolorspaces.cpp/h` | ICC 色彩配置文件管理、色彩空间转换 |

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
