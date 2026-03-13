#include"spi.h"
#include"irq.h"


/***************** Private Helper Function Headers *************************************/

static void configClkDataTransfer(SPI_TypeDef *SPIx, uint8_t clkPolPha);
static void configDuplexRxTx(SPI_TypeDef *SPIx, uint8_t dirConfig);
static void configSlaveMgnt(SPI_TypeDef *SPIx, uint8_t slaveMgmt);

/***************************************************************************************/


/***************** Global Static Variables *********************************************/

//static uint8_t state;
static SPI_Config_t config;

/***************************************************************************************/


/***************** User Application Exposed Function Definitions ***********************/

/*
 * @fn			- SPI_PeriClockControl
 *
 * @brief		- This function enables or disables peripheral clock for the
 * 				  given SPI register.
 *
 * @param[SPI_TypeDef]		- Base address of the SPI register.
 * @param[uint8_t]			- ENABLE or DISABLE macros.
 *
 * @return		- None.
 *
 * @note		- None.
 */
void SPI_PeriClockControl(SPI_TypeDef *SPIx, uint8_t EnOrDi)
{
	if(EnOrDi == ENABLE)
	{
		if(SPIx == SPI1)
			RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
		else if(SPIx == SPI2)
			RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
		else if(SPIx == SPI3)
			RCC->APB1ENR |= RCC_APB1ENR_SPI3EN;
	}
	else
	{
		if(SPIx == SPI1)
			RCC->APB2ENR &= ~RCC_APB2ENR_SPI1EN;
		else if(SPIx == SPI2)
			RCC->APB1ENR &= ~RCC_APB1ENR_SPI2EN;
		else if(SPIx == SPI3)
			RCC->APB1ENR &= ~RCC_APB1ENR_SPI3EN;
	}
}

/*
 * @fn			- SPI_InitMaster
 *
 * @brief		- This function initializes the SPI peripheral in Master Mode
 * 				  	with user-provided configurations.
 * 				  Each pin that is to be used for SPI communication (except NSS if
 * 				    using Software Slave Management) must be configured to alternate
 * 				    functionality mode first before calling this function.
 * 				  The Peripheral-Clock division is used to determine the baudrate
 * 				  	for the SPI peripheral.
 *
 * @param[SPI_TypeDef]	- SPI base address.
 * @param[SPI_Config_t]	- SPI configuration structure.
 *
 * @return		- None.
 *
 * @note		- It is recommended to always enable and use interrupts when
 * 				  	using the SPI peripheral to reduce CPU load with polling.
 * 				  If using Software Slave Management, the NSS must be set to HIGH.
 * 				  In this configuration the MOSI pin is a data output and the MISO pin is a data input.
 */
void SPI_InitMaster(SPI_TypeDef *SPIx, SPI_Config_t SPI_config)
{
	//Enable the clock for the peripheral and set configurations.
	SPI_PeriClockControl(SPIx, ENABLE);
	SPI_SetConfigurations(SPI_config);
	//Set Baud Rate.
	SPIx->CR1 |= (config.SPI_baudRateDiv << SPI_CR1_BR_Pos);
	//Select the CPOL and CPHA bits to define one of the four relationships between the data transfer and the serial clock.
	configClkDataTransfer(SPIx, config.SPI_clkRelationship);
	//Set the DFF bit to define 8- or 16-bit data frame format.
	SPIx->CR1 |= (config.SPI_dataBitMode << SPI_CR1_DFF_Pos);
	//Define the frame format (8bit or 16bit).
	SPIx->CR1 |= (config.SPI_frameFormat << SPI_CR1_LSBFIRST_Pos);
	//Configure slave management.
	configSlaveMgnt(SPIx, config.SPI_SlaveMgmt);
	//Set the FRF bit in SPI_CR2 to select the TI or Motorola protocol for serial communications.
	SPIx->CR2 |= (config.SPI_frameFormat << SPI_CR2_FRF_Pos);
	//Configure the directional data mode.
	configDuplexRxTx(SPIx, config.SPI_bidirectionalMode);
	//The MSTR and SPE bits must be set (they remain set only if the NSS pin is connected to a high-level signal).
	SPIx->CR1 |= SPI_CR1_MSTR;
}

/*
 * @fn			- SPI_MasterTransmissionStartTx
 *
 * @brief		- This function begins the data transmission process for
 * 				  master mode SPI communication. Only transmits data to
 * 				  the slave device.
 *
 * @param[SPI_TypeDef]	- SPI base address.
 * @param[uint8_t*]		- Data to transmit.
 *
 * @return		- None.
 *
 * @note		- Bidirectional Transmit-Only Mode.
 */
