
#include "WavHeader.hpp"

#include <algorithm>
#include "Endian.hpp"

// 
//  static variables
//
static constexpr char sk_HeaderChunkID[] = "RIFF";
static constexpr char sk_Format[]        = "WAVE";
static constexpr char sk_SubChunk1ID[]   = "fmt ";
static constexpr char sk_SubChunk2ID[]   = "data";


WavHeader::WavHeader( uint32_t sample_rate, BitDepth bit_depth, Channel ch, uint32_t size )
    : m_SampleRate( sample_rate ),
      m_BitDepth( bit_depth ),
      m_Channel( ch ), 
      m_DataSize( size )
{}

WavHeader::WavHeader( const StreamInfo& info )
    : m_SampleRate( info.sample_rate ),
      m_BitDepth( info.bit_depth == StreamInfo::BitDepth_8Bit ? WavHeader::BitDepth_8Bit : WavHeader::BitDepth_16Bit ),
      m_Channel( info.channel == StreamInfo::Channel_Monoral ? WavHeader::Channel_Monoral : WavHeader::Channel_Stereo ),
      m_DataSize( 0xFFFFFFFF )
{}

WavHeader::~WavHeader()
{}

uint8_t* WavHeader::CreateHeader() const
{
    uint8_t* hdr = new uint8_t[sk_HeaderSize];

    if( hdr ){
        std::copy( sk_HeaderChunkID, sk_HeaderChunkID + 4, &hdr[0] );
        exlib::Endian<exlib::LittleEndian>::SetUint32( &hdr[4], 0xFFFFFFFF );
        std::copy( sk_Format, sk_Format + 4, &hdr[8] );
        std::copy( sk_SubChunk1ID, sk_SubChunk1ID + 4, &hdr[12] );
        exlib::Endian<exlib::LittleEndian>::SetUint32( &hdr[16], 16 );
        exlib::Endian<exlib::LittleEndian>::SetUint16( &hdr[20], 1 );
        exlib::Endian<exlib::LittleEndian>::SetUint16( &hdr[22], NumOfChannels() );
        exlib::Endian<exlib::LittleEndian>::SetUint32( &hdr[24], SampleRate() );
        exlib::Endian<exlib::LittleEndian>::SetUint32( &hdr[28], ByteRate() );
        exlib::Endian<exlib::LittleEndian>::SetUint16( &hdr[32], BlockAlign() );
        exlib::Endian<exlib::LittleEndian>::SetUint16( &hdr[34], BitsPerSample() );
        std::copy( sk_SubChunk2ID, sk_SubChunk2ID + 4, &hdr[36] );
        exlib::Endian<exlib::LittleEndian>::SetUint32( &hdr[40], 0xFFFFFFFF );
    }

    return hdr;
}

uint32_t WavHeader::NumOfChannels() const
{
    return m_Channel == Channel_Monoral ? 1 : 2;
}

uint32_t WavHeader::SampleRate() const
{
    return m_SampleRate;
}

uint32_t WavHeader::ByteRate() const
{
    return SampleRate() * BlockAlign();
}

uint32_t WavHeader::BlockAlign() const
{
    return NumOfChannels() * BitsPerSample() / 8;
}

uint32_t WavHeader::BitsPerSample() const
{
    return m_BitDepth == BitDepth_8Bit ? 8 : 16;
}