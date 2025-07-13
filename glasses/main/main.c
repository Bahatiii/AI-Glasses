#include <stdio.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "camera.h"
#include "wifi_streaming.h"

static const char *TAG = "MAIN";

void app_main(void)
{
    ESP_LOGI(TAG, "🚀 启动ESP32-S3智能眼镜项目");

    // 初始化NVS存储
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "✅ NVS初始化完成");

    // 初始化摄像头
    if (init_ov3660_camera() == ESP_OK) {
        ESP_LOGI(TAG, "🎉 摄像头初始化成功");
        
        // 等待摄像头稳定
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        
        // 快速测试拍照功能
        //ESP_LOGI(TAG, "📸 开始快速拍照测试");
        //test_camera_capture();
        
        // 初始化WiFi
        ESP_LOGI(TAG, "📶 开始WiFi连接...");
        if (wifi_init_sta() == ESP_OK) {
            ESP_LOGI(TAG, "✅ WiFi连接成功");
            
            // 显示连接信息
            get_wifi_status();
            
            // 启动HTTP视频流服务器
            if (start_streaming_server() == ESP_OK) {
                ESP_LOGI(TAG, "🎥 视频流服务器启动成功");
                
                // 持续运行
                while (1) {
                    vTaskDelay(10000 / portTICK_PERIOD_MS);
                    get_wifi_status(); // 每10秒显示一次状态
                }
            } else {
                ESP_LOGE(TAG, "❌ 视频流服务器启动失败");
            }
        } else {
            ESP_LOGE(TAG, "❌ WiFi连接失败");
        }
        
    } else {
        ESP_LOGE(TAG, "💥 摄像头初始化失败");
    }
    
    ESP_LOGI(TAG, "程序结束");
}