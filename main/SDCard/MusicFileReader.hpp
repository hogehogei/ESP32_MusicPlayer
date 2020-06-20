
#ifndef MUSIC_FILE_READER_HPP_INCLUDED
#define MUSIC_FILE_READER_HPP_INCLUDED

#include <cstdint>
#include "FileSystem.hpp"

struct Chunk
{
    static constexpr int sk_ChunkSize = 32;
    uint8_t data[sk_ChunkSize];
    int length;
};

class MusicFileReader
{
private:

    class ReadBuffer
    {
    public:

        ReadBuffer();
        ReadBuffer( const File& file );
        ~ReadBuffer() noexcept;

        bool IsEnd() const;
        bool GetChunk( Chunk* chunk );

    private:

        bool readNewSector();
        bool isBufferEmpty() const;
        void readCurrentChunk( Chunk* chunk );

        static constexpr int sk_SectorSize = 512;

        File     m_File;
        uint8_t  m_Buffer[sk_SectorSize];
        uint32_t m_Length;
        uint32_t m_ReadIndex;
        bool     m_IsEOF;
    };

public:

    MusicFileReader();
    ~MusicFileReader() noexcept;

    bool OpenFile( const std::string& filepath );
    //bool IsOpenedFile() const;
    bool IsEOF() const;
    bool GetChunk( Chunk* chunk );

private:

    File                m_File;
    ReadBuffer          m_Buffer;
};

#endif      // MUSIC_FILE_READER_HPP_INCLUDED