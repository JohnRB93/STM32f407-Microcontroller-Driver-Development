#include"dma.h"



/***************** Private Helper Function Headers *************************************/

static void DMA_ConfigPeripheralAddress(DMA_RegDef_t *pDMAx, uint8_t reqStream, uint32_t periAddress);
static void DMA_ConfigMemoryAddresses(DMA_Handle_t *DMA_Handle, uint8_t reqStream, uint32_t memAddress);
static void DMA_ConfigItemsToTransfer(DMA_RegDef_t *pDMAx, uint8_t reqStream, uint8_t itemsToTransfer);
static void DMA_ConfigChannel(DMA_RegDef_t *pDMAx, uint8_t reqStream, uint8_t channel);
static void DMA_ConfigStreamPriority(DMA_RegDef_t *pDMAx, uint8_t reqStream, uint8_t priority);
static void DMA_ConfigFIFO(DMA_Handle_t *DMA_Handle, uint8_t reqStream, uint8_t EnOrDi);
static void DMA_ConfigTransDirection(DMA_RegDef_t *pDMAx, uint8_t reqStream, uint8_t dir);
static void DMA_ConfigPtrIncrement(DMA_RegDef_t *pDMAx, uint8_t reqStream, uint8_t incMode);
static void DMA_ConfigBurstTransType(DMA_RegDef_t *pDMAx, uint8_t reqStream, uint8_t memType, uint8_t periType);
static void DMA_ConfigDataWidths(DMA_RegDef_t *pDMAx, uint8_t reqStream, uint8_t msize, uint8_t psize);
static void DMA_ConfigCircularMode(DMA_RegDef_t *pDMAx, uint8_t reqStream, uint8_t EnOrDi);
static void DMA_ConfigDoubleBuffMode(DMA_RegDef_t *pDMAx, uint8_t reqStream, uint8_t EnOrDi);

/*** Private Helper Function Headers for IRQ Handling ***/
static void DMA_HandleHalfTransCmptIt(DMA_Handle_t *DMA_Handle, uint8_t reqStream);
static void DMA_HandleTransCmptIt(DMA_Handle_t *DMA_Handle, uint8_t reqStream);
static void DMA_HandleTransErrIt(DMA_Handle_t *DMA_Handle, uint8_t reqStream);
static void DMA_HandleFIFOErrIt(DMA_Handle_t *DMA_Handle, uint8_t reqStream);
static void DMA_HandleDirectErrIt(DMA_Handle_t *DMA_Handle, uint8_t reqStream);

/***************************************************************************************/


/***************** User Application Exposed Function Definitions ***********************/

/*
 * @fn			- DMA_Init
 *
 * @brief		- This function initializes the DMA peripheral with user-
 * 				  provided configurations.
 *
 * @param[DMA_TypeDef*]	- Base address of the DMA peripheral.
 *
 * @return		- None.
 *
 * @note		- None.
 */
void DMA_Init(DMA_TypeDef *pDMAx)
{
	DMA_PeriClockControl(pDMAx, ENABLE);
}

/*
 * @fn			- DMA_DeInit
 *
 * @brief		- This function de-initializes the DMA peripheral.
 *
 * @param[DMA_TypeDef*]	- Base address of the DMA peripheral.
 *
 * @return		- None.
 *
 * @note		- None.
 */
void DMA_DeInit(DMA_TypeDef *pDMAx)
{
	if(pDMAx == DMA1)
	{
		RCC->AHB1RSTR |= (1 << RCC_AHB1RSTR_DMA1RST);
		RCC->AHB1RSTR &= ~(1 << RCC_AHB1RSTR_DMA1RST);
	}else
	{
		RCC->AHB1RSTR |= (1 << RCC_AHB1RSTR_DMA2RST);
		RCC->AHB1RSTR &= ~(1 << RCC_AHB1RSTR_DMA2RST);
	}
}

/*
 * @fn			- DMA_PeriClockControl
 *
 * @brief		- This function enables or disables peripheral clock for the
 * 				  given ADC register.
 *
 * @param[DMA_TypeDef*]	- Base address of the DMA peripheral.
 * @param[uint8_t]			- ENABLE or DISABLE macros.
 *
 * @return		- None.
 *
 * @note		- None.
 */
void DMA_PeriClockControl(DMA_TypeDef *pDMAx, uint8_t EnOrDi)
{
	if(EnOrDi == ENABLE)
	{
		if(pDMAx == DMA1)  //Enable clock for DMA1
			RCC->AHB1ENR |= (1 << RCC_AHB1ENR_DMA1EN);
		else if(pDMAx == DMA2)  //Enable clock for DMA2
			RCC->AHB1ENR |= (1 << RCC_AHB1ENR_DMA2EN);
	}
	else
	{
		if(pDMAx == DMA1)  //Disable clock for DMA1
			RCC->AHB1ENR &= ~(1 << RCC_AHB1ENR_DMA1EN);
		else if(pDMAx == DMA2)  //Disable clock for DMA2
			RCC->AHB1ENR &= ~(1 << RCC_AHB1ENR_DMA2EN);
	}
}

/*
 * @fn			- DMA_ConfigStream
 *
 * @brief		- This function configures the selected DMA Stream with configuration
 * 				  settings provided by the user application in the DMA_Config structure.
 *
 * @param[ADC_Handle_t*]	- Base address of the DMA handle.
 * @param[uint8_t]			- Request Stream Number. (REQ_STREAM_0, ... REQ_STREAM_7)
 * @param[uint32_t]			- Peripheral Address.
 * @param[uint32_t]			- Memory Address.
 * @param[uint8_t]			- Request Stream Channel.(REQ_STR_CH_0, ... REQ_STR_CH_7)
 *
 * @return		- None.
 *
 * @note		- None
 */
void DMA_ConfigStream(DMA_Handle_t *DMA_Handle, uint8_t reqStream, uint32_t periAddress,
		uint32_t memAddress, uint8_t channel)
{
	DMA_ClearEN_Bit(DMA_Handle->pDMAx, reqStream);
	DMA_ConfigPeripheralAddress(DMA_Handle->pDMAx, reqStream, periAddress);
	DMA_ConfigMemoryAddresses(DMA_Handle, reqStream, memAddress);
	DMA_ConfigItemsToTransfer(DMA_Handle->pDMAx, reqStream, DMA_Handle->DMA_Config.DMA_SxNDTR);
	DMA_ConfigChannel(DMA_Handle->pDMAx, reqStream, channel);
	DMA_ConfigStreamPriority(DMA_Handle->pDMAx, reqStream, DMA_Handle->DMA_Config.DMA_ArbPriority);
	DMA_ConfigFIFO(DMA_Handle, reqStream, DMA_Handle->DMA_Config.DMA_FIFO_Mode);
	DMA_ConfigTransDirection(DMA_Handle->pDMAx, reqStream, DMA_Handle->DMA_Config.DMA_Direction);
	DMA_ConfigPtrIncrement(DMA_Handle->pDMAx, reqStream, DMA_Handle->DMA_Config.DMA_PtrInc);
	DMA_ConfigBurstTransType(DMA_Handle->pDMAx, reqStream,
			DMA_Handle->DMA_Config.DMA_MemBurstTransfer, DMA_Handle->DMA_Config.DMA_PeriBurstTransfer);
	DMA_ConfigDataWidths(DMA_Handle->pDMAx, reqStream,
			DMA_Handle->DMA_Config.DMA_SourceDataWidth, DMA_Handle->DMA_Config.DMA_DestinationDataWidth);
	DMA_ConfigCircularMode(DMA_Handle->pDMAx, reqStream, DMA_Handle->DMA_Config.DMA_CircularMode);

	if(DMA_Handle->DMA_Config.DMA_TransactionType == DMA_DOUBLE_BUFFER_TRANSACTION)
		DMA_ConfigDoubleBuffMode(DMA_Handle->pDMAx, reqStream, ENABLE);
	else//Regular Type Transatcion is used.
		DMA_ConfigDoubleBuffMode(DMA_Handle->pDMAx, reqStream, DISABLE);
}

