#include "Sys_Config.h"
#include "Utility.h"
#include "DRV_SPIFLASH.h"
#include "string.h"
#include "CRC16.h"
#include "Task_SpiFlash.h"
#include "Task_API_SpiFlash.h"
#include "Tsk_Charge_Ctrl.h"
#include "stdio.h"
#include "math.h"
#include "stdlib.h"
#include "time.h"
#include "Update.h"
#include "lwip/sockets.h"
#include "Tsk_Measure.h"
#include "eth_comm.h"
#include "drv_time.h"
#include "bsp_driver_uart.h"
#include "drv_usart.h"
#include "Drv_MMA8652FC.h"
#include "Tsk_API_Test.h"
#include "BSP_Buzzer.h"
#include "BSP_LED_Function.h"
//#include "Drv_Att7053A.h"
//#include "BSP_I2C.h"
#include "Drv_RN8029.h"

void board_test(void);

void Input_test(void)
{
    Input_TypDef i = (Input_TypDef)0;
    InPutDef InPutRet;

    BSP_GPIO_Init();
    
    for(i= (Input_TypDef)0;i < Input;i++)
    {
        InPutRet = BSP_Read_InPut(i);
        if(InPutRet == IN_HIGHE)
        {
           dbg_printf(1,"干接点%d 断开",i); 
        }
        else
        {
            dbg_printf(1,"干接点%d 闭合",i); 
        }
        osDelay(100);        
    }
}

void Output_test(void)
{
    Output_TypDef i = (Output_TypDef)0;

    BSP_GPIO_Init();
    
    for(i= (Output_TypDef)0;i < Output;i++)
    {

        BSP_GPIO_OutPut(i,OUT_HIGHE);
        dbg_printf(1,"控制%d 闭合",i); 
        osDelay(500);
        BSP_GPIO_OutPut(i,OUT_LOW);
        dbg_printf(1,"控制%d 断开",i);        
        
    }
    
}


void usart_test()
{
    
    uint8 sendBuf[128] ="12345798qwerttyu1211212212iop";  
    
    USART_Init(COM5,STOP_1,2,9600);       //BaudRate
    while(1)
    {
        int len = 16;//Drv_Uart_Read(COM5,sendBuf,128,50);
        //int len = usart_read(COM2,sendBuf,64,20);
        if(len)
        {
            Drv_Uart_Write(COM5,sendBuf,len,200);
            //usart_write(COM2,sendBuf,len,5);
        }
        osDelay(100);
        
    }
        
}

void beep_test()
{
    
    //BSP_Buzzer_Test();
        
}


void card_test()
{
    while(1)
    {
        
        
        
    }
}


static int flash_test(void *p)
{
    uint32 id = BSP_SPIFLASH_ReadJedecID();
    if(id == 0xffffff || id == 0)
    {
        dbg_printf(1,"SPI flash ERR");
        return 0;
    }
    else
    {
        dbg_printf(1,"SPI flash OK");
        return 1;
    }
    
}

static int rtc_test(void *p)
{
    uint32 sec1 = time(NULL);
    uint32 sec2 = 0;
    osDelay(1000);
    sec2 = time(NULL);
    if(sec2 - sec1 == 0)
    {
        dbg_printf(1,"RTC ERR");
        return 0;
    }
    else
    {
        dbg_printf(1,"RTC OK");
        return 1;
    }
}


int  rs232_test(void *p)
{
    uint8 Wbuf[10] = {1,2,3,4,5,6,7,8,9,0};
    uint8 Rbuf[10] = {0};
    
    int len = 0;
  
    USART_Init(COM1,STOP_1,P_NONE,57600);       //BaudRate
    USART_Init(COM5,STOP_1,P_NONE,57600);       //BaudRate
	    
    Drv_Uart_Write(COM1,Wbuf,sizeof(Wbuf),10);
    osDelay(100);
    len = Drv_Uart_Read(COM1,Rbuf,20,100);
    
    if(len > 0)
    {
        if(memcmp(Rbuf,Wbuf,sizeof(Wbuf)) == 0)
        {
            dbg_printf(1,"COM1 test OK");
        }
        else
        {
            dbg_printf(1,"COM1 test ERR");
            return 0;
        }
    }
    else
    {
        dbg_printf(1,"COM1 test ERR");
        return 0;
    }   

		
    memset(Rbuf,0,sizeof(Rbuf));
    
    Drv_Uart_Write(COM5,Wbuf,sizeof(Wbuf),100);
    len = Drv_Uart_Read(COM5,Rbuf,20,100);
    
    if(len > 0)
    {
        if(memcmp(Rbuf,Wbuf,sizeof(Wbuf)) == 0)
        {
            dbg_printf(1,"COM5 test OK");
        }
        else
        {
            dbg_printf(1,"COM5 test ERR");
            return 0;
        }
    }
    else
    {
        dbg_printf(1,"COM5 test ERR");
        return 0;
    }    
    return 1;
}


