#include "I2C.h"
#include "delay.h"
#include "lcd.h"
#include "math.h"
#include <stdio.h>
#define Delay_IIC 1
#define AD5933 1
#define AD5933_MCLK 16.776  //=536870912/MCLK;
//#define	AD5933_MCLK_USE_OUT	1	//0内部时钟  1外部时钟
// 1khz起始频率 #define AD5933_Correction 	1829536762.935	//95852505.30789	//  101615461.47044108  97205308.46（30khz） 96911637.13（10khz） 96952505.30789（1k）
#define AD5933_Correction 	(1829536762.935*1.01)
//#define AD5933_Correction 1829536762.935
//float Gain=5.015e-04;

void Ini_I2c( void )      //初始化I2C
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOC, ENABLE );	
	   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11|GPIO_Pin_12;  //C12 SDA  C11 SCL
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_SetBits(GPIOC,GPIO_Pin_11|GPIO_Pin_12); 	//PB10,PB11 输出高  
    return;
}
void NOPS(void) 
{
	delay_us(20);
} 
void SDA_1( void )
{
//    SDA_OUT();		//将SDA设置为输出模式
    SDA=1;		//SDA管脚输出为高电平    
    NOPS();
    return;
}

void SDA_0 ( void )
{
//    SDA_OUT();		//将SDA设置为输出模式
    SDA=0;	//SDA管脚输出为低电平    
    NOPS();
    return;
}

void SCL_1( void )
{
     		//将SCL设置为输出模式
    SCL=1;		//SCL管脚输出为高电平    
    NOPS();
    return;
}

void SCL_0 ( void )
{
    		//将SCL设置为输出模式
    SCL=0;	//SCL管脚输出为低电平    
    NOPS();
    return;
}

