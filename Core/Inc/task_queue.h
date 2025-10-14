#pragma once
#include <stdint.h>
#include <stddef.h>
#include <fake_unix/utils.h>
#include <stm32l4xx_ll_usart.h>
#include <stdio.h>
class task_queue
{
    struct task_type
    {
        void(*function)(void*,size_t);
        void* pthis;
        size_t param_buffer_begin;
    };
    task_type _queue[100];
    volatile size_t _task_begin;
    volatile size_t _task_end;
public:
    task_queue();
    void post(void(*function)(void*,size_t),void* pthis,size_t param_buffer_begin);
    bool empty();
    void invoke();
    bool full();
};

class param_buffer
{
    uint8_t _param_buffer[4096];
    volatile size_t _begin;
    volatile size_t _end;
public:
    param_buffer();

    template<class T>
    size_t push_values2stream(T value)
    {
        size_t new_begin=_end;
        size_t new_end=_end+sizeof(T);
        if(new_end>fake_unix::array_size<decltype(_param_buffer)>::value)
        {
            new_end=sizeof(T);
            new_begin=0;
            if(_begin<new_end)
                return size_t(~1);
        }
        
        _end=new_end;
        *reinterpret_cast<T*>(_param_buffer+new_begin)=value;
        return new_end;
    }

    template<class T>
    size_t pop_values_from_stream(size_t begin_pos,T& out)//返回本变量的end
    {
        size_t test_end=begin_pos+sizeof(T);
        if(test_end>fake_unix::array_size<decltype(_param_buffer)>::value)
            begin_pos=0;
        _begin=begin_pos;
        out=*reinterpret_cast<T*>(_param_buffer+_begin);
        return begin_pos+sizeof(T);
    }

    size_t get_end();
    void set_end(size_t end_pos);
};

extern task_queue* g_task_queue;
extern param_buffer* g_param_buffer;
 
template<class T>
class from_member_function;

template<class C>
class from_member_function<void(C::*)()>
{
public:
    template<void(C::*member_fun)()>
    static void to_unique_function(void* pthis,size_t begin_pos)
    {
        (reinterpret_cast<C*>(pthis)->*member_fun)();
    }
    template<void(C::*member_fun)()>
    static void to_c_style_function(void* pthis)
    {
        (reinterpret_cast<C*>(pthis)->*member_fun)();
    }
};

template<class C,class Arg1>
class from_member_function<void(C::*)(Arg1)>
{
public:
    template<void(C::*member_fun)(Arg1)>
    static void to_unique_function(void* pthis,size_t begin_pos)
    {
        Arg1 arg1;
        g_param_buffer->pop_values_from_stream(begin_pos,arg1);
        (reinterpret_cast<C*>(pthis)->*member_fun)(arg1);
    }

    template<void(C::*member_fun)(Arg1)>
    static void to_c_style_function(void* pthis,Arg1 arg1)
    {
        (reinterpret_cast<C*>(pthis)->*member_fun)(arg1);
    }
};
template<class C,class Arg1,class Arg2>
class from_member_function<void(C::*)(Arg1,Arg2)>
{
public:
    template<void(C::*member_fun)(Arg1,Arg2)>
    static void to_unique_function(void* pthis,size_t begin_pos)
    {
        Arg1 arg1;
        Arg2 arg2;
        begin_pos=g_param_buffer->pop_values_from_stream(begin_pos,arg1);
        g_param_buffer->pop_values_from_stream(begin_pos,arg2);
        (reinterpret_cast<C*>(pthis)->*member_fun)(arg1,arg2);
    }
    template<void(C::*member_fun)(Arg1,Arg2)>
    static void to_c_style_function(void* pthis,Arg1 arg1,Arg2 arg2)
    {
        (reinterpret_cast<C*>(pthis)->*member_fun)(arg1,arg2);
    }
};

template<class C,class Arg1,class Arg2,class Arg3>
class from_member_function<void(C::*)(Arg1,Arg2,Arg3)>
{
public:
    template<void(C::*member_fun)(Arg1,Arg2,Arg3)>
    static void to_unique_function(void* pthis,size_t begin_pos)
    {
        Arg1 arg1;
        Arg2 arg2;
        Arg3 arg3;
        begin_pos=g_param_buffer->pop_values_from_stream(begin_pos,arg1);
        begin_pos=g_param_buffer->pop_values_from_stream(begin_pos,arg2);
        g_param_buffer->pop_values_from_stream(begin_pos,arg3);
        (reinterpret_cast<C*>(pthis)->*member_fun)(arg1,arg2,arg3);
    }
};

template<class C,class Arg1,class Arg2,class Arg3,class Arg4>
class from_member_function<void(C::*)(Arg1,Arg2,Arg3,Arg4)>
{
public:
    template<void(C::*member_fun)(Arg1,Arg2,Arg3,Arg4)>
    static void to_unique_function(void* pthis,size_t begin_pos)
    {
        Arg1 arg1;
        Arg2 arg2;
        Arg3 arg3;
        Arg4 arg4;
        begin_pos=g_param_buffer->pop_values_from_stream(begin_pos,arg1);
        begin_pos=g_param_buffer->pop_values_from_stream(begin_pos,arg2);
        begin_pos=g_param_buffer->pop_values_from_stream(begin_pos,arg3);
        g_param_buffer->pop_values_from_stream(begin_pos,arg4);
        (reinterpret_cast<C*>(pthis)->*member_fun)(arg1,arg2,arg3,arg4);
    }
};

