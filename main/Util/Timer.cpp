
#include "Timer.hpp"

// 
//  esp-idf headers
//
#include "esp_system.h"
#include "esp_timer.h"

MsTimer::MsTimer()
    : m_BeginTime( esp_timer_get_time() )
{}

MsTimer::~MsTimer()
{}

bool MsTimer::IsValid() const
{
    return true;
}

bool MsTimer::IsElapsed( uint32_t time ) const
{
    int64_t nowtime = esp_timer_get_time();
    if( (nowtime - m_BeginTime) >= static_cast<int64_t>(time) ){
        return true;
    }

    return false;
}

void MsTimer::Reset()
{
    m_BeginTime = esp_timer_get_time();
}