static uint8_t AT_Cmd_Parse(char *send,char *expect,char *ret,uint32_t timeout)
{
    
    uint32_t tick = xTaskGetTickCount();
    uint8 rcvbuf[128] = {0};
    int remain = sizeof(rcvbuf);
    uint32_t len = 0;
    uint32_t rcvLen = 0;
    char *str = NULL;
    
    memset(rcvbuf,0,sizeof(rcvbuf));
    
    len = Drv_Uart_Read(COM4,(unsigned char *) rcvbuf,remain,200);//

    Drv_Uart_Write(COM4,(const unsigned char *) send,strlen(send),100);
    osDelay(200);
    
    memset(rcvbuf,0,sizeof(rcvbuf));
    
    dbg_printf(1,"send %s",send);
    
    do
    {
        len = Drv_Uart_Read(COM4,(unsigned char *)rcvbuf + rcvLen,remain,500);
        
        if(len > 0)
        {
            dbg_printf(1,"cmd rcv %s",rcvbuf);
            remain -= len;
            rcvLen += len;
            str = strstr((char *)rcvbuf,expect);
            if(str)
            {   
                if(ret != NULL)
                {
                   strcpy(ret,str);
                }
                return TRUE;
            }
        }
        else
        {
            osDelay(20);
        }
        if(remain < 0)
        {
            break;    
        }
    }while(xTaskGetTickCount() - tick < timeout);


    return FALSE;
}

int  lte_comm_test(void *p)
{
    uint8 Wbuf[10] = {1,2,3,4,5,6,7,8,9,0};
    uint8 Rbuf[10] = {0};
    
    int len = 0;
  
    USART_Init(COM4,STOP_1,P_NONE,115200);       //BaudRate
    
    Drv_Uart_Write(COM4,Wbuf,sizeof(Wbuf),10);
    osDelay(100);
    len = Drv_Uart_Read(COM4,Rbuf,20,100);
    
    if(len > 0)
    {
        if(memcmp(Rbuf,Wbuf,sizeof(Wbuf)) == 0)
        {
            dbg_printf(1,"COM4 test OK");
            return 1;
        }
        else
        {
            dbg_printf(1,"COM4 test ERR");
      
        }
    }
    else
    {
        dbg_printf(1,"COM4 test ERR");
        
    }   
    int ReConnectCount = 0;
	do
    {        
        if(AT_Cmd_Parse("+++\r\n", "a",NULL, 500)== FALSE)
        {	
            ReConnectCount++;
            
            osDelay(1000);	
        }
        else
        {
            return 1;
        }
        
    }while(ReConnectCount < 5);
    return 0;
}

 void CAN_test(void *p)
{
    //uint8 Wbuf[10] = {1,2,3,4,5,6,7,8,9,0};
    dbg_printf(1,"start can test");
//    can_test_fun();
}
static void rs485_test(void *p)
{
    //uint8 Wbuf[10] = {1,2,3,4,5,6,7,8,9,0};
    //uint8 Rbuf[10] = {0};
    
    //int len = 0;
    # if 0
    USART_Init(COM3,STOP_1,P_NONE,9600);       //BaudRate
    USART_Init(COM5,STOP_1,P_NONE,9600);       //BaudRate
  
    Drv_Uart_Write(COM3,Wbuf,sizeof(Wbuf),100);
    osDelay(100);
    len = Drv_Uart_Read(COM5,Rbuf,20,100);
    
    if(len > 0)
    {
        if(memcmp(Rbuf,Wbuf,sizeof(Wbuf)) == 0)
        {
            dbg_printf(1,"COM3->COM5 test OK");
        }
        else
        {
            dbg_printf(1,"COM3->COM5 test ERR");
        }
    }
    else
    {
        dbg_printf(1,"COM3->COM5 test ERR");
    }    
    memset(Rbuf,0,sizeof(Rbuf));
    
    Drv_Uart_Write(COM5,Wbuf,sizeof(Wbuf),100);
    osDelay(100);
    len = Drv_Uart_Read(COM3,Rbuf,20,100);
    
    if(len > 0)
    {
        if(memcmp(Rbuf,Wbuf,sizeof(Wbuf)) == 0)
        {
            dbg_printf(1,"COM5->COM3 test OK");
        }
        else
        {
            dbg_printf(1,"COM5->COM3 test ERR");
        }
    }
    else
    {
        dbg_printf(1,"COM5->COM3 test ERR");
    }
		#endif
		
}


