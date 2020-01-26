
#include <limits>
#include <algorithm>
#include "RingBuffer.hpp"


RingBuffer::RingBuffer( uint32_t bufsize )
 : m_BufHandle( NULL ),
   m_RemainDataCount( 0 )
{
    m_BufHandle = xRingbufferCreate( bufsize, RINGBUF_TYPE_BYTEBUF );
}

RingBuffer::~RingBuffer()
{
    vRingbufferDelete( m_BufHandle );
}

bool RingBuffer::IsValid() const
{
    return m_BufHandle != NULL;
}

uint32_t RingBuffer::Send( const uint8_t* src, uint32_t len )
{
    BaseType_t result = xRingbufferSend( m_BufHandle, src, sizeof(uint8_t), pdMS_TO_TICKS(sk_QueueAccessTimeOutMs) );
    uint32_t sendsize = result == pdTRUE ? len : 0;
    
    m_RemainDataCount = std::min<uint64_t>( m_RemainDataCount + sendsize, std::numeric_limits<uint32_t>::max() );

    return sendsize;
}

uint32_t RingBuffer::Recv( uint8_t* dst, uint32_t len )
{
    size_t   item_count = 0;
    uint32_t retrived_count = 0;
    uint8_t* recv_data = xRingbufferReceiveUpTo( m_BufHandle, &item_count, pdMS_TO_TICKS(sk_QueueAccessTimeOutMs), len );

    if( recv_data != NULL ){
        retrived_count = std::min( item_count, len );

        // Copy received buffer to user space
        std::copy( recv_data, recv_data + retrived_count, dst );
        // Return items
        vRingbufferReturnItem( m_BufHandle, recv_data );
    }
    else {
        retrived_count = 0;
    }

    if( m_RemainDataCount > retrived_count ){
        m_RemainDataCount = 0;
    }
    else {
        m_RemainDataCount = retrived_count;
    }

    return retrived_count;
}

uint32_t RingBuffer::RemainDataCount() const
{
    return m_RemainDataCount;
}
