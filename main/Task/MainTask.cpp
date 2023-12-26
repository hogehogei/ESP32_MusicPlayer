
#include "MainTask.hpp"

#include <memory>

// 
//  FreeRTOS headers
//
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

//
//  original headers
//
#include "AudioPlayer.hpp"
#include "ButtonInput.hpp"

//
//  type definitions
//
enum PlayerType
{
    PlayerType_BT,
    PlayerType_SDC,
};

// 
//  static variables
//
static std::unique_ptr<I_AudioPlayer> s_AudioPlayer;
static PlayerType s_PlayerType = PlayerType_BT;

extern "C"
{

void MusicTask( void* param )
{
    s_AudioPlayer = std::unique_ptr<AudioPlayerFromBT>( new AudioPlayerFromBT() );
    ButtonInput& input = ButtonInput::Instance();
    
    while( 1 ){
        if( input.ChangePlayerButtonPressed() ){
            if( s_PlayerType == PlayerType_BT ){
                s_AudioPlayer = std::unique_ptr<AudioPlayerFromSD>( new AudioPlayerFromSD() );
                s_PlayerType  = PlayerType_SDC;
            }
        }

        s_AudioPlayer->Update();

        vTaskDelay( 1 );
    }
}

}
