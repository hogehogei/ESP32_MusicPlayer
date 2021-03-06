#ifndef     I_AUDIO_SOURCE_HPP_INCLUDED
#define     I_AUDIO_SOURCE_HPP_INCLUDED

#include <cstdint>
#include "StreamInfo.hpp"

class I_AudioSource
{
public:

    I_AudioSource() {}
    virtual ~I_AudioSource() noexcept {}

    virtual uint32_t RemainDataCount() const = 0;
    virtual uint32_t Read( uint8_t* dst, uint32_t len ) = 0;
};

#endif      // I_AUDIO_SOURCE_HPP_INCLUDED
