# MRN 压缩工具

[![Version](https://img.shields.io/badge/version-2.0.0-blue.svg)](https://github.com)
[![C++](https://img.shields.io/badge/C++-17-orange.svg)](https://en.cppreference.com/w/cpp/17)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

MRN 是一款高性能的插件化压缩工具，采用 MoveRun 压缩算法管线，提供可扩展的预处理、熵编码以及完整的命令行接口。支持单文件和目录压缩，生成 `.mrn` 格式的归档文件。

## ✨ 功能特点

### 核心功能
- **插件化架构**：支持动态注册压缩算法和预处理器，易于扩展
- **MoveRun 算法管线**：Move优化 → LZ77压缩 → Huffman编码的三级压缩流程
- **多线程并行**：支持多线程并行压缩，充分利用多核CPU性能
- **智能预设**：内置 text/binary/maximum/fast 预设，支持自动文件类型检测
- **完整归档格式**：`.mrn` 格式支持多文件归档，包含元数据（时间戳、权限、校验和）

### 高级特性
- **目录扫描**：递归扫描目录，支持过滤规则（包含/排除模式、最大文件大小）
- **归档管理**：支持列出归档内容、测试归档完整性
- **配置系统**：支持用户自定义配置文件和预设
- **文件权限**：压缩时保存文件权限，解压时自动恢复
- **数据校验**：使用CRC32校验和确保数据完整性

## 🚀 快速开始

### 系统要求
- C++17 编译器（GCC 7+, Clang 5+, MSVC 2017+）
- CMake 3.15 或更高版本
- zlib 库（通常系统已预装）

### 构建项目

```bash
# 克隆仓库（如果适用）
git clone https://github.com/lezi-fun/mrn.git
cd mrn

# 配置构建
cmake -S . -B build

# 编译
cmake --build build

# 安装（可选）
cmake --install build
```

### 基本使用

```bash
# 压缩目录
./build/mrn -c my_folder -o archive.mrn

# 压缩单个文件
./build/mrn -c file.txt -o archive.mrn

# 解压归档
./build/mrn -d archive.mrn -o output_dir

# 列出归档内容
./build/mrn -l archive.mrn

# 测试归档完整性
./build/mrn -t archive.mrn
```

## 📖 使用指南

### 命令行选项

#### 基本操作
- `-c, --compress`：压缩模式（默认）
- `-d, --decompress`：解压模式
- `-l, --list`：列出归档内容
- `-t, --test`：测试归档完整性
- `-o, --output <path>`：指定输出路径（必需）

#### 压缩选项
- `--preset <name>`：使用预设（text/binary/maximum/fast/auto）
- `--algorithm <name>`：指定压缩算法（默认：moverun）
- `-j, --threads <num>`：指定线程数（默认：自动检测）
- `-v, --verbose`：详细输出模式

#### 其他选项
- `--overwrite`：覆盖已存在的文件
- `--preserve-paths`：保留文件路径结构（默认：true）
- `--no-preserve-paths`：不保留路径结构

### 使用示例

```bash
# 使用最大压缩预设
./build/mrn -c data/ -o archive.mrn --preset maximum -v

# 使用4个线程压缩
./build/mrn -c large_folder/ -o archive.mrn -j4

# 自动检测最佳预设
./build/mrn -c document.txt -o doc.mrn --preset auto

# 列出归档详细信息
./build/mrn -l archive.mrn

# 测试并验证归档
./build/mrn -t archive.mrn
```

### 预设说明

- **text**：针对文本文件优化（压缩级别 6）
- **binary**：针对二进制文件优化（压缩级别 5）
- **maximum**：最大压缩比（压缩级别 9，速度较慢）
- **fast**：快速压缩（压缩级别 3，速度优先）
- **auto**：根据文件扩展名自动选择最佳预设（推荐）

### 智能文件类型优化

MRN 会根据文件扩展名自动选择最优压缩策略，避免对已压缩文件进行无效压缩：

#### 🎬 视频和音频文件（直接存储）
视频和音频文件通常已高度压缩（如 MP4、MP3），再次压缩效果极差且耗时。MRN 会自动检测并直接存储：
- **视频**: MP4, AVI, MKV, MOV, WMV, FLV, WebM, M4V, MPG, MPEG, 3GP, OGV, TS, MTS
- **音频**: MP3, AAC, OGG, WMA, FLAC, M4A, WAV, Opus, APE

#### 🖼️ 图片文件（智能处理）
- **已压缩图片**（JPG, PNG, GIF, WebP）：使用快速模式，可能仍有少量冗余
- **未压缩图片**（BMP, TIFF, RAW, PSD）：使用二进制压缩，可获得较好压缩比

#### 📄 文档文件（分类优化）
- **文本文件**（TXT, MD, LOG）：使用文本压缩，压缩比高
- **源代码**（CPP, PY, JS, GO等）：使用文本压缩，充分利用代码冗余
- **PDF/Office文档**：使用快速模式，避免无效压缩

#### 📦 已压缩归档（直接存储）
ZIP, GZ, BZ2, 7Z, RAR 等已压缩文件直接存储，避免重复压缩。

#### 💡 使用建议
- **推荐使用 `--preset auto`**：让系统自动选择最佳策略（默认行为）
- **视频/音频文件**：系统会自动跳过压缩，直接存储，节省大量时间
- **文本和源代码**：系统会自动使用文本压缩，获得最佳压缩比
- **混合文件类型**：压缩目录时，每个文件会根据其类型自动选择最优策略

#### 📈 压缩效果对比

| 文件类型 | 策略 | 预期压缩率 | 说明 |
|---------|------|-----------|------|
| 视频文件 (MP4, AVI等) | 直接存储 | 0% | 已高度压缩，避免无效压缩 |
| 音频文件 (MP3, AAC等) | 直接存储 | 0% | 已压缩格式，直接存储 |
| 文本文件 (TXT, MD等) | 文本压缩 | 50-80% | 高压缩比 |
| 源代码 (CPP, PY等) | 文本压缩 | 60-85% | 充分利用代码冗余 |
| 已压缩图片 (JPG, PNG) | 快速压缩 | 0-5% | 可能仍有少量冗余 |
| 未压缩图片 (BMP, RAW) | 二进制压缩 | 20-50% | 可获得较好压缩比 |
| 已压缩归档 (ZIP, 7Z) | 直接存储 | 0% | 避免重复压缩 |

## 🏗️ 项目架构

### 目录结构
```
mrn/
├── include/          # 头文件
│   ├── core/        # 核心模块（压缩器、插件管理、配置）
│   ├── algorithms/  # 压缩算法（MoveRun、LZ77、Huffman）
│   ├── io/          # I/O模块（文件操作、目录扫描、归档）
│   └── utils/       # 工具模块（线程池、日志、进度追踪）
├── src/             # 源文件
├── plugins/         # 插件系统
│   ├── entropy_coders/    # 熵编码器插件
│   ├── preprocessors/     # 预处理器插件
│   └── specialized/       # 专用压缩器插件
├── tests/           # 测试代码
└── docs/            # 文档
```

### 核心模块

#### 插件系统
- **ICompressionAlgorithm**：压缩算法接口
- **IPreprocessor**：预处理器接口
- **PluginManager**：插件管理器（单例模式）

#### 压缩流水线
1. **MoveOptimizer**：数据移动优化
2. **LZ77Compressor**：基于zlib的LZ77压缩
3. **HuffmanEncoder**：Huffman熵编码

#### 归档格式
- **MRNArchiveHeader**：归档头部（版本、文件数、大小等）
- **FileEntryHeader**：文件条目（文件名、大小、偏移、校验和、权限等）

## 🔧 开发指南

### 添加新压缩算法

1. 实现 `ICompressionAlgorithm` 接口：

```cpp
class MyAlgorithm : public ICompressionAlgorithm {
public:
    static std::string getStaticName() { return "myalgo"; }
    std::string getName() const override { return "My Algorithm"; }
    // ... 实现其他接口方法
};
```

2. 使用 `REGISTER_ALGORITHM` 宏注册：

```cpp
REGISTER_ALGORITHM(MyAlgorithm);
```

### 添加新预处理器

1. 实现 `IPreprocessor` 接口
2. 在插件管理器中注册
3. 在压缩流水线配置中使用

### 配置文件

支持用户自定义配置文件，格式示例：

```ini
# 文件类型关联
filetype.txt=text
filetype.jpg=binary

# 自定义预设
preset.my_preset.algorithm=moverun
preset.my_preset.level=7
```

## 📊 性能特性

- **多线程并行**：充分利用多核CPU，大幅提升压缩速度
- **智能压缩**：小文件自动判断是否压缩，避免负压缩
- **内存高效**：流式处理，支持大文件压缩
- **快速解压**：优化的解压流程，支持快速提取

## 🧪 测试

```bash
# 构建测试（如果启用）
cmake --build build -DMRN_BUILD_TESTS=ON

# 运行测试
ctest --test-dir build
```

## 📝 许可证

本项目采用 MIT 许可证，详见 [LICENSE](LICENSE) 文件。

## 🤝 贡献

欢迎提交 Issue 和 Pull Request！

## 📚 相关文档

- [开发文档](build.md)：详细的架构设计和开发指南
- [完成清单](CHECKLIST.md)：功能完成情况检查表
- [架构文档](docs/architecture.md)：系统架构概览

## 🐛 已知问题

- Huffman编码器对小文件可能产生负压缩（已自动处理）
- 某些特殊文件权限在跨平台时可能不完全兼容

## 🔮 未来计划

- [ ] 支持更多压缩算法（BWT、算术编码等）
- [ ] 支持加密归档
- [ ] 支持增量备份
- [ ] GUI界面
- [ ] 更多预处理器插件

---

**注意**：本项目为示例性实现，生产环境使用前请充分测试。
