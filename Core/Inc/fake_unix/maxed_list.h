#pragma once
#include <fake_unix/utils.h>
#include <stddef.h>
#include <stdint.h>
#include <list>
#include <queue>
#include <cstring>
namespace fake_unix
{
    template<class T,size_t max_size>
    class simple_object_pool
    {
        uint8_t _space[sizeof(T)*max_size];
        bool _busy[max_size];
    public:
        using value_type=T;
        using reference=T&;
        using const_reference=const T&;
        using size_type=size_t;
        template<class _Tp>
        struct rebind
        {
            typedef simple_object_pool<_Tp,max_size> other;
        };
        simple_object_pool()
        {
            memset(_space,0,sizeof(_space));
            memset(_busy,0,sizeof(_busy));
        }
        T* allocate(size_t count)
        {
            bool test_success=false;
            bool test_avalible=true;
            size_t begin_index=0;
            for(size_t i=0;i<max_size+1-count;i++)
            {
                test_avalible=true;
                for(size_t j=0;j<count;j++)
                {
                    if(_busy[i+j])
                    {
                        test_avalible=false;
                        break;
                    }
                }
                if(test_avalible)
                {
                    begin_index=i;
                    test_success=true;
                    break;
                }
            }
            if(!test_success)
                return nullptr;
            for(size_t i=begin_index;i<begin_index+count;i++)
            {
                _busy[i]=true;
            }
            return reinterpret_cast<T*>(_space)+begin_index;
        }
        void deallocate(T* object_begin,size_t count)
        {
            size_t delta=object_begin-reinterpret_cast<T*>(_space);
            if(delta>=max_size||delta+count>=max_size)
                return;
            for(size_t i=0;i<count;i++)
            {
                _busy[i+delta]=false;
            }
        }
    };

    
    
    template<class T,size_t max_size>
    using list=std::list<T,simple_object_pool<T,max_size>>;  

    template<class T,size_t max_size>
    using queue=std::queue<T,list<T,max_size>>;

    template<class T>
    class loop_buffer;
}

    
