
#include "VS1053.hpp"

#include <cstdint>
#include <cstring>
#include "Timer.hpp"

// esp headers
#include "esp_log.h"

#include <esp32/rom/ets_sys.h>

static constexpr uint32_t sk_SPI_Send_Timeout = 1000;
static constexpr uint32_t sk_SPI_Recv_Timeout = 1000;
static constexpr uint32_t sk_Busy_Timeout = 1000;

VS1053_Drv_SPI VS1053_Drv_SPI::s_Driver;

VS1053_Drv_SPI::VS1053_Drv_SPI()
    : m_IsInitialized( false )
{}

VS1053_Drv_SPI::~VS1053_Drv_SPI()
{}

VS1053_Drv_SPI& VS1053_Drv_SPI::Instance()
{
    return s_Driver;
}

void VS1053_Drv_SPI::Initialize( uint32_t spi_freq )
{
    // SPI 初期化
    initialize_SPI( spi_freq );
    // CS pin 初期化
    initialize_CS( sk_CmdCS_IONum );
    CommandRelease();
	initialize_CS( sk_DataCS_IONum );
    DataRelease();
    // DREQ pin 初期化
    initialize_DReq( sk_DReq_IONum );
    // Audio Reset pin 初期化
    initialize_AudioReset( sk_AudioReset_IONum );

    m_IsInitialized = true;
}

void VS1053_Drv_SPI::ResetSPI( uint32_t spi_freq )
{
    initialize_SPI( spi_freq );
}

bool VS1053_Drv_SPI::CommandSelect()
{
    gpio_set_level( sk_CmdCS_IONum, 0 );
    return true;
}

void VS1053_Drv_SPI::CommandRelease()
{
    gpio_set_level( sk_CmdCS_IONum, 1 );
}

bool VS1053_Drv_SPI::DataSelect()
{
    gpio_set_level( sk_DataCS_IONum, 0 );
    return true;
}

void VS1053_Drv_SPI::DataRelease()
{
    gpio_set_level( sk_DataCS_IONum, 1 );
}

void VS1053_Drv_SPI::AssertReset()
{
    gpio_set_level( sk_AudioReset_IONum, 0 );
}

void VS1053_Drv_SPI::DeassertReset()
{
    gpio_set_level( sk_AudioReset_IONum, 1 );
}

bool VS1053_Drv_SPI::IsBusy() const
{
    return gpio_get_level( sk_DReq_IONum ) == 0;
}

bool VS1053_Drv_SPI::Send( const uint8_t* data, uint16_t len )
{
	static spi_transaction_t trans;
	esp_err_t ret;

	if( data == nullptr ){
		return false;
	}
	
	memset( &trans, 0, sizeof(spi_transaction_t) );
	trans.length    = len * 8;          // bit で指定
	trans.tx_buffer = data;				
	ret = spi_device_transmit( m_SPIHandle, &trans );

	return ret == ESP_OK;
}

bool VS1053_Drv_SPI::Recv( uint8_t* data, uint16_t len )
{
	static spi_transaction_t trans;

	if( data == nullptr ){
		return false;
	}

	// 受信時も 0xFF を送信するようにする。
	constexpr int k_TxBufSize = 16;
	uint8_t txbuf[k_TxBufSize] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
								  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	uint32_t remain_len = len;
	uint8_t* dstp = data;

	memset( &trans, 0, sizeof(spi_transaction_t) );

	while( remain_len >= k_TxBufSize ){
		trans.length    = k_TxBufSize * 8;
		trans.rxlength  = k_TxBufSize * 8;
		trans.tx_buffer = txbuf;
		trans.rx_buffer = dstp;
		if( spi_device_transmit( m_SPIHandle, &trans ) != ESP_OK ){
			return false;
		}
		remain_len -= k_TxBufSize;
		dstp += k_TxBufSize;
	}
	if( remain_len > 0 ){
		trans.length    = remain_len * 8;
		trans.rxlength  = remain_len * 8;
		trans.tx_buffer = txbuf;
		trans.rx_buffer = dstp;
		if( spi_device_transmit( m_SPIHandle, &trans ) != ESP_OK ){
			return false;
		}
	}

    return true;
}
    
