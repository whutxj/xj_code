/******************************************************************************
* Include Files
******************************************************************************/

#include "Tsk_API_LCD.h"
#include "Sys_Config.h"
#include <string.h>
#include <stdio.h>
#include "Tsk_Charge_Ctrl.h"
#include "MsgQueue.h"
#include "Tsk_API_Alarm.h"
#include "bsp_Led_Function.h"
#include <time.h>
/******************************************************************************
* Macros Definitions (constant/Macros)
******************************************************************************/

#define LCD_PORT COM1
#define LCD_STX  0xAA

Dwin_data g_Dwin_data;

uint8_t LCD_Put_Data(uint8_t Cmd, uint8_t *pData, uint16_t DataLen)
{
    uint8_t SendBuf[120];
    uint16_t index = 0;
    uint16_t i;

	#if 0
    while( (Drv_Uart_Read(LCD_PORT, SendBuf,1,1)) != 0)
    {
        ;
    }
#endif

		SendBuf[index++] = 0x00;
    SendBuf[index++] = LCD_STX;
    SendBuf[index++] = Cmd;

    if ((NULL != pData) && (DataLen != 0))
    {
        for(i=0; i < DataLen; i++)
        {
          SendBuf[index++] = *pData++;
        }
    }

    /*end of the frame*/
    SendBuf[index++] = 0xCC;
    SendBuf[index++] = 0x33;
    SendBuf[index++] = 0xC3;
    SendBuf[index++] = 0x3C;

    Drv_Uart_Write(LCD_PORT, (const uint8 *)SendBuf, index, 0);
    return 0;
}

int LCD_Parse(uint8 *pData, int timeout)
{
    uint16 i, rcvLen = 0,tmpLen = 0;
    uint8 rcvBuf[64];
    uint8 buf[64];
    //uint8 norData;
    uint32 currentTime;
    uint32 startTime = xTaskGetTickCount();

    memset(buf,0,sizeof(buf));

    do
    {
        if((tmpLen = Drv_Uart_Read(LCD_PORT, buf, sizeof(buf),100)) > 0)
        {
            dbg_printf(0, "LCD rcv %d", tmpLen);

            for (i = 0; i < tmpLen; i++)
            {
                if (rcvLen == 0)
                {
                    if (buf[i] == 0xAA)
                    {
                        rcvBuf[rcvLen++] = buf[i];
                        continue;
                    }
                }
                if (rcvLen == 1)
                {
                    rcvBuf[rcvLen++] = buf[i];
                    continue;
                }
                if (rcvLen >= 2)
                {
                  if (rcvLen < sizeof(buf))
                  {
                    memcpy(pData, &rcvBuf[0], rcvLen);
                    return TRUE;
                  }
                }
            }

        }
        osDelay(10);
        currentTime= xTaskGetTickCount();
    }while((currentTime - startTime)<=timeout);

    return FALSE;
}

void LCD_backlight_control_frame(uint8_t light_value)
{
  uint8_t DataBuf[1];

  DataBuf[0] = light_value;
  LCD_Put_Data(0x30, DataBuf, 1);
}

void LCD_tx_handshake_frame(void)
{
  uint8_t DataBuf[2];

  DataBuf[0] = 0;
  LCD_Put_Data(0x00, DataBuf, 0);
}

void LCD_QR_code_display_frame(uint8_t *QR_code_data, uint8_t QR_code_length)
{
  uint8_t i;
  uint8_t DataBuf[100];

  DataBuf[0] = 0x00;
  DataBuf[1] = 0x50;
  DataBuf[2] = 0x00;
  DataBuf[3] = 0x48;

  DataBuf[4] = 0x03;

  for (i = 0; i < QR_code_length; i++)
  {
    DataBuf[5 + i] = QR_code_data[i];
  }
  LCD_Put_Data(0x21, DataBuf, QR_code_length + 5);
}

void LCD_switch_images_frame(uint8_t image_ID)
{
  uint8_t DataBuf[2];
  DataBuf[0] = 0x00;
  DataBuf[1] = image_ID;
  LCD_Put_Data(0x22, DataBuf, 2);
}

