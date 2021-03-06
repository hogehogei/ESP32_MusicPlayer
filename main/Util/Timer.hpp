#ifndef     TIMER_HPP_INCLUDED
#define     TIMER_HPP_INCLUDED

#include <cstdint>

class MsTimer
{
public:

    MsTimer();
    ~MsTimer() noexcept;

    MsTimer( const MsTimer& ) = delete;
    MsTimer& operator=( const MsTimer& ) = delete;
    
    bool IsValid() const;
    bool IsElapsed( uint32_t milli_sec ) const;
    void Reset();

private:

    int64_t m_BeginTimeMs;
};

#endif      // TIMER_HPP_INCLUDED
