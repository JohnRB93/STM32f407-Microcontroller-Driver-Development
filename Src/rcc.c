#include"rcc.h"

#define __16MHZ 16000000U

uint16_t AHB_PreScaler[8] = {2, 4, 8, 16, 64, 128, 256, 512};
uint8_t APBx_PreScaler[4] = {2, 4, 8, 16};
uint32_t HSE_Clock = 0;


/***************** Private Helper Function Headers *************************************/

static void RCC_ConfigSysClk(uint8_t rccClockSource);
static void RCC_ConfigAHB_Prescaler(uint8_t ahbPrescaler);
static void RCC_ConfigAPB_LSpPrescaler(uint8_t apbLPrescaler);
static void RCC_ConfigAPB_HSpPrescaler(uint8_t apbHPrescaler);
static void RCC_ConfigHSE_DivRTC(uint8_t hseDiv);
static void RCC_ConfigMC_ClkOutput1(uint8_t mcClkOut);
static void RCC_ConfigMC_ClkOutput2(uint8_t mcClkOut);
static void RCC_ConfigI2S_ClkSelection(uint8_t i2sClkSel);
static void RCC_ConfigMC1_Prescaler(uint8_t mcPrescaler);
static void RCC_ConfigMC2_Prescaler(uint8_t mcPrescaler);

static void RCC_ConfigPLLM(uint8_t pllM);
static void RCC_ConfigPLLN(uint16_t pllN);
static void RCC_ConfigPLLP(uint8_t pllP);
static void RCC_ConfigPLLQ(uint8_t pllQ);
static void RCC_ConfigPLLSRC(uint8_t pllSrc);

/***************************************************************************************/


/***************** User Application Exposed Function Definitions ***********************/

/*
 * @fn			- RCC_Config
 *
 * @brief		- This function initializes the RCC peripheral with
 * 				  user provided settings in the configuration
 * 				  structure.
 *
 * @param[RCC_Handle_t*] - Base address of the RCC Handle.
 *
 * @return		- None.
 *
 * @note		- None.
 */
void RCC_Config(RCC_Config_t RCC_Config)
{
	RCC_ConfigSysClk(RCC_Config.RCC_ClockSource);
	RCC_ConfigAHB_Prescaler(RCC_Config.RCC_AHB_Prescaler);
	RCC_ConfigAPB_LSpPrescaler(RCC_Config.RCC_APB_LSPrescaler);
	RCC_ConfigAPB_HSpPrescaler(RCC_Config.RCC_APB_HSPrescaler);
	RCC_ConfigHSE_DivRTC(RCC_Config.RCC_HSE_DivRTC);
	RCC_ConfigMC_ClkOutput1(RCC_Config.RCC_MCO1_ClkOut);
	RCC_ConfigMC_ClkOutput2(RCC_Config.RCC_MCO2_ClkOut);
	RCC_ConfigI2S_ClkSelection(RCC_Config.RCC_I2S_ClkSel);
	RCC_ConfigMC1_Prescaler(RCC_Config.RCC_MCO1_Prescaler);
	RCC_ConfigMC2_Prescaler(RCC_Config.RCC_MCO2_Prescaler);
}

/*
 * @fn			- RCC_GetSysClkSwStatus
 *
 * @brief		- This function returns the system clock switch
 * 				  status.
 *
 * @param[void] - None.
 *
 * @return		- System Clock Switch Status(uint8_t).
 *
 * @note		- None.
 */
uint8_t RCC_GetSysClkSwStatus(void)
{
	return ((RCC->CFGR >> RCC_CFGR_SWS_Pos) & 0x3);
}

/*
 * @fn			- RCC_GetPCLK1Value
 *
 * @brief		- Calculates the peripheral1 clock value.
 *
 * @param[void] - None.
 *
 * @return		- Value of the peripheral clock value(uint32_t).
 *
 * @note		- None.
 */
