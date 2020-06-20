
#ifndef     FILE_SYSTEM_HPP_INCLUDED
#define     FILE_SYSTEM_HPP_INCLUDED

#include <memory>

//
// esp-idf headers
//
#include "esp_vfs_fat.h"

#include "PlayList.hpp"

class FileSystem
{
public:

    ~FileSystem() noexcept;
    static FileSystem& Instance();
    
    bool Initialize();

    static inline constexpr char sk_RootDirectoryPath[] = "/root";
    static inline constexpr char sk_DriveIdentifier[] = "Primary";

private:

    FileSystem();
    FileSystem( const FileSystem& );
    FileSystem& operator=( const FileSystem& );

    FATFS* m_FatFS;
};

class File
{
public:
    
    File();
    File( const std::string& filepath );
    ~File() noexcept;

    bool Read( uint8_t* dst, size_t len, uint32_t* readlen );
    bool IsEOF() const;
    bool IsOpened() const;

private:

    std::shared_ptr<FILE> m_FileDiscriptor;
};

#endif      // FILE_SYSTEM_HPP_INCLUDED