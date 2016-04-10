#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"	 
#include "24cxx.h"
#include "flash.h"
#include "touch.h"	  
#include "timer.h"
#include "InputCmd.h"
#include "stmflash.h"
#include "image2lcd.h"
#include "image.h"
#include "show.h"
#include "w5500.h"

#define lcd_key_1 1  //定义键盘按键对应的值
#define lcd_key_2 2
#define lcd_key_3 3
#define lcd_key_4 4


#define Auto_Mode 0
#define Hand_Mode 1
#define ScanOK    1
#define ScanError 0
u8 Mode;
u8 k;
/*****************************************************/
/*stm32 内部FLASH************************************/
 u8 TEXT_Buffer[3]={1,2,3};
#define SIZE 100	 			  	//数组长度
#define FLASH_SAVE_ADDR  0X08070000 				//设置FLASH 保存地址(必须为偶数)
#define FLASH_SAVE_EDIT_ADDR (FLASH_SAVE_ADDR+0X00000100)
#define FLASH_SAVE_WEIYI_ADDR (FLASH_SAVE_ADDR+0X00000300)
void read_edit_num(void);

 


/****************************************************/
/*LCD模块参数及设定*/
void beepms(u16 va);  
void refshow(void);//刷新显示	
void start_init(void);//开机界面初始化
void pwm_speed(u16 acc_time,u16 speed_max,u16 pulse_num_1);




/************************************************************/
/*编辑界面******/
void set_weiyi_init(u8 statement);
void set_weiyi(u8 statement);
void edit_init(void);//按键界面
void jia_service(void);//加按键服务函数
void jian_service(void);//减服务函数
void Move_LR_Service(void);//左右移按键服务函数
void Short_Line_flicker(void);//短线闪烁程序，用在显示改变某个点
void input_command(void);
void display_command(void);//显示命令
void set_system(void);
void set_system_2(void);
void set_system_3(void);
void save_command(void);//保存命令
void read_command(void);//读取命令
u8 init_lock=1;//用于只初始化一遍界面
u8 uc_lock=1;//防止按键重复按下
u8 state=1;
u8 command_num=0;//当前指令标号，即操作当前第几个指令,从0开始
u8 command[100];//指令代码，0为空，1为位移,2为时间，3为速度，4为输入，5为输出，6为扫描，7为跳转，8为循环，9为结束，20为循环终点，21为循环次数
u8 save_ok=0;//保存后为1，再次编辑为0
u8 edit_statement=0;//编辑状态，0为编辑指令，1为编辑指令操作数，2为编辑位移指令






Set_Weiyi Weiyi[100];//可供设置100段位移
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
	u8 ge;
	u8 shi;
	u8 bai;
	u8 qian;
}Num_4_Wei;//用于编辑4位的指令操作数
typedef struct
{
	Num_Wei Pulse_Num; 
	Num_Wei Pulse_Rate; 
	Num_Wei Pulse_Acc;
}Dis_Weiyi;//用于显示位移







Dis_Weiyi Weiyi_Dis[100];//显示100段位移
Num_Wei Set_Time[20];
u8 command_edit_num_wei[100][5];//用于编辑4位的指令操作数,拆分成位，共一百条指令，数组第二位为1~4；0舍弃
u16 command_edit_num[100];//用以编辑操作数，将上一行的4位合并，即可得到当前具体操作数
u16 Pinpoint_Num=1;//设置定位数字，用于编辑位移数字,有效数字为0~17
u16 Pinpoint_Edit=0;//用于编辑代码数字，有效数字为0,1,2,3,4
u32 flicker_count;


u8 page=1;
u8 page_max;
u8 command_max;//有效命令的个数
u8 ScanCommandNum;
u8 ScanCommand[10];



u16 run_num;
u8 end;//单条指令结束符号

void Run(void);
void Run_WeiYi(u8 dir,u16 num);
void Run_Speed(u16 num);
void Run_Time(u16 num);
void Run_Input(u16 Xi,u16 Xstate);
void Run_Output(u16 led ,u16 Xstate);
void Run_Scan(void);
void Run_Return(u16 num,u16 *p);
void Run_End(u16 startnum,u16 endnum,u16 num,u16 *p);
int Scan_GPIO(u16 Xi,u16 Xstate);
void Run_Scan_Service(u16 startnum,u16 endnum);
/*************************************************************************/
/***************************************************/


u16 speed;
float speed_now;
u16 Pm;//中间位移点
float Acc;
u8 speed_lock=1;






/****************************************************/
/*调速模块参数及设定*/
u8 speed_current;//速度的当前值
u16 speed_min;//速度最小值，即最小频率
u16 arr_p[12];//和频率有关的系数
u8 speed_i;
u16 pulse_count;
float mul_pulse;
float mul_speed;
u16 pulse_min;
u16 pulse_num;
u16 speed_base[8]={500,1450,2300,3050,3700,
										4250,4650,4900};
u16 pulse_current_base[8]={200,361,729,1156,1600,
	                         2025,2304,2500
													     };
u16 pulse_current[8];
u8 pulse_step;
u16 arr_test;

 /********************系统设置*************************************/
 u8 Select_Pro=1;//选择程序
Num_Wei Pulse_Dangliang;	//一圈脉冲
u32     PerPulseNum;
Num_Wei Distance_Per;					//每圈距离		
u32     PerDistance;															 
Num_Wei Dot_Pulse;	
Num_Wei Constant_Speed;															 
															 
															 
															 
															 
															 
															 
															 
															 
															 
															 
															 
															 
															 
															 
															 
void Load_Drow_Dialog(void)
{
	LCD_Clear(WHITE);//清屏   
 	POINT_COLOR=BLUE;//设置字体为蓝色 
	LCD_ShowString(lcddev.width-24,0,200,16,16,"RST");//显示清屏区域
  	POINT_COLOR=RED;//设置画笔蓝色 
}
void xianshi()//显示信息
{    
	u8 idstr[5];
	BACK_COLOR=WHITE;
	POINT_COLOR=RED;	
	LCD_ShowString(0,2,200,16,16,"ID:");	    
	inttohex(lcddev.id,idstr);	  
	LCD_ShowString(100,2,200,16,16,idstr);
	//显示32*32汉字
	showhanzi32(0,20,0);	 //淘
	showhanzi32(40,20,1);	 //晶
	showhanzi32(80,20,2);    //驰
	//显示16*16汉字
	showhanzi16(0,55,0);	  //专
	showhanzi16(20,55,1);	  //注
	showhanzi16(40,55,2);	  //显
	showhanzi16(60,55,3);	  //示
	showhanzi16(80,55,4);	  //方
	showhanzi16(100,55,5);	  //案
}
void showqq() //显示全屏QQ
{ 

	u16 x,y; 
	x=0;
	y=100;
	while(y<lcddev.height-39)
	{
		x=0;
		while(x<lcddev.width-39)
		{
			showimage(x,y);	
			x+=40;
		}
		y+=40;
	 }

}
void refshow(void)	 //刷新显示
{
	switch(state)
	{
		case 0:
		LCD_Clear(WHITE);
	    xianshi();
    	showqq();
		break;
		case 1:
		LCD_Clear(BLACK); 
		break;
		case 2:
		LCD_Clear(RED);
		break;
		case 3:
		LCD_Clear(GREEN);
		break;
		case 4:
		LCD_Clear(BLUE);
		break;
	}	
}
void jiance(u8 key) //检测按键
{

	if(key==1)	//KEY_RIGHT按下,则执行校准程序
	{
		LCD_Clear(WHITE);//清屏
	    TP_Adjust();  //屏幕校准 
		TP_Save_Adjdata();	 
		Load_Drow_Dialog();
	}
	if(key==2)
	{
		state++;
		 if(state==5)
		 {
		 	state=0;
			LCD_Init();	   
		 }
		refshow();//刷新显示
	}

}	 
void beepms(u16 va)
{
	beep=1;
	delay_ms(va);
	beep=0;
}					
int main(void)
 {	
   int key,i;	
u8 datatemp[SIZE];	 
	//u16 i=0;	 
	//u8 key=0;	
  	//配置IC与接口
	/*ID设置说明:目前已经包含的驱动如下：（只有如下列出的IC才支持，没有列出的不支持，本程序会不定期更新以便支持更多的IC）
	2.4寸 S6d1121    lcddev.id=0x1121
	2.4寸9325/9328   lcddev.id=0x9325
	2.8寸9325/9328   lcddev.id=0x9325
	3.2寸1289        lcddev.id=0x8989
	3.2寸8347        lcddev.id=0x0047
	3.2寸8352A       lcddev.id=0x0052
	3.2寸8352B       lcddev.id=0x0065
	3.2寸9320        lcddev.id=0x9320
	1963方案4.3寸    lcddev.id=0x1943
	工业级4.3寸      lcddev.id=0x0043
	1963方案5寸      lcddev.id=0x1905
	工业级5寸        lcddev.id=0x0050
	1963方案7寸      lcddev.id=0x1907
	工业级7寸        lcddev.id=0x0070
	工业级9寸        lcddev.id=0x0090  
	*/
	lcddev.bus16=1;//总线接口，0-8位总线，1-16位总线，修改8位总线之前先确定你手里的屏是否是8位接口，目前只有2.4和2.8寸才有支持8位的版本，3.2寸以上均不支持8位总线 
	lcddev.id=0x0043;//指定ID,0x1234为自动读ID,切记注意不是所有的屏都能读到ID,只有少部分驱动IC能读ID,比如4.3寸以及4.3寸以上的都不能读ID.


	delay_init();	    	 //延时函数初始化	  
	//TIM3_Int_Init(399,7199);//10Khz的计数频率，计数到5000为500ms 
	//100Hz的初始频率当arr为71时为所需最大频率10khz
	NVIC_Configuration(); 	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart_init(9600);	 	//串口初始化为9600
	LED_Init();			     //LED端口初始化
	GPIO_Input_Init();
   SPI_Flash_Init();//初始化Flash;
	 	System_Initialization();	//STM32系统初始化函数(初始化STM32时钟及外设)
	Load_Net_Parameters();		//装载网络参数	
	W5500_Hardware_Reset();		//硬件复位W5500
	W5500_Initialization();		//W5500初始货配置
	LCD_Init();	
	tp_dev.init();//触摸初始化
	KEY_Init();
	TIM3_PWM_Init(7199,99);
	//TIM_SetCompare3(TIM3,3000);
 	POINT_COLOR=BLACK;//设置字体为红色 
	 BACK_COLOR=BACK_COL;
	 LCD_Clear(TOP_0);
	//
	 start_init();
		edit_init();

		

	while(1)
	{  
		W5500_Socket_Set();//W5500端口初始化配置

		W5500_Interrupt_Process();//W5500中断处理程序框架

		if((S0_Data & S_RECEIVE) == S_RECEIVE)//如果Socket0接收到数据
		{
			S0_Data&=~S_RECEIVE;
			Process_Socket_Data(0);//W5500接收并发送接收到的数据
		}
		
    switch (edit_statement)
		{
			case 0: input_command();
							break;
			case 1:input_command();
							break;
			case 2: set_weiyi(state);
							break;
			case 3: Run();
							break;
			case 4: set_system();
							break;
			case 5: set_system_2();
			        break;
			case 6: set_system_3();
							break;
			case 7: Run_Scan();
							break;
							
		}
		i++; 
	}
	
}

/***************************************************************************************
若加速时间分为10份，则频率的最小值为最大值的1/20
第一段部分跑的脉冲数是加速部分的1/100,10份脉冲分别为1，3，5.。。。。。19
******************************************************************************************/
void pwm_speed(u16 acc_time,u16 speed_max,u16 pulse_num_1)//加速时间、速度最大值、脉冲个数
{
   	 pulse_num=pulse_num_1;
	   Pm= pulse_num/2;//脉冲中点值
	  speed=speed_max;
	  speed_now=speed_max*0.1;
	  TIM3->ARR=720000/speed_now-1;
	   Acc=speed_max/acc_time;
	  TIM_Cmd(TIM3, ENABLE);
}
void start_init()
{
	u8 i,j;
	LCD_Clear(BACK_COL);
	BACK_COLOR=BACK_COL;
	
	for(i=1;i<=5;i++)//显示代码标号
	{
		LCD_ShowNum_32(10,50*i-30,0);
		LCD_ShowNum_32(26,50*i-30,i-1);
	}
	POINT_COLOR=LIGHTBLUE ;
	for(i=1;i<=5;i++)
	{
		for(j=1;j<=3;j++)//显示代码输入的横线段
		{
			 LCD_DrawLine(60,50*i-30+32+j, 140,50*i-30+32+j);
			 LCD_DrawLine(150,50*i-30+32+j, 280,50*i-30+32+j);
		}
	}
	BACK_COLOR=WHITE;
}
void set_weiyi(u8 statement)//设置位移界面
{
   u16 i;
	if(init_lock)//切换状态时初始化一遍
	{
		set_weiyi_init(statement);
		edit_init();
		init_lock=0;
		flicker_count=0;
	}
	if(Weiyi[statement].Direction==0)//正方向
	{
		showhanzi32(254,20,93);
	}
	else 
	{
		showhanzi32(254,20,94);
	}
	LCD_ShowNum_32(180,80,Weiyi_Dis[statement].Pulse_Num.shiwan);//脉冲个数数值
	LCD_ShowNum_32(198,80,Weiyi_Dis[statement].Pulse_Num.wan);
	LCD_ShowNum_32(216,80,Weiyi_Dis[statement].Pulse_Num.qian);
	LCD_ShowNum_32(234,80,Weiyi_Dis[statement].Pulse_Num.bai);
	LCD_ShowNum_32(252,80,Weiyi_Dis[statement].Pulse_Num.shi);
	LCD_ShowNum_32(270,80,Weiyi_Dis[statement].Pulse_Num.ge);
	
	//LCD_ShowNum_32(180,80,Pulse_Num_dis_01.shiwan);//
	LCD_ShowNum_32(198,140,Weiyi_Dis[statement].Pulse_Rate.wan);//脉冲频率数值
	LCD_ShowNum_32(216,140,Weiyi_Dis[statement].Pulse_Rate.qian);
	LCD_ShowNum_32(234,140,Weiyi_Dis[statement].Pulse_Rate.bai);
	LCD_ShowNum_32(252,140,Weiyi_Dis[statement].Pulse_Rate.shi);
	LCD_ShowNum_32(270,140,Weiyi_Dis[statement].Pulse_Rate.ge);
	
	LCD_ShowNum_32(198,200,Weiyi_Dis[statement].Pulse_Acc.wan);//脉冲速度数值
	LCD_ShowNum_32(216,200,Weiyi_Dis[statement].Pulse_Acc.qian);
	LCD_ShowNum_32(234,200,Weiyi_Dis[statement].Pulse_Acc.bai);
	LCD_ShowNum_32(252,200,Weiyi_Dis[statement].Pulse_Acc.shi);
	LCD_ShowNum_32(270,200,Weiyi_Dis[statement].Pulse_Acc.ge);
	tp_dev.scan(0);
	flicker_count++;

	Short_Line_flicker();

	
	if(tp_dev.sta&TP_PRES_DOWN)			//触摸屏被按下
	{	
		delay_ms(10);
		if(tp_dev.sta&TP_PRES_DOWN)
		{
			if(uc_lock==1)//防止重复按下
			{
				if(tp_dev.x>300&&tp_dev.x<380&&tp_dev.y>10&&tp_dev.y<65)//翻页键
				{
//					Pinpoint_Num=1;
//				  state++;
//					if(state==17)
//					{
//						state=1;
//					}
//					uc_lock=0;
//					init_lock=1;
    
				}
		
				if(tp_dev.x>390&&tp_dev.x<470&&tp_dev.y>10&&tp_dev.y<65)//返回键
				{
					edit_statement=0;
					Pinpoint_Num=1;
					init_lock=1;
					uc_lock=0;
				}
				if(tp_dev.x>300&&tp_dev.x<380&&tp_dev.y>70&&tp_dev.y<125)//左移键
				{
					Pinpoint_Num--;
					if(Pinpoint_Num==0)
					{
						Pinpoint_Num=17;
					}
					uc_lock=0;
					Move_LR_Service();
				}
				if(tp_dev.x>390&&tp_dev.x<470&&tp_dev.y>70&&tp_dev.y<125)//右移键
				{
					Pinpoint_Num++;
					if(Pinpoint_Num==18)
					{
						Pinpoint_Num=1;
					}
					uc_lock=0;
					Move_LR_Service();
				}
				if(tp_dev.x>300&&tp_dev.x<380&&tp_dev.y>130&&tp_dev.y<185)//加键
				{
					uc_lock=0;
				 jia_service();
					
				}
				if(tp_dev.x>390&&tp_dev.x<470&&tp_dev.y>130&&tp_dev.y<185)//减键
				{
					uc_lock=0;
				 jian_service();
					
				}
				if(tp_dev.x>300&&tp_dev.x<380&&tp_dev.y>190&&tp_dev.y<245)//停止键
				{
					  end=1;
						pulse_count=0;
						p1=0;
						speed_lock=1;
						TIM_Cmd(TIM3, DISABLE);
					   edit_statement=2;
				}
				if(tp_dev.x>390&&tp_dev.x<470&&tp_dev.y>190&&tp_dev.y<245)//运行
				{
					run_num=0;
					for(i=0;i<=99;i++)
					{
						if(command[i]==1)
						{
							Weiyi[i].Pulse_Num=Weiyi_Dis[i].Pulse_Num.shiwan*100000
														+Weiyi_Dis[i].Pulse_Num.wan*10000
			                      +Weiyi_Dis[i].Pulse_Num.qian*1000
														+Weiyi_Dis[i].Pulse_Num.bai*100
														+Weiyi_Dis[i].Pulse_Num.shi*10
														+Weiyi_Dis[i].Pulse_Num.ge;//得到的是位移距离
							Weiyi[i].Pulse_Num=Weiyi[i].Pulse_Num*PerPulseNum/PerDistance;//得到脉冲个数
				
				
							Weiyi[i].Pulse_Rate=Weiyi_Dis[i].Pulse_Rate.wan*10000
			                      +Weiyi_Dis[i].Pulse_Rate.qian*1000
														+Weiyi_Dis[i].Pulse_Rate.bai*100
														+Weiyi_Dis[i].Pulse_Rate.shi*10
														+Weiyi_Dis[i].Pulse_Rate.ge;
							Weiyi[i].Speed_Acc=Weiyi_Dis[i].Pulse_Acc.wan*10000
			                      +Weiyi_Dis[i].Pulse_Acc.qian*1000
														+Weiyi_Dis[i].Pulse_Acc.bai*100
														+Weiyi_Dis[i].Pulse_Acc.shi*10
														+Weiyi_Dis[i].Pulse_Acc.ge;
						}
					}
					edit_statement=3;
					uc_lock=0;
				}
			}
			
		}
	}
	else
	{
		uc_lock=1;
	}
	
}

