
#ifndef SD_CARD_HPP_INCLUDED
#define SD_CARD_HPP_INCLUDED

#include "I_SDC_Drv_SPI.hpp"

/**
 * @brief   SDカード読み書きクラス
 *          SDカードの読み書きを実施する。
 *
 *          読み書きのためのバッファを最小限にしていることが特徴。
 *          そのため、読み書きの速度を犠牲にしていたり、読み書き手順が少し煩雑。
 *          petit_fs を使うことを想定しているため、上位でバッファを持たせるなどして煩雑さをラップする思想。
 **/
class SD_Card
{
public:
    
    /**
     * @brief   コンストラクタ
     **/
    SD_Card();

    /**
     * @brief   デストラクタ
     **/
    ~SD_Card() noexcept;


    /**
     * @brief   SDカード初期化
     * @param [in]  driver      SDカードインターフェース
     *                          このクラスではメモリ解放責務は負わない。
     * @retval  true    初期化成功
     * @ratval  false   初期化失敗
     **/
    bool Initialize( I_SDC_Drv_SPI* driver );

    /**
     * @brief   SDカード 読み込み
     * @param [out] dst         書き込み先
     * @param [in]  sector      読み出すセクタ開始位置
     * @param [in]  offset      読み込み位置offset[byte]
     * @param [in]  len         読み込みデータ長[byte]
     * @retval  true    読み出し成功
     * @ratval  false   読み出し失敗
     **/
    bool Read( uint8_t* dst, uint32_t sector, uint32_t offset, uint32_t len );

    /**
     * @brief   SDカード書き込み開始
     *          Write() で必要なデータを書き込み、WriteFinalize() をコールすることで書き込みが完了する。
     * @param [in]  sector      書き込みセクタ開始位置
     * @retval  true    処理成功
     * @retval  false   処理失敗
     **/
    bool WriteInitiate( uint32_t sector );

    /**
     * @brief   SDカード書き込み
     * @param [in]  data        書き込みデータへのポインタ
     * @param [in]  len         書き込みデータ長[byte]
     * @retval  true    処理成功
     * @retval  false   処理失敗
     **/
    bool Write( const uint8_t* data, uint32_t len );

    /**
     * @brief   書き込み終了処理
     * @retval  true    処理成功
     * @retval  false   処理失敗
     **/
    bool WriteFinalize();

    /**
     * @brief   状態取得
     * @retval  true    読み書きが可能
     * @retval  false   状態が不正なため読み書きできない。
     **/
    bool State() const;

    /**
     * @brief   初期化済みか調べる
     * @retval  true    初期化済み
     * @retvat  false   初期化に失敗している
     **/
    bool IsInitialized() const;

    /**
     * @brief   セクタ数を取得する
     * @retval  Openしたメディアのセクタ数。アクセスできない場合は0を返す
     **/
    uint32_t SectorCount() const;

    static constexpr uint32_t sk_SectorSize         = 512;      //! セクタサイズ

private:

    static constexpr uint32_t sk_ClockSpeedHz_Initializing = 200000;         // 200kHz
    static constexpr uint32_t sk_ClockSpeedHz_AccessSD     = 10000000;       // 10MHz
    class Progress
    {
    public:
        Progress();
        uint32_t addr;
        uint32_t remain_offset;
        uint32_t remain_sector;
        uint32_t remain_bytes_cursec;
        uint32_t remain_len;
    };


    void sendInitializeClock();
    uint8_t initialize_Card();
    uint8_t initialize_SDv2();
    uint8_t initialize_SDv1_or_MMCv3();
    void printSDCardInfo();
    void readSectorSize();
    uint8_t sendCmd( uint8_t cmd, uint32_t arg );
    uint8_t sendCmdRetry( uint8_t cmd, uint32_t arg, uint32_t retry_cnt );

    bool readInitiate( Progress* progress, uint32_t sector, uint32_t offset, uint32_t len );
    bool nextSectorReadPreparation( Progress* progress );
    bool waitReadDataPacket();
    bool nextSectorWritePreparation();
    void advanceNextSector( Progress* progress );

    uint8_t recvByte();
    void busyWait();
    void ignoreRead( uint32_t cnt );
    void sendAnyData( uint32_t cnt );

    I_SDC_Drv_SPI*  m_SDC_Drv;                  //! SDカード通信ドライバ
    Progress        m_W_Progress;               //! 書き込み処理 進捗管理
    bool            m_SDC_State;
    bool            m_IsWriteOpeProcessing;     //! 書き込み処理開始フラグ
    uint8_t         m_CardType;
    uint32_t        m_SectorCount;              //! SDカードのセクタ数
};

#endif      // SD_CARD_HPP_INCLUDED
