#include "wifi_streaming.h"
#include "camera.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_netif.h"
#include "mdns.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include <string.h>

static const char *TAG = "WIFI";

// WiFi事件组
static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static int s_retry_num = 0;
static httpd_handle_t stream_server = NULL;

// WiFi事件处理
static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < WIFI_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "重试连接 %d/%d", s_retry_num, WIFI_MAXIMUM_RETRY);
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "IP: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

// 启动mDNS
static void start_mdns(void)
{
    mdns_init();
    mdns_hostname_set("esp32-glasses");
    mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0);
    ESP_LOGI(TAG, "访问: http://esp32-glasses.local");
}

// WiFi初始化
esp_err_t wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
    
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "WiFi连接成功");
        start_mdns();  // 启动mDNS
        return ESP_OK;
    } else {
        ESP_LOGI(TAG, "WiFi连接失败");
        return ESP_FAIL;
    }
}

// 视频流处理
static esp_err_t stream_handler(httpd_req_t *req)
{
    camera_fb_t *fb = NULL;
    esp_err_t res = ESP_OK;
    char part_buf[64];

    res = httpd_resp_set_type(req, STREAM_CONTENT_TYPE);
    if (res != ESP_OK) return res;

    while (true) {
        fb = esp_camera_fb_get();
        if (!fb) {
            res = ESP_FAIL;
            break;
        }

        size_t hlen = snprintf(part_buf, 64, STREAM_PART, fb->len);
        res = httpd_resp_send_chunk(req, part_buf, hlen);
        if (res == ESP_OK) {
            res = httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len);
        }
        if (res == ESP_OK) {
            res = httpd_resp_send_chunk(req, STREAM_BOUNDARY, strlen(STREAM_BOUNDARY));
        }

        esp_camera_fb_return(fb);
        if (res != ESP_OK) break;
        vTaskDelay(30 / portTICK_PERIOD_MS);  // 控制帧率
    }
    return res;
}

// 主页处理 - 添加拍照功能
static esp_err_t index_handler(httpd_req_t *req)
{
    const char* html_page = 
    "<!DOCTYPE html>"
    "<html>"
    "<head>"
    "<title>ESP32-S3 智能眼镜</title>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    "<style>"
    "body { font-family: Arial; text-align: center; background: #222; color: white; margin: 0; padding: 20px; }"
    ".container { max-width: 800px; margin: 0 auto; }"
    ".video-container { margin: 20px 0; border: 2px solid #444; border-radius: 10px; overflow: hidden; }"
    "img { width: 100%; height: auto; display: block; }"
    ".button { "
    "  background: linear-gradient(45deg, #ff6b6b, #4ecdc4); "
    "  color: white; border: none; padding: 15px 30px; margin: 10px; "
    "  border-radius: 25px; font-size: 18px; cursor: pointer; "
    "  box-shadow: 0 4px 15px rgba(0,0,0,0.3); "
    "  transition: transform 0.2s; "
    "}"
    ".button:hover { transform: translateY(-2px); }"
    ".button:active { transform: translateY(0); }"
    ".status { margin: 20px; padding: 10px; background: #333; border-radius: 5px; }"
    ".info { font-size: 14px; color: #888; margin-top: 20px; }"
    "</style>"
    "</head>"
    "<body>"
    "<div class='container'>"
    "<h1>🤓 ESP32-S3 智能眼镜</h1>"
    
    "<div class='video-container'>"
    "<img id='stream' src='/stream' alt='实时视频流'>"
    "</div>"
    
    "<div>"
    "<button class='button' onclick='capturePhoto()'>📸 拍照下载</button>"
    "<button class='button' onclick='refreshStream()'>🔄 刷新视频</button>"
    "</div>"
    
    "<div class='status' id='status'>状态：正常运行</div>"
    
    "<div class='info'>"
    "<p>📱 访问地址：<strong>http://esp32-glasses.local</strong></p>"
    "<p>🎥 视频流：<a href='/stream' style='color:#4ecdc4'>/stream</a></p>"
    "<p>📸 拍照：<a href='/capture' style='color:#4ecdc4'>/capture</a></p>"
    "<p>ℹ️ 信息：<a href='/info' style='color:#4ecdc4'>/info</a></p>"
    "</div>"
    "</div>"
    
    "<script>"
    "let photoCount = 0;"
    
    // 拍照函数 - 直接下载
    "function capturePhoto() {"
    "  console.log('开始拍照...');"
    "  "
    "  // 更新状态"
    "  document.getElementById('status').innerHTML = '📸 正在拍照...';"
    "  document.getElementById('status').style.background = '#444';"
    "  "
    "  // 创建隐藏的下载链接"
    "  const downloadLink = document.createElement('a');"
    "  downloadLink.href = '/capture';"
    "  downloadLink.style.display = 'none';"
    "  document.body.appendChild(downloadLink);"
    "  "
    "  // 触发下载"
    "  downloadLink.click();"
    "  "
    "  // 清理"
    "  document.body.removeChild(downloadLink);"
    "  "
    "  // 更新计数和状态"
    "  photoCount++;"
    "  setTimeout(() => {"
    "    document.getElementById('status').innerHTML = `✅ 照片已下载 (第${photoCount}张)`;"
    "    document.getElementById('status').style.background = '#0a5d0a';"
    "  }, 1000);"
    "  "
    "  setTimeout(() => {"
    "    document.getElementById('status').innerHTML = '状态：正常运行';"
    "    document.getElementById('status').style.background = '#333';"
    "  }, 3000);"
    "}"
    
    // 刷新视频流
    "function refreshStream() {"
    "  console.log('刷新视频流...');"
    "  const img = document.getElementById('stream');"
    "  const currentSrc = img.src;"
    "  img.src = '';"
    "  setTimeout(() => {"
    "    img.src = currentSrc + '?t=' + new Date().getTime();"
    "  }, 100);"
    "  "
    "  document.getElementById('status').innerHTML = '🔄 视频流已刷新';"
    "  setTimeout(() => {"
    "    document.getElementById('status').innerHTML = '状态：正常运行';"
    "  }, 2000);"
    "}"
    
    // 自动刷新状态
    "setInterval(() => {"
    "  if (document.getElementById('status').innerHTML === '状态：正常运行') {"
    "    const now = new Date();"
    "    const timeStr = now.toLocaleTimeString();"
    "    // 这里可以添加更多状态信息"
    "  }"
    "}, 5000);"
    
    // 页面加载完成
    "console.log('ESP32智能眼镜控制面板加载完成');"
    "console.log('拍照将直接下载到设备');"
    "</script>"
    "</body>"
    "</html>";

    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, html_page, strlen(html_page));
}

