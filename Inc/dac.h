#ifndef __DAC_H_
#define __DAC_H_

#include"stm32f4xx.h"

/***************** DAC Structure Definitions *******************************************/

//DACx Configuration Structure Definition
typedef struct
{
	__IO uint8_t DAC_ChaSelect;			//@DAC_Channel
	__IO uint8_t DAC_ChaX_OutBufEn;		//@DAC_ChannelX_OutputBufferEnable
	__IO uint8_t DAC_ChaX_TrigEn;		//@DAC_ChannelX_TriggerEnable
	__IO uint8_t DAC_Cha1TrigSel;		//@DAC_ChannelTriggerSelection
	__IO uint8_t DAC_Cha2TrigSel;		//@DAC_ChannelTriggerSelection
	__IO uint8_t DAC_Cha1_WaveGenEn;		//@DAC_ChannelWaveGenerationEnable
	__IO uint8_t DAC_Cha2_WaveGenEn;		//@DAC_ChannelWaveGenerationEnable
	__IO uint8_t DAC_Cha1_MA_Sel;		//@DAC_ChannelMaskAmplitudeSelector
	__IO uint8_t DAC_Cha2_MA_Sel;		//@DAC_ChannelMaskAmplitudeSelector
	__IO uint8_t DAC_ChaX_DMA_En;		//@DAC_ChannelX_DMA_Enable
	__IO uint8_t DAC_ChaX_DMA_UrEn;		//@DAC_ChannelX_DMA_UnderrunInterruptEnable
}DAC_Config_t;

/***************************************************************************************/


/***************** Macro Definitions ***************************************************/

//@DAC_Channel
#define DAC_CHANNEL_1		1
#define DAC_CHANNEL_2		2
#define DAC_BOTH_CHANNELS	3

//@DAC_ChannelX_OutputBufferEnable
#define DAC_NO_CHA_OUT_BUF_EN		0	//Both output channels will not be enabled.
#define DAC_CHA1_OUT_BUF_EN			1	//Channel 1 output buffer will be enabled.
#define DAC_CHA2_OUT_BUF_EN			2	//Channel 2 output buffer will be enabled.
#define DAC_BOTH_OUT_BUF_EN			3	//Channel 1 and 2 output buffer will be enabled.

//@DAC_ChannelX_TriggerEnable
#define DAC_CHA_1_2_TRIG_DI			0	//Both Channels will not have triggers enabled.
#define DAC_CHA1_TRIG_EN			1	//Channel 1 will have triggers enabled.
#define DAC_CHA2_TRIG_EN			2	//Channel 2 will have triggers enabled.
#define DAC_CHA_1_2_TRIG_EN			4	//Channel 1 and 2 will have triggers enabled.

//@DAC_ChannelTriggerSelection
#define DAC_TIM6_TRGO_EVENT			0	//Channel x Timer 6 TRGO event.
#define DAC_TIM8_TRGO_EVENT			1	//Channel x Timer 8 TRGO event.
#define DAC_TIM7_TRGO_EVENT			2	//Channel x Timer 7 TRGO event.
#define DAC_TIM5_TRGO_EVENT			3	//Channel x Timer 5 TRGO event.
#define DAC_TIM2_TRGO_EVENT			4	//Channel x Timer 2 TRGO event.
#define DAC_TIM4_TRGO_EVENT			5	//Channel x Timer 4 TRGO event.
#define DAC_EXTERNAL_LINE_9			6	//Channel x External line9.
#define DAC_SOFTWARE_TRIG			7	//Channel x Software trigger.

//@DAC_ChannelWaveGenerationEnable
#define DAC_WAV_GEN_DI				0	//Channel x wave generation disabled.
#define DAC_NOISE_WAV_GEN_EN		1	//Channel x noise wave generation enabled.
#define DAC_TRIANGLE_WAV_GEN_EN		2	//Channel x triangle wave generation enabled.

//@DAC_ChannelMaskAmplitudeSelector
#define DAC_UM_BIT_0_AMP_1			0	//Channel x Unmask bit0 of LFSR/ triangle amplitude equal to 1.
#define DAC_UM_BITS_1_0_AMP_3		1	//Channel x Unmask bits[1:0] of LFSR/ triangle amplitude equal to 3.
#define DAC_UM_BITS_2_0_AMP_7		2	//Channel x Unmask bits[2:0] of LFSR/ triangle amplitude equal to 7.
#define DAC_UM_BITS_3_0_AMP_15		3	//Channel x Unmask bits[3:0] of LFSR/ triangle amplitude equal to 15.
#define DAC_UM_BITS_4_0_AMP_31		4	//Channel x Unmask bits[4:0] of LFSR/ triangle amplitude equal to 31.
#define DAC_UM_BITS_5_0_AMP_63		5	//Channel x Unmask bits[5:0] of LFSR/ triangle amplitude equal to 63.
#define DAC_UM_BITS_6_0_AMP_127		6	//Channel x Unmask bits[6:0] of LFSR/ triangle amplitude equal to 127.
#define DAC_UM_BITS_7_0_AMP_255		7	//Channel x Unmask bits[7:0] of LFSR/ triangle amplitude equal to 255.
#define DAC_UM_BITS_8_0_AMP_511		8	//Channel x Unmask bits[8:0] of LFSR/ triangle amplitude equal to 511.
#define DAC_UM_BITS_9_0_AMP_1023	9	//Channel x Unmask bits[9:0] of LFSR/ triangle amplitude equal to 1023.
#define DAC_UM_BITS_10_0_AMP_2047	10	//Channel x Unmask bits[10:0] of LFSR/ triangle amplitude equal to 2047.
#define DAC_UM_BITS_11_0_AMP_4095	11	//Channel x Unmask bits[11:0] of LFSR/ triangle amplitude equal to 4095.

//@DAC_ChannelX_DMA_Enable
#define DAC_DMA_DISABLE				0	//DMA will not used for either channel.
#define DAC_CHA1_DMA_ENABLE			1	//DMA will be used for channel 1.
#define DAC_CHA2_DMA_ENABLE			2	//DMA will be used for channel 2.
#define DAC_CHA_1_2_DMA_EN			3	//DMA will be used for channels 1 and 2.

//@DAC_ChannelX_DMA_UnderrunInterruptEnable
#define DAC_DMA_UR_IT_DISABLE		0	//DMA underrun interrupts will be disabled.
#define DAC_CHA1_UR_IT_ENABLE		1	//DMA underrun interrupts will be enabled for channel 1.
#define DAC_CHA2_UR_IT_ENABLE		2	//DMA underrun interrupts will be enabled for channel 2.
#define DAC_CHA_1_2_UR_IT_EN		3	//DMA underrun interrupts will be enabled for channels 1 and 2.


/***************************************************************************************/


/***************************************************************************************/
/*                         APIs supported by this driver                               */
/*      For more information about the APIs check the function definitions             */
/***************************************************************************************/

void DAC_Init(DAC_Config_t DAC_Config);
void DAC_DeInit(void);
void DAC_PeriClockControl(uint8_t EnOrDi);

void DAC_Load8BitDataRightAlign(uint8_t channel, uint8_t data);
void DAC_Load12BitDataLeftAlign(uint8_t channel, uint16_t data);
void DAC_Load12BitDataRightAlign(uint8_t channel, uint16_t data);
void DAC_StartSoftwareTrigConv(uint8_t channel);

void DAC_ClearDMA_UnderrunFlag(uint8_t channel);

/***************************************************************************************/

#endif /* __DAC_H_ */