uint32_t RCC_GetPCLK1Value(void)
{
	uint32_t systemClk;
	uint8_t ahbp, apb1p, temp;

	//Find System Clock.
	temp = ((RCC->CFGR >> RCC_CFGR_SWS_Pos) & 0x3);
	if(temp == RCC_SOURCE_HSI)
		systemClk = __16MHZ;//System clock is HSI, 16MHz.
	else if(temp == RCC_SOURCE_HSE)
		systemClk = HSE_Clock;//System clock is HSE.
	else if(temp == RCC_SOURCE_PLL)
		systemClk = RCC_GetPLLOutputClock();//System clock is PLL.

	//Find AHB Prescaler
	temp = ((RCC->CFGR >> RCC_CFGR_HPRE_Pos) & 0xF);
	if(temp < RCC_AHB_DIV_002)
		ahbp = 1;//System clock not divided
	else
		ahbp = AHB_PreScaler[temp - 8];

	//Find APB Low Speed Prescaler.
	temp = ((RCC->CFGR >> RCC_CFGR_PPRE1_Pos) & 0x7);
	if(temp < RCC_AHB_DIV_02)
		apb1p = 1;//Quota from system clock and ahbp not divided
	else
		apb1p = APBx_PreScaler[temp - 4];

	return ((systemClk / ahbp) / apb1p);
}

/*
 * @fn			- RCC_GetPCLK2Value
 *
 * @brief		- Calculates the peripheral clock2 value.
 *
 * @param[void] - None.
 *
 * @return		- Value of the peripheral clock value(uint32_t).
 *
 * @note		- None.
 */
uint32_t RCC_GetPCLK2Value(void)
{
	uint32_t systemClk;
	uint8_t ahbp, apb2p, temp;

	//Find System Clock.
	temp = ((RCC->CFGR >> RCC_CFGR_SWS_Pos) & 0x3);
	if(temp == RCC_SOURCE_HSI)
		systemClk = __16MHZ;//System clock is HSI, 16MHz.
	else if(temp == RCC_SOURCE_HSE)
		systemClk = HSE_Clock;//System clock is HSE.
	else if(temp == RCC_SOURCE_PLL)
		systemClk = RCC_GetPLLOutputClock();//System clock is PLL.

	//Find AHB Prescaler
	temp = ((RCC->CFGR >> RCC_CFGR_HPRE_Pos) & 0xF);
	if(temp < RCC_AHB_DIV_002)
		ahbp = 1;//System clock not divided
	else
		ahbp = AHB_PreScaler[temp - 8];

	//Find APB High Speed Prescaler.
	temp = ((RCC->CFGR >> RCC_CFGR_PPRE2_Pos) & 0x7);
	if(temp < RCC_AHB_DIV_02)
		apb2p = 1;//Quota from system clock and ahbp not divided
	else
		apb2p = APBx_PreScaler[temp - 4];

	return ((systemClk / ahbp) / apb2p);
}

/*
 * @fn			- RCC_ConfigPLLReg
 *
 * @brief		- Configures the PLL Register according to
 * 				  the application-provided settings in the
 * 				  RCC_PLL_Config_t Structure.
 *
 * @param[RCC_PLL_Config_t] - Configuration Structure for PLL.
 *
 * @return		- None.
 *
 * @note		- PLL_M: The software has to set these bits correctly to ensure that the VCO input frequency
 *				  ranges from 1 to 2 MHz. It is recommended to select a frequency of 2 MHz to limit
 *				  PLL jitter.
 *				  PLL_N: The software has to set these bits correctly to ensure that the VCO output frequency
 *				  is between 100 and 432 MHz.(VCO output frequency = VCO input frequency * PLLN)
 *				  PLL_P: The software has to set these bits correctly not to exceed 168 MHz on this domain.
 *				  (PLL output clock frequency = VCO frequency / PLLP)
 *				  PLL_Q: The USB OTG FS requires a 48 MHz clock to work correctly. The SDIO and the random
 *				  number generator need a frequency lower than or equal to 48 MHz to work correctly.
 *				  (USB OTG FS clock frequency = VCO frequency / PLLQ)
 */
void RCC_ConfigPLLReg(RCC_PLL_Config_t PLL_Config)
{
	RCC_ConfigPLLM(PLL_Config.PLL_M);
	RCC_ConfigPLLN(PLL_Config.PLL_N);
	RCC_ConfigPLLP(PLL_Config.PLL_P);
	RCC_ConfigPLLQ(PLL_Config.PLL_Q);
	RCC_ConfigPLLSRC(PLL_Config.PLL_SRC);
}

