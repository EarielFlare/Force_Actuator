#include "main.h"

void	TIM1_PWM_Init(t_vars *p_vars)
//	функция инициализации ШИМ на таймере TIM1
{
	//	включаем тактирование таймера TIM1 (шина APB2)
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM1EN);
	//	включаем тактирование порта A
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPAEN);

	//	после рестарта все порты по умолчанию находятся
	//	в состоянии Input Floating Mode (0100): CNF = 01, MODE = 00
	//	настройка PA8 как push-pull
	//	достаточно сбросить все на 00
	GPIOA->CRH &= ~GPIO_CRH_CNF8;		//	GPIO_CRH_CNF8 = 1100
	//	альтернативная функция ЕЩЕ КАК НУЖНА БЛТ! пздц подня искал косяк!
	GPIOA->CRH |= GPIO_CRH_CNF8_1;		//	GPIO_CRH_CNF8_1 = 1000
	//	сброс GPIO_CRH_MODE8 на 00
	GPIOA->CRH	&= ~GPIO_CRH_MODE8;		//	GPIO_CRH_MODE8 = 11
	//	установка GPIO_CRH_MODE8 в 10
	GPIOA->CRH	|= GPIO_CRH_MODE8_1;	//	GPIO_CRH_MODE8_1 = 10
	//	итого 1010: alter function output push-pull, 2MHz output speed

	//	то же самое для порта PA9
	GPIOA->CRH &= ~GPIO_CRH_CNF9;		//	GPIO_CRH_CNF9 = 1100
	GPIOA->CRH |= GPIO_CRH_CNF9_1;		//	GPIO_CRH_CNF9_1 = 1000
	GPIOA->CRH	&= ~GPIO_CRH_MODE9;		//	GPIO_CRH_MODE9 = 11
	GPIOA->CRH	|= GPIO_CRH_MODE9_1;	//	GPIO_CRH_MODE9_1 = 10

	//	то же самое для порта PA10
	GPIOA->CRH &= ~GPIO_CRH_CNF10;		//	GPIO_CRH_CNF9 = 1100
	GPIOA->CRH |= GPIO_CRH_CNF10_1;		//	GPIO_CRH_CNF9_1 = 1000
	GPIOA->CRH	&= ~GPIO_CRH_MODE10;		//	GPIO_CRH_MODE9 = 11
	GPIOA->CRH	|= GPIO_CRH_MODE10_1;	//	GPIO_CRH_MODE9_1 = 10

	//	сброс флага прерывания (на всякий случай)
	CLEAR_BIT(TIM1->SR, TIM_SR_UIF);
	//	подчинённый режим отключён, SMS = 000; в этом случае
	//	при CEN=1 на предделитель поступает внутренний тактовый сигнал
	CLEAR_BIT(TIM1->SMCR, TIM_SMCR_SMS);
	//	запрещаем прерывание по переполнению (перезагрузке) счетчика CNT,
	//	если эти прерывания были разрешены ранее
	WRITE_REG(TIM1->DIER, 0);
	//	глобально запрещаем прерывание
	NVIC_DisableIRQ (TIM1_UP_IRQn);

	//	обнуляем весь регистр CCER (выключаем все каналы)
	TIM1->CCER = 0;
	//	записываем в PSC число 4800 - 1 = 4799
	//	именно столько тиков должно пройти, чтобы CNT сделал один тик
	//	примечание: при PSC = 0 частоты будут равны: CK_CNT = CK_PSC
//	WRITE_REG(TIM1->PSC, SYSCLOCK / 10000 - 1);
	WRITE_REG(TIM1->PSC, SYSCLOCK / psc_div - 1);
	//	счетчик CNT будет считать до 100 - 1, чтобы период был = 10 мс
//	WRITE_REG(TIM1->ARR, 100 - 1);	//	счет идет с 0, тикнет 100 раз
	WRITE_REG(TIM1->ARR, 256 - 1);	//	счет идет с 0, тикнет 100 раз
	//	прописываем значение 20 - 1 в регистр сравнения,
	//	чтобы заполнение сигнала ШИМ была ровно 20%
