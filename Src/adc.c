#include"adc.h"

/***************** Private Helper Function Headers *************************************/

static void ADC_EnableConverter(ADC_TypeDef *ADC_T);
static void ADC_ConfigPreScaler(uint8_t prescaler);
static void ADC_ConfigBitRes(ADC_TypeDef *ADC_T, uint8_t resolution);
static void ADC_ConfigDataAlign(ADC_TypeDef *ADC_T, uint8_t dataAlign);
static void ADC_ConfigScanMode(ADC_TypeDef *ADC_T, uint8_t EnOrDi);
static void ADC_ConfigIt(ADC_TypeDef *ADC_T, uint8_t EnOrDi);
static void ADC_ConfigWtDg(ADC_TypeDef *ADC_T, uint8_t EnOrDi);
static void ADC_ConfigDMA(ADC_TypeDef *ADC_T, uint8_t EnOrDi);
static void ADC_HandleAWDIt(ADC_TypeDef *ADC_T);
static void ADC_HandleEOCIt(ADC_TypeDef *ADC_T);
static void ADC_HandleJEOCIt(ADC_TypeDef *ADC_T);
static void ADC_HandleOVRIt(ADC_TypeDef *ADC_T);

/***************************************************************************************/


/***************** User Application Exposed Function Definitions ***********************/

/*
 * @fn			- ADC_init
 *
 * @brief		- This function initializes the ADC peripheral with user-
 * 				  provided configurations.
 *
 * @param[ADC_TypeDef]	- ADC address structure.
 * @param[ADC_Config_t]	- ADC configuration structure.
 *
 * @return		- None.
 *
 * @note		- You will need to call the ADC_RegChannelSel() and/or ADC_InjChannelSel()
 * 				  function(s) to configure the conversion channels after calling this function.
 */
void ADC_Init(ADC_TypeDef *ADC_T, ADC_Config_t ADC_Config)
{
	ADC_PeriClockControl(ADC_T, ENABLE);
	ADC_EnableConverter(ADC_T);
	ADC_ConfigPreScaler(ADC_Config.ADC_ClkPreSclr);
	ADC_ConfigBitRes(ADC_T, ADC_Config.ADC_BitRes);
	ADC_ConfigDataAlign(ADC_T, ADC_Config.ADC_DataAlign);
	ADC_ConfigScanMode(ADC_T, ADC_Config.ADC_ScanMode);
	ADC_ConfigIt(ADC_T, ADC_Config.ADC_ItEnable);
	ADC_ConfigWtDg(ADC_T, ADC_Config.ADC_WtDgEnable);
	ADC_ConfigDMA(ADC_T, ADC_Config.ADC_DMAEnable);
}

/*
 * @fn			- ADC_DeInit
 *
 * @brief		- This function de-initializes the ADC peripheral.
 *
 * @param[void]	- None
 *
 * @return		- None.
 *
 * @note		- None.
 */
void ADC_DeInit(void)
{
	RCC->APB2ENR |= RCC_APB2RSTR_ADCRST;
	RCC->APB2ENR &= ~RCC_APB2RSTR_ADCRST;
}

/*
 * @fn			- ADC_PeriClockControl
 *
 * @brief		- This function enables or disables peripheral clock for the
 * 				  given ADC register.
 *
 * @param[ADC_TypeDef]	- Base address of the ADC register.
 * @param[uint8_t]			- ENABLE or DISABLE macros.
 *
 * @return		- None.
 *
 * @note		- None.
 */
void ADC_PeriClockControl(ADC_TypeDef *ADC_T, uint8_t EnOrDi)
{
	if(EnOrDi == ENABLE)
	{
		if(ADC_T == ADC1)
			RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
		else if(ADC_T == ADC2)
			RCC->APB2ENR |= RCC_APB2ENR_ADC2EN;
		else if(ADC_T == ADC3)
			RCC->APB2ENR |= RCC_APB2ENR_ADC3EN;
	}
	else
	{
		if(ADC_T == ADC1)
			RCC->APB2ENR &= ~RCC_APB2ENR_ADC1EN;
		else if(ADC_T == ADC2)
			RCC->APB2ENR &= ~RCC_APB2ENR_ADC2EN;
		else if(ADC_T == ADC3)
			RCC->APB2ENR &= ~RCC_APB2ENR_ADC3EN;
	}
}

