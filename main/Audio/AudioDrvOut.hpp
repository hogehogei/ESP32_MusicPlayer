

#ifndef     AUDIO_DRV_OUT_DEFINED
#define     AUDIO_DRV_OUT_DEFINED

#include "VS1053.hpp"

class AudioDrvOut
{
public:

    AudioDrvOut();
    ~AudioDrvOut() noexcept;

    void Start();
    void Stop();
    void VolumeUp();
    void VolumeDown();
    void FeedAudioData( I_AudioSource* source );

private:

    void sendEndFillBytes( const uint32_t send_count );
    
    bool m_IsGood;
    uint8_t m_VolumeRight;
    uint8_t m_VolumeLeft;
    
    VS1053_Drv      m_MusicDriver;
};

#endif      // AUDIO_DRV_OUT_DEFINED
