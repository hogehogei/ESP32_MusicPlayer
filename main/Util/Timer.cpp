
#include "Timer.hpp"

// 
//  esp-idf headers
//
#include "esp_system.h"
#include "esp_timer.h"

// esp headers
#include "esp_log.h"

MsTimer::MsTimer()
    : m_BeginTimeMs( esp_timer_get_time() / 1000 )
{}

MsTimer::~MsTimer()
{}

bool MsTimer::IsValid() const
{
    return true;
}

bool MsTimer::IsElapsed( uint32_t milli_sec ) const
{
    int64_t nowtime = esp_timer_get_time() / 1000;
    if( (nowtime - m_BeginTimeMs) >= static_cast<int64_t>(milli_sec) ){
        ESP_LOGI( "Timer", "nowtime : %d, begintime : %d", (int)nowtime, (int)m_BeginTimeMs);
        return true;
    }

    return false;
}

void MsTimer::Reset()
{
    m_BeginTimeMs = esp_timer_get_time() / 1000;
}
