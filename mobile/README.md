# 📱 AI智能眼镜手机端 App

本项目为“AI智能眼镜”配套手机端应用，支持 Android 系统。用于连接和控制眼镜端设备，实现视频流查看、识图模式、导航模式等功能。

## 功能简介

- **识图模式**：辅助视障用户进行物体识别（预留功能，待接入AI识别逻辑）。
- **视频模式**：通过 WebView 实时访问眼镜端视频流（`http://esp32-glasses.local/stream`），支持自动重连与错误提示。
- **导航模式**：一键跳转高德地图步行导航（示例目的地：广州塔）。

## 主要界面

- 主界面包含三个按钮：识图模式、视频模式、导航模式。
- 视频模式页面支持设备自动检测、视频流展示、连接失败重试。

## 连接说明

1. **确保眼镜端已连接同一WiFi，并已启动 HTTP 服务。**
2. 手机连接同一WiFi，点击“视频模式”即可访问实时视频流。
3. 如无法访问，请检查网络、mDNS解析或直接用IP地址替换 `esp32-glasses.local`。

## 关键代码结构

- [`MainActivity`](app/src/main/java/com/example/myapplication/MainActivity.java)：主界面与模式切换逻辑
- [`VideoActivity`](app/src/main/java/com/example/myapplication/VideoActivity.java)：视频流访问与错误处理
- 布局文件：[activity_main.xml](app/src/main/res/layout/activity_main.xml)、[activity_video.xml](app/src/main/res/layout/activity_video.xml)
- 权限与配置：[AndroidManifest.xml](app/src/main/AndroidManifest.xml)

## 依赖与权限

- 需开启 `INTERNET` 和 `ACCESS_NETWORK_STATE` 权限
- 高德地图导航需配置 API Key

## 目录结构



## 常见问题

- 视频流无法访问：请确认眼镜端与手机在同一局域网，且眼镜端服务已启动。
- 域名无法解析：可尝试直接输入设备IP地址。
- 导航跳转异常：请确保已安装高德地图App。

---

如需更多功能或定制，请参考 [glasses/README.md](../glasses/README.md)。