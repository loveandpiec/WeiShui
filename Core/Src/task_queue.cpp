#include <task_queue.h>

task_queue::task_queue()
{
    _task_begin=0;
    _task_end=0;
}
param_buffer::param_buffer()
{
    _begin=0;
    _end=0;
}

size_t param_buffer::get_end()
{
    return _end;
}
void param_buffer::set_end(size_t end)
{
    _end=end;
}
void task_queue::post(void(*function)(void*,size_t),void* pthis,size_t param_buffer_begin)
{
    if(full())
        return;  
    size_t end=_task_end;

    _queue[end].function=function;
    _queue[end].pthis=pthis;
    _queue[end].param_buffer_begin=param_buffer_begin;
    _task_end++;
    if(_task_end==fake_unix::array_size<decltype(_queue)>::value)
        _task_end=0;
}

bool task_queue::empty()
{
    return (_task_end==_task_begin);
}

bool task_queue::full()
{
    size_t end=_task_end+1;
    if(end>=fake_unix::array_size<decltype(_queue)>::value) 
        end=0;
    return _task_begin==end;
}
void task_queue::invoke()
{
    size_t begin=_task_begin;
    _queue[begin].function(_queue[begin].pthis,_queue[begin].param_buffer_begin);
    _task_begin++;
    if(_task_begin>=fake_unix::array_size<decltype(_queue)>::value)
        _task_begin=0;    
}   