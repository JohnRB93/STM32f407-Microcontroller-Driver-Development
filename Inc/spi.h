#ifndef __SPI_H_
#define __SPI_H_

/* ** Before initializing the SPI Peripheral, the appropriate GPIO pins must
 *    be configured to their respective alternate funcionalities. Below are
*	  pins that can be used for each SPI Peripheral:
* 		SPI1
* 			PA04 -> AF5 -> NSS
* 			PA05 -> AF5 -> SCK
* 			PA06 -> AF5 -> MISO
* 			PA07 -> AF5 -> MOSI
* 			PA15 -> AF5 -> NSS
* 			PB03 -> AF5 -> SCK
* 			PB04 -> AF5 -> MISO
* 			PB05 -> AF5 -> MOSI
* 		SPI2
* 			PC02 -> AF5 -> MISO
* 			PC03 -> AF5 -> MOSI
* 			PB10 -> AF5 -> SCK
* 			PB12 -> AF5 -> NSS
* 			PB13 -> AF5 -> SCK
* 			PB14 -> AF5 -> MISO
* 			PB15 -> AF5 -> MOSI
* 			PI00 -> AF5 -> MSS
* 			PI01 -> AF5 -> SCK
* 			PI02 -> AF5 -> MISO
* 			PI03 -> AF5 -> MOSI
* 			PB09 -> AF5 -> NSS
* 		SPI3
* 			PA04 -> AF6 -> NSS
* 			PA15 -> AF6 -> NSS
* 			PC10 -> AF6 -> SCK
* 			PC11 -> AF6 -> MISO
* 			PC12 -> AF6 -> MOSI
* 			PB03 -> AF6 -> SCK
* 			PB04 -> AF6 -> MISO
* 			PB05 -> AF6 -> MOSI
 */

#include"stm32f4xx.h"

#include<stdio.h>
#include<string.h>
#include"gpio.h"

//SPIx Configuration Structure
typedef struct
{
	__IO uint8_t SPI_clkRelationship;		//SPI Clock Polarity and Phase Relationships
	__IO uint8_t SPI_bidirectionalMode;		//SPI Bidirectional Mode
	__IO uint8_t SPI_dataBitMode;			//SPI Data Frame Format
	__IO uint8_t SPI_baudRateDiv;			//SPI Baud Rate Division By Pclk
	__IO uint8_t SPI_bitOrder;				//SPI MSB or LSB First
	__IO uint8_t SPI_frameFormat;			//SPI Frame Format
	__IO uint8_t SPI_SlaveMgmt;		 		//SPI Slave Management (Software or Hardware Management)
	__IO uint8_t SPI_interrupts;			//SPI Interrupts Enabled
}SPI_Config_t;


//SPI Clock Polarity and Phase Relationships
#define SPI_CLOCK_POL_PHA_0	0	//Clock low idle and data capture of first bit on first edge (rising edge) [CPOL = 0, CPHA = 0].
#define SPI_CLOCK_POL_PHA_1	1	//Clock high idle and data capture of first bit on first edge (falling edge) [CPOL = 1, CPHA = 0].
#define SPI_CLOCK_POL_PHA_2	2	//Clock low idle and data capture of first bit on second edge (falling edge) [CPOL = 0, CPHA = 1].
#define SPI_CLOCK_POL_PHA_3	3	//Clock high idle and data capture of first bit on second edge (rising edge) [CPOL = 1, CPHA = 1].

//SPI Bidirectional Mode Macros
#define SPI_2LINE_UNIDIRECTIONAL_RX_ONLY_MODE	0		//Unidirectional Receive-Only Mode.
#define SPI_2LINE_UNIDIRECTIONAL_RX_TX_MODE		1		//Full Duplex Mode.
#define SPI_1LINE_BIDRECTIONAL_RX_ONLY_MODE		2		//Bidirectional Receive-Only Mode.
#define SPI_1LINE_BIDRECTIONAL_TX_ONLY_MODE		3		//Bidirectional Transmit-Only Mode.

//SPI Data Frame Format
#define SPI_8BIT_DATA_FRAME						0
#define SPI_16BIT_DATA_FRAME					1

