//release information: 
//V1.1: open interrupt at CommonConfig(), extended TX buffer size for iapDevice.
//			bootloader before version V1.1 do not support code read back
#include "stm32f10x.h"
#include "Console.h"
#include "CommonConfig.h"
#include "iap.h"
#include "uartIapDevice.h"
#include "Timer.h"
#include "stmflash.h"
#include "powerupOption.h"
#include "LED.h"

extern const uint8_t FIRMWARE_VERSION = 0x12;
uint8_t test_buf[10] = {0,0,0,0,0,0,0,0,0,0};
// <<< Use Configuration Wizard in Context Menu >>>
// <e> RUN_IN_APP
// 	<i>Default: 0
#define RUN_IN_APP 0
// <<< end of configuration section >>>

#define FLASH_APP1_ADDR		0x08005800 

int main()
{
#if RUN_IN_APP
	SCB->VTOR = FLASH_APP1_ADDR;
#else
	SCB->VTOR = STM32_FLASH_BASE;
#endif
	CommonConfig();
	BaseTimer::Instance()->initialize();
	
	iapDevice.open();
	BaseTimer::Instance()->delay_ms(50);
	iapDevice.runReceiver();
#if !RUN_IN_APP
	if(iapDevice.data_in_read_buf() > 1)
	{
		iapDevice.runReceiver();
		iapDevice.read(test_buf, 2);
		if(test_buf[0] == 0x7F && test_buf[1] == 0x7F)
		{
			Console::Instance()->printf("Get into bootloader by emergency mode.\r\n");
			pvf::write(pvf::VAR_BOOT_OPTI, BOOT_PARAM_BL);
			BaseTimer::Instance()->delay_ms(50);
			NVIC_SystemReset();
		}else if(BOOT_PARAM_APP == pvf::read(pvf::VAR_BOOT_OPTI))
		{
			Console::Instance()->printf("iapDevice have data but not 7F and param is App, so get into APP.\r\n");
			iap_load_app(FLASH_APP1_ADDR);
		}
		
	}else if(BOOT_PARAM_APP == pvf::read(pvf::VAR_BOOT_OPTI))
	{
		Console::Instance()->printf("Get into APP.\r\n");
		iap_load_app(FLASH_APP1_ADDR);
	}
	
#endif

#if RUN_IN_APP
	BaseTimer::Instance()->delay_ms(1000);
	Console::Instance()->printf("\r\n Bootloader flasher start!\r\n");
#else	
	if(BOOT_PARAM_BL == pvf::read(pvf::VAR_BOOT_OPTI))
	{
		Console::Instance()->printf("\r\n Bootloader start!\r\n");
	}else
	{
		Console::Instance()->printf("\r\n Boot paramter error: 0x%X, now write bootparam\r\n", pvf::read(pvf::VAR_BOOT_OPTI));
		pvf::write(pvf::VAR_BOOT_OPTI, BOOT_PARAM_BL);
		Console::Instance()->printf("\r\n Boot paramter now: 0x%X\r\n", pvf::read(pvf::VAR_BOOT_OPTI));
	}
#endif
	Console::Instance()->printf("Firmware virsion: V%d.%d\r\n", FIRMWARE_VERSION>>4, FIRMWARE_VERSION&0xF);
	//InitWatchDog(500);
	while(1)
	{
		Console::Instance()->runTransmitter();
		iapDevice.run();
		iap_run();
		LED_Run();
	}
	//return 0;
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
		Console::Instance()->runTransmitter();
		if(loopTimer.isAbsoluteTimeUp())
		{
			Console::Instance()->printf("Wrong parameters value: file %s on line %d\r\n", file, line);
		}
  }
}
