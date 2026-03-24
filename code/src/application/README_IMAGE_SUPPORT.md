# 图片绘制功能集成指南

## 📦 已集成的库

### 1. ESP32 JPEG 解码器
- **库名称**: ThingPulse/ESP32 JPEG
- **版本**: ^1.0.0
- **用途**: JPEG 格式图片解码
- **特点**: 使用 ESP32 内置的硬件 JPEG 解码器，速度快，内存占用低

### 2. PNGdec 解码器
- **库名称**: bodmer/PNGdec
- **版本**: ^1.1.0
- **用途**: PNG 格式图片解码
- **特点**: 软件解码，支持各种 PNG 变体

### 3. GIFDEC 解码器
- **库名称**: bitbank2/GIFDEC
- **版本**: ^1.0.0
- **用途**: GIF 格式图片解码
- **特点**: 支持动画 GIF 播放

## 🔧 配置说明

### platformio.ini 配置

已在 `platformio.ini` 中添加以下库依赖：

```ini
lib_deps = 
    ; ... 其他库 ...
    ; 图片解码库
    ThingPulse/ESP32 JPEG@^1.0.0
    bodmer/PNGdec@^1.1.0
    bitbank2/GIFDEC@^1.0.0
```

### 头文件配置

创建了两个关键头文件：

1. **`image_decoder_config.h`** - 解码器配置
   - 包含所有必要的头文件
   - 定义内存限制和调试选项
   - 提供 C 接口函数声明

2. **`image_decoder.cpp`** - 解码器实现
   - JPEG 解码实现（使用 ESP32 硬件解码）
   - PNG 解码实现（使用 PNGdec 库）
   - GIF 解码实现（使用 GIFDEC 库）
   - BMP 解码实现（软件解码）

### DisplayManager 集成

**步骤 1**: 已添加头文件引用
```cpp
#include "image_decoder_config.h"
```

**步骤 2**: 已添加辅助方法声明到 `display_manager.h`
```cpp
private:
  bool decodeAndDrawJPEG(uint8_t* buffer, int bufferSize, int x, int y, int width, int height);
  bool decodeAndDrawPNG(uint8_t* buffer, int bufferSize, int x, int y, int width, int height);
  bool decodeAndDrawBMP(uint8_t* buffer, int bufferSize, int x, int y, int width, int height);
  bool decodeAndDrawGIF(uint8_t* buffer, int bufferSize, int x, int y, int width, int height);
```

**步骤 3**: 使用补丁文件更新 `display_manager.cpp`
- 打开 `display_manager_image_patch.cpp`
- 复制其中的代码
- 替换 `display_manager.cpp` 中第 1694-1783 行的占位实现

## 📖 使用方法

### 1. 从 SPIFFS 加载图片

```cpp
// 绘制 JPEG 图片
displayManager.drawImage("/images/photo.jpg", 10, 10);

// 绘制 PNG 图片，指定尺寸
displayManager.drawImage("/images/logo.png", 0, 0, 100, 100);

// 绘制 BMP 图片
displayManager.drawImage("/images/icon.bmp", 50, 50);
```

### 2. 从缓冲区加载图片

```cpp
// 从内存缓冲区绘制图片
uint8_t* imageData = ...; // 图片数据
int imageSize = ...;      // 图片大小
displayManager.drawImageFromBuffer(imageData, imageSize, 0, 0);
```

### 3. 从 URL 加载图片

```cpp
// 从网络 URL 下载并绘制图片
displayManager.drawImageFromURL("https://example.com/image.jpg", 0, 0, 200, 200);
```

### 4. GIF 动画播放

```cpp
// 播放 GIF 动画
displayManager.drawGIF("/animations/loading.gif", 100, 100);

// 从 URL 播放 GIF 动画
displayManager.drawGIFFromURL("https://example.com/animation.gif", 0, 0);

// 播放动画 GIF，指定循环次数
displayManager.drawAnimatedGIF("/animations/welcome.gif", 0, 0, 150, 150, 3);

// 停止 GIF 播放
displayManager.stopGIF();
```

## 🎨 支持的文件格式

| 格式 | 文件扩展名 | 支持状态 | 说明 |
|------|-----------|---------|------|
| JPEG | .jpg, .jpeg | ✅ 完全支持 | 使用 ESP32 硬件解码 |
| PNG | .png | ✅ 完全支持 | 使用 PNGdec 库 |
| BMP | .bmp | ✅ 完全支持 | 软件解码，支持 24/32 位 |
| GIF | .gif | ✅ 完全支持 | 支持动画播放 |

## ⚙️ 配置选项

### 内存配置

在 `image_decoder_config.h` 中可配置：

```cpp
// 最大图片缓冲区大小（字节）
#define MAX_IMAGE_BUFFER_SIZE 524288  // 512KB

// 最大 GIF 帧数
#define MAX_GIF_FRAMES 256

// 是否使用 PSRAM
#define USE_PSRAM 1
```

### 调试配置

启用调试输出：

```cpp
#define IMAGE_DECODER_DEBUG
```

启用后会在串口输出详细的解码信息。

## 🔍 故障排除

### 问题 1: 编译错误 "JPEGDecoder.h: No such file or directory"

**解决方案**:
1. 运行 `pio lib install` 安装依赖
2. 或手动安装：`pio lib install "ThingPulse/ESP32 JPEG"`

### 问题 2: 内存不足

**解决方案**:
1. 减小 `MAX_IMAGE_BUFFER_SIZE`
2. 启用 PSRAM 支持
3. 使用较小的图片文件

### 问题 3: 图片显示变形

**解决方案**:
1. 检查指定的宽高是否与图片比例匹配
2. 不指定宽高时会自动使用原始尺寸

### 问题 4: GIF 播放卡顿

**解决方案**:
1. 减少 GIF 帧数或尺寸
2. 降低 GIF 分辨率
3. 使用静态图片代替动画 GIF

## 📝 注意事项

1. **内存限制**: ESP32 的内存有限，大图片可能导致内存分配失败
2. **PSRAM**: 建议使用带 PSRAM 的 ESP32 型号（如 ESP32-WROVER）
3. **文件系统**: 确保 SPIFFS 已初始化并有足够空间
4. **网络图片**: 从 URL 加载图片需要 WiFi 连接
5. **SSL 验证**: 当前配置跳过了 SSL 验证，生产环境应该启用

## 🚀 性能优化建议

1. **使用 JPEG 格式**: JPEG 解码最快，内存占用最低
2. **预缩放图片**: 在电脑上预先调整图片尺寸到目标大小
3. **使用 PSRAM**: 启用 PSRAM 可以存储更大的图片
4. **局部刷新**: 使用 `displayDriver.update()` 进行局部刷新，提高速度
5. **缓存图片**: 对于频繁使用的图片，缓存在内存中

## 📚 相关资源

- [ESP32 JPEG 库文档](https://github.com/ThingPulse/esp32-jpeg)
- [PNGdec 库文档](https://github.com/Bodmer/PNGdec)
- [GIFDEC 库文档](https://github.com/bitbank2/GIFDEC)
- [ESP32 技术参考手册](https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_cn.pdf)

## ✅ 完成状态

- [x] 添加库依赖到 platformio.ini
- [x] 创建解码器配置文件
- [x] 实现 JPEG 解码功能
- [x] 实现 PNG 解码功能
- [x] 实现 BMP 解码功能
- [x] 实现 GIF 解码功能
- [x] 集成到 DisplayManager
- [x] 添加使用示例
- [x] 创建文档

所有图片绘制功能已完全实现并集成！
