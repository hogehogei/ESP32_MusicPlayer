
#include "SD_Card.hpp"

#include <limits>
#include <algorithm>

// esp-idf headers
#include "esp_log.h"

// FreeRTOS headers
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// SDCard Command Set
static constexpr uint8_t sk_CMD0        = (0x40 + 0);
static constexpr uint8_t sk_CMD1        = (0x40 + 1);
static constexpr uint8_t sk_CMD8        = (0x40 + 8);
static constexpr uint8_t sk_CMD9        = (0x40 + 9);
static constexpr uint8_t sk_CMD16       = (0x40 + 16);
static constexpr uint8_t sk_CMD17       = (0x40 + 17);
static constexpr uint8_t sk_CMD25       = (0x40 + 25);
static constexpr uint8_t sk_ACMD41      = (0xC0 + 41);
static constexpr uint8_t sk_CMD55       = (0x40 + 55);
static constexpr uint8_t sk_CMD58       = (0x40 + 58);
static constexpr uint8_t sk_ACMD_Mask   = (0x80);                   //! ACMDマスク値 コマンドにANDして1の場合はACMD
static constexpr uint32_t sk_ACMD41_HCS = (1UL << 30);
static constexpr uint32_t sk_ACMD58_CCS = (0x40);

// SDCard Command Response
static constexpr uint8_t sk_CMD_RES_InIdleState = (1 << 0);
static constexpr uint8_t sk_CMD_RES_OK  = 0x00;

// SDCard Data Token
static constexpr uint8_t sk_ErrorTokenMask     = 0x01;              //! エラートークンマスク
static constexpr uint8_t sk_DataToken_Read     = 0xFE;              //! CMD17 データトークン
static constexpr uint8_t sk_DataToken_CMD25    = 0xFC;              //! CMD25 データトークン
static constexpr uint8_t sk_StopTransToken_CMD25 = 0xFD;            //! CMD25 StopTrans

// SDCard Data Response
static constexpr uint8_t sk_DATA_RES_OK_Mask = 0x1F;
static constexpr uint8_t sk_DATA_RES_OK      = 0x05;

// SDCard Type
static constexpr uint8_t sk_SDC_Type_None   = 0;                    //! SDカードタイプ無し。初期化に失敗した時とかはこれ
static constexpr uint8_t sk_SDC_Type_SD1    = (1 << 0);             //! SDカード = SDv1
static constexpr uint8_t sk_SDC_Type_SD2    = (1 << 1);             //! SDカード = SDv2
static constexpr uint8_t sk_SDC_Type_MMC    = (1 << 2);             //! SDカード = MMC
static constexpr uint8_t sk_SDC_Block       = (1 << 3);             //! ブロックアクセス     1の場合はコマンドのアドレス指定はsector単位を用いる。
                                                                    //!                     0の場合はコマンドのアドレス指定はbyte単位。

// read() / write() 関数の引数 sector を byte に変換するときに
// sector がとりうることのできる最大値
static constexpr uint32_t k_MaxSector_ByteAccess = (std::numeric_limits<uint32_t>::max() / SD_Card::sk_SectorSize);

// SDカードアクセス時間指定

static constexpr uint8_t sk_SDC_InitDelay_ms    = 100;              //! SDC初期化ウェイト[ms]
static constexpr int sk_SDC_InitSCLK            = 10;               //! SDC初期化カウント[x8 clock]
static constexpr uint16_t sk_SDC_TimeOut_ms     = 1000;             //! SDCコマンドタイムアウト[ms]

static constexpr int sk_CMD_RespRetry           = 8192;             //! コマンドレスポンス読み出しリトライ回数
static constexpr int sk_DataPktReadWait         = 8192;             //! データパケットトークン読み出しリトライ回数
static constexpr int sk_WriteBusyCheckRetry     = 8192;             //! 書き込みビジーチェックリトライ回数

void showResp( const uint8_t resp );

SD_Card::Progress::Progress()
 :  addr( 0 ),
    remain_offset( 0 ),
    remain_sector( 0 ),
    remain_bytes_cursec( 0 ),
    remain_len( 0 )
{}

