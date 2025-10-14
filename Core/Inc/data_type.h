#pragma once
#include <stdint.h>
#include <stddef.h>
enum TLV_types
{
    Boolean = 1,
    Tiny = 43,
    Utiny = 32,
    Short = 33,
    UShort = 45,
    Int = 2,
    Uint = 35,
    Long = 36,
    Ulong = 37,
    Float = 38,
    Double = 39,
    OcterString = 4,
    String = 5,
    DataTime = 64,
    Struct = 65
};

struct datatime
{
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
};

template <TLV_types type_tag>
class _tlv_type;

template <>
class _tlv_type<Boolean>
{
public:
    constexpr static size_t length = sizeof(bool);
    using c_type = bool;
};

template <>
class _tlv_type<Tiny>
{
public:
    constexpr static size_t length = sizeof(int8_t);
    using c_type = int8_t;
};

template <>
class _tlv_type<Utiny>
{
public:
    constexpr static size_t length = sizeof(uint8_t);
    using c_type = uint8_t;
};

template <>
class _tlv_type<Short>
{
public:
    constexpr static size_t length = sizeof(int16_t);
    using c_type = int16_t;
};

template <>
class _tlv_type<UShort>
{
public:
    constexpr static size_t length = sizeof(uint16_t);
    using c_type = uint16_t;
};

template <>
class _tlv_type<Int>
{
public:
    constexpr static size_t length = sizeof(int32_t);
    using c_type = int32_t;
};

template <>
class _tlv_type<Uint>
{
public:
    constexpr static size_t length = sizeof(uint32_t);
    using c_type = uint32_t;
};

template <>
class _tlv_type<Long>
{
public:
    constexpr static size_t length = sizeof(int64_t);
    using c_type = int64_t;
};

template <>
class _tlv_type<Ulong>
{
public:
    constexpr static size_t length = sizeof(uint64_t);
    using c_type = uint64_t;
};

template <>
class _tlv_type<Float>
{
public:
    constexpr static size_t length = sizeof(float);
    using c_type = float;
};

template <>
class _tlv_type<Double>
{
public:
    constexpr static size_t length = sizeof(double);
    using c_type = double;
};

template <>
class _tlv_type<OcterString>
{
public:
    constexpr static size_t length = 0;
    using c_type = char;
};

template <>
class _tlv_type<String>
{
public:
    constexpr static size_t length = 0;
    using c_type = char;
};

template <>
class _tlv_type<DataTime>
{
public:
    constexpr static size_t length = 7;
    using c_type = datatime;
};

template <>
class _tlv_type<Struct>
{
public:
    constexpr static size_t length = 0;
    using c_type = char*;
};


template<TLV_types type_tag>
using tlv_type=typename _tlv_type<type_tag>::c_type;

template<TLV_types type_tag>
constexpr size_t tlv_length=_tlv_type<type_tag>::length;

namespace modbus
{
    enum reply_type
    {
        normal,
        broadcast,
        id_ignore,
        command_code_err,
        bad_range,
        bad_count,
        crc_err
    };

    enum modbus_error_code
    {
        invalid_function_code = 0x01,
        invalid_data_addr = 0x02,
        invalid_data_value = 0x03
    };

    enum modbus_function_code
    {
        read_coil = 0x01,
        read_discrete = 0x02,
        read_holding_register = 0x03,
        read_input_register = 0x04,
        write_single_coil = 0x05,
        write_single_register = 0x06,
        write_multi_coils = 0x0F,
        write_multi_registers = 0x10,
        extend_command_code = 0x66
    } ;

}