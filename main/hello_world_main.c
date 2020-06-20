/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

//
// esp-idf headers
//
#include "esp_system.h"
#include "esp_spi_flash.h"

#include "MainTask.hpp"
#include "InputTask.hpp"
#include "InitializeDrivers.hpp"

//
// static functions
//
static void InvokeMainTask( void );

//
// static variables
//
static const int sk_MusicTaskStackSize = 1024 * 8;
static const int sk_AppTaskPriority    = configMAX_PRIORITIES - 3;
static xTaskHandle s_MusicTaskHandle;

static const int sk_InputTaskStackSize = 1024 * 1;
static const int sk_InputTaskPriority  = configMAX_PRIORITIES - 3;
static xTaskHandle s_InputTaskHandle;

void app_main()
{
    printf("Hello world!\n");

    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is ESP32 chip with %d CPU cores, WiFi%s%s, ",
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
    
    // ドライバ初期化
    InitializeDrivers();

    // メインループ
    InvokeMainTask();

    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}

static void InvokeMainTask( void )
{
    xTaskCreate( MusicTask, "MusicTask", sk_MusicTaskStackSize, NULL, sk_AppTaskPriority, &s_MusicTaskHandle );
    xTaskCreate( InputTask, "Input", sk_InputTaskStackSize, NULL, sk_InputTaskPriority, &s_InputTaskHandle );

    const int maintask_interval = 10;
    while( 1 ) {
        vTaskDelay( maintask_interval / portTICK_RATE_MS );
    }
}
