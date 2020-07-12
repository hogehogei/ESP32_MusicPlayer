
#include "FileSystem.hpp"
#include "SDC_Drv_SPI.hpp"
#include "SD_Card.hpp"

// fatfs library diskio interface header
#include "diskio.h"
#include "ffconf.h"

// esp32 header
#include "diskio_impl.h"
#include "esp_log.h"

// static variables
static SD_Card     s_SDCard;

//
//  fatfs callbacks
//
static DSTATUS InitializeDiskCallback( uint8_t pdrv )
{
    I_SDC_Drv_SPI* spi_driver = SDC_Drv_SPI::Instance();

    if( !s_SDCard.Initialize( spi_driver ) ){
        ESP_LOGE( "Fatfs callback", "SDCard Initialization failed." );
        return STA_NODISK;
    }

    return 0;
}

static DSTATUS GetDiskStatusCallback( uint8_t pdrv )
{
    if( !s_SDCard.IsInitialized() ){
        return STA_NOINIT;
    }

    return 0;
}

static DRESULT ReadCallback( uint8_t pdrv, uint8_t* buff, uint32_t sector, uint32_t count )
{
    if( buff == nullptr ){
        return RES_PARERR;
    }
    if( count == 0 ){
        return RES_OK;
    }

    const uint32_t sector_size = SD_Card::sk_SectorSize;
    for( uint32_t read_sector_count = 0; read_sector_count < count; ++read_sector_count ){
        uint8_t* dst = buff + (sector_size * read_sector_count);

        if( !s_SDCard.Read( dst, sector, 0, SD_Card::sk_SectorSize ) ){
            ESP_LOGE( "Fatfs callback", "SDCard.Read() failed.\n" );
            return RES_ERROR;
        }
    }

    return RES_OK;
}

static DRESULT WriteCallback( uint8_t pdrv, const uint8_t* buff, uint32_t sector, uint32_t count )
{
    if( buff == nullptr || count == 0 ){
        return RES_OK;
    }
    if( !s_SDCard.WriteInitiate( sector ) ){
        ESP_LOGE( "Fatfs callback", "SDCard.WriteInitiate() failed.\n" );
        return RES_ERROR;
    }

    const uint32_t sector_size = SD_Card::sk_SectorSize;
    for( uint32_t write_sector_count = 0; write_sector_count < count; ++write_sector_count ){
        const uint8_t* src = buff + (sector_size * write_sector_count);

        if( !s_SDCard.Write( src, sector_size ) ){
            s_SDCard.WriteFinalize();
            ESP_LOGE( "Fatfs callback", "SDCard.Write() failed.\n" );
            return RES_ERROR;
        }
    }

    if( !s_SDCard.WriteFinalize() ){
        ESP_LOGE( "Fatfs callback", "SDCard.WriteFinalize() failed.\n" );
        return RES_ERROR;
    }

    return RES_OK;
}

static DRESULT IOCtlCallback( uint8_t pdrv, uint8_t cmd, void* buff )
{
    if( !s_SDCard.IsInitialized() ){
        return RES_NOTRDY;
    }
    if( !s_SDCard.State() ){
        return RES_ERROR;
    }

    DRESULT result = RES_OK;
    switch( cmd ){
    case CTRL_SYNC:
        // WriteCacheはないのでOKをそのまま返す
        result = RES_OK;
        break;
    case GET_SECTOR_COUNT:
    {
        WORD* lba = reinterpret_cast<WORD*>(buff);
        *lba = s_SDCard.SectorCount();
        result = RES_OK;
        break;
    }
    case GET_SECTOR_SIZE:
    {
        WORD* sector_size = reinterpret_cast<WORD*>(buff);
        *sector_size = SD_Card::sk_SectorSize;
        result = RES_OK;
        break;
    }
    case GET_BLOCK_SIZE:
    {
        DWORD* block_size = reinterpret_cast<DWORD*>(buff);
        *block_size = 1;                // 1を返すとunknown
        result = RES_OK;
        break;
    }
    case CTRL_TRIM:                     // TRIMは未対応
    default:
        break;
    }

    return result;
}



FileSystem::FileSystem()
{}

FileSystem::~FileSystem()
{}

FileSystem& FileSystem::Instance()
{
    static FileSystem s_FileSys;
    return s_FileSys;
}

bool FileSystem::Initialize()
{
    esp_err_t status = esp_vfs_fat_register( sk_RootDirectoryPath, sk_DriveIdentifier, 5, &m_FatFS );
    if( status != ESP_OK ){
        ESP_ERROR_CHECK( status );
        return false;
    }

    ff_diskio_impl_t diskio;
    diskio.init   = InitializeDiskCallback;
    diskio.status = GetDiskStatusCallback;
    diskio.read   = ReadCallback;
    diskio.write  = WriteCallback;
    diskio.ioctl  = IOCtlCallback;
    ff_diskio_register( m_FatFS->pdrv, &diskio );

    FRESULT result = f_mount( m_FatFS, sk_DriveIdentifier, 1 );
    if( result != FR_OK ){
        ESP_LOGE( "FileSystem", "f_mount() failed. status %x", result );
        ff_diskio_register( m_FatFS->pdrv, nullptr );
        esp_vfs_fat_unregister_path( sk_RootDirectoryPath );
        return false;
    }

    return true;
}


File::File()
:
    m_FileDiscriptor( nullptr )
{}

File::File( const std::string& filepath )
: 
    m_FilePath( filepath ),
    m_FileDiscriptor( fopen( filepath.c_str(), "rb" ), fclose )
{
    if( m_FileDiscriptor.get() ){
        ESP_LOGI( "File", "FilePath: \'%s\' open succeeded", filepath.c_str() );
    }
    else {
        ESP_LOGI( "File", "FilePath: \'%s\' open failed", filepath.c_str() ); 
    }
}

File::~File()
{}

bool File::Read( uint8_t* dst, size_t len, uint32_t* readlen )
{
    if( !IsOpened() ){
        return false;
    }
    if( IsEOF() ){
        return true;
    }
    
    *readlen = fread( dst, sizeof(uint8_t), len, m_FileDiscriptor.get() );

    return ferror( m_FileDiscriptor.get() ) == 0;
}

bool File::IsEOF() const
{
    if( !IsOpened() ){
        return true;
    }
    return feof( m_FileDiscriptor.get() ) != 0;
}

bool File::IsOpened() const
{
    return m_FileDiscriptor.get() != nullptr;
}