/*
 * @fn			- ADC_RegChannelSel
 *
 * @brief		- This function configures the selection of the regular
 * 				  group of channels, how many channels in the group, and
 * 				  the order of the channels.
 *
 * @param[ADC_TypeDef]		- Base address of the ADC register.
 *
 * @param[uint8_t]			- Number of conversions for the selected group.
 * 							  (ADC_01_CONVERSIONS, ADC_02_CONVERSIONS, ... ADC_16_CONVERSIONS).
 * @param[uint8_t]			- Array containing which channels to be used, ordered from index 0
 * 							  to last index.
 * 							  ({ADC_IN0, ADC_IN1, ... ADC_IN15})
 *
 *
 * @return		- None.
 *
 * @note		- Regular channels can have up to 16 conversions.
 * 				  ** Be sure to place the channel numbers in the array parameter in the
 * 				  order that you want to have them converted (index[0] will be first). **
 */
void ADC_RegChannelSel(ADC_TypeDef *ADC_T, uint8_t conversions, uint8_t channels[])
{
	if(conversions == ADC_01_CONVERSIONS)
	{//Only one regular conversion will be done.(Most likely for single conversion mode.)
		ADC_T->SQR1 &= ~ADC_SQR1_L;
		ADC_T->SQR3 |= (channels[0] << 0);
	}
	else if(conversions <= ADC_16_CONVERSIONS && conversions >= ADC_02_CONVERSIONS)
	{//More than one regular conversion will be done.
		ADC_T->SQR1 |= ((conversions) << 20U);
		for(uint8_t i = 0; i < conversions; i++)
		{
			if(i < 6)
				ADC_T->SQR3 |= (channels[i] << (i * 5));
			else if(i > 5 && i < 12)
				ADC_T->SQR2 |= (channels[i] << (i * 5));
			else
				ADC_T->SQR1 |= (channels[i] << (i * 5));
		}
	}
}


/*
 * @fn			- ADC_InjChannelSel
 *
 * @brief		- This function configures the selection of injected group of
 * 				  channels, how many channels in the group, and the order of
 * 				  the channels.
 *
 * @param[ADC_TypeDef]		- Base address of the ADC register.
 *
 * @param[uint8_t]			- Number of conversions for the selected group.
 * 							  (ADC_01_CONVERSIONS, ADC_02_CONVERSIONS, ... ADC_16_CONVERSIONS).
 * @param[uint8_t]			- Array containing which channels to be used, ordered from index 0
 * 							  to last index.
 * 							  (ADC_IN0, ADC_IN1, ... ADC_IN15)
 *
 *
 * @return		- None.
 *
 * @note		- Injected channels can have up to 4 conversions.
 * 				  ** Be sure to place the channel numbers in the array parameter in the
 * 				  order that you want to have them converted (index[0] will be first). **
 */
void ADC_InjChannelSel(ADC_TypeDef *ADC_T, uint8_t conversions, uint8_t channels[])
{
	if(conversions <= ADC_04_CONVERSIONS && conversions > ADC_01_CONVERSIONS)
	{
		ADC_T->JSQR |= (conversions << 20U);
		if(conversions == ADC_01_CONVERSIONS)
			ADC_T->JSQR |= (channels[0] << 15);
		else if(conversions == ADC_02_CONVERSIONS)
		{
			ADC_T->JSQR |= (channels[0] << 10);
			ADC_T->JSQR |= (channels[1] << 15);
		}
		else if(conversions == ADC_03_CONVERSIONS)
		{
			ADC_T->JSQR |= (channels[0] << 5);
			ADC_T->JSQR |= (channels[1] << 10);
			ADC_T->JSQR |= (channels[2] << 15);
		}
		else if(conversions == ADC_04_CONVERSIONS)
		{
			ADC_T->JSQR |= (channels[0] << 0);
			ADC_T->JSQR |= (channels[1] << 5);
			ADC_T->JSQR |= (channels[2] << 10);
			ADC_T->JSQR |= (channels[3] << 15);
		}
	}
}

