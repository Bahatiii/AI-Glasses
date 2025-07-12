# AI Glasses - ESP-IDF 固件

这是 AI 智能眼镜项目的 ESP-IDF 固件部分，负责硬件控制、传感器数据采集和与移动端的通信。

## 项目概述

本固件基于 ESP-IDF 框架开发，为 AI 智能眼镜提供：
- 硬件底层控制
- 传感器数据处理
- 无线通信功能
- 电源管理

## 开发环境要求

### 硬件要求
- ESP32 系列开发板
- USB 数据线
- 相关传感器模块

### 软件要求
- ESP-IDF v4.4 或更高版本
- Git

## 快速开始

### 1. 克隆项目
```bash
git clone <your-repo-url>
cd AI-Glasses/glasses
```

### 2. 设置 ESP-IDF 环境
```bash
# 安装 ESP-IDF（如果还未安装）
get_idf
```

### 3. 配置项目
```bash
# 配置目标芯片（如 ESP32）
idf.py set-target esp32

# 打开配置菜单
idf.py menuconfig
```

### 4. 编译和烧录
```bash
# 编译项目
idf.py build

# 烧录到设备
idf.py -p COM3 flash monitor
```

## 使用 Dev Container（推荐）

项目包含 `.devcontainer` 配置，支持在 VS Code 中使用容器化开发环境：

1. 安装 VS Code 和 Dev Containers 扩展
2. 打开项目文件夹
3. 按 `F1` 输入 "Dev Containers: Reopen in Container"
4. 等待容器构建完成

容器环境已预配置好 ESP-IDF 和所有依赖工具。

## 项目结构

```
glasses/
├── main/                   # 主应用代码
├── components/             # 自定义组件
├── build/                  # 构建输出（已忽略）
├── .devcontainer/          # Dev Container 配置
├── .vscode/                # VS Code 配置 （已忽略）
├── CMakeLists.txt          # CMake 配置文件
├── sdkconfig              # ESP-IDF 配置（已忽略）
├── requirements.txt        # Python 依赖
└── README.md              # 本文件
```

## 主要功能模块

### 传感器管理
- 摄像头采集数据

### 通信模块
- WiFi 连接管理
- 蓝牙通信
- 与移动端数据同步


### 电源管理
- 低功耗模式
- 电池电量监测

## 配置说明

### WiFi 配置
在 `idf.py menuconfig` 中配置：
```
Example Configuration → WiFi SSID
Example Configuration → WiFi Password
```

### 串口配置
默认波特率：115200
默认端口：根据系统自动检测

## 调试和监控

### 查看日志
```bash
idf.py monitor
```

### 清理构建
```bash
idf.py clean
```

### 完全重新构建
```bash
idf.py fullclean
idf.py build
```

## 故障排除

### 常见问题

1. **编译错误**
   - 确保 ESP-IDF 版本兼容
   - 检查 Python 环境配置

2. **烧录失败**
   - 检查 USB 连接
   - 确认端口号正确
   - 尝试按住 BOOT 按钮重新烧录

3. **设备无法连接**
   - 检查 WiFi 配置
   - 查看串口日志排查问题

### 获取帮助
- 查看 ESP-IDF 官方文档
- 检查项目 Issues
- 联系开发团队

## 开发指南

### 添加新组件
1. 在 `components/` 目录创建新文件夹
2. 添加 `CMakeLists.txt` 和源代码
3. 在主 CMakeLists.txt 中注册组件

### 修改配置
1. 运行 `idf.py menuconfig`
2. 修改相关配置项
3. 保存并重新编译

## 贡献指南

1. Fork 项目
2. 创建功能分支
3. 提交更改
4. 发起 Pull Request

## 许可证

[在此添加许可证信息]

## 更新日志

### v1.0.0 (开发中)
- 初始项目架构
- 基础传感器支持
- WiFi 通信功能