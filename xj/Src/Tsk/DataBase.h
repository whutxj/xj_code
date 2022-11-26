#ifndef DATA_BASE_H
#define DATA_BASE_H


#include "global.h"
#include "main.h"
#include "Tsk_API_IO_Ctrl.h"
enum
{
    CRG_NONE,
    CRG_STEP,
};

typedef enum
{
	RW_FLAG_READ,
	RW_FLAG_WRITE,
}RW_FLAG;

#define CRG_RECORD_TYPE_A  0
#define CRG_RECORD_TYPE_B  1
typedef struct
{
    uint16_t item;      //数据标识
    uint8_t *pData;     //数据指向的指针
    uint8_t Len;        //数据长度
    uint8_t fmt;        //数据格式
}DataBase;
typedef enum
{
	TERMINAL_STATUS_IDLE,
	TERMINAL_STATUS_CONNECT,
	TERMINAL_STATUS_CRGING,
	TERMINAL_STATUS_FAULT,
	TERMINAL_STATUS_BOOK,
}TERMINAL_STATUS; //设备状态
//V4 桩状态
typedef enum
{
    DEV_IDEL,//空闲
    DEV_CONNECT,//连接
    DEV_WAITE,//等待
    DEV_READY,//启动中
    DEV_CRGING,//充电中
    DRV_RECONNECT,//重连
    DEV_RCV_BSD,//接收BSD
    DEV_ACCOUNT,//结算
    DEV_FAULTA,//故障A
    DEV_FAULTB,//故障B
    DEV_FAULTC,//故障C
    DEV_BOOK,//预约
    DEV_CLOD_BOOK,//后台预约
    DEV_UNPAY,//等待结算
    DEV_CRG_COMPLTE,//充电完成
    DEV_APP_BOOK,//预约
    DEV_OUT_SERVE,//不在服务时间

}DEV_STATUS;
typedef struct
{
    //握手信息                      描述                       小数点位数或者单位
	uint8 ucBmsVersion[3];			//电池协议版本号
	uint8 ucBatType;				//电池类型
	uint16 Capacity;				//总容量		            XXX.X
	uint16 Voltage;					//总电压		            XXX.X
	uint8 ucProductName[4];			//电池组电池厂家名称
	uint32 iBatIndex;				//电池组的序号
	uint8 uProductYear;				//电池组生产日期：年 BCD
	uint8 uProductMonth;			//电池组生产日期：月 BCD
	uint8 uProductDay;				//电池组生产日期：日 bcd

	uint32 uChargeTotal;			//电池组充电次数
	uint8 ucBatOwnerFlag;			//电池组的产权信息：0-租赁;1-车自有
	uint8 ucReserved;				//预留
	uint8 ucVehichleInfo[17];		//车辆辨识信息
    //配置信息

 	uint16 BatMaxChargeVol;			//单体蓄电池最高允许充电电压(0.00~24.00V) xx.xx
	uint16 BatMaxChargeCur;			//最高允许充电电流(-400.0A~0A)            xx.x
	uint16 uBatMaxEnergy;			//动力蓄电池标称总容量 0~1000ah 1%        xx.x
	uint16 BatMaxTotalChargeVol;	//最高允许充电总电压(0.0~800.0V)          xx.x
	int16 uBatMaxTempPlus50;		//蓄电池模块最高允许的温度(-50~200℃)     xx.x

	uint16 uTotalBatSOC;			//整车动力蓄电池荷电状态(0.0~100.0%)      xx.x
	uint16 fTotalBatVol;			//整车动力蓄电池总电压(0.0~750.0V)        xx.x

    //充电阶段

	uint16 iBatReqVol;				//电池电压需求     XX.X
	uint16 iBatReqCur;				//电池电流需求     XX.X
	uint8 uChargeMode;				//当前充电模式
	uint16 crgRealVol;				//充电电压测量值  xx.x
	uint16 crgRealCur;				//充电电流测量值  xx.x
	uint16 nBatMaxVol;				//最高单体电压 0.01/bit XX.XX
	uint8  nBayMaxVolPosition;		//最高单体电压所在组号
	uint8 uBatCurSOC;				//当前荷电状态    xx
	uint16 uEvalChargeFinTime;		//估算充满时间   分钟
    uint16 uBatMaxVolGroupNo;       //最高单体动力蓄电池电压所在组号
    int BatMaxTempPlus50;           //最高动力蓄电池温度
    uint8 uBatMaxTempPosition;      //最高温度检测点编号
    int    BatMinTem;               //最低动力蓄电池的温度
    uint8 BatMinTemPosition;        //最低动力蓄电池的温度检测点编号
    uint16 BAT_STATUS;              //蓄电池状态

	uint8 uBmsEndChargeReason;				//BMS终止充电原因
 	uint8 uBmsEndFaultCode[2];				//BMS终止故障原因
	uint8 uBmsEndErrCode;					//BMS终止充电故障原因


    //统计阶段
	uint8 EndSOC;					//终止荷电状态
	int iBatCellMinVol;			    //单体动力蓄电池最低电压
	int iBatCellMaxVol;			    //单体动力蓄电池最高电压
	char cBatMinTempPlus50;			//电池组最低温度 -50 偏移量
	char cBatMaxTempPlus50;			//电池组最高温度 -50 偏移量

}ST_BMS_DATA;//bms 数据