/*
 * @fn			- ADC_SetDisContNumber
 *
 * @brief		- This function configures the number of conversions
 * 				  that will take place for DisContinuous Conversion
 * 				  Mode.
 *
 * @param[ADC_TypeDef]		- Base address of the ADC Register.
 * @param[uint8_t]			- Number of dicontinuous conversions
 * 							  (ADC_DISC_NUM_1, ... ADC_DISC_NUM_8).
 *
 * @return		- None.
 *
 * @note		- None.
 */
void ADC_SetDisContNumber(ADC_TypeDef *ADC_T, uint8_t numDiscConversions)
{
	if(numDiscConversions == ADC_DISC_NUM_1)
		ADC_T->CR1 &= ~ADC_CR1_DISCNUM;
	else if(numDiscConversions >= ADC_DISC_NUM_1 && numDiscConversions <= ADC_DISC_NUM_8)
		ADC_T->CR1 |= (numDiscConversions << 13);
}

/*
 * @fn			- ADC_ConfigSampRate
 *
 * @brief		- This function configures the sample rate for the given
 * 				  analog channel.
 *
 * @param[ADC_TypeDef]		- Base address of the ADC Register.
 * @param[uint8_t]			- ADC channel (ADC_IN0, ... ADC_IN18).
 * @param[uint8_t]			- Sampling time (ADC_003_CYCLES, ... ADC_480_CYCLES).
 *
 * @return		- None.
 *
 * @note		- None.
 */
void ADC_ConfigSampRate(ADC_TypeDef *ADC_T, uint8_t channel, uint8_t cycles)
{
	if(cycles == ADC_003_CYCLES)
	{
		switch(channel)
		{
			case ADC_IN0 : ADC_T->SMPR2 &= ~ADC_SMPR2_SMP0;  break;
			case ADC_IN1 : ADC_T->SMPR2 &= ~ADC_SMPR2_SMP1;  break;
			case ADC_IN2 : ADC_T->SMPR2 &= ~ADC_SMPR2_SMP2;  break;
			case ADC_IN3 : ADC_T->SMPR2 &= ~ADC_SMPR2_SMP3;  break;
			case ADC_IN4 : ADC_T->SMPR2 &= ~ADC_SMPR2_SMP4;  break;
			case ADC_IN5 : ADC_T->SMPR2 &= ~ADC_SMPR2_SMP5;  break;
			case ADC_IN6 : ADC_T->SMPR2 &= ~ADC_SMPR2_SMP6;  break;
			case ADC_IN7 : ADC_T->SMPR2 &= ~ADC_SMPR2_SMP7;  break;
			case ADC_IN8 : ADC_T->SMPR2 &= ~ADC_SMPR2_SMP8;  break;
			case ADC_IN9 : ADC_T->SMPR2 &= ~ADC_SMPR2_SMP9;  break;
			case ADC_IN10: ADC_T->SMPR1 &= ~ADC_SMPR1_SMP10; break;
			case ADC_IN11: ADC_T->SMPR1 &= ~ADC_SMPR1_SMP11; break;
			case ADC_IN12: ADC_T->SMPR1 &= ~ADC_SMPR1_SMP12; break;
			case ADC_IN13: ADC_T->SMPR1 &= ~ADC_SMPR1_SMP13; break;
			case ADC_IN14: ADC_T->SMPR1 &= ~ADC_SMPR1_SMP14; break;
			case ADC_IN15: ADC_T->SMPR1 &= ~ADC_SMPR1_SMP15; break;
			case ADC_IN16: ADC_T->SMPR1 &= ~ADC_SMPR1_SMP16; break;
			case ADC_IN17: ADC_T->SMPR1 &= ~ADC_SMPR1_SMP17; break;
			case ADC_IN18: ADC_T->SMPR1 &= ~ADC_SMPR1_SMP18; break;
		}
	}
	else
	{
		switch(channel)
		{
			case ADC_IN0 : ADC_T->SMPR2 |= (cycles << 0);  break;
			case ADC_IN1 : ADC_T->SMPR2 |= (cycles << 3);  break;
			case ADC_IN2 : ADC_T->SMPR2 |= (cycles << 6);  break;
			case ADC_IN3 : ADC_T->SMPR2 |= (cycles << 9);  break;
			case ADC_IN4 : ADC_T->SMPR2 |= (cycles << 12); break;
			case ADC_IN5 : ADC_T->SMPR2 |= (cycles << 15); break;
			case ADC_IN6 : ADC_T->SMPR2 |= (cycles << 18); break;
			case ADC_IN7 : ADC_T->SMPR2 |= (cycles << 21); break;
			case ADC_IN8 : ADC_T->SMPR2 |= (cycles << 24); break;
			case ADC_IN9 : ADC_T->SMPR2 |= (cycles << 27); break;
			case ADC_IN10: ADC_T->SMPR1 |= (cycles << 0);  break;
			case ADC_IN11: ADC_T->SMPR1 |= (cycles << 3);  break;
			case ADC_IN12: ADC_T->SMPR1 |= (cycles << 6);  break;
			case ADC_IN13: ADC_T->SMPR1 |= (cycles << 9);  break;
			case ADC_IN14: ADC_T->SMPR1 |= (cycles << 12); break;
			case ADC_IN15: ADC_T->SMPR1 |= (cycles << 15); break;
			case ADC_IN16: ADC_T->SMPR1 |= (cycles << 18); break;
			case ADC_IN17: ADC_T->SMPR1 |= (cycles << 21); break;
			case ADC_IN18: ADC_T->SMPR1 |= (cycles << 24); break;
		}
	}
}

