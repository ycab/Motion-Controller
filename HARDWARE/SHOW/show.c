#include "show.h"
#include "lcd.h"
#include "image.h"
#include "image2lcd.h"

u16 xx,yy;
u8 flag_page=0;

void show_top(void)
{
	LCD_Fill(0,0,479,17,TOP_0);
	LCD_Fill(0,18,479,79,TOP_1);
	image_display(0,80,(u8*)gImage_top_shadow_0);
	for(xx=5;xx<36;xx++)
	image_display(xx,80,(u8*)gImage_top_shadow_1);
	for(xx=92;xx<475;xx++)
	image_display(xx,80,(u8*)gImage_top_shadow_1);
	image_display(475,80,(u8*)gImage_top_shadow_2);
	image_display(36,54,(u8*)gImage_top_start);
}

void show_tip(u16 num,u16 len)
{
	LCD_Fill(132,55,479,79,TOP_1);
	for(xx=127;xx<475;xx++)
	image_display(xx,80,(u8*)gImage_top_shadow_1);//fu gai
	
	LCD_Fill(132+num,55,132+len+9+num,89,BACK_COL);//181
	image_display(127+num,80,(u8*)gImage_top_down_5);		
	for(yy=65;yy<80;yy++)
	image_display(132+num,yy,(u8*)gImage_top_down_6);		
	image_display(132+num,55,(u8*)gImage_top_down_0);		
	for(xx=137;xx<137+len;xx++)
	image_display(xx+num,55,(u8*)gImage_top_down_1);		
	image_display(137+len+num,55,(u8*)gImage_top_down_2);		
	for(yy=65;yy<80;yy++)
	image_display(137+len+num,yy,(u8*)gImage_top_down_3);
	image_display(137+len+num,80,(u8*)gImage_top_down_4);
}

void show_buttom(u16 buttom_x,u16 buttom_y,u16 size_x,u16 size_y)
{
	image_display(buttom_x,buttom_y,(u8*)gImage_buttom_0);
	LCD_Fill(9+buttom_x,1+buttom_y,9+size_x+buttom_x,8+buttom_y,WHITE);
	image_display(9+size_x+buttom_x,buttom_y,(u8*)gImage_buttom_1);
	
	for(yy=9;yy<9+size_y;yy++)
	image_display(buttom_x,yy+buttom_y,(u8*)gImage_buttom_6);
	LCD_Fill(5+buttom_x,9+buttom_y,9+3+size_x+buttom_x,9+size_y+buttom_y,WHITE);
	for(yy=9;yy<9+size_y;yy++)
	image_display(9+4+size_x+buttom_x,yy+buttom_y,(u8*)gImage_buttom_2);
	
	image_display(buttom_x,9+size_y+buttom_y,(u8*)gImage_buttom_5);
	LCD_Fill(9+buttom_x,9+size_y+buttom_y,9+size_x+buttom_x,9+3+size_y+buttom_y,WHITE);
	for(xx=9;xx<9+size_x;xx++)
	image_display(xx+buttom_x,9+3+size_y+buttom_y,(u8*)gImage_buttom_4);
	image_display(9+size_x+buttom_x,9+size_y+buttom_y,(u8*)gImage_buttom_3);
}

void key_back(void)
{
	LCD_Fill(0,18,479,269,BACK_COL);
	image_display(0,18,(u8*)gImage_buttom_0);
	image_display(471,18,(u8*)gImage_buttom_1);
	for(yy=9;yy<239;yy++)
	image_display(475,yy+18,(u8*)gImage_buttom_2);
	image_display(471,239+18,(u8*)gImage_buttom_3);
	for(xx=9;xx<471;xx++)
	image_display(xx,242+18,(u8*)gImage_buttom_4);
	image_display(0,239+18,(u8*)gImage_buttom_5);
	for(yy=9;yy<239;yy++)
	image_display(0,yy+18,(u8*)gImage_buttom_6);
	LCD_Fill(9,1+18,470,8+18,WHITE);
	LCD_Fill(5,9+18,474,256,WHITE);
	LCD_Fill(9,257,470,259,WHITE);
	

	for(xx=0;xx<200;xx+=90)
		for(yy=0;yy<180;yy+=50)
	{
		if(yy!=150||xx==90||xx==180)
		LCD_Fill(40+xx,60+yy,120+xx,100+yy,0xd69a);
	}
	BACK_COLOR=0xd69a;
	LCD_ShowNum_32(70,63,7);
	LCD_ShowNum_32(70,63+50,4);
	LCD_ShowNum_32(70,63+100,1);
	
	LCD_ShowNum_32(70+90,63,8);
	LCD_ShowNum_32(70+90,63+50,5);
	LCD_ShowNum_32(70+90,63+100,2);
	LCD_ShowNum_32(70+90,63+150,0);
	
	LCD_ShowNum_32(70+180,63,9);
	LCD_ShowNum_32(70+180,63+50,6);
	LCD_ShowNum_32(70+180,63+100,3);
	BACK_COLOR=WHITE;

}