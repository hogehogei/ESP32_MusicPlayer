
#include "InputTask.hpp"

// 
//  FreeRTOS headers
//
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "ButtonInput.hpp"

extern "C"
{

void InputTask( void* param )
{
    ButtonInput& input = ButtonInput::Instance();
    const int inputtask_interval = 10;

    while(1){
        // ボタン入力更新
        input.Update();
    
        vTaskDelay( inputtask_interval );
    }
}

}

