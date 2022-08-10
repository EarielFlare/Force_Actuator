#include "main.h"

void	Enc2_proc(t_vars *p_vars)
{
	Enc_Cnt2 = TIM2->CNT;
	Enc_Dir2 = (TIM2->CR1 & TIM_CR1_DIR)>>4;
//	Enc_Dir2 = READ_BIT(TIM2->CR1, TIM_CR1_DIR);
	if (Enc_Cnt2 != Enc_Cnt2_Last)
	{
		if (Enc_Dir2 == 1)
			cnt++;
		if (Enc_Dir2 == 0)
			cnt--;
		if (cnt > 15)
			cnt = 0;
		if (cnt < 0)
			cnt = 15;
		psc_div = p_vars->arr_psc_div[cnt / 2];
		Enc_Cnt2_Last = Enc_Cnt2;
		WRITE_REG(TIM1->PSC, SYSCLOCK / psc_div - 1);
	}
//	printf("p_vars->arr[] = %d\n", p_vars->arr[0]);
}

void TIM2_IRQHandler(void)
{
	Sys_Flag ^= 1;
	//	обязательный сброс флага прерывания
	CLEAR_BIT(TIM2->SR, TIM_SR_UIF);
}

void TIM2_Init_Encoder(void)
{
	//	включаем тактирование таймера TIM2 (шина APB1)
	SET_BIT(RCC->APB1ENR, RCC_APB1ENR_TIM2EN);
	//	включаем тактирование порта A
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPAEN);

	//	настройка выхода PA0
	//	сброс CNF на 00
	GPIOA->CRL &= ~GPIO_CRL_CNF0;		//	GPIO_CRL_CNF0 = 1100
	//	input floating: запись в CNF 01
	GPIOA->CRL |= GPIO_CRL_CNF0_0;		//	GPIO_CRL_CNF0_0 = 0100
	//	сброс MODE на 00
	GPIOA->CRL	&= ~GPIO_CRL_MODE0;		//	GPIO_CRL_MODE0 = 11
	//	то же самое для выхода PA1
	GPIOA->CRL &= ~GPIO_CRL_CNF1;		//	GPIO_CRL_CNF1 = 1100
	GPIOA->CRL |= GPIO_CRL_CNF1_0;		//	GPIO_CRL_CNF1_1 = 0100
	GPIOA->CRL	&= ~GPIO_CRL_MODE1;		//	GPIO_CRL_MODE1 = 11

	//	сброс флага прерывания (на всякий случай)
	CLEAR_BIT(TIM2->SR, TIM_SR_UIF);
	//	разрешаем прерывание по переполнению (перезагрузке) счетчика CNT
	//	это нужно для создания запроса на прерывание (!)
	SET_BIT(TIM2->DIER, TIM_DIER_UIE);
	//	глобально разрешаем прерывание
	NVIC_EnableIRQ (TIM2_IRQn);

	//	01: CC1 channel is configured as input, IC1 is mapped on TI1
	//	01: CC2 channel is configured as input, IC2 is mapped on TI2
	TIM2->CCMR1 |= (TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_0);
	TIM2->CCMR1 &= ~(TIM_CCMR1_CC1S_1 | TIM_CCMR1_CC2S_1);
	//	10: CC1 channel is configured as input, IC1 is mapped on TI2
	//	10: CC2 channel is configured as input, IC2 is mapped on TI1
//	TIM2->CCMR1 &= ~(TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_0);
//	TIM2->CCMR1 |= (TIM_CCMR1_CC1S_1 | TIM_CCMR1_CC2S_1);

	//	0: noninverted/rising edge
	TIM2->CCER &= ~(TIM_CCER_CC1P | TIM_CCER_CC2P);
	//	1: inverted/falling edge
//	TIM2->CCER |= (TIM_CCER_CC1P | TIM_CCER_CC2P);

	//	001: Encoder mode 1 - Counter counts up/down
	//	on TI2FP1 edge depending on TI1FP2 level
	TIM2->SMCR |= TIM_SMCR_SMS_0;
	TIM2->SMCR &= ~TIM_SMCR_SMS_1;
	TIM2->SMCR &= ~TIM_SMCR_SMS_2;
	//	010: Encoder mode 2 - Counter counts up/down
	//	on TI1FP1 edge depending on TI2FP2 level
//	TIM2->SMCR &= ~TIM_SMCR_SMS_0;
//	TIM2->SMCR |= TIM_SMCR_SMS_1;
//	TIM2->SMCR &= ~TIM_SMCR_SMS_2;
	//	011: Encoder mode 2 - Counter counts up/down
//	TIM2->SMCR |= TIM_SMCR_SMS_0;
//	TIM2->SMCR |= TIM_SMCR_SMS_1;
//	TIM2->SMCR &= ~TIM_SMCR_SMS_2;

	//	ARR register (MAX counter number)
	TIM2->ARR = 16 - 1;
	//	Counter enabled
	TIM2->CR1 |= TIM_CR1_CEN;
}
