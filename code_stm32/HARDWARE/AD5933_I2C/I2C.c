#include "I2C.h"
#include "delay.h"
#include "lcd.h"
#include "math.h"
#include <stdio.h>
#define Delay_IIC 1
#define AD5933 1
#define AD5933_MCLK 16.776  //=536870912/MCLK;
//#define	AD5933_MCLK_USE_OUT	1	//0�ڲ�ʱ��  1�ⲿʱ��
// 1khz��ʼƵ�� #define AD5933_Correction 	1829536762.935	//95852505.30789	//  101615461.47044108  97205308.46��30khz�� 96911637.13��10khz�� 96952505.30789��1k��
#define AD5933_Correction 	(1829536762.935*1.01)
//#define AD5933_Correction 1829536762.935
//float Gain=5.015e-04;

void Ini_I2c( void )      //��ʼ��I2C
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOC, ENABLE );	
	   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11|GPIO_Pin_12;  //C12 SDA  C11 SCL
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_SetBits(GPIOC,GPIO_Pin_11|GPIO_Pin_12); 	//PB10,PB11 �����  
    return;
}
void NOPS(void) 
{
	delay_us(20);
} 
void SDA_1( void )
{
//    SDA_OUT();		//��SDA����Ϊ���ģʽ
    SDA=1;		//SDA�ܽ����Ϊ�ߵ�ƽ    
    NOPS();
    return;
}

void SDA_0 ( void )
{
//    SDA_OUT();		//��SDA����Ϊ���ģʽ
    SDA=0;	//SDA�ܽ����Ϊ�͵�ƽ    
    NOPS();
    return;
}

void SCL_1( void )
{
     		//��SCL����Ϊ���ģʽ
    SCL=1;		//SCL�ܽ����Ϊ�ߵ�ƽ    
    NOPS();
    return;
}

void SCL_0 ( void )
{
    		//��SCL����Ϊ���ģʽ
    SCL=0;	//SCL�ܽ����Ϊ�͵�ƽ    
    NOPS();
    return;
}

void GetACK(void)
{   
	u8 ucErrTime=0;
	SDA_IN();      //SDA����Ϊ����  
	SDA=1;delay_us(Delay_IIC*1);	   
	SCL=1;delay_us(Delay_IIC*1);	 
	while(READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			STOP();
			return;
		}
	}
	SCL=0;//ʱ�����0 	
}

void SendNACK(void)
{
	SCL=0;
	SDA_OUT();
	SDA=1;
	delay_us(Delay_IIC*2);
	SCL=1;
	delay_us(Delay_IIC*2);
	SCL=0;
}
void START(void)    // ������������	
{
	SDA_OUT();     //sda�����
	SDA=1;	  	  
	SCL=1;
	delay_us(Delay_IIC*4);
 	SDA=0;//START:when CLK is high,DATA change form high to low 
	delay_us(Delay_IIC*4);
	SCL=0;//ǯסI2C���ߣ�׼�����ͻ�������� 
}

void STOP(void)
{
	SDA_OUT();//sda�����
	SCL=0;
	SDA=0;//STOP:when CLK is high DATA change form low to high
 	delay_us(Delay_IIC*4);
	SCL=1; 
	SDA=1;//����I2C���߽����ź�
	delay_us(Delay_IIC*4);		
}

void SendByte(u8 txd)	// ����һ���ֽ������Ӻ��� 
{
    u8 t;   
	SDA_OUT(); 	    
    SCL=0;//����ʱ�ӿ�ʼ���ݴ���
    for(t=0;t<8;t++)
    {              
        SDA=(txd&0x80)>>7;
        txd<<=1; 	  
		delay_us(Delay_IIC*2);   //��TEA5767��������ʱ���Ǳ����
		SCL=1;
		delay_us(Delay_IIC*2); 
		SCL=0;	
		delay_us(Delay_IIC*2);
    }	 
}

