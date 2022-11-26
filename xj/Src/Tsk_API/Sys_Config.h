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

//设备类型定义
#define  AS_3K5  0  //简易版
#define  AS_7K   1
#define  BS_3K5  2  //标准版本
#define  BS_7K   3
#define  HS_3K5  4  //高配类型
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

}CommType;//通讯类型

typedef enum
{
    KH_P,//自研平台协议
    HP_P,//
    KH_V4,
}CommProType;//通讯协议类型

typedef enum
{
    CUR_16,
    CUR_32,
    CUR_63,
}CurrentType;//电流输出类型

typedef enum
{
    TYPE_DLT_07,
    TYPE_DLT_97,
    TYPE_HARD,
    TYPE_RN8209,
}MeterType;//电流输出类型


typedef enum
{
    PHASE_1,
    PHASE_3,
}phaseType;//单相/三

typedef enum
{
    MODBUS_SLAVER,
    MODBUS_MASTER,
}WORK_MOD;
typedef enum
{

    CARD_NONE,//仅仅使用APP
    CARD_M2,//M2卡
    CARD_MT,//铭特M1
    NO_CARD_START,//无卡直接启动
    CARD_MT628,//CPU
}CARD_MOD;

typedef enum
{
    OPT_MAC,       //MAC
    OPT_SERVADDR,  //域名
    OPT_PORT,    //端口号
    OPT_NETMASK,  //子网掩码
    OPT_GETAWAY,  //网关
    OPT_OVER_VOLT, //修改过压阀值
    OPT_UNDER_VOLT, //修改欠压阀值
    OPT_PRICE1,//费率1
    OPT_PRICE2,//费率2
    OPT_PRICE3,//费率3
    OPT_PRICE4,//费率4
    OPT_PRICE5,//费率5
    OPT_PRICE6,//费率6
    OPT_PRICE7,//费率7
    OPT_PRICE8,//费率8
    OPT_PRICE9,//费率9
    OPT_PRICE10,//费率10
    OPT_PRICE11,//费率11
    OPT_PRICE12,//费率0~11
    OPT_PRICE13,//费率0~11
    OPT_PRICE14,//费率0~11
    OPT_PRICE15,//费率0~11
    OPT_PRICE16,//费率0~11
    OPT_PRICE17,//费率0~11
    OPT_PRICE18,//费率0~11
    OPT_PRICE19,//费率12~23
    OPT_PRICE20,//费率24~35
    OPT_PRICE21,//费率24~35
    OPT_PRICE22,//费率36~47
    OPT_PRICE23,//费率36~47
    OPT_PRICE24,//费率36~47
    OPT_PRICE25,//费率36~47
    OPT_PRICE26,//费率36~47
    OPT_PRICE27,//费率36~47
    OPT_PRICE28,//费率36~47
    OPT_PRICE29,//费率36~47
    OPT_PRICE30,//费率36~47
    OPT_PRICE31,//费率36~47
    OPT_PRICE32,//费率36~47
    OPT_PRICE33,//费率36~47
    OPT_PRICE34,//费率36~47
    OPT_PRICE35,//费率36~47
    OPT_PRICE36,//费率36~47
    OPT_PRICE37,//费率36~47
    OPT_PRICE38,//费率36~47
    OPT_PRICE39,//费率36~47
    OPT_PRICE40,//费率36~47
    OPT_PRICE41,//费率36~47
    OPT_PRICE42,//费率36~47
    OPT_PRICE43,//费率36~47
    OPT_PRICE44,//费率36~47
    OPT_PRICE45,//费率36~47
    OPT_PRICE46,//费率36~47
    OPT_PRICE47,//费率36~47
    OPT_PRICE48,//费率36~47

    OPT_SEV_FEE,//服务费
    OPT_FEE_VER,//费率版本
    OPT_POWER_ON,//系统开机
    OPT_SET_TIME,//系统时间
    OPT_UPDATA,  //升级记录
}OPT_TYPE;


#pragma pack(2)
typedef struct
{
    U16 ParamSize;//参数占用地址空间

    U8 LocalIp[4];
    U8 SerIp[36];// 域名
    U16 Port;
    U8 NetMask[4];
    U8 GetAway[4];
    U8 DomainSer[4];//域名服务器
    U8 MAC[6];


    U8 SerIpBake[32];//备用域名
    //10.28
    U32 restart_time;
    
    U16 PortBake;//备用端口
    U16 OverVolt;
    U16 Undervoltage;
    U16 Price[48];
    U16 FeeTime[48];
    U16 CrgServFee;       //充电服务费
    U8 FeeVersion[10];    //费率版本


    U8 GunLockEn;         //枪座是否锁定
    U8 phaseType;         //单相/三相
    U8 ScreenEn;          //屏幕使能;
    U8 DoubelGunEn;       //0单枪1双枪
    U8 AutoIP;          //自动IP 1是手动 0是自动
    U8 CurrentType;       //电流输出类型
    U8 WorkMod;           //桩间组网类型 0 从机  1主机
    U8 DeviceType;        //设备类型
    U8 BiasEn;            //倾斜使能

    U16 CommType;           //通讯协议类型 0 科华 1 其他非标
    U16 MeterType;          //电表类型   0 07协议 1 97协议 2 采集
    U16 DtuType;            //DTU类型

    U8 HardVer[20];         //硬件版本
    U8 Serial[20];          //本机序列号   桩号
    U8 DevName[20];         //设备名称

    U8 SlaveNum;        //从机个数
    U8 SlaveAddr;       //从机通讯地址

    U8 CardReaderType;  //读卡器类型
    U8 IcCardKey[6];    //IC卡密码

    U16 PassWord;       //系统密码

    U8 TrialTime[3];    //试用期时间
	  U16 DbgMod;			//调试模式
    U16 FeeSectionNum;   //费率数量
    U16 PakeFee;        //停车费
    U16 interval;
    U8 QRCodeEN;        //使能
    U8 QRCode[70];      //二维码
    U8 QR_code_length;

  	U8 TCP_ud_flag;    //TCP升级标志 1-开始标志 2-稍后重启升级
  	U32 TCP_ud_stop_adr;    //TCP升级暂停地址
  	U32 TCP_ud_stop_serial_number;   //TCP升级暂停序列号

    U8  restart_num;//重启次数
    
    
    U8 Reserve[3];      //保留3
    U16 CrcVal;         // 校验和


}SYS_CONFIG_PARAM;

#pragma pack()

typedef struct
{
  char opt_type;//操作类型
  char *fmt;
  void *val_name;//操作对象
  char val_len;//操作值长度
  char retio;  //放大倍数
}opt_record;

typedef struct
{

	char opt_type;	//修改类型
    uint32 opt_time;//修改时间
    uint8 LogVal[36];

}opt_record_info;


typedef struct
{
    uint8 LogType;

    uint8 LogVal[36];

}USR_LOG;

typedef struct
{
    char *cmd;//命令类型

	int (*fun_Proc)(char *p,void *Com);//处理函数

    char *help;//命令帮助

}CMD_INFO;

typedef struct
{
	char *name;//值名称  对应的值的名称

	char lenth;//长度    对应参数的值的长度

	void *val;//值内容   对应参数值的缓冲区

    char *fmt;//格式     对应参数的类型 ip ，int str

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

