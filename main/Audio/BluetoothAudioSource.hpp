#ifndef     BLUETOOTH_AUDIO_SOURCE_HPP
#define     BLUETOOTH_AUDIO_SOURCE_HPP

#include "I_AudioSource.hpp"

class BluetoothAudioSource : public I_AudioSource
{
public:

    BluetoothAudioSource();
    virtual ~BluetoothAudioSource() noexcept;

    virtual uint32_t RemainDataCount() const override;
    virtual uint32_t Read( uint8_t* dst, uint32_t len ) override;

private:

};

#endif      // BLUETOOTH_AUDIO_SOURCE_HPP
