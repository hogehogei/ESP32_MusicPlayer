
#include "BTAvProtoWorker.hpp"
#include "BluetoothInput.hpp"
#include "Log.hpp"

// esp headers
#include "esp_log.h"
#include "esp_gap_bt_api.h"

//#include "esp_bt.h"
//#include "esp_bt_main.h"
//#include "esp_bt_device.h"

//
// static variables
//
static const char *sk_A2d_ConnStateStr[] = {"Disconnected", "Connecting", "Connected", "Disconnecting"};
static const char *sk_A2d_AudioStateStr[] = {"Suspended", "Stopped", "Started"};

BT_A2DP_Worker::BT_A2DP_Worker( esp_a2d_cb_event_t event, esp_a2d_cb_param_t* param )
 : m_Event( event ),
   m_Param( param )
{}

BT_A2DP_Worker::~BT_A2DP_Worker()
{}

bool BT_A2DP_Worker::Invoke()
{
    ESP_LOGD( LogTagName::sk_BT_A2DP, "%s evt %d", __func__, m_Event );

    if( !m_Param ){
        return false;
    }

    esp_a2d_cb_param_t *a2d = m_Param;
    switch ( m_Event ) {
    case ESP_A2D_CONNECTION_STATE_EVT: {
        uint8_t *bda = a2d->conn_stat.remote_bda;
        ESP_LOGI(LogTagName::sk_BT_A2DP, "A2DP connection state: %s, [%02x:%02x:%02x:%02x:%02x:%02x]",
             sk_A2d_ConnStateStr[a2d->conn_stat.state], bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);
        if (a2d->conn_stat.state == ESP_A2D_CONNECTION_STATE_DISCONNECTED) {
            esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
        } else if (a2d->conn_stat.state == ESP_A2D_CONNECTION_STATE_CONNECTED){
            esp_bt_gap_set_scan_mode(ESP_BT_NON_CONNECTABLE, ESP_BT_NON_DISCOVERABLE);
        }
        break;
    }
    case ESP_A2D_AUDIO_STATE_EVT: {
        ESP_LOGI(LogTagName::sk_BT_A2DP, "A2DP audio state: %s", sk_A2d_AudioStateStr[a2d->audio_stat.state]);
        break;
    }
    case ESP_A2D_AUDIO_CFG_EVT: {
        ESP_LOGI(LogTagName::sk_BT_A2DP, "A2DP audio stream configuration, codec type %d", a2d->audio_cfg.mcc.type);
        // for now only SBC stream is supported
        if (a2d->audio_cfg.mcc.type == ESP_A2D_MCT_SBC) {
            int sample_rate = 16000;
            char oct0 = a2d->audio_cfg.mcc.cie.sbc[0];
            if (oct0 & (0x01 << 6)) {
                sample_rate = 32000;
            } else if (oct0 & (0x01 << 5)) {
                sample_rate = 44100;
            } else if (oct0 & (0x01 << 4)) {
                sample_rate = 48000;
            }

            ESP_LOGI(LogTagName::sk_BT_A2DP, "Configure audio player %x-%x-%x-%x",
                     a2d->audio_cfg.mcc.cie.sbc[0],
                     a2d->audio_cfg.mcc.cie.sbc[1],
                     a2d->audio_cfg.mcc.cie.sbc[2],
                     a2d->audio_cfg.mcc.cie.sbc[3]);
            ESP_LOGI(LogTagName::sk_BT_A2DP, "Audio player configured, sample rate=%d", sample_rate);
        }
        break;
    }
    default:
        ESP_LOGE( LogTagName::sk_BT_A2DP, "%s unhandled evt %d", __func__, m_Event );
        break;
    }

    return true;
}


BT_AVRCPTarget_Worker::BT_AVRCPTarget_Worker( esp_avrc_tg_cb_event_t event, esp_avrc_tg_cb_param_t *param )
 : m_Event( event ),
   m_Param( param )
{}

BT_AVRCPTarget_Worker::~BT_AVRCPTarget_Worker()
{}

bool BT_AVRCPTarget_Worker::Invoke()
{
    ESP_LOGD( LogTagName::sk_BT_AVRCP, "%s evt %d", __func__, m_Event );

    if( !m_Param ){
        return false;
    }

    esp_avrc_tg_cb_param_t *rc = m_Param;
    switch ( m_Event ) {
    case ESP_AVRC_TG_CONNECTION_STATE_EVT: {
        uint8_t *bda = rc->conn_stat.remote_bda;
        ESP_LOGI(LogTagName::sk_BT_AVRCP, "AVRC conn_state evt: state %d, [%02x:%02x:%02x:%02x:%02x:%02x]",
                 rc->conn_stat.connected, bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);
        break;
    }
    case ESP_AVRC_TG_PASSTHROUGH_CMD_EVT: {
        ESP_LOGI(LogTagName::sk_BT_AVRCP, "AVRC passthrough cmd: key_code 0x%x, key_state %d", rc->psth_cmd.key_code, rc->psth_cmd.key_state);
        receivePassThruCmd( rc->psth_cmd.key_code, rc->psth_cmd.key_state );
        break;
    }
    case ESP_AVRC_TG_SET_ABSOLUTE_VOLUME_CMD_EVT: {
        ESP_LOGI(LogTagName::sk_BT_AVRCP, "AVRC set absolute volume: %d%%", (int)rc->set_abs_vol.volume * 100/ 0x7f);
        break;
    }
    case ESP_AVRC_TG_REGISTER_NOTIFICATION_EVT: {
        ESP_LOGI(LogTagName::sk_BT_AVRCP, "AVRC register event notification: %d, param: 0x%x", rc->reg_ntf.event_id, rc->reg_ntf.event_parameter);
#if 0
        if (rc->reg_ntf.event_id == ESP_AVRC_RN_VOLUME_CHANGE) {
            s_volume_notify = true;
            esp_avrc_rn_param_t rn_param;
            rn_param.volume = s_volume;
            esp_avrc_tg_send_rn_rsp(ESP_AVRC_RN_VOLUME_CHANGE, ESP_AVRC_RN_RSP_INTERIM, &rn_param);
        }
#endif
        break;
    }
    case ESP_AVRC_TG_REMOTE_FEATURES_EVT: {
        ESP_LOGI(LogTagName::sk_BT_AVRCP, "AVRC remote features %x, CT features %x", rc->rmt_feats.feat_mask, rc->rmt_feats.ct_feat_flag);
        break;
    }
    default:
        ESP_LOGE( LogTagName::sk_BT_AVRCP, "%s unhandled evt %d", __func__, m_Event );
        break;
    }

    return true;
}

void BT_AVRCPTarget_Worker::receivePassThruCmd( uint8_t cmd_code, uint8_t state )
{
    if( state != PassThruCmdEventState::KeyStatePressed ){
        return;
    }

    BluetoothInput& instance = BluetoothInput::Instance();

    switch( cmd_code ){
    case ESP_AVRC_PT_CMD_PLAY:
        instance.PlayKeyPressed();
        break;
    case ESP_AVRC_PT_CMD_STOP:
        instance.StopKeyPressed();
        break;
    case ESP_AVRC_PT_CMD_PAUSE:
        instance.PauseKeyPressed();
        break;
    }
}
