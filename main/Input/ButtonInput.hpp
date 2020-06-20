#ifndef    BUTTON_INPUT_HPP_INCLUDED
#define    BUTTON_INPUT_HPP_INCLUDED

//
//    esp-idf headers
//
#include "esp_system.h"
#include "driver/gpio.h"

#include "ChatteringFilter.hpp"

class ButtonInput
{
public:

    ButtonInput();
    ~ButtonInput() noexcept;

    ButtonInput( ButtonInput& ) = delete;
    ButtonInput& operator=( ButtonInput& ) = delete;

    static ButtonInput& Instance();
    void Initialize();

    void Update();
    bool ChangePlayerButtonPressed();
    bool NextButtonPressed();

private:
    
    static constexpr gpio_num_t sk_Button_IONum = GPIO_NUM_0;

    ChatteringFilter m_NextButton;
    ChatteringFilter m_ChangePlayerButton;
};

#endif    // BUTTON_INPUT_HPP_INCLUDED
