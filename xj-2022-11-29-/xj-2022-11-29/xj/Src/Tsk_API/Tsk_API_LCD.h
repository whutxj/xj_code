/******************************************************************************
*
*  File name:     Tsk_API_LCD.h
*  Author:
*  Version:       V1.00
*  Created on:    2018-12-13
*  Description:
*  History:
*      Date        Author        Modification
*    1.2018-12-13
*    2. ...
******************************************************************************/
#ifndef TSK_API_LCD_H
#define TSK_API_LCD_H

#ifdef __cplusplus
 extern "C" {
#endif
/******************************************************************************
* Includes
******************************************************************************/
#include "main.h"
#include "lwip/arch.h"
#include "cmsis_os.h"
#include "Global.h"
#include "Utility.h"
#include <time.h>
#include "Tsk_Api_Card.h"
#include "Tsk_API_Alarm.h"
#include "cc.h"
#include "Drv_Usart.h"
#include "LibQueue.h"
#include "bsp_Driver_Uart.h"
#include <string.h>
	 
/******************************************************************************
* Macros Definitions (constant/Macros)
******************************************************************************/
enum
{
  welcome_page  = 0,
  homepage      = 1,
  turn_on_page  = 2,
  starting_page = 3,
  fault_page    = 4,
  bill_page     = 5,
  disconnected,
  connected,
  charging,
  finished,
  fault,
  online,
  offline,
  conuntdown,
};

typedef struct 
{
	uint8_t control_page;
}
Dwin_data;

extern Dwin_data g_Dwin_data;
	
int    LCD_Parse(uint8 *pData, int timeout);
void   LCD_backlight_control_frame(uint8_t light_value);
void   LCD_tx_handshake_frame(void);
void   LCD_QR_code_display_frame(uint8_t *QR_code_data, uint8_t QR_code_length);
void   LCD_switch_images_frame(uint8_t image_ID);
void   LCD_ICON_display_frame(uint8_t icon_ID);
void   LCD_variables_display_frame(u8_t front_size, u8_t integer, u8_t decimal, u32_t x_coordinate, u32_t y_coordinate, u32_t variable_data);
void LCD_asc_display_frame(u8_t front_size,  u32_t x_coordinate, u32_t y_coordinate,u8_t *asc, u8_t  len);
#ifdef __cplusplus
}
#endif

#endif
