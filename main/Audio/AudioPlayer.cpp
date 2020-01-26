
#include "AudioPlayer.hpp"
#include "BluetoothAudioSource.hpp"

AudioPlayer::AudioPlayer( AudioSourceMode srcmode )
 : m_AudioSrc( nullptr ),
   m_AudioOut( std::make_unique<AudioDriver>() ),
   m_IsPlaying( false )
{
    m_AudioSrc = std::unique_ptr<I_AudioSource>( createAudioSource( srcmode ) );
}

AudioPlayer::~AudioPlayer()
{}

void AudioPlayer::Update()
{
    Input* action = Input::Instance();

    if( action->Stop() ){
        if( m_IsPlaying ){
            m_AudioOut->Stop();
            m_IsPlaying = false;
        }
    }
    if( action->Pause() ){
        if( m_IsPlaying ){
            m_IsPlaying = false;
        }
    }
    if( action->Play() ){
        if( !m_IsPlaying ){
            m_AudioOut->Play();
            m_IsPlaying = true;
        }
    }
    if( action->VolumeUp() ){
        m_AudioOut->VolumeUp();
    }
    if( action->VolumeDown() ){
        m_AudioOut->VolumeDown();
    }

    if( m_IsPlaying && m_AudioOut->CanFeedData() ){
        m_AudioOut->FeedAudioData( m_AudioSrc );
    }
}

I_AudioSource* AudioPlayer::createAudioSource( AudioSourceMode srcmode )
{
    return new BluetoothAudioSource;
}