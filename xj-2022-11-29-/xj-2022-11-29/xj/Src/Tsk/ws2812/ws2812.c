#include"ws2812.h"
#include"main.h"
#define SEND_TIMES 20
#define HIGH 1




static uint8 Gard[3] = { 0 };




//PE4  CTRL  PE5 DATA  针对灯带的底层驱动程序
/*PE4 是控制线或者是时钟线  PE5是数据线*/
void Ctrl_1()
{
	HAL_GPIO_WritePin(Standby_state_led_GPIO_Port, Standby_state_led_Pin, GPIO_PIN_SET);
}
void Ctrl_0()
{
	HAL_GPIO_WritePin(Standby_state_led_GPIO_Port, Standby_state_led_Pin, GPIO_PIN_RESET);
}
void l_Data_1()
{
	HAL_GPIO_WritePin(work_led_GPIO_Port, work_led_Pin, GPIO_PIN_SET);
}
void l_Data_0()
{
	HAL_GPIO_WritePin(work_led_GPIO_Port, work_led_Pin, GPIO_PIN_RESET);
}


void l_dat0(void)
{
	Ctrl_1();
	//WS2811_CONTROL |= HIGH;
	osDelay(3);
	Ctrl_0();
	//WS2811_CONTROL &= ~HIGH;
	osDelay(15);
}

//===============高速编码模式BIT1(高电平时间:5us 低电平时间:0.5us)================//
void l_dat1(void)
{
	Ctrl_1();
	//WS2811_CONTROL |= HIGH;
	osDelay(16);
	Ctrl_0();
	//WS2811_CONTROL &= ~HIGH;
	osDelay(2);
}

//===================================RGB复位====================================//
void Reset(void)
{
	Ctrl_0();
	//WS2811_CONTROL &= ~HIGH;
	osDelay(2000);
}

void send_single_data(const uint8* data)
{	//
	uint8 i = 0, j = 0;
	for (i = 0; i < 3; i++) {
		for (j = 7; j > -1; j--) {
			if ((*(data + i) & (HIGH << j)) == (HIGH << j))
				l_dat1();
			else
				l_dat0();
		}
	}
}

void send_string_data(const uint8* data, uint16 size)
{
	uint8 i = 0;
	for (i = 0; i < size; i++) {
		send_single_data(data);
	}
	Reset();
}



void RGB_Lighting(uint8*LET)
{
	uint8* buff = LET;
	switch (buff[0])
	{
	case STANDBY_MODE :
	{//待机状态
		if (1) {
			//蓝色常亮
			Gard[0] = 0;//绿
			Gard[1] = 0;//红
			Gard[2] = 255;//蓝
			send_string_data(Gard, SEND_TIMES);//SEND_TIMES是灯管个数
		}
		break;
	}
	case GUN_INSERTION_AND_UNAUTHORIZED:
	{//插枪未授权，CP=9V常压
		if (0) {
			//绿色闪烁（0.5秒亮，0.5秒暗）

			Gard[0] = 255;
			Gard[1] = 0;
			Gard[2] = 0;
			send_string_data(Gard, SEND_TIMES);
			osDelay(500);//延时函数查询！！！
		}



		break;
	}
	case GUN_INSERTION_AND_AUTHORIZED:
	{//插枪授权，CP=9V常压，PWM
		if (1) {
			//绿色常亮
			Gard[0] = 255;//绿
			Gard[1] = 0;//红
			Gard[2] = 0;//蓝
			send_string_data(Gard, SEND_TIMES);
		}
		break;
	}
	case GUN_INSERTION_AND_CHARGING_START:
	{//插枪启动充电，cp=6，PWM
		if (1) {
			//绿色渐变跑马（2s循环）
			//未完成！！！！！！！！！！！！！！！！！
			Gard[0] = 255;
			Gard[1] = 0;
			Gard[2] = 0;
			send_string_data(Gard, SEND_TIMES);
		}
		break;
	}
	case NON_SHUTDOWN_FAULT:
	{//非停机类故障
		if (1) {
			//红灯闪烁（0.5秒亮，0.5秒暗）
			//!!!!!!!!!!!!!!!!!!while (state) {
				Gard[0] = 0;
				Gard[1] = 255;
				Gard[2] = 0;
				send_string_data(Gard, SEND_TIMES);
				osDelay(500);
			}
		
		break;
	}
	case SHUTDOWN_FAULT:
	{//停机类故障
		if (1) {
			//红灯常亮
			Gard[0] = 0;
			Gard[1] = 255;
			Gard[2] = 0;
			send_string_data(Gard, SEND_TIMES);
		}
		break;
	}
	case BOOT:
	{//开机通电
		if (1) {
			//LOGO蓝灯常亮
			Gard[0] = 0;
			Gard[1] = 0;
			Gard[2] = 255;
			send_string_data(Gard, SEND_TIMES);
		}
		break;
	}

	}


}
















