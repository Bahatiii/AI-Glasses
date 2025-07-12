#ifndef CAMERA_H
#define CAMERA_H

#include "esp_camera.h"
#include "esp_err.h"

// 函数声明
esp_err_t init_ov3660_camera(void);
void test_camera_capture(void);

#endif // CAMERA_H