/*
 * @fn			- ADC_SelectEOCFlagTrigger
 *
 * @brief		- This function configures the End of Conversion
 * 				  Selection bit in the CR2 register.
 *
 * @param[ADC_TypeDef]	- Base address of the ADC Register.
 * @param[ADC_Config_t]	- Enumeration for ADC configuration.
 *
 * @return		- None.
 *
 * @note		- Settings for this is provided in the config
 * 				  structure within the hanle structure.
 */
void ADC_SelectEOCFlagTrigger(ADC_TypeDef *ADC_T, ADC_Config_t ADC_Config)
{
	if(ADC_Config.ADC_EOCSelect == ADC_END_OF_EACH)
		ADC_T->CR2 |= ADC_CR2_EOCS;
	else
		ADC_T->CR2 &= ~ADC_CR2_EOCS;
}

/*
 * @fn			- ADC_StartConversion
 *
 * @brief		- This function starts ADC conversion according to the
 * 				  configurations defined in the ADC_Config Structure.
 * 				  Supported conversion modes are single, continuous,
 * 				  and discontinuous modes.
 *
 * @param[ADC_TypeDef]		- Base address of the ADC register.
 * @param[uint8_t]			- Regular group or Injected group ex.
 * 							  (ADC_REGULAR_GROUP, ADC_INJECTED_GROUP).
 * @param[uint8_t]			- Conversion Mode. ex.(ADC_SINL_CONV_MODE,
 * 							  ADC_CONT_CONV_MODE, ADC_DISCONT_CONV_MODE).
 *
 * @return		- None.
 *
 * @note		- Injected channels cannot be converted continuously. The
 * 				  only exception is when an injected channel is configured
 * 				  to be converted automatically after regular channels in
 * 				  continuous mode (using JAUTO bit).
 */