void GetACK(void)
{   
	u8 ucErrTime=0;
	SDA_IN();      //SDA设置为输入  
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
	SCL=0;//时钟输出0 	
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
void START(void)    // 启动数据总线	
{
	SDA_OUT();     //sda线输出
	SDA=1;	  	  
	SCL=1;
	delay_us(Delay_IIC*4);
 	SDA=0;//START:when CLK is high,DATA change form high to low 
	delay_us(Delay_IIC*4);
	SCL=0;//钳住I2C总线，准备发送或接收数据 
}

void STOP(void)
{
	SDA_OUT();//sda线输出
	SCL=0;
	SDA=0;//STOP:when CLK is high DATA change form low to high
 	delay_us(Delay_IIC*4);
	SCL=1; 
	SDA=1;//发送I2C总线结束信号
	delay_us(Delay_IIC*4);		
}

void SendByte(u8 txd)	// 发送一个字节数据子函数 
{
    u8 t;   
	SDA_OUT(); 	    
    SCL=0;//拉低时钟开始数据传输
    for(t=0;t<8;t++)
    {              
        SDA=(txd&0x80)>>7;
        txd<<=1; 	  
		delay_us(Delay_IIC*2);   //对TEA5767这三个延时都是必须的
		SCL=1;
		delay_us(Delay_IIC*2); 
		SCL=0;	
		delay_us(Delay_IIC*2);
    }	 
}

u8 ReadByte(void)  //读一个字节数据
{
	unsigned char i,receive=0;
	SDA_IN();//SDA设置为输入
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
void Write_Byte(char nAddr,uint nValue)//nAddr中写入字节nValue
{   
	int nTemp = 0x1A;      // AD5933的默认地址&写控制位（低）
  START(); 
	SendByte(nTemp);     // 发送地址	
	GetACK();   
	
	SendByte(nAddr);     // 发送地址字节
	GetACK();
	
	SendByte(nValue);	// 发送数据字节	
	GetACK();
		
	STOP();	// 停止总线			
    	return;
}

void SetPointer(char nAddr)  //   设置地址指针
{          
	int nTemp = 0x1A;      // AD5933的默认地址&写控制位（低）
	
  START(); 
	SendByte(nTemp);     // 发送地址	
	GetACK();     // 等待 ACK		

	SendByte(0xB0);     // 发送指针命令1101 0000
	GetACK();

	SendByte(nAddr);	// 发送地址指针	
	GetACK();	

	STOP();	// 停止总线			
   	return;
}

int Rece_Byte(char nAddr)//读取nAddr中的字节到返回值
{   
	int nTemp ;    
  SetPointer(nAddr);   //地址指针指向nAddr
  nTemp=0x1B;      // AD5933的默认地址&读控制位（高）
  START(); 

	SendByte(nTemp);     // 发送地址	
	GetACK();  

	nTemp=ReadByte();//读一个字节数据		
	SendNACK();//发送NO_ACK
	
	STOP();	// 停止总线			
	return nTemp;
}
void InitAD5933(void)
{
	Ini_I2c();    //初始化I2C
	
//	Write_Byte(0x82,0x50);//起始频率寄存器 40khz
//	delay_ms(5);
//	Write_Byte(0x83,0x00);
//	delay_ms(5);
//	Write_Byte(0x84,0x00);
//	delay_ms(5);
	
//	Write_Byte(0x82,0x0f);//起始频率寄存器 30khz
//	delay_ms(5);
//	Write_Byte(0x83,0x5c);
//	delay_ms(5);
//	Write_Byte(0x84,0x28);
//	delay_ms(5);
	
//	Write_Byte(0x82,0x28);//起始频率寄存器 20khz
//	delay_ms(5);
//	Write_Byte(0x83,0x00);
//	delay_ms(5);
//	Write_Byte(0x84,0x00);
//	delay_ms(5);
	
//	Write_Byte(0x82,0x05);//起始频率寄存器 10khz
//	delay_ms(5);
//	Write_Byte(0x83,0x1e);
//	delay_ms(5);
//	Write_Byte(0x84,0xb8);
//	delay_ms(5);
	
//	Write_Byte(0x82,0x0a);//起始频率寄存器 5khz
//	delay_ms(5);
//	Write_Byte(0x83,0x00);
//	delay_ms(5);
//	Write_Byte(0x84,0x00);
//	delay_ms(5);

//	Write_Byte(0x82,0x08);//起始频率寄存器 4khz
//	delay_ms(5);
//	Write_Byte(0x83,0x00);
//	delay_ms(5);
//	Write_Byte(0x84,0x00);
//	delay_ms(5);

//	Write_Byte(0x82,0x04);//起始频率寄存器 2khz
//	delay_ms(5);
//	Write_Byte(0x83,0x00);
//	delay_ms(5);
//	Write_Byte(0x84,0x00);
//	delay_ms(5);

//	Write_Byte(0x82,0x02);//起始频率寄存器 1khz
//	delay_ms(5);
//	Write_Byte(0x83,0x00);
//	delay_ms(5);
//	Write_Byte(0x84,0x00);
//	delay_ms(5);
	
	Write_Byte(0x82,0x01);//起始频率寄存器 500hz
	delay_ms(5);
	Write_Byte(0x83,0x00);
	delay_ms(5);
	Write_Byte(0x84,0x00);
	delay_ms(5);
	
	Write_Byte(0x85,0x00);//频率增量寄存器
	delay_ms(5);
	Write_Byte(0x86,0x00);
	delay_ms(5);
	Write_Byte(0x87,0x00);
	delay_ms(5);
	
	Write_Byte(0x88,0x00);//增量数寄存器
	delay_ms(5);
	Write_Byte(0x89,0x00);
	delay_ms(5);
	
	Write_Byte(0x80,0xb1);//标准模式 待机模式
	delay_ms(5);
	Write_Byte(0x81,0x00);//选择内部时钟
	delay_ms(5);
	Write_Byte(0x81,0x10);//复位AD5933
	delay_ms(5);
	
	//Write_Byte(0x80,0x00);
	Write_Byte(0x80,0x11);//低位：将PGA增益设置为x1 高位：0x11：以起始频率初始化 0x41:重复频率，输出电压值范围：典型值2.0V p-p
	delay_ms(100);
	Write_Byte(0x8A,0x03);//建立时间周期数
	Write_Byte(0x8B,0xff);
	Write_Byte(0x80,0x21);//启动频率扫描
	delay_ms(5);
}

double AD5933_display(void)
{
	long ReadTemp,realArr[3],imageArr[3];
  double magnitude;  
	double resistance;
	
	while(1)
  {
    ReadTemp=Rece_Byte(0x8F);  //读取状态寄存器检查DFT是否完成
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
                
	  if (realArr[2]>=0x8000)  //计算实部的原码(除符号位外，取反加一)
		{
      realArr[2]^=0xFFFF; 
      realArr[2]^=0x8000; 
		  realArr[2]+=1;
      realArr[2]^=0x8000;
		}
		if (imageArr[2]>=0x8000)  //计算虚部的原码(除符号位外，取反加一)
		{
      imageArr[2]^=0xFFFF; 
      imageArr[2]^=0x8000; 
			imageArr[2]+=1;
      imageArr[2]^=0x8000;
		}
    magnitude=sqrt(realArr[2]*realArr[2]+imageArr[2]*imageArr[2]);  //模值计算 
		resistance=AD5933_Correction/magnitude;	//resistance为float
		Write_Byte(0x80,0x41);//重复模式，输出电压典型值为2.0V p-p,PAG增益1x
		delay_ms(5);
			
			
		return resistance;
}

