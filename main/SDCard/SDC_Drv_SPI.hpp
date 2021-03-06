
#ifndef SDC_DRV_SPI_HPP_INCLUDED
#define SDC_DRV_SPI_HPP_INCLUDED

#include "I_SDC_Drv_SPI.hpp"
#include "driver/gpio.h"
#include "driver/spi_master.h"

class SDC_Drv_SPI : public I_SDC_Drv_SPI
{
public:

    // コピー代入禁止
    SDC_Drv_SPI( const SDC_Drv_SPI& ) = delete;
    SDC_Drv_SPI& operator=( const SDC_Drv_SPI& ) = delete;

    /**
     * @brief   SDカードセレクト
     **/
    virtual bool Select() override;

    /**
     * @brief   SDカード開放
     **/
    virtual void Release() override;

    /**
     * @brief   データ送信
     *          SPIでデータ送信する
     * @param [in]  data        送信データ
     * @param [in]  len         送信データ長
     * @retval  true            送信処理実行。バックグラウンドで送信する実装も考えられるので
     *                          処理完了するまで待つには flush() をコールすること。
     * @retval  false           送信処理失敗。
     **/
    virtual bool send( const uint8_t* data, uint32_t len ) override;

    /**
     * @brief   データ受信
     *          SPIでデータ受信する
     * @param [out] data        受信データの格納先
     * @param [in]  len         受信データ長
     * @retval  true            受信処理成功
     * @retval  false           受信処理失敗
     **/
    virtual bool recv( uint8_t* data, uint32_t len ) override;


    /**
     * @brief   書き込みフラッシュ
     *          書き込みバッファ等を用いた実装の場合、この関数をコールすることで
     *          書き込み待ちのデータを実際のデバイスに書き込むことを保証する。
     * @retval  true    フラッシュ成功
     * @retval  false   フラッシュ失敗　書き込み処理に失敗した
     **/
    virtual bool flush() override;

    /**
     * @brief   SPIドライバ初期化
     **/
    virtual void Initialize( uint32_t clockspeed_hz ) override;

    /**
     * @brief   SPIドライバ初期化
     **/
    static SDC_Drv_SPI* Instance();

private:

    static constexpr int sk_MISO_IONum = 27;
    static constexpr int sk_MOSI_IONum = 13;
    static constexpr int sk_SCLK_IONum = 14;
    static constexpr gpio_num_t sk_CS_IONum = GPIO_NUM_26;
    static constexpr int sk_MaxTransferSize = 1024;
    static constexpr int sk_DMAChannel = 1;

    /**
     * @brief   コンストラクタ
     **/
    SDC_Drv_SPI();

    /**
     * @brief   デストラクタ
     **/
    virtual ~SDC_Drv_SPI() noexcept override;

    static void initialize_CS();
    bool waitReady();

    spi_device_handle_t m_SPIHandle;             //!  ESP32 SPIハンドル
    bool                m_IsInitialized;         //!  初期化済みフラグ
};

#endif      // SDC_DRV_SPI_HPP_INCLUDED
