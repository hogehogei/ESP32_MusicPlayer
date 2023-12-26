
#include "BluetoothAudioSource.hpp"
#include "BluetoothAudio.hpp"

//
// class static variables
//
int BluetoothAudioSource::s_CreatedCount = 0;

BluetoothAudioSource::BluetoothAudioSource()
    : m_Initialized( false )
{}

BluetoothAudioSource::~BluetoothAudioSource()
{
    if( m_Initialized == true ){
        --s_CreatedCount;

        if( s_CreatedCount <= 0 ){
            s_CreatedCount = 0;
            BluetoothAudio::Instance().DeInitialize();
        }
    }
}

bool BluetoothAudioSource::Initialize()
{
    bool result = true;

    if( !BluetoothAudio::Instance().IsInitialized() ){
        result = BluetoothAudio::Instance().Initialize();
    }

    if( result ){
        ++s_CreatedCount;
    }

    return result;
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