void ADC_StartConversion(ADC_TypeDef *ADC_T, uint8_t group, uint8_t conversionMode)
{
	if(group == ADC_REGULAR_GROUP)
	{//Regular Group will be converted.
		if(conversionMode == ADC_SINL_CONV_MODE)
		{//Single Conversion Mode.
			ADC_T->CR2 |= ADC_CR2_SWSTART;
			ADC_T->SR &= ~ADC_SR_STRT;
		}else if(conversionMode == ADC_CONT_CONV_MODE)
		{//Continuous Conversion Mode.
			ADC_T->CR2 |= ADC_CR2_CONT;
			ADC_T->CR2 |= ADC_CR2_SWSTART;
		}
	}else
	{//Injected Group will be converted.
		if(conversionMode == ADC_SINL_CONV_MODE)
		{//Single Conversion Mode.
			ADC_T->CR2 |= ADC_CR2_JSWSTART;
			ADC_T->SR &= ~ADC_SR_JSTRT;
		}
	}
}

/*
 * @fn			- ADC_DisableContConversion
 *
 * @brief		- This function clears the CONT bit in the CR2
 * 				  register, disabling continuous conversion mode.
 *
 * @param[ADC_TypeDef]	- Base address of the ADC register.
 *
 * @return		- None.
 *
 * @note		- None.
 */
void ADC_DisableContConversion(ADC_TypeDef *ADC_T)
{
	ADC_T->CR2 &= ~ADC_CR2_CONT;
}

/*
 * @fn			- ADC_StopConversion
 *
 * @brief		- This function stops the ADC conversion. If
 * 				  continuous conversion is enabled, it is disabled.
 *
 * @param[ADC_TypeDef]	- Base address of the ADC register.
 *
 * @return		- None.
 *
 * @note		- None.
 */
void ADC_StopConversion(ADC_TypeDef *ADC_T)
{
	if(((ADC_T->CR2 >> 1) & 0x1) == SET)
		ADC_DisableContConversion(ADC_T);

	ADC_T->CR2 &= ~ADC_CR2_ADON;
}

/*
 * @fn			- ADC_ReadRegDR
 *
 * @brief		- This function reads the data register.
 *
 * @param[ADC_TypeDef]	- Base address of the ADC register.
 *
 * @return		- uint16_t Data read from the data register.
 *
 * @note		- Once the DR is read, if the EOC flag was set, it will
 * 				  be automatically cleared by hardware.
 */
uint16_t ADC_ReadRegDR(ADC_TypeDef *ADC_T)
{
	return (uint16_t)ADC_T->DR;
}

/*
 * @fn			- ADC_ReadInjDR
 *
 * @brief		- This function reads the injected data register 1 of
 * 				  injected channel.
 *
 * @param[ADC_TypeDef*]	- Base address of the ADC register.
 *
 * @return		- uint16_t Data read from the injected data register.
 *
 * @note		- None.
 */
uint16_t ADC_ReadInjDR(ADC_TypeDef *ADC_T)
{
	return (uint16_t)ADC_T->JDR1;
}

/*
 * @fn			- ADC_ExtTrigDetect
 *
 * @brief		- This function configures the trigger detection for
 * 				  falling edge, rising edge, both, or none.
 *
 * @param[ADC_TypeDef]		- Base address of the ADC register.
 * @param[uint8_t]			- Conversion Group.
 * 							  (ADC_REGULAR_GROUP, ADC_INJECTED_GROUP)
 * @param[uint8_t]			- Trigger detection.
 * 							  (ADC_DETECTION_DISABLED, ADC_RISING_EDGE,
 * 							   ADC_FALLING_EDGE, ADC_RIS_FALL_EDGE)
 *
 * @return		- None.
 *
 * @note		- None.
 */
void ADC_ExtTrigDetect(ADC_TypeDef *ADC_T, uint8_t group, uint8_t detection)
{
	if(group == ADC_REGULAR_GROUP)
	{//External trigger enable for regular channels.
		if(detection == ADC_DETECTION_DISABLED)
			ADC_T->CR2 &= ~ADC_CR2_EXTEN;
		else
			ADC_T->CR2 |= (detection << 28);
	}else
	{//External trigger enable for injected channels.
		if(detection == ADC_DETECTION_DISABLED)
			ADC_T->CR2 &= ~ADC_CR2_JEXTEN;
		else
			ADC_T->CR2 |= (detection << 20);
	}
}

