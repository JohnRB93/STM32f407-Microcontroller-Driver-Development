#ifndef __IRQ_H_
#define __IRQ_H_

#include<stdint.h>
#include"stm32f4xx.h"

//ARM Cortex Mx Processor NVIC ISERx Register Address
#define NVIC_ISER0					((__IO uint32_t*)0xE000E100)
#define NVIC_ISER1					((__IO uint32_t*)0xE000E104)
#define NVIC_ISER2					((__IO uint32_t*)0xE000E108)
#define NVIC_ISER3					((__IO uint32_t*)0xE000E10C)
#define NVIC_ISER4					((__IO uint32_t*)0xE000E110)
#define NVIC_ISER5					((__IO uint32_t*)0xE000E114)
#define NVIC_ISER6					((__IO uint32_t*)0xE000E118)
#define NVIC_ISER7					((__IO uint32_t*)0xE000E11C)

//ARM Cortex Mx Processor NVIC ICERx Register Address
#define NVIC_ICER0					((__IO uint32_t*)0xE000E180)
#define NVIC_ICER1					((__IO uint32_t*)0xE000E184)
#define NVIC_ICER2					((__IO uint32_t*)0xE000E188)
#define NVIC_ICER3					((__IO uint32_t*)0xE000E18C)
#define NVIC_ICER4					((__IO uint32_t*)0xE000E190)
#define NVIC_ICER5					((__IO uint32_t*)0xE000E194)
#define NVIC_ICER6					((__IO uint32_t*)0xE000E198)
#define NVIC_ICER7					((__IO uint32_t*)0xE000E19C)

//ARM Cortex Mx Processor Priority Register Address Calculation
#define NVIC_PR_BASEADDR			((__IO uint32_t*)0xE000E400)

//ARM Cortex Mx Processor number of priority bits implemented in Priority Register
#define NO_PR_BITS_IMPLEMENTED		4

//IRQ Priority Levels of STM32F407xx MCU.
#define NVIC_IRQ_PRIORITY0		0
#define NVIC_IRQ_PRIORITY1		1
#define NVIC_IRQ_PRIORITY2		2
#define NVIC_IRQ_PRIORITY3		3
#define NVIC_IRQ_PRIORITY4		4
#define NVIC_IRQ_PRIORITY5		5
#define NVIC_IRQ_PRIORITY6		6
#define NVIC_IRQ_PRIORITY7		7
#define NVIC_IRQ_PRIORITY8		8
#define NVIC_IRQ_PRIORITY9		9
#define NVIC_IRQ_PRIORITY10		10
#define NVIC_IRQ_PRIORITY11		11
#define NVIC_IRQ_PRIORITY12		12
#define NVIC_IRQ_PRIORITY13		13
#define NVIC_IRQ_PRIORITY14		14
#define NVIC_IRQ_PRIORITY15		15

void IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnOrDi);
void IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority);

#endif /* __IRQ_H_ */
