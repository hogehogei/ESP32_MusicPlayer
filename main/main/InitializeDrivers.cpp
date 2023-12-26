
#include "InitializeDrivers.hpp"
#include "../SDCard/FileSystem.hpp"
#include "../Audio/VS1053.hpp"

// esp-idf headers
#include "nvs.h"
#include "nvs_flash.h"

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"

extern "C"
{

void InitializeDrivers()
{
    // SDカードファイルシステム初期化
    printf( "SDC FileSystem initialize.\n" );
    FileSystem::Instance().Initialize();
    // VS1053 ドライバ初期化
    printf( "VS1053 driver initialize.\n" );
    VS1053_Drv::Instance().Initialize();

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    ESP_ERROR_CHECK( esp_bt_controller_mem_release(ESP_BT_MODE_BLE) );
}

}