
#include "BattVolt.hpp"

// esp-idf library
#include "driver/adc.h" 
#include "esp_adc_cal.h"

BattVolt::BattVolt()
{}

BattVolt::~BattVolt()
{}

BattVolt& BattVolt::Instance()
{
    static BattVolt s_BattVolt;
    return s_BattVolt;
}

void BattVolt::Initialize()
{
    // ADCを起動（ほかの部分で明示的にOFFにしてなければなくても大丈夫）
    adc_power_on();
    // ADC1_CH6を初期化
    adc_gpio_init(ADC_UNIT_1, ADC_CHANNEL_5);
    // ADC1の解像度を12bit（0~4095）に設定
    adc1_config_width(ADC_WIDTH_BIT_12);
    // ADC1の減衰を11dBに設定
    adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_11);
    // 電圧値に変換するための情報をaddCharに格納
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &m_ADC_Characteristics );
}

void BattVolt::Update()
{
    uint32_t voltage;
    // ADC1_CH5の電圧値を取得
    esp_adc_cal_get_voltage( ADC_CHANNEL_5, &m_ADC_Characteristics, &voltage );

    double previous_millvolt = m_BattMillVolt;
    m_BattMillVolt = (1.0 - sk_LPF_Const) * static_cast<double>(voltage) + sk_LPF_Const * previous_millvolt;
}

uint32_t BattVolt::MillVolt() const
{
    return m_BattMillVolt;
}