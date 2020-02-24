#ifndef     WAV_HEADER_HPP_INCLUDED
#define     WAV_HEADER_HPP_INCLUDED

#include <cstdint>
#include "StreamInfo.hpp"

class WavHeader
{
public:
    enum Channel {  
        Channel_Monoral,
        Channel_Stereo,
    };
    enum BitDepth {
        BitDepth_8Bit,
        BitDepth_16Bit,
    };

    static constexpr int sk_HeaderSize = 44;

public:
    
    WavHeader( uint32_t sample_rate, BitDepth bit_depth, Channel ch, uint32_t size );
    WavHeader( const StreamInfo& info );
    ~WavHeader() noexcept;

    uint8_t* CreateHeader() const;

    uint32_t NumOfChannels() const;
    uint32_t SampleRate() const;
    uint32_t ByteRate() const;
    uint32_t BlockAlign() const;
    uint32_t BitsPerSample() const;

private:

    uint32_t m_SampleRate;
    BitDepth m_BitDepth;
    Channel  m_Channel;     
    uint32_t m_DataSize;
};

#endif      // WAV_HEADER_HPP_INCLUDED