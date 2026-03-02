#include"gpio.h"
#include"irq.h"

/***************** Private Helper Function Headers *************************************/
static uint8_t GPIO_BaseAddressToCode(GPIO_TypeDef *GPIOx);
static uint8_t GPIO_PinNumberToExtiCode(uint8_t pinNum);


/*********************Peripheral Clock Setup***************************************/
/*
 * @fn			- GPIO_PeriClockControl
 *
 * @brief		- This function enables or disables peripheral clock for the given GPIO port.
 *
 * @param[GPIO_TypeDef*]	- Base address of the GPIO peripheral.
 * @param[uint8_t]			- ENABLE or DISABLE macros.
 *
 * @return		- None.
 *
 * @note		- None.
 */
void GPIO_PeriClockControl(GPIO_TypeDef *GPIOx, uint8_t enOrDis)
{
	if(enOrDis == ENABLE)
	{
		if(GPIOx == GPIOA)
			RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
		else if(GPIOx == GPIOB)
			RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
		else if(GPIOx == GPIOC)
			RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
		else if(GPIOx == GPIOD)
			RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
		else if(GPIOx == GPIOE)
			RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
		else if(GPIOx == GPIOF)
			RCC->AHB1ENR |= RCC_AHB1ENR_GPIOFEN;
		else if(GPIOx == GPIOG)
			RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN;
		else if(GPIOx == GPIOH)
			RCC->AHB1ENR |= RCC_AHB1ENR_GPIOHEN;
		else if(GPIOx == GPIOI)
			RCC->AHB1ENR |= RCC_AHB1ENR_GPIOIEN;
	}
	else
	{
		if(GPIOx == GPIOA)
			RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOAEN;
		else if(GPIOx == GPIOB)
			RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOBEN;
		else if(GPIOx == GPIOC)
			RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOCEN;
		else if(GPIOx == GPIOD)
			RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIODEN;
		else if(GPIOx == GPIOE)
			RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOEEN;
		else if(GPIOx == GPIOF)
			RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOFEN;
		else if(GPIOx == GPIOG)
			RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOGEN;
		else if(GPIOx == GPIOH)
			RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOHEN;
		else if(GPIOx == GPIOI)
			RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOIEN;
	}
}


/*********************Initialization and De-initialization*************************/
/*
 * @fn			- GPIO_Init
 *
 * @brief		- This function initializes the GPIO handle.
 *
 * @param[GPIO_Handle_t*]	- Base address of the GPIO handle.
 *
 * @return		- None.
 *
 * @note		- None.
 */
