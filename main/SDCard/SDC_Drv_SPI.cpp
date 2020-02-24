#include "SDC_Drv_SPI.hpp"

#include <limits>
#include <cstring>

//
// esp-idf headers
//
#include "esp_err.h"
#include "driver/gpio.h"

static constexpr int sk_SPI_Timeout_Ms = 1000;

SDC_Drv_SPI::SDC_Drv_SPI()
   : m_IsInitialized( false )
{}

SDC_Drv_SPI::~SDC_Drv_SPI()
{}

bool SDC_Drv_SPI::Select()
{
    gpio_set_level( sk_CS_IONum, 0 );

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
    gpio_set_level( sk_CS_IONum, 1 );

    // MMC/SDC の場合は SCLKに同期して DO信号の解放が行われる。
    // DO信号を確実に解放するために、1byte分クロックを送っておく。
	uint8_t tmp = 0xFF;
	send( &tmp, 1 );
}

bool SDC_Drv_SPI::send( const uint8_t* data, uint32_t len )
{
	static spi_transaction_t trans;
	esp_err_t ret;

	// 送信できる最大長より長いサイズは送れない
	if( len > std::numeric_limits<uint32_t>::max() / 8 ){
		return false;
	}
	if( data == nullptr ){
		return false;
	}
	
	memset( &trans, 0, sizeof(spi_transaction_t) );
	trans.length    = len * 8;
	trans.tx_buffer = data;				// bit で指定
	ret = spi_device_transmit( m_SPIHandle, &trans );

	return ret == ESP_OK;
}

bool SDC_Drv_SPI::recv( uint8_t* data, uint32_t len )
{
	static spi_transaction_t trans;

	// 送信できる最大長より長いサイズは送れない
	if( len > std::numeric_limits<uint32_t>::max() / 8 ){
		return false;
	}
	if( data == nullptr ){
		return false;
	}

	// 受信時も 0xFF を送信するようにする。
	// そうでないとSDカードが正しくコマンドのレスポンスを返してくれない。
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
		dstp += 16;
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

bool SDC_Drv_SPI::flush()
{
    // 書き込みバッファは持たないので、特に何もしない
    return true;
}

void SDC_Drv_SPI::Initialize( uint32_t clockspeed_hz )
{
	// 初期化済みなら一旦削除
	if( m_IsInitialized ){
		ESP_ERROR_CHECK( spi_bus_remove_device( m_SPIHandle ) );
	}

    spi_bus_config_t buscfg = {};
    buscfg.miso_io_num = sk_MISO_IONum;
    buscfg.mosi_io_num = sk_MOSI_IONum;
    buscfg.sclk_io_num = sk_SCLK_IONum;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.max_transfer_sz = sk_MaxTransferSize;

	spi_device_interface_config_t devcfg = {};
    devcfg.clock_speed_hz = clockspeed_hz;        //Set clockspeed
    devcfg.mode = 0;                              //SPI mode 0
    devcfg.spics_io_num = -1;                     //CS pin
    devcfg.queue_size = 1;                        //We want to be able to queue 1 transactions at a time

	ESP_ERROR_CHECK( spi_bus_initialize( HSPI_HOST, &buscfg, sk_DMAChannel ) );
	ESP_ERROR_CHECK( spi_bus_add_device( HSPI_HOST, &devcfg, &m_SPIHandle ) );

	// CS pin 初期化
	initialize_CS();

	m_IsInitialized = true;
}

bool SDC_Drv_SPI::waitReady()
{
	uint8_t tmp = 0;
	int64_t time = esp_timer_get_time();

	while(1){	
		recv( &tmp, 1 );
		if( tmp == 0xFF ){
			break;
		}
		int64_t nowtime = esp_timer_get_time();
		if( nowtime - time > sk_SPI_Timeout_Ms ){
			break;
		}
	}

	return tmp == 0xFF;
}

void SDC_Drv_SPI::initialize_CS()
{
	gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = static_cast<gpio_int_type_t>(GPIO_PIN_INTR_DISABLE);
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set
    io_conf.pin_bit_mask = (1ULL << sk_CS_IONum);
    //disable pull-down mode
    io_conf.pull_down_en = static_cast<gpio_pulldown_t>(0);
    //disable pull-up mode
    io_conf.pull_up_en = static_cast<gpio_pullup_t>(0);
    //configure GPIO with the given settings
    gpio_config(&io_conf);
}
