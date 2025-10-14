#pragma once
#include <stdint.h>

namespace modern_framework
{
    namespace mcu
    {
        template <class C>
        struct class_type;

        template <class C, class M>
        struct class_type<M C::*>
        {
            using class_t = C;
            using member_t = M;
        };

        template <class C, class M>
        struct class_type<M (C::*)[]>
        {
            using class_t = C;
            using member_t = M;
        };

        // T =type of member variable pointer

        template <class T, T member_ptr, size_t index = 0>
        class get_offset
        {

            static constexpr const char virtual_object = 0;
            static constexpr const typename class_type<T>::class_t &value = reinterpret_cast<const typename class_type<T>::class_t &>(virtual_object);
            static constexpr const char *base_addr = &reinterpret_cast<const char &>(value);
            static constexpr const char *field_addr = const_cast<const char *>(&reinterpret_cast<const volatile char &>(value.*member_ptr));

        public:
            static constexpr unsigned long offset = field_addr - base_addr + index * sizeof(typename class_type<T>::member_t);
        };

        template <unsigned long addr>
        struct mk_offset
        {
            static constexpr unsigned long value = addr;
        };

        template <class T, int bit_offset, int bits>
        class field
        {
        public:
            uint32_t : bit_offset;
            T value : bits;
            uint32_t : 32 - bits - bit_offset;
            field()
            {
                reinterpret_cast<uint32_t &>(*this) = 0;
            }
            operator uint32_t() const
            {
                return reinterpret_cast<const uint32_t &>(*this);
            }
            static constexpr uint32_t mask = ((1 << bits) - 1) << bit_offset;
            static constexpr uint32_t nmask = ~mask;
        };

        template <class T, class OF>
        class peripheral_reigster
        {
            struct helper
            {
#pragma pack(1)
                uint8_t dummy_bytes[OF::value];
                T object;
#pragma pack()
            };

        public:
            constexpr operator T *() const
            {
                return &(static_cast<helper *>(nullptr)->object);
            }
            constexpr T *operator->() const
            {
                return operator T *();
            }
            constexpr operator T &() const
            {
                return static_cast<helper *>(nullptr)->object;
            }
            /*constexpr T &operator=(const T &value) const
            {
                static_cast<helper *>(nullptr)->object = value;
                return static_cast<helper *>(nullptr)->object;
            }*/
            template <class F, int bit_offset, int bits>
            T &operator=(const field<F, bit_offset, bits> &value) const
            {
                reinterpret_cast<uint32_t &>(static_cast<helper *>(nullptr)->object) = (value & field<F, bit_offset, bits>::mask) | (reinterpret_cast<uint32_t &>(static_cast<helper *>(nullptr)->object) & field<F, bit_offset, bits>::nmask);
                return static_cast<helper *>(nullptr)->object;
            }
        };

        class register_object
        {
            uint32_t *_value;

        public:
            register_object() = default;
            register_object(const register_object &that) = default;
            register_object(register_object &&that) = default;
            register_object(uint32_t *value) : _value(value)
            {
            }
            template <class T>
            constexpr T &as()
            {
                return reinterpret_cast<T &>(*_value);
            }

            template <class T>
            void modify(T value, size_t bit_offset, size_t bits)
            {
                *_value = (reinterpret_cast<uint32_t &>(value) << bit_offset) & (((1 << bits) - 1) << bit_offset) | ((*_value) & ~(((1 << bits) - 1) << bit_offset));
            }

            template <class T>
            constexpr void set(T value)
            {
                *reinterpret_cast<T *>(_value) = value;
            }
        };

        template <class T>
        inline register_object get_register(unsigned long addr_base, T member_ptr)
        {
            return reinterpret_cast<uint32_t *>(addr_base + reinterpret_cast<volatile uint8_t *>(&(static_cast<typename class_type<T>::class_t *>(nullptr)->*member_ptr)) - static_cast<volatile uint8_t *>(nullptr));
        }

        template <class T>
        inline register_object get_registers(unsigned long addr_base, T member_ptr, size_t index)
        {
            return reinterpret_cast<uint32_t *>(addr_base + index * sizeof(uint32_t) + reinterpret_cast<volatile uint8_t *>(&(static_cast<typename class_type<T>::class_t *>(nullptr)->*member_ptr)) - static_cast<volatile uint8_t *>(nullptr));
        }

#define MAKE_ADDR(peripheral_addr, class_name, field_name) \
    peripheral_addr + get_offset<class_name, &class_name::field_name>::offset
    }
}