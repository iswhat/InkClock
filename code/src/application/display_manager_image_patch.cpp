/**
 * DisplayManager 图片绘制功能补丁
 * 
 * 使用说明：
 * 1. 将此文件的内容复制到 display_manager.cpp 中对应的位置
 * 2. 替换原有的占位实现（第 1694-1783 行）
 */

// ========================================
// 替换第 1694-1709 行：drawImage 方法
// ========================================
bool DisplayManager::drawImage(String imagePath, int x, int y, int width, int height) {
  if (displayDriver == nullptr) {
    return false;
  }
  
  DEBUG_PRINT("绘制图片：");
  DEBUG_PRINTLN(imagePath);
  
  // 检查文件是否存在
  if (!SPIFFS.exists(imagePath)) {
    DEBUG_PRINT("图片文件不存在：");
    DEBUG_PRINTLN(imagePath);
    return false;
  }
  
  // 打开文件
  File file = SPIFFS.open(imagePath, FILE_READ);
  if (!file) {
    DEBUG_PRINTLN("无法打开图片文件");
    return false;
  }
  
  // 读取文件到缓冲区
  size_t fileSize = file.size();
  if (fileSize > MAX_IMAGE_BUFFER_SIZE) {
    DEBUG_PRINT("图片文件太大：");
    DEBUG_PRINTLN(fileSize);
    file.close();
    return false;
  }
  
  uint8_t* buffer = (uint8_t*)malloc(fileSize);
  if (!buffer) {
    DEBUG_PRINTLN("内存分配失败");
    file.close();
    return false;
  }
  
  file.read(buffer, fileSize);
  file.close();
  
  // 根据文件扩展名判断格式并解码
  bool result = false;
  if (imagePath.endsWith(".jpg") || imagePath.endsWith(".jpeg")) {
    result = decodeAndDrawJPEG(buffer, fileSize, x, y, width, height);
  } else if (imagePath.endsWith(".png")) {
    result = decodeAndDrawPNG(buffer, fileSize, x, y, width, height);
  } else if (imagePath.endsWith(".bmp")) {
    result = decodeAndDrawBMP(buffer, fileSize, x, y, width, height);
  }
  
  free(buffer);
  return result;
}

// ========================================
// 替换第 1711-1723 行：drawImageFromBuffer 方法
// ========================================
bool DisplayManager::drawImageFromBuffer(uint8_t* buffer, int bufferSize, int x, int y, int width, int height) {
  if (displayDriver == nullptr || buffer == nullptr || bufferSize == 0) {
    return false;
  }
  
  DEBUG_PRINTLN("从缓冲区绘制图片");
  
  // 根据图片头判断格式并解码
  if (bufferSize > 2 && buffer[0] == 0xFF && buffer[1] == 0xD8) {
    // JPEG 格式
    DEBUG_PRINTLN("检测到 JPEG 格式");
    return decodeAndDrawJPEG(buffer, bufferSize, x, y, width, height);
  } else if (bufferSize > 8 && buffer[0] == 0x89 && buffer[1] == 0x50 && buffer[2] == 0x4E && buffer[3] == 0x47) {
    // PNG 格式
    DEBUG_PRINTLN("检测到 PNG 格式");
    return decodeAndDrawPNG(buffer, bufferSize, x, y, width, height);
  } else if (bufferSize > 2 && buffer[0] == 0x42 && buffer[1] == 0x4D) {
    // BMP 格式
    DEBUG_PRINTLN("检测到 BMP 格式");
    return decodeAndDrawBMP(buffer, bufferSize, x, y, width, height);
  }
  
  DEBUG_PRINTLN("不支持的图片格式");
  return false;
}

// ========================================
// 替换第 1725-1738 行：drawImageFromURL 方法
// ========================================
bool DisplayManager::drawImageFromURL(String url, int x, int y, int width, int height) {
  if (displayDriver == nullptr) {
    return false;
  }
  
  DEBUG_PRINT("从 URL 绘制图片：");
  DEBUG_PRINTLN(url);
  
  // 使用 WiFi 客户端下载图片
  WiFiClientSecure client;
  client.setInsecure(); // 跳过 SSL 验证（生产环境应该验证证书）
  
  if (!client.connect("api.rolltools.cn", 443)) {
    DEBUG_PRINTLN("无法连接到服务器");
    return false;
  }
  
  // 发送 HTTP GET 请求
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: api.rolltools.cn\r\n" +
               "Connection: close\r\n\r\n");
  
  // 等待响应
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      DEBUG_PRINTLN("服务器响应超时");
      client.stop();
      return false;
    }
  }
  
  // 跳过 HTTP 头
  while (client.available()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
  }
  
  // 读取内容长度
  int contentLength = client.parseInt();
  if (contentLength <= 0 || contentLength > MAX_IMAGE_BUFFER_SIZE) {
    DEBUG_PRINT("无效的内容长度：");
    DEBUG_PRINTLN(contentLength);
    client.stop();
    return false;
  }
  
  // 分配缓冲区
  uint8_t* buffer = (uint8_t*)malloc(contentLength);
  if (!buffer) {
    DEBUG_PRINTLN("内存分配失败");
    client.stop();
    return false;
  }
  
  // 读取图片数据
  int bytesRead = 0;
  while (bytesRead < contentLength && client.connected()) {
    int len = client.read(buffer + bytesRead, contentLength - bytesRead);
    if (len > 0) {
      bytesRead += len;
    } else if (len < 0) {
      DEBUG_PRINT("读取错误：");
      DEBUG_PRINTLN(len);
      break;
    }
  }
  
  client.stop();
  
  if (bytesRead != contentLength) {
    DEBUG_PRINT("下载不完整：");
    DEBUG_PRINT(bytesRead);
    DEBUG_PRINT("/");
    DEBUG_PRINTLN(contentLength);
    free(buffer);
    return false;
  }
  
  // 从缓冲区绘制
  bool result = drawImageFromBuffer(buffer, contentLength, x, y, width, height);
  free(buffer);
  return result;
}

