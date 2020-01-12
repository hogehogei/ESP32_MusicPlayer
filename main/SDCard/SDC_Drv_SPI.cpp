#include "sdcard/SDC_Drv_SPI.hpp"

static constexpr uint32_t sk_SPI_Timeout = 1000;

SDC_Drv_SPI::SDC_Drv_SPI()
{}

SDC_Drv_SPI::~SDC_Drv_SPI()
{}

bool SDC_Drv_SPI::Select()
{
    HAL_GPIO_WritePin( GPIOA, SDCARD_CS_Pin, GPIO_PIN_RESET );

	uint8_t tmp = 0xFF;
    if( !send( &tmp, 1 ) ){
		return false;
	}
	if( waitReady() ){
		return true;
	}
	
	Release();
	return false;
}

void SDC_Drv_SPI::Release()
{
    HAL_GPIO_WritePin( GPIOA, SDCARD_CS_Pin, GPIO_PIN_SET );

    // MMC/SDC の場合は SCLKに同期して DO信号の解放が行われる。
    // DO信号を確実に解放するために、1byte分クロックを送っておく。
	uint8_t tmp = 0xFF;
	send( &tmp, 1 );
}

bool SDC_Drv_SPI::send( const uint8_t* data, uint32_t len )
{
    HAL_StatusTypeDef status = HAL_SPI_Transmit( &hspi1, const_cast<uint8_t*>(data), static_cast<uint16_t>(len), sk_SPI_Timeout );
    return status == HAL_OK;
}

bool SDC_Drv_SPI::recv( uint8_t* data, uint32_t len )
{
	// HAL_SPI_Receive 関数では、0x00を送信してしまうため正常にコマンド送信ができない。
	// 受信時も0xFFを送信するように HAL_SPI_TransmitReceive を使う。
	constexpr int k_TxBufSize = 16;
	uint8_t txbuf[k_TxBufSize] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
								  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	uint32_t remain_len = len;
	uint8_t* dstp = data;

	while( remain_len >= k_TxBufSize ){
		if( HAL_SPI_TransmitReceive( &hspi1, txbuf, dstp, k_TxBufSize, sk_SPI_Timeout ) != HAL_OK ){
			return false;
		}
		remain_len -= k_TxBufSize;
		dstp += 16;
	}
	if( remain_len > 0 ){
		if( HAL_SPI_TransmitReceive( &hspi1, txbuf, dstp, static_cast<uint16_t>(remain_len), sk_SPI_Timeout ) != HAL_OK ){ 
			return false;
		}
	}

    return true;
}

bool SDC_Drv_SPI::flush()
{
    // 書き込みバッファは持たないので、特に何もしない
    return true;
}

void SDC_Drv_SPI::Initialize( uint32_t clockspeed_hz )
{
    esp_err_t ret;
    spi_device_handle_t spi;
    spi_bus_config_t buscfg = {
        .miso_io_num = sk_MISO_IONum,
        .mosi_io_num = sk_MOSI_IONum,
        .sclk_io_num = sk_SCLK_IONum,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = sk_MaxTransferSize;
    };

	spi_device_interface_config_t devcfg = {
        .clock_speed_hz = clockspeed,           //Clock out at 26 MHz
        .mode = 0,                              //SPI mode 0
        .spics_io_num = -1,                     //CS pin
        .queue_size = 4,                        //We want to be able to queue 7 transactions at a time
    };

	ret = spi_bus_initialize( HSPI_HOST, &buscfg, sk_DMAChannel );
	ESP_ERROR_CHECK( ret );
	ret = spi_bus_add_device( HSPI_HOST, &devcfg, &spi );
	ESP_ERROR_CHECK( ret );

	// CS pin 初期化
	initialize_CS();
}

bool SDC_Drv_SPI::waitReady()
{
	uint8_t tmp = 0;
	MsTimer timer;

	while(1){	
		recv( &tmp, 1 );
		if( tmp == 0xFF ){
			break;
		}
		if( timer.IsElapsed( sk_SPI_Timeout ) ){
			break;
		}
	}

	return tmp == 0xFF;
}
