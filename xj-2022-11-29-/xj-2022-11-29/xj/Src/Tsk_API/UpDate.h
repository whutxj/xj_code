
#ifndef UP_DATA_H
#define UP_DATA_H
#include "Utility.h"
#include "BSP_Gpio.h"
//#include "BSP_RTC.h"
#include "BSP_Pwm.h"
#include "BSP_ADc.h"
#include "BSP_buzzer.h"
#include "BSP_WatchDog.h"


#define FTPLIB_BUFSIZ   128
#define MIN_APP_SIZE    50 * 1024 

#define LEN_READ  256 
#define SPI_FLASHE_ADDR    0

#define APP_SIZE    (1024 * 196)

#define UPDATA_INFO_ADDR   (APP_SIZE + SPI_FLASHE_ADDR)
#define TCP_Update_File_bags_number 784


typedef  void (*pFunction)(void);

typedef enum
{
    UPDATEMODE_NONE,    //没有升级
    MODBUS_MOD,         //modbus协议升级
    FTP_MODE,           //ftp 协议升级
    FTP_IAP_MODE,           //ftp 协议升级
    USB_MODE,         //usb口升级
    USB_USART,        //485口升级
}UPDATEMODE;

typedef struct
{
    
    char UpdateMode;//升级方式
    short ftpPort;   //FTP端口
    char ftpSer[20];//FTP地址
    char ftpUsr[16];//FTP用户名
    char ftpPass[16];
    char ftpFile[32];
    char SoftVer[16];

	char Update_url[64];//远程FTP url地址
	short Update_port;
    char Update_Usr[16];
	char Update_Pass[16];
	char Update_File[32];
	
}UpdateInfo;

typedef struct
{
    char SoftVer[16];
	char TCP_File_name[32];//升级文件名称
	int  TCP_File_length;//升级文件长度
	//short TCP_File_bags_number;//升级包总包数
	int TCP_File_bags_serial_number;//升级包序列号
	char TCP_File_data[256];//单包-升级数据
	int TCP_File_updated_len;//已升级长度
	
	
}TCP_UpdateInfo;


void API_Save_Update_Info(UpdateInfo* pInfo);

void API_Updata_Proc(void);

void API_CheckUpdata_Proc(void);

int API_Ftp_Updata(UpdateInfo* pInfo);

void init_program(void);

int program_file(uint8 *dbuf,int read_len);
int check_file(void);
int  ftp_doload_file(UpdateInfo* pInfo);

#endif
