#include "camera.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "OV3660_CAMERA";

// OV3660摄像头引脚定义 (ESP32-S3 建议引脚配置)
#define CAM_PIN_PWDN    -1    // 电源控制引脚，不使用设为-1
#define CAM_PIN_RESET   -1    // 复位引脚，不使用设为-1  
#define CAM_PIN_XCLK    15    // 主时钟引脚 (MCLK)
#define CAM_PIN_SIOD    4     // SDA引脚 (I2C数据线)
#define CAM_PIN_SIOC    5     // SCL引脚 (I2C时钟线)

// 数据引脚 (8位并行数据)
#define CAM_PIN_D7      11    // 数据引脚D7 (MSB)
#define CAM_PIN_D6      9     // 数据引脚D6
#define CAM_PIN_D5      8     // 数据引脚D5
#define CAM_PIN_D4      10    // 数据引脚D4
#define CAM_PIN_D3      12    // 数据引脚D3
#define CAM_PIN_D2      18    // 数据引脚D2
#define CAM_PIN_D1      17    // 数据引脚D1
#define CAM_PIN_D0      16    // 数据引脚D0 (LSB)

// 同步引脚
#define CAM_PIN_VSYNC   6     // 垂直同步引脚
#define CAM_PIN_HREF    7     // 水平同步引脚  
#define CAM_PIN_PCLK    13    // 像素时钟引脚

// 摄像头配置结构体
static camera_config_t camera_config = {
    .pin_pwdn = CAM_PIN_PWDN,
    .pin_reset = CAM_PIN_RESET,
    .pin_xclk = CAM_PIN_XCLK,
    .pin_sccb_sda = CAM_PIN_SIOD,
    .pin_sccb_scl = CAM_PIN_SIOC,

    .pin_d7 = CAM_PIN_D7,
    .pin_d6 = CAM_PIN_D6,
    .pin_d5 = CAM_PIN_D5,
    .pin_d4 = CAM_PIN_D4,
    .pin_d3 = CAM_PIN_D3,
    .pin_d2 = CAM_PIN_D2,
    .pin_d1 = CAM_PIN_D1,
    .pin_d0 = CAM_PIN_D0,
    .pin_vsync = CAM_PIN_VSYNC,
    .pin_href = CAM_PIN_HREF,
    .pin_pclk = CAM_PIN_PCLK,

    // 时钟配置
    .xclk_freq_hz = 20000000,           // 20MHz主时钟
    .ledc_timer = LEDC_TIMER_0,         // LEDC定时器
    .ledc_channel = LEDC_CHANNEL_0,     // LEDC通道

    // 图像配置
    .pixel_format = PIXFORMAT_JPEG,     // JPEG格式输出
    .frame_size = FRAMESIZE_SVGA,       // 1600x1200分辨率
    .jpeg_quality = 10,                 // JPEG质量 (0-63，数字越小质量越高)
    .fb_count = 2,                      // 双帧缓冲
    .fb_location = CAMERA_FB_IN_PSRAM,  // 帧缓冲存储在PSRAM中
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY // 当缓冲区为空时抓取新帧
};

// 初始化摄像头
esp_err_t init_ov3660_camera(void)
{
    ESP_LOGI(TAG, "开始初始化OV3660摄像头...");
    
    // 初始化摄像头
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "摄像头初始化失败: 0x%x", err);
        return err;
    }

    // 获取传感器句柄
    sensor_t *s = esp_camera_sensor_get();
    if (s == NULL) {
        ESP_LOGE(TAG, "获取摄像头传感器失败");
        return ESP_FAIL;
    }

    // 验证是否为OV3660传感器
    if (s->id.PID == OV3660_PID) {
        ESP_LOGI(TAG, "✅ OV3660传感器检测成功!");
        ESP_LOGI(TAG, "传感器ID: 0x%04X", s->id.PID);
        
        // 配置OV3660特定参数
        ESP_LOGI(TAG, "配置OV3660参数...");
        
        // 基本图像质量设置
        s->set_brightness(s, 0);         // 亮度: -2 到 2
        s->set_contrast(s, 0);           // 对比度: -2 到 2  
        s->set_saturation(s, 0);         // 饱和度: -2 到 2
        s->set_sharpness(s, 0);          // 锐度: -2 到 2
        s->set_denoise(s, 0);            // 去噪: 0 到 8
        
        // JPEG质量设置
        s->set_quality(s, 10);           // JPEG质量: 0-63 (越小质量越高)
        s->set_gainceiling(s, GAINCEILING_2X); // 增益上限
        
        // 自动控制设置
        s->set_gain_ctrl(s, 1);          // 启用自动增益控制
        s->set_exposure_ctrl(s, 1);      // 启用自动曝光控制
        s->set_whitebal(s, 1);           // 启用自动白平衡
        s->set_awb_gain(s, 1);           // 启用AWB增益
        s->set_wb_mode(s, 0);            // 白平衡模式: 0=自动
        
        // 其他设置
        s->set_hmirror(s, 0);            // 水平镜像: 0=禁用, 1=启用
        s->set_vflip(s, 0);              // 垂直翻转: 0=禁用, 1=启用
        s->set_colorbar(s, 0);           // 彩条测试模式: 0=禁用
        s->set_dcw(s, 1);                // 启用DCW (数字剪裁窗口)
        s->set_bpc(s, 0);                // 黑像素消除
        s->set_wpc(s, 1);                // 白像素消除
        s->set_raw_gma(s, 1);            // Gamma校正
        s->set_lenc(s, 1);               // 镜头校正
        
        ESP_LOGI(TAG, "✅ OV3660参数配置完成");
        
    } else {
        ESP_LOGW(TAG, "⚠️  检测到的传感器ID: 0x%04X (期望OV3660: 0x%04X)", 
                 s->id.PID, OV3660_PID);
        ESP_LOGW(TAG, "传感器可能不是OV3660，但继续运行...");
    }

    ESP_LOGI(TAG, "✅ 摄像头初始化完成");
    return ESP_OK;
}

// 测试拍照功能
void test_camera_capture(void)
{
    ESP_LOGI(TAG, "开始测试摄像头拍照...");
    
    for (int i = 0; i < 5; i++) {
        // 获取一帧图像
        camera_fb_t *fb = esp_camera_fb_get();
        if (fb) {
            ESP_LOGI(TAG, "📸 第%d张照片拍摄成功!", i + 1);
            ESP_LOGI(TAG, "   图像大小: %d bytes", fb->len);
            ESP_LOGI(TAG, "   分辨率: %dx%d", fb->width, fb->height);
            ESP_LOGI(TAG, "   格式: %s", 
                    fb->format == PIXFORMAT_JPEG ? "JPEG" : "其他");
            
            // 释放帧缓冲
            esp_camera_fb_return(fb);
        } else {
            ESP_LOGE(TAG, "❌ 第%d张照片拍摄失败!", i + 1);
        }
        
        vTaskDelay(2000 / portTICK_PERIOD_MS); // 等待2秒
    }
}