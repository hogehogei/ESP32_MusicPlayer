
#include "BluetoothAudioSource.hpp"
#include "BluetoothAudio.hpp"

//
// class static variables
//
int BluetoothAudioSource::s_CreatedCount = 0;

BluetoothAudioSource::BluetoothAudioSource()
{
    if( !BluetoothAudio::Instance().IsInitialized() ){
        BluetoothAudio::Instance().Initialize();
    }

    ++s_CreatedCount;
}

BluetoothAudioSource::~BluetoothAudioSource()
{   
    --s_CreatedCount;

    if( s_CreatedCount <= 0 ){
        s_CreatedCount = 0;
        BluetoothAudio::Instance().DeInitialize();
    }
}

uint32_t BluetoothAudioSource::RemainDataCount() const
{
    BluetoothAudio& instance = BluetoothAudio::Instance();
    return instance.RemainDataCount();
}

uint32_t BluetoothAudioSource::Read( uint8_t* dst, uint32_t len )
{
    BluetoothAudio& instance = BluetoothAudio::Instance();
    return instance.ReadA2D_Data( dst, len );
}

StreamInfo BluetoothAudioSource::GetStreamInfo() const
{
    BluetoothAudio& instance = BluetoothAudio::Instance();
    return instance.GetConfiguration();
}