
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
    UPDATEMODE_NONE,    //û������
    MODBUS_MOD,         //modbusЭ������
    FTP_MODE,           //ftp Э������
    FTP_IAP_MODE,           //ftp Э������
    USB_MODE,         //usb������
    USB_USART,        //485������
}UPDATEMODE;

typedef struct
{
    
    char UpdateMode;//������ʽ
    short ftpPort;   //FTP�˿�
    char ftpSer[20];//FTP��ַ
    char ftpUsr[16];//FTP�û���
    char ftpPass[16];
    char ftpFile[32];
    char SoftVer[16];

	char Update_url[64];//Զ��FTP url��ַ
	short Update_port;
    char Update_Usr[16];
	char Update_Pass[16];
	char Update_File[32];
	
}UpdateInfo;

typedef struct
{
    char SoftVer[16];
	char TCP_File_name[32];//�����ļ�����
	int  TCP_File_length;//�����ļ�����
	//short TCP_File_bags_number;//�������ܰ���
	int TCP_File_bags_serial_number;//���������к�
	char TCP_File_data[256];//����-��������
	int TCP_File_updated_len;//����������
	
	
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
