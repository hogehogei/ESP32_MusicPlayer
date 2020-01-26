
#include "BluetoothAudioSource.hpp"
#include "BluetoothAudio.hpp"

BluetoothAudioSource::BluetoothAudioSource()
{}

BluetoothAudioSource::~BluetoothAudioSource()
{}

uint32_t BluetoothAudioSource::RemainDataCount() const
{
    BluetoothAudio* instance = BluetoothAudio::Instance();
    return instance->RemainDataCount();
}

uint32_t Read( uint8_t* dst, uint32_t len )
{
    BluetoothAudio* instance = BluetoothAudio::Instance();
    return instance->ReadA2D_Data( dst, len );
}