/*
 * @fn			- DMA_findMburstBeatPsizeMsize
 *
 * @brief		- This function finds the values for msize, psize,
 * 				  and MburstBeat. The values are assigned to the
 * 				  accordingly provided address parameters.
 *
 * @param[ADC_Handle_t*]	- Base address of the DMA handle.
 * @param[uint8_t]			- Request Stream Number. (REQ_STREAM_0, ... REQ_STREAM_7)
 * @param[uint8_t*]			- Address of the Memory Burst Beat value.
 * @param[uint8_t*]			- Address of the Peripheral Data Size value.
 * @param[uint8_t*]			- Address of the Memory Data Size value.
 *
 * @return		- None.
 *
 * @note		- If Circular Mode is used, the user-application must configure
 * 				  a multiple of dma_sxndtr from the DMA_Config struct. NDTR must also
 * 				  be a multiple of the Peripheral burst size multiplied by the
 * 				  peripheral data size, otherwise this could result in a bad DMA behavior.
 * 				  If Double Buffer Mode is used, then Circular Mode is enabled by default.
 * 				  Peripheral as flow controller not implemented.
 */
void DMA_findMburstBeatPsizeMsize(DMA_RegDef_t *pDMAx, uint8_t reqStream,
		uint8_t *MburstBeat, uint8_t *psize, uint8_t *msize)
{	//Find MburstBeat
	if(((pDMAx->DMA_Sx[reqStream].SxCR >> DMA_SXCR_MBURST) & 0x3) == DMA_INCR4)
		*MburstBeat = 4;
	else if(((pDMAx->DMA_Sx[reqStream].SxCR >> DMA_SXCR_MBURST) & 0x3) == DMA_INCR8)
		*MburstBeat = 8;
	else if(((pDMAx->DMA_Sx[reqStream].SxCR >> DMA_SXCR_MBURST) & 0x3) == DMA_INCR16)
		*MburstBeat = 16;
	else
		*MburstBeat = 1;
	//Find psize
	if(((pDMAx->DMA_Sx[reqStream].SxCR >> DMA_SXCR_PSIZE) & 0x3) == DMA_HALF_WORD)
		*psize = 16;
	else if(((pDMAx->DMA_Sx[reqStream].SxCR >> DMA_SXCR_PSIZE) & 0x3) == DMA_WORD)
		*psize = 32;
	else
		*psize = 8;
	//Find msize
	if(((pDMAx->DMA_Sx[reqStream].SxCR >> DMA_SXCR_MSIZE) & 0x3) == DMA_HALF_WORD)
		*msize = 16;
	else if(((pDMAx->DMA_Sx[reqStream].SxCR >> DMA_SXCR_MSIZE) & 0x3) == DMA_WORD)
		*msize = 32;
	else
		*psize = 8;
}

/*
 * @fn			- DMA_IncrPeriPtr
 *
 * @brief		- This function increments the peripheral register address
 * 				  according to the PSIZE bit in the SxCR register.
 *
 * @param[DMA_RegDef_t*]	- Base address of the DMA Register.
 * @param[uint8_t]			- Request Stream Number. (REQ_STREAM_0, ... REQ_STREAM_7)
 * @param[uint32_t*]		- Pointer to the peripheral register address.
 *
 * @return		- None.
 *
 * @note		- This function is useful when the peripheral pointer increment
 * 				  mode is disabled.
 */
void DMA_IncrPeriPtr(DMA_RegDef_t *pDMAx, uint8_t reqStream, uint32_t* periAddr)
{
	if(((pDMAx->DMA_Sx[reqStream].SxCR >> DMA_SXCR_PSIZE) & 0x3) == DMA_BYTE)
		periAddr++;
	else if(((pDMAx->DMA_Sx[reqStream].SxCR >> DMA_SXCR_PSIZE) & 0x3) == DMA_HALF_WORD)
		periAddr += 2;
	else
		periAddr += 4;
}

/*
 * @fn			- DMA_IncrMemPtr
 *
 * @brief		- This function increments the memory register address
 * 				  according to the MSIZE bit in the SxCR register.
 *
 * @param[DMA_RegDef_t*]	- Base address of the DMA Register.
 * @param[uint8_t]			- Request Stream Number. (REQ_STREAM_0, ... REQ_STREAM_7)
 * @param[uint32_t*]		- Pointer to the memory register address.
 *
 * @return		- None.
 *
 * @note		- This function is useful when the memory pointer increment
 * 				  mode is disabled.
 */
void DMA_IncrMemPtr(DMA_RegDef_t *pDMAx, uint8_t reqStream, uint32_t* memAddr)
{
	if(((pDMAx->DMA_Sx[reqStream].SxCR >> DMA_SXCR_MSIZE) & 0x3) == DMA_BYTE)
		memAddr++;
	else if(((pDMAx->DMA_Sx[reqStream].SxCR >> DMA_SXCR_MSIZE) & 0x3) == DMA_HALF_WORD)
		memAddr += 2;
	else
		memAddr += 4;
}

/*
 * @fn			- DMA_ActivateStream
 *
 * @brief		- This function enables the EN bit in the SxCR register,
 * 				  activating the stream for the stream number x.
 *
 * @param[DMA_RegDef_t*]	- Base address of the DMA Register.
 * @param[uint8_t]			- Request Stream Number. (REQ_STREAM_0, ... REQ_STREAM_7)
 *
 * @return		- None.
 *
 * @note		- Only call this function after configuring the stream.
 */
void DMA_ActivateStream(DMA_RegDef_t *pDMAx, uint8_t reqStream)
{
	pDMAx->DMA_Sx[reqStream].SxCR |= (1 << DMA_SXCR_EN);
}

/*
 * @fn			- DMA_ClearEN_Bit
 *
 * @brief		- This function resets the EN bit in the SxCR register,
 * 				  deactivating the stream for the stream number x.
 *
 * @param[DMA_RegDef_t*]	- Base address of the DMA Register.
 * @param[uint8_t]			- Request Stream Number. (REQ_STREAM_0, ... REQ_STREAM_7)
 *
 * @return		- None.
 *
 * @note		- None.
 */
void DMA_ClearEN_Bit(DMA_RegDef_t *pDMAx, uint8_t reqStream)
{
	if((pDMAx->DMA_Sx[reqStream].SxCR & (1 << DMA_SXCR_EN)) == SET)
	{//If the EN bit is set, then it must be cleared and wait until all
	 //transfers have finished.
		pDMAx->DMA_Sx[reqStream].SxCR &= ~(1 << DMA_SXCR_EN);
		while((pDMAx->DMA_Sx[reqStream].SxCR & (1 << DMA_SXCR_EN)) == SET);
	}
}

/*
 * @fn			- DMA_ConfigInterrupts
 *
 * @brief		- This function enables or disables event/error interrupts
 * 				  based on the interrupt settings in the DMA handle.
 *
 * @param[DMA_Handle_t*]	- Base address of the DMA Handle.
 * @param[uint8_t]			- Request Stream Number. (REQ_STREAM_0, ... REQ_STREAM_7)
 *
 * @return		- None.
 *
 * @note		- None.
 */
void DMA_ConfigInterrupts(DMA_Handle_t *DMA_Handle, uint8_t reqStream)
{
	//Check Half-Transfer control setting.
	if(DMA_Handle->DMA_Config.DMA_ItEnable.DMA_HTIE == ENABLE)
		DMA_Handle->pDMAx->DMA_Sx[reqStream].SxCR |= (1 << DMA_SXCR_HTIE);
	else
		DMA_Handle->pDMAx->DMA_Sx[reqStream].SxCR &= ~(1 << DMA_SXCR_HTIE);

	//Check Transfer Complete control setting.
	if(DMA_Handle->DMA_Config.DMA_ItEnable.DMA_TCIE == ENABLE)
		DMA_Handle->pDMAx->DMA_Sx[reqStream].SxCR |= (1 << DMA_SXCR_TCIE);
	else
		DMA_Handle->pDMAx->DMA_Sx[reqStream].SxCR &= ~(1 << DMA_SXCR_TCIE);

	//Check Transfer Error control setting.
	if(DMA_Handle->DMA_Config.DMA_ItEnable.DMA_TEIE == ENABLE)
		DMA_Handle->pDMAx->DMA_Sx[reqStream].SxCR |= (1 << DMA_SXCR_TEIE);
	else
		DMA_Handle->pDMAx->DMA_Sx[reqStream].SxCR &= ~(1 << DMA_SXCR_TEIE);

	//Check FIFO Overrun/Underrun Error control setting.
	if(DMA_Handle->DMA_Config.DMA_ItEnable.DMA_FEIE == ENABLE)
		DMA_Handle->pDMAx->DMA_Sx[reqStream].SxCR |= (1 << DMA_SXFCR_FEIE);
	else
		DMA_Handle->pDMAx->DMA_Sx[reqStream].SxCR &= ~(1 << DMA_SXFCR_FEIE);

	//Check Direct Mode Error control setting.
	if(DMA_Handle->DMA_Config.DMA_ItEnable.DMA_DMEIE == ENABLE)
		DMA_Handle->pDMAx->DMA_Sx[reqStream].SxCR |= (1 << DMA_SXCR_DMEIE);
	else
		DMA_Handle->pDMAx->DMA_Sx[reqStream].SxCR &= ~(1 << DMA_SXCR_DMEIE);
}

