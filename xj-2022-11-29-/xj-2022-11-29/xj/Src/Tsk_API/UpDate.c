#include "main.h"
#include "cmsis_os.h"
//#include "mb.h"
//#include "mb_m.h"
#include "bsp_driver_uart.h"
#include "drv_usart.h"
#include "bsp_Led_Function.h"
#include "BSP_WatchDog.h"
#include "Sys_Config.h"

#include "DRV_SPIFLASH.h"
#include "string.h"
#include "CRC16.h"
#include "Task_SpiFlash.h"
#include "Task_API_SpiFlash.h"
#include "Update.h"
#include "ftp.h"

#include "lwip/sockets.h"
#include <stdio.h>
#include "lwip/netdb.h"
#include <string.h>
#include "lwip/api.h"
#include "cmsis_os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include "lwip/dns.h"
#include "Utility.h"
#include "flash_if.h"

#define DBG_DTU 1
#define GPRS_COM  COM4
#define  GPRS_DBG 1

static netbuf *conn = NULL;

#define DEF_APP_SIZE  196 * 1024 

static uint32 g_addr = 0;
static uint32 g_fileLen = 0;
static uint8 g_fileChecksum = 0;

int ftp_connect(const char *fip,int fport,const char *fuser,const char *fpsw)
{
    
    if (!FtpConnect(fip,fport,&conn))
    {
        dbg_printf(1,"ftp connect err");
        return 0;
    }
    
    dbg_printf(1," ftp connect ok");
    osDelay(100);
    if (!FtpLogin(fuser,fpsw,conn))
    {
		dbg_printf(1,"ftp login err");
		return 0;
    }
	dbg_printf(1,"login ok");
	return 1;
}

int connect_serv(char *host,int port)
{
	int sControl;
    struct sockaddr_in sin;

    memset(&sin,0,sizeof(sin));
	sin.sin_family=AF_INET;
	sin.sin_addr.s_addr= inet_addr(host);
	sin.sin_port=htons(port);	

    sControl = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sControl == -1)
    {
        if (ftplib_debug)
            perror("socket");
        return -1;
    }

    if (connect(sControl, (struct sockaddr *)&sin, sizeof(sin)) == -1)
    {
        if (ftplib_debug)
            perror("connect");
        closesocket(sControl);
        return -1;
    }
    
	dbg_printf(1,"connect ok");
	return sControl;
}


int ftp_updata_program(const char *localfile, const char *path,
	netbuf *nControl,int typ,int mode)
{

	uint8_t   ret=1;
    int adr = APPLICATION_ADDRESS;
    int read_len;
	int err_cnt = 0;
    uint8 *dbuf;
    netbuf *nData;
    int FileSize = DEF_APP_SIZE;
    
    int fileLen = 0;
    int fileChecksum = 0;
    
    if (!FtpAccess(path, typ, mode, nControl, &nData))
    {
		dbg_printf(1,"call ftpaccess fila ");
		return 0;
    }
    if(!FtpSize(path,(unsigned int *)&FileSize,mode,nControl))
    {
        dbg_printf(1,"call ftpSize err ");        
        return 0;
    }
    
    dbuf = malloc(FTPLIB_BUFSIZ);

    FLASH_If_Init();
    
    FLASH_If_Erase(APPLICATION_ADDRESS);
      
    while (1)
    {
        read_len = FtpRead(dbuf, FTPLIB_BUFSIZ, nData);
        
        if(read_len > 0)
        {        
            FLASH_If_Write(adr,(uint32 *)dbuf,read_len);   //保存文件内容            
            adr += read_len;
            fileLen +=read_len;
            fileChecksum += Checkout_Sum(dbuf,read_len);  //计算文件较验
            if (read_len !=FTPLIB_BUFSIZ) //结束
            {
                ret=1;
            }
            osDelay(5);
            err_cnt = 0;
            
            if(fileLen >= FileSize)
            {
                ret = 1;
                break;
            }                  
        }
        else
        {
            err_cnt++;
            if(fileLen >= FileSize)
            {
                ret = 1;
                break;
            }
            if(err_cnt > 20)
            {
                ret = 0;
                break;
            }        
            osDelay(50);  
        }
    }
    
    free(dbuf);
    
    return ret;    
}


