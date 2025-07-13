#ifndef WIFI_STREAMING_H
#define WIFI_STREAMING_H

#include "esp_err.h"
#include "esp_http_server.h"


// WiFi配置 - 请修改为你的WiFi信息
#define WIFI_SSID      "你的WiFi名称"     // ← 修改这里
#define WIFI_PASSWORD  "你的WiFi密码"     // ← 修改这里
#define WIFI_MAXIMUM_RETRY  5

// HTTP服务器配置 - 优化的边界字符串
#define STREAM_CONTENT_TYPE "multipart/x-mixed-replace;boundary=frame"
#define STREAM_BOUNDARY "\r\n--frame\r\n"
#define STREAM_PART "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n"

// 函数声明
esp_err_t wifi_init_sta(void);
esp_err_t start_streaming_server(void);
void stop_streaming_server(void);
esp_err_t get_wifi_status(void);

#endif // WIFI_STREAMING_H