SD_Card::SD_Card()
 : m_SDC_Drv( nullptr ),
   m_W_Progress(),
   m_SDC_State( false ),
   m_IsWriteOpeProcessing( false ),
   m_CardType( 0 ),
   m_SectorCount( 0 )
{}

SD_Card::~SD_Card()
{}


bool SD_Card::Initialize( I_SDC_Drv_SPI* driver )
{
    // 初期化済み、かつ正常動作中なら何もしない。
    if( m_SDC_State == true ){
        return true;
    }

    // ドライバインターフェースを設定
    if( !driver ){
        m_SDC_State = false;
        return false;
    }
    m_SDC_Drv = driver;

    // SDカードドライバ初期化
    m_SDC_Drv->Initialize( sk_ClockSpeedHz_Initializing );
    sendInitializeClock();              // 初期化クロック送信
    m_CardType = initialize_Card();     // カード初期化
    // SPIの速度再設定
    m_SDC_Drv->Initialize( sk_ClockSpeedHz_AccessSD );
    
    if( m_CardType == sk_SDC_Type_None ){
        m_SDC_State = false;
    }
    else {
        // セクタサイズ読み込み
        readSectorSize();
        m_SDC_State = true;
    }

    // 初期化終了
    m_SDC_Drv->Release();

    printSDCardInfo();

    return m_SDC_State;
}

bool SD_Card::Read( uint8_t* dst, uint32_t sector, uint32_t offset, uint32_t len )
{
	// SDカード状態が不正ならエラー
	if( m_SDC_State == false ){
		return false;
	}

    if( len == 0 ){
        return true;
    }
    // 引数チェック
    // offset がセクタサイズ以上、もしくは
    // (offset + len) が32bitの取りえる値を超えていたら失敗
    if( dst == nullptr ||
        offset >= sk_SectorSize || 
        ((std::numeric_limits<uint32_t>::max() - offset) < len) ){
        return false;
    }
    // アクセス単位がbyte単位の場合に、上限値を超えたらエラー
    if( !(m_CardType & sk_SDC_Block) &&
        (sector > k_MaxSector_ByteAccess) ){
        return false;
    }

    // 進捗管理構造体
    Progress progress;
    bool result = readInitiate( &progress, sector, offset, len );

    // 3つの場合に分けて考える
    // step1: offset だけ先頭データを読み飛ばす
    // step2: 必要なデータをコピーする
    // step3: 必要なデータをすべて読み終わった後、最終セクタの余ったデータを読み飛ばす
    while( result &&
         (( progress.remain_len > 0 ) || ( progress.remain_bytes_cursec > 0 )) ){
#if 0
    	UART_Print( "Remain offset");
    	UART_HexPrint( (uint8_t* )&progress.remain_offset, 4 );
    	UART_Print( "Remain cursec");
    	UART_HexPrint( (uint8_t*)&progress.remain_bytes_cursec, 4 );
    	UART_Print( "Remain len");
    	UART_HexPrint( (uint8_t*)&progress.remain_len, 4 );
    	UART_Print( "Remain Sector");
    	UART_HexPrint( (uint8_t*)&progress.remain_sector, 4 );
#endif

        if( progress.remain_offset > 0 ){
            // step1
            ignoreRead( offset );
            progress.remain_bytes_cursec -= offset;
            progress.remain_offset = 0;
        }
        else if( progress.remain_len > 0 ){
            // step2
            uint32_t size = std::min( progress.remain_len, progress.remain_bytes_cursec );
            result = m_SDC_Drv->recv( dst, size );
            dst += size;
            progress.remain_len -= size;
            progress.remain_bytes_cursec -= size;
        }
        else if( progress.remain_bytes_cursec > 0 ){
            // step3 
            ignoreRead( progress.remain_bytes_cursec );
            progress.remain_bytes_cursec = 0;
        }
        else {
            result = false;
        }

        if( progress.remain_bytes_cursec == 0 ){
            // CRCを読み飛ばす
            ignoreRead( 2 );
            // 残りのセクタを読む必要があるなら次のセクタを読む
            if( progress.remain_sector > 0 ){
                if( !nextSectorReadPreparation( &progress ) ){
                    result = false;
                    break;
                }
            }
        }
    }

    m_SDC_Drv->Release();
    m_SDC_State = result;

    return result;
}

