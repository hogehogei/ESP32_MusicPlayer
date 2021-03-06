#ifndef     AUDIO_DRV_OUT_INCLUDED
#define     AUDIO_DRV_OUT_INCLUDED

#include <cstdint>
#include "StreamInfo.hpp"

class I_AudioSource;
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
    void SetPCMStream( const StreamInfo& info );
    uint16_t CurrentFormat();

private:

    void sendEndFillBytes( const uint32_t send_count );
    
    bool m_IsGood;
    uint8_t m_VolumeRight;
    uint8_t m_VolumeLeft;
};

#endif      // AUDIO_DRV_OUT_INCLUDED