void SPI_MasterTransmissionStartTx(SPI_TypeDef *SPIx, uint8_t *dataTx)
{
	if(config.SPI_interrupts == SPI_INTERRUPT_ENABLED)	//Data Transmission with Interrupts Enabled.
	{
		return;
	}

	//Data Transmission without Interrupts Enabled.
	uint8_t* p = dataTx;
	//Enable the SPI by setting the SPE bit to 1.
	SPIx->CR1 |= SPI_CR1_SPE;
	//Write the first data item to send into the SPI_DR register (this clears the TXE bit).
	do{ //Wait until TXE=1 and write the next data item to be transmitted.
		while(!((SPIx->SR >> SPI_SR_TXE_Pos) & 0x1U)){}
		SPIx->DR = *p;
		p++;  //Repeat this step for each data item to be transmitted.
	}while(*p != '\0');
	//After writing the last data item into the SPI_DR register, wait until TXE=1,
	//then wait until BSY=0, this indicates that the transmission of the last data is complete.
	while(!((SPIx->SR >> SPI_SR_TXE_Pos) & 0x1U)){}
	while(((SPIx->SR >> SPI_SR_BSY_Pos) & 0x1U)){}
	SPIx->CR1 &= ~SPI_CR1_SPE;
}

/*
 * @fn			- SPI_MasterTransmissionStartRx
 *
 * @brief		- This function begins the data transmission process for
 * 				  master mode SPI communication. Only receives data from
 * 				  the slave device.
 *
 * @param[SPI_TypeDef]	- SPI base address.
 * @param[uint8_t*]		- Data to be received.
 *
 * @return		- None.
 *
 * @note		- Bidirectional Receive-Only Mode.
 */
void SPI_MasterTransmissionStartRx(SPI_TypeDef *SPIx, uint8_t *dataRx)
{

}

/*
 * @fn			- SPI_MasterTransmissionStartTxRx
 *
 * @brief		- This function begins the data transmission process for
 * 				  master mode SPI communication. Transmits and receives
 * 				  data to and from the slave device.
 *
 * @param[SPI_TypeDef]	- SPI base address.
 * @param[uint8_t*]		- Data to transmit.
 * @param[uint8_t*]		- Data to be received.
 *
 * @return		- None.
 *
 * @note		- Unidirectional Transmit and Receive Mode (Full Duplex).
 */
void SPI_MasterTransmissionStartTxRx(SPI_TypeDef *SPIx, uint8_t *dataTx, uint8_t *dataRx)
{
	//Define local variables.
	uint8_t* p = dataTx;
	uint8_t dataRxInc = 0;
	//Enable the SPI by setting the SPE bit to 1.
	SPIx->CR1 |= SPI_CR1_SPE;
	//Write the first data item to be transmitted into the SPI_DR register (this clears the TXE flag).
	//SPIx->DR = *p;
	//p++;
	SPIx->DR = 0x0;
	/*
	 Wait until TXE=1 and write the second data item to be transmitted. Then wait until
	 RXNE=1 and read the SPI_DR to get the first received data item (this clears the RXNE
	 bit). Repeat this operation for each data item to be transmitted/received until the n–1
	 received data.
	 */
	do{
	while(SPI_TX_EMPTY){}
	if(*p == '\0')	//If there's no more data to send, but still receiving data, write a dummy 0 to send.
		SPIx->DR = 0x0;
	else
	{
		SPIx->DR = *p;
		p++;
	}

	while(SPI_RX_NOT_EMPTY){}
	*dataRx = SPIx->DR;
	dataRx++;
	dataRxInc++;
	}while(dataRxInc <= 11);

	//Wait until RXNE=1 and read the last received data.
	while(SPI_RX_NOT_EMPTY){}
	*dataRx = SPIx->DR;
	//Wait until TXE=1 and then wait until BSY=0 before disabling the SPI.
	while(SPI_TX_EMPTY){}
	while(SPI_BUSY_FLAG){}
	SPIx->CR1 &= ~SPI_CR1_SPE;
	dataRx -= dataRxInc;
}

/*
 * @fn			- SPI_InitSlave
 *
 * @brief		-
 *
 * @param[SPI_TypeDef]	- SPI base address.
 *
 * @return		- None.
 *
 * @note		- None.
 */
void SPI_InitSlave(SPI_TypeDef *SPIx, SPI_Config_t SPI_config)
{

}

/*
 * @fn			- SPI_SlaveTransmissionStartTx
 *
 * @brief		- This function begins the data transmission process for
 * 				  master mode SPI communication. Only transmits data to
 * 				  the slave device.
 *
 * @param[SPI_TypeDef]	- SPI base address.
 * @param[uint8_t*]		- Data to transmit.
 *
 * @return		- None.
 *
 * @note		- Bidirectional Transmit-Only Mode.
 */
