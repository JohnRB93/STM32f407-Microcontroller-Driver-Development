//#define _ANALOG_INTERRUPTS_SINGLE_CONV_UART_CONSOL
//#define _TIMER_BLINK
//#define _LCD_DISPLAY
//#define _ANALOG_MEASUREMENT_LCD
//#define _ANALOG_MEASUREMENT_LCD_UART
//#define _USER_PUSHBUTTON_INTERRUPT_LEDS
//#define _SPI_DATA_TX_ONLY
#define _SPI_DATA_TX_RX


#ifdef _ANALOG_INTERRUPTS_SINGLE_CONV_UART_CONSOL

/*
 * This program uses the Analog to Digital Conversion peripheral to convert an analog
 * input to a digital value and display it on a computer console. A UART module is used
 * to enable serial communication between the microcontroller and the computer. The value
 * of the analog input is determined by the use of a potentiometer. The ADC peripheral in
 * this case uses interrupts each time a conversion is created.
 */

#include<stdio.h>
#include"uart.h"
#include"adc.h"
#include"irq.h"
#include"gpio.h"
#include"tim.h"

/*
 * UART -> PA2
 * ADC1 -> PC0
 */

GPIO_PinConfig_t gpioConfig;
ADC_Config_t adcConfig;

uint8_t adcChannels[] = {ADC_IN10};
uint16_t analogRead = 0;

void configGpioPinPC0_analog(void);

int main(void)
{
	uart_init();
	printf("%s", "Program Start\n");

	TIM2_5_Init(TIM2, 0);
	configGpioPinPC0_analog();
	ADC_StartConversion(ADC1, adcConfig.ADC_ConvGroup, ADC_SINL_CONV_MODE);

	while(1){}
}

void configGpioPinPC0_analog(void)
{
	// Setup for analog mode pin C0, ADC1
	gpioConfig.GPIO_PinNumber = GPIO_PIN_NO_0;
	gpioConfig.GPIO_PinMode = GPIO_MODE_ANALOG;
	gpioConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	GPIO_Init(GPIOC, gpioConfig);

	adcConfig.ADC_BitRes = ADC_12BIT_RESOLUTION;
	adcConfig.ADC_SampTime = ADC_003_CYCLES;
	adcConfig.ADC_EOCSelect = ADC_END_OF_EACH;
	adcConfig.ADC_ConvGroup = ADC_REGULAR_GROUP;
	adcConfig.ADC_DataAlign = ADC_DATA_ALIGNMENT_RIGHT;
	adcConfig.ADC_ScanMode = 0;
	adcConfig.ADC_ClkPreSclr = ADC_PCLK_DIV2;
	adcConfig.ADC_ItEnable = ADC_INTERRUPT_ENABLE;
	adcConfig.ADC_WtDgEnable = ADC_WATCHDOG_DISABLE;
	adcConfig.ADC_DMAEnable = ADC_DMA_DISABLE;
	ADC_Init(ADC1, adcConfig);
	ADC_RegChannelSel(ADC1, ADC_01_CONVERSIONS, adcChannels);
	ADC_ConfigSampRate(ADC1, adcChannels[0], adcConfig.ADC_SampTime);
	ADC_SelectEOCFlagTrigger(ADC1, adcConfig);
	IRQInterruptConfig(ADC_IRQn, ENABLE);
}

void ADC_IRQHandler(void)
{
	ADC_IRQHandling(ADC1);
}

extern void ADC_ApplicationEventCallback(uint8_t appEv)
{
	if(appEv == ADC_END_OF_CONVERSION_REG || appEv == ADC_END_OF_CONVERSION_INJ)
	{
		analogRead = (((255.0 / 4094) * ADC_ReadRegDR(ADC1)) - (255.0 / 4094));
		printf("%d\n", analogRead);
		ADC_StartConversion(ADC1, adcConfig.ADC_ConvGroup, ADC_SINL_CONV_MODE);
	}
	else if(appEv == ADC_OVERRUN_SET)
		ADC_StartConversion(ADC1, adcConfig.ADC_ConvGroup, ADC_SINL_CONV_MODE);

	TIM2_5_Delay_us(TIM2, 500000);
}

#endif /* _ANALOG_INTERRUPTS_SINGLE_CONV_UART_CONSOL */

#ifdef _TIMER_BLINK

/*
 * This program uses Timer2 peripheral to blink one of the on-board leds (GPIOD pin 15)
 * at a predetermined interval. This program along with the use of either a logic analyzer or
 * an oscilloscope help to verify the accuracy of the timer's delay function.
 */

#define _PULSE_MODE
//#define NO_PULSE_MODE

#include"gpio.h"
#include"tim.h"
#include"irq.h"

