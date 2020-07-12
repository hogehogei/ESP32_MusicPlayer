
#ifndef    BT_AV_PROTO_WORKER_HPP_INCLUDED
#define    BT_AV_PROTO_WORKER_HPP_INCLUDED

#include "I_BTAppEventWorker.hpp"

// esp32 bluetooth headers
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"


class BT_A2DP_Worker : public I_BTAppEventWorker
{
public:

    BT_A2DP_Worker( esp_a2d_cb_event_t event, esp_a2d_cb_param_t* param );
    virtual ~BT_A2DP_Worker() noexcept override;

    virtual bool Invoke() override;

private:

    void receiveAudioConfigEvent( const esp_a2d_audio_state_t state );
    void configureStream( const esp_a2d_cb_param_t* a2d );

    esp_a2d_cb_event_t m_Event;
    esp_a2d_cb_param_t m_Param;
};

class BT_AVRCPTarget_Worker : public I_BTAppEventWorker
{
public:

    BT_AVRCPTarget_Worker( esp_avrc_tg_cb_event_t event, esp_avrc_tg_cb_param_t *param );
    virtual ~BT_AVRCPTarget_Worker() noexcept override;

    virtual bool Invoke() override;

private:

    enum PassThruCmdEventState
    {
        KeyStatePressed  = 0,
        KeyStateReleased = 1,
    };  

    void receivePassThruCmd( uint8_t cmd_code, uint8_t state );

    esp_avrc_tg_cb_event_t m_Event;
    esp_avrc_tg_cb_param_t m_Param;
};

#endif      // BT_AV_PROTO_WORKER_HPP_INCLUDED
