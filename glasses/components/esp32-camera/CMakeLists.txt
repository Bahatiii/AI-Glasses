# get IDF version for comparison
set(idf_version "${IDF_VERSION_MAJOR}.${IDF_VERSION_MINOR}")

# 初始化源文件列表和头文件目录
set(srcs)
set(priv_include_dirs)
set(include_dirs driver/include)

set(COMPONENT_REQUIRES driver)

# set driver sources only for supported platforms
if(IDF_TARGET STREQUAL "esp32" OR IDF_TARGET STREQUAL "esp32s2" OR IDF_TARGET STREQUAL "esp32s3")
  # 添加核心驱动文件
  list(APPEND srcs
    driver/esp_camera.c
    driver/cam_hal.c
    driver/sensor.c
    )
  
  # 只添加存在的传感器文件
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/sensors/ov3660.c")
    list(APPEND srcs sensors/ov3660.c)
  endif()

  # 添加存在的私有头文件目录
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/driver/private_include")
    list(APPEND priv_include_dirs driver/private_include)
  endif()
  
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/sensors/private_include")
    list(APPEND priv_include_dirs sensors/private_include)
  endif()
  
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/target/private_include")
    list(APPEND priv_include_dirs target/private_include)
  endif()

  # ESP32-S3平台特定文件 (你使用的平台)
  if(IDF_TARGET STREQUAL "esp32s3")
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/target/esp32s3/ll_cam.c")
      list(APPEND srcs target/esp32s3/ll_cam.c)
    endif()
  endif()

  set(priv_requires freertos nvs_flash)

  set(min_version_for_esp_timer "4.2")
  if (idf_version VERSION_GREATER_EQUAL min_version_for_esp_timer)
    list(APPEND priv_requires esp_timer)
  endif()

  # include the SCCB I2C driver
  if (idf_version VERSION_GREATER_EQUAL "5.4" AND NOT CONFIG_SCCB_HARDWARE_I2C_DRIVER_LEGACY)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/driver/sccb-ng.c")
      list(APPEND srcs driver/sccb-ng.c)
    endif()
  else()
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/driver/sccb.c")
      list(APPEND srcs driver/sccb.c)
    endif()
  endif()

endif()

idf_component_register(
  SRCS ${srcs}
  INCLUDE_DIRS ${include_dirs}
  PRIV_INCLUDE_DIRS ${priv_include_dirs}
  REQUIRES driver  # due to include of driver/gpio.h in esp_camera.h
  PRIV_REQUIRES ${priv_requires}
)
