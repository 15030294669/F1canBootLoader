//release information:
//V1.1: open interrupt at CommonConfig(), extended TX buffer size for iapDevice.
//			bootloader before version V1.1 do not support code read back
#include "stm32f10x.h"
#include "Console.h"
#include "CommonConfig.h"
#include "CCan.h"
#include "uart5IapDevice.h"
#include "Timer.h"

#include "HeartLed.h"
#include "logic_out.h"
#include "key_monitor.h"

#include <string.h>
#include <stdio.h>

#include "exti_monitor.h"

extern const uint8_t FIRMWARE_VERSION = 0x12;

/*************************************************************************/
#include "power_state_data.h"
#include "power_process.h"
#include "power_state_recoder_manager.h"
#include "spi.h"
#include "SEGGER_RTT.h"

/*************************************************************************/
const uint8_t SPI_TX_BUFFER_SIZE = 10;
const bool DEBUG = 1;
uint8_t spi_tx_buffer_pointer = 0;

int main()
{
	
	CommonConfig();
	
	BaseTimer::Instance()->initialize();
	
	key_init();							//������ť��ʼ��
	
	PSGlobalData::Instance()->is_close_exti_interrupt = true;	//��power_good�ⲿ�жϼ��
	
	EXTI_GPIO_Init();					//�ⲿ�жϼ���ʼ��																												
	EXTI_Init();
	
	logic_pin_config();					//��ؿ��ء�PC������������ʹ�ܼ��ĳ�ʼ��
	
	Initial_HeartLED();					//�����Ƶĳ�ʼ����500ms��תһ��
	
	spiInit();							//SPI��ʼ��

	traverseLastPostionAndStateInFlash(BOARD_POWER_STATE_FIRST_ADDRESS, BOARD_POWER_STATE_END_ADDRESS, PSGlobalData::Instance()->board_power_state_address_now);	//����board�������һ���ֽڵ�λ��
	
	traverseLastPostionAndStateInFlash(SWITCH_TIMES_FIRST_ADDRESS, SWITCH_TIMES_END_ADDRESS, PSGlobalData::Instance()->switch_times_address_now);					//�������ػ������������һ���ֽڵ�λ��
	
	traverseLastPostionAndStateInFlash(POWERGOOD_STATE_FIRST_ADDRESS, POWERGOOD_STATE_END_ADDRESS, PSGlobalData::Instance()->power_good_state_address_now);			//����power_good�������һ���ֽڵ�λ��
	
	/***************************************************�״ο��ػ������ĳ�ʼֵ�趨**********************************************************************/
	if(PSGlobalData::Instance()->switch_times_address_now == SWITCH_TIMES_FIRST_ADDRESS)			//��ǰ��ַΪ�׵�ַ
	{
		PSGlobalData::Instance()->switch_times_h = 0x00;
		PSGlobalData::Instance()->switch_times_l = 0x00;
	}
	else	readLastSwitchTimes(PSGlobalData::Instance()->switch_times_address_now, PSGlobalData::Instance()->switch_times_h, PSGlobalData::Instance()->switch_times_l);
	PSGlobalData::Instance()->switch_times = ((uint16_t)PSGlobalData::Instance()->switch_times_h << 8) + (uint16_t)PSGlobalData::Instance()->switch_times_l;
	/****************************************************************************************************************************************************/
	
	Timer spiTrasferTimer(2,2);		//SPI���ͼ����ʱ��

	while(1)
	{
		powerProcess();				//��Դ����
		HeartLed_Run();		
		if(spiTrasferTimer.isAbsoluteTimeUp())
		{
			/**************************************SPI����****************************************/
			PSGlobalData::Instance()->arrary[0] = 0xA5;					//֡ͷ
			PSGlobalData::Instance()->arrary[9] = 0x5A;					//֡β
			spiSendByte(PSGlobalData::Instance()->arrary[spi_tx_buffer_pointer]);	//���ֽڷ���
			spi_tx_buffer_pointer++;
			if (spi_tx_buffer_pointer >= SPI_TX_BUFFER_SIZE) spi_tx_buffer_pointer = 0;
			/*************************************************************************************/
			if(DEBUG)
			{
				SEGGER_RTT_printf(0,"\r\n board_power_state_address_now:0x%08x \r\n", PSGlobalData::Instance()->board_power_state_address_now);
				SEGGER_RTT_printf(0,"\r\n switch_times_address_now:0x%08x \r\n", PSGlobalData::Instance()->switch_times_address_now);
				SEGGER_RTT_printf(0,"\r\n power_good_state_address_now:0x%08x \r\n", PSGlobalData::Instance()->power_good_state_address_now);
				SEGGER_RTT_printf(0, "\r\n 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x \r\n", PSGlobalData::Instance()->arrary[0], 
						PSGlobalData::Instance()->arrary[1],PSGlobalData::Instance()->arrary[2],PSGlobalData::Instance()->arrary[3],PSGlobalData::Instance()->arrary[4],
						PSGlobalData::Instance()->arrary[5],PSGlobalData::Instance()->arrary[6],PSGlobalData::Instance()->arrary[7],PSGlobalData::Instance()->arrary[8],PSGlobalData::Instance()->arrary[9]);
				SEGGER_RTT_printf(0, "\r\n Firmvision is 0x%02x \r\n", FIRMWARE_VERSION);
			}
		}
	}
	return 0;
}

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	Timer loopTimer(1000,1000);

  /* Infinite loop */
  while (1)
  {
		//Console::Instance()->runTransmitter();
		if(loopTimer.isAbsoluteTimeUp())
		{
			SEGGER_RTT_printf(0, "Wrong parameters value: file %s on line %d\r\n", file, line);
		}
  }
}


