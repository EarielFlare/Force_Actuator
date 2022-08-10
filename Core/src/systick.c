#include "main.h"

void	SysTick_Handler(void)
{
//	if(SysTick_CNT > 0) SysTick_CNT--;

	Sys_Flag ^= 1;
//	if (Sys_Flag == 0) Sys_Flag = 1;
//	else if (Sys_Flag == 1) Sys_Flag = 0;
}

void	SysTick_Init(void)
//	функция настройки basic таймера SysTick (cortex system timer)
//	тактируется от HCLK (HCLK/8), считает вниз до нуля, все просто
{
	//	дефайны SysTick находятся в core_cm3.h
	//	устанавливаем значения LOAD и VAL, потом запускаем таймер

	//	регистр LOAD определяет начальное значение, которое будет
	//	загружаться в регистр VAL, когда счетчик разрешен и когда он
	//	достигает 0. значение поля RELOAD может быть любым в диапазоне
	//	0x00000001-0x00FFFFFF (16777215). начальное значение 0 также возможно,
	//	но оно не имеет никакого эффекта, потому что запрос исключения
	//	SysTick и флаг COUNTFLAG активизируются при	переходе счета с 1 на 0.
	//	зададим период так, чтобы таймер отсчитывал интервалы в 1 миллисекунду
//	MODIFY_REG(SysTick->LOAD, SysTick_LOAD_RELOAD_Msk, SYSCLOCK / 1000 - 1);
	//	зададим период так, чтобы таймер отсчитывал интервалы в 0.25 сек
	//	48000000 / 4 - 1 = 12000000 - 1 = 11999999
	MODIFY_REG(SysTick->LOAD, SysTick_LOAD_RELOAD_Msk, SYSCLOCK / 4 - 1);

	//	регистр VAL содержит текущее значение счетчика SysTick.
	//	чтение регистра возвращает это текущее значение.
	//	запись любого значения очищает поле в '0',
	//	а также сбрасывает бит COUNTFLAG в регистре CTRL.
//	MODIFY_REG(SysTick->VAL,SysTick_VAL_CURRENT_Msk, SYSCLOCK - 1);
	CLEAR_BIT(SysTick->VAL, SysTick_VAL_CURRENT_Msk);

	//	SysTick control and status register (регистр CTRL)
	//	выбор источника тактового сигнала: processor clock (AHB)
	SET_BIT(SysTick->CTRL, SysTick_CTRL_CLKSOURCE_Msk);
	//	если TICKINT установлен в "1", счет вниз до нуля
	//	вызывает запрос исключения SysTick.
	SET_BIT(SysTick->CTRL, SysTick_CTRL_TICKINT_Msk);
	//	включаем счетчик. когда бит ENABLE установлен в "1",
	//	счетчик загружается значением RELOAD из регистрира LOAD,
	//	а затем считает в обратном порядке. При достижении нуля,
	//	бит COUNTFLAG ставится в "1" и, в зависимости от значения
	//	бита TICKINT ставится запрос исключения SysTick.
	//	счетчик загружает значение RELOAD снова, и начинает счет.
	SET_BIT(SysTick->CTRL, SysTick_CTRL_ENABLE_Msk);
	//	регистр COUNTFLAG (rw) здесь не прописывается (а зачем?)
	//	ведь мы обнулили его ВЫШЕ (при работе с регистром VAL)
	//	он возвращает '1', если таймер успел досчитать до нуля,
	//	с тех пор, как последний раз было считано его значение
}