#define WRAP_FUN(MEMBER_FUNCTION_NAME)  from_member_function<decltype(&fake_unix::this2type<decltype(this)>::type::MEMBER_FUNCTION_NAME)>\
::to_c_style_function<&fake_unix::this2type<decltype(this)>::type::MEMBER_FUNCTION_NAME>

#define WRAP(MEMBER_FUNCTION_NAME,THIS)  from_member_function<decltype(&fake_unix::this2type<decltype(THIS)>::type::MEMBER_FUNCTION_NAME)>\
::to_unique_function<&fake_unix::this2type<decltype(THIS)>::type::MEMBER_FUNCTION_NAME>,THIS

template<class T>
class signal;

template<>
class signal<void()>
{
    void(*_function)(void*,size_t);
    void* _pthis;
public:
    void operator()()
    {
        g_task_queue->post(_function,_pthis,g_param_buffer->get_end());
    }
    void connect(void(*fun)(void*,size_t),void* pthis)
    {
        _function=fun;
        _pthis=pthis;
    }
};

template<class Arg1>
class signal<void(Arg1)>
{
    void(*_function)(void*,size_t);
    void* _pthis;
public:
    void operator()(Arg1 arg1)
    {
        size_t old_end = g_param_buffer->get_end();//旧的end起始就是新的begin
        if(g_param_buffer->push_values2stream(arg1)==size_t(~1))
        {
            g_param_buffer->set_end(old_end);
            return;
        }
        g_task_queue->post(_function,_pthis,old_end);
    }
    void connect(void(*fun)(void*,size_t),void* pthis)
    {
        _function=fun;
        _pthis=pthis;
    }
};

template<class Arg1,class Arg2>
class signal<void(Arg1,Arg2)>
{
    void(*_function)(void*,size_t);
    void* _pthis;
public:
    void operator()(Arg1 arg1,Arg2 arg2)
    {
        size_t old_end = g_param_buffer->get_end();//旧的end起始就是新的begin
        if(g_param_buffer->push_values2stream(arg1)==size_t(~1))
        {
            g_param_buffer->set_end(old_end);
            return;
        }
        if(g_param_buffer->push_values2stream(arg2)==size_t(~1))
        {
            g_param_buffer->set_end(old_end);
            return;
        }
        g_task_queue->post(_function,_pthis,old_end);
    }
    void connect(void(*fun)(void*,size_t),void* pthis)
    {
        _function=fun;
        _pthis=pthis;
    }
};

template<class Arg1,class Arg2,class Arg3>
class signal<void(Arg1,Arg2,Arg3)>
{
    void(*_function)(void*,size_t);
    void* _pthis;
public:
    void operator()(Arg1 arg1,Arg2 arg2,Arg3 arg3)
    {
        size_t old_end = g_param_buffer->get_end();//旧的end起始就是新的begin
        if(g_param_buffer->push_values2stream(arg1)==size_t(~1))
        {
            g_param_buffer->set_end(old_end);
            return;
        }
        if(g_param_buffer->push_values2stream(arg2)==size_t(~1))
        {
            g_param_buffer->set_end(old_end);
            return;
        }
        if(g_param_buffer->push_values2stream(arg3)==size_t(~1))
        {
            g_param_buffer->set_end(old_end);
            return;
        }
        g_task_queue->post(_function,_pthis,old_end);
    }
    void connect(void(*fun)(void*,size_t),void* pthis)
    {
        _function=fun;
        _pthis=pthis;
    }
};

template<class Arg1,class Arg2,class Arg3,class Arg4>
class signal<void(Arg1,Arg2,Arg3,Arg4)>
{
    void(*_function)(void*,size_t);
    void* _pthis;
public:
    void operator()(Arg1 arg1,Arg2 arg2,Arg3 arg3 ,Arg3 arg4 )
    {
        size_t old_end = g_param_buffer->get_end();//旧的end起始就是新的begin
        if(g_param_buffer->push_values2stream(arg1)==size_t(~1))
        {
            g_param_buffer->set_end(old_end);
            return;
        }
        if(g_param_buffer->push_values2stream(arg2)==size_t(~1))
        {
            g_param_buffer->set_end(old_end);
            return;
        }
        if(g_param_buffer->push_values2stream(arg3)==size_t(~1))
        {
            g_param_buffer->set_end(old_end);
            return;
        }
        if(g_param_buffer->push_values2stream(arg4)==size_t(~1))
        {
            g_param_buffer->set_end(old_end);
            return;
        }
        g_task_queue->post(_function,_pthis,old_end);
    }
    void connect(void(*fun)(void*,size_t),void* pthis)
    {
        _function=fun;
        _pthis=pthis;
    }
};

/*template<class... Args>
class signal<void(Args...)>
{
    void(*_function)(void*,size_t);
    void* _pthis;
public:
    void operator()(Arg1 arg1,Arg2 arg2,Arg3 arg3 ,Arg3 arg4 )
    {

    }
};*/