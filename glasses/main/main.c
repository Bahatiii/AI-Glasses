#include <stdio.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "camera.h"

static const char *TAG = "MAIN";

void app_main(void)
{
    ESP_LOGI(TAG, "🚀 启动OV3660摄像头测试程序");

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
        ESP_LOGI(TAG, "🎉 摄像头初始化成功，开始测试拍照");
        
        // 等待摄像头稳定
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        
        // 测试拍照功能
        test_camera_capture();
        
    } else {
        ESP_LOGE(TAG, "💥 摄像头初始化失败");
    }

    ESP_LOGI(TAG, "📋 程序运行完成");
}