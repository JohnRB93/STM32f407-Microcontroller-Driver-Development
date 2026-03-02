#ifndef __RCC_DRIVER_H_
#define __RCC_DRIVER_H_

#include"stm32f4xx.h"

/***************** RCC Structure Definitions *******************************************/

//PLL Configuration Structure Definition
typedef struct
{
	__IO uint8_t PLL_M;		/* Division factor for the main PLL (PLL) and
						 * audio PLL (PLLI2S) input clock.
						 * 2 <= PLLM <= 63								*/

	__IO uint16_t PLL_N;		/* Main PLL (PLL) multiplication factor for VCO.
						 * 50 <= PLLN <= 432							*/

	__IO uint8_t PLL_P;		/* Main PLL (PLL) division factor for main
						 * system clock.
						 * Possible values from @PLL_P					*/

	__IO uint8_t PLL_SRC;	/* Main PLL(PLL) and audio PLL (PLLI2S) entry
						 * clock source.
						 * Possible values from @PLL_SRC				*/

	__IO uint8_t PLL_Q;		/* Main PLL (PLL) division factor for USB OTG FS,
						 * SDIO and random number generator.
						 * 2 <= PLLQ <= 15								*/
}RCC_PLL_Config_t;

//RCC Configuration Structure Definition
typedef struct
{
	__IO uint8_t RCC_ClockSource;			//@RCC_ClockSource
	__IO uint32_t RCC_HSE_Frequency;			//4 - 26 MHz
	__IO uint8_t RCC_AHB_Prescaler;			//@AHB_Prescaler
	__IO uint8_t RCC_APB_LSPrescaler;		//@APB_LowSpeedPrescaler
	__IO uint8_t RCC_APB_HSPrescaler;		//@APB_HighSpeedPrescaler
	__IO uint8_t RCC_HSE_DivRTC;				//@HSE_DivisionFactorForRTC_Clock
	__IO uint8_t RCC_MCO1_ClkOut;			//@MicrocontrollerClockOutput1
	__IO uint8_t RCC_MCO2_ClkOut;			//@MicrocontrollerClockOutput2
	__IO uint8_t RCC_MCO1_Prescaler;			//@MicrocontrollerPrescaler
	__IO uint8_t RCC_MCO2_Prescaler;			//@MicrocontrollerPrescaler
	__IO uint8_t RCC_I2S_ClkSel;				//@I2S_ClockSelection
	RCC_PLL_Config_t RCC_PLL_Config;
}RCC_Config_t;

/***************************************************************************************/

/***************** Macro Definitions ***************************************************/

//@RCC_ClockSource
#define RCC_SOURCE_HSI			0
#define RCC_SOURCE_HSE			1
#define RCC_SOURCE_PLL			2

//@AHB_Prescaler
#define RCC_AHB_DIV_002			8
#define RCC_AHB_DIV_004			9
#define RCC_AHB_DIV_008			10
#define RCC_AHB_DIV_016			11
#define RCC_AHB_DIV_064			12
#define RCC_AHB_DIV_128			13
#define RCC_AHB_DIV_256			14
#define RCC_AHB_DIV_512			15

//@APB_Prescaler
#define RCC_AHB_DIV_02			4
#define RCC_AHB_DIV_04			5
#define RCC_AHB_DIV_08			6
#define RCC_AHB_DIV_16			7

//@HSE_DivisionFactorForRTC_Clock
#define RCC_HSE_DIV_00			0
#define RCC_HSE_DIV_01			RCC_HSE_DIV_00
#define RCC_HSE_DIV_02			2
#define RCC_HSE_DIV_03			3
#define RCC_HSE_DIV_04			4
#define RCC_HSE_DIV_05			5
#define RCC_HSE_DIV_06			6
#define RCC_HSE_DIV_07			7
#define RCC_HSE_DIV_08			8
#define RCC_HSE_DIV_09			9
#define RCC_HSE_DIV_10			10
#define RCC_HSE_DIV_11			11
#define RCC_HSE_DIV_12			12
#define RCC_HSE_DIV_13			13
#define RCC_HSE_DIV_14			14
#define RCC_HSE_DIV_15			15
#define RCC_HSE_DIV_16			16
#define RCC_HSE_DIV_17			17
#define RCC_HSE_DIV_18			18
#define RCC_HSE_DIV_19			19
#define RCC_HSE_DIV_20			20
#define RCC_HSE_DIV_21			21
#define RCC_HSE_DIV_22			22
#define RCC_HSE_DIV_23			23
#define RCC_HSE_DIV_24			24
#define RCC_HSE_DIV_25			25
#define RCC_HSE_DIV_26			26
#define RCC_HSE_DIV_27			27
#define RCC_HSE_DIV_28			28
#define RCC_HSE_DIV_29			29
#define RCC_HSE_DIV_30			30
#define RCC_HSE_DIV_31			31

//@MicrocontrollerClockOutput1
#define RCC_MCO1_HSI_OUT		0
#define RCC_MCO1_LSE_OUT		1
#define RCC_MCO1_HSE_OUT		2
#define RCC_MCO1_PLL_OUT		3

//@MicrocontrollerClockOutput2
#define RCC_MCO2_SYSCLK_OUT		0
#define RCC_MCO2_PLLI2S_OUT		1
#define RCC_MCO2_HSE_OUT		2
#define RCC_MCO2_PLL_OUT		3

//@MicrocontrollerPrescaler
#define RCC_DIV_2				4
#define RCC_DIV_3				5
#define RCC_DIV_4				6
#define RCC_DIV_5				7

//@I2S_ClockSelection
#define RCC_PLLI2S_CLK			0
#define RCC_EXTERN_CLK			1

//@PLL_P
#define PLL_P_DIV_2				0
#define PLL_P_DIV_4				1
#define PLL_P_DIV_6				2
#define PLL_P_DIV_8				3

//@PLL_SRC
#define PLL_SRC_HSI				0
#define PLL_SRC_HSE				1



/***************************************************************************************/


/***************************************************************************************/
/*                         APIs supported by this driver                               */
/*      For more information about the the APIs check the function definitions         */
/***************************************************************************************/

void RCC_Config(RCC_Config_t RCC_Config);
uint8_t RCC_GetSysClkSwStatus(void);

uint32_t RCC_GetPCLK1Value(void);
uint32_t RCC_GetPCLK2Value(void);
uint32_t RCC_GetPLLOutputClock(void);

void RCC_ConfigPLLReg(RCC_PLL_Config_t PLL_Config);
void RCC_Enable(RCC_Config_t rccConfig);
void RCC_ToggleLseClk(uint8_t EnOrDi);

#endif /* __RCC_DRIVER_H_ */
