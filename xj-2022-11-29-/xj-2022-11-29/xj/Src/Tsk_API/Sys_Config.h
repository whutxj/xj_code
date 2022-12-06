/******************************************************************************
*
*  File name:     GUI_App.h
*  Author:
*  Version:       V1.00
*  Created on:    2017-01-03
*  Description:
*  History:
*      Date        Author        Modification
*    1.2014-12-15
*    2. ...
******************************************************************************/
#ifndef SYS_CONFIG_H
#define SYS_CONFIG_H

/******************************************************************************
* Includes
******************************************************************************/
#include "main.h"
#include "cmsis_os.h"
#include "Global.h"
#include "time.h"
/******************************************************************************
* Macros Definitions (constant/Macros)
******************************************************************************/

//�豸���Ͷ���
#define  AS_3K5  0  //���װ�
#define  AS_7K   1
#define  BS_3K5  2  //��׼�汾
#define  BS_7K   3
#define  HS_3K5  4  //��������
#define  HS_7K   5
#define  HS_40K   6
/******************************************************************************
* Data Type Definitions
******************************************************************************/

typedef enum
{
    LAN,
    DTU,
    WIFI,

}CommType;//ͨѶ����

typedef enum
{
    KH_P,//����ƽ̨Э��
    HP_P,//
    KH_V4,
}CommProType;//ͨѶЭ������

typedef enum
{
    CUR_16,
    CUR_32,
    CUR_63,
}CurrentType;//�����������

typedef enum
{
    TYPE_DLT_07,
    TYPE_DLT_97,
    TYPE_HARD,
    TYPE_RN8209,
}MeterType;//�����������


typedef enum
{
    PHASE_1,
    PHASE_3,
}phaseType;//����/��

typedef enum
{
    MODBUS_SLAVER,
    MODBUS_MASTER,
}WORK_MOD;
typedef enum
{

    CARD_NONE,//����ʹ��APP
    CARD_M2,//M2��
    CARD_MT,//����M1
    NO_CARD_START,//�޿�ֱ������
    CARD_MT628,//CPU
}CARD_MOD;

typedef enum
{
    OPT_MAC,       //MAC
    OPT_SERVADDR,  //����
    OPT_PORT,    //�˿ں�
    OPT_NETMASK,  //��������
    OPT_GETAWAY,  //����
    OPT_OVER_VOLT, //�޸Ĺ�ѹ��ֵ
    OPT_UNDER_VOLT, //�޸�Ƿѹ��ֵ
    OPT_PRICE1,//����1
    OPT_PRICE2,//����2
    OPT_PRICE3,//����3
    OPT_PRICE4,//����4
    OPT_PRICE5,//����5
    OPT_PRICE6,//����6
    OPT_PRICE7,//����7
    OPT_PRICE8,//����8
    OPT_PRICE9,//����9
    OPT_PRICE10,//����10
    OPT_PRICE11,//����11
    OPT_PRICE12,//����0~11
    OPT_PRICE13,//����0~11
    OPT_PRICE14,//����0~11
    OPT_PRICE15,//����0~11
    OPT_PRICE16,//����0~11
    OPT_PRICE17,//����0~11
    OPT_PRICE18,//����0~11
    OPT_PRICE19,//����12~23
    OPT_PRICE20,//����24~35
    OPT_PRICE21,//����24~35
    OPT_PRICE22,//����36~47
    OPT_PRICE23,//����36~47
    OPT_PRICE24,//����36~47
    OPT_PRICE25,//����36~47
    OPT_PRICE26,//����36~47
    OPT_PRICE27,//����36~47
    OPT_PRICE28,//����36~47
    OPT_PRICE29,//����36~47
    OPT_PRICE30,//����36~47
    OPT_PRICE31,//����36~47
    OPT_PRICE32,//����36~47
    OPT_PRICE33,//����36~47
    OPT_PRICE34,//����36~47
    OPT_PRICE35,//����36~47
    OPT_PRICE36,//����36~47
    OPT_PRICE37,//����36~47
    OPT_PRICE38,//����36~47
    OPT_PRICE39,//����36~47
    OPT_PRICE40,//����36~47
    OPT_PRICE41,//����36~47
    OPT_PRICE42,//����36~47
    OPT_PRICE43,//����36~47
    OPT_PRICE44,//����36~47
    OPT_PRICE45,//����36~47
    OPT_PRICE46,//����36~47
    OPT_PRICE47,//����36~47
    OPT_PRICE48,//����36~47

    OPT_SEV_FEE,//�����
    OPT_FEE_VER,//���ʰ汾
    OPT_POWER_ON,//ϵͳ����
    OPT_SET_TIME,//ϵͳʱ��
    OPT_UPDATA,  //������¼
}OPT_TYPE;


