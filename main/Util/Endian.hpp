
#ifndef   EXLIB_ENDIAN_HPP_INCLUDED
#define   EXLIB_ENDIAN_HPP_INCLUDED

#include <stdint.h>
#include <algorithm>

namespace exlib
{


template <typename T>
class Endian
{
public:
    
    static uint16_t GetUint16( const uint8_t* p )
    {
        return T::GetUint16( p );
    }

    static uint32_t GetUint32( const uint8_t* p )
    {
        return T::GetUint32( p );
    }

    static void SetUint16( uint8_t* p, uint16_t data )
    {
    	T::SetUint16( p, data );
    }

    static void SetUint32( uint8_t* p, uint32_t data )
    {
    	T::SetUint32( p, data );
    }
};


class LittleEndian
{
public:
    
    static uint16_t GetUint16( const uint8_t* p )
    {
        return (static_cast<uint32_t>(p[0]) | static_cast<uint32_t>(p[1]) << 8);
    }

    static uint32_t GetUint32( const uint8_t* p )
    {
        return (static_cast<uint32_t>(p[0]) | 
                static_cast<uint32_t>(p[1]) << 8 | 
                static_cast<uint32_t>(p[2]) << 16 | 
                static_cast<uint32_t>(p[3]) << 24);
    }

    static void SetUint16( uint8_t* p, uint16_t data )
    {
    	p[0] = data & 0xFF;
    	p[1] = (data >> 8) & 0xFF;
    }

    static void SetUint32( uint8_t* p, uint32_t data )
    {
    	p[0] = data & 0xFF;
    	p[1] = (data >> 8) & 0xFF;
    	p[2] = (data >> 16) & 0xFF;
    	p[3] = (data >> 24) & 0xFF;
    }
};


class BigEndian
{
public:
    
    static uint16_t GetUint16( const uint8_t* p )
    {
        return (static_cast<uint32_t>(p[0]) << 8 | static_cast<uint32_t>(p[1]));
    }

    static uint32_t GetUint32( const uint8_t* p )
    {
        return (static_cast<uint32_t>(p[0]) << 24 | 
                static_cast<uint32_t>(p[1]) << 16 | 
                static_cast<uint32_t>(p[2]) << 8 | 
                static_cast<uint32_t>(p[3]));
    }

    static void SetUint16( uint8_t* p, uint16_t data )
    {
    	p[0] = (data >> 8) & 0xFF;
    	p[1] = data & 0xFF;
    }

    static void SetUint32( uint8_t* p, uint32_t data )
    {
    	p[0] = (data >> 24) & 0xFF;
    	p[1] = (data >> 16) & 0xFF;
    	p[2] = (data >> 8) & 0xFF;
    	p[3] = data & 0xFF;
    }
};

inline bool IsLittleEndian()
{
    int t = 1;
    return *(reinterpret_cast<char*>( &t )) == 1;
}

inline bool IsBigEndian()
{
    return !IsLittleEndian();
}


template <class T>
T ReverseEndian( T value )
{
    char volatile* first = reinterpret_cast<char volatile*>(&value);
    char volatile* last  = first + sizeof(T);
    std::reverse( first, last );

    return value;
}

}


#endif   // EXLIB_ENDIAN_HPP_INCLUDED


