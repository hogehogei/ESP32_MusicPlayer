#ifndef     BLUETOOTH_AUDIO_SOURCE_HPP_INCLUDED
#define     BLUETOOTH_AUDIO_SOURCE_HPP_INCLUDED

#include "I_AudioSource.hpp"
#include "StreamInfo.hpp"

class BluetoothAudioSource : public I_AudioSource
{
public:

    BluetoothAudioSource();
    virtual ~BluetoothAudioSource() noexcept;

    bool Initialize();
    virtual uint32_t RemainDataCount() const override;
    virtual uint32_t Read( uint8_t* dst, uint32_t len ) override;
    StreamInfo GetStreamInfo() const;

private:

    bool m_Initialized;
    static int s_CreatedCount;
};

#endif      // BLUETOOTH_AUDIO_SOURCE_HPP_INCLUDED
