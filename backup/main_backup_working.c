#include "main.h"
#define SYSCLOCK	48000000U
__IO uint32_t SysTick_CNT = 0;
__IO uint8_t Sys_Flag = 0;
__IO uint8_t Enc_Counter = 0;

void	Delay(uint32_t time){
	while (time > 0)
		time--;
}

void	Delay_ms(uint32_t ms){
	MODIFY_REG(SysTick->VAL,SysTick_VAL_CURRENT_Msk, SYSCLOCK / 1000 - 1);
	SysTick_CNT = ms;
	while(SysTick_CNT) {}
}

void	RCC_DeInit(void)
{
	//	включаем тактирование от HSI
	SET_BIT(RCC->CR, RCC_CR_HSION);
	//	ждем, пока HSI включится
	//	бит RCC_CR_HSIRDY изначально = 0,
	//	условие работает, пока (0 == 0)
	//	двинемся дальше, когда будет (1 == 0)
	while (READ_BIT(RCC->CR,	RCC_CR_HSIRDY)	== RESET){}

	//	устанавливаем RCC_CR_HSITRIM в дефолтное 16(dec)
	//	число "16" с учетом смещения от начала = 10000 000
	MODIFY_REG(RCC->CR, RCC_CR_HSITRIM, 1<<7);

	//	очищаем регистр CFGR
	CLEAR_REG(RCC->CFGR);
	//	ждем, пока бит RCC_CFGR_SWS обнулится
	//	бит RCC_CFGR_SWS изначально неизвестен,
	//	условие работает, пока (x != 0)
	//	двинемся дальше, когда будет (0 != 0)
	while (READ_BIT(RCC->CFGR,	RCC_CFGR_SWS)	!= RESET){}

	//	сбрасываем бит RCC_CR_PLLON (отключаем PLL)
	CLEAR_BIT(RCC->CR, RCC_CR_PLLON);
	//	ждем, пока будет сброшен RCC_CR_PLLRDY
	while (READ_BIT(RCC->CR,	RCC_CR_PLLRDY)	!= RESET){}

	//	сбрасываем бит RCC_CR_HSEON (отключаем HSE)
	//	сбрасываем бит RCC_CR_CSSON (отключаем CSS)
	CLEAR_BIT(RCC->CR, RCC_CR_HSEON | RCC_CR_CSSON);
	//	ждем, пока будет сброшен RCC_CR_HSERDY
	while (READ_BIT(RCC->CR,	RCC_CR_HSERDY)	!= RESET){}

	//	сброс бита, отвечающего за внешний источник тактирования
	//	нет тактирования - нет "1" в бите RCC_CR_HSEBYP
	CLEAR_BIT(RCC->CR, RCC_CR_HSEBYP);

	//	запись "1" в RCC_CSR_RMVF сбрасывает весь регистр CSR
	SET_BIT(RCC->CSR, RCC_CSR_RMVF);
}

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