/***************************************************************************************/


/***************** DMAx IRQ Handling ***************************************************/
/*
 * @fn			- DMAx_Streamx_IRQHandling
 *
 * @brief		- Each IRQHandling function can be called by
 * 				  the according DMAx Streamx. These functions
 * 				  determines which interrupt was triggered
 * 				  and handles it accordingly.
 *
 * @param[DMA_Handle_t*]	- Base address of the DMA Handle.
 *
 * @return		- None.
 *
 * @note		- This applies to all DMAx_Streamx_IRQHandling
 * 				  functions.
 */

void DMA1_Stream0_IRQHandling(DMA_Handle_t *DMA_Handle)
{
	//Check for FIFO Error Interrupt.
	if((DMA_Handle->pDMAx->LISR & (1 << DMA_LISR_FEIF0)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_0].SxCR & (1 << DMA_SXFCR_FEIE))))
		DMA_HandleFIFOErrIt(DMA_Handle, REQ_STREAM_0);

	//Check for Direct Mode Error Interrupt.
	if((DMA_Handle->pDMAx->LISR & (1 << DMA_LISR_DMEIF0)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_0].SxCR & (1 << DMA_SXCR_DMEIE))))
		DMA_HandleDirectErrIt(DMA_Handle, REQ_STREAM_0);

	//Check for Transfer Error Interrupt.
	if((DMA_Handle->pDMAx->LISR & (1 << DMA_LISR_TEIF0)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_0].SxCR & (1 << DMA_SXCR_TEIE))))
		DMA_HandleTransErrIt(DMA_Handle, REQ_STREAM_0);

	//Check for Transfer Complete Interrupt.
	if((DMA_Handle->pDMAx->LISR & (1 << DMA_LISR_TCIF0)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_0].SxCR & (1 << DMA_SXCR_TCIE))))
		DMA_HandleTransCmptIt(DMA_Handle, REQ_STREAM_0);

	//Check for Half-Transter Interrupt.
	if((DMA_Handle->pDMAx->LISR & (1 << DMA_LISR_HTIF0)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_0].SxCR & (1 << DMA_SXCR_HTIE))))
		DMA_HandleHalfTransCmptIt(DMA_Handle, REQ_STREAM_0);
}

void DMA1_Stream1_IRQHandling(DMA_Handle_t *DMA_Handle)
{
	//Check for FIFO Error Interrupt.
	if((DMA_Handle->pDMAx->LISR & (1 << DMA_LISR_FEIF1)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_1].SxCR & (1 << DMA_SXFCR_FEIE))))
		DMA_HandleFIFOErrIt(DMA_Handle, REQ_STREAM_1);

	//Check for Direct Mode Error Interrupt.
	if((DMA_Handle->pDMAx->LISR & (1 << DMA_LISR_DMEIF1)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_1].SxCR & (1 << DMA_SXCR_DMEIE))))
		DMA_HandleDirectErrIt(DMA_Handle, REQ_STREAM_1);

	//Check for Transfer Error Interrupt.
	if((DMA_Handle->pDMAx->LISR & (1 << DMA_LISR_TEIF1)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_1].SxCR & (1 << DMA_SXCR_TEIE))))
		DMA_HandleTransErrIt(DMA_Handle, REQ_STREAM_1);

	//Check for Transfer Complete Interrupt.
	if((DMA_Handle->pDMAx->LISR & (1 << DMA_LISR_TCIF1)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_1].SxCR & (1 << DMA_SXCR_TCIE))))
		DMA_HandleTransCmptIt(DMA_Handle, REQ_STREAM_1);

	//Check for Half-Transter Interrupt.
	if((DMA_Handle->pDMAx->LISR & (1 << DMA_LISR_HTIF1)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_1].SxCR & (1 << DMA_SXCR_HTIE))))
		DMA_HandleHalfTransCmptIt(DMA_Handle, REQ_STREAM_1);
}

void DMA1_Stream2_IRQHandling(DMA_Handle_t *DMA_Handle)
{
	//Check for FIFO Error Interrupt.
	if((DMA_Handle->pDMAx->LISR & (1 << DMA_LISR_FEIF2)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_2].SxCR & (1 << DMA_SXFCR_FEIE))))
		DMA_HandleFIFOErrIt(DMA_Handle, REQ_STREAM_2);

	//Check for Direct Mode Error Interrupt.
	if((DMA_Handle->pDMAx->LISR & (1 << DMA_LISR_DMEIF2)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_2].SxCR & (1 << DMA_SXCR_DMEIE))))
		DMA_HandleDirectErrIt(DMA_Handle, REQ_STREAM_2);

	//Check for Transfer Error Interrupt.
	if((DMA_Handle->pDMAx->LISR & (1 << DMA_LISR_TEIF2)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_2].SxCR & (1 << DMA_SXCR_TEIE))))
		DMA_HandleTransErrIt(DMA_Handle, REQ_STREAM_2);

	//Check for Transfer Complete Interrupt.
	if((DMA_Handle->pDMAx->LISR & (1 << DMA_LISR_TCIF2)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_2].SxCR & (1 << DMA_SXCR_TCIE))))
		DMA_HandleTransCmptIt(DMA_Handle, REQ_STREAM_2);

	//Check for Half-Transter Interrupt.
	if((DMA_Handle->pDMAx->LISR & (1 << DMA_LISR_HTIF2)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_2].SxCR & (1 << DMA_SXCR_HTIE))))
		DMA_HandleHalfTransCmptIt(DMA_Handle, REQ_STREAM_2);
}

void DMA1_Stream3_IRQHandling(DMA_Handle_t *DMA_Handle)
{
	//Check for FIFO Error Interrupt.
	if((DMA_Handle->pDMAx->LISR & (1 << DMA_LISR_FEIF3)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_3].SxCR & (1 << DMA_SXFCR_FEIE))))
		DMA_HandleFIFOErrIt(DMA_Handle, REQ_STREAM_3);

	//Check for Direct Mode Error Interrupt.
	if((DMA_Handle->pDMAx->LISR & (1 << DMA_LISR_DMEIF3)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_3].SxCR & (1 << DMA_SXCR_DMEIE))))
		DMA_HandleDirectErrIt(DMA_Handle, REQ_STREAM_3);

	//Check for Transfer Error Interrupt.
	if((DMA_Handle->pDMAx->LISR & (1 << DMA_LISR_TEIF3)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_3].SxCR & (1 << DMA_SXCR_TEIE))))
		DMA_HandleTransErrIt(DMA_Handle, REQ_STREAM_3);

	//Check for Transfer Complete Interrupt.
	if((DMA_Handle->pDMAx->LISR & (1 << DMA_LISR_TCIF3)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_3].SxCR & (1 << DMA_SXCR_TCIE))))
		DMA_HandleTransCmptIt(DMA_Handle, REQ_STREAM_3);

	//Check for Half-Transter Interrupt.
	if((DMA_Handle->pDMAx->LISR & (1 << DMA_LISR_HTIF3)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_3].SxCR & (1 << DMA_SXCR_HTIE))))
		DMA_HandleHalfTransCmptIt(DMA_Handle, REQ_STREAM_3);
}

void DMA1_Stream4_IRQHandling(DMA_Handle_t *DMA_Handle)
{
	//Check for FIFO Error Interrupt.
	if((DMA_Handle->pDMAx->HISR & (1 << DMA_HISR_FEIF4)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_4].SxCR & (1 << DMA_SXFCR_FEIE))))
		DMA_HandleFIFOErrIt(DMA_Handle, REQ_STREAM_4);

	//Check for Direct Mode Error Interrupt.
	if((DMA_Handle->pDMAx->HISR & (1 << DMA_HISR_DMEIF4)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_4].SxCR & (1 << DMA_SXCR_DMEIE))))
		DMA_HandleDirectErrIt(DMA_Handle, REQ_STREAM_4);

	//Check for Transfer Error Interrupt.
	if((DMA_Handle->pDMAx->HISR & (1 << DMA_HISR_TEIF4)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_4].SxCR & (1 << DMA_SXCR_TEIE))))
		DMA_HandleTransErrIt(DMA_Handle, REQ_STREAM_4);

	//Check for Transfer Complete Interrupt.
	if((DMA_Handle->pDMAx->HISR & (1 << DMA_HISR_TCIF4)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_4].SxCR & (1 << DMA_SXCR_TCIE))))
		DMA_HandleTransCmptIt(DMA_Handle, REQ_STREAM_4);

	//Check for Half-Transter Interrupt.
	if((DMA_Handle->pDMAx->HISR & (1 << DMA_HISR_HTIF4)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_4].SxCR & (1 << DMA_SXCR_HTIE))))
		DMA_HandleHalfTransCmptIt(DMA_Handle, REQ_STREAM_4);
}