void set_weiyi_init(u8 statement)//界面初始化
{
	//u8 ge;
	//u8 shi;
	//ge=statement%10;
	//shi=statement/10;
	LCD_Clear(WHITE);
//	showhanzi32(0,20,3);                                //设
//	showhanzi32(36,20,4);                               //定
//	LCD_ShowNum_32(72,20,shi);                            //0
//	LCD_ShowNum_32(90,20,ge);                            //1
//	showhanzi32(108,20,5);                              //段
//	showhanzi32(144,20,6);                              //位
//	showhanzi32(180,20,7);                              //移
	
	showhanzi32(0,20,6);                                //位
	showhanzi32(36,20,7);                               //移
	showhanzi32(72,20,91);                              //方
	showhanzi32(108,20,92);                              //向

	
	showhanzi32(0,80,6);                                //位
	showhanzi32(36,80,7);                               //移
  showhanzi32(72,80,73);                              //距
	showhanzi32(108,80,74);                             //离
	
	showhanzi32(0,140,8);                               //脉
	showhanzi32(36,140,9);                              //冲
  showhanzi32(72,140,12);                             //频
	showhanzi32(108,140,13);                            //率
	
	showhanzi32(0,200,14);                              //加
	showhanzi32(36,200,15);                             //减
  showhanzi32(72,200,16);                             //速
	showhanzi32(108,200,32);                            //时
	showhanzi32(144,200,33);                            //间

	LCD_ShowNum_32(180,80,Weiyi_Dis[statement].Pulse_Num.shiwan);//脉冲个数数值
	LCD_ShowNum_32(198,80,Weiyi_Dis[statement].Pulse_Num.wan);
	LCD_ShowNum_32(216,80,Weiyi_Dis[statement].Pulse_Num.qian);
	LCD_ShowNum_32(234,80,Weiyi_Dis[statement].Pulse_Num.bai);
	LCD_ShowNum_32(252,80,Weiyi_Dis[statement].Pulse_Num.shi);
	LCD_ShowNum_32(270,80,Weiyi_Dis[statement].Pulse_Num.ge);
	
	//LCD_ShowNum_32(180,80,Pulse_Num_dis_01.shiwan);//
	LCD_ShowNum_32(198,140,Weiyi_Dis[statement].Pulse_Rate.wan);//脉冲频率数值
	LCD_ShowNum_32(216,140,Weiyi_Dis[statement].Pulse_Rate.qian);
	LCD_ShowNum_32(234,140,Weiyi_Dis[statement].Pulse_Rate.bai);
	LCD_ShowNum_32(252,140,Weiyi_Dis[statement].Pulse_Rate.shi);
	LCD_ShowNum_32(270,140,Weiyi_Dis[statement].Pulse_Rate.ge);
	
	LCD_ShowNum_32(198,200,Weiyi_Dis[statement].Pulse_Acc.wan);//脉冲速度数值
	LCD_ShowNum_32(216,200,Weiyi_Dis[statement].Pulse_Acc.qian);
	LCD_ShowNum_32(234,200,Weiyi_Dis[statement].Pulse_Acc.bai);
	LCD_ShowNum_32(252,200,Weiyi_Dis[statement].Pulse_Acc.shi);
	LCD_ShowNum_32(270,200,Weiyi_Dis[statement].Pulse_Acc.ge);
	tp_dev.scan(0);
}


