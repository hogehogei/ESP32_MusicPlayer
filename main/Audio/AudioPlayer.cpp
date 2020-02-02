
#include "AudioPlayer.hpp"
#include "BluetoothAudioSource.hpp"
#include "AudioDrvOut.hpp"
#include "BluetoothInput.hpp"

AudioPlayer::AudioPlayer( AudioSourceMode srcmode )
 : m_AudioSrc( nullptr ),
   m_AudioOut( std::unique_ptr<AudioDrvOut>( new AudioDrvOut() ) ),
   m_IsPlaying( false )
{
    m_AudioSrc = std::unique_ptr<I_AudioSource>( createAudioSource( srcmode ) );
}

AudioPlayer::~AudioPlayer()
{}

void AudioPlayer::Update()
{
    BluetoothInput& input = BluetoothInput::Instance();

    if( input.IsStopKeyPressed() ){
        if( m_IsPlaying ){
            m_AudioOut->Stop();
            m_IsPlaying = false;
        }
    }
    if( input.IsPauseKeyPressed() ){
        if( m_IsPlaying ){
            m_IsPlaying = false;
        }
    }
    if( input.IsPlayKeyPressed() ){
        if( !m_IsPlaying ){
            m_AudioOut->Start();
            m_IsPlaying = true;
        }
    }
#if 0
    if( input->VolumeUp() ){
        m_AudioOut->VolumeUp();
    }
    if( input->VolumeDown() ){
        m_AudioOut->VolumeDown();
    }
#endif

    if( m_IsPlaying ){
        m_AudioOut->FeedAudioData( m_AudioSrc.get() );
    }
}

I_AudioSource* AudioPlayer::createAudioSource( AudioSourceMode srcmode )
{
    return new BluetoothAudioSource;
}