void DMA1_Stream5_IRQHandling(DMA_Handle_t *DMA_Handle)
{
	//Check for FIFO Error Interrupt.
	if((DMA_Handle->pDMAx->HISR & (1 << DMA_HISR_FEIF5)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_5].SxCR & (1 << DMA_SXFCR_FEIE))))
		DMA_HandleFIFOErrIt(DMA_Handle, REQ_STREAM_5);

	//Check for Direct Mode Error Interrupt.
	if((DMA_Handle->pDMAx->HISR & (1 << DMA_HISR_DMEIF5)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_5].SxCR & (1 << DMA_SXCR_DMEIE))))
		DMA_HandleDirectErrIt(DMA_Handle, REQ_STREAM_5);

	//Check for Transfer Error Interrupt.
	if((DMA_Handle->pDMAx->HISR & (1 << DMA_HISR_TEIF5)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_5].SxCR & (1 << DMA_SXCR_TEIE))))
		DMA_HandleTransErrIt(DMA_Handle, REQ_STREAM_5);

	//Check for Transfer Complete Interrupt.
	if((DMA_Handle->pDMAx->HISR & (1 << DMA_HISR_TCIF5)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_5].SxCR & (1 << DMA_SXCR_TCIE))))
		DMA_HandleTransCmptIt(DMA_Handle, REQ_STREAM_5);

	//Check for Half-Transter Interrupt.
	if((DMA_Handle->pDMAx->HISR & (1 << DMA_HISR_HTIF5)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_5].SxCR & (1 << DMA_SXCR_HTIE))))
		DMA_HandleHalfTransCmptIt(DMA_Handle, REQ_STREAM_5);
}

void DMA1_Stream6_IRQHandling(DMA_Handle_t *DMA_Handle)
{
	//Check for FIFO Error Interrupt.
	if((DMA_Handle->pDMAx->HISR & (1 << DMA_HISR_FEIF6)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_6].SxCR & (1 << DMA_SXFCR_FEIE))))
		DMA_HandleFIFOErrIt(DMA_Handle, REQ_STREAM_6);

	//Check for Direct Mode Error Interrupt.
	if((DMA_Handle->pDMAx->HISR & (1 << DMA_HISR_DMEIF6)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_6].SxCR & (1 << DMA_SXCR_DMEIE))))
		DMA_HandleDirectErrIt(DMA_Handle, REQ_STREAM_6);

	//Check for Transfer Error Interrupt.
	if((DMA_Handle->pDMAx->HISR & (1 << DMA_HISR_TEIF6)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_6].SxCR & (1 << DMA_SXCR_TEIE))))
		DMA_HandleTransErrIt(DMA_Handle, REQ_STREAM_6);

	//Check for Transfer Complete Interrupt.
	if((DMA_Handle->pDMAx->HISR & (1 << DMA_HISR_TCIF6)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_6].SxCR & (1 << DMA_SXCR_TCIE))))
		DMA_HandleTransCmptIt(DMA_Handle, REQ_STREAM_6);

	//Check for Half-Transter Interrupt.
	if((DMA_Handle->pDMAx->HISR & (1 << DMA_HISR_HTIF6)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_6].SxCR & (1 << DMA_SXCR_HTIE))))
		DMA_HandleHalfTransCmptIt(DMA_Handle, REQ_STREAM_6);
}

void DMA1_Stream7_IRQHandling(DMA_Handle_t *DMA_Handle)
{
	//Check for FIFO Error Interrupt.
	if((DMA_Handle->pDMAx->HISR & (1 << DMA_HISR_FEIF7)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_7].SxCR & (1 << DMA_SXFCR_FEIE))))
		DMA_HandleFIFOErrIt(DMA_Handle, REQ_STREAM_7);

	//Check for Direct Mode Error Interrupt.
	if((DMA_Handle->pDMAx->HISR & (1 << DMA_HISR_DMEIF7)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_7].SxCR & (1 << DMA_SXCR_DMEIE))))
		DMA_HandleDirectErrIt(DMA_Handle, REQ_STREAM_7);

	//Check for Transfer Error Interrupt.
	if((DMA_Handle->pDMAx->HISR & (1 << DMA_HISR_TEIF7)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_7].SxCR & (1 << DMA_SXCR_TEIE))))
		DMA_HandleTransErrIt(DMA_Handle, REQ_STREAM_7);

	//Check for Transfer Complete Interrupt.
	if((DMA_Handle->pDMAx->HISR & (1 << DMA_HISR_TCIF7)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_7].SxCR & (1 << DMA_SXCR_TCIE))))
		DMA_HandleTransCmptIt(DMA_Handle, REQ_STREAM_7);

	//Check for Half-Transter Interrupt.
	if((DMA_Handle->pDMAx->HISR & (1 << DMA_HISR_HTIF7)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_7].SxCR & (1 << DMA_SXCR_HTIE))))
		DMA_HandleHalfTransCmptIt(DMA_Handle, REQ_STREAM_7);
}

void DMA2_Stream0_IRQHandling(DMA_Handle_t *DMA_Handle)
{
	//Check for FIFO Error Interrupt.
	if((DMA_Handle->pDMAx->LISR & (1 << DMA_LISR_FEIF0)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_0].SxCR & (1 << DMA_SXFCR_FEIE))))
		DMA_HandleFIFOErrIt(DMA_Handle, REQ_STREAM_0);

	//Check for Direct Mode Error Interrupt.
	if((DMA_Handle->pDMAx->LISR & (1 << DMA_LISR_DMEIF0)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_0].SxCR & (1 << DMA_SXCR_DMEIE))))
		DMA_HandleDirectErrIt(DMA_Handle, REQ_STREAM_0);

	//Check for Transfer Error Interrupt.
	if((DMA_Handle->pDMAx->LISR & (1 << DMA_LISR_TEIF0)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_0].SxCR & (1 << DMA_SXCR_TEIE))))
		DMA_HandleTransErrIt(DMA_Handle, REQ_STREAM_0);

	//Check for Transfer Complete Interrupt.
	if((DMA_Handle->pDMAx->LISR & (1 << DMA_LISR_TCIF0)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_0].SxCR & (1 << DMA_SXCR_TCIE))))
		DMA_HandleTransCmptIt(DMA_Handle, REQ_STREAM_0);

	//Check for Half-Transter Interrupt.
	if((DMA_Handle->pDMAx->LISR & (1 << DMA_LISR_HTIF0)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_0].SxCR & (1 << DMA_SXCR_HTIE))))
		DMA_HandleHalfTransCmptIt(DMA_Handle, REQ_STREAM_0);
}

void DMA2_Stream1_IRQHandling(DMA_Handle_t *DMA_Handle)
{
	//Check for FIFO Error Interrupt.
	if((DMA_Handle->pDMAx->LISR & (1 << DMA_LISR_FEIF1)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_1].SxCR & (1 << DMA_SXFCR_FEIE))))
		DMA_HandleFIFOErrIt(DMA_Handle, REQ_STREAM_1);

	//Check for Direct Mode Error Interrupt.
	if((DMA_Handle->pDMAx->LISR & (1 << DMA_LISR_DMEIF1)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_1].SxCR & (1 << DMA_SXCR_DMEIE))))
		DMA_HandleDirectErrIt(DMA_Handle, REQ_STREAM_1);

	//Check for Transfer Error Interrupt.
	if((DMA_Handle->pDMAx->LISR & (1 << DMA_LISR_TEIF1)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_1].SxCR & (1 << DMA_SXCR_TEIE))))
		DMA_HandleTransErrIt(DMA_Handle, REQ_STREAM_1);

	//Check for Transfer Complete Interrupt.
	if((DMA_Handle->pDMAx->LISR & (1 << DMA_LISR_TCIF1)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_1].SxCR & (1 << DMA_SXCR_TCIE))))
		DMA_HandleTransCmptIt(DMA_Handle, REQ_STREAM_1);

	//Check for Half-Transter Interrupt.
	if((DMA_Handle->pDMAx->LISR & (1 << DMA_LISR_HTIF1)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_1].SxCR & (1 << DMA_SXCR_HTIE))))
		DMA_HandleHalfTransCmptIt(DMA_Handle, REQ_STREAM_1);
}

