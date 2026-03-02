#include"tim.h"


void TIM2_5_Init(TIM_TypeDef *TIM, uint8_t pulseMode)
{
	// Enable clock access to the appropriate timer
	if(TIM == TIM2)
		RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	else if (TIM == TIM3)
		RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	else if (TIM == TIM4)
		RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
	else if (TIM == TIM5)
		RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;

	// Set the prescaler
	TIM->PSC = 16U - 1U;
	// Make sure Update Disable is cleared
	TIM->CR1 &= ~TIM_CR1_UDIS;
	// Check if pulse mode is to be enabled
	if(pulseMode == PULSE_MODE_ENABLE)
	{	// Enable update interrupts
		TIM->DIER |= TIM_DIER_UIE;
		// Enable one pulse mode so the counter enable bit clears after an event
		TIM->CR1 |= TIM_CR1_OPM;
	}
}

void TIM2_5_Delay_us(TIM_TypeDef *TIM, uint32_t us)
{
	// Set the auto-reload value to desired us
	TIM->ARR = us - 1U;
	// Clear counter
	TIM->CNT = 0;
	// Enable the counter
	TIM->CR1 |= TIM_CR1_CEN;
	// Wait for the event flag to be set
	while(!((TIM->SR >> 0) & 1)){}
	// Clear event flag
	TIM->SR &= ~TIM_SR_UIF;
	// Disable the counter
	TIM->CR1 &= ~TIM_CR1_CEN;
}
void TIM2_5_Delay_ms(TIM_TypeDef *TIM, uint32_t ms)
{

}

void TIM2_5_Pulse_us(TIM_TypeDef *TIM, uint32_t us)
{
	// Set the auto-reload value to desired us
	TIM->ARR = us - 1U;
	// Clear counter
	TIM->CNT = 0;
	// Enable the counter
	TIM->CR1 |= TIM_CR1_CEN;
}

void TIM2_5_Pulse_ms(TIM_TypeDef *TIM, uint32_t ms)
{

}

/***************************************************************************************/


/***************** TIMx IRQ Handling ***************************************************/

/* Weak function that can be implemented in user application. */
void __attribute__((weak)) TIM_ApplicationEventCallback(void){}

void TIM2_IRQHandling(TIM_TypeDef *TIM)
{
	if(((TIM->SR >> 0) & 0x1) == 1)
	{
		// Clear update interrupt flag
		TIM->SR &= ~TIM_SR_UIF;
		TIM_ApplicationEventCallback();
	}
}