bool SD_Card::WriteInitiate( uint32_t sector )
{
    // SDカード状態が不正
    // 前回書き込み処理が完了していなければエラー
    if( m_SDC_State == false ||
        m_IsWriteOpeProcessing == true ){
        return false;
    }

    // アクセス単位がbyte単位の場合に、上限値を超えたらエラー
    if( (m_CardType & sk_SDC_Block) == 0 &&
        (sector > k_MaxSector_ByteAccess) ){
        return false;
    }

    bool result = false;

    // 開始アドレスを算出
    uint32_t addr = m_CardType & sk_SDC_Block ? sector : sector * sk_SectorSize;

    m_SDC_Drv->Select();

    if( sendCmd( sk_CMD25, addr ) == sk_CMD_RES_OK ){
        m_W_Progress.addr                = addr;
        m_W_Progress.remain_sector       = 0;
        m_W_Progress.remain_bytes_cursec = sk_SectorSize;
        m_W_Progress.remain_len          = 0;

        // 1byte以上待つ
        ignoreRead( 1 );

        m_IsWriteOpeProcessing = true;
        result = true;
    }

    m_SDC_Drv->Release();
    m_SDC_State = result;

    return result;
}

bool SD_Card::Write( const uint8_t* data, uint32_t len )
{
    // SDカード状態が不正
    // 書き込むデータがnullptr、もしくは WriteInitiate() を呼んでいなければ失敗
    if( m_SDC_State == false ||
        data == nullptr ||
        m_IsWriteOpeProcessing == false ){
        return false;
    }

    // 書き込み長が0なら何もせず成功で終了
    if( len == 0 ){
        return true;
    }

    bool result = true;             // 戻り値
    const uint8_t* writep = data;   // 書き込みデータポインタ
    uint8_t datatok = sk_DataToken_CMD25;

    m_W_Progress.remain_len = len;

    m_SDC_Drv->Select();

    // 書き込みが失敗するか、すべて書き込むまで
    while( result && m_W_Progress.remain_len > 0 ){
        // 今回のセクタに初回書き込みの場合は、先にDataTokenを送信
        if( m_W_Progress.remain_bytes_cursec == sk_SectorSize ){
            result = m_SDC_Drv->send( &datatok, 1 );
        }

        uint8_t size = std::min( m_W_Progress.remain_len, m_W_Progress.remain_bytes_cursec );
        result = m_SDC_Drv->send( writep, size );
        writep += size;
        m_W_Progress.remain_len -= size;
        m_W_Progress.remain_bytes_cursec -= size;

        // 今回のセクタのデータを書き終わったら、次のセクタに書く
        if( result && m_W_Progress.remain_bytes_cursec == 0 ){
            if( !nextSectorWritePreparation() ){
                result = false;
                break;
            }

            m_W_Progress.remain_bytes_cursec = sk_SectorSize;
        }
    }

    m_SDC_Drv->Release();
    m_SDC_State = result;

    return result;
}

bool SD_Card::WriteFinalize()
{
    // 書き込み処理が開始されていなければエラー
    if( m_SDC_State == false ||
        m_IsWriteOpeProcessing == false ){
        return false;
    }

    bool result = true;             // 戻り値
    uint8_t stoptrans = sk_StopTransToken_CMD25;

    m_SDC_Drv->Select();
    // 現在セクタの残りスペースを0埋めする
    sendAnyData( m_W_Progress.remain_bytes_cursec );
    // StopTransToken 送信後、1byte読み飛ばして ビジー解除まで待つ
    m_SDC_Drv->send( &stoptrans, 1 );
    ignoreRead( 1 );
    busyWait();
    m_SDC_Drv->Release();

    // 書き込みフラッシュ完了したので書き込み処理中フラグを下ろす
    m_W_Progress.remain_bytes_cursec = 0;
    m_IsWriteOpeProcessing = false;

    return result;
}

bool SD_Card::State() const
{
    return m_SDC_State;
}

bool SD_Card::IsInitialized() const
{
    return m_CardType != sk_SDC_Type_None;
}

