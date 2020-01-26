
#define    AUDIO_PLAYER_HPP
#define    AUDIO_PLAYER_HPP

#include <memory>

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
    std::unique_ptr<AudioDriver>   m_AudioOut;
    bool m_IsPlaying;
};

#endif      // AUDIO_PLAYER_HPP
