/*
 * i2s.c - I2S feeder routines
 */

#include "i2s.h"

DMA_InitTypeDef DMA_InitStructure, DMA_InitStructure2;
uint32_t txbuf, rxbuf, szbuf;

/*
 * Init I2S channel for DMA with IRQ per block 
 */
void I2S_Block_Init(void)  
{ 
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Enable the DMA clock */
	RCC_AHB1PeriphClockCmd(AUDIO_I2S_DMA_CLOCK, ENABLE); 

	/* Configure the TX DMA Stream */
	DMA_Cmd(AUDIO_I2S_DMA_STREAM, DISABLE);
	DMA_DeInit(AUDIO_I2S_DMA_STREAM);
	/* Set the parameters to be configured */
	DMA_InitStructure.DMA_Channel = AUDIO_I2S_DMA_CHANNEL;  
	DMA_InitStructure.DMA_PeripheralBaseAddr = AUDIO_I2S_DMA_DREG;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)0;      /* This field will be configured in play function */
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_BufferSize = (uint32_t)0xFFFE;      /* This field will be configured in play function */
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; 
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;  
	DMA_Init(AUDIO_I2S_DMA_STREAM, &DMA_InitStructure);  
	
	/* Enable the I2S DMA request */
	SPI_I2S_DMACmd(CODEC_I2S, SPI_I2S_DMAReq_Tx, ENABLE);

	/* Configure the RX DMA Stream */
	DMA_Cmd(AUDIO_I2S_EXT_DMA_STREAM, DISABLE);
	DMA_DeInit(AUDIO_I2S_EXT_DMA_STREAM);
	
	/* Set the parameters to be configured */
	/* why is a separate initstructure needed here? */
	DMA_InitStructure2.DMA_Channel = AUDIO_I2S_EXT_DMA_CHANNEL;  
	DMA_InitStructure2.DMA_PeripheralBaseAddr = AUDIO_I2S_EXT_DMA_DREG;
	DMA_InitStructure2.DMA_Memory0BaseAddr = (uint32_t)0;      /* This field will be configured in play function */
	DMA_InitStructure2.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure2.DMA_BufferSize = (uint32_t)0xFFFE;      /* This field will be configured in play function */
	DMA_InitStructure2.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure2.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure2.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure2.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; 
	DMA_InitStructure2.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure2.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure2.DMA_FIFOMode = DMA_FIFOMode_Disable;         
	DMA_InitStructure2.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
	DMA_InitStructure2.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure2.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;  
	DMA_Init(AUDIO_I2S_EXT_DMA_STREAM, &DMA_InitStructure2);  
	
	/* Enable the Half & Complete DMA interrupts  */
	DMA_ITConfig(AUDIO_I2S_EXT_DMA_STREAM, DMA_IT_TC | DMA_IT_HT, ENABLE);
    
	/* I2S DMA IRQ Channel configuration */
	NVIC_EnableIRQ(AUDIO_I2S_EXT_DMA_IRQ);

	/* Enable the I2S DMA request */
	SPI_I2S_DMACmd(CODEC_I2S_EXT, SPI_I2S_DMAReq_Rx, ENABLE);

}

/**
  * @brief  Starts playing & recording audio stream from/to the audio Media.
  * @param  None
  * @retval None
  */
void I2S_Block_PlayRec(uint32_t txAddr, uint32_t rxAddr, uint32_t Size)
{
	uint32_t i;
	
	/* save for IRQ svc  */
	txbuf = txAddr;
	rxbuf = rxAddr;
	szbuf = Size;
	
	/* Configure the tx buffer address and size */
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)txAddr;
	DMA_InitStructure.DMA_BufferSize = (uint32_t)Size;

	/* Configure the DMA Stream with the new parameters */
	DMA_Init(AUDIO_I2S_DMA_STREAM, &DMA_InitStructure);

	/* Configure the rx buffer address and size */
	/* Again with the separate initstructure. Baroo?? */
	DMA_InitStructure2.DMA_Memory0BaseAddr = (uint32_t)rxAddr;
	DMA_InitStructure2.DMA_BufferSize = (uint32_t)Size;

	/* Configure the DMA Stream with the new parameters */
	DMA_Init(AUDIO_I2S_EXT_DMA_STREAM, &DMA_InitStructure2);

	/* Enable the I2S DMA Streams */
	DMA_Cmd(AUDIO_I2S_DMA_STREAM, ENABLE);   
	DMA_Cmd(AUDIO_I2S_EXT_DMA_STREAM, ENABLE);   

	/* If the I2S peripheral is still not enabled, enable it */
	if ((CODEC_I2S->I2SCFGR & 0x0400) == 0)
	{
		I2S_Cmd(CODEC_I2S, ENABLE);
	}
	if ((CODEC_I2S_EXT->I2SCFGR & 0x0400) == 0)
	{
		I2S_Cmd(CODEC_I2S_EXT, ENABLE);
	}
}

#define FLAG_RX (1 << 7) /* port B, pin 7 */
#define FLAG_TX (1 << 6) /* port B, pin 6 */

/**
  * @brief  This function handles I2S RX DMA block interrupt. 
  * @param  None
  * @retval none
  */
void DMA1_Stream3_IRQHandler(void)
{ 
	int16_t *src, *dst, sz;
	
	/* Raise activity flag */
	GPIOB->BSRRL = FLAG_RX;

	/* Transfer complete interrupt */
	if (DMA_GetFlagStatus(AUDIO_I2S_EXT_DMA_STREAM, AUDIO_I2S_EXT_DMA_FLAG_TC) != RESET)
	{
		/* Point to 2nd half of buffers */
		sz = szbuf/2;
		src = (int16_t *)(rxbuf) + sz;
		dst = (int16_t *)(txbuf) + sz;
		
		/* Handle 2nd half */  
		I2S_RX_CallBack(src, dst, sz, 0);    

		/* Clear the Interrupt flag */
		DMA_ClearFlag(AUDIO_I2S_EXT_DMA_STREAM, AUDIO_I2S_EXT_DMA_FLAG_TC);
	}

	/* Half Transfer complete interrupt */
	if (DMA_GetFlagStatus(AUDIO_I2S_EXT_DMA_STREAM, AUDIO_I2S_EXT_DMA_FLAG_HT) != RESET)
	{
		/* Point to 1st half of buffers */
		sz = szbuf/2;
		src = (int16_t *)(rxbuf);
		dst = (int16_t *)(txbuf);

		/* Handle 1st half */  
		I2S_RX_CallBack(src, dst, sz, 1);    

		/* Clear the Interrupt flag */
		DMA_ClearFlag(AUDIO_I2S_EXT_DMA_STREAM, AUDIO_I2S_EXT_DMA_FLAG_HT);    
	}
	
	/* Lower activity flag */
	GPIOB->BSRRH = FLAG_RX;
}