void GPIO_Init(GPIO_TypeDef *GPIOx, GPIO_PinConfig_t gpioConfig)
{
	uint32_t temp = 0;

	//Enable the peripheral clock.
	GPIO_PeriClockControl(GPIOx, ENABLE);

	//1. Configure the mode of the GPIO pin
	if (!(gpioConfig.GPIO_PinMode >= GPIO_MODE_IT_FT)) //Non-Interrupt Mode.
	{
		temp = (gpioConfig.GPIO_PinMode << (2 * gpioConfig.GPIO_PinNumber));
		GPIOx->MODER &= ~(0x3 << gpioConfig.GPIO_PinNumber); //Clearing
		GPIOx->MODER |= temp; //Setting
	}
	else //Interrupt Mode.
	{
		//Configure GPIOx port mode to input mode.
		GPIOx->MODER &= ~(0x3U << (2 * gpioConfig.GPIO_PinNumber));

		//Enable clock access to SYSCFG and select PORTx for EXTIx.
		uint8_t pinNumDiv4, pinNumMod4, portCode;
		pinNumDiv4 = gpioConfig.GPIO_PinNumber / 4;
		pinNumMod4 = gpioConfig.GPIO_PinNumber % 4;
		portCode = GPIO_BaseAddressToCode(GPIOx);
		RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
		SYSCFG->EXTICR[pinNumDiv4] |= (portCode << (pinNumMod4 * 4));

		//Unmask the EXTIx using Interrupt Mask Register (IMR).
		EXTI->IMR |= (1 << gpioConfig.GPIO_PinNumber);

		//Configure Rising Edge, Falling Edge, or both.
		if(gpioConfig.GPIO_PinMode == GPIO_MODE_IT_FT)
		{//Configure for Falling Edge Trigger.
			EXTI->FTSR |= (1 << gpioConfig.GPIO_PinNumber);
			//Clear the corresponding RTSR bit.
			EXTI->RTSR &= ~(1 << gpioConfig.GPIO_PinNumber);
		}
		else if(gpioConfig.GPIO_PinMode == GPIO_MODE_IT_RT)
		{//Configure for Rising Edge Trigger.
			EXTI->RTSR |= (1 << gpioConfig.GPIO_PinNumber);
			//Clear the corresponding FTSR bit.
			EXTI->FTSR &= ~(1 << gpioConfig.GPIO_PinNumber);
		}
		else if(gpioConfig.GPIO_PinMode == GPIO_MODE_IT_RFT)
		{//Configure both FTSR and RTSR.
			EXTI->RTSR |= (1 << gpioConfig.GPIO_PinNumber);
			EXTI->FTSR |= (1 << gpioConfig.GPIO_PinNumber);
		}

		// Enable EXTI Line
		temp = GPIO_PinNumberToExtiCode(gpioConfig.GPIO_PinNumber);
		IRQInterruptConfig(temp, ENABLE);
	}

	temp = 0;

	//2. Configure the speed
	temp = (gpioConfig.GPIO_PinSpeed << (2 * gpioConfig.GPIO_PinNumber));
	GPIOx->OSPEEDR &= ~(0x3 << gpioConfig.GPIO_PinNumber); //Clearing
	GPIOx->OSPEEDR |= temp; //Setting
	temp = 0;

	//3. Configure the pull up / pull down settings
	temp = (gpioConfig.GPIO_PinPuPdControl << (2 * gpioConfig.GPIO_PinNumber));
	GPIOx->PUPDR &= ~(0x3 << gpioConfig.GPIO_PinNumber); //Clearing
	GPIOx->PUPDR |= temp; //Setting
	temp = 0;

	//4. Configure the optype
	temp = (gpioConfig.GPIO_PinOPType << gpioConfig.GPIO_PinNumber);
	GPIOx->OTYPER &= ~(0x1 << gpioConfig.GPIO_PinNumber); //Clearing
	GPIOx->OTYPER |= temp; //Setting
	temp = 0;

	//5. Configure the alternate functionality
	if(gpioConfig.GPIO_PinMode == GPIO_MODE_ALTFN)
	{
		uint8_t temp1, temp2;
		temp1 = (gpioConfig.GPIO_PinNumber / 8);
		temp2 = (gpioConfig.GPIO_PinNumber % 8);
		GPIOx->AFR[temp1] &= ~(0xF << (4 * temp2)); //Clearing
		GPIOx->AFR[temp1] |= (gpioConfig.GPIO_PinAltFunMode << (4 * temp2)); //Setting
	}
}

/*
 * @fn			- GPIO_DeInit
 *
 * @brief		- This function de-initializes the GPIO handle.
 *
 * @param[GPIO_TypeDef*]	- Base address of the GPIO handle.
 *
 * @return		- None.
 *
 * @note		- None.
 */
