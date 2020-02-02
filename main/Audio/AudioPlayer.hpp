
#ifndef    AUDIO_PLAYER_HPP_INCLUDED
#define    AUDIO_PLAYER_HPP_INCLUDED

#include <memory>
#include "I_AudioSource.hpp"
#include "AudioDrvOut.hpp"

class AudioPlayer
{
public:

    enum AudioSourceMode 
    {
        AudioSrc_Bluetooth,
        AudioSrc_SDC,
    };

public:

    AudioPlayer( AudioSourceMode srcmode );
    ~AudioPlayer() noexcept;

    AudioPlayer( const AudioPlayer& ) = delete;
    AudioPlayer& operator=( const AudioPlayer& ) = delete;

    void Update();

private:

    I_AudioSource* createAudioSource( AudioSourceMode srcmode );

    std::unique_ptr<I_AudioSource> m_AudioSrc;
    std::unique_ptr<AudioDrvOut>   m_AudioOut;
    bool m_IsPlaying;
};

#endif      // AUDIO_PLAYER_HPP_INCLUDED