// ========================================
// 替换第 1740-1754 行：drawGIF 方法
// ========================================
bool DisplayManager::drawGIF(String gifPath, int x, int y, int width, int height) {
  if (displayDriver == nullptr) {
    return false;
  }
  
  DEBUG_PRINT("绘制 GIF 图片：");
  DEBUG_PRINTLN(gifPath);
  
  // 检查文件是否存在
  if (!SPIFFS.exists(gifPath)) {
    DEBUG_PRINT("GIF 文件不存在：");
    DEBUG_PRINTLN(gifPath);
    return false;
  }
  
  // 打开文件
  File file = SPIFFS.open(gifPath, FILE_READ);
  if (!file) {
    DEBUG_PRINTLN("无法打开 GIF 文件");
    return false;
  }
  
  // 读取文件到缓冲区
  size_t fileSize = file.size();
  if (fileSize > MAX_IMAGE_BUFFER_SIZE) {
    DEBUG_PRINT("GIF 文件太大：");
    DEBUG_PRINTLN(fileSize);
    file.close();
    return false;
  }
  
  uint8_t* buffer = (uint8_t*)malloc(fileSize);
  if (!buffer) {
    DEBUG_PRINTLN("内存分配失败");
    file.close();
    return false;
  }
  
  file.read(buffer, fileSize);
  file.close();
  
  // 解码并绘制 GIF
  bool result = decodeAndDrawGIF(buffer, fileSize, x, y, width, height);
  free(buffer);
  return result;
}

// ========================================
// 替换第 1756-1768 行：drawGIFFromBuffer 方法
// ========================================
bool DisplayManager::drawGIFFromBuffer(uint8_t* buffer, int bufferSize, int x, int y, int width, int height) {
  if (displayDriver == nullptr || buffer == nullptr || bufferSize == 0) {
    return false;
  }
  
  DEBUG_PRINTLN("从缓冲区绘制 GIF 图片");
  
  // 检查 GIF 文件头（GIF87a 或 GIF89a）
  if (bufferSize < 6) {
    DEBUG_PRINTLN("GIF 文件太小");
    return false;
  }
  
  if (memcmp(buffer, "GIF87a", 6) != 0 && memcmp(buffer, "GIF89a", 6) != 0) {
    DEBUG_PRINTLN("不是有效的 GIF 文件");
    return false;
  }
  
  // 解码并绘制 GIF
  return decodeAndDrawGIF(buffer, bufferSize, x, y, width, height);
}

// ========================================
// 替换第 1770-1783 行：drawGIFFromURL 方法
// ========================================
bool DisplayManager::drawGIFFromURL(String url, int x, int y, int width, int height) {
  if (displayDriver == nullptr) {
    return false;
  }
  
  DEBUG_PRINT("从 URL 绘制 GIF 图片：");
  DEBUG_PRINTLN(url);
  
  // 使用 WiFi 客户端下载 GIF（与 drawImageFromURL 类似的实现）
  WiFiClientSecure client;
  client.setInsecure();
  
  if (!client.connect("api.rolltools.cn", 443)) {
    DEBUG_PRINTLN("无法连接到服务器");
    return false;
  }
  
  // 发送 HTTP GET 请求
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: api.rolltools.cn\r\n" +
               "Connection: close\r\n\r\n");
  
  // 等待响应
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      DEBUG_PRINTLN("服务器响应超时");
      client.stop();
      return false;
    }
  }
  
  // 跳过 HTTP 头
  while (client.available()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
  }
  
  // 读取内容长度
  int contentLength = client.parseInt();
  if (contentLength <= 0 || contentLength > MAX_IMAGE_BUFFER_SIZE) {
    DEBUG_PRINT("无效的内容长度：");
    DEBUG_PRINTLN(contentLength);
    client.stop();
    return false;
  }
  
  // 分配缓冲区
  uint8_t* buffer = (uint8_t*)malloc(contentLength);
  if (!buffer) {
    DEBUG_PRINTLN("内存分配失败");
    client.stop();
    return false;
  }
  
  // 读取 GIF 数据
  int bytesRead = 0;
  while (bytesRead < contentLength && client.connected()) {
    int len = client.read(buffer + bytesRead, contentLength - bytesRead);
    if (len > 0) {
      bytesRead += len;
    } else if (len < 0) {
      break;
    }
  }
  
  client.stop();
  
  if (bytesRead != contentLength) {
    DEBUG_PRINT("下载不完整：");
    DEBUG_PRINT(bytesRead);
    DEBUG_PRINT("/");
    DEBUG_PRINTLN(contentLength);
    free(buffer);
    return false;
  }
  
  // 从缓冲区绘制 GIF
  bool result = drawGIFFromBuffer(buffer, contentLength, x, y, width, height);
  free(buffer);
  return result;
}
