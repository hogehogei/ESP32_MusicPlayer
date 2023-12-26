#ifndef     SDC_AUDIO_SOURCE_HPP_INCLUDED
#define     SDC_AUDIO_SOURCE_HPP_INCLUDED

#include <cstdint>
#include "I_AudioSource.hpp"
#include "StreamInfo.hpp"
#include "PlayList.hpp"
#include "MusicFileReader.hpp"


class SDCAudioSource : public I_AudioSource
{
public:

    SDCAudioSource();
    virtual ~SDCAudioSource() noexcept;

    virtual uint32_t RemainDataCount() const;
    virtual uint32_t Read( uint8_t* dst, uint32_t len );
    
    bool IsEOF() const;
    void NextEntry();
    bool IsGood() const;

private:

    PlayList m_PlayList;
    MusicFileReader m_FileReader;
    bool m_IsGood;
};

#endif      // SDC_AUDIO_SOURCE_HPP_INCLUDED