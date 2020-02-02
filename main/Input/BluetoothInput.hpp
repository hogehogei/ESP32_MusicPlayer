
#ifndef     BLUETOOTH_INPUT_HPP_INCLUDED
#define     BLUETOOTH_INPUT_HPP_INCLUDED

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

class BluetoothInput
{
public:

    ~BluetoothInput() noexcept;
    BluetoothInput( const BluetoothInput& ) = delete;
    BluetoothInput& operator=( const BluetoothInput& ) = delete;

    static BluetoothInput& Instance();

    void PlayKeyPressed();
    void PauseKeyPressed();
    void StopKeyPressed();

    bool IsPlayKeyPressed();
    bool IsPauseKeyPressed();
    bool IsStopKeyPressed();

private:

    BluetoothInput();

    void setKeyPressed( bool* key );
    bool getKeyPressed( bool* key );

    SemaphoreHandle_t m_Mutex;

    bool m_PlayKeyPressed;
    bool m_StopKeyPressed;
    bool m_PauseKeyPressed;
};

#endif      // BLUETOOTH_INPUT_HPP_INCLUDED