u8 ReadByte(void)  //��һ���ֽ�����
{
	unsigned char i,receive=0;
	SDA_IN();//SDA����Ϊ����
    for(i=0;i<8;i++ )
	{
        SCL=0; 
        delay_us(Delay_IIC*2);
		SCL=1;
        receive<<=1;
        if(READ_SDA)receive++;   
		delay_us(Delay_IIC*1); 
    }					 
		SendNACK();
    return receive;
}
void Write_Byte(char nAddr,uint nValue)//nAddr��д���ֽ�nValue
{   
	int nTemp = 0x1A;      // AD5933��Ĭ�ϵ�ַ&д����λ���ͣ�
  START(); 
	SendByte(nTemp);     // ���͵�ַ	
	GetACK();   
	
	SendByte(nAddr);     // ���͵�ַ�ֽ�
	GetACK();
	
	SendByte(nValue);	// ���������ֽ�	
	GetACK();
		
	STOP();	// ֹͣ����			
    	return;
}

void SetPointer(char nAddr)  //   ���õ�ַָ��
{          
	int nTemp = 0x1A;      // AD5933��Ĭ�ϵ�ַ&д����λ���ͣ�
	
  START(); 
	SendByte(nTemp);     // ���͵�ַ	
	GetACK();     // �ȴ� ACK		

	SendByte(0xB0);     // ����ָ������1101 0000
	GetACK();

	SendByte(nAddr);	// ���͵�ַָ��	
	GetACK();	

	STOP();	// ֹͣ����			
   	return;
}

int Rece_Byte(char nAddr)//��ȡnAddr�е��ֽڵ�����ֵ
{   
	int nTemp ;    
  SetPointer(nAddr);   //��ַָ��ָ��nAddr
  nTemp=0x1B;      // AD5933��Ĭ�ϵ�ַ&������λ���ߣ�
  START(); 

	SendByte(nTemp);     // ���͵�ַ	
	GetACK();  

	nTemp=ReadByte();//��һ���ֽ�����		
	SendNACK();//����NO_ACK
	
	STOP();	// ֹͣ����			
	return nTemp;
}
void InitAD5933(void)
{
	Ini_I2c();    //��ʼ��I2C
	
//	Write_Byte(0x82,0x50);//��ʼƵ�ʼĴ��� 40khz
//	delay_ms(5);
//	Write_Byte(0x83,0x00);
//	delay_ms(5);
//	Write_Byte(0x84,0x00);
//	delay_ms(5);
	
//	Write_Byte(0x82,0x0f);//��ʼƵ�ʼĴ��� 30khz
//	delay_ms(5);
//	Write_Byte(0x83,0x5c);
//	delay_ms(5);
//	Write_Byte(0x84,0x28);
//	delay_ms(5);
	
//	Write_Byte(0x82,0x28);//��ʼƵ�ʼĴ��� 20khz
//	delay_ms(5);
//	Write_Byte(0x83,0x00);
//	delay_ms(5);
//	Write_Byte(0x84,0x00);
//	delay_ms(5);
	
//	Write_Byte(0x82,0x05);//��ʼƵ�ʼĴ��� 10khz
//	delay_ms(5);
//	Write_Byte(0x83,0x1e);
//	delay_ms(5);
//	Write_Byte(0x84,0xb8);
//	delay_ms(5);
	
//	Write_Byte(0x82,0x0a);//��ʼƵ�ʼĴ��� 5khz
//	delay_ms(5);
//	Write_Byte(0x83,0x00);
//	delay_ms(5);
//	Write_Byte(0x84,0x00);
//	delay_ms(5);

//	Write_Byte(0x82,0x08);//��ʼƵ�ʼĴ��� 4khz
//	delay_ms(5);
//	Write_Byte(0x83,0x00);
//	delay_ms(5);
//	Write_Byte(0x84,0x00);
//	delay_ms(5);

//	Write_Byte(0x82,0x04);//��ʼƵ�ʼĴ��� 2khz
//	delay_ms(5);
//	Write_Byte(0x83,0x00);
//	delay_ms(5);
//	Write_Byte(0x84,0x00);
//	delay_ms(5);

//	Write_Byte(0x82,0x02);//��ʼƵ�ʼĴ��� 1khz
//	delay_ms(5);
//	Write_Byte(0x83,0x00);
//	delay_ms(5);
//	Write_Byte(0x84,0x00);
//	delay_ms(5);
	
	Write_Byte(0x82,0x01);//��ʼƵ�ʼĴ��� 500hz
	delay_ms(5);
	Write_Byte(0x83,0x00);
	delay_ms(5);
	Write_Byte(0x84,0x00);
	delay_ms(5);
	
	Write_Byte(0x85,0x00);//Ƶ�������Ĵ���
	delay_ms(5);
	Write_Byte(0x86,0x00);
	delay_ms(5);
	Write_Byte(0x87,0x00);
	delay_ms(5);
	
	Write_Byte(0x88,0x00);//�������Ĵ���
	delay_ms(5);
	Write_Byte(0x89,0x00);
	delay_ms(5);
	
	Write_Byte(0x80,0xb1);//��׼ģʽ ����ģʽ
	delay_ms(5);
	Write_Byte(0x81,0x00);//ѡ���ڲ�ʱ��
	delay_ms(5);
	Write_Byte(0x81,0x10);//��λAD5933
	delay_ms(5);
	
	//Write_Byte(0x80,0x00);
	Write_Byte(0x80,0x11);//��λ����PGA��������Ϊx1 ��λ��0x11������ʼƵ�ʳ�ʼ�� 0x41:�ظ�Ƶ�ʣ������ѹֵ��Χ������ֵ2.0V p-p
	delay_ms(100);
	Write_Byte(0x8A,0x03);//����ʱ��������
	Write_Byte(0x8B,0xff);
	Write_Byte(0x80,0x21);//����Ƶ��ɨ��
	delay_ms(5);
}

