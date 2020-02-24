
#include "AudioPlayer.hpp"
#include "BluetoothAudioSource.hpp"
#include "AudioDrvOut.hpp"
#include "BluetoothInput.hpp"

AudioPlayer::AudioPlayer( AudioSourceMode srcmode )
 : m_AudioSrc( nullptr ),
   m_AudioOut( std::unique_ptr<AudioDrvOut>( new AudioDrvOut() ) ),
   m_Mode( Mode_Stop )
{
    m_AudioSrc = std::unique_ptr<I_AudioSource>( createAudioSource( srcmode ) );
}

AudioPlayer::~AudioPlayer()
{}

void AudioPlayer::Update()
{
    BluetoothInput& input = BluetoothInput::Instance();

    if( input.IsStopKeyPressed() ){
        if( m_Mode != Mode_Stop ){
            m_AudioOut->Stop();
            m_Mode = Mode_Stop;
        }
    }
    if( input.IsPauseKeyPressed() ){
        m_Mode = Mode_Pause;
    }
    if( input.IsPlayKeyPressed() ){
        if( m_Mode != Mode_Playing ){
            if( m_Mode == Mode_Stop ){
                StreamInfo info = m_AudioSrc->GetStreamInfo();
                m_AudioOut->SetPCMStream( info );
            }
            m_AudioOut->Start();
            m_Mode = Mode_Playing;
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

    if( m_Mode == Mode_Playing ){
        m_AudioOut->FeedAudioData( m_AudioSrc.get() );
    }
}

I_AudioSource* AudioPlayer::createAudioSource( AudioSourceMode srcmode )
{
    return new BluetoothAudioSource;
}