uint32_t SD_Card::SectorCount() const
{
    return m_SectorCount;
}

/**
 * @brief   初期化クロック送信
 *          電源電圧が規定の範囲(2.7～3.6V)に達したあと少なくとも1ms待ち、
 *          DI,CSをHレベルにしてSCLKを74クロック以上入れるとコマンドを受け付ける準備ができます。
 **/
void SD_Card::sendInitializeClock()
{
    // CSをHレベルに設定
    m_SDC_Drv->Release();
    // DIはハードでプルアップしているので処理なし

    // 1ms以上待つ
    const TickType_t xDelayMs = 10 / portTICK_PERIOD_MS;
    vTaskDelay( xDelayMs );

    for( int i = 0; i < sk_SDC_InitSCLK; ++i ){
        // SPI 初期化クロック送信
        uint8_t t = 0xFF;
        m_SDC_Drv->send( &t, 1 );
    }
}

/**
 * @brief   カード初期化
 * @return  type    初期化時に特定したカードタイプのビットをたてて返す。
 *                  sk_SDC_Type_None   = 0;                    SDカードタイプ無し。初期化に失敗した時とかはこれ
 *                  sk_SDC_Type_SD1    = (1 << 0);             SDカード = SDv1
 *                  sk_SDC_Type_SD2    = (1 << 1);             SDカード = SDv2
 *                  sk_SDC_Type_MMC    = (1 << 2);             SDカード = MMC
 *                  sk_SDC_Block       = (1 << 3);             ブロックアクセス     1の場合はコマンドのアドレス指定はsector単位を用いる。
 *                                                                                 0の場合はコマンドのアドレス指定はbyte単位。
 **/
uint8_t SD_Card::initialize_Card()
{
    // 初期化開始

    uint8_t type = sk_SDC_Type_None;
    // CMD0 初期化コマンド送信
    if( sendCmd( sk_CMD0, 0 ) == sk_CMD_RES_InIdleState ){
        ESP_LOGI( "SDCard", "CMD0 Respond." );
        // CMD8 SDv2 専用コマンドを送信して SDv2 かそれ以外かを確かめる
        if( sendCmd( sk_CMD8, 0x1AA ) == sk_CMD_RES_InIdleState ){
            ESP_LOGI( "SDCard", "Initialize SDv2" );
            // SDv2
            type = initialize_SDv2();
        }
        else {
            ESP_LOGI( "SDCard", "Initialize SDv1 or MMCv3" );
            // CMD8 がリジェクトされたので SDv1/MMC の初期化を試す
            type = initialize_SDv1_or_MMCv3();
        }
    }

    return type;
}

/**
 * @brief   SDv2 初期化
 *          上位側でSDv2であるか判断する。
 * @return  SDカード情報
 **/
uint8_t SD_Card::initialize_SDv2()
{
    uint8_t buf[4] = {0};
    uint8_t type = sk_SDC_Type_None;

    // CMD8が通ったので、コマンドの追加引数を受信
    m_SDC_Drv->recv( buf, sizeof(buf) );

    // 下位12bit が0x1AAなら、SDCv2 で2.7V～3.6Vで動作可能
    if( buf[2] == 0x01 && buf[3] == 0xAA ){
        // 初期化コマンド送信　1秒間リトライする
        uint8_t res = sendCmdRetry( sk_ACMD41, sk_ACMD41_HCS, sk_SDC_TimeOut_ms );

        // OCR読み出しコマンド送信
        if( (res == sk_CMD_RES_OK) && (sendCmd( sk_CMD58, 0 ) == sk_CMD_RES_OK) ){
            ESP_LOGI( "SDCard", "ACMD41/ACMD58 Respond." );
            // OCR受信
            m_SDC_Drv->recv( buf, sizeof(buf) );

            if( (buf[0] & sk_ACMD58_CCS) == sk_ACMD58_CCS ){
                type = sk_SDC_Type_SD2 | sk_SDC_Block;        // アクセスはブロック単位
            }
            else {
                type = sk_SDC_Type_SD2;                       // アクセスはbyte単位
            }
        }
    }

    return type;
}

