

/******************************************************************************
* Includes
******************************************************************************/
#include "Utility.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "stdio.h"
#include "time.h"
#include "eth_comm.h"

/******************************************************************************
* Macros Definitions (constant/Macros)
******************************************************************************/
#define PARAM_ADDR           0x0000
#define PRAMRA_BACK_UP_ADDR  0x1000
#define DBG	 1	

#define ETH_DBG
#define DBG_COM   5
#define MAX_BUF_SIZE   256

/******************************************************************************
* Data Type Definitions
******************************************************************************/

/******************************************************************************
* Global Variable Definitions
******************************************************************************/
static char dbg_buf[MAX_BUF_SIZE];

static osSemaphoreId g_hd;

uint8_t Checkout_Sum(uint8_t *pBuf, uint16_t uLen)
{
    uint8_t sum = 0;

    int i = 0;

    for(i = 0; i < uLen;i++)
    {
        sum += pBuf[i];
    }
    return sum;
}

uint8_t BcdToHex8(uint8_t bcd)
{
    return ((bcd&0xf)+((bcd&0xf0)>>4)*10);
}

int16_t Bcd2ToHex16(uint8_t *bcd)
{
    int16_t tmpValue=0;
    tmpValue+=(((*(bcd+1))&0xf0)>>4)*1000;
    tmpValue+=((*(bcd+1))&0x0f)*100;
    tmpValue+=(((*bcd)&0xf0)>>4)*10;
    tmpValue+=((*bcd)&0x0f);
    return tmpValue;
	
}

I32 Bcd3ToHex32(uint8_t *bcd)
{
    I32 tmpValue=0;
    tmpValue+=(((*(bcd+2))&0x00)>>4)*100000;
    tmpValue+=((*(bcd+2))&0x0f)*10000;
    tmpValue+=(((*(bcd+1))&0xf0)>>4)*1000;
    tmpValue+=((*(bcd+1))&0x0f)*100;
    tmpValue+=((*(bcd)&0xf0)>>4)*10;
    tmpValue+=((*bcd)&0x0f);

    if(bcd[2] & 0x80)
    {
        tmpValue = -tmpValue;
    }
 
    return tmpValue;
	
}

I32 Bcd4ToHexInt32(uint8_t *bcd)
{
    I32 tmpValue=0;
    tmpValue+=(((*(bcd+3))&0x00)>>4)*10000000;
    tmpValue+=((*(bcd+3))&0x0f)*1000000;	
    tmpValue+=(((*(bcd+2))&0xf0)>>4)*100000;
    tmpValue+=((*(bcd+2))&0x0f)*10000;
    tmpValue+=(((*(bcd+1))&0xf0)>>4)*1000;
    tmpValue+=((*(bcd+1))&0x0f)*100;
    tmpValue+=(((*bcd)&0xf0)>>4)*10;
    tmpValue+=((*bcd)&0x0f);

    if(bcd[3] & 0x80)
    {
        tmpValue = -tmpValue;
    }
    return tmpValue;
}

U32 Bcd4ToHexUint32(uint8_t *bcd)
{
    U32 tmpValue=0;
    tmpValue+=(((*(bcd+3))&0x70)>>4)*10000000;
    tmpValue+=((*(bcd+3))&0x0f)*1000000;	
    tmpValue+=(((*(bcd+2))&0xf0)>>4)*100000;
    tmpValue+=((*(bcd+2))&0x0f)*10000;
    tmpValue+=(((*(bcd+1))&0xf0)>>4)*1000;
    tmpValue+=((*(bcd+1))&0x0f)*100;
    tmpValue+=(((*bcd)&0xf0)>>4)*10;
    tmpValue+=((*bcd)&0x0f);

    return tmpValue;
}


void HextoBCD(int Dec, uint8_t *Bcd, int length)
{
    int index;
    int temp;

    for(index=length-1; index>=0; index--)
    {
        temp = Dec%100;
        Bcd[index] = ((temp/10)<<4) + ((temp%10) & 0x0F);
        Dec /= 100;
    }
}
uint8_t HexToBcd(uint8_t hex)
{
    uint8_t bcd=0;
    bcd=((hex/10)<<4)+(hex%10);
    return bcd;
}


uint8_t ByteToBcd(uint8_t Byte)
{
	if(Byte >= 99)
		return 0xaa;
	else
	{
		return ((Byte/10 << 4) + Byte % 10);
	}
	
}



void dbg_printf(uint8 leve,uint8 *pFormat,...)
{
    
    
	va_list args;
	int i = 0;
    
    struct tm st_time;;
    struct tm *st_tmp;
    time_t  tim = time(NULL);
	if(leve == 0) 
	
		return;
	#if (DBG == 0)
		return;
	#endif

	if (osSemaphoreWait( g_hd, 100)!=osOK)
	{
		return;
	}
    
    //get_RTC_Calendar(&st_time);
    st_tmp = gmtime((const time_t *)&tim);
    st_time = *st_tmp;
	memset(dbg_buf,0,sizeof(dbg_buf));
	sprintf((char *)dbg_buf,"[%.2d:%.2d:%.2d]",st_time.tm_hour,st_time.tm_min,st_time.tm_sec);
    i = strlen((const char *)dbg_buf);
    
	va_start(args, pFormat);
	vsprintf((char *)dbg_buf+i,(char*)pFormat,args);
	va_end( args );
	i = strlen((const char *)dbg_buf);
	sprintf((char *)dbg_buf+i,"%s","\n");
    
#ifdef ETH_DBG

	eth_udp_dbg((uint8*)dbg_buf,strlen(dbg_buf));
#else
	Drv_Uart_Write(DBG_COM,(uint8*)dbg_buf,strlen(dbg_buf),100);
#endif

	osSemaphoreRelease(g_hd);
}

