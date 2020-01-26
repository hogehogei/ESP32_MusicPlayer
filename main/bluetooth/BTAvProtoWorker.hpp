
#ifndef    BT_AV_PROTO_WORKER_HPP
#define    BT_AV_PROTO_WORKER_HPP

#include "I_BTAppEventWorker.hpp"

// esp32 bluetooth headers
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"


class BT_A2DP_Worker : public I_BTAppEventWorker
{
public:

    BT_A2DP_Worker( esp_a2d_cb_event_t event, esp_a2d_cb_param_t* param );
    virtual ~BT_A2DP_Worker() noexcept;

    virtual bool Invoke();

private:

    esp_a2d_cb_event_t  m_Event;
    esp_a2d_cb_param_t* m_Param;
};

class BT_AVRCPTarget_Worker : public I_BTAppEventWorker
{
public:

    BT_AVRCPTarget_Worker( esp_avrc_tg_cb_event_t event, esp_avrc_tg_cb_param_t *param );
    virtual ~BT_AVRCPTarget_Worker() noexcept;

    virtual bool Invoke();

private:

    esp_avrc_tg_cb_event_t  m_Event;
    esp_avrc_tg_cb_param_t* m_Param;
};

#endif      // BT_AV_PROTO_WORKER_HPP