void SPI_SlaveTransmissionStartTx(SPI_TypeDef *SPIx, uint8_t *dataTx)
{

}

/*
 * @fn			- SPI_SlaveTransmissionStartRx
 *
 * @brief		- This function begins the data transmission process for
 * 				  master mode SPI communication. Only receives data from
 * 				  the slave device.
 *
 * @param[SPI_TypeDef]	- SPI base address.
 * @param[uint8_t*]		- Data to be received.
 *
 * @return		- None.
 *
 * @note		- Bidirectional Receive-Only Mode.
 */
void SPI_SlaveTransmissionStartRx(SPI_TypeDef *SPIx, uint8_t *dataRx)
{

}

/*
 * @fn			- SPI_SlaveTransmissionStartTxRx
 *
 * @brief		- This function begins the data transmission process for
 * 				  master mode SPI communication. Transmits and receives
 * 				  data to and from the slave device.
 *
 * @param[SPI_TypeDef]	- SPI base address.
 * @param[uint8_t*]		- Data to transmit.
 * @param[uint8_t*]		- Data to be received.
 *
 * @return		- None.
 *
 * @note		- Unidirectional Transmit and Receive Mode (Full Duplex).
 */
void SPI_SlaveTransmissionStartTxRx(SPI_TypeDef *SPIx, uint8_t *dataTx, uint8_t *dataRx)
{

}

/*
 * @fn			- SPI_DisableSlave
 *
 * @brief		- This function disables the SPI peripheral.
 *
 * @param[SPI_TypeDef]	- SPI base address.
 *
 * @return		- None.
 *
 * @note		- None.
 */
void SPI_DisableSlave(SPI_TypeDef *SPIx, SPI_Config_t SPI_config)
{

}

/*
 * @fn			- SPI_SetConfigurations
 *
 * @brief		- This function disables the SPI peripheral.
 *
 * @param[SPI_TypeDef]	- SPI base address.
 *
 * @return		- None.
 *
 * @note		- None.
 */
void SPI_SetConfigurations(SPI_Config_t SPI_config)
{
	config.SPI_SlaveMgmt         = SPI_config.SPI_SlaveMgmt;
	config.SPI_baudRateDiv       = SPI_config.SPI_baudRateDiv;
	config.SPI_bidirectionalMode = SPI_config.SPI_bidirectionalMode;
	config.SPI_bitOrder          = SPI_config.SPI_bitOrder;
	config.SPI_clkRelationship   = SPI_config.SPI_clkRelationship;
	config.SPI_dataBitMode       = SPI_config.SPI_frameFormat;
	config.SPI_frameFormat       = SPI_config.SPI_frameFormat;
}

/*
 * @fn			- SPI_NSS_Enable
 *
 * @brief		- This function enables the slave device by setting the NSS
 * 				  pin to LOW.
 *
 * @param[GPIO_TypeDef]	- GPIOx base address.
 * @param[uint8_t]		- GPIO pin number.
 *
 * @return		- None.
 *
 * @note		- None.
 */
void SPI_NSS_Enable(GPIO_TypeDef *GPIOx, uint8_t gpioPin)
{
	GPIOx->ODR &= ~(1U << gpioPin);
}

/*
 * @fn			- SPI_NSS_Disable
 *
 * @brief		- This function disables the slave device by setting the NSS
 * 				  pin to HIGH.
 *
 * @param[GPIO_TypeDef]	- GPIOx base address.
 * @param[uint8_t]		- GPIO pin number.
 *
 * @return		- None.
 *
 * @note		- None.
 */
void SPI_NSS_Disable(GPIO_TypeDef *GPIOx, uint8_t gpioPin)
{
	GPIOx->ODR |= (1U << gpioPin);
}

/*
 * @fn			- SPI_EnableInterrupts
 *
 * @brief		- This function enables the SPI interrupts.
 *
 * @param[SPI_TypeDef]	- SPIx base address.
 *
 * @return		- None.
 *
 * @note		- None.
 */
void SPI_EnableInterrupts(SPI_TypeDef *SPIx)
{
	config.SPI_interrupts = SPI_INTERRUPT_ENABLED;

	SPIx->CR2 |= SPI_CR2_TXEIE;
	SPIx->CR2 |= SPI_CR2_RXNEIE;
	SPIx->CR2 |= SPI_CR2_ERRIE;

	if(SPIx == SPI1)
		IRQInterruptConfig(SPI1_IRQn, ENABLE);
	else if(SPIx == SPI2)
		IRQInterruptConfig(SPI1_IRQn, ENABLE);
	else if(SPIx == SPI3)
		IRQInterruptConfig(SPI1_IRQn, ENABLE);
}