bool VS1053_Drv_SPI::SendRecv( const uint8_t* senddata, uint8_t* recvdata, uint16_t recvlen )
{
	static spi_transaction_t trans;

	if( senddata == nullptr || recvdata == nullptr ){
		return false;
	}

	memset( &trans, 0, sizeof(spi_transaction_t) );
	trans.length    = recvlen * 8;
	trans.rxlength  = recvlen * 8;
	trans.tx_buffer = senddata;
	trans.rx_buffer = recvdata;

    return spi_device_transmit( m_SPIHandle, &trans ) == ESP_OK;
}

void VS1053_Drv_SPI::initialize_SPI( uint32_t spi_freq )
{
    spi_bus_config_t buscfg = {};
    buscfg.miso_io_num = sk_MISO_IONum;
    buscfg.mosi_io_num = sk_MOSI_IONum;
    buscfg.sclk_io_num = sk_SCLK_IONum;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.max_transfer_sz = sk_MaxTransferSize;

	spi_device_interface_config_t devcfg = {};
    devcfg.clock_speed_hz = spi_freq;             //Clock out 
    devcfg.mode = 0;                              //SPI mode 0
    devcfg.spics_io_num = -1;                     //CS pin
    devcfg.queue_size = 1;                        //We want to be able to queue 1 transactions at a time

    if( m_IsInitialized ){
        ESP_ERROR_CHECK( spi_bus_remove_device( m_SPIHandle ) );
        ESP_ERROR_CHECK( spi_bus_free( VSPI_HOST ) );
    }

	ESP_ERROR_CHECK( spi_bus_initialize( VSPI_HOST, &buscfg, sk_DMAChannel ) );
	ESP_ERROR_CHECK( spi_bus_add_device( VSPI_HOST, &devcfg, &m_SPIHandle ) );
}

void VS1053_Drv_SPI::initialize_CS( gpio_num_t ionum )
{
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = static_cast<gpio_int_type_t>(GPIO_PIN_INTR_DISABLE);
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set
    io_conf.pin_bit_mask = (1ULL << ionum);
    //disable pull-down mode
    io_conf.pull_down_en = static_cast<gpio_pulldown_t>(0);
    //disable pull-up mode
    io_conf.pull_up_en = static_cast<gpio_pullup_t>(0);
    //configure GPIO with the given settings
    gpio_config(&io_conf);
}

void VS1053_Drv_SPI::initialize_DReq( gpio_num_t ionum )
{
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = static_cast<gpio_int_type_t>(GPIO_PIN_INTR_DISABLE);
    //set as output mode
    io_conf.mode = GPIO_MODE_INPUT;
    //bit mask of the pins that you want to set
    io_conf.pin_bit_mask = (1ULL << ionum);
    //disable pull-down mode
    io_conf.pull_down_en = static_cast<gpio_pulldown_t>(0);
    //disable pull-up mode
    io_conf.pull_up_en = static_cast<gpio_pullup_t>(0);
    //configure GPIO with the given settings
    gpio_config(&io_conf);
}

void VS1053_Drv_SPI::initialize_AudioReset( gpio_num_t ionum )
{
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = static_cast<gpio_int_type_t>(GPIO_PIN_INTR_DISABLE);
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set
    io_conf.pin_bit_mask = (1ULL << ionum);
    //disable pull-down mode
    io_conf.pull_down_en = static_cast<gpio_pulldown_t>(0);
    //disable pull-up mode
    io_conf.pull_up_en = static_cast<gpio_pullup_t>(0);
    //configure GPIO with the given settings
    gpio_config(&io_conf);
}





VS1053_Drv::VS1053_Drv()
    : m_IsInitialized( false )
{}

VS1053_Drv::~VS1053_Drv()
{}

VS1053_Drv& VS1053_Drv::Instance()
{
    static VS1053_Drv s_Instance;
    return s_Instance;
}