//	WRITE_REG(TIM1->CCR1, 70 - 1);
//	WRITE_REG(TIM1->CCR2, 70 - 1);
//	WRITE_REG(TIM1->CCR3, 10 - 1);
//	WRITE_REG(TIM1->CCR1, 50 - 1);
//	WRITE_REG(TIM1->CCR2, 50 - 1);
//	WRITE_REG(TIM1->CCR3, 50 - 1);
/*
	ccr_ind1 = Enc_Ind3 / 2;
	ccr_ind2 = ccr_ind1 + 85;
	ccr_ind3 = ccr_ind1 + 170;
	ccr_val1 = p_vars->sin_val[ccr_ind1];
	ccr_val2 = p_vars->sin_val[ccr_ind2];
	ccr_val3 = p_vars->sin_val[ccr_ind3];
	WRITE_REG(TIM1->CCR1, p_vars->sin_val[ccr_ind1]);// - 1);
	WRITE_REG(TIM1->CCR2, p_vars->sin_val[ccr_ind2]);// - 1);
	WRITE_REG(TIM1->CCR3, p_vars->sin_val[ccr_ind3]);// - 1);
*/
	WRITE_REG(TIM1->CCR1, 190);
	WRITE_REG(TIM1->CCR2, 1);
	WRITE_REG(TIM1->CCR3, 190);

	//	устанавливаем бит TIM_BDTR_MOE, чтобы можно было
	//	использовать выводы таймера TIM1 как выходы
//	TIM1->BDTR |= TIM_BDTR_MOE;
	SET_BIT(TIM1->BDTR, TIM_BDTR_MOE);

	//	включаем канал CH1 в режиме формирования ШИМ-сигнала
	//	совмещая два флага, создаем значение 110 (см. доки)
//	TIM1->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2;
	SET_BIT(TIM1->CCMR1, TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2);
	//	включаем буферизацию регистра CCR1 (OC1PE = 1)
	SET_BIT(TIM1->CCMR1, TIM_CCMR1_OC1PE);
	//	включаем канал CH2 в режиме формирования ШИМ-сигнала
	//	совмещая два флага, создаем значение 110 (см. доки)
	SET_BIT(TIM1->CCMR1, TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2);
	//	включаем буферизацию регистра CCR2 (OC2PE = 1)
	SET_BIT(TIM1->CCMR1, TIM_CCMR1_OC2PE);
	//	включаем канал CH3 в режиме формирования ШИМ-сигнала
	//	совмещая два флага, создаем значение 110 (см. доки)
	SET_BIT(TIM1->CCMR2, TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2);
	//	включаем буферизацию регистра CCR3 (OC3PE = 1)
	SET_BIT(TIM1->CCMR2, TIM_CCMR2_OC3PE);
	//	ставим бит включения режима предзагрузки ARPE,
	//	а также устанавливаем режим CMS = 11 (счет ведется вверх и вниз)
	TIM1->CR1=TIM_CR1_ARPE | TIM_CR1_CMS_1 | TIM_CR1_CMS_0;

	//	включаем нужные каналы (сейчас это CH1)
//	TIM1->CCER |= TIM_CCER_CC1E;
	SET_BIT(TIM1->CCER, TIM_CCER_CC1E);
	//	включаем нужные каналы (сейчас это CH2)
	SET_BIT(TIM1->CCER, TIM_CCER_CC2E);
	//	включаем нужные каналы (сейчас это CH3)
	SET_BIT(TIM1->CCER, TIM_CCER_CC3E);

	//	включаем таймер TIM1 (после его настройки)
	//считаем вверх
//	TIM1->CR1 &= ~TIM_CR1_DIR;
	//выравнивание по фронту, Fast PWM
//	TIM1->CR1 &= ~TIM_CR1_CMS;
	//включаем счётчик
	SET_BIT(TIM1->CR1, TIM_CR1_CEN);
	//	здесь будет лучше перезаписать весь регистр CR1
//	TIM1->CR1 = TIM_CR1_CEN;
}
