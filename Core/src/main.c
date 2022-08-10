#include "main.h"
/*
void	delay_ms(uint16_t delay_ms)
{
	volatile unsigned int num;
	for (num = 0; num < delay_ms; num++)
	{
		delay_us(1000);
	}
}
*/
void	global_var_init(t_vars *p_vars)
{
	SysTick_CNT = 0;
	Sys_Flag = 0;
	Sys_Flag_Last = 0;
	Enc_Dir2 = 0;
	Enc_Dir3 = 0;
	Enc_Cnt2 = 0;
	Enc_Cnt2_Last = 0;
	Enc_Cnt3 = 0;
	p_vars->arr_psc_div[0] = 10000;		//	50 Hz
	p_vars->arr_psc_div[1] = 50000;		//	250 Hz
	p_vars->arr_psc_div[2] = 100000;	//	500 Hz
	p_vars->arr_psc_div[3] = 200000;	//	1000 Hz	//	no vibrations
	p_vars->arr_psc_div[4] = 400000;	//	2000 Hz
	p_vars->arr_psc_div[5] = 800000;	//	4000 Hz
	p_vars->arr_psc_div[6] = 1600000;	//	8000 Hz
	p_vars->arr_psc_div[7] = 3200000;	//	16000 Hz
	p_vars->arr_psc_div[8] = 6400000;	//	32000 Hz
	cnt = 0;
	psc_div = p_vars->arr_psc_div[8];
	Enc_Ind3 = 0;
	ccr_ind1 = 0;
	ccr_ind2 = 0;
	ccr_ind3 = 0;
	ccr_val1 = 0;
	ccr_val2 = 0;
	ccr_val3 = 0;
}

int main(void)
{
	t_vars	*p_vars;

	p_vars = (t_vars *)malloc(sizeof(t_vars));
	if (!p_vars)
		return (0);
	global_var_init(p_vars);
//	printf("p_vars->arr[0] = %d\n", p_vars->arr[0]);
//	printf("p_vars->arr[0] = %d\n", p_vars->arr[1]);
//	printf("p_vars->arr[0] = %d\n", p_vars->arr[2]);
//	printf("p_vars->arr[0] = %d\n", p_vars->arr[3]);
//	y_dvdr = 128;
//	y_base = y_dvdr - 1;

//	shape_1(p_vars);	//	no
	shape_2(p_vars);	//	sin
//	shape_3(p_vars);	//	omg
//	shape_4(p_vars);	//	трапеция 1/6Т
//	shape_5(p_vars);	//	трапеция 2/6Т
//	shape_6(p_vars);	//	седло

/*
	x = 0;
	y_dvdr = 128;
	while (x <= 85)
	{
		p_vars->sin_val[x] = y_dvdr +
			((int)((y_dvdr - 1) * sinf(
			x * (acos(-1.0) / y_dvdr))));
		x++;
	}
	while (x <= 170)
	{
		p_vars->sin_val[x] = y_dvdr +
			((int)((y_dvdr - 1) * sinf(
			(x - 42) * (acos(-1.0) / y_dvdr))));
		x++;
	}
	while (x <= 255)
	{
		p_vars->sin_val[x] = y_dvdr;
		x++;
	}
*/
/*
	x = 0;
	y_dvdr = 128;
	while (x <= 64)
	{
		p_vars->sin_val[x] = y_dvdr +
			((int)((y_dvdr - 1) * sinf(
			x * (acos(-1.0) / 96))));
		x++;
	}
	while (x <= 128)
	{
		p_vars->sin_val[x] = y_dvdr +
			((int)((y_dvdr - 1) * sinf(
			(x - 32) * (acos(-1.0) / 96))));
		x++;
	}
	x--;
	while (x <= 192)
	{
		p_vars->sin_val[x] = y_dvdr +
			((int)((y_dvdr - 1) * sinf(
			(x - 32) * (acos(-1.0) / 96))));
		x++;
	}
	while (x <= 255)
	{
		p_vars->sin_val[x] = y_dvdr +
			((int)((y_dvdr - 1) * sinf(
			(x - 64) * (acos(-1.0) / 96))));
		x++;
	}
*/
/*
	x = 0;
	y_dvdr = 128;
	while (x <= 64)
	{
		p_vars->sin_val[x] = y_dvdr + x;
		x++;
	}
	while (x <= 192)
	{
		p_vars->sin_val[x] = y_dvdr - x;
		x++;
	}
	while (x <= 255)
	{
		p_vars->sin_val[x] = y_dvdr + x;
		x++;
	}
*/
	//	разрешаем тактирование порта C
	//	код на SPL
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	//	можно заменить на:
//	RCC->APB2ENR	|= RCC_APB2ENR_IOPCEN;
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPCEN);
	//	после включения все выводы находятся в состоянии
	//	плавающего (высокоимпедансного) входа (Hi-Z)
	GPIOC->CRH		&= GPIO_CRH_CNF13;
	GPIOC->CRH		|= GPIO_CRH_MODE13_1;
	//	ДЕинициализации HSE/PLL (переход на HSI)
	RCC_DeInit();
	//	инициализация HSE на частоте 72MHz
//	ClockInit();
	//	инициализация HSE на частоте 48MHz
	//	(чтобы USB порт можно было подключить)
	if (RCC_Init_48MHz() == 0)
	//	если получилось включить тактирование 48 MHz
	{
//		trash_acts_1();
//		trash_acts_2();
	//	настройка таймера TIM1
//		TIM1_Init();
		TIM1_PWM_Init(p_vars);
		TIM2_Init_Encoder();
		TIM3_Init_Encoder();
		TIM4_Init_Encoder();
		//	цикл для TIM1
		while(1){
//			trash_acts_3();
			if (Sys_Flag != Sys_Flag_Last)
			{
				GPIOC->ODR ^= GPIO_ODR_ODR13;
			}
//			Enc2_proc(p_vars);
			Enc3_proc(p_vars);
			Sys_Flag_Last = Sys_Flag;
			//			while ((RCC->CFGR & RCC_CFGR_SWS) != (0x02<<RCC_CFGR_SWS_Pos)){}
			//			while (READ_BIT(RCC->CFGR, RCC_CFGR_SWS) != RCC_CFGR_SWS_1){}

			Delay(10000);
			Enc_Ind3++;
			if (Enc_Ind3 > 511)
				Enc_Ind3 = 0;
			if (Enc_Ind3 < 0)
				Enc_Ind3 = 511;
			ccr_ind1 = Enc_Ind3 / 2;
			ccr_ind2 = ccr_ind1 + (85);
			ccr_ind3 = ccr_ind1 + (170);
			ccr_val1 = p_vars->sin_val1[ccr_ind1];
			ccr_val2 = p_vars->sin_val1[ccr_ind2];
			ccr_val3 = p_vars->sin_val1[ccr_ind3];
			WRITE_REG(TIM1->CCR1, ccr_val1);// - 1);
			WRITE_REG(TIM1->CCR2, ccr_val2);// - 1);
			WRITE_REG(TIM1->CCR3, ccr_val3);// - 1);

		}
	}
	//	всякая дичь
//	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
//	NVIC_EnableIRQ(TIM2_IRQn);
//	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM1);
}