void DMA2_Stream2_IRQHandling(DMA_Handle_t *DMA_Handle)
{
	//Check for FIFO Error Interrupt.
	if((DMA_Handle->pDMAx->LISR & (1 << DMA_LISR_FEIF2)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_2].SxCR & (1 << DMA_SXFCR_FEIE))))
		DMA_HandleFIFOErrIt(DMA_Handle, REQ_STREAM_2);

	//Check for Direct Mode Error Interrupt.
	if((DMA_Handle->pDMAx->LISR & (1 << DMA_LISR_DMEIF2)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_2].SxCR & (1 << DMA_SXCR_DMEIE))))
		DMA_HandleDirectErrIt(DMA_Handle, REQ_STREAM_2);

	//Check for Transfer Error Interrupt.
	if((DMA_Handle->pDMAx->LISR & (1 << DMA_LISR_TEIF2)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_2].SxCR & (1 << DMA_SXCR_TEIE))))
		DMA_HandleTransErrIt(DMA_Handle, REQ_STREAM_2);

	//Check for Transfer Complete Interrupt.
	if((DMA_Handle->pDMAx->LISR & (1 << DMA_LISR_TCIF2)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_2].SxCR & (1 << DMA_SXCR_TCIE))))
		DMA_HandleTransCmptIt(DMA_Handle, REQ_STREAM_2);

	//Check for Half-Transter Interrupt.
	if((DMA_Handle->pDMAx->LISR & (1 << DMA_LISR_HTIF2)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_2].SxCR & (1 << DMA_SXCR_HTIE))))
		DMA_HandleHalfTransCmptIt(DMA_Handle, REQ_STREAM_2);
}

void DMA2_Stream3_IRQHandling(DMA_Handle_t *DMA_Handle)
{
	//Check for FIFO Error Interrupt.
	if((DMA_Handle->pDMAx->LISR & (1 << DMA_LISR_FEIF3)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_3].SxCR & (1 << DMA_SXFCR_FEIE))))
		DMA_HandleFIFOErrIt(DMA_Handle, REQ_STREAM_3);

	//Check for Direct Mode Error Interrupt.
	if((DMA_Handle->pDMAx->LISR & (1 << DMA_LISR_DMEIF3)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_3].SxCR & (1 << DMA_SXCR_DMEIE))))
		DMA_HandleDirectErrIt(DMA_Handle, REQ_STREAM_3);

	//Check for Transfer Error Interrupt.
	if((DMA_Handle->pDMAx->LISR & (1 << DMA_LISR_TEIF3)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_3].SxCR & (1 << DMA_SXCR_TEIE))))
		DMA_HandleTransErrIt(DMA_Handle, REQ_STREAM_3);

	//Check for Transfer Complete Interrupt.
	if((DMA_Handle->pDMAx->LISR & (1 << DMA_LISR_TCIF3)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_3].SxCR & (1 << DMA_SXCR_TCIE))))
		DMA_HandleTransCmptIt(DMA_Handle, REQ_STREAM_3);

	//Check for Half-Transter Interrupt.
	if((DMA_Handle->pDMAx->LISR & (1 << DMA_LISR_HTIF3)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_3].SxCR & (1 << DMA_SXCR_HTIE))))
		DMA_HandleHalfTransCmptIt(DMA_Handle, REQ_STREAM_3);
}

void DMA2_Stream4_IRQHandling(DMA_Handle_t *DMA_Handle)
{
	//Check for FIFO Error Interrupt.
	if((DMA_Handle->pDMAx->HISR & (1 << DMA_HISR_FEIF4)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_4].SxCR & (1 << DMA_SXFCR_FEIE))))
		DMA_HandleFIFOErrIt(DMA_Handle, REQ_STREAM_4);

	//Check for Direct Mode Error Interrupt.
	if((DMA_Handle->pDMAx->HISR & (1 << DMA_HISR_DMEIF4)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_4].SxCR & (1 << DMA_SXCR_DMEIE))))
		DMA_HandleDirectErrIt(DMA_Handle, REQ_STREAM_4);

	//Check for Transfer Error Interrupt.
	if((DMA_Handle->pDMAx->HISR & (1 << DMA_HISR_TEIF4)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_4].SxCR & (1 << DMA_SXCR_TEIE))))
		DMA_HandleTransErrIt(DMA_Handle, REQ_STREAM_4);

	//Check for Transfer Complete Interrupt.
	if((DMA_Handle->pDMAx->HISR & (1 << DMA_HISR_TCIF4)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_4].SxCR & (1 << DMA_SXCR_TCIE))))
		DMA_HandleTransCmptIt(DMA_Handle, REQ_STREAM_4);

	//Check for Half-Transter Interrupt.
	if((DMA_Handle->pDMAx->HISR & (1 << DMA_HISR_HTIF4)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_4].SxCR & (1 << DMA_SXCR_HTIE))))
		DMA_HandleHalfTransCmptIt(DMA_Handle, REQ_STREAM_4);
}

void DMA2_Stream5_IRQHandling(DMA_Handle_t *DMA_Handle)
{
	//Check for FIFO Error Interrupt.
	if((DMA_Handle->pDMAx->HISR & (1 << DMA_HISR_FEIF5)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_5].SxCR & (1 << DMA_SXFCR_FEIE))))
		DMA_HandleFIFOErrIt(DMA_Handle, REQ_STREAM_5);

	//Check for Direct Mode Error Interrupt.
	if((DMA_Handle->pDMAx->HISR & (1 << DMA_HISR_DMEIF5)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_5].SxCR & (1 << DMA_SXCR_DMEIE))))
		DMA_HandleDirectErrIt(DMA_Handle, REQ_STREAM_5);

	//Check for Transfer Error Interrupt.
	if((DMA_Handle->pDMAx->HISR & (1 << DMA_HISR_TEIF5)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_5].SxCR & (1 << DMA_SXCR_TEIE))))
		DMA_HandleTransErrIt(DMA_Handle, REQ_STREAM_5);

	//Check for Transfer Complete Interrupt.
	if((DMA_Handle->pDMAx->HISR & (1 << DMA_HISR_TCIF5)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_5].SxCR & (1 << DMA_SXCR_TCIE))))
		DMA_HandleTransCmptIt(DMA_Handle, REQ_STREAM_5);

	//Check for Half-Transter Interrupt.
	if((DMA_Handle->pDMAx->HISR & (1 << DMA_HISR_HTIF5)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_5].SxCR & (1 << DMA_SXCR_HTIE))))
		DMA_HandleHalfTransCmptIt(DMA_Handle, REQ_STREAM_5);
}

void DMA2_Stream6_IRQHandling(DMA_Handle_t *DMA_Handle)
{
	//Check for FIFO Error Interrupt.
	if((DMA_Handle->pDMAx->HISR & (1 << DMA_HISR_FEIF6)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_6].SxCR & (1 << DMA_SXFCR_FEIE))))
		DMA_HandleFIFOErrIt(DMA_Handle, REQ_STREAM_6);

	//Check for Direct Mode Error Interrupt.
	if((DMA_Handle->pDMAx->HISR & (1 << DMA_HISR_DMEIF6)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_6].SxCR & (1 << DMA_SXCR_DMEIE))))
		DMA_HandleDirectErrIt(DMA_Handle, REQ_STREAM_6);

	//Check for Transfer Error Interrupt.
	if((DMA_Handle->pDMAx->HISR & (1 << DMA_HISR_TEIF6)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_6].SxCR & (1 << DMA_SXCR_TEIE))))
		DMA_HandleTransErrIt(DMA_Handle, REQ_STREAM_6);

	//Check for Transfer Complete Interrupt.
	if((DMA_Handle->pDMAx->HISR & (1 << DMA_HISR_TCIF6)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_6].SxCR & (1 << DMA_SXCR_TCIE))))
		DMA_HandleTransCmptIt(DMA_Handle, REQ_STREAM_6);

	//Check for Half-Transter Interrupt.
	if((DMA_Handle->pDMAx->HISR & (1 << DMA_HISR_HTIF6)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_6].SxCR & (1 << DMA_SXCR_HTIE))))
		DMA_HandleHalfTransCmptIt(DMA_Handle, REQ_STREAM_6);
}