typedef struct
{
    uint16 crgVolt;                 //充电电压测量值    xx.x
    uint16 crgCur;                  //充电电流测量值    XX.X
    uint16 crgEnergy;               //充电电能          xx.x
    uint32 crgStartTime;            //充电开始时间 从1970年1月1号的秒
    uint32 crgStopTime;             //充电结束时间 从1970年1月1号的秒
    uint16 crgTime;                 //充电时间          秒
    uint32 crgStartEnergy;          //充电开始电表读数  xx.xx
    uint32 crgStopEnergy;           //充电结束电表读数  xx.xx
    uint32 CrgMoney;                //充电金额

    uint8 UsrId[20];                //用户卡号          ascii
    uint32 RemainMoney;             //用户余额          xx.xx元

    int32 crgGunTmp;                //充电枪的温度      xx.x

}ST_DC_CRG_DATA;//直流充电数据

typedef struct
{
    uint8 StopBit:1;        //急停状态  0 没有按下 1 按下
    uint8 ThunderBit : 1;   //防雷器状态 :1 异常 0 正常
    uint8 CrgGunBit :1;     //充电枪状态  0 是没有连接 1连接
    uint8 fuseBit:1;        //保险丝状态 0 正常 1 异常
    uint8 PEBit:1;          //地线状态 0 正常  1 异常
    uint8 RelayBit:1;       //接触器状态
    uint8 MeterComBit:1;    //表通讯状态 0 正常 1 异常
    uint8 CardComBit:1;     //卡通讯 0 正常 1 异常
    uint8 ModuleComBit:1;   //模块通讯 0 正常 1 异常
    uint8 bmsComBit:1;      //bms 通讯 0 正常 1异常
    uint8 touchComBit:1;    //触摸屏异常 0 正常 1 异常
    uint8 PileLockBit:1;   // 桩上锁状态 1 锁
    uint8 DoorBit:1;		// 门禁

}SYS_IO_STATUS;//IO 口的状态