int main(void)
{
	// Initialize LED PD15
	GPIO_PinConfig_t ledConfig;
	ledConfig.GPIO_PinMode = GPIO_MODE_OUT;
	ledConfig.GPIO_PinNumber = GPIO_PIN_NO_15;
	ledConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	ledConfig.GPIO_PinSpeed = GPIO_SPEED_HIGH;
	GPIO_Init(GPIOD, ledConfig);
	TIM2_5_Init(TIM2, PULSE_MODE_ENABLE);
	IRQInterruptConfig(TIM2_IRQn, ENABLE);

	while(1)
	{
#ifdef _PULSE_MODE
		GPIO_WriteToOutputPin(GPIOD, GPIO_PIN_NO_15, SET);
		TIM2_5_Pulse_us(TIM2, 10);
		GPIO_WriteToOutputPin(GPIOD, GPIO_PIN_NO_15, RESET);
		for(uint32_t i = 0; i < 1000000; i++){}
#endif /* _PULSE_MODE */
#ifdef _NO_PULSE_MODE
		TIM2_5_Delay_us(TIM2, 10);
		GPIO_ToggleOutputPin(GPIOD, GPIO_PIN_NO_15);
#endif /* _NO_PULSE_MODE */
	}
}

void TIM2_IRQHandler(void)
{
	TIM2_IRQHandling(TIM2);
}

extern void TIM_ApplicationEventCallback(void)
{
	GPIO_ToggleOutputPin(GPIOD, GPIO_PIN_NO_15);
	GPIO_ToggleOutputPin(GPIOD, GPIO_PIN_NO_15);
}

#endif /* _TIMER_BLINK */

#ifdef _LCD_DISPLAY

/*
 * This program uses the LCD_Print() function to display variables onto a 2x16 LCD module.
 * The LCD module is used in 8-bit mode and requires 11 connections in total. Most of the GPIOD
 * peripheral is used to acheive the necessary connections. A potentiometer is used to alter the
 * analog input value.
 */

#include<stdio.h>
#include<string.h>
#include"lcd2_16.h"
#include"tim.h"

/*
 * GPIO_PIN_PD0  -> LCD_DB0
 * GPIO_PIN_PD1  -> LCD_DB1
 * GPIO_PIN_PD2  -> LCD_DB2
 * GPIO_PIN_PD3  -> LCD_DB3
 * GPIO_PIN_PD4  -> LCD_DB4
 * GPIO_PIN_PD5  -> LCD_DB5
 * GPIO_PIN_PD6  -> LCD_DB6
 * GPIO_PIN_PD7  -> LCD_DB7
 * GPIO_PIN_PD8  -> LCD_RW
 * GPIO_PIN_PD9  -> LCD_RS
 * GPIO_PIN_PD10 -> LCD_E
 */

int main(void)
{
	LCD_Init8Bit();
	uint32_t numA = 50;
	uint32_t numB = 5;
	uint32_t numC = numA * numB;
	char* myStr = "Let's do math: %d * %d = %d\n";
	LCD_Print(myStr, numA, numB, numC);
	myStr = "It's fun when it works!";
	LCD_Print(myStr);
	while(1)
	{
		if(LCD_GetScrollFlag())
		{
			LCD_ScrollDisplay();
		}
	}
}

#endif /* _LCD_DISPLAY */

#ifdef _ANALOG_MEASUREMENT_LCD

/*
 * This program uses the Analog to Digital Conversion peripheral to convert an analog
 * input to a digital value and display it on a 2x16 LCD module. The LCD module uses
 * 8-bit configuration as well as the Timer 2 peripheral. The value of the analog input
 * is determined by the use of a potentiometer.
 */

#include<stdio.h>
#include<string.h>
#include"lcd2_16.h"
#include"tim.h"
#include"adc.h"
#include"irq.h"
#include"gpio.h"

#define POT_MAX 10400U
#define ADC_MAX 4094.0

/*
 * GPIO_PIN_PC0  -> ADC1
 *
 * GPIO_PIN_PD0  -> LCD_DB0
 * GPIO_PIN_PD1  -> LCD_DB1
 * GPIO_PIN_PD2  -> LCD_DB2
 * GPIO_PIN_PD3  -> LCD_DB3
 * GPIO_PIN_PD4  -> LCD_DB4
 * GPIO_PIN_PD5  -> LCD_DB5
 * GPIO_PIN_PD6  -> LCD_DB6
 * GPIO_PIN_PD7  -> LCD_DB7
 * GPIO_PIN_PD8  -> LCD_RW
 * GPIO_PIN_PD9  -> LCD_RS
 * GPIO_PIN_PD10 -> LCD_E
 *
 * GPIO_PIN_PB0  -> Red LED
 * GPIO_PIN_PB1  -> Yellow LED
 * GPIO_PIN_PB2  -> Green LED
 */

/************ Global Variables ********************************************************************************/
GPIO_PinConfig_t gpioConfig;
ADC_Config_t adcConfig;

uint8_t adcChannels[] = {ADC_IN10};
uint16_t analogRead = 0;
/**************************************************************************************************************/

/************ Function Headers ********************************************************************************/
void configGpioPinPC0_analog(void);
void configGpioLEDs(void);
/**************************************************************************************************************/

/************ Main Function ***********************************************************************************/
int main(void)
{
	LCD_Init8Bit();
	configGpioPinPC0_analog();
	configGpioLEDs();
	ADC_StartConversion(ADC1, adcConfig.ADC_ConvGroup, ADC_SINL_CONV_MODE);

	while(1){}
}
/**************************************************************************************************************/

