
#include"BLT.h"

#define BLT1_RX  USART1
#define BLT1_TEST COM5
#define  BLT_READ 0XFF
#define BLT_WRITE 0XFF
#define BLT_Auth  OX01


#define HIGH 1
uint8 g_ATSendBuff[200];//ȫ�ֵ��������ص�AT�ַ���
uint16 BLT_AT_MOD;
uint16 BLT_DATA_MOD;
char BLT_Default_connection;//Ĭ������������
//����flash����������ַ����һ���ṹ���ڣ��ṹ���ڰ����ʹ�������������ӣ��滻����ʹ���������кţ��д����ṹ��

char BLT_connection_NUM;//����������




/********************************************************
��    �� : ����ATָ���

������� : 

������� :��
��    �� :true

��    �� :

����ʱ�� :2022-11-15
*********************************************************/
static uint8 BLT_test(void)
{
    char* ret;
	int ReConnectCount = 0;
	while(ReConnectCount>3)
    {
        BLT_AT_MOD = 1;//AT��־��1
		if (BLT_AT_Cmd_Parse("+++a", "OK", NULL, 1000) == FALSE) {

			osDelay(1000);
            ReConnectCount++;
            dbg_printf(1, "+++a not ok!");
		}
		if (BLT_AT_Cmd_Parse("AT+MODE=U", "OK", NULL, 1000) == FALSE) {
			osDelay(1000);
            ReConnectCount++;
            dbg_printf(1, "AT+MODE=M not ok!");

        }
        //ɨ����Χ�豸������{
        if (BLT_AT_Cmd_Parse("AT+SCAN", "OK", ret, 1000) == FALSE) {
            osDelay(1000);
            ReConnectCount++;
            dbg_printf(1, "AT+SCAN not ok!");

        }
        
    }
    //�õ������б��ָ�
    //vector<char*> res_split;//�洢�ָ����ַ���
    char BLT_NUM[100];
    memcpy(BLT_NUM, ret, 100);
    const char split[] = "No:";
    char* res = strtok(BLT_NUM, split);
    char num[8];
    int i=0;
    int pnum=0;
    while (res != NULL)
    {
        res = strtok(NULL, split);
        num[i] = *res;
        i++;
    }
    for (i = 0; i < 8; i++) {
        if (num[i] == BLT_Default_connection)
        {
            BLT_connection_NUM = BLT_Default_connection;
             pnum = i;
            break;
        }
    }

    //���ѡ��һ������
    if (0) {
        pnum = 3;
        BLT_Default_connection = num[3];
    }
    

    char*pBLT_connection_NUM;
    char* BLT_connection_NUM;
    char*AT_PDATA;
    AT_PDATA = "AT+CONN=";
    char*pdata1 = ",";
    sprintf(BLT_connection_NUM, "%s%s%s%d", AT_PDATA, pBLT_connection_NUM, pdata1,pnum);


        //�õ������б���Զ�����֮ǰ���ӵ�Ĭ���豸
        // ת�����ݴ���ģʽ����״̬��־λ��blt_AT_MOD,[]BLT_DATA_MOD,������ģʽ
        //Ĭ�����ӵ�һ���豸
    //dbg_printf(1,"��ǰɨ���%s", g_ATSendBuff);
    if (BLT_AT_Cmd_Parse(BLT_connection_NUM, "OK", NULL, 1000) == FALSE) {
        osDelay(1000);
        ReConnectCount++;
        dbg_printf(1, "AT+CONN not ok!");
    }
		//if (BLT_AT_Cmd_Parse("AT+SCAN", "OK", NULL, 1000) == FALSE) {
		//	osDelay(1000);
  //          ReConnectCount++;
  //          dbg_printf(1, "AT+MODE=M not ok!");
		//}
        if (BLT_AT_Cmd_Parse("AT+LINK", "OK", NULL, 1000) == FALSE) {
            osDelay(1000);
            ReConnectCount++;
            dbg_printf(1, "AT+LINK not ok!");

        }
      


        BLT_AT_MOD = 0;//AT��־��0
        BLT_DATA_MOD = 1;//���ݴ�����1
        return TRUE;
	}
     
    




uint16 rcvbuf_judge(uint8* pData, char* expect) {
    char* str;
    str = strstr((uint8*)pData, expect);

    if (str&&(pData[0]==0x05|| pData[0] == 0xFF)) {
        BLT_DATA_MOD=0;
        return 1;
    }
    else if(str&& (pData[0] != 0x05 || pData[0] != 0xFF)) {
        BLT_AT_MOD = 1;
        return 0;
    }
        



}




/********************************************************
��    �� : ����Э���װ��������֡����

������� : SendBuf:ȫ�ֱ���
            cmdnumber  ���������
            Cmd ������
            pData ������
            Len ������ ����

������� :��
��    �� :index ���ĳ���

��    �� :

����ʱ�� :2022-11-15
*********************************************************/

static uint16 BLT_Pack(uint8* SendBuf, uint16 cmdnumber, uint16 Cmd, uint8* pData, uint16 length)
{
    static uint8 Buf[200] = { 0 };
    uint16 index = 0;
    uint16 i;
    Buf[index++] = 0x5A;
    Buf[index++] = 0xFF;
    Buf[index++] = length + 4;
    Buf[index++] = Cmd;
    Buf[index++] = cmdnumber;
    if ((NULL != pData) && (length != 0))
    {
        for (i = 0; i < length; i++)
        {
            Buf[index++] = *pData++;
        }
    }
    uint16 j;
    for (j = 0; j <= index; j++) {
        SendBuf[j] = Buf[j];
    }

    return index;



}