void GPIO_DeInit(GPIO_TypeDef *GPIOx)
{
	if(GPIOx == GPIOA)
	{
		RCC->AHB1RSTR |= RCC_AHB1RSTR_GPIOARST;
		RCC->AHB1RSTR &= ~RCC_AHB1RSTR_GPIOARST;
	}else if(GPIOx == GPIOB)
	{
		RCC->AHB1RSTR |= RCC_AHB1RSTR_GPIOBRST;
		RCC->AHB1RSTR &= ~RCC_AHB1RSTR_GPIOBRST;
	}else if(GPIOx == GPIOC)
	{
		RCC->AHB1RSTR |= RCC_AHB1RSTR_GPIOCRST;
		RCC->AHB1RSTR &= ~RCC_AHB1RSTR_GPIOCRST;
	}else if(GPIOx == GPIOD)
	{
		RCC->AHB1RSTR |= RCC_AHB1RSTR_GPIODRST;
		RCC->AHB1RSTR &= ~RCC_AHB1RSTR_GPIODRST;
	}else if(GPIOx == GPIOE)
	{
		RCC->AHB1RSTR |= RCC_AHB1RSTR_GPIOERST;
		RCC->AHB1RSTR &= ~RCC_AHB1RSTR_GPIOERST;
	}else if(GPIOx == GPIOF)
	{
		RCC->AHB1RSTR |= RCC_AHB1RSTR_GPIOFRST;
		RCC->AHB1RSTR &= ~RCC_AHB1RSTR_GPIOFRST;
	}else if(GPIOx == GPIOG)
	{
		RCC->AHB1RSTR |= RCC_AHB1RSTR_GPIOGRST;
		RCC->AHB1RSTR &= ~RCC_AHB1RSTR_GPIOGRST;
	}else if(GPIOx == GPIOH)
	{
		RCC->AHB1RSTR |= RCC_AHB1RSTR_GPIOHRST;
		RCC->AHB1RSTR &= ~RCC_AHB1RSTR_GPIOHRST;
	}else if(GPIOx == GPIOI)
	{
		RCC->AHB1RSTR |= RCC_AHB1RSTR_GPIOIRST;
		RCC->AHB1RSTR &= ~RCC_AHB1RSTR_GPIOIRST;
	}
}


/*********************Read Data and Write Data*************************************/
/*
 * @fn			- GPIO_ReadFromInputPin
 *
 * @brief		- This function reads data from the input pin.
 *
 * @param[in]	- Base address of the GPIOx register.
 * @param[in]	- Number of the GPIO pin used.
 *
 * @return		- uint8_t, 0 or 1.
 *
 * @note		- None.
 */
uint8_t GPIO_ReadFromInputPin(GPIO_TypeDef *GPIOx, uint8_t PinNumber)
{
	return (uint8_t)((GPIOx->IDR >> PinNumber) & 0x00000001);
}

/*
 * @fn			- GPIO_ReadFromInputPort
 *
 * @brief		- This function reads data from the input port.
 *
 * @param[in]	- Base address of the GPIOx port register.
 *
 * @return		- uint16_t.
 *
 * @note		- None.
 */
uint16_t GPIO_ReadFromInputPort(GPIO_TypeDef *GPIOx)
{
	return (uint16_t)(GPIOx->IDR);
}

/*
 * @fn			- GPIO_WriteToOutputPin
 *
 * @brief		- This function writes data to the GPIO output pin.
 *
 * @param[GPIO_TypeDef*]	- Base address of the GPIOx port register.
 * @param[uint8_t]			- Number of GPIOx pin used.
 * @param[uint8_t]			- Number (0 or 1) to write to the pin.
 *
 * @return		- None.
 *
 * @note		- None.
 */
void GPIO_WriteToOutputPin(GPIO_TypeDef *GPIOx, uint8_t PinNumber, uint8_t Value)
{
	if(Value == SET) //Write 1 to the output data register at the bit field corresponding to the pin.
		GPIOx->ODR |= (1 << PinNumber);
	else //Write 0 to the output data register at the bit field corresponding to the pin.
		GPIOx->ODR &= ~(1 << PinNumber);
}

/*
 * @fn			- GPIO_WriteToOutputPort
 *
 * @brief		- This function writes data to the GPIO output port.
 *
 * @param[in]	- Base address of the GPIOx port register.
 * @param[in]	- Number value to write to the pin.
 *
 * @return		- None.
 *
 * @note		- None.
 */
void GPIO_WriteToOutputPort(GPIO_TypeDef *GPIOx, uint16_t Value)
{
	GPIOx->ODR = Value;
}

/*
 * @fn			- GPIO_ToggleOutputPin
 *
 * @brief		- This function toggles the GPIO pin between high(1) and low(0).
 *
 * @param[in]	- Base address of the GPIOx port register.
 * @param[in]	- Number of GPIOx pin used.
 *
 * @return		- None.
 *
 * @note		- None.
 */
void GPIO_ToggleOutputPin(GPIO_TypeDef *GPIOx, uint8_t PinNumber)
{
	GPIOx->ODR ^= (1 << PinNumber);
}

