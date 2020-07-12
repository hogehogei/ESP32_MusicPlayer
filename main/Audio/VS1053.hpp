#ifndef VS1053_HPP_INCLUDED
#define VS1053_HPP_INCLUDED

#include <cstdint>

//
//    esp-idf headers
//
#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"


class VS1053_Drv_SPI
{
public:

    ~VS1053_Drv_SPI();
    VS1053_Drv_SPI& operator=( const VS1053_Drv_SPI& ) = delete;
    VS1053_Drv_SPI( const VS1053_Drv_SPI& ) = delete;

    static VS1053_Drv_SPI& Instance();

    void Initialize( uint32_t spi_freq );
    void ResetSPI( uint32_t spi_freq );
    bool CommandSelect();
    void CommandRelease();
    bool DataSelect();
    void DataRelease();
    void AssertReset();
    void DeassertReset();

    bool IsBusy() const;

    bool Send( const uint8_t* data, uint16_t len );
    bool Recv( uint8_t* data, uint16_t len );
    bool SendRecv( const uint8_t* senddata, uint8_t* recvdata, uint16_t recvlen );

private:


    static constexpr int sk_MISO_IONum = 19;
    static constexpr int sk_MOSI_IONum = 23;
    static constexpr int sk_SCLK_IONum = 18;
    static constexpr gpio_num_t sk_CmdCS_IONum  = GPIO_NUM_4;
    static constexpr gpio_num_t sk_DataCS_IONum = GPIO_NUM_16;
    static constexpr gpio_num_t sk_DReq_IONum = GPIO_NUM_17;
    static constexpr gpio_num_t sk_AudioReset_IONum = GPIO_NUM_5;
    static constexpr int sk_MaxTransferSize = 1024;
    static constexpr int sk_DMAChannel = 2;

    static VS1053_Drv_SPI s_Driver;

    VS1053_Drv_SPI();

    void initialize_SPI( uint32_t spi_freq );
    void initialize_CS( gpio_num_t ionum );
    void initialize_DReq( gpio_num_t ionum );
    void initialize_AudioReset( gpio_num_t ionum );

    spi_device_handle_t m_SPIHandle;             //!  ESP32 SPIハンドル
};

class VS1053_Drv
{
public:

    enum SCI_Register
    {
        SCI_MODE        = 0x00,
        SCI_STATUS      = 0x01,
        SCI_BASS        = 0x02,
        SCI_CLOCKF      = 0x03,
        SCI_DECODE_TIME = 0x04,
        SCI_AUDATA      = 0x05,
        SCI_WRAM        = 0x06,
        SCI_WRAMADDR    = 0x07,
        SCI_HDAT0       = 0x08,
        SCI_HDAT1       = 0x09,
        SCI_AIADDR      = 0x0A,
        SCI_VOL         = 0x0B,
        SCI_AICTRL0     = 0x0C,
        SCI_AICTRL1     = 0x0D,
        SCI_AICTRL2     = 0x0E,
        SCI_AICTRL3     = 0x0F
    };

    static constexpr uint8_t sk_DefaultVolume = 80;

public:

    ~VS1053_Drv() noexcept;

    VS1053_Drv( const VS1053_Drv& ) = delete;
    VS1053_Drv& operator=( const VS1053_Drv& ) = delete;

    static VS1053_Drv& Instance();

    bool Initialize();
    bool ReadSCI( SCI_Register regaddr, uint16_t* data );
    bool WriteSCI( SCI_Register regaddr, uint16_t data );
    bool WriteSDI( const uint8_t* data, uint16_t len );
    uint16_t ReadWRAM( uint16_t addr );

    bool IsBusy() const;
    bool SetVolume( uint8_t left, uint8_t right );
    uint8_t ReadEndFillByte();

    void SoftReset();
    
private:

    VS1053_Drv();
    
    bool setClock();

    static constexpr int sk_SPIClockSpeed_Hz = 6000000;             // 6MHz
    static constexpr int sk_SPIClockSpeedInitialize_Hz = 1000000;   // 1MHz
    static constexpr uint8_t sk_SCI_ReadInstruction  = 0x03;
    static constexpr uint8_t sk_SCI_WriteInstruction = 0x02;
    static constexpr uint16_t sk_EndFillByte_Address = 0x1E06;

    bool m_IsInitialized;
};

#endif    // VS1053_HPP_INCLUDED
