#ifndef MAIN_H
# define MAIN_H
# include "stm32f1xx.h"
# include <stdlib.h>
# include <math.h>
# include <stdio.h>

# define SYSCLOCK	48000000U
__IO uint32_t	SysTick_CNT;
__IO uint8_t	Sys_Flag;
__IO uint8_t	Sys_Flag_Last;
__IO uint8_t	Enc_Dir2;
__IO uint8_t	Enc_Dir3;
__IO uint8_t	Enc_Cnt2;
__IO uint8_t	Enc_Cnt2_Last;
__IO uint8_t	Enc_Cnt3;
__IO uint8_t	Enc_Cnt3_Last;
__IO int8_t		cnt;
__IO int16_t	Enc_Ind3;
__IO uint32_t	psc_div;
__IO uint8_t	ccr_ind1;
__IO uint8_t	ccr_ind2;
__IO uint8_t	ccr_ind3;
__IO uint8_t	ccr_val1;
__IO uint8_t	ccr_val2;
__IO uint8_t	ccr_val3;
typedef struct s_vars
{
	__IO uint32_t	arr_psc_div[8];
//	синусоида
	__IO int8_t	sin_val1[256];
//	трапеция с плоским верхом 1/6Т
//	__IO uint8_t	sin_val1[256];
//	трапеция с плоским верхом 2/6Т
//	__IO uint8_t	sin_val1[256];
//	седловидный график
//	__IO uint8_t	sin_val1[256];	//	седло
	__IO int16_t	sin_val0[256];	//	sin
	__IO int16_t	sin_val2[256];	//	пила
}	t_vars;

void	Delay(uint32_t time);
void	Delay_ms(uint32_t ms);
void	SysTick_Handler(void);
void	SysTick_Init(void);
int		ClockInit(void);
void	RCC_DeInit(void);
int		RCC_Init_48MHz(void);
void	TIM1_UP_IRQHandler(void);
void	TIM1_Init(void);
void	Enc2_proc(t_vars *p_vars);
void	TIM2_IRQHandler(void);
void	TIM2_Init_Encoder(void);
void	Enc3_proc(t_vars *p_vars);
void	TIM3_IRQHandler(void);
void	TIM3_Init_Encoder(void);
void	TIM4_IRQHandler(void);
void	TIM4_Init_Encoder(void);
void	TIM1_PWM_Init(t_vars *p_vars);
void	trash_acts_1();
void	trash_acts_2();
void	trash_acts_4();
void	global_var_init(t_vars *p_vars);
//void	shape_1(t_vars *p_vars);
void	shape_2(t_vars *p_vars);
void	shape_3(t_vars *p_vars);
void	shape_4(t_vars *p_vars);
void	shape_5(t_vars *p_vars);
void	shape_6(t_vars *p_vars);

#endif
