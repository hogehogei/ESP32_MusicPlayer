
#include "MainTask.hpp"

#include <memory>
#include "AudioPlayer.hpp"

// 
//  static variables
//
static std::unique_ptr<AudioPlayer> s_AudioPlayer;

extern "C"
{

void MusicTask( void* param )
{
    s_AudioPlayer = std::unique_ptr<AudioPlayer>( new AudioPlayer(AudioPlayer::AudioSrc_Bluetooth) );

    while( 1 ){
        s_AudioPlayer->Update();
    }
}

}
