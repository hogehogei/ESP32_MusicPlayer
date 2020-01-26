
#include "AudioDrvOut.hpp"
#include "I_AudioSource.hpp"

AudioDrvOut::AudioDrvOut()
{
}

AudioDrvOut::~AudioDrvOut()
{
}

void AudioDrvOut::Start()
{
}

void AudioDrvOut::Stop()
{
    constexpr int sk_EndFillByteSendNum = 2052;
    constexpr int sk_EndFillByteCheckCancel = 32;
    constexpr int sk_SendEndFillBytesMaxForWaitEnding = 64;     // 送信終了時に送信するデータのMAX値[x 32byte]

    // EOF にたどり着いたときの終了処理を参考にしている
    // 通常のSTOP処理では、サウンドデータを追加で送信する必要があるが
    // この関数の引数には渡したくないため（再生停止処理でサウンドデータを渡すのはおかしくない？）
    
    sendEndFillBytes( sk_EndFillByteSendNum );
    // SM_CANCEL をセット
    m_MusicDriver.WriteSCI( VS1053_Drv::SCI_MODE, 0x0808 );

    int end_fill_bytes_sendcnt = 0;
    uint16_t sci_mode_reg;
    bool is_ending_succeed = false;
    while(1){
        sendEndFillBytes( sk_EndFillByteCheckCancel );
        m_MusicDriver.ReadSCI( VS1053_Drv::SCI_MODE, &sci_mode_reg );
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
        m_MusicDriver.SoftReset();
    }
}

void AudioDrvOut::VolumeUp()
{
    m_VolumeLeft = m_VolumeLeft > 0 ? m_VolumeLeft - 1 : 0;
    m_VolumeRight = m_VolumeRight > 0 ? m_VolumeRight - 1 : 0;

    m_MusicDriver.SetVolume( m_VolumeLeft, m_VolumeRight );
}

void AudioDrvOut::VolumeDown()
{
    m_VolumeLeft = m_VolumeLeft < std::numeric_limits<uint8_t>::max() ? m_VolumeLeft + 1 : std::numeric_limits<uint8_t>::max();
    m_VolumeRight = m_VolumeRight < std::numeric_limits<uint8_t>::max() ? m_VolumeRight + 1 : std::numeric_limits<uint8_t>::max();

    m_MusicDriver.SetVolume( m_VolumeLeft, m_VolumeRight );
}

void AudioDrvOut::FeedAudioData( I_AudioSource* source )
{
    static constexpr sk_FeedDataChunkByteSize = 32;
    uint8_t buf[sk_FeedDataChunkByteSize];

    if( !m_MusicDriver.IsBusy() ){
        if( source->RemainDataCount() >= sk_FeedDataChunkByteSize ){
            source->Read( buf, sk_FeedDataChunkByteSize );
            m_MusicDriver.WriteSDI( buf, sk_FeedDataChunkByteSize );
        }
    }
}

void AudioDrvOut::sendEndFillBytes( const uint32_t send_count )
{
    uint8_t end_fill_byte = m_MusicDriver.ReadEndFillByte();
 
    for( uint32_t i = 0; i < send_count; ++i ){
        m_MusicDriver.WriteSDI( &end_fill_byte, 1 );
    }
}
