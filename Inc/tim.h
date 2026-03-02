#ifndef TIM_H_
#define TIM_H_

#include"stm32f4xx.h"

//TIM2_5 Configuration Structure
typedef struct
{
	__IO uint8_t TIM2_5_Prescaler;
	__IO uint8_t TIM2_5_CounterDirection;
	__IO uint16_t TIM2_5_ReloadValue;
}TIM2_5_Config_t;


/***************** Macro Definitions ***************************************************/

// Counter Direction
#define COUNT_UP					0
#define COUNT_DOWN					1

// Pulse Mode Option
#define PULSE_MODE_DISABLE			0
#define PULSE_MODE_ENABLE			1


/***************************************************************************************/
/*                         APIs supported by this driver                               */
/***************************************************************************************/

void TIM2_5_Init(TIM_TypeDef *TIM, uint8_t pulseMode);
void TIM2_5_Delay_us(TIM_TypeDef *TIM, uint32_t us);
void TIM2_5_Delay_ms(TIM_TypeDef *TIM, uint32_t ms);
void TIM2_5_Pulse_us(TIM_TypeDef *TIM, uint32_t us);
void TIM2_5_Pulse_ms(TIM_TypeDef *TIM, uint32_t us);

void TIM2_IRQHandling(TIM_TypeDef *TIM);

#endif /* TIM_H_ */
