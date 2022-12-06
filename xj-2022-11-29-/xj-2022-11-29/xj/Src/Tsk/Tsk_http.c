
#include "time.h"
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
#include "lwip/netdb.h"//wangyu add
#include "string.h"
#include "Update.h"
#include "bsp_driver_uart.h"
#include "Sys_Config.h"


/*外部接口*/
#define  HTTP_SIZE 1024//1024+256
//char data_buf[BUFSIZE];
static char data_buf[HTTP_SIZE];
#define net_uart_port     COM4

#define SIZE_TYPE 64
#define SIZE_FILE 64

#define HEAD_INFO       "http://"

#define HEAD_INFO_S     "https://"
#define BUFSIZE 300

static char g_start_http_flag = 0;//下载标志
static uint16_t updata_time_out = 0;
static uint8_t  connect_way = LAN;//LAN 以太网   DTU 4g模块

extern UpdateInfo JWT_UpdateInfo;
uint8 updata_flag = 0;


struct resp_header 
{
    int status_code;
    char content_type[SIZE_TYPE];
    long content_length;
    char file_name[SIZE_FILE];
};

uint32_t Get_Cur_Second(void) 
{
	return time(NULL);
}


void parse_url(const char *url, char *domain, int *port, char *file_name)
{
   
    int j = 0;
    int start = 0;
    int flag = 0;
    *port = 80;
    char *patterns[] = {HEAD_INFO, HEAD_INFO_S, NULL};

    for (int i = 0; patterns[i]; i++)
        if (strncmp(url, patterns[i], strlen(patterns[i])) == 0)
            start = strlen(patterns[i]);


    for (int i = start; url[i] != '/' && url[i] != '\0'; i++, j++)
        domain[j] = url[i];
    domain[j] = '\0';


    char *pos = strstr(domain, ":");
    if (pos)
        sscanf(pos, ":%d", port);



    for (int i = 0; i < (int)strlen(domain); i++)
    {
        if (domain[i] == ':')
        {
            domain[i] = '\0';
            break;
        }
    }

    j = 0;
    for (int i = start; url[i] != '\0'; i++)
    {
//        if (url[i] == '/')
//        {
//            if (i !=  strlen(url) - 1)
//                j = 0;
//            continue;
//        }
//        else
//            file_name[j++] = url[i];
          if (url[i] == '/' && flag == 0)
          {
              j = 0;
              flag = 1;
              continue;
              
          }
          if(flag == 1)
          {
              file_name[j++] = url[i];
          }
    }
    file_name[j] = '\0';
}

struct resp_header get_resp_header(const char *response)
{

    struct resp_header resp;

    char *pos = strstr(response, "HTTP/");
    if (pos)
        sscanf(pos, "%*s %d", &resp.status_code);

    pos = strstr(response, "Content-Type:");
    if (pos)
        sscanf(pos, "%*s %s", resp.content_type);

    pos = strstr(response, "Content-Length:");
    if (pos)
        sscanf(pos, "%*s %ld", &resp.content_length);

    return resp;
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
    
    len = Drv_Uart_Read(net_uart_port,(unsigned char *) rcvbuf,remain,200);//

    Drv_Uart_Write(net_uart_port,(const unsigned char *) send,strlen(send),100);
    osDelay(200);
    
    memset(rcvbuf,0,sizeof(rcvbuf));
    
    dbg_printf(1,"send %s",send);
    
