
#ifndef    BLUETOOTH_AUDIO_HPP
#define    BLUETOOTH_AUDIO_HPP

#include <cstdint>

// FreeRTOS headers
#include <freertos/FreeRTOS.h>
#include <freertos/ringbuf.h>
#include <freertos/task.h>

// esp32 bluetooth headers
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"

#include "I_BTAppEventWorker.hpp"
#include "RingBuffer.hpp"

class BluetoothAudio
{
public:

    BluetoothAudio( const BluetoothAudio& ) = delete;
    BluetoothAudio& operator=( const BluetoothAudio& ) = delete;

    void Initialize();
    static BluetoothAudio* Instance();

    uint32_t RemainDataCount() const;
    uint32_t ReadA2D_Data( uint8_t* dst, uint32_t len );
    
private:

    static constexpr char sk_DeviceName[]       = "ESP_SPEAKER";
    static constexpr int sk_AppTaskQueueSize    = 10;
    static constexpr int sk_AppTaskStackSize    = 3072;
    static constexpr int sk_AppTaskPriority     = configMAX_PRIORITIES - 3;
    static constexpr int sk_AudioBufferSize     = 1024 * 8;         // 8KB

    BluetoothAudio();
    ~BluetoothAudio() noexcept;
    
    // Support initializing
    void startBT_Task();
    void startBT_AVProtocols();

    // For Application Task
    static void BT_AppTask( void* param );
    static bool BT_QueueAppWorker( I_BTAppEventWorker* worker );

    static void BT_A2D_Callback( esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param );
    static void BT_A2D_DataCallback( const uint8_t *data, uint32_t len );
    static void BT_AVRC_TargetCallback( esp_avrc_tg_cb_event_t event, esp_avrc_tg_cb_param_t *param );

    xTaskHandle  m_BT_AppTaskHandle;
    xQueueHandle m_BT_AppTaskQueue;
    RingBuffer   m_RingBuffer;
};

#endif    // BLUETOOTH_AUDIO_HPP