/************ Function Definitions ****************************************************************************/
void configGpioPinPC0_analog(void)
{
	// Setup for analog mode pin C0, ADC1
	gpioConfig.GPIO_PinNumber = GPIO_PIN_NO_0;
	gpioConfig.GPIO_PinMode = GPIO_MODE_ANALOG;
	gpioConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	GPIO_Init(GPIOC, gpioConfig);

	adcConfig.ADC_BitRes = ADC_12BIT_RESOLUTION;
	adcConfig.ADC_SampTime = ADC_003_CYCLES;
	adcConfig.ADC_EOCSelect = ADC_END_OF_EACH;
	adcConfig.ADC_ConvGroup = ADC_REGULAR_GROUP;
	adcConfig.ADC_DataAlign = ADC_DATA_ALIGNMENT_RIGHT;
	adcConfig.ADC_ScanMode = 0;
	adcConfig.ADC_ClkPreSclr = ADC_PCLK_DIV2;
	adcConfig.ADC_ItEnable = ADC_INTERRUPT_ENABLE;
	adcConfig.ADC_WtDgEnable = ADC_WATCHDOG_DISABLE;
	adcConfig.ADC_DMAEnable = ADC_DMA_DISABLE;
	ADC_Init(ADC1, adcConfig);
	ADC_RegChannelSel(ADC1, ADC_01_CONVERSIONS, adcChannels);
	ADC_ConfigSampRate(ADC1, adcChannels[0], adcConfig.ADC_SampTime);
	ADC_SelectEOCFlagTrigger(ADC1, adcConfig);
	IRQInterruptConfig(ADC_IRQn, ENABLE);
}

void configGpioLEDs(void)
{
	// Configure Red LED
	gpioConfig.GPIO_PinNumber = GPIO_PIN_NO_0;
	gpioConfig.GPIO_PinMode = GPIO_MODE_OUT;
	gpioConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	GPIO_Init(GPIOB, gpioConfig);

	// Configure Yellow LED
	gpioConfig.GPIO_PinNumber = GPIO_PIN_NO_1;
	GPIO_Init(GPIOB, gpioConfig);

	// Configure Green LED
	gpioConfig.GPIO_PinNumber = GPIO_PIN_NO_2;
	GPIO_Init(GPIOB, gpioConfig);

	GPIOB->MODER = 0x15;
}
/**************************************************************************************************************/

/************ IRQ Handling and Application Event Callbacks ****************************************************/
void ADC_IRQHandler(void)
{
	ADC_IRQHandling(ADC1);
}

extern void ADC_ApplicationEventCallback(uint8_t appEv)
{
	if(appEv == ADC_END_OF_CONVERSION_REG || appEv == ADC_END_OF_CONVERSION_INJ)
	{
		analogRead = (((255.0 / ADC_MAX) * ADC_ReadRegDR(ADC1)) - (255.0 / ADC_MAX));
		LCD_Print("Value: %u   ", analogRead);
		LCD_SetRowBottom();

		if(analogRead > 245)
		{
			LCD_Print("%s%c", "Red Light     ", 'R');
			GPIO_WriteToOutputPin(GPIOB, GPIO_PIN_NO_0, SET);
			GPIO_WriteToOutputPin(GPIOB, GPIO_PIN_NO_1, RESET);
			GPIO_WriteToOutputPin(GPIOB, GPIO_PIN_NO_2, RESET);
		}
		else if(analogRead <= 245 && analogRead > 55)
		{
			LCD_Print("%s%c", "Yellow Light  ", 'Y');
			GPIO_WriteToOutputPin(GPIOB, GPIO_PIN_NO_0, RESET);
			GPIO_WriteToOutputPin(GPIOB, GPIO_PIN_NO_1, SET);
			GPIO_WriteToOutputPin(GPIOB, GPIO_PIN_NO_2, RESET);
		}
		else
		{
			LCD_Print("%s%c", "Green Light   ", 'G');
			GPIO_WriteToOutputPin(GPIOB, GPIO_PIN_NO_0, RESET);
			GPIO_WriteToOutputPin(GPIOB, GPIO_PIN_NO_1, RESET);
			GPIO_WriteToOutputPin(GPIOB, GPIO_PIN_NO_2, SET);
		}

		ADC_StartConversion(ADC1, adcConfig.ADC_ConvGroup, ADC_SINL_CONV_MODE);
	}
	else if(appEv == ADC_OVERRUN_SET)
		ADC_StartConversion(ADC1, adcConfig.ADC_ConvGroup, ADC_SINL_CONV_MODE);

	LCD_SetRowTop();
	TIM2_5_Delay_us(TIM2, 1000000/4);
}

#endif /* _ANALOG_MEASUREMENT_LCD */

#ifdef _ANALOG_MEASUREMENT_LCD_UART

/*
 * This program uses the Analog to Digital Conversion peripheral to convert an analog
 * input to a digital value and display it on a 2x16 LCD module. The LCD module uses
 * 8-bit configuration as well as the Timer 2 peripheral. The value of the analog input
 * is determined by the use of a potentiometer. Functionallity of the LCD_Print() function
 * is demonstrated by printing the ADC converted value on the LCD module. Three LEDs will
 * light up on a bread board depending on the value. The contents printed on the LCD module
 * are also printed on a console via a UART USB module.
 */

#include<stdio.h>
#include<string.h>
#include"lcd2_16.h"
#include"tim.h"
#include"adc.h"
#include"irq.h"
#include"gpio.h"
#include"uart.h"

#define POT_MAX 10400U
#define ADC_MAX 4094.0