    do
    {
        len = Drv_Uart_Read(net_uart_port,(unsigned char *)rcvbuf + rcvLen,remain,500);
        
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

uint8 connect_4G_YiYuan(char *ip,uint32 port )
{
  char cmd[64] = {0};
  char cmd_2[64] = {0};
  int ReConnectCount = 0;
  int start = 0;
    
  while(ReConnectCount < 5)
  {
    if(AT_Cmd_Parse("+++", "OK",NULL, 1000)== FALSE)
    {
      //ReConnectCount++;
      osDelay(1000);
      //continue;
    }      
    if(AT_Cmd_Parse("ATE0\r", "OK",NULL, 500)== FALSE)
    {
      osDelay(1000);
      ReConnectCount++;
      dbg_printf(1,"ATE0 not ok!");
      continue;
    }
    if(AT_Cmd_Parse("AT+QICSGP=1,1,\"\",\"\",\"\",1\r", "OK",NULL, 500)== FALSE)
    {
      osDelay(1000);
      ReConnectCount++;
      dbg_printf(1,"AT+QICSGP not ok!");
      continue;
    } 
    if(AT_Cmd_Parse("AT+QIDEACT=1\r", "OK",NULL, 500)== FALSE)
    {
      osDelay(1000);
      ReConnectCount++;
      dbg_printf(1,"AT+QIDEACT not ok!");
      continue;
    }
    if(AT_Cmd_Parse("AT+QIACT=1\r", "OK",NULL, 1000)== FALSE)
    {
      osDelay(1000);
    }
    osDelay(200);
    memset(cmd,0,sizeof(cmd));
    memset(cmd_2,0,sizeof(cmd_2));

    sprintf(cmd_2,"%s","\"");
    sprintf(cmd_2,"%s%s",cmd_2,ip);
    sprintf(cmd_2,"%s%s",cmd_2,"\"");
    sprintf(cmd,"AT+QIOPEN=1,1,%s,%s,%d,0,2\r","\"TCP\"",cmd_2,port);
    dbg_printf(0,"cmd =%s",cmd);
    
    if(AT_Cmd_Parse(cmd, "CONNECT",NULL, 1000)== FALSE)
    {
      ReConnectCount++;
      osDelay(1000);
      dbg_printf(1,"TCP CONNECT not ok!");
      continue;
    }

    #if 0
    start = time(NULL);
    while(1)
    {
        memset(cmd,0,sizeof(cmd));
        int len = Drv_Uart_Read(COM4,(unsigned char *) cmd,sizeof(cmd),200);//
        if(len > 0)
        {
          dbg_printf(1,"YiYuan reboot rcv %s",cmd);
          
          if(strstr(cmd,"ready"))
          {
              break;
          }
         
        }
        else
          osDelay(10);
        if(time(NULL) - start > 60)
          break;
    }
    #endif
    //osDelay(25000); 
    dbg_printf(1,"SET 4g OK");
    return TRUE;

      
  }

//  BSP_GPIO_OutPut(POWER_4G,OUT_HIGHE);
//    osDelay(200);  
//    BSP_GPIO_OutPut(POWER_4G,OUT_LOW); 
//    dbg_printf(1,"RSET 4g");
//    osDelay(1000);  

  return FALSE;    
}



uint8 connect_4G(char *ip,uint32 port )
{

    char cmd[64] = {0};
  
 
    int ReConnectCount = 0;
    int start = 0;
    
	while(ReConnectCount < 5)
	{
        //dbg_printf(1,"+++");

        if(AT_Cmd_Parse("+++\r\n", "a",NULL, 500)== FALSE)
        {	
	        ReConnectCount++;
	        osDelay(1000);	
	        continue;
        }
        
        if(AT_Cmd_Parse("a\r\n", "+ok",NULL, 500)== FALSE)
        {	
            ReConnectCount++;
            osDelay(1000);	
            continue;
        }
		
        //
        osDelay(500);
        sprintf(cmd,"AT+SOCKA=TCP,%s,%d\r\n",ip,port);
        dbg_printf(1,"set dtu ip %s",ip);
        if(AT_Cmd_Parse(cmd, "OK",NULL,1000)== FALSE)  	  
        {	
            ReConnectCount++;
            continue;
        }

        osDelay(500);
        memset(cmd,0,sizeof(cmd));

        dbg_printf(1,"REBOOT 4G");
        if(AT_Cmd_Parse("AT+Z\r\n","OK",NULL, 1000)== FALSE)  
        {	
            ReConnectCount++;
            continue;
        }
        // 
        start = time(NULL);
        while(1)
        {
            memset(cmd,0,sizeof(cmd));
            int len = Drv_Uart_Read(COM4,(unsigned char *) cmd,sizeof(cmd),200);//
            if(len > 0)
            {
              dbg_printf(1,"reboot rcv %s",cmd);
              
              if(strstr(cmd,"ready"))
              {
                  break;
              }
             
            }
            else
              osDelay(10);
            if(time(NULL) - start > 60)
              break;
        }
        //osDelay(25000); 
        dbg_printf(1,"SET 4g OK");
        return TRUE;

			
	}

//	BSP_GPIO_OutPut(POWER_4G,OUT_HIGHE);
//    osDelay(200);  
//    BSP_GPIO_OutPut(POWER_4G,OUT_LOW); 
//    dbg_printf(1,"RSET 4g");
//    osDelay(1000);  

	return FALSE;    
}

#if 0
int install_file_4G( char *path , uint16_t time) 
{
	
  int cfd;
  int cnt = 0;
  struct sockaddr_in cadd;
  struct hostent *pURL = NULL;
  struct resp_header resp = {0};//
  char host[36] = {0};
  int port = 0;
  int len = 0;
  int length = 0;
  char read_buf[1] = {0};
  uint32_t sec =0;
  updata_time_out = time;
	

  parse_url(path, host, &port, resp.file_name);

	//
 
	memset(data_buf, 0, sizeof(data_buf));
	strcat(data_buf, "GET /");
	strcat(data_buf, resp.file_name);
	strcat(data_buf, " HTTP/1.1\r\n");
	strcat(data_buf, "HOST: ");
	strcat(data_buf, host);
	strcat(data_buf, "\r\n");
	strcat(data_buf, "Cache-Control: no-cache\r\n\r\n");
    //dbg_printf(1,"CMD-> %s",resp.file_name);
	//dbg_printf(1,"CMD-> %s",data_buf);
	int cc;
    
  dbg_printf(1,"connect host %s:%d",host,port);
	if (0  == connect_4G(host,port)){
		return -1;
	}

	int cs;
restart:
  length = 0;
  for(cs = 0; cs < 5;cs++)
  {
      Drv_Uart_Write(net_uart_port,(const unsigned char *) data_buf,strlen(data_buf),100);
      cc = Drv_Uart_Read(net_uart_port, read_buf, 1,10000);
      dbg_printf(1,"send http post -> %s len =%d ",data_buf,strlen(data_buf));
      //if(cc == 1)
      if(cc > 0)
      {
         memset(data_buf,0,sizeof(data_buf));
         data_buf[length++] = read_buf[0];
         dbg_printf(1,"http rcv -> 0X%X break",data_buf[0]);
         break;
      }
  }
  
  sec = Get_Cur_Second();
	
  while (1)
  {
    len = Drv_Uart_Read(net_uart_port, read_buf, 1,0);
    //dbg_printf(1,"len =%d ",len);
    
  	if(len > 0)
  	{
  		data_buf[length++] = read_buf[0];
  	}
    
    if(length > sizeof(data_buf))
    {
        dbg_printf(1,"find head err");
       
        return -1;
    }
    
    int flag = 0;
    for (int i = strlen(data_buf) - 1; data_buf[i] == '\n' || data_buf[i] == '\r'; i--, flag++);
    if (flag == 4)
    {
        dbg_printf(1,"head lenth %d %s",length,data_buf);
        break;
    }
    if(Get_Cur_Second() - sec > 10)
    {
        dbg_printf(1,"time out %d,%s",length,data_buf);
        print_hex_data(data_buf,length);
        if(strstr(data_buf,"ready"))
        {
          cnt++;
          if(cnt < 3)
            goto restart;
        }
        return -1;
    }
  }
    
  memset(resp.file_name,0,sizeof(resp.file_name));
  resp = get_resp_header(data_buf);
	
	strcpy(resp.file_name, resp.file_name);
    
	dbg_printf(1,"res file code %d len %d",resp.status_code, resp.content_length);

  return Write_Updata_File_CallBack(cfd,data_buf,sizeof(data_buf),resp.content_length);
}
#endif
int Write_Updata_File_CallBack_4G(int fd,uint8_t *buf,int max_len,int file_len)
{
	uint32_t adr = SPI_FLASHE_ADDR;
	uint8_t  ret = 0;
	uint32_t FileLen = 0;
	uint8_t CheckSum = 0;
	//uint8_t  tmpBuffer[FTPLIB_BUFSIZ] = {0};
	uint32_t err_cnt = 0;
	int len = 0;
	uint32_t sec = Get_Cur_Second();
	uint32_t FileSize = APP_SIZE;
	sec = Get_Cur_Second();
  int cnt = 0;

  BSP_SPIFLASH_BufferWrite(buf,adr,max_len);
  adr += max_len;
   
  FileLen += max_len;

  
      while(1)
      {
          //osDelay(50);
          //len = read(fd, buf,512);
          len =Drv_Uart_Read(net_uart_port, buf, HTTP_SIZE,5);
          //read(fd, tmpBuffer, FTPLIB_BUFSIZ,0);
          if(len > 0)
          {
            cnt ++;
		        BSP_SPIFLASH_BufferWrite(buf,adr,len);
            adr += len;
             
            FileLen += len;
            
            //dbg_printf(1,"addr 0x%x,pack %d len %d buf=[0x%x:0x%x]",adr-len,cnt,len,buf[0],buf[len-1]);
 
			      if (((FileLen * 100 / FileSize) % 10) == 0)
            {
                dbg_printf(1,"ftp complete %d ",(FileLen *100 / FileSize));
                //dbg_printf(1,"addr 0x%x,pack %d len %d buf=[0x%x:0x%x]",adr-len,cnt,len,buf[0],buf[len-1]);
 
           } 
           
            
            if(FileLen >= FileSize)
            {
              break;
            }
              
          }		
          if(Get_Cur_Second() - sec > updata_time_out)
          {
              dbg_printf(1,"rcv time out %d   adr=%d ",updata_time_out,adr);
              close(fd);
              ret =  0;
              break;
          }

        }
		if(check_file())
    {
        dbg_printf(1,"check_file ok ");
        ret = 1;
    }
    else
    {
        dbg_printf(1,"check_file err ");
        ret = 0;
    }
	return ret;
  }

int install_file_4G( char *path , uint16_t time) 
{
	
  int cfd;
  int cnt = 0;
  struct sockaddr_in cadd;
  struct hostent *pURL = NULL;
  struct resp_header resp = {0};//
  char host[36] = {0};
  int port = 0;
  int len = 0;
  int length = 0;
  char read_buf[1] = {0};
  uint32_t sec =0;
  updata_time_out = time;


  Erase_spi_flash_app_data();

  parse_url(path, host, &port, resp.file_name);

	//
 
	memset(data_buf, 0, sizeof(data_buf));
	strcat(data_buf, "GET /");
	strcat(data_buf, resp.file_name);
	strcat(data_buf, " HTTP/1.1\r\n");
	strcat(data_buf, "HOST: ");
	strcat(data_buf, host);
	strcat(data_buf, "\r\n");
	strcat(data_buf, "Cache-Control: no-cache\r\n\r\n");
    //dbg_printf(1,"CMD-> %s",resp.file_name);
	//dbg_printf(1,"CMD-> %s",data_buf);
	int cc;
    
  dbg_printf(1,"connect host %s:%d",host,port);
	if (1 == connect_4G(host,port))
  {    
	}
  else if(1 == connect_4G_YiYuan(host,port))
  {
  }
  else
  {
    return -1;
  }

	int cs;
restart:
  length = 0;
  for(cs = 0; cs < 5;cs++)
  {
      dbg_printf(1,"send http post -> %s len =%d ",data_buf,strlen(data_buf));
      Drv_Uart_Write(net_uart_port,(const unsigned char *) data_buf,strlen(data_buf),100);
      cc = Drv_Uart_Read(net_uart_port, data_buf, HTTP_SIZE,10000);   
      //if(cc == 1)
      if(cc > 0)
      {
        char *str1, *str2, *str3;
        int file_length, size, packlen;
        
        dbg_printf(1,"rcv %d :%s\r\n",cc,data_buf);
        str1 = strstr((char *)data_buf, "HTTP/1.1 200");
        str2 = strstr((char *)data_buf, "Content-Length:");
        str3 = strstr((char *)data_buf, "\r\n\r\n");
        if (str1 == NULL || str2 == NULL || str3 == NULL)
        {
            if(cnt++<3)
              continue;
            else
              return -1;
        }
        while (str2) {
          if (*str2 >= '0' &&  *str2 <= '9')
            break;
          str2++;
        }
        file_length = atoi(str2);
        if (file_length < 1024 * 50 || file_length > 256 * 1024)
        {
          dbg_printf(1,"Firmware Size Wrong, Update Failed...\r\n");
          return -1;
        }
        else
          dbg_printf(1,"Firmware Size %d,\r\n",file_length);
        str3 += 4;

        packlen = cc - (str3 - data_buf);
         
        if (packlen <= 0)
          return -1;
        // write_file(str3,packlen); 
        memmove(data_buf,str3,packlen);
        dbg_printf(1,"data[] 0x%x 0x%x len %d",data_buf[0],data_buf[packlen - 1],packlen);
        return Write_Updata_File_CallBack_4G(0,data_buf,packlen,file_length);
      }
  }

    
}

int Write_Updata_File_CallBack_LAN(int fd,uint8_t *buf,int max_len,int file_len)
{
	uint32_t adr = SPI_FLASHE_ADDR;
	uint8_t  ret = 0;
	uint32_t FileLen = 0;
	uint8_t CheckSum = 0;
	uint8_t  tmpBuffer[256] = {0};
	uint32_t err_cnt = 0;
	int len = 0;
	uint32_t sec = Get_Cur_Second();
	uint32_t FileSize = APP_SIZE;
	sec = Get_Cur_Second();
  int cnt = 0;
      while(1)
      {
          len = read(fd, buf,256);
          if(len > 0)
          {
            cnt ++;
		        BSP_SPIFLASH_BufferWrite(buf,adr,len);
            adr += len;
             
            FileLen += len;
            
            dbg_printf(1,"addr  0x%x,pack %d len %d buf=[0x%x:0x%x]",adr-len,cnt,len,buf[0],buf[len-1]);
 
			      if (((FileLen * 100 / FileSize) % 10) == 0)
            {
                dbg_printf(1,"ftp complete %d ",(FileLen *100 / FileSize));
            } 
            
            if(FileLen >= FileSize)
            {
              break;
            }

          }		
          if(Get_Cur_Second() - sec > updata_time_out)
          {
              dbg_printf(1,"rcv time out %d   adr=%d ",updata_time_out,adr);
              close(fd);
              ret =  0;
              break;
          }

        }


		close(fd);

		if(check_file())
    {
        dbg_printf(1,"check_file ok ");
        ret = 1;
    }
    else
    {
        dbg_printf(1,"check_file err ");
        ret = 0;
    }
	return ret;
  }


int install_file_LAN( char *path , uint16_t time) 
{
	
  int cfd;
  struct sockaddr_in cadd;
  struct hostent *pURL = NULL;
  struct resp_header resp = {0};//
  char host[36] = {0};
  int port = 0;
  int len = 0;
  int length = 0;
  char read_buf[1] = {0};
  uint32_t sec =0;
	updata_time_out = time;
	Erase_spi_flash_app_data();
  
	if (-1 == (cfd = socket(AF_INET, SOCK_STREAM, 0))) {
		dbg_printf(1,"%16.16s %4d socket error\r\n", __func__, __LINE__);
		return -1;
	}
    parse_url(path, host, &port, resp.file_name);
	pURL = (struct hostent *)lwip_gethostbyname(host);
	memset(&cadd, 0,sizeof(struct sockaddr_in));
	cadd.sin_family = AF_INET;
	//wangyu mask have question begin 2022-11-09
	//cadd.sin_addr.s_addr = (u32_t)(*((unsigned long*) ((struct hostent *)pURL->h_addr_list[0])));
	//wangyu mask have question end  2022-11-09
	cadd.sin_port = htons(port);
	//
	memset(data_buf, 0, BUFSIZE);
	strcat(data_buf, "GET /");
	strcat(data_buf, resp.file_name);
	strcat(data_buf, " HTTP/1.1\r\n");
	strcat(data_buf, "HOST: ");
	strcat(data_buf, host);
	strcat(data_buf, "\r\n");
	strcat(data_buf, "Cache-Control: no-cache\r\n\r\n");
    //dbg_printf(1,"CMD-> %s",resp.file_name);
	//dbg_printf(1,"CMD-> %s",data_buf);
	int cc;
	if (-1 == (cc = connect(cfd, (struct sockaddr*) &cadd,
		(socklen_t) sizeof(cadd)))) {
        close(cfd);
        dbg_printf(1,"connect failed \r\n");
		return -1;
	}

	int cs;
	if (-1 == (cs = send(cfd, data_buf, strlen(data_buf), 0))) {
		dbg_printf(1,"%16.16s %4d send err \r\n", __func__, __LINE__);
        close(cfd);
		return -1;
	}
    sec = Get_Cur_Second();
	memset(data_buf,0,sizeof(data_buf));
    while (1)
    {
        len = recv(cfd, read_buf, 1,0);
    	if(len > 0)
    	{
    		data_buf[length++] = read_buf[0];
    	}
        
        if(length > sizeof(data_buf))
        {
            dbg_printf(1,"find head err");
            close(cfd);
            return -1;
        }
        int flag = 0;
        for (int i = strlen(data_buf) - 1; data_buf[i] == '\n' || data_buf[i] == '\r'; i--, flag++);
        if (flag == 4)
        {
            //dbg_printf(1,"head lenth %d",length);
            break;
        }
        if(Get_Cur_Second() - sec > 10)
        {
            dbg_printf(1,"time out %s",data_buf);
            return -1;
        }
    }
    
    memset(resp.file_name,0,sizeof(resp.file_name));
    resp = get_resp_header(data_buf);
	
	strcpy(resp.file_name, resp.file_name);
    
	dbg_printf(1,"res file code %d len %d",resp.status_code, resp.content_length);

    return Write_Updata_File_CallBack_LAN(cfd,data_buf,sizeof(data_buf),resp.content_length);
}

void start_http_download(uint8_t type)
{
	g_start_http_flag = 1;

  connect_way = type;
  if(connect_way != LAN && connect_way != DTU)
      connect_way = LAN;
  
  dbg_printf(1,"connet_way = %d ",connect_way);

}


static void http_download_tsk(const void *p)
{
//	memcpy(FTP_Info.url,"http://sy-airport.oss-cn-beijing.aliyuncs.com/platform.bin",70);
//	dbg_printf(1," usl=%s ",FTP_Info.url);
//	FTP_Info.time_out = 60;
//	g_start_http_flag = 1;
//	erase_flash_app_data();

	while(1)
    {
    	if(g_start_http_flag)
    	{	

  	    //int flag = install_file(FTP_Info.Update_url,120);
  	    int flag = install_file_4G("http://120.78.213.97/image/judian/AC_7k135.bin",120);
  	    //int flag = install_file_4G(FTP_Info.Update_url,120);
  			if(flag == 0 || flag == -1)
  			{
  				updata_flag = 2;
  				dbg_printf(1,"update fail \r");

  			}
  			else
  			{
  				updata_flag = 1;

  				save_updata_record(1);
  				dbg_printf(1,"update ok\r");
  				JWT_UpdateInfo.UpdateMode = FTP_IAP_MODE;
  				BSP_SPIFLASH_SectorErase(UPDATA_INFO_ADDR);
  				BSP_SPIFLASH_BufferWrite((uint8_t*)&JWT_UpdateInfo,UPDATA_INFO_ADDR,sizeof(JWT_UpdateInfo));
//  				dbg_printf(1,"reboot...\n");
//  				osDelay(4000);
//  				NVIC_SystemReset(); 
  			}

			  g_start_http_flag = 0;
        break;
		  } 
		else
	    {
	    	osDelay(1000);
	    }
		
    }
}


void http_download_process(void)
{
  int flag = 0;
  if(g_start_http_flag)
  { 
    //int flag = install_file_4G("http://120.78.213.97/image/judian/AC_7k135.bin",120);
    if(connect_way == LAN)
    {
      flag = install_file_LAN(JWT_UpdateInfo.Update_url,120);
      //flag = install_file_LAN("http://192.168.1.14/AC_7k135.bin",120);
    }
    else
    {
      flag = install_file_4G(JWT_UpdateInfo.Update_url,120);
    }

    if(flag == 0 || flag == -1)
    {
      updata_flag = 2;//下载失败
      dbg_printf(1,"update fail \r");

    }
    else
    {
      updata_flag = 1;//下载成功

      save_updata_record(1);
      dbg_printf(1,"update ok\r");
      JWT_UpdateInfo.UpdateMode = FTP_IAP_MODE;
      BSP_SPIFLASH_SectorErase(UPDATA_INFO_ADDR);
      BSP_SPIFLASH_BufferWrite((uint8_t*)&JWT_UpdateInfo,UPDATA_INFO_ADDR,sizeof(JWT_UpdateInfo));
//          dbg_printf(1,"reboot...\n");
//          osDelay(4000);
//          
//          NVIC_SystemReset(); 
    }

    g_start_http_flag = 0;
  } 
  else
  {
    osDelay(1000);
  }

}
void Setup_http_download_tsk(osPriority ThreadPriority,uint8_t StackNum)
{
	//osThreadDef(App_http, http_download_tsk, ThreadPriority, 0, StackNum*configMINIMAL_STACK_SIZE);
	//osThreadCreate (osThread(App_http), NULL);
}



