#include"irq.h"

/*
 * @fn			- IRQInterruptConfig
 *
 * @brief		- This function configures the IRQ Interrupt.
 *
 * @param[uint8_t]	- IRQ Number.
 * @param[uint8_t]	- ENABLE or DISABLE macro.
 *
 * @return		- None.
 *
 * @note		- None.
 */
void IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnOrDi)
{
	if(EnOrDi == ENABLE){
		if(IRQNumber <= 31)
			*NVIC_ISER0 |= (1 << IRQNumber);//Program ISER0 register.
		else if(IRQNumber > 31 && IRQNumber < 64)
			*NVIC_ISER1 |= (1 << IRQNumber % 32);//Program ISER1 register.
		else if(IRQNumber >= 64 && IRQNumber < 96)
			*NVIC_ISER3 |= (1 << IRQNumber % 64);//Program ISER2 register.
	}else{
		if(IRQNumber <= 31)
			*NVIC_ICER0 |= (1 << IRQNumber);//Program ICER0 register.
		else if(IRQNumber > 31 && IRQNumber < 64)
			*NVIC_ICER1 |= (1 << IRQNumber % 32);//Program ICER1 register.
		else if(IRQNumber >= 64 && IRQNumber < 96)
			*NVIC_ICER3 |= (1 << IRQNumber % 64);//Program ICER2 register.
	}
}

/*
 * @fn			- IRQPriorityConfig
 *
 * @brief		- This function configures the IRQ Priority.
 *
 * @param[uint8_t]	- IRQ Number.
 * @param[uint8_t]	- IRQ Priority.
 *
 * @return		- None.
 *
 * @note		- None.
 */
void IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority)
{	//Find out IPR register.
	uint8_t iprx = IRQNumber / 4;
	uint8_t iprx_section = IRQNumber % 4;
	uint8_t shift_amount = (8 * iprx_section) + (8 - NO_PR_BITS_IMPLEMENTED);
	*(NVIC_PR_BASEADDR + iprx) |= (IRQPriority << shift_amount);
}