/**
 * @brief   SDv1 or MMCv3 初期化
 *          上位側で SDv1 もしくは MMCv3 であるか判断する。
 * @return  SDカード情報
 **/
uint8_t SD_Card::initialize_SDv1_or_MMCv3()
{
    uint8_t type = sk_SDC_Type_None;
    uint8_t cmd = 0;
    uint8_t res = 0;

    // ACMD41 SDC専用の初期化コマンドを試す
    // InIdleState もしくは OK が帰ってくるか確認
    res = sendCmd( sk_ACMD41, 0 );
    if( (res == sk_CMD_RES_InIdleState) || (res == sk_CMD_RES_OK) ){
        ESP_LOGI( "SDCard", "SDv1 or MMCv3 : ACMD41 Respond." );
        // ACMD41が通った。CMD8は通らず SDv2 ではなかったので、SDv1
        cmd = sk_ACMD41;
        type = sk_SDC_Type_SD1;
    }
    else {
        // ACMD41がリジェクトされたので、MMCと仮定
        ESP_LOGI( "SDCard", "SDv1 or MMCv3 : ACMD41 not respond." );
        cmd = sk_CMD1;
        type = sk_SDC_Type_MMC;
    }

    // 初期化コマンド送信
    res = sendCmdRetry( cmd, 0, sk_SDC_TimeOut_ms );

    // 初期化コマンド失敗、もしくはセクタサイズ固定に失敗したらカードタイプは不明とする
    if( res != sk_CMD_RES_OK ){
        type = sk_SDC_Type_None;
    }
    else {
        // セクタサイズを512byteに固定
        if( sendCmd( sk_CMD16, sk_SectorSize ) != sk_CMD_RES_OK ){
            type = sk_SDC_Type_None;
        }
    }

    return type;
}

/**
 * @brief   SDカード情報を出力
 **/
void SD_Card::printSDCardInfo()
{
    if( (m_CardType & sk_SDC_Type_SD1) == sk_SDC_Type_SD1 ){
        ESP_LOGI( "SD_Card", "SDCard initialized SDv1." );
    }
    else if( (m_CardType & sk_SDC_Type_SD2) == sk_SDC_Type_SD2 ){
        ESP_LOGI( "SD_Card", "SDCard initialized SDv2." );
    }
    else if( (m_CardType & sk_SDC_Type_MMC) == sk_SDC_Type_MMC ){
        ESP_LOGI( "SD_Card", "SDCard initialized MMC." );
    }
    else {
        ESP_LOGE( "SD_Card", "No detect card or invalid type." );
    }
}

/**
 * @brief   セクタ数読み込み
 **/
void SD_Card::readSectorSize()
{
    if( sendCmd( sk_CMD9, 0 ) != sk_CMD_RES_OK ){
        ESP_LOGE( "SD_Card", "Read sector command(CMD9) failed." );
        m_SectorCount = 0;
        return;
    }
    if( !waitReadDataPacket() ){
        ESP_LOGE( "SD_Card", "Read sector command(CMD9) not respond." );
        m_SectorCount = 0;
        return;
    }

    // CSDデータ = 16byte
    // CRC = 2byte
    uint8_t csd_data[16 + 2];
    m_SDC_Drv->recv( csd_data, sizeof(csd_data) );

    // SDv1 と SDv2 でcsdレジスタの配置が違うので、読み分ける
    uint64_t sdcsize_byte = 0;
    if( (m_CardType & sk_SDC_Type_SD2) == sk_SDC_Type_SD2 ){
        uint64_t c_size = csd_data[9] | (csd_data[8] << 8) | ((csd_data[7] & 0x3F) << 16);
        sdcsize_byte = (c_size + 1ULL) * 512ULL * 1024ULL;
    } 
    else if( ((m_CardType & sk_SDC_Type_SD1) == sk_SDC_Type_SD1) ||
             ((m_CardType & sk_SDC_Type_MMC) == sk_SDC_Type_MMC) )
    {
        uint64_t c_size = ((csd_data[8] & 0xC0) >> 6) | (csd_data[7] << 2) | ((csd_data[6] & 0x03) << 10);
        uint64_t c_size_mult = ((csd_data[10] & 0x80) >> 7) | (csd_data[9] & 0x03);
        uint64_t block_len = csd_data[8] & 0x0F;

        block_len = block_len == 0 ? 1 : block_len;
        sdcsize_byte = (c_size + 1) * (2ULL << (c_size_mult + 1)) * (2ULL << (block_len - 1));
    }

    m_SectorCount = sdcsize_byte / sk_SectorSize;
    ESP_LOGI( "SD_Card", "sdcard size : %llu,  sector count : %u", sdcsize_byte, m_SectorCount );
}

