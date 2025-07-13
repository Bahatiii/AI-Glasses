#ifndef CAMERA_H
#define CAMERA_H

#include "esp_camera.h"
#include "esp_err.h"
#include "sensor.h"          // ← 添加这行，包含 OV3660_PID 的定义

// 函数声明
esp_err_t init_ov3660_camera(void);
void test_camera_capture(void);

#endif // CAMERA_H