
#include "BluetoothAudio.hpp"
#include "BTAvProtoWorker.hpp"
#include "Log.hpp"

// esp32 headers
#include "esp_system.h"
#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"

//
// const variables
//


BluetoothAudio::BluetoothAudio()
    : m_IsInitialized( false ),
      m_RingBuffer( sk_AudioBufferSize )
{}

BluetoothAudio::~BluetoothAudio()
{}

void BluetoothAudio::Initialize()
{
    if( m_IsInitialized ){
        return;
    }

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK( esp_bt_controller_init(&bt_cfg) );

	ESP_ERROR_CHECK( esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT) );
	ESP_ERROR_CHECK( esp_bluedroid_init() );
	ESP_ERROR_CHECK( esp_bluedroid_enable() );

	/* create application task */
    startBT_Task();

	/* Bluetooth device name, connection mode and profile set up */
    startBT_AVProtocols();

    m_IsInitialized = true;
}

bool BluetoothAudio::IsInitialized() const
{
    return m_IsInitialized;
}

void BluetoothAudio::DeInitialize()
{
    ESP_LOGD(LogTagName::sk_BT_AV, "%s deinitialize Bluetooth AV Protocols.", __func__);

    ESP_ERROR_CHECK( esp_avrc_tg_deinit() );
    ESP_ERROR_CHECK( esp_a2d_sink_deinit() );
    ESP_ERROR_CHECK( esp_bluedroid_disable() );
    ESP_ERROR_CHECK( esp_bluedroid_deinit() );
    ESP_ERROR_CHECK( esp_bt_controller_disable() );
    ESP_ERROR_CHECK( esp_bt_controller_deinit() );
}

 BluetoothAudio& BluetoothAudio::Instance()
 {
     static BluetoothAudio s_BT_Instance;
     return s_BT_Instance;
 }

uint32_t BluetoothAudio::RemainDataCount() const
{
    return m_RingBuffer.RemainDataCount();
}

uint32_t BluetoothAudio::ReadA2D_Data( uint8_t* dst, uint32_t len )
{
    return m_RingBuffer.Recv( dst, len );
}

void BluetoothAudio::SetConfiguration( const StreamInfo& info )
{
    m_SBCConfiguraiton = info;
}

StreamInfo BluetoothAudio::GetConfiguration() const
{
    return m_SBCConfiguraiton;
}

void BluetoothAudio::startBT_Task()
{
    m_BT_AppTaskQueue = xQueueCreate( sk_AppTaskQueueSize, sizeof(I_BTAppEventWorker*) );
    xTaskCreate( BT_AppTask, "BT_AppTask", sk_AppTaskStackSize, NULL, sk_AppTaskPriority, &m_BT_AppTaskHandle );
}

void BluetoothAudio::startBT_AVProtocols()
{
    ESP_LOGD(LogTagName::sk_BT_AV, "%s setup Bluetooth AV Protocols.", __func__);

	/* set up device name */
	esp_bt_dev_set_device_name( sk_DeviceName );

	/* initialize A2DP sink */
	ESP_ERROR_CHECK( esp_a2d_register_callback( BT_A2D_Callback ) );
	ESP_ERROR_CHECK( esp_a2d_sink_register_data_callback( BT_A2D_DataCallback )  );
	ESP_ERROR_CHECK( esp_a2d_sink_init() );

	/* initialize AVRCP target */
    ESP_ERROR_CHECK( esp_avrc_tg_init() );
    ESP_ERROR_CHECK( esp_avrc_tg_register_callback( BT_AVRC_TargetCallback ) );
        
    esp_avrc_rn_evt_cap_mask_t evt_set = {};
    esp_avrc_rn_evt_bit_mask_operation(ESP_AVRC_BIT_MASK_OP_SET, &evt_set, ESP_AVRC_RN_VOLUME_CHANGE);
    ESP_ERROR_CHECK( esp_avrc_tg_set_rn_evt_cap(&evt_set) );

	/* set discoverable and connectable mode, wait to be connected */
	ESP_ERROR_CHECK( esp_bt_gap_set_scan_mode( ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE ) );
}

void BluetoothAudio::BT_AppTask( void* param )
{
    I_BTAppEventWorker* worker = nullptr;
    BluetoothAudio& instance = BluetoothAudio::Instance();
    for (;;) {
        if( xQueueReceive( instance.m_BT_AppTaskQueue, &worker, (portTickType)portMAX_DELAY ) == pdTRUE ){
            if( worker ){
                ESP_LOGI(LogTagName::sk_BT_AV, "(%s) call worker function addr->(%X)", __func__, reinterpret_cast<unsigned>(worker));
                worker->Invoke();
            }

            delete worker;
        }
    }
}

bool BluetoothAudio::BT_QueueAppWorker( I_BTAppEventWorker* worker )
{
    if( !worker ){
        return false;
    }

    BluetoothAudio& instance = BluetoothAudio::Instance();
    if ( xQueueSend( instance.m_BT_AppTaskQueue, &worker, 10 / portTICK_RATE_MS) != pdTRUE ) {
        ESP_LOGE(LogTagName::sk_BT_APPCORE, "%s xQueue send failed", __func__);
        return false;
    }

    return true;
}

void BluetoothAudio::BT_A2D_Callback( esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param )
{
    ESP_LOGD(LogTagName::sk_BT_AV, "%s A2DP callback.", __func__);

    switch (event) {
    case ESP_A2D_CONNECTION_STATE_EVT:
    case ESP_A2D_AUDIO_STATE_EVT:
    case ESP_A2D_AUDIO_CFG_EVT:
    {
        BT_A2DP_Worker* bt2_eventworker = new BT_A2DP_Worker( event, param );
        BT_QueueAppWorker( bt2_eventworker );
        break;
    }
    default:
        ESP_LOGE(LogTagName::sk_BT_AV, "a2dp invalid cb event: %d", event);
        break;
    }
}

void BluetoothAudio::BT_A2D_DataCallback( const uint8_t *data, uint32_t len )
{
    //ESP_LOGD(LogTagName::sk_BT_AV, "%s A2DP data callback.", __func__);
    
    BluetoothAudio& instance = BluetoothAudio::Instance();
    instance.m_RingBuffer.Send( data, len );
}

void BluetoothAudio::BT_AVRC_TargetCallback( esp_avrc_tg_cb_event_t event, esp_avrc_tg_cb_param_t *param )
{
    ESP_LOGD(LogTagName::sk_BT_AV, "%s AVRCP callback.", __func__);

    switch (event) {
    case ESP_AVRC_TG_CONNECTION_STATE_EVT:
    case ESP_AVRC_TG_REMOTE_FEATURES_EVT:
    case ESP_AVRC_TG_PASSTHROUGH_CMD_EVT:
    case ESP_AVRC_TG_SET_ABSOLUTE_VOLUME_CMD_EVT:
    case ESP_AVRC_TG_REGISTER_NOTIFICATION_EVT:
    {
        BT_AVRCPTarget_Worker* avrc_eventworker = new BT_AVRCPTarget_Worker( event, param );
        BT_QueueAppWorker( avrc_eventworker );
        break;
    }
    default:
        break;
    }
}