void DMA2_Stream7_IRQHandling(DMA_Handle_t *DMA_Handle)
{
	//Check for FIFO Error Interrupt.
	if((DMA_Handle->pDMAx->HISR & (1 << DMA_HISR_FEIF7)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_7].SxCR & (1 << DMA_SXFCR_FEIE))))
		DMA_HandleFIFOErrIt(DMA_Handle, REQ_STREAM_7);

	//Check for Direct Mode Error Interrupt.
	if((DMA_Handle->pDMAx->HISR & (1 << DMA_HISR_DMEIF7)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_7].SxCR & (1 << DMA_SXCR_DMEIE))))
		DMA_HandleDirectErrIt(DMA_Handle, REQ_STREAM_7);

	//Check for Transfer Error Interrupt.
	if((DMA_Handle->pDMAx->HISR & (1 << DMA_HISR_TEIF7)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_7].SxCR & (1 << DMA_SXCR_TEIE))))
		DMA_HandleTransErrIt(DMA_Handle, REQ_STREAM_7);

	//Check for Transfer Complete Interrupt.
	if((DMA_Handle->pDMAx->HISR & (1 << DMA_HISR_TCIF7)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_7].SxCR & (1 << DMA_SXCR_TCIE))))
		DMA_HandleTransCmptIt(DMA_Handle, REQ_STREAM_7);

	//Check for Half-Transter Interrupt.
	if((DMA_Handle->pDMAx->HISR & (1 << DMA_HISR_HTIF7)) &&
			((DMA_Handle->pDMAx->DMA_Sx[REQ_STREAM_7].SxCR & (1 << DMA_SXCR_HTIE))))
		DMA_HandleHalfTransCmptIt(DMA_Handle, REQ_STREAM_7);
}

/* Weak function that can be implemented in user application. */
void __attribute__((weak)) DMA_ApplicationEventCallback(uint8_t appEv, uint8_t reqStream) {}

/***************************************************************************************/


/***************** Private Helper Function Definitions *********************************/

/*
 * @fn			- DMA_ConfigPeripheralAddress
 *
 * @brief		- This function sets the address of the
 * 				  peripheral register.
 *
 * @param[DMA_RegDef_t*]	- Base address of the DMA Register.
 * @param[uint8_t]			- Request Stream Number.
 * @param[uint32_t]			- Address of the peripheral register.
 * 							  ex.((uint32_t)&pADC->DR)
 *
 * @return		- None.
 *
 * @note		- Private Helper Function.
 */
static void DMA_ConfigPeripheralAddress(DMA_RegDef_t *pDMAx, uint8_t reqStream, uint32_t periAddress)
{
	pDMAx->DMA_Sx[reqStream].SxPAR = periAddress;
}

/*
 * @fn			- DMA_ConfigMemoryAddresses
 *
 * @brief		- This function sets the address of the
 * 				  memory location.
 *
 * @param[DMA_RegDef_t*]	- Base address of the DMA Register.
 * @param[uint8_t]			- Request Stream Number.
 * @param[uint32_t]			- Address of the memory location.
 * 							  ex.((uint32_t)&data)
 *
 * @return		- None.
 *
 * @note		- Private Helper Function.
 */
static void DMA_ConfigMemoryAddresses(DMA_Handle_t *DMA_Handle, uint8_t reqStream, uint32_t memAddress)
{
	if(DMA_Handle->DMA_Config.DMA_TransactionType == DMA_DOUBLE_BUFFER_TRANSACTION)
	{//Double Buffer Transaction will be used.
		DMA_Handle->pDMAx->DMA_Sx[reqStream].SxM0AR = memAddress;
		DMA_Handle->pDMAx->DMA_Sx[reqStream].SxM1AR = memAddress;
	}else//Regular Type Transaction will be used.
		DMA_Handle->pDMAx->DMA_Sx[reqStream].SxM0AR = memAddress;
}

/*
 * @fn			- DMA_ConfigItemsToTransfer
 *
 * @brief		- This function sets the number of items to
 * 				  transfer.
 *
 * @param[DMA_RegDef_t*]	- Base address of the DMA Register.
 * @param[uint8_t]			- Request Stream Number.
 * @param[uint8_t]			- Number of items to transfer.
 *
 * @return		- None.
 *
 * @note		- Private Helper Function. If circular mode is
 * 				  enabled, this number automatically resets
 * 				  once it reaches zero.
 */
static void DMA_ConfigItemsToTransfer(DMA_RegDef_t *pDMAx, uint8_t reqStream, uint8_t itemsToTransfer)
{
	pDMAx->DMA_Sx[reqStream].SxNDTR = itemsToTransfer;
}

/*
 * @fn			- DMA_ConfigChannel
 *
 * @brief		- This function configures which channel of the
 * 				  request stream will be used.
 *
 * @param[DMA_RegDef_t*]	- Base address of the DMA Register.
 * @param[uint8_t]			- Request Stream Number.
 * @param[uint8_t]			- Request Stream Channel.
 *
 * @return		- None.
 *
 * @note		- Private Helper Function.
 */
static void DMA_ConfigChannel(DMA_RegDef_t *pDMAx, uint8_t reqStream, uint8_t channel)
{
	if(channel == ADC_IN0)
		pDMAx->DMA_Sx[reqStream].SxCR &= ~(channel << DMA_SXCR_CHSEL);
	else
		pDMAx->DMA_Sx[reqStream].SxCR |= (channel << DMA_SXCR_CHSEL);
}

/*
 * @fn			- DMA_ConfigStreamPriority
 *
 * @brief		- This function configures the arbiter
 * 				  priority value for the given DMA stream.
 *
 * @param[DMA_RegDef_t*]	- Base address of the DMA Register.
 * @param[uint8_t]			- Request Stream Number.
 * @param[uint8_t]			- Request Stream Priority Value.
 *
 * @return		- None.
 *
 * @note		- Private Helper Function.
 */
static void DMA_ConfigStreamPriority(DMA_RegDef_t *pDMAx, uint8_t reqStream, uint8_t priority)
{
	pDMAx->DMA_Sx[reqStream].SxCR |= (priority << DMA_SXCR_PL);
}

/*
 * @fn			- DMA_ConfigFIFO
 *
 * @brief		- This function configures the use of fifo mode.
 * 				  If fifo mode is used, the fifo threshold is
 * 				  also configured.
 *
 * @param[DMA_RegDef_t*]	- Base address of the DMA Register.
 * @param[uint8_t]			- Request Stream Number.
 * @param[uint8_t]			- ENABLE or DISABLE.
 *
 * @return		- None.
 *
 * @note		- Private Helper Function.
 */
static void DMA_ConfigFIFO(DMA_Handle_t *DMA_Handle, uint8_t reqStream, uint8_t EnOrDi)
{
	if(EnOrDi == ENABLE)
	{//FIFO Mode is used.
		DMA_Handle->pDMAx->DMA_Sx[reqStream].SxFCR |= (1 << DMA_SXFCR_DMDIS);
		if(DMA_Handle->DMA_Config.DMA_FIFO_Threshold == DMA_1_4_FULL_FIFO)
			DMA_Handle->pDMAx->DMA_Sx[reqStream].SxFCR &= ~(1 << DMA_SXFCR_FTH);

		DMA_Handle->pDMAx->DMA_Sx[reqStream].SxFCR |= (DMA_Handle->DMA_Config.DMA_FIFO_Threshold << DMA_SXFCR_FTH);
	}else//Direct Mode is used.
		DMA_Handle->pDMAx->DMA_Sx[reqStream].SxFCR &= ~(1 << DMA_SXFCR_DMDIS);
}

/*
 * @fn			- DMA_ConfigTransDirection
 *
 * @brief		- This function configures the direction of
 * 				  DMA transfer stream.
 *
 * @param[DMA_RegDef_t*]	- Base address of the DMA Register.
 * @param[uint8_t]			- Request Stream Number.
 * @param[uint8_t]			- Direction of the DMA stream.
 * 							  (DMA_PERIPHERAL_TO_MEMORY,
 * 							  DMA_MEMORY_TO_PERIPHERAL,
 * 							  DMA_MEMORY_TO_MEMORY)
 *
 * @return		- None.
 *
 * @note		- Private Helper Function.
 */
static void DMA_ConfigTransDirection(DMA_RegDef_t *pDMAx, uint8_t reqStream, uint8_t dir)
{
	if(dir == DMA_PERIPHERAL_TO_MEMORY)
		pDMAx->DMA_Sx[reqStream].SxCR &= ~(1 << DMA_SXCR_DIR);
	else
		pDMAx->DMA_Sx[reqStream].SxCR |= (dir << DMA_SXCR_DIR);
}

