#ifndef __GPIO_H_
#define __GPIO_H_

#include"stm32f4xx.h"

//This is a configuration structure for a GPIO pin
typedef struct
{
	__IO uint8_t GPIO_PinNumber;			/*!<Possible values from @GPIO_PIN_NUMBERS>*/
	__IO uint8_t GPIO_PinMode;				/*!<Possible values from @GPIO_PIN_MODES>*/
	__IO uint8_t GPIO_PinSpeed;				/*!<Possible values from @GPIO_PIN_SPEED>*/
	__IO uint8_t GPIO_PinPuPdControl;		/*!<Possible values from @GPIO_PIN_PULL_UP_PULL_DOWN>*/
	__IO uint8_t GPIO_PinOPType;			/*!<Possible values from @GPIO_PIN_OUTPUT_TYPE>*/
	__IO uint8_t GPIO_PinAltFunMode;		/*!<Possible values from @GPIO_ALT_FUNCTION_NUMBER>*/
}GPIO_PinConfig_t;


//GPIO pin numbers
//@GPIO_PIN_NUMBERS
#define GPIO_PIN_NO_0		0
#define GPIO_PIN_NO_1		1
#define GPIO_PIN_NO_2		2
#define GPIO_PIN_NO_3		3
#define GPIO_PIN_NO_4		4
#define GPIO_PIN_NO_5		5
#define GPIO_PIN_NO_6		6
#define GPIO_PIN_NO_7		7
#define GPIO_PIN_NO_8		8
#define GPIO_PIN_NO_9		9
#define GPIO_PIN_NO_10		10
#define GPIO_PIN_NO_11		11
#define GPIO_PIN_NO_12		12
#define GPIO_PIN_NO_13		13
#define GPIO_PIN_NO_14		14
#define GPIO_PIN_NO_15		15

//GPIO pin possible modes
//@GPIO_PIN_MODES
#define GPIO_MODE_IN		0			//Input Mode
#define GPIO_MODE_OUT		1			//Output Mode
#define GPIO_MODE_ALTFN		2			//Alternate Function Mode
#define GPIO_MODE_ANALOG	3			//Analog Mode
#define GPIO_MODE_IT_FT		4			//Interrupt Falling Edge Trigger
#define GPIO_MODE_IT_RT		5			//Interrupt Rising Edge Trigger
#define GPIO_MODE_IT_RFT	6			//Interrupt Rising Falling Edge Trigger

//GPIO pin possible output types
//@GPIO_PIN_OUTPUT_TYPE
#define GPIO_OP_TYPE_PP		0			//Output push-pull
#define GPIO_OP_TYPE_OD		1			//Output open-drain

//GPIO pin possible output speeds
//@GPIO_PIN_SPEED
#define GPIO_SPEED_LOW		0			//Low speed
#define GPIO_SPEED_MEDIUM	1			//Medium speed
#define GPIO_SPEED_FAST		2			//High speed
#define GPIO_SPEED_HIGH		3			//Very High speed

//GPIO pin pull up / pull down configuration macros
//@GPIO_PIN_PULL_UP_PULL_DOWN
#define GPIO_NO_PUPD		0			//No pull-up, pull-down
#define GPIO_PIN_PU			1			//Pull up
#define GPIO_PIN_PD			2			//Pull down

//GPIO alternate function numbers
//@GPIO_ALT_FUNCTION_NUMBER
#define GPIO_AF_00			0
#define GPIO_AF_01			1
#define GPIO_AF_02			2
#define GPIO_AF_03			3
#define GPIO_AF_04			4
#define GPIO_AF_05			5
#define GPIO_AF_06			6
#define GPIO_AF_07			7
#define GPIO_AF_08			8
#define GPIO_AF_09			9
#define GPIO_AF_10			10
#define GPIO_AF_11			11
#define GPIO_AF_12			12
#define GPIO_AF_13			13
#define GPIO_AF_14			14
#define GPIO_AF_15			15

/*******************************************************************************************************************
* 										APIs supported by this driver											   *
*******************************************************************************************************************/

//Peripheral Clock Setup
void GPIO_PeriClockControl(GPIO_TypeDef *GPIOx, uint8_t enOrDis);

//Initialization and De-initialization
void GPIO_Init(GPIO_TypeDef *GPIOx, GPIO_PinConfig_t gpioConfig);
void GPIO_DeInit(GPIO_TypeDef *GPIOx);

//Read Data and Write Data
uint8_t GPIO_ReadFromInputPin(GPIO_TypeDef *GPIOx, uint8_t pinNumber);
uint16_t GPIO_ReadFromInputPort(GPIO_TypeDef *GPIOx);
void GPIO_WriteToOutputPin(GPIO_TypeDef *GPIOx, uint8_t pinNumber, uint8_t value);
void GPIO_WriteToOutputPort(GPIO_TypeDef *GPIOx, uint16_t value);
void GPIO_ToggleOutputPin(GPIO_TypeDef *GPIOx, uint8_t pinNumber);
void GPIO_LockRegister(GPIO_TypeDef *GPIOx, uint8_t *pinNumber, uint8_t length);

//IRQ Hhandling
void EXTI0_IRQHandling(void);
void EXTI1_IRQHandling(void);
void EXTI2_IRQHandling(void);
void EXTI3_IRQHandling(void);

#endif /* __GPIO_H_ */
