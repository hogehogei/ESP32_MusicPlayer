
#include "AudioDrvOut.hpp"

#include <limits>
#include <algorithm>
#include "I_AudioSource.hpp"
#include "VS1053.hpp"
#include "WavHeader.hpp"

// esp headers
#include "esp_log.h"

AudioDrvOut::AudioDrvOut()
{
}

AudioDrvOut::~AudioDrvOut()
{
    Stop();
}

void AudioDrvOut::Start()
{
}

void AudioDrvOut::Stop()
{
    constexpr int sk_EndFillByteSendNum = 2052;
    constexpr int sk_EndFillByteCheckCancel = 32;
    constexpr int sk_SendEndFillBytesMaxForWaitEnding = 64;     // 送信終了時に送信するデータのMAX値[x 32byte]

    VS1053_Drv& driver = VS1053_Drv::Instance();

    // EOF にたどり着いたときの終了処理を参考にしている
    // 通常のSTOP処理では、サウンドデータを追加で送信する必要があるが
    // この関数の引数には渡したくないため（再生停止処理でサウンドデータを渡すのはおかしくない？）
    
    sendEndFillBytes( sk_EndFillByteSendNum );
    // SM_CANCEL をセット
    driver.WriteSCI( VS1053_Drv::SCI_MODE, 0x0808 );

    int end_fill_bytes_sendcnt = 0;
    uint16_t sci_mode_reg;
    bool is_ending_succeed = false;
    while(1){
        sendEndFillBytes( sk_EndFillByteCheckCancel );
        driver.ReadSCI( VS1053_Drv::SCI_MODE, &sci_mode_reg );
        if( (sci_mode_reg & 0x0008) != 0x0008 ){
            is_ending_succeed = true;
            break;
        }
        ++end_fill_bytes_sendcnt;
        if( end_fill_bytes_sendcnt >= sk_SendEndFillBytesMaxForWaitEnding ){
            break;
        }
    }

    if( !is_ending_succeed ){
        driver.SoftReset();
    }
}

void AudioDrvOut::VolumeUp()
{
    VS1053_Drv& driver = VS1053_Drv::Instance();

    m_VolumeLeft = m_VolumeLeft > 0 ? m_VolumeLeft - 1 : 0;
    m_VolumeRight = m_VolumeRight > 0 ? m_VolumeRight - 1 : 0;

    driver.SetVolume( m_VolumeLeft, m_VolumeRight );
}

void AudioDrvOut::VolumeDown()
{
    VS1053_Drv& driver = VS1053_Drv::Instance();

    m_VolumeLeft = m_VolumeLeft < std::numeric_limits<uint8_t>::max() ? m_VolumeLeft + 1 : std::numeric_limits<uint8_t>::max();
    m_VolumeRight = m_VolumeRight < std::numeric_limits<uint8_t>::max() ? m_VolumeRight + 1 : std::numeric_limits<uint8_t>::max();

    driver.SetVolume( m_VolumeLeft, m_VolumeRight );
}

void AudioDrvOut::FeedAudioData( I_AudioSource* source )
{
    static constexpr int sk_FeedDataChunkByteSize = 32;
    static uint8_t buf[sk_FeedDataChunkByteSize];
    VS1053_Drv& driver = VS1053_Drv::Instance();
    
    while( !driver.IsBusy() && source->RemainDataCount() > 0 ){
        int size = source->Read( buf, sk_FeedDataChunkByteSize );
        driver.WriteSDI( buf, size );
        //ESP_LOGI("AudioDrvOut", "CurrentFormat : %x", CurrentFormat() );
        //ESP_LOGI("AudioDrvOut", "readsize : %d", size );
    }
}

void AudioDrvOut::SetPCMStream( const StreamInfo& info )
{
    static constexpr int sk_FeedDataChunkByteSize = 32;
    VS1053_Drv& driver = VS1053_Drv::Instance();
    WavHeader wav_header( info );
    uint8_t* wav_header_bytes = wav_header.CreateHeader();

    ESP_LOGI("AudioDrvOut", "SetPCMStream");

    int cnt = 0;
    for( int i = 0; i < 8; ++i ){
        for( int j = 0; j < 16; ++j ){
            if( cnt >= WavHeader::sk_HeaderSize ){
                i = 8; break;
            }

            printf( "%02X ", (unsigned)wav_header_bytes[i * 16 + j] );
            ++cnt;
        }
        printf( "\n" );
    }

    uint32_t write_size = 0;
    while( write_size < WavHeader::sk_HeaderSize ){
        if( !driver.IsBusy() ){
            uint32_t size = std::min<uint32_t>( WavHeader::sk_HeaderSize - write_size, sk_FeedDataChunkByteSize );
            driver.WriteSDI( &wav_header_bytes[write_size], size );
            write_size += size;
        }
    }

    delete [] wav_header_bytes;
}

uint16_t AudioDrvOut::CurrentFormat()
{
    VS1053_Drv& driver = VS1053_Drv::Instance();
    uint16_t data;

    driver.ReadSCI( VS1053_Drv::SCI_HDAT1, &data );

    return data;
}

void AudioDrvOut::sendEndFillBytes( const uint32_t send_count )
{
    VS1053_Drv& driver = VS1053_Drv::Instance();
    uint8_t end_fill_byte = driver.ReadEndFillByte();
 
    for( uint32_t i = 0; i < send_count; ++i ){
        driver.WriteSDI( &end_fill_byte, 1 );
    }
}
