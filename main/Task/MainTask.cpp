
#include "MainTask.hpp"

#include <memory>
#include "AudioPlayer.hpp"

// 
//  FreeRTOS headers
//
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// 
//  static variables
//
static std::unique_ptr<AudioPlayer> s_AudioPlayer;

extern "C"
{

void MusicTask( void* param )
{
    s_AudioPlayer = std::unique_ptr<AudioPlayer>( new AudioPlayer(AudioPlayer::AudioSrc_Bluetooth) );
    const int delay_ms = 1;
    
    while( 1 ){
        s_AudioPlayer->Update();
        vTaskDelay( delay_ms / portTICK_RATE_MS );
    }
}

}
