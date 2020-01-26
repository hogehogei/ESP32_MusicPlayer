
#include "BluetoothAudio.hpp"
#include "BTAvProtoWorker.hpp"
#include "Log.hpp"

// esp32 headers
#include "esp_system.h"
#include "esp_log.h"

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
    : m_RingBuffer( sk_AudioBufferSize )
{}

BluetoothAudio::~BluetoothAudio()
{}

void BluetoothAudio::Initialize()
{
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
	if (esp_bt_controller_init(&bt_cfg) != ESP_OK) {
		ESP_LOGE(LogTagName::sk_BT_AV, "%s initialize controller failed\n", __func__);
		return;
	}

	if (esp_bt_controller_enable(ESP_BT_MODE_BTDM) != ESP_OK) {
		ESP_LOGE(LogTagName::sk_BT_AV, "%s enable controller failed\n", __func__);
		return;
	}

	if (esp_bluedroid_init() != ESP_OK) {
		ESP_LOGE(LogTagName::sk_BT_AV, "%s initialize bluedroid failed\n", __func__);
		return;
	}

	if (esp_bluedroid_enable() != ESP_OK) {
		ESP_LOGE(LogTagName::sk_BT_AV, "%s enable bluedroid failed\n", __func__);
		return;
	}

	/* create application task */
    startBT_Task();

	/* Bluetooth device name, connection mode and profile set up */
    startBT_AVProtocols();
}

 BluetoothAudio* BluetoothAudio::Instance()
 {
     static BluetoothAudio s_BT_Instance;
     return &s_BT_Instance;
 }

uint32_t BluetoothAudio::RemainDataCount() const
{
    return m_RingBuffer.RemainDataCount();
}

uint32_t BluetoothAudio::ReadA2D_Data( uint8_t* dst, uint32_t len )
{
    return m_RingBuffer.Recv( dst, len );
}

void BluetoothAudio::startBT_Task()
{
    m_BT_AppTaskQueue = xQueueCreate( sk_AppTaskQueueSize, sizeof(I_BTAppEventWorker*) );
    xTaskCreate( BT_AppTask, "BT_AppTask", sk_AppTaskStackSize, NULL, sk_AppTaskPriority, &m_BT_AppTaskHandle );
}

void BluetoothAudio::startBT_AVProtocols()
{
	ESP_LOGD(LogTagName::sk_BT_AV, "%s evt", __func__ );

	/* set up device name */
	esp_bt_dev_set_device_name( sk_DeviceName );

	/* initialize A2DP sink */
	esp_a2d_register_callback( BT_A2D_Callback );
	esp_a2d_sink_register_data_callback( BT_A2D_DataCallback );
	esp_a2d_sink_init();

	/* initialize AVRCP target */
    assert (esp_avrc_tg_init() == ESP_OK);
    esp_avrc_tg_register_callback( BT_AVRC_TargetCallback );

	/* set discoverable and connectable mode, wait to be connected */
	esp_bt_gap_set_scan_mode( ESP_BT_CONNECTABLE, ESP_BT_NON_DISCOVERABLE );
}

void BluetoothAudio::BT_AppTask( void* param )
{
    I_BTAppEventWorker* worker = nullptr;
    BluetoothAudio* instance = BluetoothAudio::Instance();
    for (;;) {
        if( xQueueReceive( instance->m_BT_AppTaskQueue, &worker, (portTickType)portMAX_DELAY ) == pdTRUE ){
            if( worker ){
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

    BluetoothAudio* instance = BluetoothAudio::Instance();
    if ( xQueueSend( instance->m_BT_AppTaskQueue, worker, 10 / portTICK_RATE_MS) != pdTRUE ) {
        ESP_LOGE(LogTagName::sk_BT_APPCORE, "%s xQueue send failed", __func__);
        return false;
    }

    return true;
}

void BluetoothAudio::BT_A2D_Callback( esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param )
{
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
    BluetoothAudio* instance = BluetoothAudio::Instance();
    instance->m_RingBuffer.Send( data, len );
}

void BluetoothAudio::BT_AVRC_TargetCallback( esp_avrc_tg_cb_event_t event, esp_avrc_tg_cb_param_t *param )
{
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