typedef struct
{
	uint8_t 		deviceType;     //0001-设备类型
	uint32_t 		terminalNo;     //0002-桩体号
	uint8_t 		terminalIP[4];  //0003
	uint8_t     terminalGate[4];    //0004
	uint8_t			terminalMask[4];//0005
	uint16_t    terminalPort;         //0006
	uint8_t     terminalName[8];    //0007-桩名
	uint8_t			controlType;    //0008-控制方式0本地1远程
	uint8_t			payType;        //0009-结算方式0本地1远程
	uint8_t     calType;            //000A-计量方式0分时1统一
	uint32_t    maxVolt;            //000B桩体最高电压
	uint32_t		maxCurrent;     //000C状体最高电流
	uint32_t		minVolt;        //000D桩体最低电压
	uint32_t		ratedVolt;      //000E桩体额定电压
	uint32_t		ratedCurrent;   //000F桩体额定电流
	uint32_t    inAVolt;            //0010进相A电压
	uint32_t    inBVolt;            //0011进相B电压
	uint32_t    inCVolt;            //0012进相C电压
	uint8_t     addParkingFee;      //0013是否加收停车费0不加收1加收
	uint16_t     parkingFee;          //0014停车费单价
	uint16_t    stageFee[10];         //0015-0018阶段电价
	uint8_t     stageTime[10][2];   //0019-001C阶段时间段
	uint32_t    crgServiceFee;        //充电服务费
	uint8_t     sotfVer[20];        //软件版本号
	uint8_t     hardVer[20];        //硬件版本号
	uint8_t     serNo[20];          //机器串号
    uint8_t     feeRuleVer[10];     //计费版本

    //uint8_t     SysTime[7];    //

}DeviceInfo;//设备信息
typedef struct
{
	uint8_t 		crgType;            //0101充电方式
	uint8_t			crgMode;            //0102充电模式
    uint8_t			cardType;           //0103充电卡类型
	uint8_t			cardNO[20];         //0104充电卡号
	uint8_t         carNO[17];          //0105车牌号 //20
	uint32_t        preLeftMoney;       //0106充电前余额
	uint32_t		crgV;               //0107充电电压
	uint32_t        crgI;               //0108充电电流
	uint32_t        crgTime;            //0109充电时间
	uint32_t        crgMoney;           //010A充电金额
	uint32_t        crgE;               //010B充电电能
	uint32_t        startE;             //010C充电开始电能
	uint32_t        currentE;           //010D充电结束电能
	uint32_t 		leftTime;           //010E剩余时间
	uint8_t   	    currentSOC;         //010F当前SOC
	uint8_t			upHost;             //0110是否上传主站
	uint8_t			payMoney;           //0111是否付费
	uint8_t			stopReason;         //0112充电终止原因
	uint8_t			crgStartTime[7];    //0113充电开始时间
	uint8_t			endStartTime[7];    //0114充电结束时间
	uint32_t		crgIndex;           //0115记录流水号
	uint32_t		storeIndex;         //0116记录存储序号
    uint32      crgServiceFee;          //0117充电服务费2016.09.13
    uint32      totalFee;               //0118总费用     2016.09.13
    uint8       feeRuleVer1[10];        //0119计费版本  2016.09.13
    uint8       soc;                    //011Asoc  2017.11.8

    uint16_t       SectionE[10];        //011A 电量  2017.11.8
}RecordData;//充电记录

typedef struct
{
	uint32_t		alarmLocal;         //0201告警点
	uint32_t		alarmReason;        //0202告警原因
	uint8_t			alarmStartTime[7];  //0203告警开始时间
	uint8_t			alarmEndTime[7];    //0204告警结束时间
	uint32_t		alarmLastTime;      //0205告警持续时间
	uint8_t			alarmAllowCrg;      //0206是否影响充电
	uint8_t         upHost;             //0207是否上传主站
	uint32_t		alarmIndex;         //0208记录流水号
	uint32_t		storeIndex;         //0209记录存储序号
}AlarmData;//告警数据
typedef struct
{
	uint32_t	 		outputAV;       //0301输出A相电压
	uint32_t			outputBV;       //0302输出B相电压
	uint32_t			outputCV;       //0303输出C相电压
	uint8_t				pwm;            //0304占空比
}OUT_PUT;
typedef struct
{
	uint32_t			testV;          //0501测量电压
	uint32_t			testI;          //0502测量电流
	uint32_t			testP;          //0503测量功率
	uint8_t				meterAddr[6];   //0504通讯地址
	uint32_t            testE;                //0505测量电能
	uint32_t  		    pulseValue;         //0506有功脉冲常数
	uint8_t				meterNO[6];     //0507表号
	uint8_t				meterType[5];   //0508表型号
	uint8_t             meterMadingTime[5];   //0509生产日期
	uint8_t				protocolVersion[5];//050A协议版本号
	uint8_t				userNO[6];      //050B用户号
}MeterData;//表数据
typedef struct
{
	uint16_t			meterBaut;  //0A01电能表波特率
	uint16_t			touchBaut;  //0A02触摸屏波特率
	uint16_t			cardBaut;   //0A03读卡器波特率
	uint16_t			printBaut;  //0A04打印机波特率
}CommData;//通讯参数
typedef struct
{
	uint8_t			deviceStatus;//0B01设备状态
	uint8_t			crgCardNO[20];//0B02充电卡号
	uint8_t			carNO[17];  //0B03车牌号
	uint32_t		crgV;       //0B04充电电压
	uint32_t		crgI;       //0B05充电电流
	uint32_t		crgTime;    //0B06充电时间
	uint32_t		crgMoney;   //0B07充电金额
	uint32_t		crgE;       //0B08充电电能
	uint32_t		leftTime;   //0B09剩余时间
	uint8_t			currentSOC;	//0B0A当前SOC
	uint8_t			alarmMessage[8];//0B0C告警信息
	uint32_t        leftMoney;
	uint8_t			cardType;
	uint8_t         crgType;
	uint8_t         crgMode;
    uint32_t        crgServiceFee;
    uint32_t        totalFee;
    uint32_t        crgIndex;
    uint32_t		storeIndex;
}RunningData;//运行信息