/*
 * GPIO_PIN_PC0  -> ADC1
 *
 * GPIO_PIN_PA2  -> UART2
 *
 * GPIO_PIN_PD0  -> LCD_DB0
 * GPIO_PIN_PD1  -> LCD_DB1
 * GPIO_PIN_PD2  -> LCD_DB2
 * GPIO_PIN_PD3  -> LCD_DB3
 * GPIO_PIN_PD4  -> LCD_DB4
 * GPIO_PIN_PD5  -> LCD_DB5
 * GPIO_PIN_PD6  -> LCD_DB6
 * GPIO_PIN_PD7  -> LCD_DB7
 * GPIO_PIN_PD8  -> LCD_RW
 * GPIO_PIN_PD9  -> LCD_RS
 * GPIO_PIN_PD10 -> LCD_E
 *
 * GPIO_PIN_PB0  -> Red LED
 * GPIO_PIN_PB1  -> Yellow LED
 * GPIO_PIN_PB2  -> Green LED
 */

/************ Global Variables ********************************************************************************/
GPIO_PinConfig_t gpioConfig;
ADC_Config_t adcConfig;

uint8_t adcChannels[] = {ADC_IN10};
uint16_t analogRead = 0;
/**************************************************************************************************************/

/************ Function Headers ********************************************************************************/
void configGpioPinPC0_analog(void);
void configGpioLEDs(void);
/**************************************************************************************************************/

/************ Main Function ***********************************************************************************/
int main(void)
{
	LCD_Init8Bit();
	configGpioPinPC0_analog();
	uart_init();
	printf("Progam Start\n\n");
	configGpioLEDs();
	ADC_StartConversion(ADC1, adcConfig.ADC_ConvGroup, ADC_SINL_CONV_MODE);

	while(1){}
}
/**************************************************************************************************************/

/************ Function Definitions ****************************************************************************/
void configGpioPinPC0_analog(void)
{
	// Setup for analog mode pin C0, ADC1
	gpioConfig.GPIO_PinNumber = GPIO_PIN_NO_0;
	gpioConfig.GPIO_PinMode = GPIO_MODE_ANALOG;
	gpioConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	GPIO_Init(GPIOC, gpioConfig);

	adcConfig.ADC_BitRes = ADC_12BIT_RESOLUTION;
	adcConfig.ADC_SampTime = ADC_003_CYCLES;
	adcConfig.ADC_EOCSelect = ADC_END_OF_EACH;
	adcConfig.ADC_ConvGroup = ADC_REGULAR_GROUP;
	adcConfig.ADC_DataAlign = ADC_DATA_ALIGNMENT_RIGHT;
	adcConfig.ADC_ScanMode = 0;
	adcConfig.ADC_ClkPreSclr = ADC_PCLK_DIV2;
	adcConfig.ADC_ItEnable = ADC_INTERRUPT_ENABLE;
	adcConfig.ADC_WtDgEnable = ADC_WATCHDOG_DISABLE;
	adcConfig.ADC_DMAEnable = ADC_DMA_DISABLE;
	ADC_Init(ADC1, adcConfig);
	ADC_RegChannelSel(ADC1, ADC_01_CONVERSIONS, adcChannels);
	ADC_ConfigSampRate(ADC1, adcChannels[0], adcConfig.ADC_SampTime);
	ADC_SelectEOCFlagTrigger(ADC1, adcConfig);
	IRQInterruptConfig(ADC_IRQn, ENABLE);
}

void configGpioLEDs(void)
{
	// Configure Red LED
	gpioConfig.GPIO_PinNumber = GPIO_PIN_NO_0;
	gpioConfig.GPIO_PinMode = GPIO_MODE_OUT;
	gpioConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	GPIO_Init(GPIOB, gpioConfig);

	// Configure Yellow LED
	gpioConfig.GPIO_PinNumber = GPIO_PIN_NO_1;
	GPIO_Init(GPIOB, gpioConfig);

	// Configure Green LED
	gpioConfig.GPIO_PinNumber = GPIO_PIN_NO_2;
	GPIO_Init(GPIOB, gpioConfig);

	GPIOB->MODER = 0x15;
}
/**************************************************************************************************************/

/************ IRQ Handling and Application Event Callbacks ****************************************************/
void ADC_IRQHandler(void)
{
	ADC_IRQHandling(ADC1);
}

