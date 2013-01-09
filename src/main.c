/**
  ******************************************************************************
  * @file     main.c
  * @author  Martin Howse
  * @version
  * @date    january 2013
  * @brief   the dark interpreter
  ******************************************************************************
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307, USA.
  */

#include "main.h"
#include "stm32f4_discovery.h"

uint16_t audiobuff[BUFF_LEN] = {0};  // buff len is 384

__IO uint16_t 			ADC3ConvertedValue = 0;
RCC_ClocksTypeDef       RCC_Clocks;
GPIO_InitTypeDef        GPIO_InitStructure;
uint8_t                 state = OFF;
__IO uint32_t 	TimingDelay = 50;

void ADC3_CH12_DMA_Config(void);
void randfill(uint16_t offset, uint16_t len);


int main(void)
{
  STM_EVAL_LEDInit(LED3); // orange LED
  STM_EVAL_LEDInit(LED4); // green LED
  STM_EVAL_LEDInit(LED5); // red LED
  STM_EVAL_LEDInit(LED6); // blue LED
  STM_EVAL_LEDOn(LED4);

  /* Initialize User Button */
  STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_GPIO);

  /* Initialise the onboard random number generator ! */
  RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, ENABLE);
  RNG_Cmd(ENABLE);

  /* ADC3 configuration *******************************************************/
   /*  - Enable peripheral clocks                                              */
   /*  - DMA2_Stream0 channel2 configuration                                   */
   /*  - Configure ADC Channel12 pin as analog input  : PC2                    */
   /*  - Configure ADC3 Channel12                                              */
   ADC3_CH12_DMA_Config();

   /* Start ADC3 Software Conversion */
   ADC_SoftwareStartConv(ADC3);

   /***************************************************************************/

  EVAL_AUDIO_Init( OUTPUT_DEVICE_AUTO, VOL, SAMPLERATE);
  EVAL_AUDIO_Play((uint16_t*)audiobuff, BUFF_LEN);


  while (1)
  {
    //Interrupts handlers are doing their job...;
  }
}

/**
  * @brief  Basic management of the timeout situation.
  * @param  None
  * @retval None
  */
uint32_t Codec_TIMEOUT_UserCallback(void)
{
	STM_EVAL_LEDOn(LED5); /*  alert : red LED !  */
	return (0);
}
//---------------------------------------------------------------------------
/**
* @brief  Manages the DMA Half Transfer complete interrupt.
* @param  None
* @retval None
*/
void EVAL_AUDIO_HalfTransfer_CallBack(uint32_t pBuffer, uint32_t Size)
{
  /* Generally this interrupt routine is used to load the buffer when
  a streaming scheme is used: When first Half buffer is already transferred load
  the new data to the first half of buffer while DMA is transferring data from
  the second half. And when Transfer complete occurs, load the second half of
  the buffer while the DMA is transferring from the first half ... */

  //	sawtooth_runproc(0, BUFF_LEN_DIV4);

  randfill(0, BUFF_LEN_DIV4);

}

//---------------------------------------------------------------------------
/**
* @brief  Manages the DMA Complete Transfer complete interrupt.
* @param  None
* @retval None
*/
void EVAL_AUDIO_TransferComplete_CallBack(uint32_t pBuffer, uint32_t Size)
{

  //	sawtooth_runproc(BUFF_LEN_DIV2, BUFF_LEN_DIV4);
  randfill(BUFF_LEN_DIV2, BUFF_LEN_DIV4);

}
//---------------------------------------------------------------------------
/**
* @brief  Get next data sample callback
* @param  None
* @retval Next data sample to be sent
*/
uint16_t EVAL_AUDIO_GetSampleCallBack(void)
{
  return 0;
}

#define HTONS(A)  ((((u16)(A) & 0xff00) >> 8) | \
                   (((u16)(A) & 0x00ff) << 8))


void randfill(uint16_t offset, uint16_t len){

  uint16_t  *outp;
  uint16_t value;

  outp    = audiobuff + offset;

  do {
    //    value=randomNum();
    //    value = (uint16_t)((int16_t)((32767.0f) * y ));

    //    value = (uint16_t)((int16_t)((ADC3ConvertedValue<<4)-32768));
    value = ADC3ConvertedValue<<4;
    *outp++ = value; // left channel sample
    *outp++ = 0; // right channel sample
    len-=2;
  }
    while (len);

}

//---------------------------------------------------------------------------
/**************
* returns a random float between 0 and 1
*****************/
uint16_t randomNum(void)
  {
    //    static float x;
    //    x+=0.001f;
        float random = 1.0f;
    if (RNG_GetFlagStatus(RNG_FLAG_DRDY) == SET)
    {
      //      random = (float)(RNG_GetRandomNumber()/4294967294.0f)-1.0f;

      random = RNG_GetRandomNumber()/131072;
    }
    return random;
    //    if (x>1) x=-1.0f;
    //    return x;
  }
//---------------------------------------------------------------------------
/**
  * @brief  ADC3 channel12 with DMA configuration
  * @param  None
  * @retval None
  */
void ADC3_CH12_DMA_Config(void)
{
  ADC_InitTypeDef       ADC_InitStructure;
  ADC_CommonInitTypeDef ADC_CommonInitStructure;
  DMA_InitTypeDef       DMA_InitStructure;
  GPIO_InitTypeDef      GPIO_InitStructure;

  /* Enable ADC3, DMA2 and GPIO clocks ****************************************/
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2 | RCC_AHB1Periph_GPIOC, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);

  /* DMA2 Stream0 channel0 configuration **************************************/
  DMA_InitStructure.DMA_Channel = DMA_Channel_2;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC3_DR_ADDRESS;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&ADC3ConvertedValue;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = 1;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA2_Stream0, &DMA_InitStructure);
  DMA_Cmd(DMA2_Stream0, ENABLE);

  /* Configure ADC3 Channel12 pin as analog input ******************************/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  /* ADC Common Init **********************************************************/
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div8;
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
  ADC_CommonInit(&ADC_CommonInitStructure);

  /* ADC3 Init ****************************************************************/
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion = 1;
  ADC_Init(ADC3, &ADC_InitStructure);

  /* ADC3 regular channel12 configuration *************************************/
  ADC_RegularChannelConfig(ADC3, ADC_Channel_12, 1, ADC_SampleTime_480Cycles);

 /* Enable DMA request after last transfer (Single-ADC mode) */
  ADC_DMARequestAfterLastTransferCmd(ADC3, ENABLE);

  /* Enable ADC3 DMA */
  ADC_DMACmd(ADC3, ENABLE);

  /* Enable ADC3 */
  ADC_Cmd(ADC3, ENABLE);
}