/*
 * @fn			- GPIO_LockRegister
 *
 * @brief		- This function Locks the port pin configuration
 * 				  until the peripheral or the MCU is reset.
 *
 * @param[GPIO_TypeDef*]	- Base address of the GPIOx port register.
 * @param[uint8_t*]			- An array of pins to be locked.
 * @param[uint8_t]			- Length of array.
 *
 * @return		- None.
 *
 * @note		- None.
 */
void GPIO_LockRegister(GPIO_TypeDef *GPIOx, uint8_t *PinNumber, uint8_t length)
{
	if(((GPIOx->LCKR >> 16) & 0x1) == RESET)
	{
		uint32_t pins = 0;
		for(uint8_t i = 0; i < length; i++)
		{
			pins |= (1 << *PinNumber);
			if(!(i + 1 == length))
				PinNumber++;
		}
		GPIOx->LCKR = pins;
		//Lock Sequence.
		GPIOx->LCKR = (uint32_t)((GPIOx->LCKR | (1 << 16)) + ((GPIOx->LCKR >> 15) & 0xf));
		GPIOx->LCKR = (uint32_t)((GPIOx->LCKR & ~(1 << 16)) + ((GPIOx->LCKR >> 15) & 0xf));
		GPIOx->LCKR = (uint32_t)((GPIOx->LCKR | (1 << 16)) + ((GPIOx->LCKR >> 15) & 0xf));
		GPIOx->LCKR;
		while(!((GPIOx->LCKR >> 16) & 0x1));
	}
}


/***************** Private Helper Function Definitions *************************************/
static uint8_t GPIO_BaseAddressToCode(GPIO_TypeDef *GPIOx)
{
	if(GPIOx == GPIOA)
		return 0;
	else if(GPIOx == GPIOB)
		return 1;
	else if(GPIOx == GPIOC)
		return 2;
	else if(GPIOx == GPIOD)
		return 3;
	else if(GPIOx == GPIOE)
		return 4;
	else if(GPIOx == GPIOF)
		return 5;
	else if(GPIOx == GPIOG)
		return 6;
	else if(GPIOx == GPIOH)
		return 7;
	else if(GPIOx == GPIOI)
		return 8;
	else
		return 0;
}

static uint8_t GPIO_PinNumberToExtiCode(uint8_t pinNum)
{
	switch(pinNum)
	{
	case 0: return EXTI0_IRQn;
	case 1: return EXTI1_IRQn;
	case 2: return EXTI2_IRQn;
	case 3: return EXTI3_IRQn;
	case 4: return EXTI4_IRQn;
	case 5:
	case 6:
	case 7:
	case 8:
	case 9: return EXTI9_5_IRQn;
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15: return EXTI15_10_IRQn;
	}
	return 0;
}


/*********************IRQ Configuration and ISR handling***************************/

/* Weak function that can be implemented in user application. */
void __attribute__((weak)) GPIO_ApplicationEventCallBack(uint8_t pinNumber) {}


void EXTI0_IRQHandling(void)
{
	//Clear the EXTI PR register corresponding to the pin number.
	EXTI->PR |= (1 << GPIO_PIN_NO_0);// Writting a 1 clears the EXTI PR
	GPIO_ApplicationEventCallBack(GPIO_PIN_NO_0);
}

void EXTI1_IRQHandling(void)
{
	//Clear the EXTI PR register corresponding to the pin number.
	EXTI->PR |= (1 << GPIO_PIN_NO_1);// Writting a 1 clears the EXTI PR
	GPIO_ApplicationEventCallBack(GPIO_PIN_NO_1);
}

void EXTI2_IRQHandling(void)
{
	//Clear the EXTI PR register corresponding to the pin number.
	EXTI->PR |= (1 << GPIO_PIN_NO_2);// Writting a 1 clears the EXTI PR
	GPIO_ApplicationEventCallBack(GPIO_PIN_NO_2);
}

void EXTI3_IRQHandling(void)
{
	//Clear the EXTI PR register corresponding to the pin number.
	EXTI->PR |= (1 << GPIO_PIN_NO_3);// Writting a 1 clears the EXTI PR
	GPIO_ApplicationEventCallBack(GPIO_PIN_NO_3);
}