void edit_init(void)//编辑界面即按键界面
{
	if(edit_statement==0||edit_statement==1)
	{
		show_buttom(290,15,80,32);
		showhanzi32(305,20,18);//翻页
		showhanzi32(340,20,19);
		
		//LCD_DrawRectangle(390,15 ,470 ,61 );
		show_buttom(380,15,80,32);
		showhanzi32(395,20,3);//设置
		showhanzi32(430,20,62);
		
		//LCD_DrawRectangle(300,65 ,380 ,111 );
		show_buttom(290,65,80,32);
		showhanzi32(305,70,22);//左移
		showhanzi32(340,70,23);
		
		//LCD_DrawRectangle(390,65 ,470 ,111 );
		show_buttom(380,65,80,32);
		showhanzi32(395,70,24);//右移
		showhanzi32(430,70,25);
		
		//LCD_DrawRectangle(300,115 ,380 ,161 );
		show_buttom(290,115,80,32);
		showhanzi32(322,120,26);//加
	 
		
		//LCD_DrawRectangle(390,115 ,470 ,161 );
		show_buttom(380,115,80,32);
		showhanzi32(412,120,27);//减
		
		
		//LCD_DrawRectangle(300,165 ,380 ,211 );
		show_buttom(290,165,80,32);
		showhanzi32(305,170,47);//删除
		showhanzi32(340,170,48);
		
		//LCD_DrawRectangle(390,165 ,470 ,211 );
		show_buttom(380,165,80,32);
		showhanzi32(395,170,53);//添加
		showhanzi32(430,170,26);
		
	// LCD_DrawRectangle(300,215 ,380 ,261 );
	show_buttom(290,215,80,32);
		showhanzi32(305,220,77);//保存
		showhanzi32(340,220,78);
	//	
	//	LCD_DrawRectangle(390,215 ,470 ,261 );
	show_buttom(380,215,80,32);
		showhanzi32(395,220,28);//运行
		showhanzi32(430,220,29);
	}
		if(edit_statement==2)
	{
		LCD_DrawRectangle(300,10 ,380 ,65 );
		showhanzi32(305,20,18);//翻页
		showhanzi32(340,20,19);
		
		LCD_DrawRectangle(390,10 ,470 ,65 );
		showhanzi32(395,20,49);//返回
		showhanzi32(430,20,50);
		
		LCD_DrawRectangle(300,70 ,380 ,125 );
		showhanzi32(305,80,22);//左移
		showhanzi32(340,80,23);
		
		LCD_DrawRectangle(390,70 ,470 ,125 );
		showhanzi32(395,80,24);//右移
		showhanzi32(430,80,25);
		
		LCD_DrawRectangle(300,130 ,380 ,185 );
		showhanzi32(322,140,26);//加
	 
		
		LCD_DrawRectangle(390,130 ,470 ,185 );
		showhanzi32(412,140,27);//减
		
		
		LCD_DrawRectangle(300,190 ,380 ,245 );
		showhanzi32(305,200,47);//删除
		showhanzi32(340,200,48);
		
		LCD_DrawRectangle(390,190 ,470 ,245 );
		showhanzi32(395,200,28);//运行
		showhanzi32(430,200,29);
	}
		if(edit_statement==4||edit_statement==5||edit_statement==6)
	{
		LCD_DrawRectangle(300,10 ,380 ,65 );
		showhanzi32(305,20,18);//翻页
		showhanzi32(340,20,19);
		
		LCD_DrawRectangle(390,10 ,470 ,65 );
		showhanzi32(395,20,49);//返回
		showhanzi32(430,20,50);
		
		LCD_DrawRectangle(300,70 ,380 ,125 );
		showhanzi32(305,80,22);//左移
		showhanzi32(340,80,23);
		
		LCD_DrawRectangle(390,70 ,470 ,125 );
		showhanzi32(395,80,24);//右移
		showhanzi32(430,80,25);
		
		LCD_DrawRectangle(300,130 ,380 ,185 );
		showhanzi32(322,140,26);//加
	 
		
		LCD_DrawRectangle(390,130 ,470 ,185 );
		showhanzi32(412,140,27);//减
		
		
		LCD_DrawRectangle(300,190 ,380 ,245 );
		showhanzi32(305,200,75);//读取
		showhanzi32(340,200,76);
		
		LCD_DrawRectangle(390,190 ,470 ,245 );
		showhanzi32(395,200,77);//保存
		showhanzi32(430,200,78);
	}
}
void jia_service(void)//加服务函数
{
	   u8 i;
		if(edit_statement==0)//编辑指令
		{
			for(i=1;i<=4;i++)
			{
			command_edit_num_wei[command_num][i]=0;
			}
			
			if(command[command_num]>=0&&command[command_num]<=15)
			{
				command[command_num]++;
			}
			if(command[command_num]==10)//指令构成循环
			{
				command[command_num]=0;
			}
			if(command[command_num]==2)//如果从位移跳到下一个指令，清除位移的参数
			{
					Weiyi_Dis[command_num].Pulse_Num.shiwan=0;//脉冲个数数值
					Weiyi_Dis[command_num].Pulse_Num.wan=0;
			    Weiyi_Dis[command_num].Pulse_Num.qian=0;
					Weiyi_Dis[command_num].Pulse_Num.bai=0;
					Weiyi_Dis[command_num].Pulse_Num.shi=0;
					Weiyi_Dis[command_num].Pulse_Num.ge=0;
	
	//LCD_ShowNum_32(180,80,Pulse_Num_dis_01.shiwan);//
					Weiyi_Dis[command_num].Pulse_Rate.wan=0;//脉冲频率数值
					Weiyi_Dis[command_num].Pulse_Rate.qian=0;
					Weiyi_Dis[command_num].Pulse_Rate.bai=0;
					Weiyi_Dis[command_num].Pulse_Rate.shi=0;
					Weiyi_Dis[command_num].Pulse_Rate.ge=0;
	
					Weiyi_Dis[command_num].Pulse_Acc.wan=0;//脉冲速度数值
					Weiyi_Dis[command_num].Pulse_Acc.qian=0;
					Weiyi_Dis[command_num].Pulse_Acc.bai=0;
					Weiyi_Dis[command_num].Pulse_Acc.shi=0;
					Weiyi_Dis[command_num].Pulse_Acc.ge=0;
				
				Weiyi[command_num].Direction=0;
				Weiyi[command_num].Pulse_Num=0;
				Weiyi[command_num].Pulse_Rate=0;
				Weiyi[command_num].Speed_Acc=0;
			}
			if(command[command_num]==8)//如果是循环指令，还要设置另外2个指令
			{
					for(i=99;i>command_num+2;i--)
					{
						command[i]=command[i-2];
						command_edit_num_wei[i][1]=command_edit_num_wei[i-2][1];
						command_edit_num_wei[i][2]=command_edit_num_wei[i-2][2];
						command_edit_num_wei[i][3]=command_edit_num_wei[i-2][3];
						command_edit_num_wei[i][4]=command_edit_num_wei[i-2][4];
					}
		      command[command_num+1]=20;
					command[command_num+2]=21;
					command_edit_num_wei[command_num+1][1]=0;
					command_edit_num_wei[command_num+1][2]=0;
					command_edit_num_wei[command_num+1][3]=0;
					command_edit_num_wei[command_num+1][4]=0;
					command_edit_num_wei[command_num+2][1]=0;
					command_edit_num_wei[command_num+2][2]=0;
					command_edit_num_wei[command_num+2][3]=0;
					command_edit_num_wei[command_num+2][4]=0;
			}
			if(command[command_num]==9)//跳过循环指令将多出的指令去掉
			{
					for(i=1;(i+command_num)<=100;i++)
					{
						command[command_num+i]=command[command_num+i+2];
						command_edit_num_wei[command_num+i][1]=command_edit_num_wei[command_num+i+2][1];
						command_edit_num_wei[command_num+i][2]=command_edit_num_wei[command_num+i+2][2];
						command_edit_num_wei[command_num+i][3]=command_edit_num_wei[command_num+i+2][3];
						command_edit_num_wei[command_num+i][4]=command_edit_num_wei[command_num+i+2][4];
					}
			}
			display_command();
		}
		
		
		
		
		if(edit_statement==1)//编辑指令操作数
		{
				  if(command[command_num]==1||command[command_num]==2||command[command_num]==3||command[command_num]==7||command[command_num]==8||command[command_num]==20||command[command_num]==21)
				{
					command_edit_num_wei[command_num][Pinpoint_Edit]++;//对应得操作数++；command_num用于指示当前编辑第几条指令，Pinpoint_Edit用于指示指令4位操作数的某一位
					if(command_edit_num_wei[command_num][Pinpoint_Edit]==10)
					{
						command_edit_num_wei[command_num][Pinpoint_Edit]=0;
					}
					display_command();
				}
			  if(command[command_num]==4||command[command_num]==6)//输入 扫描
				{
					if(Pinpoint_Edit==3)
					{
						command_edit_num_wei[command_num][Pinpoint_Edit]++;//对应得操作数++；command_num用于指示当前编辑第几条指令，Pinpoint_Edit用于指示指令4位操作数的某一位
						if(command_edit_num_wei[command_num][Pinpoint_Edit]==7)
						{
							command_edit_num_wei[command_num][Pinpoint_Edit]=1;
						}
						display_command();
					}
						if(Pinpoint_Edit==4)//0,1用来指示高低
					{
						command_edit_num_wei[command_num][Pinpoint_Edit]++;//对应得操作数++；command_num用于指示当前编辑第几条指令，Pinpoint_Edit用于指示指令4位操作数的某一位
						if(command_edit_num_wei[command_num][Pinpoint_Edit]==2)
						{
							command_edit_num_wei[command_num][Pinpoint_Edit]=0;
						}
						display_command();
					}
				}
						if(command[command_num]==5)//输出
				{
					if(Pinpoint_Edit==3)
					{
						command_edit_num_wei[command_num][Pinpoint_Edit]++;//对应得操作数++；command_num用于指示当前编辑第几条指令，Pinpoint_Edit用于指示指令4位操作数的某一位
						if(command_edit_num_wei[command_num][Pinpoint_Edit]==5)
						{
							command_edit_num_wei[command_num][Pinpoint_Edit]=1;
						}
						display_command();
					}
						if(Pinpoint_Edit==4)//0,1用来指示高低
					{
						command_edit_num_wei[command_num][Pinpoint_Edit]++;//对应得操作数++；command_num用于指示当前编辑第几条指令，Pinpoint_Edit用于指示指令4位操作数的某一位
						if(command_edit_num_wei[command_num][Pinpoint_Edit]==2)
						{
							command_edit_num_wei[command_num][Pinpoint_Edit]=0;
						}
						display_command();
					}
				}
			
		}
		
		
		
		
		
	  if(edit_statement==2)//编辑位移
		{
			switch(Pinpoint_Num)
			{
				case 1:	  if(Weiyi[state].Direction==0)
									{
										Weiyi[state].Direction=1;
									}
									else
									{
										Weiyi[state].Direction=0;
									}
									break;
				case 2:	Weiyi_Dis[state].Pulse_Num.shiwan++;
									if(Weiyi_Dis[state].Pulse_Num.shiwan==10)
									{
										Weiyi_Dis[state].Pulse_Num.shiwan=0;
									}
									break;
				case 3:	Weiyi_Dis[state].Pulse_Num.wan++;
									if(Weiyi_Dis[state].Pulse_Num.wan==10)
									{
										Weiyi_Dis[state].Pulse_Num.wan=0;
									}
									break;
				case 4:	Weiyi_Dis[state].Pulse_Num.qian++;
									if(Weiyi_Dis[state].Pulse_Num.qian==10)
									{
										Weiyi_Dis[state].Pulse_Num.qian=0;
									}
									break;
				case 5:	Weiyi_Dis[state].Pulse_Num.bai++;
									if(Weiyi_Dis[state].Pulse_Num.bai==10)
									{
										Weiyi_Dis[state].Pulse_Num.bai=0;
									}
									break;
				case 6:	Weiyi_Dis[state].Pulse_Num.shi++;
									if(Weiyi_Dis[state].Pulse_Num.shi==10)
									{
									Weiyi_Dis[state].Pulse_Num.shi=0;
									}
									break;
				case 7:	Weiyi_Dis[state].Pulse_Num.ge++;
									if(Weiyi_Dis[state].Pulse_Num.ge==10)
									{
										Weiyi_Dis[state].Pulse_Num.ge=0;
									}
									break;
				case 8:	Weiyi_Dis[state].Pulse_Rate.wan++;
									if(Weiyi_Dis[state].Pulse_Rate.wan==10)
									{
										Weiyi_Dis[state].Pulse_Rate.wan=0;
									}
									break;
				case 9:	Weiyi_Dis[state].Pulse_Rate.qian++;
									if(Weiyi_Dis[state].Pulse_Rate.qian==10)
									{
										Weiyi_Dis[state].Pulse_Rate.qian=0;
									}
									break;
				case 10:	Weiyi_Dis[state].Pulse_Rate.bai++;
									if(Weiyi_Dis[state].Pulse_Rate.bai==10)
									{
										Weiyi_Dis[state].Pulse_Rate.bai=0;
									}
									break;
				case 11:	Weiyi_Dis[state].Pulse_Rate.shi++;
									if(Weiyi_Dis[state].Pulse_Rate.shi==10)
									{
										Weiyi_Dis[state].Pulse_Rate.shi=0;
									}
									break;
				case 12:	Weiyi_Dis[state].Pulse_Rate.ge++;
									if(Weiyi_Dis[state].Pulse_Rate.ge==10)
									{
										Weiyi_Dis[state].Pulse_Rate.ge=0;
									}
									break;
				case 13:	Weiyi_Dis[state].Pulse_Acc.wan++;
									if(Weiyi_Dis[state].Pulse_Acc.wan==10)
									{
										Weiyi_Dis[state].Pulse_Acc.wan=0;
									}
									break;
				case 14:	Weiyi_Dis[state].Pulse_Acc.qian++;
									if(Weiyi_Dis[state].Pulse_Acc.qian==10)
									{
										Weiyi_Dis[state].Pulse_Acc.qian=0;
									}
									break;
					case 15:	Weiyi_Dis[state].Pulse_Acc.bai++;
									if(Weiyi_Dis[state].Pulse_Acc.bai==10)
									{
										Weiyi_Dis[state].Pulse_Acc.bai=0;
									}
									break;
					case 16:	Weiyi_Dis[state].Pulse_Acc.shi++;
									if(Weiyi_Dis[state].Pulse_Acc.shi==10)
									{
										Weiyi_Dis[state].Pulse_Acc.shi=0;
									}
									break;
					case 17:	Weiyi_Dis[state].Pulse_Acc.ge++;
									if(Weiyi_Dis[state].Pulse_Acc.ge==10)
									{
										Weiyi_Dis[state].Pulse_Acc.ge=0;
									}
									break;							
				}
			/**********************************************************/
			/*计算位移的脉冲个数
				          脉冲频率
									加速时间*/
				Weiyi[state].Pulse_Num=Weiyi_Dis[state].Pulse_Num.shiwan*100000
														+Weiyi_Dis[state].Pulse_Num.wan*10000
			                      +Weiyi_Dis[state].Pulse_Num.qian*1000
														+Weiyi_Dis[state].Pulse_Num.bai*100
														+Weiyi_Dis[state].Pulse_Num.shi*10
														+Weiyi_Dis[state].Pulse_Num.ge;//得到的是位移距离
			  Weiyi[state].Pulse_Num=Weiyi[state].Pulse_Num*PerPulseNum/PerDistance;//得到脉冲个数
				
				
				Weiyi[state].Pulse_Rate=Weiyi_Dis[state].Pulse_Rate.wan*10000
			                      +Weiyi_Dis[state].Pulse_Rate.qian*1000
														+Weiyi_Dis[state].Pulse_Rate.bai*100
														+Weiyi_Dis[state].Pulse_Rate.shi*10
														+Weiyi_Dis[state].Pulse_Rate.ge;
				Weiyi[state].Speed_Acc=Weiyi_Dis[state].Pulse_Acc.wan*10000
			                      +Weiyi_Dis[state].Pulse_Acc.qian*1000
														+Weiyi_Dis[state].Pulse_Acc.bai*100
														+Weiyi_Dis[state].Pulse_Acc.shi*10
														+Weiyi_Dis[state].Pulse_Acc.ge;
			}
		if(edit_statement==4)//系统设置
		{
			switch(Pinpoint_Num)
			{
				case 1: Select_Pro++;
								if(Select_Pro==4)
								{
									Select_Pro=1;
								}
								break;	
        case 2:if(Mode==Auto_Mode)
				        {
									Mode=Hand_Mode;
								}
								else
								{
									Mode=Auto_Mode;
								}
			}
		}
		if(edit_statement==5)//系统设置2
		{
			switch(Pinpoint_Num)
			{
				case 1:Pulse_Dangliang.wan++;
								if(Pulse_Dangliang.wan==10)
								{
									Pulse_Dangliang.wan=0;
								}
								break;
				case 2:Pulse_Dangliang.qian++;
								if(Pulse_Dangliang.qian==10)
								{
									Pulse_Dangliang.qian=0;
								}
								break;
				case 3:Pulse_Dangliang.bai++;
								if(Pulse_Dangliang.bai==10)
								{
									Pulse_Dangliang.bai=0;
								}
								break;
				case 4:Pulse_Dangliang.shi++;
								if(Pulse_Dangliang.shi==10)
								{
									Pulse_Dangliang.shi=0;
								}
								break;
				case 5:Pulse_Dangliang.ge++;
								if(Pulse_Dangliang.ge==10)
								{
									Pulse_Dangliang.ge=0;
								}
								break;
				case 6:Distance_Per.wan++;
								if(Distance_Per.wan==10)
								{
									Distance_Per.wan=0;
								}
								break;
				case 7: Distance_Per.qian++;
								if(Distance_Per.qian==10)
								{
									Distance_Per.qian=0;
								}
								break;
				case 8:Distance_Per.bai++;
								if(Distance_Per.bai==10)
								{
									Distance_Per.bai=0;
								}
								break;		
				case 9:Distance_Per.shi++;
								if(Distance_Per.shi==10)
								{
									Distance_Per.shi=0;
								}
								break;
				case 10:Distance_Per.ge++;
								if(Distance_Per.ge==10)
								{
									Distance_Per.ge=0;
								}
								break;								
			}
			
				PerPulseNum=Pulse_Dangliang.wan*10000
			              +Pulse_Dangliang.qian*1000
										+Pulse_Dangliang.bai*100
										+Pulse_Dangliang.shi*10
										+Pulse_Dangliang.ge;
				PerDistance=Distance_Per.wan*10000
			                      +Distance_Per.qian*1000
														+Distance_Per.bai*100
														+Distance_Per.shi*10
														+Distance_Per.ge;
		}
		if(edit_statement==6)//系统设置2
		{
			switch(Pinpoint_Num)
			{
				case 1:Dot_Pulse.wan++;
								if(Dot_Pulse.wan==10)
								{
									Dot_Pulse.wan=0;
								}
								break;
				case 2:Dot_Pulse.qian++;
								if(Dot_Pulse.qian==10)
								{
									Dot_Pulse.qian=0;
								}
								break;
				case 3:Dot_Pulse.bai++;
								if(Dot_Pulse.bai==10)
								{
									Dot_Pulse.bai=0;
								}
								break;
				case 4:Dot_Pulse.shi++;
								if(Dot_Pulse.shi==10)
								{
									Dot_Pulse.shi=0;
								}
								break;
				case 5:Dot_Pulse.ge++;
								if(Dot_Pulse.ge==10)
								{
									Dot_Pulse.ge=0;
								}
								break;
				case 6:Constant_Speed.wan++;
								if(Constant_Speed.wan==10)
								{
									Constant_Speed.wan=0;
								}
								break;
				case 7: Constant_Speed.qian++;
								if(Constant_Speed.qian==10)
								{
									Constant_Speed.qian=0;
								}
								break;
				case 8:Constant_Speed.bai++;
								if(Constant_Speed.bai==10)
								{
									Constant_Speed.bai=0;
								}
								break;		
				case 9:Constant_Speed.shi++;
								if(Constant_Speed.shi==10)
								{
									Constant_Speed.shi=0;
								}
								break;
				case 10:Constant_Speed.ge++;
								if(Constant_Speed.ge==10)
								{
									Constant_Speed.ge=0;
								}
								break;								
			}
		}
    		
}
void jian_service(void)//减服务函数
{
		  u8 i;
		if(edit_statement==0)//编辑指令
		{
			for(i=1;i<=4;i++)
			{
			command_edit_num_wei[command_num][i]=0;
			}
			command[command_num]--;
			if(command[command_num]==255)
			{
				command[command_num]=9;
			}
			if(command[command_num]==0)
			{
				Weiyi_Dis[command_num].Pulse_Num.shiwan=0;//脉冲个数数值
					Weiyi_Dis[command_num].Pulse_Num.wan=0;
			    Weiyi_Dis[command_num].Pulse_Num.qian=0;
					Weiyi_Dis[command_num].Pulse_Num.bai=0;
					Weiyi_Dis[command_num].Pulse_Num.shi=0;
					Weiyi_Dis[command_num].Pulse_Num.ge=0;
	
	//LCD_ShowNum_32(180,80,Pulse_Num_dis_01.shiwan);//
					Weiyi_Dis[command_num].Pulse_Rate.wan=0;//脉冲频率数值
					Weiyi_Dis[command_num].Pulse_Rate.qian=0;
					Weiyi_Dis[command_num].Pulse_Rate.bai=0;
					Weiyi_Dis[command_num].Pulse_Rate.shi=0;
					Weiyi_Dis[command_num].Pulse_Rate.ge=0;
	
					Weiyi_Dis[command_num].Pulse_Acc.wan=0;//脉冲速度数值
					Weiyi_Dis[command_num].Pulse_Acc.qian=0;
					Weiyi_Dis[command_num].Pulse_Acc.bai=0;
					Weiyi_Dis[command_num].Pulse_Acc.shi=0;
					Weiyi_Dis[command_num].Pulse_Acc.ge=0;
				
				Weiyi[command_num].Direction=0;
				Weiyi[command_num].Pulse_Num=0;
				Weiyi[command_num].Pulse_Rate=0;
				Weiyi[command_num].Speed_Acc=0;
			}
			if(command[command_num]==8)//如果是循环指令，还要设置另外2个指令
			{
					for(i=99;i>command_num+2;i--)
					{
						command[i]=command[i-2];
						command_edit_num_wei[i][1]=command_edit_num_wei[i-2][1];
						command_edit_num_wei[i][2]=command_edit_num_wei[i-2][2];
						command_edit_num_wei[i][3]=command_edit_num_wei[i-2][3];
						command_edit_num_wei[i][4]=command_edit_num_wei[i-2][4];
					}
		      command[command_num+1]=20;
					command[command_num+2]=21;
					command_edit_num_wei[command_num+1][1]=0;
					command_edit_num_wei[command_num+1][2]=0;
					command_edit_num_wei[command_num+1][3]=0;
					command_edit_num_wei[command_num+1][4]=0;
					command_edit_num_wei[command_num+2][1]=0;
					command_edit_num_wei[command_num+2][2]=0;
					command_edit_num_wei[command_num+2][3]=0;
					command_edit_num_wei[command_num+2][4]=0;
			}
			if(command[command_num]==7)//跳过循环指令将多出的指令去掉
			{
					for(i=1;(i+command_num)<=100;i++)
					{
						command[command_num+i]=command[command_num+i+2];
						command_edit_num_wei[command_num+i][1]=command_edit_num_wei[command_num+i+2][1];
						command_edit_num_wei[command_num+i][2]=command_edit_num_wei[command_num+i+2][2];
						command_edit_num_wei[command_num+i][3]=command_edit_num_wei[command_num+i+2][3];
						command_edit_num_wei[command_num+i][4]=command_edit_num_wei[command_num+i+2][4];
					}
			}
			display_command();
		}
		///////////////////////////////////////////////////////////////////
		
		
		
		
		if(edit_statement==1)//编辑指令操作数
		{
		  	if(command[command_num]==1||command[command_num]==2||command[command_num]==3||command[command_num]==7||command[command_num]==8||command[command_num]==20||command[command_num]==21)
				{
					command_edit_num_wei[command_num][Pinpoint_Edit]--;//对应得操作数++；command_num用于指示当前编辑第几条指令，Pinpoint_Edit用于指示指令4位操作数的某一位
					if(command_edit_num_wei[command_num][Pinpoint_Edit]==255)
					{
						command_edit_num_wei[command_num][Pinpoint_Edit]=9;
					}
					display_command();
				}
			  if(command[command_num]==4||command[command_num]==5||command[command_num]==6)
				{
					if(Pinpoint_Edit==3)
					{
						command_edit_num_wei[command_num][Pinpoint_Edit]--;//对应得操作数++；command_num用于指示当前编辑第几条指令，Pinpoint_Edit用于指示指令4位操作数的某一位
						if(command_edit_num_wei[command_num][Pinpoint_Edit]==255)
						{
							command_edit_num_wei[command_num][Pinpoint_Edit]=9;
						}
						display_command();
					}
						if(Pinpoint_Edit==4)//0,1用来指示高低
					{
						command_edit_num_wei[command_num][Pinpoint_Edit]--;//对应得操作数++；command_num用于指示当前编辑第几条指令，Pinpoint_Edit用于指示指令4位操作数的某一位
						if(command_edit_num_wei[command_num][Pinpoint_Edit]==255)
						{
							command_edit_num_wei[command_num][Pinpoint_Edit]=1;
						}
						display_command();
					}
				}
		}
		//////////////////////////////////////////////////////////////////////
		
		
		
		if(edit_statement==2)//编辑位移
		{
			switch(Pinpoint_Num)
			{
				case 1:		if(Weiyi[state].Direction==0)
									{
										Weiyi[state].Direction=1;
									}
									else
									{
										Weiyi[state].Direction=0;
									}
									break;
				case 2:	Weiyi_Dis[state].Pulse_Num.shiwan--;
									if(Weiyi_Dis[state].Pulse_Num.shiwan==255)
									{
										Weiyi_Dis[state].Pulse_Num.shiwan=9;
									}
									break;
				case 3:	Weiyi_Dis[state].Pulse_Num.wan--;
									if(Weiyi_Dis[state].Pulse_Num.wan==255)
									{
										Weiyi_Dis[state].Pulse_Num.wan=9;
									}
									break;
				case 4:	Weiyi_Dis[state].Pulse_Num.qian--;
									if(Weiyi_Dis[state].Pulse_Num.qian==255)
									{
										Weiyi_Dis[state].Pulse_Num.qian=9;
									}
									break;
				case 5:	Weiyi_Dis[state].Pulse_Num.bai--;
									if(Weiyi_Dis[state].Pulse_Num.bai==255)
									{
										Weiyi_Dis[state].Pulse_Num.bai=9;
									}
									break;
				case 6:	Weiyi_Dis[state].Pulse_Num.shi--;
									if(Weiyi_Dis[state].Pulse_Num.shi==255)
									{
									Weiyi_Dis[state].Pulse_Num.shi=9;
									}
									break;
				case 7:	Weiyi_Dis[state].Pulse_Num.ge--;
									if(Weiyi_Dis[state].Pulse_Num.ge==255)
									{
										Weiyi_Dis[state].Pulse_Num.ge=9;
									}
									break;
				case 8:	Weiyi_Dis[state].Pulse_Rate.wan--;
									if(Weiyi_Dis[state].Pulse_Rate.wan==255)
									{
										Weiyi_Dis[state].Pulse_Rate.wan=9;
									}
									break;
				case 9:	Weiyi_Dis[state].Pulse_Rate.qian--;
									if(Weiyi_Dis[state].Pulse_Rate.qian==255)
									{
										Weiyi_Dis[state].Pulse_Rate.qian=9;
									}
									break;
				case 10:	Weiyi_Dis[state].Pulse_Rate.bai--;
									if(Weiyi_Dis[state].Pulse_Rate.bai==255)
									{
										Weiyi_Dis[state].Pulse_Rate.bai=9;
									}
									break;
				case 11:	Weiyi_Dis[state].Pulse_Rate.shi--;
									if(Weiyi_Dis[state].Pulse_Rate.shi==255)
									{
										Weiyi_Dis[state].Pulse_Rate.shi=9;
									}
									break;
				case 12:	Weiyi_Dis[state].Pulse_Rate.ge--;
									if(Weiyi_Dis[state].Pulse_Rate.ge==255)
									{
										Weiyi_Dis[state].Pulse_Rate.ge=9;
									}
									break;
				case 13:	Weiyi_Dis[state].Pulse_Acc.wan--;
									if(Weiyi_Dis[state].Pulse_Acc.wan==255)
									{
										Weiyi_Dis[state].Pulse_Acc.wan=9;
									}
									break;
				case 14:	Weiyi_Dis[state].Pulse_Acc.qian--;
									if(Weiyi_Dis[state].Pulse_Acc.qian==255)
									{
										Weiyi_Dis[state].Pulse_Acc.qian=9;
									}
									break;
					case 15:	Weiyi_Dis[state].Pulse_Acc.bai--;
									if(Weiyi_Dis[state].Pulse_Acc.bai==255)
									{
										Weiyi_Dis[state].Pulse_Acc.bai=9;
									}
									break;
					case 16:	Weiyi_Dis[state].Pulse_Acc.shi--;
									if(Weiyi_Dis[state].Pulse_Acc.shi==255)
									{
										Weiyi_Dis[state].Pulse_Acc.shi=9;
									}
									break;
					case 17:	Weiyi_Dis[state].Pulse_Acc.ge--;
									if(Weiyi_Dis[state].Pulse_Acc.ge==255)
									{
										Weiyi_Dis[state].Pulse_Acc.ge=9;
									}
									break;							
			}
				Weiyi[state].Pulse_Num=Weiyi_Dis[state].Pulse_Num.shiwan*100000
														+Weiyi_Dis[state].Pulse_Num.wan*10000
			                      +Weiyi_Dis[state].Pulse_Num.qian*1000
														+Weiyi_Dis[state].Pulse_Num.bai*100
														+Weiyi_Dis[state].Pulse_Num.shi*10
														+Weiyi_Dis[state].Pulse_Num.ge;//得到的其实是位移距离
			 Weiyi[state].Pulse_Num=Weiyi[state].Pulse_Num*PerPulseNum/PerDistance;//得到脉冲个数
			
				Weiyi[state].Pulse_Rate=Weiyi_Dis[state].Pulse_Rate.wan*10000
			                      +Weiyi_Dis[state].Pulse_Rate.qian*1000
														+Weiyi_Dis[state].Pulse_Rate.bai*100
														+Weiyi_Dis[state].Pulse_Rate.shi*10
														+Weiyi_Dis[state].Pulse_Rate.ge;
				Weiyi[state].Speed_Acc=Weiyi_Dis[state].Pulse_Acc.wan*10000
			                      +Weiyi_Dis[state].Pulse_Acc.qian*1000
														+Weiyi_Dis[state].Pulse_Acc.bai*100
														+Weiyi_Dis[state].Pulse_Acc.shi*10
														+Weiyi_Dis[state].Pulse_Acc.ge;
		}
		if(edit_statement==4)//系统设置
		{
			switch(Pinpoint_Num)
			{
				case 1: Select_Pro--;
								if(Select_Pro==0)
								{
									Select_Pro=3;
								}
								break;	
        case 2:if(Mode==Auto_Mode)
				        {
									Mode=Hand_Mode;
								}
								else
								{
									Mode=Auto_Mode;
								}
			
		  }
			
		}
		if(edit_statement==5)//系统设置
		{
			switch(Pinpoint_Num)
			{
		
				case 1:Pulse_Dangliang.wan--;
								if(Pulse_Dangliang.wan==255)
								{
									Pulse_Dangliang.wan=9;
								}
								break;
				case 2:Pulse_Dangliang.qian--;
								if(Pulse_Dangliang.qian==255)
								{
									Pulse_Dangliang.qian=9;
								}
								break;
				case 3:Pulse_Dangliang.bai--;
								if(Pulse_Dangliang.bai==255)
								{
									Pulse_Dangliang.bai=9;
								}
								break;
				case 4:Pulse_Dangliang.shi--;
								if(Pulse_Dangliang.shi==255)
								{
									Pulse_Dangliang.shi=9;
								}
								break;
				case 5:Pulse_Dangliang.ge--;
								if(Pulse_Dangliang.ge==255)
								{
									Pulse_Dangliang.ge=9;
								}
								break;
				case 6:Distance_Per.wan--;
								if(Distance_Per.wan==255)
								{
									Distance_Per.wan=9;
								}
								break;
				case 7: Distance_Per.qian--;
								if(Distance_Per.qian==255)
								{
									Distance_Per.qian=9;
								}
								break;
				case 8:Distance_Per.bai--;
								if(Distance_Per.bai==255)
								{
									Distance_Per.bai=9;
								}
								break;		
				case 9:Distance_Per.shi--;
								if(Distance_Per.shi==255)
								{
									Distance_Per.shi=9;
								}
								break;
				case 10:Distance_Per.ge--;
								if(Distance_Per.ge==255)
								{
									Distance_Per.ge=9;
								}
								break;								
			}
				PerPulseNum=Pulse_Dangliang.wan*10000
			              +Pulse_Dangliang.qian*1000
										+Pulse_Dangliang.bai*100
										+Pulse_Dangliang.shi*10
										+Pulse_Dangliang.ge;
				PerDistance=Distance_Per.wan*10000
			              +Distance_Per.qian*1000
									  +Distance_Per.bai*100
										+Distance_Per.shi*10
										+Distance_Per.ge;
		}
		
		if(edit_statement==6)//系统设置
		{
			switch(Pinpoint_Num)
			{
		
				case 1:Dot_Pulse.wan--;
								if(Dot_Pulse.wan==255)
								{
									Dot_Pulse.wan=9;
								}
								break;
				case 2:Dot_Pulse.qian--;
								if(Dot_Pulse.qian==255)
								{
									Dot_Pulse.qian=9;
								}
								break;
				case 3:Dot_Pulse.bai--;
								if(Dot_Pulse.bai==255)
								{
									Dot_Pulse.bai=9;
								}
								break;
				case 4:Dot_Pulse.shi--;
								if(Dot_Pulse.shi==255)
								{
									Dot_Pulse.shi=9;
								}
								break;
				case 5:Dot_Pulse.ge--;
								if(Dot_Pulse.ge==255)
								{
									Dot_Pulse.ge=9;
								}
								break;
				case 6:Constant_Speed.wan--;
								if(Constant_Speed.wan==255)
								{
									Constant_Speed.wan=9;
								}
								break;
				case 7: Constant_Speed.qian--;
								if(Constant_Speed.qian==255)
								{
									Constant_Speed.qian=9;
								}
								break;
				case 8:Constant_Speed.bai--;
								if(Constant_Speed.bai==255)
								{
									Constant_Speed.bai=9;
								}
								break;		
				case 9:Constant_Speed.shi--;
								if(Constant_Speed.shi==255)
								{
									Constant_Speed.shi=9;
								}
								break;
				case 10:Constant_Speed.ge--;
								if(Constant_Speed.ge==255)
								{
									Constant_Speed.ge=9;
								}
								break;								
			}
			
		}


}
void Move_LR_Service(void)//左右移服务函数
{
	int i;
	if(edit_statement==0)
	{
		
	}
	if(edit_statement==2)//编辑位移状态
	{
			LCD_Fill(180,57, 288, 60,WHITE);
			LCD_Fill(180,117, 288, 120,WHITE);
			LCD_Fill(198,177, 288, 180,WHITE);
			LCD_Fill(198,237, 288, 240,WHITE);
		  if(Pinpoint_Num<=1)
			{
				for(i=0;i<=2;i++)
				{
					//showhanzi32(254,20,93);
					LCD_DrawLine(254, 57+i, 286, 57+i);//横线出现
				}
			}
			else if(Pinpoint_Num<=7)
			{
				for(i=0;i<=2;i++)
				{
					LCD_DrawLine(144+Pinpoint_Num*18, 117+i, 162+18*Pinpoint_Num, 117+i);
				}
			}
			else if(Pinpoint_Num<=12)
			{
				for(i=0;i<=2;i++)
				{
					LCD_DrawLine(162+(Pinpoint_Num-6)*18, 177+i, 180+18*(Pinpoint_Num-6), 177+i);
				}
			}	
			else if(Pinpoint_Num<=17)
			{
				for(i=0;i<=2;i++)
				{
					LCD_DrawLine(162+(Pinpoint_Num-11)*18, 237+i, 180+18*(Pinpoint_Num-11),237+i);
				}
			}
 }
	if(edit_statement==4)//编辑系统
	{
			LCD_Fill(180,117, 288, 120,WHITE);
			LCD_Fill(198,177, 288, 180,WHITE);
			LCD_Fill(198,237, 288, 240,WHITE);
				if(Pinpoint_Num==1)
			{
				for(i=0;i<=2;i++)
				{
					LCD_DrawLine(270, 117+i, 288, 117+i);
				}
			}
			else if(Pinpoint_Num==2)
			{
				for(i=0;i<=2;i++)
				{
					LCD_DrawLine(222, 177+i, 286, 177+i);
				}
			}			
  }
		if(edit_statement==5)//编辑系统2
	{
			LCD_Fill(198,177, 288, 180,WHITE);
			LCD_Fill(198,237, 288, 240,WHITE);
		
			if(Pinpoint_Num<=5)
			{
				for(i=0;i<=2;i++)
				{
					LCD_DrawLine(198+(Pinpoint_Num-1)*18, 177+i, 216+18*(Pinpoint_Num-1), 177+i);
				}
			}	
			else if(Pinpoint_Num<=10)
			{
				for(i=0;i<=2;i++)
				{
					LCD_DrawLine(198+(Pinpoint_Num-6)*18, 237+i, 216+18*(Pinpoint_Num-6),237+i);
				}
			}
   }
			if(edit_statement==6)//编辑系统3
	{
			LCD_Fill(198,177, 288, 180,WHITE);
			LCD_Fill(198,237, 288, 240,WHITE);
		
			if(Pinpoint_Num<=5)
			{
				for(i=0;i<=2;i++)
				{
					LCD_DrawLine(198+(Pinpoint_Num-1)*18, 177+i, 216+18*(Pinpoint_Num-1), 177+i);
				}
			}	
			else if(Pinpoint_Num<=10)
			{
				for(i=0;i<=2;i++)
				{
					LCD_DrawLine(198+(Pinpoint_Num-6)*18, 237+i, 216+18*(Pinpoint_Num-6),237+i);
				}
			}
   }
}
void Short_Line_flicker(void)//短线闪烁
{
	  int i,j;
	 	if(edit_statement==0)//编辑指令
	{
		if(flicker_count==2000)//在程序中计数
		{
			for(j=1;j<=3;j++)//显示代码输入的横线段
		  {
					 LCD_DrawLine(60,50*(command_num%5+1)-30+32+j, 140,50*(command_num%5+1)-30+32+j);
			}
		}
		if(flicker_count>4000)
		{
			LCD_Fill(60, 50*(command_num%5+1)-30+32+1, 140,50*(command_num%5+1)-30+32+3,WHITE);//横线清0
			flicker_count=0;
		//	LED1=~LED1;
	  }
	}
		if(edit_statement==1)//编辑操作数
	{
		if(command[command_num]==7||command[command_num]==1||command[command_num]==8||command[command_num]==20||command[command_num]==21)//1、7指令的闪烁
		{
			if(flicker_count==2000)//在程序中计数
			{
				for(j=1;j<=3;j++)//显示代码输入的横线段
				{
						 LCD_DrawLine(150+35*(Pinpoint_Edit-1),50*(command_num%5+1)-30+32+j, 150+35*(Pinpoint_Edit-1)+25,50*(command_num%5+1)-30+32+j);
				}
			}
			if(flicker_count==4000)
			{
				for(j=1;j<=3;j++)
				{
					LCD_Fill(150+35*(Pinpoint_Edit-1),50*(command_num%5+1)-30+32+j, 150+35*(Pinpoint_Edit-1)+25,50*(command_num%5+1)-30+32+j,WHITE);//横线清0
					flicker_count=0;
				//	LED1=~LED1;
				}
			}
		}
			if(command[command_num]==2||command[command_num]==3)//2,3三个指令的闪烁
		{
			if(flicker_count==2000)//在程序中计数
			{
				for(j=1;j<=3;j++)//显示代码输入的横线段
				{
						 LCD_DrawLine(150+35*(Pinpoint_Edit-1),50*(command_num%5+1)-30+32+j, 150+35*(Pinpoint_Edit-1)+25,50*(command_num%5+1)-30+32+j);
				}
			}
			if(flicker_count==4000)
			{
				for(j=1;j<=3;j++)
				{
					LCD_Fill(150+35*(Pinpoint_Edit-1),50*(command_num%5+1)-30+32+j, 150+35*(Pinpoint_Edit-1)+25,50*(command_num%5+1)-30+32+j,WHITE);//横线清0
					flicker_count=0;
				//	LED1=~LED1;
				}
			}
		}
			if(command[command_num]==4||command[command_num]==5||command[command_num]==6)//4,5,6三个指令的闪烁
		{
			if(flicker_count==2000)//在程序中计数
			{
				for(j=1;j<=3;j++)//显示代码输入的横线段
				{
						 LCD_DrawLine(150+35*(2*Pinpoint_Edit-5),50*(command_num%5+1)-30+32+j, 150+35*(2*Pinpoint_Edit-5)+25,50*(command_num%5+1)-30+32+j);
				}
			}
			if(flicker_count==4000)
			{
				for(j=1;j<=3;j++)
				{
					LCD_Fill(150+35*(2*Pinpoint_Edit-5),50*(command_num%5+1)-30+32+j, 150+35*(2*Pinpoint_Edit-5)+25,50*(command_num%5+1)-30+32+j,WHITE);//横线清0
					flicker_count=0;
					//LED1=~LED1;
				}
			}
		}
		
	}
		if(edit_statement==2)//编辑位移段
	{
		if(Pinpoint_Num<=1)
		{
			if(flicker_count==50)//在程序中计数
			{
				for(i=0;i<=2;i++)
				{
					//showhanzi32(254,20,93);
					LCD_DrawLine(254, 57+i, 286, 57+i);//横线出现
				}
			}
			if(flicker_count==100)
			{
				LCD_Fill(254, 57, 286,60,WHITE);//横线清0
				flicker_count=0;
			//	LED0=~LED0;
			}
			
		}
		else if(Pinpoint_Num<=7)//设置位移分成3行
		{
			if(flicker_count==50)//在程序中计数
			{
				for(i=0;i<=2;i++)
				{
					LCD_DrawLine(144+Pinpoint_Num*18, 117+i, 162+18*Pinpoint_Num, 117+i);//横线出现
				}
			}
			if(flicker_count==100)
			{
				LCD_Fill(144+Pinpoint_Num*18, 117, 162+18*Pinpoint_Num, 120,WHITE);//横线清0
				flicker_count=0;
			//	LED0=~LED0;
			}

		}
		else if(Pinpoint_Num<=12)
		{
			if(flicker_count==50)
			{
				for(i=0;i<=2;i++)
				{
					LCD_DrawLine(162+(Pinpoint_Num-6)*18, 177+i, 180+18*(Pinpoint_Num-6), 177+i);
				}
			}
			if(flicker_count==100)
			{
				LCD_Fill(162+(Pinpoint_Num-6)*18, 177, 180+18*(Pinpoint_Num-6), 180,WHITE);
				flicker_count=0;
				//LED0=~LED0;
			}

		}
		else if(Pinpoint_Num<=17)
		{
			if(flicker_count==50)
			{
				for(i=0;i<=2;i++)
				{
					LCD_DrawLine(162+(Pinpoint_Num-11)*18, 237+i, 180+18*(Pinpoint_Num-11),237+i);
				}
			}
			if(flicker_count==100)
			{
				LCD_Fill(162+(Pinpoint_Num-11)*18, 237, 180+18*(Pinpoint_Num-11), 240,WHITE);
				flicker_count=0;
				//LED0=~LED0;
			}
		}
	}
			if(edit_statement==4)//编辑系统
	{
		if(Pinpoint_Num==1)//设置位移分成3行
		{
			if(flicker_count==50)//在程序中计数
			{
				for(i=0;i<=2;i++)
				{
					LCD_DrawLine(270, 117+i, 288, 117+i);//横线出现
				}
			}
			if(flicker_count==100)
			{
				LCD_Fill(270, 117, 288, 120,WHITE);//横线清0
				flicker_count=0;
		//		LED0=~LED0;
			}

		}
		else if(Pinpoint_Num==2)
		{
			if(flicker_count==50)
			{
				for(i=0;i<=2;i++)
				{
					LCD_DrawLine(222, 177+i,286 , 177+i);
				}
			}
			if(flicker_count==100)
			{
				LCD_Fill(222, 177, 286, 180,WHITE);
				flicker_count=0;
			//	LED0=~LED0;
			}

		}

	}
			if(edit_statement==5)//编辑系统
	{

		 if(Pinpoint_Num<=5)
		{
			if(flicker_count==50)
			{
				for(i=0;i<=2;i++)
				{
					LCD_DrawLine(198+(Pinpoint_Num-1)*18, 177+i, 216+18*(Pinpoint_Num-1), 177+i);
				}
			}
			if(flicker_count==100)
			{
				LCD_Fill(198+(Pinpoint_Num-1)*18, 177, 216+18*(Pinpoint_Num-1), 180,WHITE);
				flicker_count=0;
			//	LED0=~LED0;
			}

		}
		else if(Pinpoint_Num<=10)
		{
			if(flicker_count==50)
			{
				for(i=0;i<=2;i++)
				{
					LCD_DrawLine(198+(Pinpoint_Num-6)*18, 237+i, 216+18*(Pinpoint_Num-6),237+i);
				}
			}
			if(flicker_count==100)
			{
				LCD_Fill(198+(Pinpoint_Num-6)*18, 237, 216+18*(Pinpoint_Num-6), 240,WHITE);
				flicker_count=0;
			//	LED0=~LED0;
			}
		}
	}
			if(edit_statement==6)//编辑系统
	{

		 if(Pinpoint_Num<=5)
		{
			if(flicker_count==50)
			{
				for(i=0;i<=2;i++)
				{
					LCD_DrawLine(198+(Pinpoint_Num-1)*18, 177+i, 216+18*(Pinpoint_Num-1), 177+i);
				}
			}
			if(flicker_count==100)
			{
				LCD_Fill(198+(Pinpoint_Num-1)*18, 177, 216+18*(Pinpoint_Num-1), 180,WHITE);
				flicker_count=0;
			//	LED0=~LED0;
			}

		}
		else if(Pinpoint_Num<=10)
		{
			if(flicker_count==50)
			{
				for(i=0;i<=2;i++)
				{
					LCD_DrawLine(198+(Pinpoint_Num-6)*18, 237+i, 216+18*(Pinpoint_Num-6),237+i);
				}
			}
			if(flicker_count==100)
			{
				LCD_Fill(198+(Pinpoint_Num-6)*18, 237, 216+18*(Pinpoint_Num-6), 240,WHITE);
				flicker_count=0;
			//	LED0=~LED0;
			}
		}
	}
	
	

}
void input_command(void)//扫描某个按键按下
{
	u8 i,j,k;
	if(init_lock)//切换状态时初始化一遍
	{
		start_init();
		edit_init();
		init_lock=0;
		flicker_count=0;
		display_command();
	}
	
	//display_command();
	if(save_ok==0)
	{
	 flicker_count++;//闪烁计数自加
	Short_Line_flicker();
	}
	
	tp_dev.scan(0);
		if(tp_dev.sta&TP_PRES_DOWN)			//触摸屏被按下
	{	
		delay_ms(10);
		if(tp_dev.sta&TP_PRES_DOWN)
		{
			if(uc_lock==1)//防止重复按下
			{
				if(tp_dev.x>300&&tp_dev.x<380&&tp_dev.y>15&&tp_dev.y<61)//翻页键
				{
					for(k=0;k<=100;k++)
					{
						if(command[k]==0)
						{
							command_max=k-1;
							if(command_max==255)
							{
								command_max=0;
							}
							break;
						}
					}
					page_max=command_max/5+1;
				  if(((command_max%5)==4||page<page_max)&&(page<=((command_max+1)/5)))
					{
						page++;
						if(page>page_max)
						{
							page_max=page;
						}
					}
					else
					{
						page=1;
					}
					command_num=page*5-5;//翻页后命令起始于第一行
					edit_statement=0;//翻页后编辑状态为编辑指令
					display_command();
//					Pinpoint_Num=1;
//				  state++;
//					if(state==17)
//					{
//						state=1;
//					}
					uc_lock=0;
//					init_lock=1;
    
				}
				if(tp_dev.x>390&&tp_dev.x<470&&tp_dev.y>15&&tp_dev.y<61)//位移键
				{
					edit_statement=4;
					init_lock=1;
					uc_lock=0;
				}
				if(tp_dev.x>300&&tp_dev.x<380&&tp_dev.y>65&&tp_dev.y<111)//左移键    适用于编辑操作数和位移
				{				
					if(edit_statement==1)
					{
						if(command[command_num]==2||command[command_num]==3)
						{
							 for(j=1;j<=3;j++)//左移时显示前一段的小短横
							{
								LCD_DrawLine(150+35*(Pinpoint_Edit-1),50*(command_num%5+1)-30+32+j, 150+35*(Pinpoint_Edit-1)+25,50*(command_num%5+1)-30+32+j);
							}
						}
						if(command[command_num]==4||command[command_num]==5||command[command_num]==6)
						{
							 for(j=1;j<=3;j++)//左移时显示前一段的小短横
							{
								LCD_DrawLine(150+35*(2*Pinpoint_Edit-5),50*(command_num%5+1)-30+32+j, 150+35*(2*Pinpoint_Edit-5)+25,50*(command_num%5+1)-30+32+j);
							}
						}
							if(command[command_num]==7)
						{
							 for(j=1;j<=3;j++)//左移时显示前一段的小短横
							{
								LCD_DrawLine(150+35*(Pinpoint_Edit-1),50*(command_num%5+1)-30+32+j, 150+35*(Pinpoint_Edit-1)+25,50*(command_num%5+1)-30+32+j);
							}
						}
						
//							if(command[command_num]==1)
//						{
//							Pinpoint_Edit--;
//							if(Pinpoint_Edit==2)
//							{
//								Pinpoint_Edit=4;
//							}
//						}
						
						
						if(command[command_num]==2)
						{
							Pinpoint_Edit--;
							if(Pinpoint_Edit==0)
							{
								Pinpoint_Edit=4;
							}
						}
						if(command[command_num]==3)
						{
							Pinpoint_Edit--;
							if(Pinpoint_Edit==0)
							{
								Pinpoint_Edit=4;
							}
						}
						if(command[command_num]==4)
						{
							Pinpoint_Edit--;
							if(Pinpoint_Edit==2)
							{
								Pinpoint_Edit=4;
							}
						}
						if(command[command_num]==5)
						{
							Pinpoint_Edit--;
							if(Pinpoint_Edit==2)
							{
								Pinpoint_Edit=4;
							}
						}
						if(command[command_num]==6)
						{
							Pinpoint_Edit--;
							if(Pinpoint_Edit==2)
							{
								Pinpoint_Edit=4;
							}
						}
						if(command[command_num]==7)
						{
							Pinpoint_Edit--;
							if(Pinpoint_Edit==2)
							{
								Pinpoint_Edit=4;
							}
						}
						if(command[command_num]==8)
						{
							Pinpoint_Edit--;
							if(Pinpoint_Edit==2)
							{
								Pinpoint_Edit=4;
							}
						}
							if(command[command_num]==20)
						{
							Pinpoint_Edit--;
							if(Pinpoint_Edit==2)
							{
								Pinpoint_Edit=4;
							}
						}
						if(command[command_num]==21)
						{
							Pinpoint_Edit--;
							if(Pinpoint_Edit==2)
							{
								Pinpoint_Edit=4;
							}
						}
						uc_lock=0;
					}
					
					
					
					
					if(edit_statement==2)
					{
						Pinpoint_Num--;
						if(Pinpoint_Num==0)
						{
							Pinpoint_Num=16;
						}
						uc_lock=0;
						Move_LR_Service();
					}
				}
				if(tp_dev.x>390&&tp_dev.x<470&&tp_dev.y>65&&tp_dev.y<111)//右移键
				{
					if(edit_statement==0)
					{
						if(command[command_num]>1)//只有不是空指令和位移时才右移
						{
							for(j=1;j<=3;j++)//显示代码输入的横线段
							{
							LCD_DrawLine(60,50*(command_num%5+1)-30+32+j, 140,50*(command_num%5+1)-30+32+j);
							}
						  edit_statement=1;
						  Pinpoint_Edit=0;
							if(command[command_num]==4||command[command_num]==5||command[command_num]==6||command[command_num]==7||command[command_num]==8||command[command_num]==20||command[command_num]==21)
							{
								Pinpoint_Edit=2;
							}
							uc_lock=0;
						}
					}
					
					
					
					if(edit_statement==1)
					{
						if(command[command_num]==2||command[command_num]==3)
						{
							 for(j=1;j<=3;j++)//右移时显示前一段的小短横
							{
								LCD_DrawLine(150+35*(Pinpoint_Edit-1),50*(command_num%5+1)-30+32+j, 150+35*(Pinpoint_Edit-1)+25,50*(command_num%5+1)-30+32+j);
							}
						}
						if(command[command_num]==4||command[command_num]==5||command[command_num]==6)
						{
							 for(j=1;j<=3;j++)//右移时显示前一段的小短横
							{
								LCD_DrawLine(150+35*(2*Pinpoint_Edit-5),50*(command_num%5+1)-30+32+j, 150+35*(2*Pinpoint_Edit-5)+25,50*(command_num%5+1)-30+32+j);
							}
						}
							if(command[command_num]==7)
						{
							 for(j=1;j<=3;j++)//右移时显示前一段的小短横
							{
								LCD_DrawLine(150+35*(Pinpoint_Edit-1),50*(command_num%5+1)-30+32+j, 150+35*(Pinpoint_Edit-1)+25,50*(command_num%5+1)-30+32+j);
							}
						}
					
//						if(command[command_num]==1)
//						{
//							Pinpoint_Edit++;
//							if(Pinpoint_Edit==5)
//							{
//								Pinpoint_Edit=3;
//							}
//						}
					
						if(command[command_num]==2)
						{
							Pinpoint_Edit++;
							if(Pinpoint_Edit==5)
							{
								Pinpoint_Edit=1;
							}
						}
						if(command[command_num]==3)
						{
							Pinpoint_Edit++;
							if(Pinpoint_Edit==5)
							{
								Pinpoint_Edit=1;
							}
						}
						if(command[command_num]==4)
						{
							Pinpoint_Edit++;
							if(Pinpoint_Edit==5)
							{
								Pinpoint_Edit=3;
							}
						}
						if(command[command_num]==5)
						{
							Pinpoint_Edit++;
							if(Pinpoint_Edit==5)
							{
								Pinpoint_Edit=3;
							}
						}
						if(command[command_num]==6)
						{
							Pinpoint_Edit++;
							if(Pinpoint_Edit==5)
							{
								Pinpoint_Edit=3;
							}
						}
						if(command[command_num]==7)
						{
							Pinpoint_Edit++;
							if(Pinpoint_Edit==5)
							{
								Pinpoint_Edit=3;
							}
						}
						if(command[command_num]==8)
						{
							Pinpoint_Edit++;
							if(Pinpoint_Edit==5)
							{
								Pinpoint_Edit=3;
							}
						}
						if(command[command_num]==20)
						{
							Pinpoint_Edit++;
							if(Pinpoint_Edit==5)
							{
								Pinpoint_Edit=3;
							}
						}
						if(command[command_num]==21)
						{
							Pinpoint_Edit++;
							if(Pinpoint_Edit==5)
							{
								Pinpoint_Edit=3;
							}
						}
						uc_lock=0;
					}
					
					
					
					
					if(edit_statement==2)
					{
						Pinpoint_Num++;
						if(Pinpoint_Num==17)
						{
							Pinpoint_Num=1;
						}
						uc_lock=0;
						Move_LR_Service();
					}
				}
				if(tp_dev.x>300&&tp_dev.x<380&&tp_dev.y>115&&tp_dev.y<161)//加键
				{
					uc_lock=0;
				 jia_service();
					
				}
				if(tp_dev.x>390&&tp_dev.x<470&&tp_dev.y>115&&tp_dev.y<161)//减键
				{
					uc_lock=0;
				 jian_service();
					
				}
				if(tp_dev.x>300&&tp_dev.x<380&&tp_dev.y>165&&tp_dev.y<211)//删除
				{
					for(i=0;(i+command_num)<=100;i++)
					{
						command[command_num+i]=command[command_num+i+1];
						command_edit_num_wei[command_num+i][1]=command_edit_num_wei[command_num+i+1][1];
						command_edit_num_wei[command_num+i][2]=command_edit_num_wei[command_num+i+1][2];
						command_edit_num_wei[command_num+i][3]=command_edit_num_wei[command_num+i+1][3];
						command_edit_num_wei[command_num+i][4]=command_edit_num_wei[command_num+i+1][4];
					}
					uc_lock=0;
					display_command();
				}
				if(tp_dev.x>390&&tp_dev.x<470&&tp_dev.y>165&&tp_dev.y<211)//添加
				{
					for(i=99;i>command_num;i--)
					{
						command[i]=command[i-1];
						command_edit_num_wei[i][1]=command_edit_num_wei[i-1][1];
						command_edit_num_wei[i][2]=command_edit_num_wei[i-1][2];
						command_edit_num_wei[i][3]=command_edit_num_wei[i-1][3];
						command_edit_num_wei[i][4]=command_edit_num_wei[i-1][4];
					}
					command[command_num]=0;
					command_edit_num_wei[command_num][1]=0;
					command_edit_num_wei[command_num][2]=0;
					command_edit_num_wei[command_num][3]=0;
					command_edit_num_wei[command_num][4]=0;
					uc_lock=0;
					display_command();
				}
				if(tp_dev.x>300&&tp_dev.x<380&&tp_dev.y>215&&tp_dev.y<261)//保存
				{
					save_command();
				}
					if(tp_dev.x>390&&tp_dev.x<470&&tp_dev.y>215&&tp_dev.y<261)//运行
				{
					if(save_ok==1)//
					{
								run_num=0;
								ScanCommandNum=0;//每次运行前初始化扫描指令个数为0；
								for(i=0;i<=9;i++)
								{
									ScanCommand[i]=0;
								}
								for(i=0;i<=99;i++)//每次运行前计算位移
								{
									if(command[i]==1)
									{
										Weiyi[i].Pulse_Num=Weiyi_Dis[i].Pulse_Num.shiwan*100000
																	+Weiyi_Dis[i].Pulse_Num.wan*10000
																	+Weiyi_Dis[i].Pulse_Num.qian*1000
																	+Weiyi_Dis[i].Pulse_Num.bai*100
																	+Weiyi_Dis[i].Pulse_Num.shi*10
																	+Weiyi_Dis[i].Pulse_Num.ge;//得到的是位移距离
										Weiyi[i].Pulse_Num=Weiyi[i].Pulse_Num*PerPulseNum/PerDistance;//得到脉冲个数
							
							
										Weiyi[i].Pulse_Rate=Weiyi_Dis[i].Pulse_Rate.wan*10000
																	+Weiyi_Dis[i].Pulse_Rate.qian*1000
																	+Weiyi_Dis[i].Pulse_Rate.bai*100
																	+Weiyi_Dis[i].Pulse_Rate.shi*10
																	+Weiyi_Dis[i].Pulse_Rate.ge;
										Weiyi[i].Speed_Acc=Weiyi_Dis[i].Pulse_Acc.wan*10000
																	+Weiyi_Dis[i].Pulse_Acc.qian*1000
																	+Weiyi_Dis[i].Pulse_Acc.bai*100
																	+Weiyi_Dis[i].Pulse_Acc.shi*10
																	+Weiyi_Dis[i].Pulse_Acc.ge;
									}
									
								}
								for(i=0;i<=99;i++)//判断是否有扫描指令
								{
									if(command[i]==6)
									{
										ScanCommand[ScanCommandNum]=i;
										ScanCommandNum++;
									}
								}
								if(ScanCommandNum>0)//有扫描指令
								{
									edit_statement=7;
								}
								else
								{
									edit_statement=3;
									uc_lock=0;
								}
								 showhanzi32(395,220,30);//停止
								 showhanzi32(430,220,31);
							//	delay_ms(1000);
								//delay_ms(1000);
					}
					else
					{
						  LCD_Fill(80, 80, 250,150,WHITE);
	            LCD_DrawRectangle(80,80 ,250,150);
							showhanzi32(100,100,95);//请先
							showhanzi32(132,100,96);
							showhanzi32(164,100,77);//保存
							showhanzi32(196,100,78);
						  delay_ms(1000);
            	delay_ms(1000);
	            LCD_Fill(0, 0, 290,280,BACK_COL);
	            display_command();
					}
				}
				for(i=1;i<=5;i++)//判断是否是指令段按下
				{
					if(tp_dev.x>80&&tp_dev.x<160&&tp_dev.y>(50*i-44)&&tp_dev.y<(50*i+3))
					{
						save_ok=0;//可编辑状态
						
						
						if(edit_statement==0)//切换状态时将原来的横线补上
						{
							for(j=1;j<=3;j++)//显示代码输入的横线段
							{
							LCD_DrawLine(60,50*(command_num%5+1)-30+32+j, 140,50*(command_num%5+1)-30+32+j);
							}
						}
							if(edit_statement==1)//切换状态时将原来的横线补上
						{
							for(j=1;j<=3;j++)//显示代码输入的横线段
							{
							 LCD_DrawLine(150+35*(Pinpoint_Edit-1),50*(command_num%5+1)-30+32+j, 150+35*(Pinpoint_Edit-1)+25,50*(command_num%5+1)-30+32+j);
							}
						}
						command_num=i+(page-1)*5-1;
						edit_statement=0;//切换状态
						display_command();
					}
				}
					for(i=1;i<=5;i++)//判断是否是操作数按下
				{
					if(tp_dev.x>170&&tp_dev.x<280&&tp_dev.y>(50*i-44)&&tp_dev.y<(50*i+3))
					{
						save_ok=0;//可编辑状态
						command_num=i+(page-1)*5-1;//得到第几行按下
						
						if(command[command_num]==1)
						{
							edit_statement=2;
							init_lock=1;
						  state=command_num;//将当前第几段位移赋值给state
						}
					
					}
				}
				
				
				
				
				
				
				
				
				

			}
			
		}
	}

	else
	{
		uc_lock=1;
	}
	
}
void display_command(void)//除了显示指令，还有计算指令操作数，按完按键之后调用
{
	u8 i,j,m;
	for(i=0;i<=99;i++)
	{
		if(command[i]==0)
		{
			  command_edit_num[i]=0;
		}
		else
		{
		  command_edit_num[i]=command_edit_num_wei[i][1]*1000
												 +command_edit_num_wei[i][2]*100
												 +command_edit_num_wei[i][3]*10
												 +command_edit_num_wei[i][4];			
		}
	}
	for(i=1;i<=5;i++)//显示代码标号
	{
		LCD_ShowNum_32(10,50*i-30,(page-1)*5/10);
		LCD_ShowNum_32(26,50*i-30,(page-1)*5%10+i-1);
	}
	for(i=1;i<=5;i++)//显示指令
	{
		LCD_Fill(60,50*i-30,140,50*i+2,WHITE);//清除汉字
		LCD_Fill(150,50*i-30,280,50*i+2,WHITE);
		if(command[i+(page-1)*5-1]==0)
		{
				for(j=1;j<=3;j++)//显示代码输入的横线段
			{
			 LCD_DrawLine(60,50*i-30+32+j, 140,50*i-30+32+j);
			 LCD_DrawLine(150,50*i-30+32+j, 280,50*i-30+32+j);
			}	
		}
		if(command[i+(page-1)*5-1]==1)//位移
		{
			LCD_Fill(150, 50*i-30+32+1, 280,50*i-30+32+3,WHITE);//横线清0
				for(j=1;j<=3;j++)//显示代码输入的横线段
			{
			 LCD_DrawLine(60,50*i-30+32+j, 140,50*i-30+32+j);
			 LCD_DrawLine(150,50*i-30+32+j, 280,50*i-30+32+j);
			}
		
			
			showhanzi32(60,50*i-30,6);//位移                            
	    showhanzi32(100,50*i-30,7);
			
			
			showhanzi32(150,50*i-30,43);//点击
			showhanzi32(182,50*i-30,44);
			showhanzi32(214,50*i-30,45);//设置
			showhanzi32(246,50*i-30,46);
			
			//showhanzi32(150,50*i-30,5);//段号
			//showhanzi32(182,50*i-30,52);
			
			//LCD_ShowNum_32(225,50*i-30,command_edit_num_wei[i+(page-1)*5-1][3]);
		//	LCD_ShowNum_32(260,50*i-30,command_edit_num_wei[i+(page-1)*5-1][4]);
			
		}
		if(command[i+(page-1)*5-1]==2)//时间
		{
			showhanzi32(60,50*i-30,32);//时间                           
	    showhanzi32(100,50*i-30,33);
			LCD_Fill(150, 50*i-30+32+1, 280,50*i-30+32+3,WHITE);//横线清0
			for(m=0;m<=3;m++)
			{
				for(j=1;j<=3;j++)
				{
					LCD_DrawLine(150+35*m,50*i-30+32+j, 150+35*m+25,50*i-30+32+j);
				}
			}

					LCD_ShowNum_32(155,50*i-30,command_edit_num_wei[i+(page-1)*5-1][1]);
					LCD_ShowNum_32(190,50*i-30,command_edit_num_wei[i+(page-1)*5-1][2]);
					LCD_ShowNum_32(225,50*i-30,command_edit_num_wei[i+(page-1)*5-1][3]);
					LCD_ShowNum_32(260,50*i-30,command_edit_num_wei[i+(page-1)*5-1][4]);
		}
		if(command[i+(page-1)*5-1]==3)//
		{
			showhanzi32(60,50*i-30,16);//速度                          
	    showhanzi32(100,50*i-30,17);
			LCD_Fill(150, 50*i-30+32+1, 280,50*i-30+32+3,WHITE);//横线清0
			for(m=0;m<=3;m++)
			{
				for(j=1;j<=3;j++)
				{
					LCD_DrawLine(150+35*m,50*i-30+32+j, 150+35*m+25,50*i-30+32+j);
				}
			}
					LCD_ShowNum_32(155,50*i-30,command_edit_num_wei[i+(page-1)*5-1][1]);
					LCD_ShowNum_32(190,50*i-30,command_edit_num_wei[i+(page-1)*5-1][2]);
					LCD_ShowNum_32(225,50*i-30,command_edit_num_wei[i+(page-1)*5-1][3]);
					LCD_ShowNum_32(260,50*i-30,command_edit_num_wei[i+(page-1)*5-1][4]);
		}
		if(command[i+(page-1)*5-1]==4)//输入
		{
			LCD_Fill(150, 50*i-30+32+1, 280,50*i-30+32+3,WHITE);//横线清0
			for(m=0;m<=3;m++)
			{
				for(j=1;j<=3;j++)
				{
					LCD_DrawLine(150+35*m,50*i-30+32+j, 150+35*m+25,50*i-30+32+j);
				}
			}
			showhanzi32(60,50*i-30,34);//输入                          
	    showhanzi32(100,50*i-30,35);
			
			LCD_ShowNum_32(155,50*i-30,10);//X
			showhanzi32(215,50*i-30,51);
			
			LCD_ShowNum_32(190,50*i-30,command_edit_num_wei[i+(page-1)*5-1][3]);
			LCD_ShowNum_32(260,50*i-30,command_edit_num_wei[i+(page-1)*5-1][4]);
		}
		if(command[i+(page-1)*5-1]==5)//输出
		{
			LCD_Fill(150, 50*i-30+32+1, 280,50*i-30+32+3,WHITE);//横线清0
			for(m=0;m<=3;m++)
			{
				for(j=1;j<=3;j++)
				{
					LCD_DrawLine(150+35*m,50*i-30+32+j, 150+35*m+25,50*i-30+32+j);
				}
			}
			showhanzi32(60,50*i-30,34);//输出                          
	    showhanzi32(100,50*i-30,36);
			
			LCD_ShowNum_32(155,50*i-30,11);//Y
			showhanzi32(215,50*i-30,51);//为
			LCD_ShowNum_32(190,50*i-30,command_edit_num_wei[i+(page-1)*5-1][3]);
			LCD_ShowNum_32(260,50*i-30,command_edit_num_wei[i+(page-1)*5-1][4]);
		}
		if(command[i+(page-1)*5-1]==6)//扫描
		{
			LCD_Fill(150, 50*i-30+32+1, 280,50*i-30+32+3,WHITE);//横线清0
			for(m=0;m<=3;m++)
			{
				for(j=1;j<=3;j++)
				{
					LCD_DrawLine(150+35*m,50*i-30+32+j, 150+35*m+25,50*i-30+32+j);
				}
			}
			showhanzi32(60,50*i-30,37);//扫描                          
	    showhanzi32(100,50*i-30,38);
			
			LCD_ShowNum_32(155,50*i-30,10);//X
			showhanzi32(215,50*i-30,51);//为
			LCD_ShowNum_32(190,50*i-30,command_edit_num_wei[i+(page-1)*5-1][3]);
			LCD_ShowNum_32(260,50*i-30,command_edit_num_wei[i+(page-1)*5-1][4]);
		}
		if(command[i+(page-1)*5-1]==7)//跳转
		{
				LCD_Fill(150, 50*i-30+32+1, 280,50*i-30+32+3,WHITE);//横线清0
			for(m=0;m<=3;m++)
			{
				for(j=1;j<=3;j++)
				{
					LCD_DrawLine(150+35*m,50*i-30+32+j, 150+35*m+25,50*i-30+32+j);
				}
			}
			showhanzi32(60,50*i-30,39);//跳转                       
	    showhanzi32(100,50*i-30,40);
			
			LCD_ShowNum_32(155,50*i-30,command_edit_num_wei[i+(page-1)*5-1][1]);
			LCD_ShowNum_32(190,50*i-30,command_edit_num_wei[i+(page-1)*5-1][2]);
			LCD_ShowNum_32(225,50*i-30,command_edit_num_wei[i+(page-1)*5-1][3]);
			LCD_ShowNum_32(260,50*i-30,command_edit_num_wei[i+(page-1)*5-1][4]);
			
		}
			if(command[i+(page-1)*5-1]==8)//循环
		{
				LCD_Fill(150, 50*i-30+32+1, 280,50*i-30+32+3,WHITE);//横线清0
				for(j=1;j<=3;j++)//显示代码输入的横线段
			{
			 LCD_DrawLine(60,50*i-30+32+j, 140,50*i-30+32+j);
			 LCD_DrawLine(150,50*i-30+32+j, 210,50*i-30+32+j);
			}
				for(m=2;m<=3;m++)
			{
				for(j=1;j<=3;j++)
				{
					LCD_DrawLine(150+35*m,50*i-30+32+j, 150+35*m+25,50*i-30+32+j);
				}
			}
			
			showhanzi32(60,50*i-30,60);//循环                            
	    showhanzi32(100,50*i-30,61);
			
			showhanzi32(150,50*i-30,54);//起点
			showhanzi32(182,50*i-30,55);
			
			LCD_ShowNum_32(225,50*i-30,command_edit_num_wei[i+(page-1)*5-1][3]);
			LCD_ShowNum_32(260,50*i-30,command_edit_num_wei[i+(page-1)*5-1][4]);
		}
			if(command[i+(page-1)*5-1]==9)//结束
		{
			for(j=1;j<=3;j++)//显示代码输入的横线段
			{
			 LCD_DrawLine(60,50*i-30+32+j, 140,50*i-30+32+j);
			 LCD_DrawLine(150,50*i-30+32+j, 280,50*i-30+32+j);
			}
		//	LCD_Fill(150, 50*i-30+32+1, 280,50*i-30+32+3,WHITE);//横线清0
			showhanzi32(60,50*i-30,41);//结束                         
	    showhanzi32(100,50*i-30,42);
		}
			if(command[i+(page-1)*5-1]==20)
		{
				LCD_Fill(150, 50*i-30+32+1, 280,50*i-30+32+3,WHITE);//横线清0
			LCD_Fill(60, 50*i-30+32+1, 140,50*i-30+32+3,WHITE);
				for(j=1;j<=3;j++)//显示代码输入的横线段
			{
			 LCD_DrawLine(150,50*i-30+32+j, 210,50*i-30+32+j);
			}
				for(m=2;m<=3;m++)
			{
				for(j=1;j<=3;j++)
				{
					LCD_DrawLine(150+35*m,50*i-30+32+j, 150+35*m+25,50*i-30+32+j);
				}
			}
			
			
			showhanzi32(150,50*i-30,56);//终点
			showhanzi32(182,50*i-30,57);
			
			LCD_ShowNum_32(225,50*i-30,command_edit_num_wei[i+(page-1)*5-1][3]);
			LCD_ShowNum_32(260,50*i-30,command_edit_num_wei[i+(page-1)*5-1][4]);
		}
			if(command[i+(page-1)*5-1]==21)
		{
				LCD_Fill(150, 50*i-30+32+1, 280,50*i-30+32+3,WHITE);//横线清0
			LCD_Fill(60, 50*i-30+32+1, 140,50*i-30+32+3,WHITE);
				for(j=1;j<=3;j++)//显示代码输入的横线段
			{
			 LCD_DrawLine(150,50*i-30+32+j, 210,50*i-30+32+j);
			}
				for(m=2;m<=3;m++)
			{
				for(j=1;j<=3;j++)
				{
					LCD_DrawLine(150+35*m,50*i-30+32+j, 150+35*m+25,50*i-30+32+j);
				}
			}
			
			
			showhanzi32(150,50*i-30,58);//终点
			showhanzi32(182,50*i-30,59);
			
			LCD_ShowNum_32(225,50*i-30,command_edit_num_wei[i+(page-1)*5-1][3]);
			LCD_ShowNum_32(260,50*i-30,command_edit_num_wei[i+(page-1)*5-1][4]);
		}
	}
}
void save_command(void)
{
	u8 i;
	save_ok=1;
	
		for(i=0;i<=99;i++)//计算存储的位移
  {
		if(command[i]==1)
		{
		Weiyi[i].Pulse_Num=Weiyi_Dis[i].Pulse_Num.shiwan*100000
											+Weiyi_Dis[i].Pulse_Num.wan*10000
											+Weiyi_Dis[i].Pulse_Num.qian*1000
											+Weiyi_Dis[i].Pulse_Num.bai*100
											+Weiyi_Dis[i].Pulse_Num.shi*10
											+Weiyi_Dis[i].Pulse_Num.ge;//得到的是位移距离
									
		Weiyi[i].Pulse_Rate=Weiyi_Dis[i].Pulse_Rate.wan*10000
												+Weiyi_Dis[i].Pulse_Rate.qian*1000
												+Weiyi_Dis[i].Pulse_Rate.bai*100
												+Weiyi_Dis[i].Pulse_Rate.shi*10
												+Weiyi_Dis[i].Pulse_Rate.ge;
		Weiyi[i].Speed_Acc=Weiyi_Dis[i].Pulse_Acc.wan*10000
												+Weiyi_Dis[i].Pulse_Acc.qian*1000
												+Weiyi_Dis[i].Pulse_Acc.bai*100
												+Weiyi_Dis[i].Pulse_Acc.shi*10
												+Weiyi_Dis[i].Pulse_Acc.ge;
		}
									
	}
	if(Select_Pro==1)
	{
		STMFLASH_Write(FLASH_SAVE_ADDR,(u16*)command,SIZE);
		STMFLASH_Write(FLASH_SAVE_EDIT_ADDR,(u16*)command_edit_num,SIZE);
		STMFLASH_Write(FLASH_SAVE_WEIYI_ADDR,(u16*)Weiyi,SIZE);
	}
	if(Select_Pro==2)
	{
		STMFLASH_Write(FLASH_SAVE_ADDR+1600,(u16*)command,SIZE);
		STMFLASH_Write(FLASH_SAVE_EDIT_ADDR+1600,(u16*)command_edit_num,SIZE);
		STMFLASH_Write(FLASH_SAVE_WEIYI_ADDR+1600,(u16*)Weiyi,SIZE);
	}
	if(Select_Pro==3)
	{
		STMFLASH_Write(FLASH_SAVE_ADDR+3200,(u16*)command,SIZE);
		STMFLASH_Write(FLASH_SAVE_EDIT_ADDR+3200,(u16*)command_edit_num,SIZE);
		STMFLASH_Write(FLASH_SAVE_WEIYI_ADDR+3200,(u16*)Weiyi,SIZE);
	}

	LCD_Fill(80, 80, 250,150,WHITE);
	LCD_DrawRectangle(80,80 ,250,150);
	
	showhanzi32(100,100,77);//保存
	showhanzi32(132,100,78);
	showhanzi32(164,100,81);//成功
	showhanzi32(196,100,82);
  delay_ms(1000);
	delay_ms(1000);
	//LCD_Fill(0, 0, 290,280,BACK_COL);
	init_lock=1;
}
void read_command()
{
	u8 i;
		if(Select_Pro==1)
							{
								STMFLASH_Read(FLASH_SAVE_ADDR,(u16*)command,SIZE);
								STMFLASH_Read(FLASH_SAVE_EDIT_ADDR,(u16*)command_edit_num,SIZE);
								STMFLASH_Read(FLASH_SAVE_WEIYI_ADDR,(u16*)Weiyi,SIZE);
							}
							if(Select_Pro==2)
							{
								STMFLASH_Read(FLASH_SAVE_ADDR+1600,(u16*)command,SIZE);
								STMFLASH_Read(FLASH_SAVE_EDIT_ADDR+1600,(u16*)command_edit_num,SIZE);
								STMFLASH_Read(FLASH_SAVE_WEIYI_ADDR+1600,(u16*)Weiyi,SIZE);
							}
							if(Select_Pro==3)
							{
								STMFLASH_Read(FLASH_SAVE_ADDR+3200,(u16*)command,SIZE);
								STMFLASH_Read(FLASH_SAVE_EDIT_ADDR+3200,(u16*)command_edit_num,SIZE);
								STMFLASH_Read(FLASH_SAVE_WEIYI_ADDR+3200,(u16*)Weiyi,SIZE);
							}
							for(i=0;i<100;i++)
							{
								command_edit_num_wei[i][1]=command_edit_num[i]/1000;
								command_edit_num_wei[i][2]=(command_edit_num[i]-command_edit_num_wei[i][1]*1000)/100;
								command_edit_num_wei[i][3]=(command_edit_num[i]-command_edit_num_wei[i][1]*1000-command_edit_num_wei[i][2]*100)/10;
								command_edit_num_wei[i][4]=command_edit_num[i]-command_edit_num_wei[i][1]*1000-command_edit_num_wei[i][2]*100-command_edit_num_wei[i][3]*10;
							 
								Weiyi_Dis[i].Pulse_Num.shiwan=Weiyi[i].Pulse_Num/100000;
								Weiyi_Dis[i].Pulse_Num.wan=(Weiyi[i].Pulse_Num-Weiyi_Dis[i].Pulse_Num.shiwan*100000)/10000;
								Weiyi_Dis[i].Pulse_Num.qian=(Weiyi[i].Pulse_Num-Weiyi_Dis[i].Pulse_Num.shiwan*100000-Weiyi_Dis[i].Pulse_Num.wan*10000)/1000;
								Weiyi_Dis[i].Pulse_Num.bai=(Weiyi[i].Pulse_Num-Weiyi_Dis[i].Pulse_Num.shiwan*100000-Weiyi_Dis[i].Pulse_Num.wan*10000-Weiyi_Dis[i].Pulse_Num.qian*1000)/100;
								Weiyi_Dis[i].Pulse_Num.shi=(Weiyi[i].Pulse_Num-Weiyi_Dis[i].Pulse_Num.shiwan*100000-Weiyi_Dis[i].Pulse_Num.wan*10000-Weiyi_Dis[i].Pulse_Num.qian*1000-Weiyi_Dis[i].Pulse_Num.bai*100)/10;
								Weiyi_Dis[i].Pulse_Num.ge=(Weiyi[i].Pulse_Num-Weiyi_Dis[i].Pulse_Num.shiwan*100000-Weiyi_Dis[i].Pulse_Num.wan*10000-Weiyi_Dis[i].Pulse_Num.qian*1000-Weiyi_Dis[i].Pulse_Num.bai*100-Weiyi_Dis[i].Pulse_Num.shi*10)/1;

							  Weiyi_Dis[i].Pulse_Rate.shiwan=Weiyi[i].Pulse_Rate/100000;
								Weiyi_Dis[i].Pulse_Rate.wan=(Weiyi[i].Pulse_Rate-Weiyi_Dis[i].Pulse_Rate.shiwan*100000)/10000;
								Weiyi_Dis[i].Pulse_Rate.qian=(Weiyi[i].Pulse_Rate-Weiyi_Dis[i].Pulse_Rate.shiwan*100000-Weiyi_Dis[i].Pulse_Rate.wan*10000)/1000;
								Weiyi_Dis[i].Pulse_Rate.bai=(Weiyi[i].Pulse_Rate-Weiyi_Dis[i].Pulse_Rate.shiwan*100000-Weiyi_Dis[i].Pulse_Rate.wan*10000-Weiyi_Dis[i].Pulse_Rate.qian*1000)/100;
								Weiyi_Dis[i].Pulse_Rate.shi=(Weiyi[i].Pulse_Rate-Weiyi_Dis[i].Pulse_Rate.shiwan*100000-Weiyi_Dis[i].Pulse_Rate.wan*10000-Weiyi_Dis[i].Pulse_Rate.qian*1000-Weiyi_Dis[i].Pulse_Rate.bai*100)/10;
								Weiyi_Dis[i].Pulse_Rate.ge=(Weiyi[i].Pulse_Rate-Weiyi_Dis[i].Pulse_Rate.shiwan*100000-Weiyi_Dis[i].Pulse_Rate.wan*10000-Weiyi_Dis[i].Pulse_Rate.qian*1000-Weiyi_Dis[i].Pulse_Rate.bai*100-Weiyi_Dis[i].Pulse_Rate.shi*10)/1;
							
							  Weiyi_Dis[i].Pulse_Acc.shiwan=Weiyi[i].Speed_Acc/100000;
								Weiyi_Dis[i].Pulse_Acc.wan=(Weiyi[i].Speed_Acc-Weiyi_Dis[i].Pulse_Acc.shiwan*100000)/10000;
								Weiyi_Dis[i].Pulse_Acc.qian=(Weiyi[i].Speed_Acc-Weiyi_Dis[i].Pulse_Acc.shiwan*100000-Weiyi_Dis[i].Pulse_Acc.wan*10000)/1000;
								Weiyi_Dis[i].Pulse_Acc.bai=(Weiyi[i].Speed_Acc-Weiyi_Dis[i].Pulse_Acc.shiwan*100000-Weiyi_Dis[i].Pulse_Acc.wan*10000-Weiyi_Dis[i].Pulse_Acc.qian*1000)/100;
								Weiyi_Dis[i].Pulse_Acc.shi=(Weiyi[i].Speed_Acc-Weiyi_Dis[i].Pulse_Acc.shiwan*100000-Weiyi_Dis[i].Pulse_Acc.wan*10000-Weiyi_Dis[i].Pulse_Acc.qian*1000-Weiyi_Dis[i].Pulse_Acc.bai*100)/10;
								Weiyi_Dis[i].Pulse_Acc.ge=(Weiyi[i].Speed_Acc-Weiyi_Dis[i].Pulse_Acc.shiwan*100000-Weiyi_Dis[i].Pulse_Acc.wan*10000-Weiyi_Dis[i].Pulse_Acc.qian*1000-Weiyi_Dis[i].Pulse_Acc.bai*100-Weiyi_Dis[i].Pulse_Acc.shi*10)/1;
							
							
							}
							if(edit_statement==0||edit_statement==1)
							{
								display_command();
							}
							for(k=0;k<=100;k++)
							{
								if(command[k]==0)
								{
										command_max=k-1;
									if(command_max==255)
									{
										command_max=0;
									}
										break;
								}
							}
							page_max=command_max/5+1;
}
void Run(void)//运行指令
{
	switch(command[run_num])                                                                                                                                                                        
	{
		case 1:Run_WeiYi(Weiyi[run_num].Direction,run_num);break;
		case 2:Run_Time(command_edit_num[run_num]);break;
		case 3:Run_Speed(command_edit_num[run_num]);break;
		case 4:Run_Input(command_edit_num_wei[run_num][3],command_edit_num_wei[run_num][4]);break;
		case 5:Run_Output(command_edit_num_wei[run_num][3],command_edit_num_wei[run_num][4]);break;
		case 7:Run_Return(command_edit_num[run_num],&run_num);break;
		case 8:Run_End(command_edit_num[run_num],command_edit_num[run_num+1],command_edit_num[run_num+2],&run_num);break;
	}
	delay_ms(5);
	run_num++;
	if(command[run_num]==0)
	{
								 end=1;
						pulse_count=0;
						p1=0;
						speed_lock=1;
						init_lock=1;
					  edit_statement=0;
	}
	
}
void Run_WeiYi(u8 dir,u16 num)
{
	end=0;
	time_state=1;
	if(dir==0)
	{
		RunDirection=1;
	}
	else
	{
		RunDirection=0;
	}
	pwm_speed(Weiyi[num].Speed_Acc,Weiyi[num].Pulse_Rate,Weiyi[num].Pulse_Num);
	while(end==0)
	{
		tp_dev.scan(0);
		if(tp_dev.sta&TP_PRES_DOWN)			//触摸屏被按下
		{	
			
			delay_ms(10);
			if(tp_dev.sta&TP_PRES_DOWN)
			{
					if(tp_dev.x>300&&tp_dev.x<380&&tp_dev.y>190&&tp_dev.y<245)//停止键
					{
						 end=1;
						pulse_count=0;
						p1=0;
						speed_lock=1;
						init_lock=1;
						TIM_Cmd(TIM3, DISABLE);
					   edit_statement=0;
						 break;
					}
			}
		}
	};//程序没结束就死循环
	
}
void Run_Time(u16 num)
{
	int i;
	for(i=0;i<num;i++)
	{
		delay_ms(100);
    tp_dev.scan(0);
	 	if(tp_dev.sta&TP_PRES_DOWN)			//触摸屏被按下
		{	
			delay_ms(10);
			if(tp_dev.sta&TP_PRES_DOWN)
			{
					if(tp_dev.x>300&&tp_dev.x<380&&tp_dev.y>190&&tp_dev.y<245)//停止键
					{
						 
					   edit_statement=2;
						 break;
					}
				
			}
		}
	}
}
void Run_Speed(u16 num)
{
	time_state=3;//跑速度指令
	if(num==0)
	{
			end=1;
			pulse_count=0;
			p1=0;
			speed_lock=1;
			TIM_Cmd(TIM3, DISABLE);
	}else
	{
	pwm_speed(1,num,10000);
	}
}
void Run_Input(u16 Xi,u16 Xstate)
{
	int GO_ON=0;
		switch (Xi)
		{
			case 1:while(GO_ON==0)
						{
							if(X1==Xstate)
							{
								delay_ms(10);
								if(X1==Xstate)
								{
									GO_ON=1;
								}
							}
						}
							break;
			case 2:while(GO_ON==0)
						{
							if(X2==Xstate)
							{
								delay_ms(10);
								if(X2==Xstate)
								{
									GO_ON=1;
								}
							}
						}
							break;
			case 3:while(GO_ON==0)
						{
							if(X3==Xstate)
							{
								delay_ms(10);
								if(X3==Xstate)
								{
									GO_ON=1;
								}
							}
						}
							break;
			case 4:while(GO_ON==0)
						{
							if(X4==Xstate)
							{
								delay_ms(10);
								if(X4==Xstate)
								{
									GO_ON=1;
								}
							}
						}
							break;	
			case 5:while(GO_ON==0)
						{
							if(X5==Xstate)
							{
								delay_ms(10);
								if(X5==Xstate)
								{
									GO_ON=1;
								}
							}
						}
							break;		
			case 6:while(GO_ON==0)
						{
							if(X6==Xstate)
							{
								delay_ms(10);
								if(X6==Xstate)
								{
									GO_ON=1;
								}
							}
						}
							break;
		}
	

	
}
void Run_Output(u16 led ,u16 Xstate)
{
	if(Xstate==1)
	{
			switch(led)
		{
			case 1:LED0=1;break;
			case 2:LED1=1;break;
		}
	}
		if(Xstate==0)
	{
			switch(led)
		{
			case 1:LED0=0;break;
			case 2:LED1=0;break;
		}
	}
}
void Run_Scan(void)
{
	int i;
	int scan;
			tp_dev.scan(0);
		if(tp_dev.sta&TP_PRES_DOWN)			//触摸屏被按下
		{	
			
			delay_ms(10);
			if(tp_dev.sta&TP_PRES_DOWN)
			{
					if(tp_dev.x>390&&tp_dev.x<470&&tp_dev.y>215&&tp_dev.y<261)//停止键
					{
						 end=1;
						pulse_count=0;
						p1=0;
						speed_lock=1;
						TIM_Cmd(TIM3, DISABLE);
						init_lock=1;
					   edit_statement=0;
					}
			}
		}
		
			
	for(i=0;i<ScanCommandNum;i++)//扫描端口，前者为端口号，后者为电平状态
	{
		scan=Scan_GPIO(command_edit_num_wei[ScanCommand[i]][3],command_edit_num_wei[ScanCommand[i]][4]);
		if(scan==ScanOK)
		{
			Run_Scan_Service(ScanCommand[i]+1,99);
		}
	}
}
void Run_Return(u16 num,u16 *p)
{
	*p=num-1;
}
void Run_End(u16 startnum,u16 endnum,u16 num,u16 *p)
{
	u16 i,j;
	for(i=1;i<=num;i++)
	{
		for(j=startnum;j<=endnum;j++)
		{
				switch(command[j])                                                                                                                                                                        
			{
			case 1:Run_WeiYi(Weiyi[j].Direction,j);break;
			case 2:Run_Time(command_edit_num[j]);break;
			case 3:Run_Speed(command_edit_num[j]);break;
			case 4:Run_Input(command_edit_num_wei[run_num][3],command_edit_num_wei[run_num][4]);break;
			case 5:Run_Output(command_edit_num_wei[run_num][3],command_edit_num_wei[run_num][4]);break;
			case 7:Run_Return(command_edit_num[j],&j);break;
			case 8:Run_End(command_edit_num[j],command_edit_num[j+1],command_edit_num[j+2],&j);break;
			}
		}
	}
	*p=*p+2;
	
}
void set_system(void)
{
	int i;
		if(init_lock)//切换状态时初始化一遍
	{
		LCD_Clear(WHITE);
		init_lock=0;
		flicker_count=0;
		edit_init();
	}
	LCD_ShowNum_32(270,80,Select_Pro);
	
	showhanzi32(0,70,63);                                //选择程序
	showhanzi32(36,70,64);                               
  showhanzi32(72,70,65);                              
	showhanzi32(108,70,66);                             

	showhanzi32(0,140,63);                           //选择模式  
	showhanzi32(36,140,64);                              
  showhanzi32(72,140,83);                             
	showhanzi32(108,140,84);        
	if(Mode==Auto_Mode)//自动
	{
		showhanzi32(222,140,85);                             
	  showhanzi32(254,140,86);    
	}
	if(Mode==Hand_Mode)//手动
	{
		showhanzi32(222,140,87);                             
	  showhanzi32(254,140,88);    
	}

	
	tp_dev.scan(0);
	flicker_count++;

	Short_Line_flicker();

	
	if(tp_dev.sta&TP_PRES_DOWN)			//触摸屏被按下
	{	
		delay_ms(10);
		if(tp_dev.sta&TP_PRES_DOWN)
		{
			if(uc_lock==1)//防止重复按下
			{
				if(tp_dev.x>300&&tp_dev.x<380&&tp_dev.y>10&&tp_dev.y<65)//翻页键
				{
					edit_statement=5;
					init_lock=1;
					Pinpoint_Num=1;
					uc_lock=0;
				}
		
				if(tp_dev.x>390&&tp_dev.x<470&&tp_dev.y>10&&tp_dev.y<65)//返回键
				{
					edit_statement=0;
					Pinpoint_Num=1;
					init_lock=1;
					uc_lock=0;
				}
				if(tp_dev.x>300&&tp_dev.x<380&&tp_dev.y>70&&tp_dev.y<125)//左移键
				{
					Pinpoint_Num--;
					if(Pinpoint_Num==0)
					{
						Pinpoint_Num=2;
					}
					uc_lock=0;
					Move_LR_Service();
				}
				if(tp_dev.x>390&&tp_dev.x<470&&tp_dev.y>70&&tp_dev.y<125)//右移键
				{
					Pinpoint_Num++;
					if(Pinpoint_Num==3)
					{
						Pinpoint_Num=1;
					}
					uc_lock=0;
					Move_LR_Service();
				}
				if(tp_dev.x>300&&tp_dev.x<380&&tp_dev.y>130&&tp_dev.y<185)//加键
				{
					uc_lock=0;
				 jia_service();
					
				}
				if(tp_dev.x>390&&tp_dev.x<470&&tp_dev.y>130&&tp_dev.y<185)//减键
				{
					uc_lock=0;
				 jian_service();
					
				}
				if(tp_dev.x>300&&tp_dev.x<380&&tp_dev.y>190&&tp_dev.y<245)//读取
				{
          read_command();
				}
				if(tp_dev.x>390&&tp_dev.x<470&&tp_dev.y>190&&tp_dev.y<245)//保存
				{
					save_command();
				}
			
			}
			
		}
	}
	else
	{
		uc_lock=1;
	}
	
	
}
void set_system_2(void)
{
	  int i;
		if(init_lock)//切换状态时初始化一遍
	{
		
		init_lock=0;
		flicker_count=0;
	LCD_Clear(WHITE);
	showhanzi32(0,20,85);                             //自动模式设置
	showhanzi32(36,20,86);
	showhanzi32(72,20,83);                              
	showhanzi32(108,20,84);
  showhanzi32(140,20,3);                             
	showhanzi32(172,20,62);	
		                          

	showhanzi32(0,140,71);                           //一圈脉冲
	showhanzi32(36,140,72);                              
  showhanzi32(72,140,67);                             
	showhanzi32(108,140,68);                            
	
	showhanzi32(0,200,71);                             //一圈距离
	showhanzi32(36,200,72);                            
  showhanzi32(72,200,73);                             
	showhanzi32(108,200,74); 
	
	
	//LCD_ShowNum_32(180,80,Pulse_Num_dis_01.shiwan);//
	LCD_ShowNum_32(198,140,Pulse_Dangliang.wan);//脉冲个数数值
	LCD_ShowNum_32(216,140,Pulse_Dangliang.qian);
	LCD_ShowNum_32(234,140,Pulse_Dangliang.bai);
	LCD_ShowNum_32(252,140,Pulse_Dangliang.shi);
	LCD_ShowNum_32(270,140,Pulse_Dangliang.ge);
	
	LCD_ShowNum_32(198,200,Distance_Per.wan);//每圈距离数值
	LCD_ShowNum_32(216,200,Distance_Per.qian);
	LCD_ShowNum_32(234,200,Distance_Per.bai);
	LCD_ShowNum_32(252,200,Distance_Per.shi);
	LCD_ShowNum_32(270,200,Distance_Per.ge);
		edit_init();
	}
	
	
	//LCD_ShowNum_32(180,80,Pulse_Num_dis_01.shiwan);//
	LCD_ShowNum_32(198,140,Pulse_Dangliang.wan);//脉冲个数
	LCD_ShowNum_32(216,140,Pulse_Dangliang.qian);
	LCD_ShowNum_32(234,140,Pulse_Dangliang.bai);
	LCD_ShowNum_32(252,140,Pulse_Dangliang.shi);
	LCD_ShowNum_32(270,140,Pulse_Dangliang.ge);
	
	LCD_ShowNum_32(198,200,Distance_Per.wan);//每圈距离
	LCD_ShowNum_32(216,200,Distance_Per.qian);
	LCD_ShowNum_32(234,200,Distance_Per.bai);
	LCD_ShowNum_32(252,200,Distance_Per.shi);
	LCD_ShowNum_32(270,200,Distance_Per.ge);
	
	tp_dev.scan(0);
	flicker_count++;

	Short_Line_flicker();

	
	if(tp_dev.sta&TP_PRES_DOWN)			//触摸屏被按下
	{	
		delay_ms(10);
		if(tp_dev.sta&TP_PRES_DOWN)
		{
			if(uc_lock==1)//防止重复按下
			{
				if(tp_dev.x>300&&tp_dev.x<380&&tp_dev.y>10&&tp_dev.y<65)//翻页键
				{
					
					edit_statement=6;
						Pinpoint_Num=1;
					init_lock=1;
					uc_lock=0;
				}
		
				if(tp_dev.x>390&&tp_dev.x<470&&tp_dev.y>10&&tp_dev.y<65)//返回键
				{
					edit_statement=0;
					Pinpoint_Num=1;
					init_lock=1;
					uc_lock=0;
				}
				if(tp_dev.x>300&&tp_dev.x<380&&tp_dev.y>70&&tp_dev.y<125)//左移键
				{
					Pinpoint_Num--;
					if(Pinpoint_Num==0)
					{
						Pinpoint_Num=10;
					}
					uc_lock=0;
					Move_LR_Service();
				}
				if(tp_dev.x>390&&tp_dev.x<470&&tp_dev.y>70&&tp_dev.y<125)//右移键
				{
					Pinpoint_Num++;
					if(Pinpoint_Num==11)
					{
						Pinpoint_Num=1;
					}
					uc_lock=0;
					Move_LR_Service();
				}
				if(tp_dev.x>300&&tp_dev.x<380&&tp_dev.y>130&&tp_dev.y<185)//加键
				{
					uc_lock=0;
				 jia_service();
					
				}
				if(tp_dev.x>390&&tp_dev.x<470&&tp_dev.y>130&&tp_dev.y<185)//减键
				{
					uc_lock=0;
				 jian_service();
					
				}
				if(tp_dev.x>300&&tp_dev.x<380&&tp_dev.y>190&&tp_dev.y<245)//读取
				{
            read_command();
				}
				if(tp_dev.x>390&&tp_dev.x<470&&tp_dev.y>190&&tp_dev.y<245)//保存
				{
						save_command();
				}

			}
			
		}
	}
	else
	{
		uc_lock=1;
	}
	
	
}

