#include <algorithm>

#include "SDCAudioSource.hpp"
#include "FileSystem.hpp"

SDCAudioSource::SDCAudioSource() 
    : m_PlayList( FileSystem::sk_RootDirectoryPath ),
      m_IsGood( true )
{
    const std::string& play_filename = m_PlayList.Current();
    if( play_filename.empty() ){
        m_IsGood = false;
        return;
    }

    if( !m_FileReader.OpenFile( play_filename ) ){
        m_IsGood = false;
        return;
    }
}

SDCAudioSource::~SDCAudioSource()
{}

uint32_t SDCAudioSource::RemainDataCount() const
{
    return IsEOF() ? 0 : 32;
}

uint32_t SDCAudioSource::Read( uint8_t* dst, uint32_t len )
{
    Chunk chunk;
    if( !m_FileReader.GetChunk( &chunk ) ){
        return 0;
    }

    uint32_t copy_len = std::min<uint32_t>( len, chunk.length );
    std::copy( chunk.data , chunk.data  + copy_len, dst );

    return copy_len;
}

bool SDCAudioSource::IsEOF() const
{
    return m_FileReader.IsEOF();
}

void SDCAudioSource::NextEntry()
{
    m_PlayList.Next();
    m_FileReader.OpenFile( m_PlayList.Current() );
}