int ftp_proc_file(const char *localfile, const char *path,
	netbuf *nControl,int typ,int mode)
{

	uint8_t   ret=1;
    int adr = SPI_FLASHE_ADDR;
    int read_len;
	int err_cnt = 0;
    uint8 *dbuf;
    netbuf *nData;
    int FileSize = DEF_APP_SIZE;
    
    int fileLen = 0;
    char fileChecksum = 0;

//    for (adr = SPI_FLASHE_ADDR;adr <  (SPI_FLASHE_ADDR + APP_SIZE);adr += 4096)
//    {
//        BSP_SPIFLASH_SectorErase(adr);
//    }
    
    if (!FtpAccess(path, typ, mode, nControl, &nData))
    {
		dbg_printf(1,"call ftpaccess fila ");
		return 0;
    }
    else
    {
        dbg_printf(1,"start trasfer data >");
    }
    
//    if(!FtpSize(path,(unsigned int *)&FileSize,mode,nControl))
//    {
//        dbg_printf(1,"call ftpSize err ");
//        FileSize = 4 * MIN_APP_SIZE;
//        //return 0;
//    }
    
    dbuf = malloc(FTPLIB_BUFSIZ);

    if(dbuf == NULL)
    {
        dbg_printf(1,"malloc err %d",FTPLIB_BUFSIZ);
        return 0;
    }
    osDelay(2000);
    adr = SPI_FLASHE_ADDR;    
    while (1)
    {
        read_len = FtpRead(dbuf, FTPLIB_BUFSIZ, nData);
        
        if(read_len > 0)
        {

            BSP_SPIFLASH_BufferWrite(dbuf,adr,read_len);   //保存文件内容
            adr += read_len;
            fileLen +=read_len;                
            if (((fileLen * 100 / FileSize) % 10) == 0)
            {
                dbg_printf(1,"ftp complete %d ",(fileLen *100 / FileSize));
            } 
            
            if(fileLen >= FileSize)
            {
                fileChecksum += Checkout_Sum(dbuf,read_len - 1);  //计算文件较验                
                if(fileChecksum == dbuf[read_len - 1])
                {
                    ret = 1;
                    dbg_printf(1,"check ok 0x%x ",fileChecksum);
                }
                else
                {
                    dbg_printf(1,"check err %x ",fileChecksum);
                    ret = 0;
                }
                break;
            }
            else
            {
                fileChecksum += Checkout_Sum(dbuf,read_len);  //计算文件较验    
            }                   
            osDelay(5);
            err_cnt = 0;
            
                 
        }
        else
        {
            err_cnt++;
            dbg_printf(1,"ftp rcv err %d",err_cnt);
            if(err_cnt < 20)
            {
                osDelay(err_cnt * 100);
                ret = 0;
            }
            if(err_cnt > 20)
            {
                if(fileLen < DEF_APP_SIZE)
                {
                    ret = 0;
                }
                else
                {
                    ret = 1;
                }
                break;
            }        
            osDelay(50);  
        }
    }
    
    free(dbuf);
    
    return ret;    
}
/*gprs ftp 升级*/


