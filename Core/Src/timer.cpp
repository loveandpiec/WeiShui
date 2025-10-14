#include <timer.h>
#include <task_queue.h>
#include <stm32l4xx_ll_tim.h>
#include <math.h>
extern "C"
{
    timer *all_timers[10];
    size_t timer_count;
    uint64_t global_mstimestamp;
}

timer::timer()
{
    all_timers[timer_count++] = this; // 往所有timer列表中插入自身
    _timeout_stamp = uint64_t(~1ULL);
}

auto timer::check_timeout() -> uint8_t
{
    if (_timeout_stamp > global_mstimestamp)
        return false;
    else
    {
        g_task_queue->post(_function, _pthis, g_param_buffer->get_end());
        _timeout_stamp = ~1ULL;
        return true;
    }
}

void timer::executate_after(size_t ms, void (*executor)(void *, size_t), void *pthis)
{
    _timeout_stamp = global_mstimestamp + ms;
    _function = executor;
    _pthis = pthis;
}

void timer::cancel()
{
    _timeout_stamp = uint64_t(~1ULL);
}

extern "C" void SysTick_Handler(void)
{
//    HAL_IncTick();
    global_mstimestamp+=1;
    timer* cur=timer::header;
    while(cur)
    {
        cur->check_timeout();
        cur=cur->next;
    }
}

// int step = 0;
// bool start = true;
// uint8_t ratios[201];
// extern "C" void TIM1_UP_TIM16_IRQHandler(void)
// {
//     /*if (start)
//     {
//         start=false;
//         for(int i=0;i<201;i++)
//         {
//             ratios[i]=(1.0 + sin(0.031415926 * i)) * 50.0;
//             ratios[i]=100-ratios[i];
//         }
//     }*/
//     if (LL_TIM_IsActiveFlag_UPDATE(TIM1) == 1)
//     {
//         LL_TIM_ClearFlag_UPDATE(TIM1);
//         global_mstimestamp += 5;
//         for (size_t i = 0; i < timer_count; i++)
//             all_timers[i]->check_timeout();

        
//         /*LL_TIM_OC_SetCompareCH1(TIM2, ratios[step++/5]);
//         if (step > 1005)
//             step = 0;*/
//     }
// }
