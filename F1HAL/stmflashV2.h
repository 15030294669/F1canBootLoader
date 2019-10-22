#ifndef  __ST_FLASH_H
#define  __ST_FLASH_H
 
#include "stm32f10x.h"
#include "stm32f10x_flash.h"
#include "Singleton.h"

const u8 BOARD_POWER_ON_RECORD_BUFFER[]={0x01,0xAA};
const u8 BOARD_POWER_OFF_RECORD_BUFFER[]={0x02,0xAA};
const u8 PC_POWER_ON_RECORD_BUFFER[]={0x01,0xBB};
const u8 PC_POWER_OFF_RECORD_BUFFER[]={0x00,0xBB};
#define SIZE sizeof(BOARD_POWER_ON_RECORD_BUFFER)		//���鳤��

#define BOARD_POWER_STATE_FIRST_ADDRESS 0X08005000			//page 20��
#define BOARD_POWER_STATE_END_ADDRESS 0X080057FF//0X080057FF			//page 21β

#define SWITCH_TIMES_FIRST_ADDRESS 0X08005C00				//page 23��
#define SWITCH_TIMES_END_ADDRESS 0x08005FFF//0x08005FFF //0x08005C0F				//page 23β

#define POWERGOOD_STATE_FIRST_ADDRESS 0x08006400			//page 25��
#define POWERGOOD_STATE_END_ADDRESS	0x080067FF//0x080067FF				//page 25β

//////////////////////////////////////////////////////////////////////////////////////////////////////
//�û������Լ�����Ҫ����
#define STM32_FLASH_SIZE  128	 		//��ѡSTM32��FLASH������С(��λΪK)
#define STM32_FLASH_WREN 1              //ʹ��FLASHд��(0��������;1��ʹ��)
//////////////////////////////////////////////////////////////////////////////////////////////////////
#if STM32_FLASH_SIZE<256
#define STM_SECTOR_SIZE 1024 //�ֽ�
#else 
#define STM_SECTOR_SIZE	2048
#endif		 
//FLASH��ʼ��ַ
#define STM32_FLASH_BASE 0x08000000 	//STM32 FLASH����ʼ��ַ
 
class STFLASH
{
private:
	bool mUseHalfWord;//
	uint32_t mStartAddress;//
public:
	STFLASH(uint32_t startAddress=(0x08000000+1000),bool useHalfWord=true);
	u16 ReadHalfWord(u32 faddr); 
	void Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite) ;
	void Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite);	 
	void Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead) ;  
};

typedef NormalSingleton<STFLASH> flasher;
 
 
#endif












