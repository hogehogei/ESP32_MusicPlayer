
#include "MusicFileReader.hpp"

#include <algorithm>

MusicFileReader::ReadBuffer::ReadBuffer()
:
    m_File(),
    m_Length( 0 ),
    m_ReadIndex( 0 ),
    m_IsEOF( true )
{}

MusicFileReader::ReadBuffer::ReadBuffer( const File& file )
: 
    m_File( file ),
    m_Length( 0 ),
    m_ReadIndex( 0 ),
    m_IsEOF( false )
{}

MusicFileReader::ReadBuffer::~ReadBuffer() 
{}

bool MusicFileReader::ReadBuffer::IsEnd() const
{
    return m_IsEOF && isBufferEmpty();
}

bool MusicFileReader::ReadBuffer::GetChunk( Chunk* chunk )
{
    if( isBufferEmpty() ){
        if( !readNewSector() ){
            return false;
        }
    }

    if( IsEnd() ){
        return false;
    }

    readCurrentChunk( chunk );

    return true;
}

bool MusicFileReader::ReadBuffer::readNewSector()
{
    if( m_IsEOF ){
        return false;
    }

    uint32_t readlen = 0;
    if( !m_File.Read( m_Buffer, sk_SectorSize, &readlen ) ){
        ESP_LOGE( "MusicFileReader", "Read sector failed." );
        m_IsEOF  = true;
        m_Length = 0;
        m_ReadIndex = 0;
        return false;
    }

    if( m_File.IsEOF() ){
        m_IsEOF = true;
    }
    
    m_Length    = readlen;
    m_ReadIndex = 0;

    return true;
}

bool MusicFileReader::ReadBuffer::isBufferEmpty() const
{
    return m_ReadIndex >= m_Length;
}

void MusicFileReader::ReadBuffer::readCurrentChunk( Chunk* chunk )
{
    // ChunkSize分 か 最終データのどちらかまでコピー
    uint32_t copylast = std::min( m_ReadIndex + Chunk::sk_ChunkSize, m_Length );
    std::copy( &m_Buffer[m_ReadIndex], &m_Buffer[copylast], chunk->data );

    chunk->length = copylast - m_ReadIndex;
    m_ReadIndex = copylast;
}

MusicFileReader::MusicFileReader()
:
    m_File(),   
    m_Buffer()
{}

MusicFileReader::~MusicFileReader()
{}

bool MusicFileReader::OpenFile( const std::string& filepath )
{
    m_File = File( filepath );
    m_Buffer = ReadBuffer( m_File );

    return m_File.IsOpened();
}

bool MusicFileReader::IsEOF() const
{
    return m_Buffer.IsEnd();
}

bool MusicFileReader::GetChunk( Chunk* chunk )
{
    return m_Buffer.GetChunk( chunk );
}