/*
 * @fn			- DMA_ConfigPtrIncrement
 *
 * @brief		- This function configures which address
 * 				  pointer(s) will be automatically incremented
 * 				  after each transfer.
 *
 * @param[DMA_RegDef_t*]	- Base address of the DMA Register.
 * @param[uint8_t]			- Request Stream Number.
 * @param[uint8_t]			- Increment Mode. (DMA_FIXED_MODE,
 * 							  DMA_MEM_INC_MODE_ENABLE,
 * 							  DMA_PER_INC_MODE_ENABLE,
 * 							  DMA_MEM_PERI_INC_MODE_EN)
 *
 * @return		- None.
 *
 * @note		- Private Helper Function.
 */
static void DMA_ConfigPtrIncrement(DMA_RegDef_t *pDMAx, uint8_t reqStream, uint8_t incMode)
{
	if(incMode == DMA_MEM_INC_MODE_ENABLE)
	{//Ptr increment for memory only.
		pDMAx->DMA_Sx[reqStream].SxCR &= ~(1 << DMA_SXCR_PINC);
		pDMAx->DMA_Sx[reqStream].SxCR |= (1 << DMA_SXCR_MINC);
	}else if(incMode == DMA_PER_INC_MODE_ENABLE)
	{//Ptr increment for peripheral only.
		pDMAx->DMA_Sx[reqStream].SxCR &= ~(1 << DMA_SXCR_MINC);
		pDMAx->DMA_Sx[reqStream].SxCR |= (1 << DMA_SXCR_PINC);
	}else if(incMode == DMA_MEM_PERI_INC_MODE_EN)
	{//Ptr increment for both.
		pDMAx->DMA_Sx[reqStream].SxCR |= (1 << DMA_SXCR_PINC);
		pDMAx->DMA_Sx[reqStream].SxCR |= (1 << DMA_SXCR_MINC);
	}else
	{//Fixed mode is used.
		pDMAx->DMA_Sx[reqStream].SxCR &= ~(1 << DMA_SXCR_PINC);
		pDMAx->DMA_Sx[reqStream].SxCR &= ~(1 << DMA_SXCR_MINC);
	}
}

/*
 * @fn			- DMA_ConfigBurstTransType
 *
 * @brief		- This function configures the burst
 * 				  configuration.
 *
 * @param[DMA_RegDef_t*]	- Base address of the DMA Register.
 * @param[uint8_t]			- Request Stream Number.
 * @param[uint8_t]			- Memory Burst Type.
 * 							  (DMA_INCR4, ... DMA_INCR16)
 * @param[uint8_t]			- Peripheral Burst Type.
 * 							  (DMA_INCR4, ... DMA_INCR16)
 *
 * @return		- None.
 *
 * @note		- Private Helper Function.
 */
static void DMA_ConfigBurstTransType(DMA_RegDef_t *pDMAx, uint8_t reqStream, uint8_t memType, uint8_t periType)
{
	if((memType <= DMA_INCR16 && memType >= DMA_INCR4) &&
			(periType <= DMA_INCR16 && periType >= DMA_INCR4))
	{//Burst Transfer Configuration.
		pDMAx->DMA_Sx[reqStream].SxCR |= (memType << DMA_SXCR_MBURST);
		pDMAx->DMA_Sx[reqStream].SxCR |= (periType << DMA_SXCR_PBURST);
	}else if(memType == DMA_SINGLE_TRANSFER && periType == DMA_SINGLE_TRANSFER)
	{//Single Transfer Configuration.
		pDMAx->DMA_Sx[reqStream].SxCR &= ~(memType << DMA_SXCR_MBURST);
		pDMAx->DMA_Sx[reqStream].SxCR &= ~(periType << DMA_SXCR_PBURST);
	}
}

/*
 * @fn			- DMA_ConfigDataWidths
 *
 * @brief		- This function configures the data widths
 * 				  for each address.
 *
 * @param[DMA_RegDef_t*]	- Base address of the DMA Register.
 * @param[uint8_t]			- Request Stream Number.
 * @param[uint8_t]			- Memory Data Size.
 * 							  (DMA_BYTE, DMA_HALF_WORD, DMA_WORD)
 * @param[uint8_t]			- Peripheral Data Size.
 * 							  (DMA_BYTE, DMA_HALF_WORD, DMA_WORD)
 *
 * @return		- None.
 *
 * @note		- Private Helper Function.
 */
static void DMA_ConfigDataWidths(DMA_RegDef_t *pDMAx, uint8_t reqStream, uint8_t msize, uint8_t psize)
{
	if((msize == DMA_WORD || msize == DMA_HALF_WORD) &&
			(psize == DMA_WORD || psize == DMA_HALF_WORD))
	{
		pDMAx->DMA_Sx[reqStream].SxCR |= (msize << DMA_SXCR_MSIZE);
		pDMAx->DMA_Sx[reqStream].SxCR |= (psize << DMA_SXCR_PSIZE);
	}else
	{	//Psize, Msize will be a BYTE.
		pDMAx->DMA_Sx[reqStream].SxCR &= ~(msize << DMA_SXCR_MSIZE);
		pDMAx->DMA_Sx[reqStream].SxCR &= ~(psize << DMA_SXCR_PSIZE);
	}
}

/*
 * @fn			- DMA_ConfigCircularMode
 *
 * @brief		- This function configures the use of circular mode.
 *
 * @param[DMA_RegDef_t*]	- Base address of the DMA Register.
 * @param[uint8_t]			- Request Stream Number.
 * @param[uint8_t]			- ENABLE or DISABLE
 *
 * @return		- None.
 *
 * @note		- Private Helper Function.
 */
static void DMA_ConfigCircularMode(DMA_RegDef_t *pDMAx, uint8_t reqStream, uint8_t EnOrDi)
{
	if(EnOrDi == ENABLE)
		pDMAx->DMA_Sx[reqStream].SxCR |= (1 << DMA_SXCR_CIRC);//Enable Circular Mode
	else
		pDMAx->DMA_Sx[reqStream].SxCR &= ~(1 << DMA_SXCR_CIRC);//Disable Circular Mode
}

/*
 * @fn			- DMA_ConfigDoubleBuffMode
 *
 * @brief		- This function configures the use of double
 * 				  buffer mode.
 *
 * @param[DMA_RegDef_t*]	- Base address of the DMA Register.
 * @param[uint8_t]			- Request Stream Number.
 * @param[uint8_t]			- ENABLE or DISABLE
 *
 * @return		- None.
 *
 * @note		- Private Helper Function.
 */
static void DMA_ConfigDoubleBuffMode(DMA_RegDef_t *pDMAx, uint8_t reqStream, uint8_t EnOrDi)
{
	if(EnOrDi == ENABLE)
		pDMAx->DMA_Sx[reqStream].SxCR |= (1 << DMA_SXCR_DBM);//Enable Double Buffer Mode
	else
		pDMAx->DMA_Sx[reqStream].SxCR &= ~(1 << DMA_SXCR_DBM);//Disable Double Buffer Mode
}


/*** Interrupt Helper Function Definitions ****/

/*
 * @fn			- DMA_HandleHalfTransCmptIt
 *
 * @brief		- This function handles the Half Transfer
 * 				  Complete interrupt by setting the CHTIFx
 * 				  bit in the xIFCR register for the appropriate
 * 				  stream number.
 *
 * @param[DMA_RegDef_t*]	- Base address of the DMA Register.
 * @param[uint8_t]			- Request Stream Number.
 *
 * @return		- None.
 *
 * @note		- Private Helper Function.
 */
static void DMA_HandleHalfTransCmptIt(DMA_Handle_t *DMA_Handle, uint8_t reqStream)
{
	switch(reqStream)
	{
		case REQ_STREAM_0: DMA_Handle->pDMAx->LIFCR |= (1 << DMA_LIFCR_CHTIF0); break;
		case REQ_STREAM_1: DMA_Handle->pDMAx->LIFCR |= (1 << DMA_LIFCR_CHTIF1); break;
		case REQ_STREAM_2: DMA_Handle->pDMAx->LIFCR |= (1 << DMA_LIFCR_CHTIF2); break;
		case REQ_STREAM_3: DMA_Handle->pDMAx->LIFCR |= (1 << DMA_LIFCR_CHTIF3); break;
		case REQ_STREAM_4: DMA_Handle->pDMAx->HIFCR |= (1 << DMA_HIFCR_CHTIF4); break;
		case REQ_STREAM_5: DMA_Handle->pDMAx->HIFCR |= (1 << DMA_HIFCR_CHTIF5); break;
		case REQ_STREAM_6: DMA_Handle->pDMAx->HIFCR |= (1 << DMA_HIFCR_CHTIF6); break;
		case REQ_STREAM_7: DMA_Handle->pDMAx->HIFCR |= (1 << DMA_HIFCR_CHTIF7); break;
	}
	DMA_Handle->DMA_status = DMA_HALF_TRANSFER_COMPLETE;
	DMA_ApplicationEventCallback(DMA_Handle->DMA_status, reqStream);
}