extern void ADC_ApplicationEventCallback(uint8_t appEv)
{
	if(appEv == ADC_END_OF_CONVERSION_REG || appEv == ADC_END_OF_CONVERSION_INJ)
	{
		analogRead = (((255.0 / ADC_MAX) * ADC_ReadRegDR(ADC1)) - (255.0 / ADC_MAX));
		LCD_Print("Value: %u   ", analogRead);
		printf("Value: %u\n", analogRead);
		LCD_SetRowBottom();

		if(analogRead > 245)
		{
			LCD_Print("%s%c", "Red Light     ", 'R');
			printf("%s%c\n\n", "Red Light     ", 'R');
			GPIO_WriteToOutputPin(GPIOB, GPIO_PIN_NO_0, SET);
			GPIO_WriteToOutputPin(GPIOB, GPIO_PIN_NO_1, RESET);
			GPIO_WriteToOutputPin(GPIOB, GPIO_PIN_NO_2, RESET);
		}
		else if(analogRead <= 245 && analogRead > 55)
		{
			LCD_Print("%s%c", "Yellow Light  ", 'Y');
			printf("%s%c\n\n", "Yellow Light  ", 'Y');
			GPIO_WriteToOutputPin(GPIOB, GPIO_PIN_NO_0, RESET);
			GPIO_WriteToOutputPin(GPIOB, GPIO_PIN_NO_1, SET);
			GPIO_WriteToOutputPin(GPIOB, GPIO_PIN_NO_2, RESET);
		}
		else
		{
			LCD_Print("%s%c", "Green Light   ", 'G');
			printf("%s%c\n\n", "Green Light   ", 'G');
			GPIO_WriteToOutputPin(GPIOB, GPIO_PIN_NO_0, RESET);
			GPIO_WriteToOutputPin(GPIOB, GPIO_PIN_NO_1, RESET);
			GPIO_WriteToOutputPin(GPIOB, GPIO_PIN_NO_2, SET);
		}

		ADC_StartConversion(ADC1, adcConfig.ADC_ConvGroup, ADC_SINL_CONV_MODE);
	}
	else if(appEv == ADC_OVERRUN_SET)
		ADC_StartConversion(ADC1, adcConfig.ADC_ConvGroup, ADC_SINL_CONV_MODE);

	LCD_SetRowTop();
	TIM2_5_Delay_us(TIM2, 1000000/4);
}

#endif /* _ANALOG_MEASUREMENT_LCD_UART */

#ifdef _USER_PUSHBUTTON_INTERRUPT_LEDS

/*
 * This program uses the four LED lights on board and the user button
 * to demonstrate the functionality of External Interrupts. Upon stating,
 * the program lights up the on-board LEDs in clockwise order. If the
 * user button is pressed, an external interrupt occurs and changes
 * direction of the LED cycle.
 */

/*
 * GPIO_PIN_PA00 -> User Push Button
 *
 * GPIO_PIN_PD12 -> Green LED
 * GPIO_PIN_PD13 -> Orange LED
 * GPIO_PIN_PD14 -> Red LED
 * GPIO_PIN_PD15 -> Blue LED
 */

#include<stdint.h>
#include"gpio.h"
#include"irq.h"
#include"tim.h"

/************ Global Variables and Macros *********************************************************************/
#define CLOCKWISE 0U
#define COUNTER_CLOCKWISE 1U
#define DELAY 100000U

GPIO_PinConfig_t userBtn;
volatile uint32_t delay_us = DELAY;
volatile uint8_t ledPosition = 0b0001U;
volatile uint8_t direction = CLOCKWISE;
/**************************************************************************************************************/

/************ Function Headers ********************************************************************************/
void gpio_leds_init(void);
void gpio_userBtn_init(void);
uint8_t rotateBitsClockwise(uint8_t value);
uint8_t rotateBitsCounterClockwise(uint8_t value);
/**************************************************************************************************************/

/************ Main Function ***********************************************************************************/
int main(void)
{
	gpio_leds_init();
	gpio_userBtn_init();
	TIM2_5_Init(TIM2, PULSE_MODE_DISABLE);

	while(1)
	{
		if(direction == CLOCKWISE)
		{
			for(uint8_t i = 0; i < 4; i++)
				GPIO_WriteToOutputPin(GPIOD, GPIO_PIN_NO_12 + i, ((ledPosition >> i) & 0b0001U));

			ledPosition = rotateBitsClockwise(ledPosition);
		}
		else
		{
			for(uint8_t i = 0; i < 4; i++)
				GPIO_WriteToOutputPin(GPIOD, GPIO_PIN_NO_12 + i, ((ledPosition >> i) & 0b0001U));

			ledPosition = rotateBitsCounterClockwise(ledPosition);
		}

		TIM2_5_Delay_us(TIM2, delay_us);
	}
}
/**************************************************************************************************************/

/************ Function Definitions ****************************************************************************/
void gpio_leds_init(void)
{
	GPIO_PinConfig_t leds;
	leds.GPIO_PinMode = GPIO_MODE_OUT;
	leds.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	leds.GPIO_PinPuPdControl = GPIO_NO_PUPD;
	leds.GPIO_PinSpeed = GPIO_SPEED_FAST;

	leds.GPIO_PinNumber = GPIO_PIN_NO_12;
	GPIO_Init(GPIOD, leds);// Green LED

	leds.GPIO_PinNumber = GPIO_PIN_NO_13;
	GPIO_Init(GPIOD, leds);// Orange LED

	leds.GPIO_PinNumber = GPIO_PIN_NO_14;
	GPIO_Init(GPIOD, leds);// Red LED

	leds.GPIO_PinNumber = GPIO_PIN_NO_15;
	GPIO_Init(GPIOD, leds);// Blue LED
}

void gpio_userBtn_init(void)
{
	userBtn.GPIO_PinMode = GPIO_MODE_IT_RT;
	userBtn.GPIO_PinNumber = GPIO_PIN_NO_0;
	userBtn.GPIO_PinPuPdControl = GPIO_PIN_PD;
	GPIO_Init(GPIOA, userBtn);
}

uint8_t rotateBitsClockwise(uint8_t value)
{
	return ((value << 1) | (value >> (4 - 1)));
}

