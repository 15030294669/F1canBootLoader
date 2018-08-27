/********************************************************************************
* @file    CSpiConsole.cpp
* @author  Wuzeling
* @version V1.0
* @date    2018-08-14
* @brief   this file defines the Console function that can printf with spi.
*	   This is for STM32F1.
********************************************************************************/
#include "CSpiConsole.h"
#include "Timer.h"
#include <cstring>
namespace{
	uint8_t sendData;
	Timer rxTimer(100,100);
}

CSpiConsole::CSpiConsole()
#if STM32_CONSOLE_SPI == 1
	:_SPI(SPI1)
#elif STM32_CONSOLE_SPI == 2
	:_SPI(SPI2)
#else 
#	error
#endif
{
	_inited = false;
}



bool CSpiConsole::open()
{
	SPI_InitTypeDef  SPI_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
#if STM32_CONSOLE_SPI == 1
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 |GPIO_Pin_5 | GPIO_Pin_6; 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
#elif STM32_CONSOLE_SPI == 2
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB , ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI2, ENABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  
  GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
#else 
#	error
#endif
	
	SPI_Cmd(_SPI, DISABLE);   //����ǰ�ȹر�SPI
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //ȫ˫������ģʽ��
  SPI_InitStructure.SPI_Mode = SPI_Mode_Slave;      //����SPI1Ϊ��ģʽ��
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;  //����λ16λ
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;  //����ʱ��Ϊ�ߣ�DSP�Ǳ�Ҳ��Ϊ�ߡ�
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge; //ʱ����λ�������ڵ�2�������ر��ɼ�
  SPI_InitStructure.SPI_NSS = SPI_NSS_Hard;     //CS����Ϊ��ģʽ����ͨ���������Ƭѡ�š� 
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;  //256��ƵΪ�����ʣ���Ϊ���������������ṩ�ġ���������������û�����塣
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;//�ȴ����ֽڣ���ΪDSPֻ�и��ֽڴ������ַ�ʽ����������Ҫ����Ϊ���ֽ���ǰ����Ȼ�����ˡ�
  SPI_InitStructure.SPI_CRCPolynomial = 7;  //CRC����ʽ�����ã�Ĭ�ϡ�
  SPI_Init(_SPI, &SPI_InitStructure);

  SPI_Cmd(_SPI, ENABLE);   /* ʹ�� SPI1  */
	_inited = true;
  
}

/**
  * @brief  get free size
  * @param  None
  * @retval None
  */
uint16_t CSpiConsole::getFreeSize()
{
	return 1;
}

void CSpiConsole::runTransmitter()
{ 
	/* Loop while DR register in not emplty */  
	if(SPI_I2S_GetFlagStatus(_SPI, SPI_I2S_FLAG_TXE) == RESET) return;
		
	//SPI_I2S_SendData(_SPI, _txBuf[i]);    
	SPI_I2S_SendData(_SPI, sendData);
		
	sendData = 0xFF;
	/* Wait to receive a byte */  
	while(SPI_I2S_GetFlagStatus(_SPI, SPI_I2S_FLAG_RXNE) == RESET)
	{
		if(rxTimer.isAbsoluteTimeUp())
			return;
	}
	
	/* Return the byte read from the SPI bus */  
	SPI_I2S_ReceiveData(_SPI);
}

uint16_t CSpiConsole::write(uint8_t* srcBuf, uint16_t srcLen)
{
	uint16_t writeLen = getFreeSize();
	if(srcLen < writeLen)
		writeLen = srcLen;
	
	sendData = srcBuf[0];
	return writeLen;
}

bool CSpiConsole::isIdel()
{
	if(SPI_I2S_GetFlagStatus(_SPI, SPI_I2S_FLAG_TXE) == RESET) return false;
	return true;
}