//SPI Baud Rate Division By Pclk Macros
#define SPI_BAUD_DIV_2							0
#define SPI_BAUD_DIV_4							1
#define SPI_BAUD_DIV_8							2
#define SPI_BAUD_DIV_16							3
#define SPI_BAUD_DIV_32							4
#define SPI_BAUD_DIV_64							5
#define SPI_BAUD_DIV_128						6
#define SPI_BAUD_DIV_256						7

//SPI MSB or LSB First Macros
#define SPI_MSB_TRANSMIT_FIRST					0
#define SPI_LSB_TRANSMIT_FIRST					1

//SPI Frame Format
#define SPI_MOTOROLA_MODE						0
#define SPI_TI_MODE								1

//SPI Hardware/Software Slave Management Macros
#define SPI_HWS_MGNT_NSS_OP_ENABLED				0	/* NSS output enabled (SSM = 0, SSOE = 1)
													 * This configuration is used only when the device operates in master mode. The
													 * NSS signal is driven low when the master starts the communication and is kept
													 * low until the SPI is disabled. */
#define SPI_HWS_MGNT_NSS_OP_DISABLED			1	/* NSS output disabled (SSM = 0, SSOE = 0)
													 * This configuration allows multimaster capability for devices operating in master
													 * mode. For devices set as slave, the NSS pin acts as a classical NSS input: the
													 * slave is selected when NSS is low and deselected when NSS high. */
#define SPI_SWS_MGNT							2	//Software NSS management

//SPI Interrupts Enable Macros
#define SPI_INTERRUPT_DISABLED					0
#define SPI_INTERRUPT_ENABLED					1

//SPI DMA Enable Macros
#define SPI_DMA_DISABLED						0
#define SPI_DMA_ENABLED							1


//SPI Device Mode
#define MASTER_MODE		0
#define SLAVE_MODE		1

//SPI Flag Status
#define SPI_TX_EMPTY			(!((SPIx->SR >> SPI_SR_TXE_Pos) & 0x1U))		//1: Tx buffer empty
#define SPI_RX_NOT_EMPTY		(!((SPIx->SR >> SPI_SR_RXNE_Pos) & 0x1U))		//1: Rx buffer not empty
#define SPI_BUSY_FLAG			(((SPIx->SR >> SPI_SR_BSY_Pos) & 0x1U))			//1: SPI (or I2S) is busy in communication or Tx buffer is not empty

/*******************************************************************************************************************
* 										APIs supported by this driver											   *
*******************************************************************************************************************/

void SPI_InitMaster(SPI_TypeDef *SPIx, SPI_Config_t SPI_config);
void SPI_MasterTransmissionStartTx(SPI_TypeDef *SPIx, uint8_t *dataTX);
void SPI_MasterTransmissionStartRx(SPI_TypeDef *SPIx, uint8_t *dataRx);
void SPI_MasterTransmissionStartTxRx(SPI_TypeDef *SPIx, uint8_t *dataTX, uint8_t *dataRx);
void SPI_DisableMaster(SPI_TypeDef *SPIx, SPI_Config_t SPI_config);

void SPI_InitSlave(SPI_TypeDef *SPIx, SPI_Config_t SPI_config);
void SPI_SlaveTransmissionStartTx(SPI_TypeDef *SPIx, uint8_t *dataTX);
void SPI_SlaveTransmissionStartRx(SPI_TypeDef *SPIx, uint8_t *dataRx);
void SPI_SlaveTransmissionStartTxRx(SPI_TypeDef *SPIx, uint8_t *dataTX, uint8_t *dataRx);
void SPI_DisableSlave(SPI_TypeDef *SPIx, SPI_Config_t SPI_config);

void SPI_SetConfigurations(SPI_Config_t SPI_config);

void SPI_NSS_Enable(GPIO_TypeDef *GPIOx, uint8_t gpioPin);
void SPI_NSS_Disable(GPIO_TypeDef *GPIOx, uint8_t gpioPin);

void SPI_EnableInterrupts(SPI_TypeDef *SPIx);
void SPI_EnableDMA(SPI_TypeDef *SPIx);

void SPI_IRQHandling(SPI_TypeDef *SPIx);

#endif /* __SPI_H_ */