static void Out_put_test(void *p)
{
   
    BSP_GPIO_Init();
    
    BSP_GPIO_OutPut(MK1_OUT,OUT_HIGHE);
    dbg_printf(1,"控制KM1输出");
    osDelay(1000);
    BSP_GPIO_OutPut(MK1_OUT,OUT_LOW);
    dbg_printf(1,"关闭KM1输出");
    
    dbg_printf(1,"正向驱动电子锁");
    BSP_GPIO_OutPut(ELECTR1_LOCK1,OUT_HIGHE);
    BSP_GPIO_OutPut(ELECTR1_LOCK2,OUT_LOW);
    osDelay(1000);
    BSP_GPIO_OutPut(ELECTR1_LOCK1,OUT_LOW);
    BSP_GPIO_OutPut(ELECTR1_LOCK2,OUT_LOW);
    dbg_printf(1,"关闭驱动电子锁");
    
    dbg_printf(1,"反向驱动电子锁");
    BSP_GPIO_OutPut(ELECTR1_LOCK1,OUT_LOW);
    BSP_GPIO_OutPut(ELECTR1_LOCK2,OUT_HIGHE);
    osDelay(1000);
    BSP_GPIO_OutPut(ELECTR1_LOCK1,OUT_LOW);
    BSP_GPIO_OutPut(ELECTR1_LOCK2,OUT_LOW);    
    dbg_printf(1,"关闭驱动电子锁"); 

		
}

void In_put_test(void *p)
{
 
   
    InPutDef InPutRet;
    
    InPutRet = BSP_Read_InPut(STOP_DETECT);
    
    if(InPutRet == IN_HIGHE)
    {
        dbg_printf(1,"STOP         断开");
    }
    else
    {
        dbg_printf(1,"STOP         闭合");
    }
    
    InPutRet = BSP_Read_InPut(SPD_DETECT1);
    
    if(InPutRet == IN_HIGHE)
    {
        dbg_printf(1,"SPD          断开");
    }
    else
    {
        dbg_printf(1,"SPD          闭合");
    }    
//    InPutRet = BSP_Read_InPut(GUN_LOCK_DETECT1);
//    
//    if(InPutRet == IN_HIGHE)
//    {
//        dbg_printf(1,"枪监测       断开");
//    }
//    else
//    {
//        dbg_printf(1,"枪监测       闭合");
//    } 
    InPutRet = BSP_Read_InPut(ELECTR_DETECT1);
    
    if(InPutRet == IN_HIGHE)
    {
        dbg_printf(1,"电子锁监测   解锁");
    }
    else
    {
        dbg_printf(1,"电子锁监测   锁定");
    }
    InPutRet = BSP_Read_InPut(KM1_DETECT);
    
    if(InPutRet == IN_HIGHE)
    {
        dbg_printf(1,"KM1监测      断开");
    }
    else
    {
        dbg_printf(1,"KM1监测      闭合");
    }    
}

static int CP_test(void *p)
{
    int iVal;
    int ret = 0;
    BSP_Pwm_Out(53,0);    
    
    int sec = time(NULL);
//    while(1)
    {
        osDelay(1000);
        
        iVal = Get_CP_Sample(0);
        
        dbg_printf(1,"CP = %d.%2.2d",iVal/100,iVal%100);
        
//        osDelay(1000);
//        
//        if(time(NULL) - sec > 5)
//        {
//            break;
//        }
        if(Get_Car_Link(0) == LEVEL_6_V)
        {
            ret  = 1;
        }      
    }
    BSP_Pwm_Out(100,0); 
    dbg_printf(1,"PWM 采样测试结束");
    return ret;
}