/*
 * @fn			- RCC_GetPLLOutputClock
 *
 * @brief		- Calculates and returns the PLL output clock
 * 				  value for SYSCLK.
 *
 * @param[void] - None.
 *
 * @return		- PLL output clock value(uint32_t).
 *
 * @note		- Functionality for VCOclock that uses PLL_Q
 * 				  (USB OTG FS, SDIO, RNG clock output) not yet
 * 				  implemented.
 */
uint32_t RCC_GetPLLOutputClock(void)
{
	uint32_t vcoClk, pllClkInput;
	uint16_t pllNTemp;
	uint8_t pllP, temp;

	//Find PLL Clock Input.
	temp = ((RCC->PLLCFGR >> RCC_PLLCFGR_PLLSRC_Pos) & 0x1);
	pllClkInput = (temp == PLL_SRC_HSI) ? __16MHZ : HSE_Clock;

	//Find VCO Clock.
	pllNTemp = ((RCC->PLLCFGR >> RCC_PLLCFGR_PLLN_Pos) & 0x1FF);//PLLN
	temp = ((RCC->PLLCFGR >> RCC_PLLCFGR_PLLM_Pos) & 0x3F);//PLLM
	vcoClk = pllClkInput * (pllNTemp / temp);

	//Find PLLP.
	temp = ((RCC->PLLCFGR >> RCC_PLLCFGR_PLLP_Pos) & 0x3);
	if(temp == PLL_P_DIV_2)
		pllP = 2;
	else if(temp == PLL_P_DIV_4)
		pllP = 4;
	else if(temp == PLL_P_DIV_6)
		pllP = 6;
	else
		pllP = 8;

	return vcoClk / pllP;
}

/*
 * @fn			- RCC_Enable
 *
 * @brief		- This function enables oscillators according to
 * 				  application defined settings in the RCC
 * 				  Configuration Structure.
 *
 * @param[RCC_RegDef_t*]		- Base address of the RCC Register.
 * @param[RCC_Config_t]			- RCC Configuration Structure.
 *
 * @return		- None.
 *
 * @note		- None.
 */
void RCC_Enable(RCC_Config_t rccConfig)
{
	if(rccConfig.RCC_ClockSource == RCC_SOURCE_HSI)
	{
		RCC->CR |= RCC_CR_HSION;
		RCC->CR &= ~RCC_CR_HSEON;
		RCC->CR &= ~RCC_CR_PLLON;
	}
	else if(rccConfig.RCC_ClockSource == RCC_SOURCE_HSE)
	{
		RCC->CR |= RCC_CR_HSEON;
		RCC->CR &= ~RCC_CR_HSION;
		RCC->CR &= ~RCC_CR_PLLON;
	}
	else if(rccConfig.RCC_ClockSource == RCC_SOURCE_PLL)
	{
		FLASH->ACR |= (0x5 << 0);
		FLASH->ACR |= FLASH_ACR_PRFTEN;
		FLASH->ACR |= FLASH_ACR_ICEN;
		FLASH->ACR |= FLASH_ACR_DCEN;
		while(! (((FLASH->ACR >> 0) & 0x5) == 0x5) );

		RCC->CFGR &= ~RCC_CFGR_SW_Msk;
		RCC->CFGR |= RCC_CFGR_SW_1;
		RCC->CR &= ~RCC_CR_HSEON;
		RCC->CR |= RCC_CR_PLLON;

		while(RCC_GetSysClkSwStatus() != 0x2);
	}
}

/*
 * @fn			- RCC_ToggleLseClk
 *
 * @brief		- This function enables or disables the
 * 				  LSE Clock.
 *
 * @param[uint8_t]			- ENABLE or DISABLE
 *
 * @return		- None.
 *
 * @note		- None.
 */
void RCC_ToggleLseClk(uint8_t EnOrDi)
{
	if(EnOrDi == ENABLE)
		RCC->BDCR |= RCC_BDCR_LSEON;
	else
		RCC->BDCR &= ~RCC_BDCR_LSEON;
}


/***************************************************************************************/


/***************** Private Helper Function Definitions *********************************/

