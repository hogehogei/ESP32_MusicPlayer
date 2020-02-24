#ifndef     STREAM_INFO_HPP_INCLUDED
#define     STREAM_INFO_HPP_INCLUDED

#include <cstdint>

struct StreamInfo
{
    enum StreamType
    {
        Type_PCM,
        Type_AudioFile,
    };
    enum BitDepth
    {
        BitDepth_8Bit,
        BitDepth_16Bit,
    };
    enum Channel
    {
        Channel_Monoral,
        Channel_Stereo,
    };

    StreamType type;
    uint32_t sample_rate;
    BitDepth bit_depth;
    Channel  channel;        
};

#endif      // STREAM_INFO_HPP_INCLUDED