/*
 * @fn			- ADC_SelectExtEvReg
 *
 * @brief		- This function configures the selection of which
 * 				  external events will be used to generate external
 * 				  interrupts.
 *
 * @param[ADC_TypeDef]		- Base address of the ADC register.
 * @param[uint8_t]			- Conversion Group.
 * 							  (ADC_REGULAR_GROUP, ADC_INJECTED_GROUP)
 * @param[uint8_t]			- External event.
 * 							  (ADC_TIM1_CC1_EVENT, ... ADC_EXTI_LINE_15)
 *
 * @return		- None.
 *
 * @note		- None.
 */
void ADC_SelectExtEvReg(ADC_TypeDef *ADC_T, uint8_t group, uint8_t event)
{
	if(group == ADC_REGULAR_GROUP)
	{//External event selection for regular channels.
		if(event == ADC_TIM1_CC1_EVENT)
			ADC_T->CR2 &= ~ADC_CR2_EXTSEL;
		else
			ADC_T->CR2 |= (event << 24);
	}else
	{//External event selection for injected channels.
		if(event == ADC_TIM1_CC4_EVENT)
			ADC_T->CR2 &= ~ADC_CR2_JEXTSEL;
		else
			ADC_T->CR2 |= (event << 16);
	}
}

/*
 * @fn			- ADC_SelectWatchDogChannel
 *
 * @brief		- This function configures which regular channel
 * 				  will be guarded by the watch dog feature.
 *
 * @param[ADC_TypeDef]		- Base address of the ADC register.
 * @param[uint8_t]			- The channel to be guarded.(ADC_IN0 ... ADC_IN15)
 *
 * @return		- None.
 *
 * @note		- None.
 */
void ADC_SelectWatchDogChannel(ADC_TypeDef *ADC_T, uint8_t channel)
{
	ADC_T->CR1 |= (channel << 0);
}

/***************************************************************************************/


/***************** ADCx IRQ Handling ***************************************************/

/* Weak function that can be implemented in user application. */
void __attribute__((weak)) ADC_ApplicationEventCallback(uint8_t){}

/*
 * @fn			- ADC_IRQHandling
 *
 * @brief		- This function handles an ADC interrupt.
 *
 * @param[ADC_TypeDef]	- Base Address of the ADC Handle.
 *
 * @return		- None.
 *
 * @note		- The interrupt bits(OVRIE, EOCIE, JEOCIE, AWDIE) must
 * 				  be set in order for an interrupt to be triggered.
 */
void ADC_IRQHandling(ADC_TypeDef *ADC_T)
{
	if(((ADC_T->SR >> 1) & 0x1) == SET)  //End of Conversion flag.
	{
		ADC_HandleEOCIt(ADC_T);
		ADC_ApplicationEventCallback(ADC_END_OF_CONVERSION_REG);
	}
	if(((ADC_T->SR >> 2) & 0x1) == SET)  //End of Injected Conversion flag.
	{
		ADC_HandleJEOCIt(ADC_T);
		ADC_ApplicationEventCallback(ADC_END_OF_CONVERSION_INJ);
	}
	if(((ADC_T->SR >> 0) & 0x1) == SET)  //Analog Watchdog flag.
	{
		ADC_HandleAWDIt(ADC_T);
		ADC_ApplicationEventCallback(ADC_WATCHDOG_SET);
	}
	if(((ADC_T->SR >> 5) & 0x1) == SET)  //Overrun flag.
	{
		ADC_HandleOVRIt(ADC_T);
		ADC_ApplicationEventCallback(ADC_OVERRUN_SET);
	}
}



/***************************************************************************************/


/***************** Private Helper Function Definitions *********************************/

/*
 * @fn			- ADC_StartConverter
 *
 * @brief		- This function turns on the ADC Converter.
 *
 * @param[ADC_TypeDef*]	- Base Address of the ADC Register.
 *
 * @return		- None.
 *
 * @note		- Private Helper Function.
 */
static void ADC_EnableConverter(ADC_TypeDef *ADC_T)
{//Set the ADON bit, turning on the ADC.
	ADC_T->CR2 |= ADC_CR2_ADON;
}