double AD5933_display(void)
{
	long ReadTemp,realArr[3],imageArr[3];
  double magnitude;  
	double resistance;
	
	while(1)
  {
    ReadTemp=Rece_Byte(0x8F);  //��ȡ״̬�Ĵ������DFT�Ƿ����
		delay_ms(5);
//	   ReadTemp=ReadTemp&0x07;
    if (ReadTemp&0x02)
        break;
  }                  
    realArr[0]=Rece_Byte(0x94);
    realArr[1]=Rece_Byte(0x95);
    realArr[2]=realArr[0]*0x100+realArr[1];
                
    imageArr[0]=Rece_Byte(0x96);
    imageArr[1]=Rece_Byte(0x97);
    imageArr[2]=imageArr[0]*0x100+imageArr[1];      
                
	  if (realArr[2]>=0x8000)  //����ʵ����ԭ��(������λ�⣬ȡ����һ)
		{
      realArr[2]^=0xFFFF; 
      realArr[2]^=0x8000; 
		  realArr[2]+=1;
      realArr[2]^=0x8000;
		}
		if (imageArr[2]>=0x8000)  //�����鲿��ԭ��(������λ�⣬ȡ����һ)
		{
      imageArr[2]^=0xFFFF; 
      imageArr[2]^=0x8000; 
			imageArr[2]+=1;
      imageArr[2]^=0x8000;
		}
    magnitude=sqrt(realArr[2]*realArr[2]+imageArr[2]*imageArr[2]);  //ģֵ���� 
		resistance=AD5933_Correction/magnitude;	//resistanceΪfloat
		Write_Byte(0x80,0x41);//�ظ�ģʽ�������ѹ����ֵΪ2.0V p-p,PAG����1x
		delay_ms(5);
			
			
		return resistance;
}