void print_hex_data(const uint8 *hex,int len)
{
    uint8 g_data[200] = {0};

	int i = 0;
	int index = 0;
    
	memset(g_data,0,sizeof(g_data));
	
	for(i = 0; i < len;i++)
	{
		sprintf((char*)g_data + index ,"%2.2X ",hex[i]);

		index = strlen((const char*)g_data);
        
		if(index >= sizeof(g_data) - 3) 
        {
            dbg_printf(1,"%s",g_data);
            memset(g_data,0,sizeof(g_data));
            index = 0;
        }
            //break;
	}
	dbg_printf(1,"%s",g_data);
}

void shift_hex_data(uint8 *DispBuf,uint8 *hex,int len)
{

	int i = 0;
	int index = 0;
    
	memset(DispBuf,0,len * 2);
	
	for(i = 0; i < len;i++)
	{
		sprintf((char*)DispBuf + index ,"%2.2X",hex[i]);

		index = strlen((const char*)DispBuf);

	}
    
}

void shift_time_data(uint8 *DispBuf,uint32 time)
{



    struct tm *stTime;
    
    if(DispBuf == NULL)
    {
        return;
    }
    
	memset(DispBuf,0,32);
    
	stTime  = gmtime((time_t*)&time);
    
    sprintf((char*)DispBuf,"%d-%2.2d-%2.2d-%2.2d:%2.2d:%2.2d",
            stTime->tm_year + 1900,
            stTime->tm_mon + 1,
            stTime->tm_mday,stTime->tm_hour,
            stTime->tm_min,stTime->tm_sec);
    
}



void dbg_init(void)
{

	osSemaphoreDef(SEM);
	
	g_hd = osSemaphoreCreate(osSemaphore(SEM) , 1 );

	#ifdef ETH_DBG
	eth_dbg_init(168);
	#endif
}

uint8_t DataXorCheck(uint8_t *buf,uint8_t len)
{
    uint8_t i=0;
    uint8_t ret=0;
    for(i=0;i < len;i++)
    {
        ret^=buf[i];
    }
    return ret;
}

uint8 Tick_Out(uint32 *tick,uint32 timeout)
{
    if(timeout == 0)
    {
        *tick = HAL_GetTick();

        return TRUE;
    }
    else
    {
        if(abs(HAL_GetTick() - *tick) >= timeout)
        {
            return TRUE;
        }
    }
    return FALSE;
}

void reversalByte(uint8 *dst,uint8 *src,int len)
{
	int i = 0;

	for(i = 0; i < len;i++)
	{
		dst[i] = src[len - i - 1];
	}
}

uint8 asc2hex(uint8 * Dst,uint8 * src,int len)
{
    int i = 0;
    int int_H = 0;
    int int_L = 0;
    int index = 0;
    for(i = 0;i < len;)
    {
        if((src[i] < 0x30 || src[i] > 'F' )|| (src[i] > '9' && src[i] < 'A'))
        {
            return 0;
        }
        else
        {
            if(src[i] <= '9')
            {
                int_H = src[i] - '0';
            }
            else if(src[i] >= 'A')
            {
                int_H = src[i] - 'A' + 10;
            }           
        }
        
        if((src[i+1] < '0' || src[i+1] > 'F') || (src[i+1] > '9' && src[i+1] < 'A'))
        {
            return 0;
        }
        else
        {
            if(src[i + 1] <= '9')
            {
                int_L = src[i + 1] - '0';
            }
            else if(src[i + 1] >= 'A')
            {
                int_L = src[i + 1] - 'A' + 10;
            }           
        }
        Dst[index ++] = (int_H << 4) + int_L;     
        i += 2;      
    }
    return index;
}


U8 Get_Chip_ID(uint8 *id)
{
    uint32_t ChipUniqueID[3] = {0};

    ChipUniqueID[2] = *(__IO uint32_t*)(0X1FFFF7E8);  // µÍ×Ö½Ú

    ChipUniqueID[1] = *(__IO uint32_t *)(0X1FFFF7EC); // 

    ChipUniqueID[0] = *(__IO uint32_t *)(0X1FFFF7F0);
    
    memcpy(id,(uint8*)ChipUniqueID,12);
    
    return 0;
}

char is_Leap_Year(int year)
{
	if((year % 4 == 0 && year %100 != 0)|| year %400 == 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}

}

int check_date(int year,int mon ,int day)
{
	if(year < 2000 || year >= 10000)
	{
		return 0;
	}
	if(day > 31 || day <= 0 || mon > 12 || mon <= 0 )
	{
		return 0;
	}
	if(mon == 4 || mon == 6 || mon == 9|| mon == 11)
	{
		if(day > 30)
		{
			 return 0;
		}
	}
    else
    {
        if(day > 31)
        {
            return 0;
        }
    }
	if(is_Leap_Year(year) == 1 )
	{

		if(day > 29 && (mon == 2))
		{
			return 0;
		}
	
	}
    else
	{

		if(day > 28 && (mon == 2))
		{
			return 0;
		}
	
	}
	return 1;
}