static void RCC_ConfigSysClk(uint8_t rccClockSource)
{
	if(rccClockSource == RCC_SOURCE_HSI)  //HSI will be the clock source.
		RCC->CFGR &= ~RCC_CFGR_SW_Msk;
	else if(rccClockSource == RCC_SOURCE_HSE)
	{//HSE will be the clock source.
		RCC->CFGR &= ~RCC_CFGR_SW_Msk;
		RCC->CFGR |= RCC_CFGR_SW_0;
		HSE_Clock = rccConfig.RCC_HSE_Frequency;
	}
	else if(rccClockSource == RCC_SOURCE_PLL)
	{//PLL will be the clock source.
		RCC->CFGR &= ~RCC_CFGR_SW_Msk;
		RCC->CFGR |= RCC_CFGR_SW_1;
	}else  //HSI will be the clock source.
		RCC->CFGR &= ~RCC_CFGR_SW_Msk;
}

static void RCC_ConfigAHB_Prescaler(uint8_t ahbPrescaler)
{
	if(ahbPrescaler < RCC_AHB_DIV_002)
		RCC->CFGR &= ~RCC_CFGR_HPRE_Msk;//No prescaler is applied.
	else
		RCC->CFGR |= (ahbPrescaler << RCC_CFGR_HPRE_Pos);//Prescaler will be applied.
}

static void RCC_ConfigAPB_LSpPrescaler(uint8_t apbLPrescaler)
{
	if(apbLPrescaler < RCC_AHB_DIV_02)
		RCC->CFGR &= ~RCC_CFGR_PPRE1_Msk;//No prescaler is applied.
	else
		RCC->CFGR |= (apbLPrescaler << RCC_CFGR_PPRE1_Pos);//Prescaler will be applied.
}

static void RCC_ConfigAPB_HSpPrescaler(uint8_t apbHPrescaler)
{
	if(apbHPrescaler < RCC_AHB_DIV_02)
		RCC->CFGR &= ~RCC_CFGR_PPRE2_Msk;//No prescaler is applied.
	else
		RCC->CFGR |= (apbHPrescaler << RCC_CFGR_PPRE2_Pos);//Prescaler will be applied.
}

static void RCC_ConfigHSE_DivRTC(uint8_t hseDiv)
{
	/*Caution: The software has to set these bits correctly to ensure that the clock supplied to the
	RTC is 1 MHz. These bits must be configured if needed before selecting the RTC
	clock source.*/
	if(hseDiv == RCC_HSE_DIV_00 || hseDiv == RCC_HSE_DIV_01)
		RCC->CFGR &= ~RCC_CFGR_RTCPRE;
	else
		RCC->CFGR |= (hseDiv << RCC_CFGR_RTCPRE_Pos);
}

static void RCC_ConfigMC_ClkOutput1(uint8_t mcClkOut)
{
	if(mcClkOut == RCC_MCO1_HSI_OUT)
		RCC->CFGR &= ~RCC_CFGR_MCO1;//HSI clock selected.
	else
		RCC->CFGR |= (mcClkOut << RCC_CFGR_MCO1_Pos);//Either LSE, HSE, or PLL.
}

static void RCC_ConfigMC_ClkOutput2(uint8_t mcClkOut)
{
	if(mcClkOut == RCC_MCO1_HSI_OUT)
		RCC->CFGR &= ~RCC_CFGR_MCO2;//System clock selected.
	else
		RCC->CFGR |= (mcClkOut << RCC_CFGR_MCO2_Pos);//Either PLLI2S, HSE, or PLL.
}

static void RCC_ConfigI2S_ClkSelection(uint8_t i2sClkSel)
{
	if(i2sClkSel == RCC_PLLI2S_CLK)
		RCC->CFGR &= ~RCC_CFGR_I2SSRC;
	else
		RCC->CFGR |= RCC_CFGR_I2SSRC;
}

static void RCC_ConfigMC1_Prescaler(uint8_t mcPrescaler)
{
	if(mcPrescaler < RCC_DIV_2)
		RCC->CFGR &= ~RCC_CFGR_MCO1PRE;//No prescaler is applied.
	else
		RCC->CFGR |= (mcPrescaler << RCC_CFGR_MCO1PRE_Pos);//Prescaler will be applied.
}

static void RCC_ConfigMC2_Prescaler(uint8_t mcPrescaler)
{
	if(mcPrescaler < RCC_DIV_2)
		RCC->CFGR &= ~RCC_CFGR_MCO2PRE;//No prescaler is applied.
	else
		RCC->CFGR |= (mcPrescaler << RCC_CFGR_MCO2PRE_Pos);//Prescaler will be applied.
}