void LCD_ICON_display_frame(uint8_t icon_ID)
{
  uint8_t DataBuf[9];

  DataBuf[0] = 0x00;
  DataBuf[1] = 0x80;
  DataBuf[2] = 0x00;
  DataBuf[3] = 0x40;
  DataBuf[4] = 0x86;
  DataBuf[5] = icon_ID;
  DataBuf[6] = 0x00;
  DataBuf[7] = 0x00;
  DataBuf[8] = 0x00;

  LCD_Put_Data(0x23, DataBuf, 9);
}

void LCD_variables_display_frame(u8_t front_size, u8_t integer, u8_t decimal, u32_t x_coordinate, u32_t y_coordinate, u32_t variable_data)
{
  uint8_t DataBuf[15];

  if (front_size > 0x09)
  {
    front_size = 0x09;
  }
  DataBuf[0] = 0x0F & front_size;
  DataBuf[1]  = 0x00;
  DataBuf[2]  = 0x00;


//ºÚµ×°××Ö	
//	DataBuf[0]  = 0x80 | front_size;	
//	DataBuf[1]  = 0xFF;
//  DataBuf[2]  = 0xFF;
  DataBuf[3]  = 0x00;
  DataBuf[4]  = 0x00;

  DataBuf[5]  = integer;
  DataBuf[6]  = decimal;

  DataBuf[7]  = x_coordinate >>8;
  DataBuf[8]  = x_coordinate;
  DataBuf[9]  = y_coordinate >>8;
  DataBuf[10] = y_coordinate;

  DataBuf[11] = variable_data >>24;
  DataBuf[12] = variable_data >>16;
  DataBuf[13] = variable_data >>8;
  DataBuf[14] = variable_data;

  LCD_Put_Data(0x14, DataBuf, 15);
}

void LCD_asc_display_frame(u8_t front_size,  u32_t x_coordinate, u32_t y_coordinate,u8_t *asc, u8_t  len)
{
  uint8_t DataBuf[120];
  int send_len  = 0;
  if (front_size > 0x09)
  {
    front_size = 0x09;
  }
  DataBuf[send_len++] = 0x0F & front_size;
  DataBuf[send_len++]  = 0x00;
  DataBuf[send_len++]  = 0x00;
	
//	ºÚµ×°××Ö
//  DataBuf[send_len++] = 0x40 | front_size;	
//	DataBuf[send_len++]  = 0xFF;
//  DataBuf[send_len++]  = 0xFF;

  DataBuf[send_len++]  = 0x00;
  DataBuf[send_len++]  = 0x00;

  DataBuf[send_len++]  = x_coordinate >>8;
  DataBuf[send_len++]  = x_coordinate;
  DataBuf[send_len++]  = y_coordinate >>8;
  DataBuf[send_len++] =  y_coordinate;

  memcpy(DataBuf + send_len,asc,len);
  send_len += len;
  LCD_Put_Data(0x11, DataBuf, send_len);
}

void LCD_0x25_frame(void)
{
  uint8_t DataBuf[120];
  int send_len  = 0;

  DataBuf[send_len++] = 0x01;

  DataBuf[send_len++]  = 0x01;

  LCD_Put_Data(0x25, DataBuf, send_len);
}

void LCD_0x26_frame(int x1,int y1,int x2,int y2,int x,int y)
{
  uint8_t DataBuf[120];
  int send_len  = 0;

  DataBuf[send_len++]  = x1 >>8;
  DataBuf[send_len++]  = x1;
  DataBuf[send_len++]  = y1 >>8;
  DataBuf[send_len++] =  y1;

  DataBuf[send_len++]  = x2 >>8;
  DataBuf[send_len++]  = x2;
  DataBuf[send_len++]  = y2 >>8;
  DataBuf[send_len++] =  y2;
	
  DataBuf[send_len++]  = x >>8;
  DataBuf[send_len++]  = x;
  DataBuf[send_len++]  = y >>8;
  DataBuf[send_len++] =  y;


  LCD_Put_Data(0x26, DataBuf, send_len);
}