/*
 * @fn			- DMA_HandleTransCmptIt
 *
 * @brief		- This function handles the Transfer
 * 				  Complete interrupt by setting the CTCIFx
 * 				  bit in the xIFCR register for the appropriate
 * 				  stream number.
 *
 * @param[DMA_RegDef_t*]	- Base address of the DMA Register.
 * @param[uint8_t]			- Request Stream Number.
 *
 * @return		- None.
 *
 * @note		- Private Helper Function.
 */
static void DMA_HandleTransCmptIt(DMA_Handle_t *DMA_Handle, uint8_t reqStream)
{
	switch(reqStream)
	{
		case REQ_STREAM_0: DMA_Handle->pDMAx->LIFCR |= (1 << DMA_LIFCR_CTCIF0); break;
		case REQ_STREAM_1: DMA_Handle->pDMAx->LIFCR |= (1 << DMA_LIFCR_CTCIF1); break;
		case REQ_STREAM_2: DMA_Handle->pDMAx->LIFCR |= (1 << DMA_LIFCR_CTCIF2); break;
		case REQ_STREAM_3: DMA_Handle->pDMAx->LIFCR |= (1 << DMA_LIFCR_CTCIF3); break;
		case REQ_STREAM_4: DMA_Handle->pDMAx->HIFCR |= (1 << DMA_HIFCR_CTCIF4); break;
		case REQ_STREAM_5: DMA_Handle->pDMAx->HIFCR |= (1 << DMA_HIFCR_CTCIF5); break;
		case REQ_STREAM_6: DMA_Handle->pDMAx->HIFCR |= (1 << DMA_HIFCR_CTCIF6); break;
		case REQ_STREAM_7: DMA_Handle->pDMAx->HIFCR |= (1 << DMA_HIFCR_CTCIF7); break;
	}
	DMA_Handle->DMA_status = DMA_TRANSFER_COMPLETE;
	DMA_ApplicationEventCallback(DMA_Handle->DMA_status, reqStream);
}

/*
 * @fn			- DMA_HandleTransErrIt
 *
 * @brief		- This function handles the Transfer
 * 				  Error interrupt by setting the CTEIFx
 * 				  bit in the xIFCR register for the
 * 				  appropriate stream number.
 *
 * @param[DMA_RegDef_t*]	- Base address of the DMA Register.
 * @param[uint8_t]			- Request Stream Number.
 *
 * @return		- None.
 *
 * @note		- Private Helper Function.
 */
static void DMA_HandleTransErrIt(DMA_Handle_t *DMA_Handle, uint8_t reqStream)
{
	switch(reqStream)
	{
		case REQ_STREAM_0: DMA_Handle->pDMAx->LIFCR |= (1 << DMA_LIFCR_CTEIF0); break;
		case REQ_STREAM_1: DMA_Handle->pDMAx->LIFCR |= (1 << DMA_LIFCR_CTEIF1); break;
		case REQ_STREAM_2: DMA_Handle->pDMAx->LIFCR |= (1 << DMA_LIFCR_CTEIF2); break;
		case REQ_STREAM_3: DMA_Handle->pDMAx->LIFCR |= (1 << DMA_LIFCR_CTEIF3); break;
		case REQ_STREAM_4: DMA_Handle->pDMAx->HIFCR |= (1 << DMA_HIFCR_CTEIF4); break;
		case REQ_STREAM_5: DMA_Handle->pDMAx->HIFCR |= (1 << DMA_HIFCR_CTEIF5); break;
		case REQ_STREAM_6: DMA_Handle->pDMAx->HIFCR |= (1 << DMA_HIFCR_CTEIF6); break;
		case REQ_STREAM_7: DMA_Handle->pDMAx->HIFCR |= (1 << DMA_HIFCR_CTEIF7); break;
	}
	DMA_Handle->DMA_status = DMA_TRANSFER_ERROR;
	DMA_ApplicationEventCallback(DMA_Handle->DMA_status, reqStream);
}

/*
 * @fn			- DMA_HandleFIFOErrIt
 *
 * @brief		- This function handles the FIFO
 * 				  Error interrupt by setting the CFEIFx
 * 				  bit in the xIFCR register for the
 * 				  appropriate stream number.
 *
 * @param[DMA_RegDef_t*]	- Base address of the DMA Register.
 * @param[uint8_t]			- Request Stream Number.
 *
 * @return		- None.
 *
 * @note		- Private Helper Function.
 */
static void DMA_HandleFIFOErrIt(DMA_Handle_t *DMA_Handle, uint8_t reqStream)
{
	switch(reqStream)
	{
		case REQ_STREAM_0: DMA_Handle->pDMAx->LIFCR |= (1 << DMA_LIFCR_CFEIF0); break;
		case REQ_STREAM_1: DMA_Handle->pDMAx->LIFCR |= (1 << DMA_LIFCR_CFEIF1); break;
		case REQ_STREAM_2: DMA_Handle->pDMAx->LIFCR |= (1 << DMA_LIFCR_CFEIF2); break;
		case REQ_STREAM_3: DMA_Handle->pDMAx->LIFCR |= (1 << DMA_LIFCR_CFEIF3); break;
		case REQ_STREAM_4: DMA_Handle->pDMAx->HIFCR |= (1 << DMA_HIFCR_CFEIF4); break;
		case REQ_STREAM_5: DMA_Handle->pDMAx->HIFCR |= (1 << DMA_HIFCR_CFEIF5); break;
		case REQ_STREAM_6: DMA_Handle->pDMAx->HIFCR |= (1 << DMA_HIFCR_CFEIF6); break;
		case REQ_STREAM_7: DMA_Handle->pDMAx->HIFCR |= (1 << DMA_HIFCR_CFEIF7); break;
	}
	DMA_Handle->DMA_status = DMA_FIFO_ERROR;
	DMA_ApplicationEventCallback(DMA_Handle->DMA_status, reqStream);
}

/*
 * @fn			- DMA_HandleDirectErrIt
 *
 * @brief		- This function handles the Direct Mode
 * 				  Error interrupt by setting the CDMEIFx
 * 				  bit in the xIFCR register for the
 * 				  appropriate stream number.
 *
 * @param[DMA_RegDef_t*]	- Base address of the DMA Register.
 * @param[uint8_t]			- Request Stream Number.
 *
 * @return		- None.
 *
 * @note		- Private Helper Function.
 */
static void DMA_HandleDirectErrIt(DMA_Handle_t *DMA_Handle, uint8_t reqStream)
{
	switch(reqStream)
	{
		case REQ_STREAM_0: DMA_Handle->pDMAx->LIFCR |= (1 << DMA_LIFCR_CDMEIF0); break;
		case REQ_STREAM_1: DMA_Handle->pDMAx->LIFCR |= (1 << DMA_LIFCR_CDMEIF1); break;
		case REQ_STREAM_2: DMA_Handle->pDMAx->LIFCR |= (1 << DMA_LIFCR_CDMEIF2); break;
		case REQ_STREAM_3: DMA_Handle->pDMAx->LIFCR |= (1 << DMA_LIFCR_CDMEIF3); break;
		case REQ_STREAM_4: DMA_Handle->pDMAx->HIFCR |= (1 << DMA_HIFCR_CDMEIF4); break;
		case REQ_STREAM_5: DMA_Handle->pDMAx->HIFCR |= (1 << DMA_HIFCR_CDMEIF5); break;
		case REQ_STREAM_6: DMA_Handle->pDMAx->HIFCR |= (1 << DMA_HIFCR_CDMEIF6); break;
		case REQ_STREAM_7: DMA_Handle->pDMAx->HIFCR |= (1 << DMA_HIFCR_CDMEIF7); break;
	}
	DMA_Handle->DMA_status = DMA_DIRECT_ERROR;
	DMA_ApplicationEventCallback(DMA_Handle->DMA_status, reqStream);
}


/***************************************************************************************/
