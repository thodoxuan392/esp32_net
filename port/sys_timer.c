/*
 * sys_timer.c
 *
 *  Created on: Mar 24, 2022
 *      Author: DO XUAN THO
 */


/**
 * @brief Override LWIP weak function
 */
#include "main.h"

uint32_t sys_get_tick_ms(void)
{
    return HAL_GetTick();
}


uint32_t sys_now(void)
{
    return sys_get_tick_ms();
}

/**
 * @brief Override LWIP weak function
 */
uint32_t sys_jiffies(void)
{
    return sys_get_tick_ms();
}

uint32_t sys_rand()
{
    return SysTick->VAL;
}