static void Led_test(void *p)
{
    dbg_printf(1,"LED 红绿黄交替亮");
//    uint8 led_bit = 0;
//    int i = 0;
//    
//    for(i = 0;i< 2;i++)
//    {       
//        led_bit = (1<<2) + (1<<5);
//        BSP_Led_Data(led_bit);
//        osDelay(500);

//        led_bit = (1<<1) + (1<<4);
//        BSP_Led_Data(led_bit);
//        osDelay(500);    

//        led_bit = (1<<3) + (1<<6);
//        BSP_Led_Data(led_bit);
//        osDelay(500); 
//    }
//    led_bit = 0;
//    BSP_Led_Data(led_bit);

    LED_Function(RED_LED1,LED_On);
   // LED_Function(RED_LED2,LED_On);
    osDelay(500);
    LED_Function(RED_LED1,LED_Off);
   // LED_Function(RED_LED2,LED_Off);     
    
    LED_Function(YALLOW_LED1,LED_On);
   // LED_Function(YALLOW_LED2,LED_On);
    osDelay(500);
    LED_Function(YALLOW_LED1,LED_Off);
   // LED_Function(YALLOW_LED2,LED_Off);
 
    LED_Function(GRERN_LED1,LED_On);
 //   LED_Function(GRERN_LED2,LED_On);
    osDelay(500);
    LED_Function(GRERN_LED1,LED_Off);
 //   LED_Function(GRERN_LED2,LED_Off);    

      
    dbg_printf(1,"LED 测试结束");

}

static int  Measure_Test(void)
{
    uint32_t Buf[3] = {0};
    BSP_GPIO_OutPut(MK1_OUT,OUT_HIGHE);
    BSP_GPIO_OutPut(2,OUT_HIGHE);
    BSP_GPIO_OutPut(1,OUT_HIGHE);
    osDelay(2000);
    RN8209_Read_Val(Buf);;
    BSP_GPIO_OutPut(MK1_OUT,OUT_LOW);
    BSP_GPIO_OutPut(2,OUT_LOW);
    BSP_GPIO_OutPut(1,OUT_LOW);
    
    if(Buf[0] >= 20000 && Buf[0] <= 25000)
    {
        return 1;
    }
    return 0;
    
}
static void MMA8652_Test(void *p)
{
    //uint8 reg = 0;
}

void test_help(void)
{
    dbg_printf(1,"1:SPI flash test");
    dbg_printf(1,"2:时钟测试");
    dbg_printf(1,"3:RS232通讯测试");
    dbg_printf(1,"4:RS465通讯测试");
    dbg_printf(1,"5:输出控制测试");
    dbg_printf(1,"6:输入干接点测试");
    dbg_printf(1,"7:PWM信号测试");
    dbg_printf(1,"8:LED测试");
    dbg_printf(1,"9:测量电能测试");
    dbg_printf(1,"A:加速度传感器测试");
    dbg_printf(1,"B:全部测试");
    dbg_printf(1,"C:电能芯片8209测试");    
    dbg_printf(1,"D:CAN 测试");  
}


int  Measure_8209_test(void)
{
    uint32 ID = 0;
    
    Drv_RN8209_SPI_Init();
    RN8209_Init_Para();//
    ID = RN8209_Read_ID();
    
    if(ID == 0x820900)
    {
        dbg_printf(1,"8209电能测量 OK");
        return 1;
    }
    else
    {
        dbg_printf(1,"8209电能测量 ERR");
        return 0;
    }
    
}
int testFunProc(char *src,void *p)
{
  
    switch(src[0])
    {
        case '0':
        {
            test_help();
            break;
        }
        case '1':
        {
            flash_test(p);
            break;
        }
        case '2':
        {
            rtc_test(p);
            break;
        }
        case '3':
        {
            rs232_test(p);
            break;
        }
        case '4':
        {
            rs485_test(p);
            break;
        }
        case '5':
        {
            Out_put_test(p);
            break;
        }
        case '6':
        {
            In_put_test(p);
            break;
        }
        case '7':
        {
            CP_test(p);
            break;
        }
        case '8':
        {
            Led_test(p);
            break;
        }
        case '9':
        {
            Measure_Test();
            break;
        }
        case 'A':
        {
            MMA8652_Test(p);
            break;
        }
        case 'B':
        {
            flash_test(p);
            rtc_test(p);
            rs232_test(p);
            rs485_test(p);
            Out_put_test(p);
            In_put_test(p);
            CP_test(p);
            Led_test(p);            
            break;
        }
        case 'C':
        {
            Measure_8209_test();
            break;
        }
        case 'D':
        {
            CAN_test(p);
            break;
        }
        
    }
    
    return 0;
}

