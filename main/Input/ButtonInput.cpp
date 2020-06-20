

#include "ButtonInput.hpp"


ButtonInput::ButtonInput()
:
    m_NextButton( 10, 10, ChatteringFilter::sk_Level_LO, ChatteringFilter::sk_Level_HI ),
    m_ChangePlayerButton( 300, 300, ChatteringFilter::sk_Level_LO, ChatteringFilter::sk_Level_HI )
{}

ButtonInput::~ButtonInput()
{}

ButtonInput& ButtonInput::Instance()
{
    static ButtonInput s_Input;
    return s_Input;
}

void ButtonInput::Initialize()
{
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = static_cast<gpio_int_type_t>(GPIO_PIN_INTR_DISABLE);
    //set as output mode
    io_conf.mode = GPIO_MODE_INPUT;
    //bit mask of the pins that you want to set
    io_conf.pin_bit_mask = (1ULL << sk_Button_IONum);
    //disable pull-down mode
    io_conf.pull_down_en = static_cast<gpio_pulldown_t>(0);
    //disable pull-up mode
    io_conf.pull_up_en = static_cast<gpio_pullup_t>(0);
    //configure GPIO with the given settings
    gpio_config(&io_conf);
}

void ButtonInput::Update()
{
    m_NextButton.Update( gpio_get_level(sk_Button_IONum) == 1 );
    m_ChangePlayerButton.Update( gpio_get_level(sk_Button_IONum) == 1 );
}

bool ButtonInput::ChangePlayerButtonPressed()
{
    return m_ChangePlayerButton.IsFallingEdg();
}

bool ButtonInput::NextButtonPressed()
{
    return m_ChangePlayerButton.IsOff() && m_NextButton.IsFallingEdg();
}
