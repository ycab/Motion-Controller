#include "timer.h"
#include "led.h"
#include "key.h"
#include "sys.h"
#include "lcd.h"

u8 KEY0Flag,KEY1Flag,KEY0FirFlag,KEY1FirFlag; 
s16 SpeedX0=72,SpeedX1;
u16 time;
u16 time_sec;
u16 p1;
u8 time_state;//设置定时器状态

//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板
//定时器 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/3
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////  

//通用定时器3中断初始化
//这里时钟选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数
//这里使用的是定时器3!

void TIM3_Int_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //时钟使能
	
	//定时器TIM3初始化
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); //使能指定的TIM3中断,允许更新中断

	//中断优先级NVIC设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器


	TIM_Cmd(TIM3, ENABLE);  //使能TIMx					 
}

void TIM3_PWM_Init(u16 arr,u16 psc)
{
		GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);	//使能定时器3时钟
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB  | RCC_APB2Periph_AFIO, ENABLE);  //使能GPIO外设和AFIO复用功能模块时钟
	
	//GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE); //Timer3部分重映射  TIM3_CH2->PB5    
 
   //设置该引脚为复用输出功能,输出TIM3 CH3的PWM脉冲波形	GPIOB.0
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; //TIM_CH3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIO
 
   //初始化TIM3
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值 
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
	
	//初始化TIM3 Channel2 PWM模式	 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //选择定时器模式:TIM脉冲宽度调制模式2
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性高
	TIM_OC3Init(TIM3, &TIM_OCInitStructure);  //根据T指定的参数初始化外设TIM3 OC2

	TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);  //使能TIM3在CCR2上的预装载寄存器
 TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); //使能指定的TIM3中断,允许更新中断

	//中断优先级NVIC设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器
	
	//TIM_Cmd(TIM3, ENABLE);  //使能TIM3
}

//定时器3中断服务程序
void TIM3_IRQHandler(void)   //TIM3中断
{
		if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) //检查指定的TIM中断发生与否:TIM 中断源 
		{
			TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //清除TIMx的中断待处理位:TIM 中断源 
        
////      if(pulse_count==100)			
////			{	LED0=~LED0;
////				pulse_count=0;
////			}	
//			pulse_count++;
//			if(pulse_count==pulse_num)
//			{
//				TIM_Cmd(TIM3, DISABLE);
//			}
//			if(pulse_count<pulse_current[7])
//			{
//				  if(pulse_count==pulse_current[pulse_step])
//					{
//						TIM3->ARR=arr_p[pulse_step+1];
//						pulse_step++;
//						TIM_SetCompare3(TIM3,TIM3->ARR/2);
//					}
//			}
//			if(pulse_count==pulse_current[7])
//			{
//				pulse_step=0;
//			}
//		  	if(pulse_count>(pulse_num-pulse_current[7]))
//			{
//				  if(pulse_count==pulse_num-pulse_current[6-pulse_step]&&pulse_step<=6)
//					{
//						TIM3->ARR=arr_p[6-pulse_step];
//						pulse_step++;
//						TIM_SetCompare3(TIM3,TIM3->ARR/2);
//					}
//			}
//					
//	
     if(time_state==1)
		 {
				pulse_count++;
			if(pulse_count==pulse_num)
			{
				end=1;
				pulse_count=0;
				p1=0;
				speed_lock=1;
				TIM_Cmd(TIM3, DISABLE);
			}
			if(pulse_count<Pm) 
			{ 
				if(speed_now>=speed)
				 {
					 if(speed_lock==1)
					 {
					   p1=pulse_count;
					   speed_lock=0;
						 speed_now=speed;
					 }
				 }
				 else
				 {
						speed_now=speed_now+Acc/speed_now;
						TIM3->ARR=720000/speed_now-1; 
						TIM_SetCompare3(TIM3,TIM3->ARR/2);
				 }
		 }
		 else
		 {
			 if(speed_lock==1)//如果到中点还达不到最大值说明达不到
			{
					p1=Pm;
			}
			 if(pulse_count>(pulse_num-p1))
			 {
				 speed_now=speed_now-Acc/speed_now;
				 TIM3->ARR=720000/speed_now-1;
				 TIM_SetCompare3(TIM3,TIM3->ARR/2);
			 }
		 }
   }
	if(time_state==3)
	{
		if(speed_now>=speed)
		{
			
		}
		else
		{
				speed_now=speed_now+Acc/speed_now;
				TIM3->ARR=720000/speed_now-1; 
				TIM_SetCompare3(TIM3,TIM3->ARR/2);
		}
	}



	}
		
}









