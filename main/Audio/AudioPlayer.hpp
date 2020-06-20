
#ifndef    AUDIO_PLAYER_HPP_INCLUDED
#define    AUDIO_PLAYER_HPP_INCLUDED

#include <memory>
#include "I_AudioSource.hpp"
#include "I_AudioPlayer.hpp"
#include "BluetoothAudioSource.hpp"
#include "SDCAudioSource.hpp"
#include "AudioDrvOut.hpp"

class AudioPlayerFromSD : public I_AudioPlayer
{

    enum PlayingMode
    {
        Mode_Stop,
        Mode_Pause,
        Mode_Playing,
    };

public:

    AudioPlayerFromSD();
    ~AudioPlayerFromSD() noexcept;

    AudioPlayerFromSD( const AudioPlayerFromSD& ) = delete;
    AudioPlayerFromSD& operator=( const AudioPlayerFromSD& ) = delete;

    void Update();

private:

    std::unique_ptr<SDCAudioSource> m_AudioSrc;
    std::unique_ptr<AudioDrvOut>   m_AudioOut;
    PlayingMode                    m_Mode;
};

class AudioPlayerFromBT : public I_AudioPlayer
{
public:

    enum PlayingMode
    {
        Mode_Stop,
        Mode_Pause,
        Mode_Playing,
    };

public:

    AudioPlayerFromBT();
    ~AudioPlayerFromBT() noexcept;

    AudioPlayerFromBT( const AudioPlayerFromBT& ) = delete;
    AudioPlayerFromBT& operator=( const AudioPlayerFromBT& ) = delete;

    void Update();

private:

    std::unique_ptr<BluetoothAudioSource> m_AudioSrc;
    std::unique_ptr<AudioDrvOut>   m_AudioOut;
    PlayingMode                    m_Mode;
};

#endif      // AUDIO_PLAYER_HPP_INCLUDED
