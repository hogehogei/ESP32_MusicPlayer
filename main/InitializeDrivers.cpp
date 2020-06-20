
#include "InitializeDrivers.hpp"
#include "SDCard/FileSystem.hpp"
#include "Audio/VS1053.hpp"

extern "C"
{

void InitializeDrivers()
{
    // SDカードファイルシステム初期化
    printf( "SDC FileSystem initialize.\n" );
    FileSystem::Instance().Initialize();
    // VS1053 ドライバ初期化
    printf( "VS1053 driver initialize.\n" );
    VS1053_Drv::Instance().Initialize();
}

}