/********************************************************
��    �� : �����������ݽ���

������� :  Send:      ���͵�ATָ��
            expect:    ���ܴ��ڷ����������Ƿ����expect�ַ���
            ret��      ���ڷ��ص����ݣ�����
            timeout    ��ʱ

������� :��
��    �� :index ���ĳ���

��    �� :

����ʱ�� :2022-11-15
*********************************************************/
static uint16 BLT_Parse(uint8* pData, int timeout)
{
    uint32 ms = 0;
    uint8 FramLen = 0;
    uint8 DataLen = 0;
    uint8 rcvLen = 0;
    uint8 rcvBuf[64] = { 0 };
    uint8 DataBuf[64] = { 0 };
    int i = 0;
    uint8 Sum = 0;

    ms = HAL_GetTick();

    do
    {
        rcvLen = Drv_Uart_Read(BLT1_TEST, rcvBuf, 64, 100);

        if (rcvLen > 0)
        {
            for (i = 0; i < rcvLen; i++)
            {
                if (FramLen == 0)
                {
                    if (rcvBuf[i] == 0x5A)
                    {
                        DataBuf[FramLen++] = rcvBuf[i];
                        continue;
                    }
                }
                if (FramLen == 1)
                {
                    if (rcvBuf[i] == 0xFF)
                    {
                        DataBuf[FramLen++] = rcvBuf[i];
                        continue;
                    }
                }
                if ((FramLen >= 2) && (FramLen <= 3))
                {
                    DataBuf[FramLen++] = rcvBuf[i];
                    continue;
                }

                if (FramLen >= 3)
                {
                    int i = 3;
                    for (i = 4; i < rcvLen; i++)
                    {
                            DataBuf[FramLen++] = rcvBuf[i];
                        
                    }

                    DataLen = DataBuf[3];
                    //
                    if (FramLen == DataLen + 4)
                    {

                            memcpy(pData, DataBuf + 8, DataLen - 5);
                           // g_CommTick = 0;     //clear
                           // g_CommFault = FALSE;//clear
                            return TRUE;
 
                    }
                    else
                    {
                        if (FramLen > sizeof(DataBuf))
                        {
                            FramLen = 0;
                        }
                    }
                }


                if (FramLen > sizeof(DataBuf))
                {
                    FramLen = 0;
                }
            }

        }
        else
        {
            osDelay(5);
        }

    } while (HAL_GetTick() - ms < timeout);

   // g_CommTick++;
    return FALSE;
}

/********************************************************
��    �� : ����ATָ�����

������� :  Send:      ���͵�ATָ��
            expect:    ���ܴ��ڷ����������Ƿ����expect�ַ���
            ret��      ���ڷ��ص����ݣ�����
            timeout    ��ʱ

������� :��
��    �� :index ���ĳ���

��    �� :

����ʱ�� :2022-11-15
*********************************************************/
static uint8 BLT_AT_Cmd_Parse(char* send, char* expect, char* ret, uint32 timeout) {

  
	uint32 tick = xTaskGetTickCount();
	uint8 rcvbuf[128] = { 0 };
	int remain = sizeof(rcvbuf);
	uint32 len = 0;
	uint32 rcvLen = 0;
	char* str = NULL;
    uint16 rcvbuf_state;
	memset(rcvbuf, 0, sizeof(rcvbuf));
    //��ȡ�������ݳ��ȣ�����1�����ںţ�����2�����������ݣ�����3����Ҫ�������ݳ��ȣ�����4������ʱ�䣬���أ����ݳ���
	//len = Drv_Uart_Read(BLT1_TEST, (unsigned char*)rcvbuf, remain, 200);//��ȡ��������
    //�������ݣ�����1�����ںţ�����2�����͵����ݣ������������͵����ݳ��ȣ�����4������ʱ�䣬���أ�ʵ�ʷ��ͳ���
	//Drv_Uart_Write(BLT1_TEST, (const unsigned char*)send, strlen(send), 100);
	osDelay(200);

	memset(rcvbuf, 0, sizeof(rcvbuf));
	dbg_printf(1, "send %s", send);

    
     //��ȡ���ݺ��ж�״̬�Ƕ�ָ�����ATָ�����Ҫ��
       // len = Drv_Uart_Read(net_uart_port, (unsigned char*)rcvbuf + rcvLen, remain, 500);
       // len = Drv_Uart_Read(BLT1_TEST, (unsigned char*)rcvbuf, remain, 500);
	
        //rcvbuf_state=rcvbuf_judge1(rcvbuf);//wangyu mask 2022-11-29
        int i;
        for(i=0;i<129;i++)
        {
            g_ATSendBuff[i] = rcvbuf[i];
        }

       

        if (len > 0)
        {
            dbg_printf(1, "cmd rcv %s", rcvbuf);
            remain -= len;
            rcvLen += len;
            str = strstr((char*)rcvbuf, expect);
            if (str)
            {
                if (ret != NULL)
                {
                    strcpy(ret, str);
                }
                return TRUE;
            }
        }
        else
        {
            osDelay(20);
        }
    return FALSE;


}