/**
 * @brief   コマンド送信
 * @param [in]  cmd         送信コマンド
 * @param [in]  arg         送信コマンド引数
 * @return                  コマンドレスポンス一覧の値を返す
 **/
uint8_t SD_Card::sendCmd( uint8_t cmd, uint32_t arg )
{
    uint8_t buf[6] = {0};
    uint8_t res = 0;

    // ACMD の場合は 先にCMD55を実施
    if( (cmd & sk_ACMD_Mask) == sk_ACMD_Mask ){
        res = sendCmd( sk_CMD55, 0 );
        cmd &= ~sk_ACMD_Mask;

        if( res != sk_CMD_RES_OK && res != sk_CMD_RES_InIdleState ) {
        	return res;
        }
    }

    // byte0 はコマンド
    // byte1～4に引数（ビッグエンディアン）
    buf[0] = cmd;
    buf[1] = (arg >> 24) & 0xFF;
    buf[2] = (arg >> 16) & 0xFF;
    buf[3] = (arg >>  8) & 0xFF;
    buf[4] = arg & 0xFF;
    // byte5 ダミーのCRC7  SPIの場合はCRCチェックはなくてもよい（ダミーデータは必要）
    buf[5] = 0x01;
    // CMD0, CMD8 のCRC7は必要なので、決め打ち
    if( cmd == sk_CMD0 ){
        buf[5] = 0x95;
    }
    if( cmd == sk_CMD8 ){
        buf[5] = 0x87;
    }

    m_SDC_Drv->Release();
    m_SDC_Drv->Select();
    m_SDC_Drv->send( buf, sizeof(buf) );

    // コマンドレスポンスを待つ
    for( int i = 0; i < sk_CMD_RespRetry; ++i ){
        res = recvByte();

        // コマンドレスポンスが帰ってくると、最上位ビットが0になる
        if( (res & 0x80) != 0x80 ){
            break;
        }
    }

    return res;
}

/**
 * @brief   コマンド送信　リトライ機能付き
 *          コマンドレスポンスが OK になるまでリトライする。
 * @param [in]  cmd         送信コマンド
 * @param [in]  arg         送信コマンド引数
 * @param [in]  retry_cnt   リトライ回数
 * @return                  コマンドレスポンス一覧の値を返す
 * @note    リトライの間隔は1ms
 **/
uint8_t SD_Card::sendCmdRetry( uint8_t cmd, uint32_t arg, uint32_t retry_cnt )
{
    uint8_t res = sk_CMD_RES_OK;

    // 設定された回数コマンド送信をリトライ
    // リトライの間隔は 1ms
    for( uint32_t i = 0; i < retry_cnt; ++i ){
        res = sendCmd( cmd, arg );
        if( res == sk_CMD_RES_OK ){
           break;
        }

        //HAL_Delay( 1 );		// 1ms待つ
        const TickType_t xDelayMs = 1 / portTICK_PERIOD_MS;
        vTaskDelay( xDelayMs );
    }

    return res;
}

/**
 * @brief   読み出し初期化
 * @param [in]  progress    進捗管理構造体
 * @param [in]  sector      読み出し開始セクタ
 * @param [in]  offset      読み出し開始セクタのアドレスから、読み出し開始位置へのオフセット
 * @param [in]  len         読み出し長
 **/
