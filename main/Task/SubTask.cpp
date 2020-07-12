
#include "SubTask.hpp"
#include "ButtonInput.hpp"
#include "BattVolt.hpp"

// 
//  FreeRTOS headers
//
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// esp-idf headers
#include "esp_log.h"
#include "esp_sleep.h"


extern "C"
{

static int s_BattVoltShowCnt = 0;

static bool BattVoltLowJdg();

void SubTask_10ms( void* param )
{
    ButtonInput& input = ButtonInput::Instance();
    BattVolt& battvolt = BattVolt::Instance();

    battvolt.Initialize();
    
    const int inputtask_interval = 10;
    while(1){
        // ボタン入力更新
        input.Update();

        // バッテリ電圧更新
        battvolt.Update();
    
        if( ++s_BattVoltShowCnt >= 500 ){
            ESP_LOGI( "BattVolt", "BattVolt %u[mV]", battvolt.MillVolt() );
            s_BattVoltShowCnt = 0;
        }
        if( BattVoltLowJdg() ){
            //esp_deep_sleep_start();
        }
        vTaskDelay( inputtask_interval / portTICK_RATE_MS );
    }
}

static bool BattVoltLowJdg()
{
    static constexpr int sk_BattVoltLowJdgTime = 1000;                   //! バッテリ電圧Low判定時間[10ms]
    static constexpr uint32_t sk_BattVoltLowThreshMillVolt = 2000;       //! バッテリ電圧Low閾値[mV]
    static int s_LowJdgCount = 0;

    BattVolt& battvolt = BattVolt::Instance();
    if( battvolt.MillVolt() < sk_BattVoltLowThreshMillVolt ){
        if( s_LowJdgCount < sk_BattVoltLowJdgTime ){
            ++s_LowJdgCount;
        }
        if( s_LowJdgCount >= sk_BattVoltLowJdgTime ){
            return true;
        }
    }
    else {
        s_LowJdgCount = 0;
    }

    return false;
}

}

