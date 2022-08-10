#include "main.h"

void	Enc3_proc(t_vars *p_vars)
{
	Enc_Cnt3 = TIM3->CNT;
	Enc_Dir3 = (TIM3->CR1 & TIM_CR1_DIR)>>4;
//	Enc_Dir3 = READ_BIT(TIM3->CR1, TIM_CR1_DIR);

	if (Enc_Cnt3 != Enc_Cnt3_Last)
	{
		if (Enc_Dir3 == 1)
			Enc_Ind3++;
		if (Enc_Dir3 == 0)
			Enc_Ind3--;
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
		Enc_Cnt3_Last = Enc_Cnt3;
	}

}

void TIM3_IRQHandler(void)
{
	Sys_Flag ^= 1;
	//	обязательный сброс флага прерывания
	CLEAR_BIT(TIM3->SR, TIM_SR_UIF);
}

void TIM3_Init_Encoder(void)
{
	//	включаем тактирование таймера TIM3 (шина APB1)
	SET_BIT(RCC->APB1ENR, RCC_APB1ENR_TIM3EN);
	//	включаем тактирование порта A
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPAEN);

	//	настройка выхода PA6
	//	сброс CNF на 00
	GPIOA->CRL &= ~GPIO_CRL_CNF6;		//	GPIO_CRL_CNF0 = 1100
	//	input floating: запись в CNF 01
	GPIOA->CRL |= GPIO_CRL_CNF6_0;		//	GPIO_CRL_CNF0_0 = 0100
	//	сброс MODE на 00
	GPIOA->CRL	&= ~GPIO_CRL_MODE6;		//	GPIO_CRL_MODE0 = 11
	//	то же самое для выхода PA7
	GPIOA->CRL &= ~GPIO_CRL_CNF7;		//	GPIO_CRL_CNF1 = 1100
	GPIOA->CRL |= GPIO_CRL_CNF7_0;		//	GPIO_CRL_CNF1_1 = 0100
	GPIOA->CRL	&= ~GPIO_CRL_MODE7;		//	GPIO_CRL_MODE1 = 11

	//	сброс флага прерывания (на всякий случай)
	CLEAR_BIT(TIM3->SR, TIM_SR_UIF);
	//	разрешаем прерывание по переполнению (перезагрузке) счетчика CNT
	//	это нужно для создания запроса на прерывание (!)
	SET_BIT(TIM3->DIER, TIM_DIER_UIE);
	//	глобально разрешаем прерывание
	NVIC_EnableIRQ (TIM3_IRQn);

	//	01: CC1 channel is configured as input, IC1 is mapped on TI1
	//	01: CC2 channel is configured as input, IC2 is mapped on TI2
	TIM3->CCMR1 |= (TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_0);
	TIM3->CCMR1 &= ~(TIM_CCMR1_CC1S_1 | TIM_CCMR1_CC2S_1);
	//	10: CC1 channel is configured as input, IC1 is mapped on TI2
	//	10: CC2 channel is configured as input, IC2 is mapped on TI1
//	TIM3->CCMR1 &= ~(TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_0);
//	TIM3->CCMR1 |= (TIM_CCMR1_CC1S_1 | TIM_CCMR1_CC2S_1);

	//	0: noninverted/rising edge
	TIM3->CCER &= ~(TIM_CCER_CC1P | TIM_CCER_CC2P);
	//	1: inverted/falling edge
//	TIM3->CCER |= (TIM_CCER_CC1P | TIM_CCER_CC2P);

	//	001: Encoder mode 1 - Counter counts up/down
	//	on TI2FP1 edge depending on TI1FP2 level
	TIM3->SMCR |= TIM_SMCR_SMS_0;
	TIM3->SMCR &= ~TIM_SMCR_SMS_1;
	TIM3->SMCR &= ~TIM_SMCR_SMS_2;
	//	010: Encoder mode 2 - Counter counts up/down
	//	on TI1FP1 edge depending on TI2FP2 level
//	TIM3->SMCR &= ~TIM_SMCR_SMS_0;
//	TIM3->SMCR |= TIM_SMCR_SMS_1;
//	TIM3->SMCR &= ~TIM_SMCR_SMS_2;
	//	011: Encoder mode 2 - Counter counts up/down
//	TIM3->SMCR |= TIM_SMCR_SMS_0;
//	TIM3->SMCR |= TIM_SMCR_SMS_1;
//	TIM3->SMCR &= ~TIM_SMCR_SMS_2;

	//	ARR register (MAX counter number)
	TIM3->ARR = 512 - 1;
	//	Counter enabled
	TIM3->CR1 |= TIM_CR1_CEN;
}
