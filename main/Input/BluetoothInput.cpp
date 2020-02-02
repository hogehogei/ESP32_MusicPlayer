
#include "BluetoothInput.hpp"

BluetoothInput::BluetoothInput()
    : m_PlayKeyPressed( false ),
      m_StopKeyPressed( false ),
      m_PauseKeyPressed( false )
{
    m_Mutex = xSemaphoreCreateMutex();
}

BluetoothInput::~BluetoothInput()
{}

BluetoothInput& BluetoothInput::Instance()
{
    static BluetoothInput s_Instance;
    return s_Instance;
}

void BluetoothInput::PlayKeyPressed()
{
    setKeyPressed( &m_PlayKeyPressed );
}

void BluetoothInput::PauseKeyPressed()
{
    setKeyPressed( &m_PauseKeyPressed );
}

void BluetoothInput::StopKeyPressed()
{
    setKeyPressed( &m_StopKeyPressed );
}

bool BluetoothInput::IsPlayKeyPressed()
{
    return getKeyPressed( &m_PlayKeyPressed );
}

bool BluetoothInput::IsPauseKeyPressed()
{
    return getKeyPressed( &m_PauseKeyPressed );
}

bool BluetoothInput::IsStopKeyPressed()
{
    return getKeyPressed( &m_StopKeyPressed );
}

void BluetoothInput::setKeyPressed( bool* key )
{
    if( xSemaphoreTake( m_Mutex, static_cast<TickType_t>( 1000 )) == pdTRUE ){
        *key = true;
        xSemaphoreGive( m_Mutex );
    }
}

bool BluetoothInput::getKeyPressed( bool* key )
{
    bool is_pressed = false;
    if( xSemaphoreTake( m_Mutex, static_cast<TickType_t>( 1000 )) == pdTRUE ){
        is_pressed = *key;
        *key = false;
        xSemaphoreGive( m_Mutex );
    }

    return is_pressed;
}