int	RCC_Init_48MHz(void)
//	настраиваем тактирование системы от внешнего
//	кварца 8MHz через PLL на частоте 48MHz
//	функция возвращает:
//  0 - завершено успешно
//  1 - не запустился кварц
//  2 - не запустился PLL
{
	__IO uint32_t	StartUpCounter;
	__IO uint32_t	HSEStartUp_TimeOut;

	HSEStartUp_TimeOut	= 10000;
	//	включаем буфер предвыборки FLASH
	//	хотя после сброса он и так выставлен в "1"
//	FLASH->ACR |= FLASH_ACR_PRFTBE;
	SET_BIT(FLASH->ACR, FLASH_ACR_PRFTBE);
	//	ожидаем установки флага FLASH_ACR_PRFTBS
	//	хотя после сброса он и так выставлен в "1"
	while (READ_BIT(FLASH->ACR,	FLASH_ACR_PRFTBS) == RESET){}

	//	так как частота ядра 48MHz < SYSCLK <= 72MHz
	//	устанавливаем 2 цикла ожидания для Flash (48MHz)
	//	сначала обнуляем содержимое FLASH_ACR_LATENCY: 000
	CLEAR_BIT(FLASH->ACR, FLASH_ACR_LATENCY);
	//	затем устанавливаем флаг FLASH_ACR_LATENCY_1: 010
	SET_BIT(FLASH->ACR, FLASH_ACR_LATENCY_1);

	//	устанавливаем бит RCC_CR_CSSON (включаем CSS)
	SET_BIT(RCC->CR, RCC_CR_CSSON);
	//	запускаем кварцевый генератор (HSE)
//	RCC->CR |= (1<<RCC_CR_HSEON_Pos);
	SET_BIT(RCC->CR, RCC_CR_HSEON);
	//	входной сигнал НЕ делим на 2: RCC_CFGR_PLLXTPRE = 0
//	RCC->CFGR &= (1<<RCC_CFGR_PLLXTPRE_Pos);
	CLEAR_BIT(RCC->CR, RCC_CFGR_PLLXTPRE);

	StartUpCounter = 0;
	do	{
		StartUpCounter++;
	}	while((READ_BIT(RCC->CR, RCC_CR_HSERDY) == 0)
			&& (StartUpCounter != HSEStartUp_TimeOut));
	//	если выход произошел по счетчику, значит HSE не работает
	//	значит, отключаем бит RCC_CR_HSEON и возвращаем "1"
	if (StartUpCounter == HSEStartUp_TimeOut){
		//	останавливаем генератор (HSE)
//		RCC->CR &= ~(1<<RCC_CR_HSEON_Pos);
		CLEAR_BIT(RCC->CR, RCC_CR_HSEON);
		return (1);
	}

	//	HCLK = SYSCLK
	//	AHB prescaler, ставим SYSCLK not divided (0xxx)
//	RCC->CFGR |= (0x00<<RCC_CFGR_HPRE_Pos);
	SET_BIT(RCC->CFGR, RCC_CFGR_HPRE_DIV1);
	//	PCLK2 = HCLK
	//	APB2 prescaler (high-speed), ставим HCLK not divided (0xx)
//	RCC->CFGR |= (0x00<<RCC_CFGR_PPRE2_Pos);
	SET_BIT(RCC->CFGR, RCC_CFGR_PPRE2_DIV1);
	//	PCLK1 = HCLK
	//	APB1 prescaler (low-speed), ставим частоту HCLK/2 (100)
//	RCC->CFGR |= (0x04<<RCC_CFGR_PPRE1_Pos);
	SET_BIT(RCC->CFGR, RCC_CFGR_PPRE1_DIV2);

	//	настраиваем PLL
	//	сначала обнуляем содержимое RCC_CFGR_PLLSRC: 0
	CLEAR_BIT(RCC->CFGR, RCC_CFGR_PLLSRC);
	//	а также обнуляем содержимое RCC_CFGR_PLLMULL: 000x
	CLEAR_BIT(RCC->CFGR, RCC_CFGR_PLLMULL);
	//	затем устанавливаем флаг RCC_CFGR_PLLSRC: 1
	//	т.е. источник тактирования PLL - кварц HSE (на 8MHz)
	SET_BIT(RCC->CFGR, RCC_CFGR_PLLSRC);
	//	а также устанавливаем флаг RCC_CFGR_PLLMULL_2: 0100
	//	поскольку это множитель 6: PLLCLK = HSE * 6 = 48MHz
	SET_BIT(RCC->CFGR, RCC_CFGR_PLLMULL_2);

	//	включаем PLL
//	RCC->CR |= (1<<RCC_CR_PLLON_Pos);
	SET_BIT(RCC->CR, RCC_CR_PLLON);
	//	ожидаем, пока PLL выставит бит готовности
	StartUpCounter = 0;
	do	{
		StartUpCounter++;
	}	while((READ_BIT(RCC->CR, RCC_CR_PLLRDY) == 0)
			&& (StartUpCounter != HSEStartUp_TimeOut));
	//	если выход произошел по счетчику, значит PLL не работает
	//	значит, отключаем бит RCC_CR_PLLON и возвращаем "2"
	if (StartUpCounter == HSEStartUp_TimeOut){
		//	останавливаем генератор (HSE)
//		RCC->CR &= ~(1<<RCC_CR_HSEON_Pos);
		//	останавливаем тактирование (PLL)
//		RCC->CR &= ~(1<<RCC_CR_PLLON_Pos);
		CLEAR_BIT(RCC->CR, RCC_CR_HSEON);
		CLEAR_BIT(RCC->CR, RCC_CR_PLLON);
		return (2);
	}

	//	выбираем PLL как источник системной частоты
	//	сбрасываем бит RCC_CFGR_SW
	//	хотя в этом вообще НЕТ необходимости
//	RCC->CFGR &= ~(RCC_CFGR_SW);
//	CLEAR_BIT(RCC->CFGR, RCC_CFGR_SW);
	//	устанавливаем бит RCC_CFGR_SW_1: 10
// 	RCC->CFGR |= (0x02<<RCC_CFGR_SW_Pos);
//	RCC->CFGR |= RCC_CFGR_SW_1;
	SET_BIT(RCC->CFGR, RCC_CFGR_SW_1);
//	ожидаем установки бита готовности RCC_CFGR_SWS
// 	while ((RCC->CFGR & RCC_CFGR_SWS) != (0x02<<RCC_CFGR_SWS_Pos)){}
	while (READ_BIT(RCC->CFGR, RCC_CFGR_SWS) != RCC_CFGR_SWS_1){}

	//	после переключения на тактирование от PLL
	//	отключаем внутренний RC-генератор (HSI)
//	RCC->CR &= ~(1<<RCC_CR_HSION_Pos);
//	CLEAR_BIT(RCC->CR, RCC_CR_HSION);

	return (0);
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

void SysTick_Handler(void)
{
//	if(SysTick_CNT > 0) SysTick_CNT--;

	Sys_Flag ^= 1;
//	if (Sys_Flag == 0) Sys_Flag = 1;
//	else if (Sys_Flag == 1) Sys_Flag = 0;
}

void	TIM1_Init(void)
//	функция настройки таймера общего назначения (TIM1) на счет
{
	//	включаем тактирование таймера TIM1 (шина APB2)
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM1EN);

	//	сброс флага прерывания (на всякий случай)
	CLEAR_BIT(TIM1->SR, TIM_SR_UIF);
	//	подчинённый режим отключён, SMS = 000; в этом случае
	//	при CEN=1 на предделитель поступает внутренний тактовый сигнал
	CLEAR_BIT(TIM1->SMCR, TIM_SMCR_SMS);
	//	записываем в PSC число 48000 - 1 = 47999
	//	именно столько тиков должно пройти, чтобы CNT сделал один тик
	//	примечание: при PSC = 0 частоты будут равны: CK_CNT = CK_PSC
	WRITE_REG(TIM1->PSC, SYSCLOCK / 1000 - 1);
	//	счетчик CNT будет считать до 500 - 1, чтобы период был = 0.5 сек
	WRITE_REG(TIM1->ARR, 500 - 1);	//	счет идет с 0, тикнет 500 раз
	//	разрешаем прерывание по переполнению (перезагрузке) счетчика CNT
	//	это нужно для создания запроса на прерывание (!)
	SET_BIT(TIM1->DIER, TIM_DIER_UIE);
	//	глобально разрешаем прерывание
	NVIC_EnableIRQ (TIM1_UP_IRQn);

	//	включаем таймер TIM1 (после его настройки)
//	SET_BIT(TIM1->CR1, TIM_CR1_CEN);
	//	здесь будет лучше перезаписать весь регистр CR1
	TIM1->CR1 = TIM_CR1_CEN;
}