void out_test()
{
    BSP_GPIO_Init();
    
    BSP_GPIO_OutPut(MK1_OUT,OUT_HIGHE);
    dbg_printf(1,"控制KM1输出");
    osDelay(1000);
    BSP_GPIO_OutPut(MK1_OUT,OUT_LOW);
    dbg_printf(1,"关闭KM1输出");
    
    dbg_printf(1,"正向驱动电子锁");
    BSP_GPIO_OutPut(ELECTR1_LOCK1,OUT_HIGHE);
    //BSP_GPIO_OutPut(ELECTR1_LOCK2,OUT_LOW);
    osDelay(1000);
    BSP_GPIO_OutPut(ELECTR1_LOCK1,OUT_LOW);
    //BSP_GPIO_OutPut(ELECTR1_LOCK2,OUT_LOW);
    dbg_printf(1,"关闭驱动电子锁");
    
    dbg_printf(1,"反向驱动电子锁");
    BSP_GPIO_OutPut(ELECTR1_LOCK1,OUT_LOW);
    //BSP_GPIO_OutPut(ELECTR1_LOCK2,OUT_HIGHE);
    osDelay(1000);
    BSP_GPIO_OutPut(ELECTR1_LOCK1,OUT_LOW);
    //BSP_GPIO_OutPut(ELECTR1_LOCK2,OUT_LOW);    
    dbg_printf(1,"关闭驱动电子锁");  

//    BSP_GPIO_OutPut(MK2_OUT,OUT_HIGHE);
    dbg_printf(1,"控制KM2输出");
    osDelay(1000);
//    BSP_GPIO_OutPut(MK2_OUT,OUT_LOW);
    dbg_printf(1,"关闭KM2输出");
    		
}
int all_test(void)
{
    static int Terminateflag = 0;
    extern osThreadId JWT_Thread_Id;  
    extern osThreadId Charge_Ctrl_Thread_Id;  
    if(Terminateflag == 0)
    {
        osThreadTerminate(Charge_Ctrl_Thread_Id);
        osThreadTerminate(JWT_Thread_Id);
        Terminateflag = 1;
    }
    
    LED_Function(YALLOW_LED1,LED_Off);
    LED_Function(RED_LED1,LED_Off);
    LED_Function(GRERN_LED1,LED_Off);
    
    if(flash_test(NULL) == 0)
    {
        return 0;
    }
    if(rtc_test(NULL) == 0)
    {
        return 0;
    }
    if(rs232_test(NULL) == 0)
    {
        return 0;
    }
    if(CP_test(NULL) == 0)
    {
        return 0;
    }
    if(lte_comm_test(NULL) == 0)
    {
        return 0;
    }
    
    if(Measure_8209_test() == 0)
    {
        return 0;
    }
    
    if(Measure_Test() == 0)
    {
        return 0;
    }
    
    return 1;
    

}

void board_test(void)
{
    InPutDef Ret ;
    int test_pass = 0;
    Ret = BSP_Read_InPut(SPD_DETECT1);
    if(Ret == IN_LOW)
    {
        if(all_test() == 0)
        {
            test_pass = 0;//LED_Function(RED_LED1,LED_On);
        }
        else
        {
            test_pass = 1;
        }
        while(1)
        {
            Ret = BSP_Read_InPut(SPD_DETECT1);
            if(Ret != IN_LOW)
            {
                LED_Function(YALLOW_LED1,LED_Off);
                LED_Function(RED_LED1,LED_Off);
                
                LED_Function(NET_LED,LED_Off);
                LED_Function(GRERN_LED1,LED_Off);                    
                break;
            }
            if(test_pass)
            {
                LED_Function(YALLOW_LED1,LED_On);
                LED_Function(RED_LED1,LED_On);
                LED_Function(NET_LED,LED_On);
                LED_Function(GRERN_LED1,LED_On);               
            }
            else
            {
                LED_Function(RED_LED1,LED_Toggle);
            }
            osDelay(200);
            BSP_WatchDog_Refresh();//喂狗
        }        
    }
}
