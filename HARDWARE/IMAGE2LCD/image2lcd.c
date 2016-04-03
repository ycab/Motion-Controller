#include "sys.h"
#include "lcd.h"
#include "image2lcd.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//Mini STM32开发板
//FLASH图片显示	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2011/10/09
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 正点原子 2009-2019
//All rights reserved		
//********************************************************************************
//V1.1修改说明 20121027
//1，添加对ILI9341 LCD的支持
//2，将LCD_Set_Window放入本代码（因为ILI93xx.c不再包含该函数）
//3，修改image_show，以支持ILI9341。     
////////////////////////////////////////////////////////////////////////////////// 	
     	 
//设置窗口
//sx,sy,ex,ey窗口坐标
//窗口大小:(ex-sx+1)*(ey-ex+1)
//注意,确保ex>=sx;ey>=sy!!!!
//9320/9325/9328/4531/4535/1505/b505/8989等IC已经实际测试	   
void LCD_Set_Window(u16 sx,u16 sy,u16 ex,u16 ey)	   
{
	u8 hsareg,heareg,vsareg,veareg;
	u16 hsaval,heaval,vsaval,veaval;  
// #if USE_HORIZONTAL  //使用横屏	
	//窗口值
	hsaval=sy;				
	heaval=ey;
	vsaval=479-ex;
	veaval=479-sx;	
// #else				//竖屏
// 	//窗口值
// 	hsaval=sx;				
// 	heaval=ex;
// 	vsaval=sy;
// 	veaval=ey;	
// #endif
	if(lcddev.id==0X9341)
	{
		LCD_WR_REG(0x2A); 
		LCD_WR_DATA(hsaval>>8); 
		LCD_WR_DATA(hsaval&0XFF);	 
		LCD_WR_DATA(heaval>>8); 
		LCD_WR_DATA(heaval&0XFF);	 
		LCD_WR_REG(0x2B); 
		LCD_WR_DATA(vsaval>>8); 
		LCD_WR_DATA(vsaval&0XFF);	 
		LCD_WR_DATA(veaval>>8); 
		LCD_WR_DATA(veaval&0XFF);	 
	}else
	{
	 	if(lcddev.id==0X8989)//8989 IC
		{
			hsareg=0X44;heareg=0X44;//水平方向窗口寄存器 (1289的由一个寄存器控制)
			hsaval|=(heaval<<8);	//得到寄存器值.
			heaval=hsaval;
			vsareg=0X45;veareg=0X46;//垂直方向窗口寄存器	  
		}else  //其他驱动IC
		{
			hsareg=0X50;heareg=0X51;//水平方向窗口寄存器
			vsareg=0X52;veareg=0X53;//垂直方向窗口寄存器	  
		}								  
		//设置寄存器值
		LCD_WR_REG_DATA(hsareg,hsaval);
		LCD_WR_REG_DATA(heareg,heaval);
		LCD_WR_REG_DATA(vsareg,vsaval);
		LCD_WR_REG_DATA(veareg,veaval);
	}
} 


//从8位数据获得16位颜色
//mode:0,低位在前,高位在后.
//     1,高位在前,低位在后.
//str:数据
u16 image_getcolor(u8 mode,u8 *str)
{
	u16 color;
	if(mode)
	{
		color=((u16)*str++)<<8;
		color|=*str;
	}else
	{
		color=*str++;
		color|=((u16)*str)<<8;
	}
	return color;	
}
//在液晶上画图		   
//xsta,ysta,xend,yend:画图区域
//scan:见image2lcd V2.9的说明.
//*p:图像数据
void image_show(u16 xsta,u16 ysta,u16 xend,u16 yend,u8 scan,u8 *p)
{  
	u32 i,j,yy;
	u32 len=0;
	if((scan&0x03)==0)//水平扫描
	{
		switch(scan>>6)//设置扫描方式
		{
			case 0:
				LCD_Scan_Dir(L2R_U2D);//从左到右,从上到下
				LCD_Set_Window(xsta,ysta,xend,yend);
				LCD_SetCursor(xsta,ysta);//设置光标位置 
				break; 
			case 1:
				LCD_Scan_Dir(L2R_D2U);//从左到右,从下到上
				LCD_Set_Window(xsta,ysta,xend,yend);
				LCD_SetCursor(xsta,yend);//设置光标位置 
				break; 
			case 2:
				LCD_Scan_Dir(R2L_U2D);//从右到左,从上到下
				LCD_Set_Window(xsta,ysta,xend,yend);
				LCD_SetCursor(xend,ysta);//设置光标位置 
				break; 
			case 3:
				LCD_Scan_Dir(R2L_D2U);//从右到左,从下到上
				LCD_Set_Window(xsta,ysta,xend,yend);
				LCD_SetCursor(xend,yend);//设置光标位置 
				break; 
		}
	}else  //垂直扫描
	{
		switch(scan>>6)//设置扫描方式
		{
			case 0:
				LCD_Scan_Dir(U2D_L2R);//从上到下,从左到右
				LCD_Set_Window(xsta,ysta,xend,yend);
				LCD_SetCursor(xsta,ysta);//设置光标位置 
				break; 
			case 1:
				LCD_Scan_Dir(D2U_L2R);//从下到上从,左到右
				LCD_Set_Window(xsta,ysta,xend,yend);
				LCD_SetCursor(xsta,yend);//设置光标位置 
				break; 
			case 2:
				LCD_Scan_Dir(U2D_R2L);//从上到下,从右到左 
				LCD_Set_Window(xsta,ysta,xend,yend);
				LCD_SetCursor(xend,ysta);//设置光标位置 
				break; 
			case 3:
				LCD_Scan_Dir(D2U_R2L);//从下到上,从右到左
				LCD_Set_Window(xsta,ysta,xend,yend);
				LCD_SetCursor(xend,yend);//设置光标位置 
				break; 
		}
	}
				LCD_Set_Window(xsta,ysta,xend,yend);
				LCD_SetCursor(xsta,ysta);//设置光标位置 

	LCD_WriteRAM_Prepare();     		//开始写入GRAM
	//len=(xend-xsta+1)*(yend-ysta+1);	//写入的数据长度
	yy=ysta;
	for(j=0;j<(yend-ysta+1);j++){
	for(i=0;i<(xend-xsta+1);i++)
	{
		LCD_WR_DATA(image_getcolor(scan&(1<<4),p));
		p+=2;
	}
	yy++;
	LCD_SetCursor(xsta,yy);
	LCD_WriteRAM_Prepare();
}
	LCD_Set_Window(0,0,269,479);
   					  	    
}  

//在指定的位置显示一个图片
//此函数可以显示image2lcd软件生成的任意16位真彩色图片.
//限制:1,尺寸不能超过屏幕的区域.
//     2,生成数据时不能勾选:高位在前(MSB First)
//     3,必须包含图片信息头数据
//x,y:指定位置
//imgx:图片数据(必须包含图片信息头,"4096色/16位真彩色/18位真彩色/24位真彩色/32位真彩色”的图像数据头)
//注意:针对STM32,不能选择image2lcd的"高位在前(MSB First)"选项,否则imginfo的数据将不正确!!
void image_display(u16 x,u16 y,u8 * imgx)
{
	HEADCOLOR *imginfo;
 	u8 ifosize=sizeof(HEADCOLOR);//得到HEADCOLOR结构体的大小
	imginfo=(HEADCOLOR*)imgx;
 	image_show(x,y,x+imginfo->w-1,y+imginfo->h-1,imginfo->scan,imgx+ifosize);		
}















