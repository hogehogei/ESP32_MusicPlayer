
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
#include "VS1053.hpp"

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
    ButtonInput& input = ButtonInput::Instance();
    std::unique_ptr<AudioPlayerFromBT> btplayer = std::unique_ptr<AudioPlayerFromBT>( new AudioPlayerFromBT() );
    std::unique_ptr<AudioPlayerFromSD> sdplayer;

    bool is_initialize_ok = btplayer->Initialize();
    if( !is_initialize_ok ){
        goto MUSIC_TASK_FAILED;
    }
    s_AudioPlayer = std::move( btplayer );
    
    while( 1 ){
        if( input.ChangePlayerButtonPressed() ){
            if( s_PlayerType == PlayerType_BT ){
                sdplayer = std::unique_ptr<AudioPlayerFromSD>( new AudioPlayerFromSD() );
                is_initialize_ok = sdplayer->Initialize();
                if( !is_initialize_ok ){
                    goto MUSIC_TASK_FAILED;
                }

                if( !VS1053_Drv::Instance().Initialize() ){
                    goto MUSIC_TASK_FAILED;
                }
                
                s_AudioPlayer = std::move( sdplayer );
                s_PlayerType  = PlayerType_SDC;
            }
        }

        s_AudioPlayer->Update();

        vTaskDelay( 1 );
    }

MUSIC_TASK_FAILED:
    while( 1 ){
        // 初期化に失敗して何もできない。リセットを待ち何もしない。
        vTaskDelay( 1 );
    }
}

}
