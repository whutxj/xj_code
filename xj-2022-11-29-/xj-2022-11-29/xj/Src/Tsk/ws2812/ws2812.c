#include"ws2812.h"
#include"main.h"
#define SEND_TIMES 20
#define HIGH 1




static uint8 Gard[3] = { 0 };




//PE4  CTRL  PE5 DATA  ��Եƴ��ĵײ���������
/*PE4 �ǿ����߻�����ʱ����  PE5��������*/
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

//===============���ٱ���ģʽBIT1(�ߵ�ƽʱ��:5us �͵�ƽʱ��:0.5us)================//
void l_dat1(void)
{
	Ctrl_1();
	//WS2811_CONTROL |= HIGH;
	osDelay(16);
	Ctrl_0();
	//WS2811_CONTROL &= ~HIGH;
	osDelay(2);
}

//===================================RGB��λ====================================//
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
	{//����״̬
		if (1) {
			//��ɫ����
			Gard[0] = 0;//��
			Gard[1] = 0;//��
			Gard[2] = 255;//��
			send_string_data(Gard, SEND_TIMES);//SEND_TIMES�ǵƹܸ���
		}
		break;
	}
	case GUN_INSERTION_AND_UNAUTHORIZED:
	{//��ǹδ��Ȩ��CP=9V��ѹ
		if (0) {
			//��ɫ��˸��0.5������0.5�밵��

			Gard[0] = 255;
			Gard[1] = 0;
			Gard[2] = 0;
			send_string_data(Gard, SEND_TIMES);
			osDelay(500);//��ʱ������ѯ������
		}



		break;
	}
	case GUN_INSERTION_AND_AUTHORIZED:
	{//��ǹ��Ȩ��CP=9V��ѹ��PWM
		if (1) {
			//��ɫ����
			Gard[0] = 255;//��
			Gard[1] = 0;//��
			Gard[2] = 0;//��
			send_string_data(Gard, SEND_TIMES);
		}
		break;
	}
	case GUN_INSERTION_AND_CHARGING_START:
	{//��ǹ������磬cp=6��PWM
		if (1) {
			//��ɫ��������2sѭ����
			//δ��ɣ���������������������������������
			Gard[0] = 255;
			Gard[1] = 0;
			Gard[2] = 0;
			send_string_data(Gard, SEND_TIMES);
		}
		break;
	}
	case NON_SHUTDOWN_FAULT:
	{//��ͣ�������
		if (1) {
			//�����˸��0.5������0.5�밵��
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
	{//ͣ�������
		if (1) {
			//��Ƴ���
			Gard[0] = 0;
			Gard[1] = 255;
			Gard[2] = 0;
			send_string_data(Gard, SEND_TIMES);
		}
		break;
	}
	case BOOT:
	{//����ͨ��
		if (1) {
			//LOGO���Ƴ���
			Gard[0] = 0;
			Gard[1] = 0;
			Gard[2] = 255;
			send_string_data(Gard, SEND_TIMES);
		}
		break;
	}

	}


}
