// 单张图片获取处理
static esp_err_t capture_handler(httpd_req_t *req)
{
    camera_fb_t *fb = NULL;
    esp_err_t res = ESP_OK;
    static uint32_t photo_counter = 0;  // 静态计数器，每次调用自动递增
    ESP_LOGI(TAG, "📸 收到拍照请求");
    
    fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAG, "❌ 获取图片失败");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    
    // 使用计数器生成文件名（更简单可靠）
    char filename[48];
    photo_counter++;
    snprintf(filename, sizeof(filename), "ESP32_Glasses_%04lu.jpg", photo_counter);
    
    // 设置HTTP响应头，触发浏览器下载
    char content_disposition[80];
    snprintf(content_disposition, sizeof(content_disposition), 
             "attachment; filename=\"%s\"", filename);
    
    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Content-Disposition", content_disposition);  // ← 关键：触发下载
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
    
    // 发送图片数据
    res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
    
    ESP_LOGI(TAG, "📷 照片已发送下载: %s (%d bytes)", filename, fb->len);
    esp_camera_fb_return(fb);
    
    return res;
}

// 获取图片信息的接口
static esp_err_t info_handler(httpd_req_t *req)
{
    // 获取当前IP地址
    esp_netif_ip_info_t ip_info;
    esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), &ip_info);
    
    // 获取WiFi信息
    wifi_ap_record_t ap_info;
    esp_err_t wifi_ret = esp_wifi_sta_get_ap_info(&ap_info);
    
    // 创建JSON响应
    char json_response[512];
    snprintf(json_response, sizeof(json_response),
        "{"
        "\"status\":\"online\","
        "\"device\":\"ESP32-S3 Smart Glasses\","
        "\"camera\":\"OV3660\","
        "\"resolution\":\"1600x1200\","
        "\"format\":\"JPEG\","
        "\"wifi_ssid\":\"%s\","
        "\"ip_address\":\"" IPSTR "\","  // ← 直接使用IPSTR宏
        "\"hostname\":\"esp32-glasses.local\","
        "\"endpoints\":{"
        "\"stream\":\"/stream\","
        "\"capture\":\"/capture\","
        "\"info\":\"/info\""
        "}"
        "}",
        (wifi_ret == ESP_OK) ? (char*)ap_info.ssid : "Unknown",
        IP2STR(&ip_info.ip));  // ← 直接使用，不在三元运算符中
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, json_response, strlen(json_response));
}

// 启动HTTP服务器
esp_err_t start_streaming_server(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    config.max_uri_handlers = 8;  // 增加处理器数量
    
    if (httpd_start(&stream_server, &config) == ESP_OK) {
        httpd_uri_t index_uri = {.uri = "/", .method = HTTP_GET, .handler = index_handler};
        httpd_uri_t stream_uri = {.uri = "/stream", .method = HTTP_GET, .handler = stream_handler};
        httpd_uri_t capture_uri = {.uri = "/capture", .method = HTTP_GET, .handler = capture_handler};
        httpd_uri_t info_uri = {.uri = "/info", .method = HTTP_GET, .handler = info_handler};
        
        httpd_register_uri_handler(stream_server, &index_uri);
        httpd_register_uri_handler(stream_server, &stream_uri);
        httpd_register_uri_handler(stream_server, &capture_uri);
        httpd_register_uri_handler(stream_server, &info_uri);
        
        ESP_LOGI(TAG, "HTTP服务器启动成功");
        ESP_LOGI(TAG, "📱 主页: http://esp32-glasses.local");
        ESP_LOGI(TAG, "🎥 视频流: http://esp32-glasses.local/stream");
        ESP_LOGI(TAG, "📸 拍照: http://esp32-glasses.local/capture");
        ESP_LOGI(TAG, "ℹ️  信息: http://esp32-glasses.local/info");
        return ESP_OK;
    }
    return ESP_FAIL;
}

// 停止服务器
void stop_streaming_server(void)
{
    if (stream_server) {
        httpd_stop(stream_server);
        stream_server = NULL;
    }
    mdns_free();
}

// 获取WiFi状态
esp_err_t get_wifi_status(void)
{
    wifi_ap_record_t ap_info;
    if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
        // 获取IP地址
        esp_netif_ip_info_t ip_info;
        esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), &ip_info);
        
        ESP_LOGI(TAG, "WiFi: %s", ap_info.ssid);
        ESP_LOGI(TAG, "IP: " IPSTR, IP2STR(&ip_info.ip));
        ESP_LOGI(TAG, "域名: http://esp32-glasses.local");  // ← 添加域名提示
        return ESP_OK;
    }
    return ESP_FAIL;
}