bool VS1053_Drv::Initialize()
{
    m_IsInitialized = false;

    VS1053_Drv_SPI& driver = VS1053_Drv_SPI::Instance();

    // VS1053bをRESETピンでハードウェアリセット
    driver.AssertReset();
    ets_delay_us( 100 * 1000 );

    // 初期化時のSPI速度で設定
    driver.Initialize( sk_SPIClockSpeedInitialize_Hz );
    driver.DeassertReset();

    constexpr uint16_t k_Status_WhenInitCompleted = 0x0040;
    uint16_t status = 0x0000;
    {
        MsTimer timer;

        while( status != k_Status_WhenInitCompleted ){
            if( !ReadSCI( SCI_STATUS, &status ) ){
                break;
            }
            ESP_LOGI("VS1053_Drv", "Initialize status %x", status);
            if( timer.IsElapsed( sk_Busy_Timeout ) ){
                return false;
            }
        }
    }
    
    if( status != k_Status_WhenInitCompleted ){
        return false;
    }
    
    // クロックのセット
    if( !setClock() ){
        return false;
    }
    // デフォルトのボリュームをセット
    // 初期値は最大ボリュームなのである程度静かにしておく
    if( !SetVolume( sk_DefaultVolume, sk_DefaultVolume ) ){
        return false;
    }

    if( !WriteSCI( SCI_BASS, 0x00FF ) ){
        return false;
    }

    // SPIクロックを増やしたので、本来の速度に再設定
    driver.ResetSPI( sk_SPIClockSpeed_Hz );
    m_IsInitialized = true;

    ESP_LOGI("VS1053_Drv", "Initialize completed!");

    return true;
}

bool VS1053_Drv::ReadSCI( SCI_Register regaddr, uint16_t* data )
{
    VS1053_Drv_SPI& driver = VS1053_Drv_SPI::Instance();
    {
        MsTimer timer;
        while( driver.IsBusy() ){
            if( timer.IsElapsed( sk_Busy_Timeout ) ){
                return false;
            }
        }
    }

    const uint8_t senddata[4] = { sk_SCI_ReadInstruction, regaddr, 0xFF, 0xFF };
    uint8_t recvdata[4];
    bool is_command_send_ok = false;

    driver.CommandSelect();
    is_command_send_ok = driver.SendRecv( senddata, recvdata, 4 );
    driver.CommandRelease();

    if( is_command_send_ok ){
        *data = (recvdata[2] << 8) | recvdata[3];
    }

    return is_command_send_ok;
}

bool VS1053_Drv::WriteSCI( SCI_Register regaddr, uint16_t data )
{
    VS1053_Drv_SPI& driver = VS1053_Drv_SPI::Instance();
    {
        MsTimer timer;
        while( driver.IsBusy() ){
            if( timer.IsElapsed( sk_Busy_Timeout ) ){
                return false;
            }
        }
    }

    const uint8_t senddata[4] = { sk_SCI_WriteInstruction, regaddr, static_cast<uint8_t>(data >> 8), static_cast<uint8_t>(data) };

    bool is_command_send_ok = false;

    driver.CommandSelect();
    is_command_send_ok = driver.Send( senddata, 4 );
    driver.CommandRelease();

    return is_command_send_ok;
}

bool VS1053_Drv::WriteSDI( const uint8_t* data, uint16_t len )
{
    VS1053_Drv_SPI& driver = VS1053_Drv_SPI::Instance();
    {
        MsTimer timer;
        while( driver.IsBusy() ){
            if( timer.IsElapsed( sk_Busy_Timeout ) ){
                return false;
            }
        }
    }

    bool is_data_send_ok = false;

    driver.DataSelect();
    is_data_send_ok = driver.Send( data, len );
    driver.DataRelease();

    return is_data_send_ok;
}

uint16_t VS1053_Drv::ReadWRAM( uint16_t addr )
{
    uint16_t wram = 0;
    WriteSCI( SCI_WRAMADDR, addr );
    ReadSCI( SCI_WRAM, &wram );

    return wram;
}

bool VS1053_Drv::IsBusy() const
{
    return VS1053_Drv_SPI::Instance().IsBusy();
}

bool VS1053_Drv::SetVolume( uint8_t left, uint8_t right )
{
    uint16_t volume = (static_cast<uint16_t>(left) << 8) | (static_cast<uint16_t>(right) & 0x00FF);
    return WriteSCI( SCI_VOL, volume );
}

uint8_t VS1053_Drv::ReadEndFillByte()
{
    return (ReadWRAM( sk_EndFillByte_Address ) & 0xFF);
}

void VS1053_Drv::SoftReset()
{

}

bool VS1053_Drv::setClock()
{
    return WriteSCI( SCI_CLOCKF, 0x8800 );
}