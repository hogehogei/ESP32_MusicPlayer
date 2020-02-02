
#ifndef    RING_BUFFER_HPP
#define    RING_BUFFER_HPP

#include <freertos/FreeRTOS.h>
#include <freertos/ringbuf.h>

class RingBuffer
{
public:

    RingBuffer( uint32_t bufsize );
    ~RingBuffer() noexcept;

    RingBuffer( const RingBuffer& ) = delete;
    RingBuffer& operator=( const RingBuffer& ) = delete;

    bool IsValid() const;
    uint32_t Send( const uint8_t* src, uint32_t len );
    uint32_t Recv( uint8_t* dst, uint32_t len );
    uint32_t RemainDataCount() const;
    
private:

    static constexpr int sk_QueueAccessTimeOutMs = 1000;
    RingbufHandle_t m_BufHandle;
    uint32_t        m_RemainDataCount;
};

#endif      // RING_BUFFER_HPP