#ifndef BLT_H
#define BLT_H
#include"main.h"
#include"Tsk_JWT.h"
//#define TRUE 1
//#define FALSE 0
//#define NULL ((void *)0)



typedef unsigned short     int uint16;
typedef unsigned          char uint8;
typedef unsigned           int uint32;




typedef enum
{
    STATE_IDLE,         //³äµç¿ÕÏÐ´ý»ú×´Ì¬
    STATE_CHARGEING,    //³äµç×´Ì¬£¬
    STATE_END_CHARGE,   //³äµçÍ£Ö¹×´Ì¬
    STATE_BOOK,         //³äµçÔ¤Ô¼×´Ì¬
}BLT_STATE;











static uint8 BLT_test(void);
uint16 rcvbuf_judge(uint8* pData, char* expect);
static uint16 BLT_Pack(uint8* SendBuf, uint16 cmdnumber, uint16 Cmd, uint8* pData, uint16 length);
static uint16 BLT_Parse(uint8* pData, int timeout);
static uint8 BLT_AT_Cmd_Parse(char* send, char* expect, char* ret, uint32 timeout);




#endif