uint8_t rotateBitsCounterClockwise(uint8_t value)
{
	return ((value >> 1) | (value << (4 - 1)));
}
/**************************************************************************************************************/

/************ IRQ Handling and Application Event Callbacks ****************************************************/
void EXTI0_IRQHandler(void)
{
	GPIO_IRQHandling(GPIO_PIN_NO_0);
}

extern void GPIO_ApplicationEventCallBack(void)
{
	if(direction == CLOCKWISE)
		direction = COUNTER_CLOCKWISE;
	else
		direction = CLOCKWISE;
}
/**************************************************************************************************************/

#endif /* _USER_PUSHBUTTON_INTERRUPT_LEDS */

#ifdef _SPI_DATA_TX_ONLY

/*
 * This program demonstrates SPI communication with an arduino board; STM32 board as master and the
 * arduino as slave. The STM32 board is connected to four push buttons and the arduino through a
 * voltage level converter module. The arduino is connected to four LEDs that correspond to the STM's
 * push buttons. When a push button is pressed, the STM32 sends data to the arduino via SPI peripheral
 * to turn on the corresponding LED. The press of the button causes an Interrupt Service Routine to send
 * the data. The baud rate for the SPI communication is 500kHz.
 */

/*
 * SPI2 Pin Configurations:
 * 	PB12 -> NSS
 * 	PB13 -> SCK
 * 	PB14 -> MISO
 * 	PB15 -> MOSI
 *
 * Push Button Pins:
 *  PC00 -> Red Push Button
 *  PC01 -> Yellow Push Button
 *  PC02 -> Green Push Button
 *  PC03 -> Blue Push Button
 */

#include<stdint.h>
#include<string.h>
#include"spi.h"
#include"gpio.h"

/************ Global Variables and Macros *********************************************************************/
#define NSS				GPIO_PIN_NO_12
#define CLK				GPIO_PIN_NO_13
#define MISO			GPIO_PIN_NO_14
#define MOSI			GPIO_PIN_NO_15

#define RED_BTN			GPIO_PIN_NO_4
#define YELLOW_BTN		GPIO_PIN_NO_1
#define GREEN_BTN		GPIO_PIN_NO_2
#define BLUE_BTN		GPIO_PIN_NO_3

#define RED_BTN_PRESSED			10
#define YELLOW_BTN_PRESSED		15
#define GREEN_BTN_PRESSED		20
#define BLUE_BTN_PRESSED		25

uint8_t btnPressed;
/**************************************************************************************************************/

/************ Function Headers ********************************************************************************/
void setUpSpi(void);
void setUpButtons(void);
/**************************************************************************************************************/

/************ Main Function ***********************************************************************************/
int main(void)
{
	setUpButtons();
	setUpSpi();
	btnPressed = 0;
	uint8_t dataTx[1];
	while(1)
	{
		if(btnPressed == RED_BTN_PRESSED)
		{
			dataTx[0] = 'r';
			GPIO_WriteToOutputPin(GPIOB, NSS, RESET);
			SPI_MasterTransmissionStartTx(SPI2, dataTx);
			GPIO_WriteToOutputPin(GPIOB, NSS, SET);
			btnPressed = 0;
		}
		else if(btnPressed == YELLOW_BTN_PRESSED)
		{
			dataTx[0] = 'y';
			GPIO_WriteToOutputPin(GPIOB, NSS, RESET);
			SPI_MasterTransmissionStartTx(SPI2, dataTx);
			GPIO_WriteToOutputPin(GPIOB, NSS, SET);
			btnPressed = 0;
		}
		else if(btnPressed == GREEN_BTN_PRESSED)
		{
			dataTx[0] = 'g';
			GPIO_WriteToOutputPin(GPIOB, NSS, RESET);
			SPI_MasterTransmissionStartTx(SPI2, dataTx);
			GPIO_WriteToOutputPin(GPIOB, NSS, SET);
			btnPressed = 0;
		}
		else if(btnPressed == BLUE_BTN_PRESSED)
		{
			dataTx[0] = 'b';
			GPIO_WriteToOutputPin(GPIOB, NSS, RESET);
			SPI_MasterTransmissionStartTx(SPI2, dataTx);
			GPIO_WriteToOutputPin(GPIOB, NSS, SET);
			btnPressed = 0;
		}
	}
}
/**************************************************************************************************************/

/************ Function Definitions ****************************************************************************/
void setUpSpi(void)
{
	SPI_Config_t spiConfig;
	GPIO_PinConfig_t pins;

	//Configure GPIO pins for SPI2
	pins.GPIO_PinNumber = NSS;
	pins.GPIO_PinMode = GPIO_MODE_OUT;
	GPIO_Init(GPIOB, pins); //NSS
	GPIO_WriteToOutputPin(GPIOB, NSS, SET);

	pins.GPIO_PinMode = GPIO_MODE_ALTFN;
	pins.GPIO_PinAltFunMode = GPIO_AF_05;
	pins.GPIO_PinNumber = CLK;
	GPIO_Init(GPIOB, pins); //CLK

	//pins.GPIO_PinNumber = MISO;
	//GPIO_Init(GPIOB, pins); //MISO

	pins.GPIO_PinNumber = MOSI;
	pins.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	GPIO_Init(GPIOB, pins); //MOSI

	//Configure SPI Peripheral
	spiConfig.SPI_bidirectionalMode = SPI_1LINE_BIDRECTIONAL_TX_ONLY_MODE;
	spiConfig.SPI_baudRateDiv = SPI_BAUD_DIV_32; //Baud Rate = 16MHz/32 = 500kHz
	spiConfig.SPI_bitOrder = SPI_MSB_TRANSMIT_FIRST;
	spiConfig.SPI_dataBitMode = SPI_8BIT_DATA_FRAME;
	spiConfig.SPI_clkRelationship = SPI_CLOCK_POL_PHA_1;

	SPI_InitMaster(SPI2, spiConfig);
}