typedef struct
{
	uint8_t			crgGun;//充电枪
    uint8_t			alrmCode;//
    uint8_t			alrmStatus;//
    uint8_t         sysTime[7];//
    uint32_t         crgModeData;//自动充电 金额充电数

}sysRunningDataBase;//运行信息

typedef struct
{
	unsigned char head;			//记录头指示当前是否可用
	unsigned int old_addr;	//最老的一条的地址
	unsigned int  cur_addr;	//当前待存储的地址
	unsigned int  read_addr;	//未上报地址
	unsigned int index;//for test
	unsigned char check_sum;

}record_param;
/*********************************************************************************
 函数名称：DB_Get_Bms_Data
 功能描述：获取BMS数据
 输    入：无
 输	   出：无
 返 回 值：BMS数据的副本值
 日    期：
 修改记录：
*********************************************************************************/
ST_BMS_DATA  DB_Get_Bms_Data(uint8 Ch);

/*********************************************************************************
 函数名称：DB_Get_Bms_Data
 功能描述：获取直流充电机数据实时值
 输    入：无
 输	   出：无
 返 回 值：BMS数据的副本值
 日    期：
 修改记录：
*********************************************************************************/
ST_DC_CRG_DATA  DB_Get_DC_CRG_Data(uint8 Ch);


/*********************************************************************************
 函数名称：DB_Get_Crg_Info
 功能描述：获取直流充电机记录数据
 输    入：无
 输	   出：无
 返 回 值：BMS数据的副本值
 日    期：
 修改记录：
*********************************************************************************/

SYS_IO_STATUS DB_Get_SYS_IO_Status(uint8 Ch);

uint8 MainLogic_Get_SubStep(uint8 Ch);

uint8 Get_Stop_Bit(uint8 Ch);
uint8 Get_Car_Link_Status(uint8 Ch);
unsigned char Get_UnReport_Data(int type,uint8* pData,int Ch);
unsigned char Updata_UnReport_Data(int type,uint8* pData,int Ch);
uint8 Get_Device_Info(uint8 Ch);
uint16_t Read_DataBase(uint16_t item,uint8_t *pData);
uint16_t Write_DataBase(uint16_t item,uint8_t *pData);
uint16_t Read_HistoryData(uint16_t item,uint8_t *pData);
uint16_t Read_AlarmRecord(uint16_t item,uint8_t *pData);
uint16_t Read_RealChargeData(uint16_t item,uint8_t *pData);
uint16_t Read_RealDataBase_V4(uint16_t item,uint8_t *pData);
uint16_t Read_RealChargeData_V4(uint16_t item,uint8_t *pData);
uint16_t Read_DataBase_V4(uint16_t item,uint8_t *pData);
uint8 Get_Device_Info_V4(uint8 Ch);

extern AlarmData        g_AlarmData;
extern RunningData      g_RunningData;
uint8 Get_deviceType(void);
#endif