void TIM1_UP_IRQHandler(void)
{
	Sys_Flag ^= 1;
	//	обязательный сброс флага прерывания
	CLEAR_BIT(TIM1->SR, TIM_SR_UIF);
}

void TIM2_Init_Encoder(void)
{
	//	включаем тактирование таймера TIM2 (шина APB1)
	SET_BIT(RCC->APB1ENR, RCC_APB1ENR_TIM2EN);
	//	включаем тактирование порта A
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPAEN);

	//	настройка выхода PA0
	GPIOA->CRL &= ~GPIO_CRL_CNF0;		//	GPIO_CRL_CNF0 = 1100
	//	input floating
	GPIOA->CRL |= GPIO_CRL_CNF0_0;		//	GPIO_CRL_CNF0_0 = 0100
	//	input pull-up
//	GPIOA->CRL |= GPIO_CRL_CNF0_1;		//	GPIO_CRL_CNF0_0 = 1000
	//	сброс GPIO_CRH_MODE0 на 00
	GPIOA->CRL	&= ~GPIO_CRL_MODE0;		//	GPIO_CRL_MODE0 = 11
	//	установка GPIO_CRH_MODE0 в 10
//	GPIOA->CRL	|= GPIO_CRL_MODE0_1;	//	GPIO_CRL_MODE0_1 = 10
	//	итого 0100: input floating
