#include "main.h"

int	ClockInit(void)
//	настраиваем тактирование системы от внешнего
//	кварца 8MHz через PLL на частоте 72MHz
//	функция возвращает:
//  0 - завершено успешно
//  1 - не запустился кварц
//  2 - не запустился PLL
{
	__IO int StartUpCounter;
	//	запускаем кварцевый генератор (HSE)
	RCC->CR |= (1<<RCC_CR_HSEON_Pos);
	//	ждем успешного запуска или окончания тайм-аута
	for (StartUpCounter = 0; ; StartUpCounter++)
	{
		//	если успешно запустилось, то выходим из цикла
		if (RCC->CR & (1<<RCC_CR_HSERDY_Pos))
			break;
		//	если не запустилось, то отключаем
		//	все, что включили и возвращаем ошибку
		if (StartUpCounter > 0x1000)
		{
			//	останавливаем генератор (HSE)
			RCC->CR &= ~(1<<RCC_CR_HSEON_Pos);
			return (1);
		}
	}

	//	настраиваем и запускаем PLL
	//	сначала настраиваем PLL, множитель = 9
	RCC->CFGR |= (0x07<<RCC_CFGR_PLLMULL_Pos)
			//Тактирование PLL от HSE
			| (0x01<<RCC_CFGR_PLLSRC_Pos);
	//	запускаем PLL
	RCC->CR |= (1<<RCC_CR_PLLON_Pos);

	//	ждем успешного запуска или окончания тайм-аута
	for (StartUpCounter=0; ; StartUpCounter++)
	{
		//	если успешно запустилось, то выходим из цикла
		if (RCC->CR & (1<<RCC_CR_PLLRDY_Pos))
			break;
		//	если по каким-то причинам не запустился PLL, то
		//	отключаем все, что включили и возвращаем ошибку
		if (StartUpCounter > 0x1000)
		{
			//	останавливаем HSE
			RCC->CR &= ~(1<<RCC_CR_HSEON_Pos);
			//	останавливаем PLL
			RCC->CR &= ~(1<<RCC_CR_PLLON_Pos);
			return (2);
		}
	}
	//	настраиваем FLASH и делители
	//	устанавливаем 2 цикла ожидания для Flash
	//	так как частота ядра у нас будет 48MHz < SYSCLK <= 72MHz
	FLASH->ACR |= (0x02<<FLASH_ACR_LATENCY_Pos);
	//	делители
	RCC->CFGR |= (0x00<<RCC_CFGR_PPRE2_Pos)	//	делитель шины APB2 отключен
			| (0x04<<RCC_CFGR_PPRE1_Pos)	//	делитель шины APB1 равен 2
			| (0x00<<RCC_CFGR_HPRE_Pos);	//	делитель AHB отключен
	RCC->CFGR |= (0x02<<RCC_CFGR_SW_Pos);	//	переключаемся на работу от PLL
	//	ждем, пока переключимся
	while ((RCC->CFGR & RCC_CFGR_SWS_Msk) != (0x02<<RCC_CFGR_SWS_Pos)){}
	//	после того, как переключились на внешний источник тактирования,
	//	отключаем внутренний RC-генератор для экономии энергии
	RCC->CR &= ~(1<<RCC_CR_HSION_Pos);
	//	настройка и переключение системы
	//	на внешний кварцевый генератор и PLL завершилось успехом
	return (0);
}