void setUpButtons(void)
{
	GPIO_PinConfig_t btnConfig;
	btnConfig.GPIO_PinMode = GPIO_MODE_IT_RT;
	btnConfig.GPIO_PinPuPdControl = GPIO_PIN_PD;
	btnConfig.GPIO_PinNumber = RED_BTN;
	GPIO_Init(GPIOC, btnConfig);	//Red Push Button

	btnConfig.GPIO_PinNumber = YELLOW_BTN;
	GPIO_Init(GPIOC, btnConfig);	// Yellow Push Button

	btnConfig.GPIO_PinNumber = GREEN_BTN;
	GPIO_Init(GPIOC, btnConfig);	// Green Push Button

	btnConfig.GPIO_PinNumber = BLUE_BTN;
	GPIO_Init(GPIOC, btnConfig);	// Blue Push Button

	GPIOC->PUPDR |= (0x2A8 << 0);
}
/**************************************************************************************************************/

/************ IRQ Handling and Application Event Callbacks ****************************************************/

void EXTI1_IRQHandler(void)
{
	EXTI1_IRQHandling();
}

void EXTI2_IRQHandler(void)
{
	EXTI2_IRQHandling();
}

void EXTI3_IRQHandler(void)
{
	EXTI3_IRQHandling();
}

void EXTI4_IRQHandler(void)
{
	EXTI4_IRQHandling();
}

extern void GPIO_ApplicationEventCallBack(uint8_t pinNumber)
{
	if(pinNumber == RED_BTN)
		btnPressed = RED_BTN_PRESSED;
	else if(pinNumber == YELLOW_BTN)
		btnPressed = YELLOW_BTN_PRESSED;
	else if(pinNumber == GREEN_BTN)
		btnPressed = GREEN_BTN_PRESSED;
	else if(pinNumber == BLUE_BTN)
		btnPressed = BLUE_BTN_PRESSED;
}
/**************************************************************************************************************/

#endif /* _SPI_DATA_TX_ONLY */

#ifdef _SPI_DATA_TX_RX

/*
 * This program demonstrates SPI communication with an arduino board; STM32 board as master and the
 * arduino as slave. The STM32 board is connected to four push buttons and the arduino through a
 * voltage level converter module. The arduino is connected to four LEDs that correspond to the STM's
 * push buttons. When a push button is pressed, the STM32 sends data to the arduino via SPI peripheral
 * to turn on the corresponding LED. The press of the button causes an Interrupt Service Routine to send
 * the data. The baud rate for the SPI communication is 250kHz, any faster and the Arduino cannot keep up.
 */

/*
 * SPI2 Pin Configurations:
 * 	PB12 -> NSS
 * 	PB13 -> SCK
 * 	PB14 -> MISO
 * 	PB15 -> MOSI
 *
 * Push Button Pins:
 *  PC00 -> Red Push Button
 *  PC01 -> Yellow Push Button
 *  PC02 -> Green Push Button
 *  PC03 -> Blue Push Button
 */

#include<stdint.h>
#include<string.h>
#include"spi.h"
#include"gpio.h"

/************ Global Variables and Macros *********************************************************************/
#define NSS				GPIO_PIN_NO_12
#define CLK				GPIO_PIN_NO_13
#define MISO			GPIO_PIN_NO_14
#define MOSI			GPIO_PIN_NO_15

#define RED_BTN			GPIO_PIN_NO_4
#define YELLOW_BTN		GPIO_PIN_NO_1
#define GREEN_BTN		GPIO_PIN_NO_2
#define BLUE_BTN		GPIO_PIN_NO_3

#define RED_BTN_PRESSED			10
#define YELLOW_BTN_PRESSED		15
#define GREEN_BTN_PRESSED		20
#define BLUE_BTN_PRESSED		25

uint8_t* dataRx;
uint8_t btnPressed;
/**************************************************************************************************************/

/************ Function Headers ********************************************************************************/
void setUpSpi(void);
void setUpButtons(void);
void strToUint8P(uint8_t* pUint8, char* pChr);
/**************************************************************************************************************/