//	GPIOA->ODR	|= GPIO_ODR_ODR0;

	//	то же самое для выхода PA1
	GPIOA->CRL &= ~GPIO_CRL_CNF1;		//	GPIO_CRL_CNF1 = 1100
	GPIOA->CRL |= GPIO_CRL_CNF1_0;		//	GPIO_CRL_CNF1_1 = 0100
//	GPIOA->CRL |= GPIO_CRL_CNF1_1;		//	GPIO_CRL_CNF1_1 = 1000
	GPIOA->CRL	&= ~GPIO_CRL_MODE1;		//	GPIO_CRL_MODE1 = 11
//	GPIOA->CRL	|= GPIO_CRL_MODE1_1;	//	GPIO_CRL_MODE1_1 = 10
//	GPIOA->ODR	|= GPIO_ODR_ODR1;

	//	01: CC1 channel is configured as input, IC1 is mapped on TI1
	//	01: CC2 channel is configured as input, IC2 is mapped on TI2
	TIM2->CCMR1 |= (TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_0);
	TIM2->CCMR1 &= ~(TIM_CCMR1_CC1S_1 | TIM_CCMR1_CC2S_1);

	TIM2->CCER &= ~(TIM_CCER_CC1P | TIM_CCER_CC2P);
	TIM2->CCER &= ~(TIM_CCER_CC2NP | TIM_CCER_CC2NP);

//	TIM2->CCER |= (TIM_CCER_CC1E | TIM_CCER_CC2E);
//	TIM2->CCER |= (TIM_CCER_CC1NE | TIM_CCER_CC2NE);
	//	11: inverted/falling edge
//	TIM2->CCER |= (TIM_CCER_CC1P | TIM_CCER_CC2P);
//	TIM2->CCER |= (TIM_CCER_CC1NP | TIM_CCER_CC2NP);

//	TIM2->CCER = TIM_CCER_CC1P | TIM_CCER_CC2P;

	//	001: Encoder mode 1 - Counter counts up/down on TI2FP1 edge depending on TI1FP2 level */
	TIM2->SMCR |= TIM_SMCR_SMS_0;
	TIM2->SMCR &= ~TIM_SMCR_SMS_1;
	TIM2->SMCR &= ~TIM_SMCR_SMS_2;
	//	010: Encoder mode 2 - Counter counts up/down on TI2FP1 edge depending on TI1FP2 level */
