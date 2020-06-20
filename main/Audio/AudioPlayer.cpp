
#include "AudioPlayer.hpp"
#include "BluetoothAudioSource.hpp"
#include "AudioDrvOut.hpp"
#include "BluetoothInput.hpp"
#include "ButtonInput.hpp"


AudioPlayerFromSD::AudioPlayerFromSD()
 : m_AudioSrc( std::unique_ptr<SDCAudioSource>( new SDCAudioSource() ) ),
   m_AudioOut( std::unique_ptr<AudioDrvOut>( new AudioDrvOut() ) ),
   m_Mode( Mode_Stop )
{}

AudioPlayerFromSD::~AudioPlayerFromSD()
{}

void AudioPlayerFromSD::Update()
{
    ButtonInput& input = ButtonInput::Instance();
    m_AudioOut->FeedAudioData( m_AudioSrc.get() );

    if( m_AudioSrc->IsEOF() ){
        m_AudioOut->Stop();
        m_AudioSrc->NextEntry();
        m_AudioOut->Start();
    }
    if( input.NextButtonPressed() ){
        m_AudioOut->Stop();
        m_AudioSrc->NextEntry();
        m_AudioOut->Start();
    }
}





AudioPlayerFromBT::AudioPlayerFromBT()
 : m_AudioSrc( std::unique_ptr<BluetoothAudioSource>( new BluetoothAudioSource() )),
   m_AudioOut( std::unique_ptr<AudioDrvOut>( new AudioDrvOut() )),
   m_Mode( Mode_Stop )
{}

AudioPlayerFromBT::~AudioPlayerFromBT()
{}

void AudioPlayerFromBT::Update()
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

    if( m_Mode == Mode_Playing ){
        m_AudioOut->FeedAudioData( m_AudioSrc.get() );
    }
}