#pragma pack(2)
typedef struct
{
    U16 ParamSize;//����ռ�õ�ַ�ռ�

    U8 LocalIp[4];
    U8 SerIp[36];// ����
    U16 Port;
    U8 NetMask[4];
    U8 GetAway[4];
    U8 DomainSer[4];//����������
    U8 MAC[6];


    U8 SerIpBake[32];//��������
    //10.28
    U32 restart_time;
    
    U16 PortBake;//���ö˿�
    U16 OverVolt;
    U16 Undervoltage;
    U16 Price[48];
    U16 FeeTime[48];
    U16 CrgServFee;       //�������
    U8 FeeVersion[10];    //���ʰ汾


    U8 GunLockEn;         //ǹ���Ƿ�����
    U8 phaseType;         //����/����
    U8 ScreenEn;          //��Ļʹ��;
    U8 DoubelGunEn;       //0��ǹ1˫ǹ
    U8 AutoIP;          //�Զ�IP 1���ֶ� 0���Զ�
    U8 CurrentType;       //�����������
    U8 WorkMod;           //׮���������� 0 �ӻ�  1����
    U8 DeviceType;        //�豸����
    U8 BiasEn;            //��бʹ��

    U16 CommType;           //ͨѶЭ������ 0 �ƻ� 1 �����Ǳ�
    U16 MeterType;          //�������   0 07Э�� 1 97Э�� 2 �ɼ�
    U16 DtuType;            //DTU����

    U8 HardVer[20];         //Ӳ���汾
    U8 Serial[20];          //�������к�   ׮��
    U8 DevName[20];         //�豸����

    U8 SlaveNum;        //�ӻ�����
    U8 SlaveAddr;       //�ӻ�ͨѶ��ַ

    U8 CardReaderType;  //����������
    U8 IcCardKey[6];    //IC������

    U16 PassWord;       //ϵͳ����

    U8 TrialTime[3];    //������ʱ��
	  U16 DbgMod;			//����ģʽ
    U16 FeeSectionNum;   //��������
    U16 PakeFee;        //ͣ����
    U16 interval;
    U8 QRCodeEN;        //ʹ��
    U8 QRCode[70];      //��ά��
    U8 QR_code_length;

  	U8 TCP_ud_flag;    //TCP������־ 1-��ʼ��־ 2-�Ժ���������
  	U32 TCP_ud_stop_adr;    //TCP������ͣ��ַ
  	U32 TCP_ud_stop_serial_number;   //TCP������ͣ���к�

    U8  restart_num;//��������
    
    
    U8 Reserve[3];      //����3
    U16 CrcVal;         // У���


}SYS_CONFIG_PARAM;

#pragma pack()

typedef struct
{
  char opt_type;//��������
  char *fmt;
  void *val_name;//��������
  char val_len;//����ֵ����
  char retio;  //�Ŵ���
}opt_record;

typedef struct
{

	char opt_type;	//�޸�����
    uint32 opt_time;//�޸�ʱ��
    uint8 LogVal[36];

}opt_record_info;


typedef struct
{
    uint8 LogType;

    uint8 LogVal[36];

}USR_LOG;

typedef struct
{
    char *cmd;//��������

	int (*fun_Proc)(char *p,void *Com);//������

    char *help;//�������

}CMD_INFO;

typedef struct
{
	char *name;//ֵ����  ��Ӧ��ֵ������

	char lenth;//����    ��Ӧ������ֵ�ĳ���

	void *val;//ֵ����   ��Ӧ����ֵ�Ļ�����

    char *fmt;//��ʽ     ��Ӧ���������� ip ��int str

}PARAM_INFO;

typedef struct
{
    int link_type;
    int fd;
}COMM_TYPE;

extern SYS_CONFIG_PARAM g_Sys_Config_Param;
/******************************************************************************
* Global Variable Definitions
******************************************************************************/
extern char SoftVer[20];
#ifdef __cplusplus
 extern "C" {
#endif

/******************************************************************************
* Function Prototype Definitions
******************************************************************************/

void eth_param_cfg_proc(void);
int Parse_Line(COMM_TYPE *cmm,char *line,int len);
uint8 param_config_init(void);
void check_opt_record(void);
void Sys_Config_Para_Load(void);
SYS_CONFIG_PARAM *GetSys_Config_Param(void);
SYS_CONFIG_PARAM *GetTmp_Config_Param(void);

U8 Get_phaseType(void);
char *Get_ComlieTime(void);
U8 Get_DoubelGunEn(void);
U8 Get_Server_Ip(U8 *ip);
U8 Get_Cur_Out_Type(void);
U8 Get_Local_Ip(U8 *ip);
U8 Get_NetMask(U8 *Mask);
U8 Get_GetAway(U8 *GetAway);
U8 Get_NetMAC(U8 *MAC);
U8 Get_DomainSer(U8 *Ser);
U8 Get_Dev_Name(uint8 *dev);
U16 Get_Server_Port(void);
void Get_Card_Key(uint8 *Key);
U8 Get_Local_Ip(U8 *ip);
U8 Get_NetMask(U8 *Mask);
U8 Get_GetAway(U8 *GetAway);
U8 Get_NetMAC(U8 *MAC);
U8 Get_Cur_Out_Type(void);
U8 Get_ScreenEn(void);
U8 Get_LocKEn(void);
U16 Get_OverVolt(void);
U16 Get_Undervoltage(void);
U8 Get_WorkMod(void);
U8 Get_SlaverAddr(void);
U8 Get_MeterType(void);
U16 Get_Price(uint8 index);
U8 Get_Card_Read_Type(void);
U8 Get_Dev_Name(uint8 *dev);
uint8 Trial_Time_Check(void);
U8 Get_Protocol_Type(void);
char Get_AutoIP_En(void);
char Get_Screen_En(void);
U8 Get_BiasEn(void);
void set_sys_time_record(struct tm set_time);
void save_updata_record(uint8 res);
U8 Get_ip(void);

#ifdef __cplusplus
}
#endif

#endif

