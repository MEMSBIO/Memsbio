#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "lcd.h"
#include "I2C.h"
#include "timer.h"
#define start 255
#define end 254
uint16_t counter = 0;

int main(void)
 { 
	double  R;
	uint16_t  data[3] = {0,0,0};
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	//LED_Init();
	delay_init();	    	 //��ʱ������ʼ��	  
	uart_init(9600);	 	//���ڳ�ʼ��Ϊ9600
	InitAD5933();       ////C12 SDA  C11 SCL
	//TIM3_Int_Init(100,7199);//10Khz�ļ���Ƶ�ʣ�������100Ϊ10ms����10ms�ж�һ��
//	LED0=0;
//	LED1=0;
	
  while(1) 
	{		
		counter = 0;
		R = AD5933_display()/10; //10��
		if(R>999999){
			data[0] = 100;
			data[1] = 0;
			data[2] = 0;
		}
		else{
			data[0] = R/10000;
			data[1] = (R-(R/10000)*10000)/100;
			data[1] = (R-data[0]*10000)/100;
			data[2] = (int)R % 100;
		}
//			data[0] = 1;
//			data[1] = 2;
//			data[2] = 3;
			//data[2]++;
		  //data[2] = data[2] % 100 ;
		
		USART_SendData(USART1,start);
		delay_ms(1);
		USART_SendData(USART1,data[0]);
		delay_ms(1);
		USART_SendData(USART1,data[1]);
		delay_ms(1);
		USART_SendData(USART1,data[2]);
		delay_ms(1);
		USART_SendData(USART1,counter);
		delay_ms(1);
		USART_SendData(USART1,end);
	} 
}