/*
 * @fn			- ADC_ConfigPreScaler
 *
 * @brief		- This function configures the prescaler value for
 * 				  the ADC peripheral.
 *
 * @param[uint8_t]			- The prescaler value for the ADC peripheral
 * 							  (ADC_PCLK_DIV2, ... ADC_PCLK_DIV8).
 *
 * @return		- None.
 *
 * @note		- Private Helper Function.
 */
static void ADC_ConfigPreScaler(uint8_t prescaler)
{
	switch(prescaler)
	{
		case ADC_PCLK_DIV2: ADC->CCR |= (ADC_PCLK_DIV2 << 16); break;
		case ADC_PCLK_DIV4: ADC->CCR |= (ADC_PCLK_DIV4 << 16); break;
		case ADC_PCLK_DIV6: ADC->CCR |= (ADC_PCLK_DIV6 << 16); break;
		case ADC_PCLK_DIV8: ADC->CCR |= (ADC_PCLK_DIV8 << 16); break;
		default: ADC->CCR |= (ADC_PCLK_DIV2 << 16); break;
	}
}

/*
 * @fn			- ADC_ConfigBitRes
 *
 * @brief		- This function configures the bit resolution.
 *
 * @param[ADC_TypeDef]		- Base Address of the ADC Register.
 * @param[uint8_t]			- Bit Resulotion(ADC_12BIT_RESOLUTION, ...
 * 							  ADC_06BIT_RESOLUTION).
 *
 * @return		- None.
 *
 * @note		- Private Helper Function.
 */
static void ADC_ConfigBitRes(ADC_TypeDef *ADC_T, uint8_t resolution)
{
	ADC_T->CR1 |= (resolution << 24);
}

/*
 * @fn			- ADC_ConfigDataAlign
 *
 * @brief		- This function configures the data alignment in the data register.
 *
 * @param[ADC_TypeDef]		- Base Address of the ADC Register.
 * @param[uint8_t]			- Data Alignment(ADC_DATA_ALIGNMENT_RIGHT, ...
 * 							  ADC_DATA_ALIGNMENT_LEFT).
 *
 * @return		- None.
 *
 * @note		- Private Helper Function.
 */
static void ADC_ConfigDataAlign(ADC_TypeDef *ADC_T, uint8_t dataAlign)
{
	if(dataAlign == ADC_DATA_ALIGNMENT_RIGHT)
		ADC_T->CR2 &= ~ADC_CR2_ALIGN;//Align to the right.
	else
		ADC_T->CR2 |= ADC_CR2_ALIGN; //Align to the left.
}

/*
 * @fn			- ADC_ConfigScanMode
 *
 * @brief		- This function configures the use of scan mode.
 *
 * @param[ADC_TypeDef]		- Base Address of the ADC Register.
 * @param[uint8_t]			- ENABLE or DISABLE
 *
 * @return		- None.
 *
 * @note		- Private Helper Function.
 */
static void ADC_ConfigScanMode(ADC_TypeDef *ADC_T, uint8_t EnOrDi)
{
	if(EnOrDi == ENABLE)
		ADC_T->CR1 |= ADC_CR1_SCAN;   //Scan Mode will be used.
	else
		ADC_T->CR1 &= ~ADC_CR1_SCAN;  //Scan Mode will not be used.
}

/*
 * @fn			- ADC_ConfigWtDg
 *
 * @brief		- This function enables or disables the ADC watch dog feature.
 *
 * @param[ADC_TypeDef]		- Base Address of the ADC Register.
 * @param[uint8_t]			- ENABLE or DISABLE
 *
 * @return		- None.
 *
 * @note		- Private Helper Function.
 */
static void ADC_ConfigWtDg(ADC_TypeDef *ADC_T, uint8_t EnOrDi)
{
	if(EnOrDi == ADC_WATCHDOG_ENABLE)
	{
		ADC_T->CR1 |= ADC_CR1_AWDEN;
		ADC_T->CR1 |= ADC_CR1_JAWDEN;
		ADC_T->CR1 |= ADC_CR1_AWDIE;
	}else
	{
		ADC_T->CR1 &= ~ADC_CR1_AWDEN;
		ADC_T->CR1 &= ~ADC_CR1_JAWDEN;
		ADC_T->CR1 &= ~ADC_CR1_AWDIE;
	}
}