/******************************************************************************
* Function: 
* Parameter:
* Return:   Void
* Description:
*   
*
******************************************************************************/


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
    
    dbg_printf(DBG_DTU,"send %s",send);
    
    do
    {
        len = Drv_Uart_Read(COM4,(unsigned char *)rcvbuf + rcvLen,remain,500);
        
        if(len > 0)
        {
            dbg_printf(GPRS_DBG,"cmd rcv %s",rcvbuf);
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

int m26_gprs_read_data(uint8_t *data,int len,int timeout)
{
    char send[64] = {0};

    uint32_t tick = HAL_GetTick();
    int rcvLen = 0;
    int index = 0;
    char *str[6];
    Drv_Uart_Read(GPRS_COM,data,len,400);
    memset(data,0,len);
    
    sprintf(send,"AT+QIRD=%d,1,0,%d\r\n",0,len);
    Drv_Uart_Write(GPRS_COM,(const unsigned char *) send,strlen(send),100);
    dbg_printf(GPRS_DBG,"read cmd %s",send);
    do
    {
    
        rcvLen = Drv_Uart_Read(GPRS_COM,data,len,400);
        
        if(rcvLen > 0)
        {
            dbg_printf(GPRS_DBG,"gprs rcv %s",data);
            
            str[0] = strstr((char*)data,"+QIRD:");
            
            if(str[0])//"+QIDR:ip:port,type,lenth data,OK"
            {
                str[1] = strstr(&str[0][6],":");
                //  
                if(str[1])//PORT
                {
                    str[2] = strstr(&str[1][1],",");

                    if(str[2])//,type
                    {
                        str[3] = strstr(&str[2][1],",");
                        if(str[3])//,len
                        {

                            int i = 0;
                            index = atoi(&str[3][1]);
                            
                            if(index == 0)
                            {
                                return 0;
                            }

                            for(i = 10; i < rcvLen;i++)
                            {
                                if(data[i- 1] == 0x0d && data[i] == 0x0a)
                                {   
                                    break;
                                }
                            }
                            
                            //dbg_printf(GPRS_DBG,"data len %d",i);
                            
                            memmove(data,&data[i+1],index);
                            
                            return index;
                        }
                    }
                }
            }
            else if(strstr((char*)data,"OK\r\n"))
            {
                return 0;
            }
            else if(strstr((char*)data,"ERROR"))
            {
                return -1;
            }
        }
    }while(HAL_GetTick() - tick < timeout);
    return 0;  
}



static int conect_server(char *ser,uint16 port)
{
    char send[64] = {0};
    uint8 data[64] = {0};

    uint32_t tick = HAL_GetTick();
    int rcvLen = 0;
    int index = 0;
    char *str[6];
    int len = sizeof(data);
    
    Drv_Uart_Read(GPRS_COM,data,len,400);
    memset(data,0,len);
    
    sprintf(send,"AT+QFTPOPEN=\"%s\",%d\r\n",ser,port);
    Drv_Uart_Write(GPRS_COM,(const unsigned char *) send,strlen(send),100);
    dbg_printf(GPRS_DBG,"read cmd %s",send);
    do
    {
    
        rcvLen = Drv_Uart_Read(GPRS_COM,data,len,400);
        
        if(rcvLen > 0)
        {
            dbg_printf(GPRS_DBG,"gprs rcv %s",data);
            
            str[0] = strstr((char*)data,"+QFTPOPEN:");
            
            if(str[0])//+QFTPOPEN:0
            {
           
                index = atoi(&str[0][10]);
                
                if(index == 0)
                {
                    return 0;
                }
                else
                {
                    return -1;
                }
            }
            else if(strstr((char*)data,"ERROR"))
            {
                return -1;
            }
        }
    }while(HAL_GetTick() - tick < 8000);

    return -1;
    
}


static int ftp_get_file_len(void)
{
    char send[64] = {0};
    uint8 data[64] = {0};

    uint32_t tick = HAL_GetTick();
    int rcvLen = 0;
    int index = 0;
    char *str[6];
    int len = sizeof(data);
    
    Drv_Uart_Read(GPRS_COM,data,len,400);
    memset(data,0,len);
    
    sprintf(send,"AT+QFTPLEN\r\n");
    Drv_Uart_Write(GPRS_COM,(const unsigned char *) send,strlen(send),100);
    dbg_printf(GPRS_DBG,"read cmd %s",send);
    do
    {
    
        rcvLen = Drv_Uart_Read(GPRS_COM,data,len,400);
        
        if(rcvLen > 0)
        {
            dbg_printf(GPRS_DBG,"gprs rcv %s",data);
            
            str[0] = strstr((char*)data,"+QFTPLEN:");
            
            if(str[0])//+QFTPOPEN:0
            {
        
                index = atoi(&str[0][9]);
                
                return index;
            }
            else if(strstr((char*)data,"ERROR"))
            {
                return -1;
            }
        }
    }while(HAL_GetTick() - tick < 5000);

    return -1;
    
}

static int ftp_get_file_len_lte(void)
{
    char send[64] = {0};
    uint8 data[64] = {0};

    uint32_t tick = HAL_GetTick();
    int rcvLen = 0;
    int index = 0;
    char *str[6];
    int len = sizeof(data);
    
    Drv_Uart_Read(GPRS_COM,data,len,400);
    memset(data,0,len);
    
    sprintf(send,"AT+QFTPLEN\r\n");
    Drv_Uart_Write(GPRS_COM,(const unsigned char *) send,strlen(send),100);
    dbg_printf(GPRS_DBG,"read cmd %s",send);
    do
    {
    
        rcvLen = Drv_Uart_Read(GPRS_COM,data,len,400);
        
        if(rcvLen > 0)
        {
            dbg_printf(GPRS_DBG,"gprs rcv %s",data);
            
            str[0] = strstr((char*)data,"+QFTPLEN: 0,");
            
            if(str[0])//+QFTPOPEN:0
            {
        
                index = atoi(&str[0][12]);
                
                return index;
            }
            else if(strstr((char*)data,"ERROR"))
            {
                return -1;
            }
        }
    }while(HAL_GetTick() - tick < 5000);

    return -1;
    
}


static int ftp_get_file(void)
{
    char send[64] = {0};
    uint8 data[64] = {0};

    uint32_t tick = time(NULL);
    int rcvLen = 0;
    int index = 0;
    char *str[6];
    int len = sizeof(data);
    
    Drv_Uart_Read(GPRS_COM,data,len,400);
    memset(data,0,len);
    
    sprintf(send,"AT+QFTPGET=\"EVA_App.bin\"\r\n");
    
    Drv_Uart_Write(GPRS_COM,(const unsigned char *) send,strlen(send),100);
    dbg_printf(GPRS_DBG,"read cmd %s",send);
    
    do
    {
    
        rcvLen = Drv_Uart_Read(GPRS_COM,data,len,400);
        
        if(rcvLen > 0)
        {
            //dbg_printf(GPRS_DBG,"gprs rcv %s",data);
            
            str[0] = strstr((char*)data,"+QFTPGET:");
            
            if(str[0])//+QFTPOPEN:0
            {
  
                index = atoi(&str[0][8]);
                
                return index;

            }
            else if(strstr((char*)data,"ERROR"))
            {
                return -1;
            }
        }
    }while(time(NULL)- tick < 120);// 120S

    return -1;
    
}

static int ftp_get_file_lte(void)
{
    char send[64] = {0};
    uint8 data[64] = {0};

    uint32_t tick = time(NULL);
    int rcvLen = 0;
    int index = 0;
    char *str[6];
    int len = sizeof(data);
    
    Drv_Uart_Read(GPRS_COM,data,len,400);
    memset(data,0,len);
    
    sprintf(send,"AT+QFTPGET=\"EVA_App.bin\",\"RAM:EVA_App.bin\"\r\n"); 
    
    Drv_Uart_Write(GPRS_COM,(const unsigned char *) send,strlen(send),100);
    dbg_printf(1,"read cmd %s",send);
    
    do
    {
    
        rcvLen = Drv_Uart_Read(GPRS_COM,data,len,400);
        
        if(rcvLen > 0)
        {
            dbg_printf(GPRS_DBG,"gprs rcv %s",data);
            
            str[0] = strstr((char*)data,"+QFTPGET:");
            
            if(str[0])//+QFTPOPEN:0
            {
  
                index = atoi(&str[0][8]);
                
                return index;

            }
            else if(strstr((char*)data,"ERROR"))
            {
                return -1;
            }
        }
    }while(time(NULL)- tick < 120);// 120S

    return -1;
    
}

static int get_ram_file_fd(void)
{
    char send[64] = {0};
    uint8 data[64] = {0};

    uint32_t tick = HAL_GetTick();
    int rcvLen = 0;
    int index = 0;
    char *str[6];
    int len = sizeof(data);
    
    Drv_Uart_Read(GPRS_COM,data,len,400);
    memset(data,0,len);
    
    sprintf(send,"AT+QFOPEN=\"RAM:EVA_App.bin\",%d\r\n",2);
    
    Drv_Uart_Write(GPRS_COM,(const unsigned char *) send,strlen(send),100);
    dbg_printf(GPRS_DBG,"read cmd %s",send);
    do
    {
    
        rcvLen = Drv_Uart_Read(GPRS_COM,data,len,400);
        
        if(rcvLen > 0)
        {
            dbg_printf(GPRS_DBG,"gprs rcv %s",data);
            
            str[0] = strstr((char*)data,"+QFOPEN:");
            
            if(str[0])//+QFOPEN:
            {
    
                index = atoi(&str[0][8]);
                return index;
            }
            else if(strstr((char*)data,"ERROR"))
            {
                return -1;
            }
        }
    }while(HAL_GetTick() - tick < 30000);

    return -1;
}

void init_program(void)
{
    int adr = SPI_FLASHE_ADDR;
    uint8 read[32] = {0};
    uint8 empty[32] = {0};
    //清空数据
    BSP_SPIFLASH_BufferRead((uint8_t*)&read,adr,sizeof(empty));
    
    memset(empty,0xff,sizeof(empty));
    
    if(memcmp(empty,read,sizeof(read)))
    {
        for (adr = SPI_FLASHE_ADDR;adr <  (SPI_FLASHE_ADDR + APP_SIZE);adr += 4096)
        {
            BSP_SPIFLASH_SectorErase(adr);
        }          
    }     
    
    g_addr = SPI_FLASHE_ADDR;
    g_fileLen = 0;
    g_fileChecksum = 0;    
}

static int  write_file(uint8 *dbuf,int read_len)
{
    //
    BSP_SPIFLASH_BufferWrite(dbuf,g_addr,read_len);   //保存文件内容
    g_addr += read_len;
    g_fileLen +=read_len;
    
    if(g_fileLen == DEF_APP_SIZE)
    {
        g_fileChecksum += Checkout_Sum(dbuf,read_len - 1);  //计算文件较验
    }
    else
    {
        g_fileChecksum += Checkout_Sum(dbuf,read_len);  //计算文件较验
    }

    if(g_fileLen == DEF_APP_SIZE)
    {
        if(g_fileChecksum == dbuf[read_len - 1])
        {
            dbg_printf(DBG_DTU,"write_file ok~~~~~~~~~~~~!!!");
            return 0;   
        }
        dbg_printf(DBG_DTU,"write_file crc err!!!");
        return -1;
    }

    return 1;
}


int program_file(uint8 *dbuf,int read_len)
{
    //
    BSP_SPIFLASH_BufferWrite(dbuf,g_addr,read_len);   //保存文件内容
    g_addr += read_len;
    return 1;
}

int check_file(void)
{
    //
    uint8 buf[256] = {0};
    uint32 addr = 0;
    uint8 Sum = 0;
    int len = sizeof(buf);
    for(addr =SPI_FLASHE_ADDR;addr < APP_SIZE;)
    {
        BSP_SPIFLASH_BufferRead(buf,addr,len);
     
        addr += sizeof(buf);
        
        if(addr >= APP_SIZE)
        {
            Sum += Checkout_Sum(buf,len - 1);  //计算文件较验           
            if(Sum == buf[len - 1])
            {
            dbg_printf(1,"Sum1:%x",Sum);
                return 1;
            }
        }
        else
        {
            Sum += Checkout_Sum(buf,len);  //计算文件较验
        }
    }
    
	dbg_printf(1,"Sum:%x",Sum);	
    return 0;
}


static int read_ram_file(int fd,int filelen)
{
    char send[64] = {0};
    uint8 data[286] = {0};

    uint32_t tick = HAL_GetTick();
    int rcvLen = 0;
    int read_len = 0;
    int remain_len = 0;
    char *str[6];
    int len = sizeof(data);
    remain_len = len;
    Drv_Uart_Read(GPRS_COM,data,len,10);
    memset(data,0,len);
    
    sprintf(send,"AT+QFREAD=%d,%d\r\n",fd,filelen);
    
    Drv_Uart_Write(GPRS_COM,(const unsigned char *) send,strlen(send),100);
    
    dbg_printf(GPRS_DBG,"read cmd %s",send);
    
    //osDelay(200);
    
    do
    {
    
        read_len = Drv_Uart_Read(GPRS_COM,data + rcvLen,remain_len,400);
        
        if(read_len > 0)
        {
            rcvLen += read_len;
            remain_len -= read_len;
            if(rcvLen >= sizeof(data))
            {
                return -1;
            }
            //dbg_printf(GPRS_DBG,"gprs rcv %s",data);
            
            str[0] = strstr((char*)data,"CONNECT");
            
            if(str[0])//+QFOPEN:
            {
                //
                int index = 0;
                
                index = atoi(&str[0][7]);
                if(index > 0 && rcvLen > index + 16)
                {
                    int k = 0;
                    
                    for(k = 0; k < rcvLen;k++)
                    {
                        if(str[0][7 + k] == 0x0d 
                            && str[0][7 + k + 1] == 0x0a )
                        {
                            break;
                        }
                    }
                    
                    dbg_printf(GPRS_DBG,"write data-> %d",index);
                    print_hex_data((uint8*)&str[0][7 + k + 2],16);
                    return write_file((uint8*)&str[0][7 + k + 2],index); 
                }
                else
                {
                    return -1;
                }
            }
            else if(strstr((char*)data,"ERROR"))
            {
                return -1;
            }
        }
        else
        {
            osDelay(50);
        }
    }while(HAL_GetTick() - tick < 5000);

    return -1;
}


static int set_file_offset(int fd,uint32 offset)
{
    char cmd[64] = {0};

    sprintf(cmd,"AT+QFSEEK=%d,%d,0\r\n",fd,offset);

    if(AT_Cmd_Parse(cmd, "OK",NULL,1000)== FALSE)     
    {   
        dbg_printf(DBG_DTU,"err QFSEEK");
        return 0;
    }
    return 1;
}


static int close_ftp(void)
{
    char cmd[64] = {0};

    sprintf(cmd,"AT+QFTPCLOSE\r\n");

    if(AT_Cmd_Parse(cmd, "OK",NULL,5000)== FALSE)     
    {   
        dbg_printf(DBG_DTU,"err QFTPCLOSE");
        return 0;
    }
    return 1;
}


//使用TFP下载文件
int  ftp_doload_file(UpdateInfo* pInfo)
{
    
    #define READ_LEN  256
    int cnt = 0;
    char cmd[64] = {0};
    uint8 path[64] = {0};
    int path_len = 0;
    uint32 get_len = 0;
    int fd = 0;
    uint32 remain = 0;
    uint32 down_len = 0;
    uint32 offset = 0;
    int ret = 0;
    int connct = 0;

    do
    {
        if(AT_Cmd_Parse("+++++\r\n", "OK",NULL, 500)== FALSE)
        {   
            cnt ++; 
        }
        else
        {
            break;
        }
    }while(cnt <3);

    if(cnt >=3)
    {
        dbg_printf(DBG_DTU,"err 3");
        return 0;
    }
    
    //发送4G模块初始化设置请求
    osDelay(500);
    sprintf(cmd,"set_param WorkMode %d\r\n",1);
    
    if(AT_Cmd_Parse(cmd, "OK",NULL,1000)== FALSE)     
    {   
        dbg_printf(DBG_DTU,"err WorkMode");
        return 0;
    }
    
    osDelay(1000);
    memset(cmd,0,sizeof(cmd));
    dbg_printf(1,"set data mode %d");
    //进入透传模式到模块
    cnt = 0;
    do
    {
        if(AT_Cmd_Parse("AT+DATA \r\n","OK",NULL, 1000)== FALSE)  
        {   
            cnt ++; 
        }
        else
        {
            break;
        }
    }while(cnt <3);

    if(cnt >=3)
    {
        dbg_printf(DBG_DTU,"err 3");
        return 0;
    }    
    
    osDelay(1000);
    //连接到服务器
    while(1)
    {
        
        memset(cmd,0,sizeof(cmd));
        sprintf(cmd,"AT+QIFGCNT=%d\r\n",0);
        if(AT_Cmd_Parse(cmd,"OK",NULL, 2000)== FALSE) 
        {
            dbg_printf(DBG_DTU,"err QIFGCNT");
            
        }
        osDelay(500);
        
        //关掉 FTP
        //close_ftp();  
        //set user
        memset(cmd,0,sizeof(cmd));
        sprintf(cmd,"AT+QFTPUSER=\"%s\"\r\n",pInfo->ftpUsr);
        if(AT_Cmd_Parse(cmd, "OK",NULL,5000)== FALSE)     
        {   
            dbg_printf(DBG_DTU,"err QFTPUSER");
            return 0;
        }
        osDelay(200);  
        //set pass
        memset(cmd,0,sizeof(cmd));
        sprintf(cmd,"AT+QFTPPASS=\"%s\"\r\n",pInfo->ftpPass);
        
        if(AT_Cmd_Parse(cmd, "OK",NULL,5000)== FALSE)     
        {   
            dbg_printf(DBG_DTU,"err QFTPPASS");
            return 0;
        }
        osDelay(500);  
        //set addr and port
        if(conect_server(pInfo->ftpSer,pInfo->ftpPort) != 0)
        {
            close_ftp();
            dbg_printf(DBG_DTU,"err conect_server %d",connct);
            connct++;
            osDelay(3000);  
            if(connct > 3)
            {
                return 0;
            }
        }
        else
        {
            break;
        }
    }
    osDelay(200);
    //set path
    memset(cmd,0,sizeof(cmd));
    memset(path,0,sizeof(path));
    path_len = strlen(pInfo->ftpFile);
    path_len -= 11;//strlen("EVA_App.bin");
    memcpy(path,pInfo->ftpFile,path_len);
    sprintf(cmd,"AT+QFTPPATH=\"%s\"\r\n",path);
    
    if(AT_Cmd_Parse(cmd, "OK",NULL,5000)== FALSE)     
    {   
        close_ftp();
        dbg_printf(DBG_DTU,"err QFTPPATH");
        return 0;
    }

    //删掉文件
    memset(cmd,0,sizeof(cmd));
    sprintf(cmd,"AT+QFDEL=\"RAM:*\"\r\n"); 
    if(AT_Cmd_Parse(cmd, "OK",NULL,5000)== FALSE)     
    { 
        dbg_printf(DBG_DTU,"err QFDEL");
    } 
    
    //下载文件到RAM
    
    //AT+QFTPCFG="RAM:EVA_App.bin"
    memset(cmd,0,sizeof(cmd));
    sprintf(cmd,"AT+QFTPCFG=4,\"/RAM/EVA_App.bin\"\r\n"); 
    if(AT_Cmd_Parse(cmd, "OK",NULL,5000)== FALSE)     
    {   
        close_ftp();
        dbg_printf(DBG_DTU,"err QFTPCFG 4");
        return 0;
    } 
    //偏移地址AT+QFTPCFG=3,0
    memset(cmd,0,sizeof(cmd));
    sprintf(cmd,"AT+QFTPCFG=3,0\r\n");
    
    if(AT_Cmd_Parse(cmd, "OK",NULL,5000)== FALSE)     
    {   
        close_ftp();
        dbg_printf(DBG_DTU,"err QFTPCFG 3");
        return 0;
    }
    //下载文件
    if(ftp_get_file() != 0)
    {
        close_ftp();
        dbg_printf(DBG_DTU,"err ftp_get_file 1");
        return 0;
    }
    //
    get_len = ftp_get_file_len();
    down_len = get_len;
    
    if(get_len < DEF_APP_SIZE)
    {
        remain = DEF_APP_SIZE - get_len;
    }
    
    if(get_len <= 0)
    {
        close_ftp();
        dbg_printf(DBG_DTU,"err get_len %d",get_len);
        return 0;
    }

    
    fd = get_ram_file_fd();
    offset = 0;
    
    //初始化地址
    g_addr = SPI_FLASHE_ADDR;
    g_fileLen = 0;
    g_fileChecksum = 0;
    set_file_offset(fd,0);
    
    //下载文件到本地
    if(fd <= 0)
    {
        close_ftp();
        dbg_printf(DBG_DTU,"err fd %d",fd);
        return 0;
    }
    else
    {
        do
        {
            //
            int read_len = 0;
            
            if(get_len > READ_LEN)
            {
                read_len = READ_LEN;
            }
            else
            {
                read_len = get_len;
            }
            
            ret = read_ram_file(fd,read_len);
            if(ret == 1)
            {
                get_len -= read_len;
                offset += read_len;
                ret = 0;
            }
            else if(ret == 0)
            {
                ret = 1;
                close_ftp();
                return ret;
            }
            else
            {
                set_file_offset(fd,offset);
            }                
            
        }while(get_len >0);   
    }
    //关闭文件
    //
    memset(cmd,0,sizeof(cmd));
    sprintf(cmd,"AT+QFCLOSE=%d\r\n",fd); 
    if(AT_Cmd_Parse(cmd, "OK",NULL,1000)== FALSE)     
    { 
        dbg_printf(DBG_DTU,"err QFCLOSE");
    }       
    //删掉文件
    memset(cmd,0,sizeof(cmd));
    sprintf(cmd,"AT+QFDEL=\"RAM:*\"\r\n"); 
    if(AT_Cmd_Parse(cmd, "OK",NULL,1000)== FALSE)     
    { 
        dbg_printf(DBG_DTU,"err QFDEL");
    }     

    //下载剩余的文件

    //AT+QFTPCFG="RAM:EVA_App.bin"
    memset(cmd,0,sizeof(cmd));
    sprintf(cmd,"AT+QFTPCFG=4,\"/RAM/EVA_App.bin\"\r\n"); 
    if(AT_Cmd_Parse(cmd, "OK",NULL,1000)== FALSE)     
    {   
        close_ftp();
        dbg_printf(DBG_DTU,"err QFTPCFG 4 1");
        return 0;
    } 
    //偏移地址AT+QFTPCFG=3,0
    memset(cmd,0,sizeof(cmd));
    sprintf(cmd,"AT+QFTPCFG=3,%d\r\n",down_len);
    
    if(AT_Cmd_Parse(cmd, "OK",NULL,1000)== FALSE)     
    {   
        close_ftp();
        dbg_printf(DBG_DTU,"err QFTPCFG 3 1");
        return 0;
    }
    //下载文件
    if(ftp_get_file() != 0)
    {
        close_ftp();
        dbg_printf(DBG_DTU,"err ftp_get_file 3 1");
        return 0;
    }
    //获取文件长度
    get_len = ftp_get_file_len();

    if(get_len != remain)
    {
        close_ftp();
        dbg_printf(DBG_DTU,"err get_len != remain 1");
        return 0;
    }
    
    if(get_len <= 0)
    {
        close_ftp();
        dbg_printf(DBG_DTU,"err get_len < 0");
        return 0;
    }
    //打开文件
    fd = get_ram_file_fd();
    offset = 0;
    //设置文件偏移地址
    set_file_offset(fd,0);
    
    //下载文件到本地
    if(fd <= 0)
    {
        close_ftp();
        return 0;
    }
    else
    {
        
        do
        {
            //
            int read_len = 0;
            
            if(get_len > READ_LEN)
            {
                read_len = READ_LEN;
            }
            else
            {
                read_len = get_len;
            }
            ret = read_ram_file(fd,read_len);
            if(ret == 1)
            {
                get_len -= read_len;
                offset += read_len;
                ret = 0;
            }
            else if(ret == 0)
            {
                ret = 1;
                break;
            }
            else
            {
                ret =  -1;
                set_file_offset(fd,offset);
            }
            
        }while(get_len > 0);   
    }    
    //关闭文件
    memset(cmd,0,sizeof(cmd));
    sprintf(cmd,"AT+QFCLOSE=%d\r\n",fd); 
    if(AT_Cmd_Parse(cmd, "OK",NULL,1000)== FALSE)     
    { 
        dbg_printf(DBG_DTU,"err QFCLOSE");
    }
    //关闭FTP
    close_ftp();
   
    if(ret == 1)
    {
        dbg_printf(1,"FTP升级成功");
    }
    else
    {
        dbg_printf(1,"FTP升级失败");
    }
    return ret;
}


//使用TFP下载文件
int  ftp_doload_file_lte(UpdateInfo* pInfo)
{
    
    #define READ_LEN  256
    int cnt = 0;
    char cmd[64] = {0};
    uint8 path[64] = {0};
    int path_len = 0;
    uint32 get_len = 0;
    int fd = 0;
    uint32 remain = 0;
    uint32 down_len = 0;
    uint32 offset = 0;
    int ret = 0;
    int connct = 0;
    int i = 0;
    
    for(i = 0; i < 5;i++)
    {
        cnt = 0;
        do
        {
            if(AT_Cmd_Parse("+++++\r\n", "OK",NULL, 500)== FALSE)
            {   
                cnt ++; 
            }
            else
            {
                break;
            }
        }while(cnt <3);

        if(cnt >=3)
        {
            dbg_printf(DBG_DTU,"err 3");
            continue;
        }
        
        //发送4G模块初始化设置请求
        osDelay(500);
        sprintf(cmd,"set_param WorkMode %d\r\n",1);
        cnt = 0;
        if(AT_Cmd_Parse(cmd, "OK",NULL,1000)== FALSE)     
        {   
            dbg_printf(DBG_DTU,"err WorkMode");
            osDelay(3000);
            continue;
        }
        
        osDelay(1000);
        memset(cmd,0,sizeof(cmd));
        dbg_printf(1,"set data mode %d");
        //进入透传模式到模块
        cnt = 0;
        do
        {
            if(AT_Cmd_Parse("AT+DATA \r\n","OK",NULL, 2000)== FALSE)  
            {   
                cnt ++; 
            }
            else
            {
                break;
            }
        }while(cnt <3);

        if(cnt >=3)
        {
            dbg_printf(DBG_DTU,"err 3");
            continue;
        }
        else
        {
            break;
        }
        
    }    
    
    if(i>=5)
    {
        dbg_printf(DBG_DTU,"err ++++++");
        return 0;
    }
    //连接到服务器
    while(1)
    {
        
        memset(cmd,0,sizeof(cmd));
        sprintf(cmd,"AT+QFTPCFG=\"contextid\",1\r\n");
        if(AT_Cmd_Parse(cmd,"OK",NULL, 2000)== FALSE) 
        {
            dbg_printf(DBG_DTU,"err QIFGCNT");
            
        }
        osDelay(500);
        
        //关掉 FTP
        //close_ftp();  
        //set user
        memset(cmd,0,sizeof(cmd));
        sprintf(cmd,"AT+QFTPCFG=\"account\",\"%s\",\"%s\"\r\n",pInfo->ftpUsr,pInfo->ftpPass);
        if(AT_Cmd_Parse(cmd, "OK",NULL,5000)== FALSE)     
        {   
            dbg_printf(DBG_DTU,"err QFTPUSER");
            return 0;
        }
        osDelay(200);  
        //set filetype
        memset(cmd,0,sizeof(cmd));
        sprintf(cmd,"AT+QFTPCFG=\"filetype\",1\r\n");
        
        if(AT_Cmd_Parse(cmd, "OK",NULL,5000)== FALSE)     
        {   
            dbg_printf(DBG_DTU,"err filetype");
            return 0;
        }
        osDelay(500); 
        //set tranmode
        memset(cmd,0,sizeof(cmd));
        sprintf(cmd,"AT+QFTPCFG=\"transmode\",1\r\n");
        
        if(AT_Cmd_Parse(cmd, "OK",NULL,5000)== FALSE)     
        {   
            dbg_printf(DBG_DTU,"err transmode");
            return 0;
        }
        osDelay(500); 

        //set resptime
        memset(cmd,0,sizeof(cmd));
        sprintf(cmd,"AT+QFTPCFG=\"rsptimeout\",90\r\n");
        
        if(AT_Cmd_Parse(cmd, "OK",NULL,5000)== FALSE)     
        {   
            dbg_printf(DBG_DTU,"err rsptimeout");
            return 0;
        }
        osDelay(500); 
        
        //set addr and port
        if(conect_server(pInfo->ftpSer,pInfo->ftpPort) != 0)
        {
            close_ftp();
            dbg_printf(DBG_DTU,"err conect_server %d",connct);
            connct++;
            osDelay(3000);  
            if(connct > 3)
            {
                return 0;
            }
        }
        else
        {
            break;
        }
    }
    osDelay(200);
    //set path
    memset(cmd,0,sizeof(cmd));
    memset(path,0,sizeof(path));
    path_len = strlen(pInfo->ftpFile);
    path_len -= 11;//strlen("EVA_App.bin");
    memcpy(path,pInfo->ftpFile,path_len);
    sprintf(cmd,"AT+QFTPCWD=\"%s\"\r\n",path);
    
    if(AT_Cmd_Parse(cmd, "OK",NULL,5000)== FALSE)     
    {   
        close_ftp();
        dbg_printf(DBG_DTU,"err QFTPPATH");
        return 0;
    }

    //删掉文件
    memset(cmd,0,sizeof(cmd));
    sprintf(cmd,"AT+QFDEL=\"RAM:*\"\r\n"); 
    if(AT_Cmd_Parse(cmd, "OK",NULL,5000)== FALSE)     
    { 
        dbg_printf(DBG_DTU,"err QFDEL");
    } 
    
    //下载文件到RAM
    
    //AT+QFTPCFG="RAM:EVA_App.bin"
//    memset(cmd,0,sizeof(cmd));
//    sprintf(cmd,"AT+QFTPCFG=\"EVA_App.bin\",\"RAM:EVA_App.bin\"\r\n"); 
//    if(AT_Cmd_Parse(cmd, "OK",NULL,5000)== FALSE)     
//    {   
//        close_ftp();
//        dbg_printf(DBG_DTU,"err QFTP get1");
//        return 0;
//    } 
    //下载文件
    if(ftp_get_file_lte() != 0)
    {
        close_ftp();
        dbg_printf(DBG_DTU,"err ftp_get_file 1");
        return 0;
    }
    //
    get_len = ftp_get_file_len_lte();
    down_len = get_len;
    down_len = down_len;
    if(get_len < DEF_APP_SIZE)
    {
        remain = DEF_APP_SIZE - get_len;
    }
    remain = remain;
    if(get_len <= 0)
    {
        close_ftp();
        dbg_printf(DBG_DTU,"err get_len %d",get_len);
        return 0;
    }

    
    fd = get_ram_file_fd();
    offset = 0;
    
    //初始化地址
    g_addr = SPI_FLASHE_ADDR;
    g_fileLen = 0;
    g_fileChecksum = 0;
    set_file_offset(fd,0);
    
    //下载文件到本地
    if(fd <= 0)
    {
        close_ftp();
        dbg_printf(DBG_DTU,"err fd %d",fd);
        return 0;
    }
    else
    {
        do
        {
            //
            int read_len = 0;
            
            if(get_len > READ_LEN)
            {
                read_len = READ_LEN;
            }
            else
            {
                read_len = get_len;
            }
            
            ret = read_ram_file(fd,read_len);
            if(ret == 1)
            {
                get_len -= read_len;
                offset += read_len;
                ret = 0;
            }
            else if(ret == 0)
            {
                ret = 1;
                close_ftp();
                break;
            }
            else
            {
                set_file_offset(fd,offset);
            }                
            
        }while(get_len >0);   
    }

    //AT+QFTPCFG="RAM:EVA_App.bin"
    //关闭文件
    memset(cmd,0,sizeof(cmd));
    sprintf(cmd,"AT+QFCLOSE=%d\r\n",fd); 
    if(AT_Cmd_Parse(cmd, "OK",NULL,5000)== FALSE)     
    { 
        dbg_printf(DBG_DTU,"err close get1"); 
    } 
    
    //关闭FTP
    close_ftp();
   
    if(ret == 1)
    {
        dbg_printf(1,"FTP升级成功");
    }
    else
    {
        dbg_printf(1,"FTP升级失败");
    }
    return ret;
}


int API_Ftp_Updata(UpdateInfo* pInfo)
{
    int ret = 0;
    int time  = 0;
    
    FtpInit();
    
    while(1)
    {
        if(ftp_connect(pInfo->ftpSer,pInfo->ftpPort,pInfo->ftpUsr,
            pInfo->ftpPass) == 0)
        {
            time++;
            if( time > 3)
                return 0;
        }
        else
        {
            break;
        }
    }
    
    osDelay(100);
    
    FtpOptions(FTPLIB_CALLBACK, (long) NULL, conn);
    
    if(pInfo->UpdateMode == FTP_MODE)
    {
        ret = ftp_proc_file(NULL,(const char*)pInfo->ftpFile,conn,FTPLIB_FILE_READ,'I');
    }
    else
    {
        
        ret = ftp_updata_program(NULL,(const char*)pInfo->ftpFile,conn,FTPLIB_FILE_READ,'I');
        
    }
    
    FtpClose(conn); 
    
    return ret;
    
}


int Ftp_Updata(UpdateInfo* pInfo)
{
    int ret = 0;
    int time  = 0;
    
    FtpInit();
    
    while(1)
    {
        if(ftp_connect(pInfo->Update_url,pInfo->Update_port,pInfo->Update_Usr,
            pInfo->Update_Pass) == 0)
        {
            time++;
            if( time > 3)
                //return 0;
                return 2;
        }
        else
        {
            break;
        }
    }
    
    osDelay(100);
    
    FtpOptions(FTPLIB_CALLBACK, (long) NULL, conn);
    
    if(pInfo->UpdateMode == FTP_MODE)
    {
        ret = ftp_proc_file(NULL,(const char*)pInfo->Update_File,conn,FTPLIB_FILE_READ,'I');
    }
    else
    {
        
        ret = ftp_updata_program(NULL,(const char*)pInfo->Update_File,conn,FTPLIB_FILE_READ,'I');
        
    }
    
    FtpClose(conn); 
    
    return ret;
    
}

  
