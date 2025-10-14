#pragma once
#include <stddef.h>
#include <stdint.h>
#include <linked_list.h>
/**
 * timer支持在若干毫秒后 执行某项操作
 * 每个timer只支持延时执行1个任务,第二个任务会覆盖前一个
 * 系统中暂不支持超过10个timer,也不能析构
 * 回调使用的成员函数类型需要为void(void)
 * 直接用WRAP(member_fun,this)的形式
*/
class timer:public modern_framework::linked_list<timer>
{
    uint64_t _timeout_stamp;
    void(*_function)(void*,size_t);
    void* _pthis;
    
public:
    timer();
    uint8_t check_timeout();
    void executate_after(size_t ms,void(*executor)(void*,size_t),void* pthis);
    void cancel();
};


#define TIMSLOT(member_fun) MKSLOT(member_fun)->get_exec()