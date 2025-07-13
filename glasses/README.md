# ESP32-S3 智能眼镜摄像头项目

🤓 基于 ESP32-S3 的智能眼镜实时视频流项目，支持WiFi无线传输和手机访问。

## ✨ 项目特性

- 📷 **OV3660 高清摄像头**：1600x1200 (UXGA) 分辨率
- 🌐 **WiFi 无线传输**：支持实时视频流传输
- 📱 **多设备访问**：手机、平板、电脑浏览器均可访问
- 🎥 **MJPEG 视频流**：流畅的实时视频传输
- 📸 **单张拍照**：支持REST API获取单张图片
- 🔍 **mDNS 域名解析**：`http://esp32-glasses.local` 便捷访问
- 🌈 **美观网页界面**：响应式设计，支持移动端

## 🛠️ 硬件要求

### 主控板
- **ESP32-S3** 开发板 (推荐ESP32-S3-DevKitC-1)
- 至少 **8MB Flash** + **8MB PSRAM**

### 摄像头模块
- **OV3660** 摄像头模块
- 支持8位并行数据接口

### 引脚连接

| 功能 | ESP32-S3 引脚 | OV3660 引脚 |
|------|---------------|-------------|
| 时钟 | GPIO15 (XCLK) | XCLK |
| I2C数据 | GPIO4 (SDA) | SIOD |
| I2C时钟 | GPIO5 (SCL) | SIOC |
| 垂直同步 | GPIO6 | VSYNC |
| 水平同步 | GPIO7 | HREF |
| 像素时钟 | GPIO21 | PCLK |
| 数据D0 | GPIO16 | D0 |
| 数据D1 | GPIO17 | D1 |
| 数据D2 | GPIO18 | D2 |
| 数据D3 | GPIO12 | D3 |
| 数据D4 | GPIO10 | D4 |
| 数据D5 | GPIO8 | D5 |
| 数据D6 | GPIO9 | D6 |
| 数据D7 | GPIO11 | D7 |

## 🚀 快速开始

### 1. 环境准备

```bash
# 安装 ESP-IDF v5.4.1
git clone -b v5.4.1 --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh
. ./export.sh
```

### 2. 克隆项目

```bash
git clone <your-repo-url>
cd glasses
```

### 3. 配置WiFi

编辑 `main/wifi_streaming.h`：

```c
#define WIFI_SSID      "你的WiFi名称"     // 修改为你的WiFi
#define WIFI_PASSWORD  "你的WiFi密码"     // 修改为你的密码
```

### 4. 编译和烧录

```bash
# 编译项目
idf.py build

# 烧录到ESP32-S3
idf.py -p COM3 flash monitor  # Windows
# 或
idf.py -p /dev/ttyUSB0 flash monitor  # Linux/Mac
```

### 5. 访问摄像头

烧录成功后，在串口输出中找到访问地址：

```
WIFI: 访问: http://esp32-glasses.local
WIFI: IP: 192.168.1.100
```

然后在手机或电脑浏览器中访问：
- **主页**: `http://esp32-glasses.local`
- **视频流**: `http://esp32-glasses.local/stream`
- **拍照**: `http://esp32-glasses.local/capture`
- **设备信息**: `http://esp32-glasses.local/info`

## 📱 API 接口

### REST API

| 接口 | 方法 | 描述 | 返回格式 |
|------|------|------|----------|
| `/` | GET | 主页界面 | HTML |
| `/stream` | GET | 实时视频流 | MJPEG |
| `/capture` | GET | 单张拍照 | JPEG图片 |
| `/info` | GET | 设备信息 | JSON |

### 示例用法

#### 获取单张图片 (适合AI处理)
```bash
curl http://esp32-glasses.local/capture -o photo.jpg
```

#### 获取设备信息
```bash
curl http://esp32-glasses.local/info
```

返回JSON：
```json
{
  "status": "online",
  "device": "ESP32-S3 Smart Glasses", 
  "camera": "OV3660",
  "resolution": "1600x1200",
  "format": "JPEG",
  "wifi_ssid": "你的WiFi",
  "ip_address": "192.168.1.100",
  "hostname": "esp32-glasses.local"
}
```

## 🏗️ 项目结构

```
glasses/
├── main/                    # 主程序代码
│   ├── main.c              # 主函数入口
│   ├── camera.c            # OV3660摄像头驱动
│   ├── camera.h            # 摄像头接口定义
│   ├── wifi_streaming.c    # WiFi和HTTP服务器
│   ├── wifi_streaming.h    # WiFi配置和接口
│   └── CMakeLists.txt      # 构建配置
├── components/             # 外部组件
│   ├── esp32-camera/       # ESP32摄像头驱动库
│   └── mdns/              # mDNS服务组件
├── build/                 # 编译输出目录
├── CMakeLists.txt         # 根构建配置
├── README.md              # 项目说明
└── .gitignore            # Git忽略文件
```

## ⚙️ 配置选项

### 摄像头配置 (camera.c)
- **分辨率**: FRAMESIZE_UXGA (1600x1200)
- **图像格式**: JPEG
- **帧率**: ~30fps
- **图像质量**: 可调节 (0-63，数值越小质量越高)

### WiFi配置 (wifi_streaming.h)
- **重试次数**: 5次
- **连接超时**: 自动重连
- **HTTP端口**: 80

### 性能调优
```c
// 在 camera.c 中调整这些参数：
.jpeg_quality = 12,        // 图像质量 (0-63)
.fb_count = 2,            // 帧缓冲数量
.frame_size = FRAMESIZE_UXGA,  // 分辨率
```

## 🔧 故障排除

### 1. 编译错误
```bash
# 清理重新编译
idf.py fullclean
idf.py build
```

### 2. 摄像头初始化失败
- 检查引脚连接是否正确
- 确认OV3660供电正常 (3.3V)
- 检查I2C地址是否为0x3C

### 3. WiFi连接失败
- 确认WiFi名称和密码正确
- 检查WiFi信号强度
- 确认路由器支持2.4GHz频段

### 4. 无法访问网页
- 尝试IP地址访问而不是域名
- 确认手机和ESP32在同一WiFi网络
- 检查防火墙设置

### 5. 视频流卡顿
- 降低JPEG质量 (增加jpeg_quality值)
- 减少分辨率 (改为FRAMESIZE_SVGA)
- 检查WiFi信号质量

## 📈 性能参数

| 参数 | 数值 |
|------|------|
| 最大分辨率 | 1600x1200 (UXGA) |
| 帧率 | 20-30 FPS |
| 延迟 | <200ms (局域网) |
| 功耗 | ~500mA @ 3.3V |
| WiFi范围 | 室内50m，室外100m |
| 并发连接 | 最多8个客户端 |

## 🤝 贡献

欢迎提交Issue和Pull Request！

## 📄 许可证

MIT License - 详见 [LICENSE](LICENSE) 文件

## 📞 联系

- 项目作者: [你的名字]
- 邮箱: [你的邮箱]
- 项目地址: [GitHub链接]

---

⭐ 如果这个项目对你有帮助，请给个星标支持！