/*
 * @fn			- RCC_ConfigPLLM
 *
 * @brief		- This function configures the PLL_M bit in the
 * 				  PLLCFGR register according to the application
 * 				  provided setting in the PLL Configuration
 * 				  Structure.
 *
 * @param[uint8_t]				- Value to set the PLL_M bit to.
 *
 * @return		- None.
 *
 * @note		- Private helper function.
 * 				  If the pllM parameter is not between 2(b0 0000 0010)
 * 				  and 63(0b 0011 1111) inclusive, then nothing happens.
 */
static void RCC_ConfigPLLM(uint8_t pllmValue)
{
	if((pllmValue >= 2) && (pllmValue <= 63))
	{
		RCC->PLLCFGR &= ~(63 << RCC_PLLCFGR_PLLM_Pos);
		RCC->PLLCFGR |= (pllmValue << RCC_PLLCFGR_PLLM_Pos);
	}
}

/*
 * @fn			- RCC_ConfigPLLN
 *
 * @brief		- This function configures the PLL_N bit in the
 * 				  PLLCFGR register according to the application
 * 				  provided setting in the PLL Configuration
 * 				  Structure.
 *
 * @param[uint8_t]				- Value to set the PLL_N bit to.
 *
 * @return		- None.
 *
 * @note		- Private helper function.
 * 				  If the pllN parameter is not between
 * 				  50(b0 0000 0000 0011 0010) and 511(b0 0000 0001 1111 1111)
 * 				  inclusive, then nothing happens.
 */
static void RCC_ConfigPLLN(uint16_t pllnValue)
{
	if((pllnValue >= 50) && (pllnValue <= 432))
	{
		RCC->PLLCFGR &= ~(511 << RCC_PLLCFGR_PLLN_Pos);
		RCC->PLLCFGR |= (pllnValue << RCC_PLLCFGR_PLLN_Pos);
	}
}

/*
 * @fn			- RCC_ConfigPLLP
 *
 * @brief		- This function configures the PLL_P bit in the
 * 				  PLLCFGR register according to the application
 * 				  provided setting in the PLL Configuration
 * 				  Structure.
 *
 * @param[uint8_t]				- Value to set the PLL_P bit to.
 *
 * @return		- None.
 *
 * @note		- Private helper function.
 */
static void RCC_ConfigPLLP(uint8_t pllpValue)
{
	if(pllpValue == PLL_P_DIV_2)
		RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLP_Msk;
	else
	{
		RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLP_Msk;
		RCC->PLLCFGR |= (pllpValue << RCC_PLLCFGR_PLLP_Pos);
	}
}

/*
 * @fn			- RCC_ConfigPLLQ
 *
 * @brief		- This function configures the PLL_Q bit in the
 * 				  PLLCFGR register according to the application
 * 				  provided setting in the PLL Configuration
 * 				  Structure.
 *
 * @param[uint8_t]				- Value to set the PLL_Q bit to.
 *
 * @return		- None.
 *
 * @note		- Private helper function.
 * 				  If the pllQ parameter is not between 2(b0 0000 0010)
 * 				  and 15(b0 0000 1111) inclusive, then nothing will
 * 				  happen.
 */
static void RCC_ConfigPLLQ(uint8_t pllqValue)
{
	if((pllqValue >= 2) && (pllqValue <= 15))
	{
		RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLQ_Msk;
		RCC->PLLCFGR |= (pllqValue << RCC_PLLCFGR_PLLQ_Pos);
	}
}

/*
 * @fn			- RCC_ConfigPLLSRC
 *
 * @brief		- This function configures the PLL_SRC bit in the
 * 				  PLLCFGR register according to the application
 * 				  provided setting in the PLL Configuration
 * 				  Structure.
 *
 * @param[uint8_t]				- Value to set the PLL_SCR bit to.
 *
 * @return		- None.
 *
 * @note		- Private helper function.
 */
static void RCC_ConfigPLLSRC(uint8_t pllSrc)
{
	if(pllSrc != PLL_SRC_HSE)
		RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLSRC;//HSI will be clock source.
	else
		RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC;//HSE will be clock source.
}