bool SD_Card::readInitiate( Progress* progress, uint32_t sector, uint32_t offset, uint32_t len )
{
	if( len < 1 ){
		return true;
	}

    bool result = false;

    // 開始アドレスを算出
    uint32_t addr = m_CardType & sk_SDC_Block ? sector : sector * sk_SectorSize;

    if( sendCmd( sk_CMD17, addr ) == sk_CMD_RES_OK ){
        // データトークンを待つ
        result = waitReadDataPacket();
    }

    if( result ){
        progress->addr = addr;
        // 総読み込みセクタを算出　1セクタより小さい値の場合でも、1セクタは読み込む
        progress->remain_sector       = (len + offset - 1) / sk_SectorSize;
        progress->remain_bytes_cursec = sk_SectorSize;
        progress->remain_offset       = offset;
        progress->remain_len          = len;
    }

    return result;
}

/**
 * @brief   次セクタ読み出し開始準備
 * @param [in]  progress    進捗管理構造体
 **/
bool SD_Card::nextSectorReadPreparation( Progress* progress )
{
    bool result = false;

    // 進捗を次のセクタに進める
    advanceNextSector( progress );

    if( sendCmd( sk_CMD17, progress->addr ) == sk_CMD_RES_OK ){
        // データトークンを待つ
        result = waitReadDataPacket();
    }

    return result;
}

/**
 * @brief   データパケット受信待ち
 **/
bool SD_Card::waitReadDataPacket()
{
    uint32_t token = 0;

    //  データトークンを受信するまで待つ
    for( int i = 0; i < sk_DataPktReadWait; ++i ){
        token = recvByte();

        if( token != 0xFF ){
        	break;
        }
    }
    // データトークンを受信したらOK
    if( token == sk_DataToken_Read ){
        return true;
    }

    return false;
}

/**
 * @brief   次セクタ書き込み開始準備
 **/
bool SD_Card::nextSectorWritePreparation()
{
    bool result = false;
    uint8_t resp = 0;

    // CRC 2byte分送信
    sendAnyData( 2 );
    // データレスポンス受信
    resp = recvByte();

    if( (resp & sk_DATA_RES_OK_Mask) == sk_DATA_RES_OK ){
        // ビジー解除待ち
        busyWait();
    }

    return result;
}

/**
 * @brief   次セクタに進める
 **/
void SD_Card::advanceNextSector( Progress* progress )
{
    // アドレスを進める
    // カードタイプがブロックアクセスなら +1
    // アドレス指定タイプなら +1セクタサイズ
    progress->addr += m_CardType & sk_SDC_Block ? 1 : sk_SectorSize;
    // 現在セクタの残り読み込み数をセット
    progress->remain_bytes_cursec = sk_SectorSize;
    // 残りセクタ数を減らす
    --progress->remain_sector;
}

/**
 * @brief   1バイト読み込み
 **/
uint8_t SD_Card::recvByte()
{
    // 受信バッファが4Byte境界にないと、ライブラリ側でメモリ確保＆コピーが発生し
    // 処理にものすごく時間がかかる。
    // ので、必ず4Byte境界に配置されるように uint32_t をキャストして使用
    uint32_t buf = 0;
    uint8_t* p = reinterpret_cast<uint8_t*>(&buf);

    m_SDC_Drv->recv( p, 1 );
    return *p;
}

/**
 * @brief   ビジー解除待ち
 **/
void SD_Card::busyWait()
{
    uint8_t busycheck = 0;

    // SDカード内部処理中はDOがLoになる（ビジー）ので、解除まで待つ
    for( int i = 0; i < sk_WriteBusyCheckRetry; ++i ){
        busycheck = recvByte();
        // 書き込み後、SDカードのDOがビジー解除されたらbreak
        if( busycheck == 0xFF ){
            break;
        }
    }
}

/**
 * @brief   データ読み取りスキップ
 * @param [in]  cnt     読み取り個数
 **/
void SD_Card::ignoreRead( uint32_t cnt )
{
    static uint8_t buf[32] __attribute__ ((aligned(4)));
    uint32_t len = cnt;

    while( len > 0 ){
        uint32_t c = std::min( len, static_cast<uint32_t>(sizeof(buf)) );
        m_SDC_Drv->recv( buf, c );
        len -= c;
    }
}

/**
 * @brief   適当なデータを送信する
 * @param [in]  cnt     書き込み個数
 **/
void SD_Card::sendAnyData( uint32_t cnt )
{
    // 読み飛ばせれば何でもよい
    ignoreRead( cnt );
}
