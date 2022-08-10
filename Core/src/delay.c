#include "main.h"

void	Delay(uint32_t time)
{
	while (time > 0)
		time--;
}

void	Delay_ms(uint32_t ms)
{
	MODIFY_REG(SysTick->VAL,SysTick_VAL_CURRENT_Msk, SYSCLOCK / 1000 - 1);
	SysTick_CNT = ms;
	while(SysTick_CNT) {}
}
