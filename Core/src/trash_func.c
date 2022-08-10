#include "main.h"

void	trash_acts_1()
{
//	простейший цикл с задержками Delay
	while(1)
	{
		// _BR13 - включает светодиод
		GPIOC->BSRR = GPIO_BSRR_BR13;
		Delay(500000);
		// _BS13 -  выключает светодиод
		GPIOC->BSRR = GPIO_BSRR_BS13;
		Delay(1500000);
	}
}

void	trash_acts_2()
{
//	настройка системного таймера SysTick
	SysTick_Init();
	//	цикл для SysTick
	while(1){
		if (Sys_Flag == 0)
		// _BR13 - включает светодиод
		GPIOC->BSRR = GPIO_BSRR_BR13;
		if (Sys_Flag == 1)
		// _BS13 -  выключает светодиод
		GPIOC->BSRR = GPIO_BSRR_BS13;
	}
}

void	trash_acts_3()
{
	if (Sys_Flag == 0)
	// _BR13 - включает светодиод
	GPIOC->BSRR = GPIO_BSRR_BR13;
	if (Sys_Flag == 1)
	// _BS13 -  выключает светодиод
	GPIOC->BSRR = GPIO_BSRR_BS13;
}