/*
 * @fn			- ADC_ConfigIt
 *
 * @brief		- This function enables or disables the ADC interrupts.
 *
 * @param[ADC_TypeDef]		- Base Address of the ADC Register.
 * @param[uint8_t]			- ENABLE or DISABLE
 *
 * @return		- None.
 *
 * @note		- Private Helper Function.
 */
static void ADC_ConfigIt(ADC_TypeDef *ADC_T, uint8_t EnOrDi)
{
	if(EnOrDi == ADC_INTERRUPT_ENABLE)
	{
		ADC_T->CR1 |= ADC_CR1_OVRIE;
		ADC_T->CR1 |= ADC_CR1_JEOCIE;
		ADC_T->CR1 |= ADC_CR1_EOCIE;
	}else
	{
		ADC_T->CR1 &= ~ADC_CR1_OVRIE;
		ADC_T->CR1 &= ~ADC_CR1_JEOCIE;
		ADC_T->CR1 &= ~ADC_CR1_EOCIE;
	}
}

/*
 * @fn			- ADC_ConfigDMA
 *
 * @brief		- This function enables or disables the DMA.
 *
 * @param[ADC_TypeDef]		- Base Address of the ADC Register.
 * @param[uint8_t]			- ENABLE or DISABLE
 *
 * @return		- None.
 *
 * @note		- Private Helper Function.
 */
static void ADC_ConfigDMA(ADC_TypeDef *ADC_T, uint8_t EnOrDi)
{
	if(EnOrDi == ADC_DMA_ENABLE)
	{
		ADC_T->CR2 |= ADC_CR2_DMA;
		ADC_T->CR2 |= ADC_CR2_DDS;
	}else
	{
		ADC_T->CR2 &= ~ADC_CR2_DMA;
		ADC_T->CR2 &= ~ADC_CR2_DDS;
	}
}

/*
 * @fn			- ADC_HandleAWDIt
 *
 * @brief		- This function handles the Analog Watchdog
 * 				  Interrupt.
 *
 * @param[ADC_TypeDef]	- Base Address of the ADC Handle.
 *
 * @return		- None.
 *
 * @note		- Private Helper Function.
 */
static void ADC_HandleAWDIt(ADC_TypeDef *ADC_T)
{
	ADC_T->SR &= ~ADC_SR_AWD;
}

/*
 * @fn			- ADC_HandleEOCIt
 *
 * @brief		- This function handles the End of Conversion
 * 				  Interrupt.
 *
 * @param[ADC_TypeDef]	- Base Address of the ADC Handle.
 *
 * @return		- None.
 *
 * @note		- Private Helper Function.
 */
static void ADC_HandleEOCIt(ADC_TypeDef *ADC_T)
{
	ADC_T->SR &= ~ADC_SR_EOC;
}

/*
 * @fn			- ADC_HandleJEOCIt
 *
 * @brief		- This function handles the Injection End of
 * 				  Conversion Interrupt.
 *
 * @param[ADC_TypeDef]	- Base Address of the ADC Handle.
 *
 * @return		- None.
 *
 * @note		- Private Helper Function.
 */
static void ADC_HandleJEOCIt(ADC_TypeDef *ADC_T)
{
	ADC_T->SR &= ~ADC_SR_JEOC;
}

/*
 * @fn			- ADC_HandleOVRIt
 *
 * @brief		- This function handles the Overrun
 * 				  Interrupt.
 *
 * @param[ADC_TypeDef*]	- Base Address of the ADC Handle.
 *
 * @return		- None.
 *
 * @note		- Private Helper Function.
 */
static void ADC_HandleOVRIt(ADC_TypeDef *ADC_T)
{
	ADC_T->SR &= ~ADC_SR_OVR;
}

/***************************************************************************************/
