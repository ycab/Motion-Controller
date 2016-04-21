#ifndef __TIMER_H
#define __TIMER_H
#include "sys.h"
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
extern u16 pulse_min;
extern u16 pulse_count;
extern u16 pulse_num;
extern u8 speed_i;
extern u8 pulse_step;
extern u16 pulse_current[8];
extern u16 speed_min;
extern u16 arr_p[12];//和频率有关的系数
extern float speed_now;
extern float Acc;
extern u16 speed;
extern u8 speed_lock;
extern u16 Pm;
extern u16 run_num;
extern u8 command[100];
extern u16 command_edit_num[100];
extern u16 p1;
extern u8 end;
extern u8 time_state;//设置定时器状态值
typedef struct
{
	u8  Direction;
	u32 Pulse_Num;//脉冲个数
	u16 Pulse_Rate;//脉冲频率
	u16 Speed_Acc;//脉冲加减速度
}Set_Weiyi;
typedef struct
{
	u8 ge;
	u8 shi;
	u8 bai;
	u8 qian;
	u8 wan;
	u8 shiwan;

}Num_Wei;
typedef struct
{
	Num_Wei Pulse_Num; 
	Num_Wei Pulse_Rate; 
	Num_Wei Pulse_Acc;
}Dis_Weiyi;//用于显示位移
extern Set_Weiyi Weiyi[100];
void TIM3_Int_Init(u16 arr,u16 psc);
void TIM3_PWM_Init(u16 arr,u16 psc);
#endif