/*
 * @fn			- SPI_EnableDMA
 *
 * @brief		- This function enables the SPI DMA.
 *
 * @param[SPI_TypeDef]	- SPIx base address.
 *
 * @return		- None.
 *
 * @note		- None.
 */
void SPI_EnableDMA(SPI_TypeDef *SPIx)
{
	SPIx->CR2 |= SPI_CR2_TXDMAEN;
	SPIx->CR2 |= SPI_CR2_RXDMAEN;
}


/***************************************************************************************/


/***************** SPI IRQ Handling ***************************************************/

/* Weak function that can be implemented in user application. */
void __attribute__((weak)) SPI_ApplicationEventCallback(void){}

/*
 * @fn			- SPI_IRQHandling
 *
 * @brief		- This function handles an SPI interrupt.
 *
 * @param[SPI_TypeDef]	- Base Address of the SPI Peripheral.
 *
 * @return		- None.
 *
 * @note		- The interrupt bits(TXEIE, RXNEIE, or ERRIE) must
 * 				  be set in order for an interrupt to be triggered.
 */
void SPI_IRQHandling(SPI_TypeDef *SPIx)
{
	SPI_ApplicationEventCallback();
}




/***************************************************************************************/


/***************** Private Helper Function Definitions *********************************/

static void configClkDataTransfer(SPI_TypeDef *SPIx, uint8_t clkPolPha)
{
	if(clkPolPha == SPI_CLOCK_POL_PHA_0)
	{
		SPIx->CR1 &= ~SPI_CR1_CPOL;
		SPIx->CR1 &= ~SPI_CR1_CPHA;
	}
	else if(clkPolPha == SPI_CLOCK_POL_PHA_1)
	{
		SPIx->CR1 |= SPI_CR1_CPOL;
		SPIx->CR1 &= ~SPI_CR1_CPHA;
	}
	else if(clkPolPha == SPI_CLOCK_POL_PHA_1)
	{
		SPIx->CR1 &= ~SPI_CR1_CPOL;
		SPIx->CR1 |= SPI_CR1_CPHA;
	}
	else if(clkPolPha == SPI_CLOCK_POL_PHA_1)
	{
		SPIx->CR1 &= ~SPI_CR1_CPOL;
		SPIx->CR1 &= ~SPI_CR1_CPHA;
	}
}

static void configDuplexRxTx(SPI_TypeDef *SPIx, uint8_t dirConfig)
{
	if(dirConfig == SPI_2LINE_UNIDIRECTIONAL_RX_ONLY_MODE)
	{	//Unidirectional Receive-Only Mode.
		SPIx->CR1 &= ~SPI_CR1_BIDIMODE;
		SPIx->CR1 |= SPI_CR1_RXONLY;
	}
	else if(dirConfig == SPI_2LINE_UNIDIRECTIONAL_RX_TX_MODE)
	{	//Full Duplex Mode.
		SPIx->CR1 &= ~SPI_CR1_BIDIMODE;
		SPIx->CR1 &= ~SPI_CR1_RXONLY;
	}
	else if(dirConfig == SPI_1LINE_BIDRECTIONAL_RX_ONLY_MODE)
	{	//Bidirectional Receive-Only Mode.
		SPIx->CR1 |= SPI_CR1_BIDIMODE;
		SPIx->CR1 &= ~SPI_CR1_BIDIOE;
	}
	else if(dirConfig == SPI_1LINE_BIDRECTIONAL_TX_ONLY_MODE)
	{	//Bidirectional Transmit-Only Mode.
		SPIx->CR1 |= SPI_CR1_BIDIMODE;
		SPIx->CR1 |= SPI_CR1_BIDIOE;
	}
}

static void configSlaveMgnt(SPI_TypeDef *SPIx, uint8_t slaveMgmt)
{
	if(slaveMgmt == SPI_HWS_MGNT_NSS_OP_ENABLED)
	{
		SPIx->CR1 &= ~SPI_CR1_SSM;
		SPIx->CR2 |= SPI_CR2_SSOE;
	}
	else if(slaveMgmt == SPI_HWS_MGNT_NSS_OP_DISABLED)
	{
		SPIx->CR1 &= ~SPI_CR1_SSM;
		SPIx->CR2 &= ~SPI_CR2_SSOE;
	}
	else if(slaveMgmt == SPI_SWS_MGNT)
	{
		SPIx->CR1 |= SPI_CR1_SSM;
		SPIx->CR1 |= SPI_CR1_SSI;
	}
}


/***************************************************************************************/