//	TIM2->SMCR &= ~TIM_SMCR_SMS_0;
//	TIM2->SMCR |= TIM_SMCR_SMS_1;
//	TIM2->SMCR &= ~TIM_SMCR_SMS_2;
	//	1111: fSAMPLING = fDTS / 32, N = 8
//	TIM3->CCMR1 |= (TIM_CCMR1_IC1F_0 | TIM_CCMR1_IC1F_1 | TIM_CCMR1_IC1F_2 | TIM_CCMR1_IC1F_3);
//	TIM3->CCMR1 |= (TIM_CCMR1_IC2F_0 | TIM_CCMR1_IC2F_1 | TIM_CCMR1_IC2F_2 | TIM_CCMR1_IC2F_3);
	//	Auto-Reload Register (MAX counter number)
	TIM2->ARR = 100 - 1;
	//	Counter enabled
	TIM2->CR1 |= TIM_CR1_CEN;
}

void	TIM1_PWM_Init(void)
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
	WRITE_REG(TIM1->PSC, SYSCLOCK / 10000 - 1);
	//	счетчик CNT будет считать до 100 - 1, чтобы период был = 10 мс
	WRITE_REG(TIM1->ARR, 100 - 1);	//	счет идет с 0, тикнет 100 раз
	//	прописываем значение 20 - 1 в регистр сравнения,
	//	чтобы заполнение сигнала ШИМ была ровно 20%
	WRITE_REG(TIM1->CCR1, 70 - 1);
	WRITE_REG(TIM1->CCR2, 70 - 1);
	WRITE_REG(TIM1->CCR3, 10 - 1);
//	WRITE_REG(TIM1->CCR1, 50 - 1);
//	WRITE_REG(TIM1->CCR2, 50 - 1);
//	WRITE_REG(TIM1->CCR3, 50 - 1);

	//	устанавливаем бит TIM_BDTR_MOE, чтобы можно было
	//	использовать выводы таймера TIM1 как выходы
//	TIM1->BDTR |= TIM_BDTR_MOE;
	SET_BIT(TIM1->BDTR, TIM_BDTR_MOE);

	//	включаем канал CH1 в режиме формирования ШИМ-сигнала
	//	совмещая два флага, создаем значение 110 (см. доки)
//	TIM1->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2;
	SET_BIT(TIM1->CCMR1, TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2);
	//	включаем канал CH2 в режиме формирования ШИМ-сигнала
	//	совмещая два флага, создаем значение 110 (см. доки)
	SET_BIT(TIM1->CCMR1, TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2);
	//	включаем канал CH3 в режиме формирования ШИМ-сигнала
	//	совмещая два флага, создаем значение 110 (см. доки)
	SET_BIT(TIM1->CCMR2, TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2);

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

int main(void)
{
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
/*
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
*/
		//	настройка системного таймера SysTick
//		SysTick_Init();
/*
		//	цикл для SysTick
		while(1){
			if (Sys_Flag == 0)
			// _BR13 - включает светодиод
			GPIOC->BSRR = GPIO_BSRR_BR13;
			if (Sys_Flag == 1)
			// _BS13 -  выключает светодиод
			GPIOC->BSRR = GPIO_BSRR_BS13;
		}
*/
		//	настройка таймера TIM1
//		TIM1_Init();
		TIM1_PWM_Init();
		TIM2_Init_Encoder();
		//	цикл для TIM1
		while(1){
			if (Sys_Flag == 0)
			// _BR13 - включает светодиод
			GPIOC->BSRR = GPIO_BSRR_BR13;
			if (Sys_Flag == 1)
			// _BS13 -  выключает светодиод
			GPIOC->BSRR = GPIO_BSRR_BS13;
			Enc_Counter = TIM2->CNT;
		}
	}
	//	всякая дичь
//	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
//	NVIC_EnableIRQ(TIM2_IRQn);
//	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM1);
}