void set_system_3(void)
{
	  int i;
		if(init_lock)//切换状态时初始化一遍
	{
		
		init_lock=0;
		flicker_count=0;
	LCD_Clear(WHITE);
	showhanzi32(0,20,87);                             //手动模式设置
	showhanzi32(36,20,88);
	showhanzi32(72,20,83);                              
	showhanzi32(108,20,84);
  showhanzi32(140,20,3);                             
	showhanzi32(172,20,62);	
		                          

	showhanzi32(0,140,57);                           //点动脉冲
	showhanzi32(36,140,88);                              
  showhanzi32(72,140,67);                             
	showhanzi32(108,140,68);                            
	
	showhanzi32(0,200,89);                             //连按频率
	showhanzi32(36,200,90);                            
  showhanzi32(72,200,12);                             
	showhanzi32(108,200,13); 
	
	
	//LCD_ShowNum_32(180,80,Pulse_Num_dis_01.shiwan);//
	LCD_ShowNum_32(198,140,Dot_Pulse.wan);//点动脉冲
	LCD_ShowNum_32(216,140,Dot_Pulse.qian);
	LCD_ShowNum_32(234,140,Dot_Pulse.bai);
	LCD_ShowNum_32(252,140,Dot_Pulse.shi);
	LCD_ShowNum_32(270,140,Dot_Pulse.ge);
	
	LCD_ShowNum_32(198,200,Constant_Speed.wan);//连按频率
	LCD_ShowNum_32(216,200,Constant_Speed.qian);
	LCD_ShowNum_32(234,200,Constant_Speed.bai);
	LCD_ShowNum_32(252,200,Constant_Speed.shi);
	LCD_ShowNum_32(270,200,Constant_Speed.ge);
		edit_init();
	}
	
	
	//LCD_ShowNum_32(180,80,Pulse_Num_dis_01.shiwan);//
	LCD_ShowNum_32(198,140,Dot_Pulse.wan);//点动脉冲
	LCD_ShowNum_32(216,140,Dot_Pulse.qian);
	LCD_ShowNum_32(234,140,Dot_Pulse.bai);
	LCD_ShowNum_32(252,140,Dot_Pulse.shi);
	LCD_ShowNum_32(270,140,Dot_Pulse.ge);
	
	LCD_ShowNum_32(198,200,Constant_Speed.wan);//连按频率
	LCD_ShowNum_32(216,200,Constant_Speed.qian);
	LCD_ShowNum_32(234,200,Constant_Speed.bai);
	LCD_ShowNum_32(252,200,Constant_Speed.shi);
	LCD_ShowNum_32(270,200,Constant_Speed.ge);
	
	tp_dev.scan(0);
	flicker_count++;

	Short_Line_flicker();

	
	if(tp_dev.sta&TP_PRES_DOWN)			//触摸屏被按下
	{	
		delay_ms(10);
		if(tp_dev.sta&TP_PRES_DOWN)
		{
			if(uc_lock==1)//防止重复按下
			{
				if(tp_dev.x>300&&tp_dev.x<380&&tp_dev.y>10&&tp_dev.y<65)//翻页键
				{
					
					edit_statement=4;
						Pinpoint_Num=1;
					init_lock=1;
					uc_lock=0;
				}
		
				if(tp_dev.x>390&&tp_dev.x<470&&tp_dev.y>10&&tp_dev.y<65)//返回键
				{
					edit_statement=0;
					Pinpoint_Num=1;
					init_lock=1;
					uc_lock=0;
				}
				if(tp_dev.x>300&&tp_dev.x<380&&tp_dev.y>70&&tp_dev.y<125)//左移键
				{
					Pinpoint_Num--;
					if(Pinpoint_Num==0)
					{
						Pinpoint_Num=10;
					}
					uc_lock=0;
					Move_LR_Service();
				}
				if(tp_dev.x>390&&tp_dev.x<470&&tp_dev.y>70&&tp_dev.y<125)//右移键
				{
					Pinpoint_Num++;
					if(Pinpoint_Num==11)
					{
						Pinpoint_Num=1;
					}
					uc_lock=0;
					Move_LR_Service();
				}
				if(tp_dev.x>300&&tp_dev.x<380&&tp_dev.y>130&&tp_dev.y<185)//加键
				{
					uc_lock=0;
				 jia_service();
					
				}
				if(tp_dev.x>390&&tp_dev.x<470&&tp_dev.y>130&&tp_dev.y<185)//减键
				{
					uc_lock=0;
				 jian_service();
					
				}
				if(tp_dev.x>300&&tp_dev.x<380&&tp_dev.y>190&&tp_dev.y<245)//读取
				{
						read_command();
				}
				if(tp_dev.x>390&&tp_dev.x<470&&tp_dev.y>190&&tp_dev.y<245)//保存
				{
						save_command();
				}
			}
			
		}
	}
	else
	{
		uc_lock=1;
	}
	
	
}
void save_edit_num(void)
{
	
}
int Scan_GPIO(u16 Xi,u16 Xstate)
{
	  
			switch (Xi)
		{
			case 1:if(X1==Xstate)
							{
								delay_ms(10);
								if(X1==Xstate)
								{
									return ScanOK;
								}
								else return ScanError;
							}
							else return ScanError;
							break;
			case 2:if(X2==Xstate)
							{
								delay_ms(10);
								if(X2==Xstate)
								{
									return ScanOK;
								}
								else return ScanError;
							}
							else return ScanError;
							break;
			case 3:if(X3==Xstate)
							{
								delay_ms(10);
								if(X3==Xstate)
								{
									return ScanOK;
								}
								else return ScanError;
							}
							else return ScanError;
							break;
			case 4:if(X4==Xstate)
							{
								delay_ms(10);
								if(X4==Xstate)
								{
									return ScanOK;
								}
								else return ScanError;
							}
							else return ScanError;
							break;	
			case 5:if(X5==Xstate)
							{
								delay_ms(10);
								if(X5==Xstate)
								{
									return ScanOK;
								}
								else return ScanError;
							}
							else return ScanError;
							break;		
			case 6:if(X6==Xstate)
							{
								delay_ms(10);
								if(X6==Xstate)
								{
									return ScanOK;
								}
								else return ScanError;
							}
							else return ScanError;
							break;
		}
}
void Run_Scan_Service(u16 startnum,u16 endnum)
{
	u16 i,j;
		for(j=startnum;j<=endnum;j++)
		{
				switch(command[j])                                                                                                                                                                        
			{
					case 1:Run_WeiYi(Weiyi[j].Direction,j);break;
					case 2:Run_Time(command_edit_num[j]);break;
					case 3:Run_Speed(command_edit_num[j]);break;
					case 4:Run_Input(command_edit_num_wei[run_num][3],command_edit_num_wei[run_num][4]);break;
					case 5:Run_Output(command_edit_num_wei[run_num][3],command_edit_num_wei[run_num][4]);break;
				  case 6:j=endnum+1;break;
					case 7:Run_Return(command_edit_num[j],&j);break;
					case 8:Run_End(command_edit_num[j],command_edit_num[j+1],command_edit_num[j+2],&j);break;
					case 9:j=end+1;break;
			}
		}
	
}