/************ Main Function ***********************************************************************************/
int main(void)
{
	setUpButtons();
	setUpSpi();
	btnPressed = 0;
	while(1)
	{
		switch(btnPressed)
		{
			case RED_BTN_PRESSED:
			{
				uint8_t dataTx[3];
				strToUint8P(dataTx, "red");
				GPIO_WriteToOutputPin(GPIOB, NSS, RESET);
				SPI_MasterTransmissionStartTxRx(SPI2, dataTx, dataRx);
				GPIO_WriteToOutputPin(GPIOB, NSS, SET);
				btnPressed = 0;
				break;
			}
			case YELLOW_BTN_PRESSED:
			{
				uint8_t dataTx[6];
				strToUint8P(dataTx, "yellow");
				GPIO_WriteToOutputPin(GPIOB, NSS, RESET);
				SPI_MasterTransmissionStartTxRx(SPI2, dataTx, dataRx);
				GPIO_WriteToOutputPin(GPIOB, NSS, SET);
				btnPressed = 0;
				break;
			}
			case GREEN_BTN_PRESSED:
			{
				uint8_t dataTx[5];
				strToUint8P(dataTx, "green");
				GPIO_WriteToOutputPin(GPIOB, NSS, RESET);
				SPI_MasterTransmissionStartTxRx(SPI2, dataTx, dataRx);
				GPIO_WriteToOutputPin(GPIOB, NSS, SET);
				btnPressed = 0;
				break;
			}
			case BLUE_BTN_PRESSED:
			{
				uint8_t dataTx[4];
				strToUint8P(dataTx, "blue");
				GPIO_WriteToOutputPin(GPIOB, NSS, RESET);
				SPI_MasterTransmissionStartTxRx(SPI2, dataTx, dataRx);
				GPIO_WriteToOutputPin(GPIOB, NSS, SET);
				btnPressed = 0;
				break;
			}
		}
	}
}
/**************************************************************************************************************/

/************ Function Definitions ****************************************************************************/
void setUpSpi(void)
{
	SPI_Config_t spiConfig;
	GPIO_PinConfig_t pins;

	//Configure GPIO pins for SPI2
	pins.GPIO_PinNumber = NSS;
	pins.GPIO_PinMode = GPIO_MODE_OUT;
	GPIO_Init(GPIOB, pins); //NSS
	GPIO_WriteToOutputPin(GPIOB, NSS, SET);

	pins.GPIO_PinMode = GPIO_MODE_ALTFN;
	pins.GPIO_PinAltFunMode = GPIO_AF_05;
	pins.GPIO_PinNumber = CLK;
	GPIO_Init(GPIOB, pins); //CLK

	pins.GPIO_PinNumber = MISO;
	GPIO_Init(GPIOB, pins); //MISO

	pins.GPIO_PinNumber = MOSI;
	pins.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	GPIO_Init(GPIOB, pins); //MOSI

	//Configure SPI Peripheral
	spiConfig.SPI_bidirectionalMode = SPI_2LINE_UNIDIRECTIONAL_RX_TX_MODE;
	spiConfig.SPI_baudRateDiv = SPI_BAUD_DIV_64; //Baud Rate = 16MHz/64 = 250kHz
	spiConfig.SPI_bitOrder = SPI_MSB_TRANSMIT_FIRST;
	spiConfig.SPI_dataBitMode = SPI_8BIT_DATA_FRAME;
	spiConfig.SPI_clkRelationship = SPI_CLOCK_POL_PHA_1;

	SPI_InitMaster(SPI2, spiConfig);
}

void setUpButtons(void)
{
	GPIO_PinConfig_t btnConfig;
	btnConfig.GPIO_PinMode = GPIO_MODE_IT_RT;
	btnConfig.GPIO_PinPuPdControl = GPIO_PIN_PD;
	btnConfig.GPIO_PinNumber = RED_BTN;
	GPIO_Init(GPIOC, btnConfig);	//Red Push Button

	btnConfig.GPIO_PinNumber = YELLOW_BTN;
	GPIO_Init(GPIOC, btnConfig);	// Yellow Push Button

	btnConfig.GPIO_PinNumber = GREEN_BTN;
	GPIO_Init(GPIOC, btnConfig);	// Green Push Button

	btnConfig.GPIO_PinNumber = BLUE_BTN;
	GPIO_Init(GPIOC, btnConfig);	// Blue Push Button

	GPIOC->PUPDR |= (0x2A8 << 0);
}

void strToUint8P(uint8_t* pUint8, char* pChr)
{
	uint8_t inc = 0;
	for(char* p = pChr; *p != '\0'; p++)
	{
		*pUint8 = (uint8_t)*p;
		pUint8++;
		inc++;
	}
	*pUint8 = '\0';
	pUint8 -= inc;
}
/**************************************************************************************************************/

/************ IRQ Handling and Application Event Callbacks ****************************************************/

void EXTI1_IRQHandler(void)
{
	EXTI1_IRQHandling();
}

void EXTI2_IRQHandler(void)
{
	EXTI2_IRQHandling();
}

void EXTI3_IRQHandler(void)
{
	EXTI3_IRQHandling();
}

void EXTI4_IRQHandler(void)
{
	EXTI4_IRQHandling();
}

extern void GPIO_ApplicationEventCallBack(uint8_t pinNumber)
{
	if(pinNumber == RED_BTN)
		btnPressed = RED_BTN_PRESSED;
	else if(pinNumber == YELLOW_BTN)
		btnPressed = YELLOW_BTN_PRESSED;
	else if(pinNumber == GREEN_BTN)
		btnPressed = GREEN_BTN_PRESSED;
	else if(pinNumber == BLUE_BTN)
		btnPressed = BLUE_BTN_PRESSED;
}
/**************************************************************************************************************/

#endif /* _SPI_DATA_TX_RX */
