#ifndef     BATT_VOLT_HPP_INCLUDED
#define     BATT_VOLT_HPP_INCLUDED

#include <cstdint>

// esp-idf headers
#include "driver/adc.h" 
#include "esp_adc_cal.h"


class BattVolt
{
public:
    BattVolt();
    ~BattVolt() noexcept;

    static BattVolt& Instance();
    void Initialize();

    void Update();
    uint32_t MillVolt() const;

private:

    static constexpr double sk_UpdateInterval = 0.01;              //! 更新周期[s]
    static constexpr double sk_LPF_CutOffFreq = 1.0;               //! カットオフ周波数[Hz]
                                                                   //! LPF定数
    static constexpr double sk_LPF_Const      = (sk_LPF_CutOffFreq / (sk_UpdateInterval + sk_LPF_CutOffFreq));

    esp_adc_cal_characteristics_t m_ADC_Characteristics;
    double m_BattMillVolt;
};

#endif      // BATT_VOLT_HPP_INCLUDED