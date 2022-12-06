/******************************************************************************

******************************************************************************/

#ifndef      TSK_TOUCHDW_H
#define      TSK_DISPLAY_H

/******************************************************************************
* Includes
******************************************************************************/
#include <stdint.h>
#include "cmsis_os.h"
/******************************************************************************
* Definitions
******************************************************************************/

/******************************************************************************
* Data Type Definitions
******************************************************************************/
#define HeadFram1 0XA5
#define HeadFram2 0X5A

#define CH_A  0
#define CH_B  1

#define NOADMIN      0    //无管理员登陆
#define SUPERADMIN   1
#define NORMALADMIN  2

#define METER_ADDR  0x500

#define MIAN_SOFT_VERSION_ADDR		0x0320	//触摸屏软件版本页起始地址 
#define NET_SET_ADDR                0X0600  //网络设置页面的起始地址
#define FEE_ONE_SET_ADDR            0X03B7  //费率一设置页面起始地址
#define FEE_TWO_SET_ADDR            0X03D1  //费率二设置页面起始地址
#define PARA_ONE_SET_ADDR           0X03FD  //参数设置一页面起始地址
#define PARA_TWO_SET_ADDR           0X0411  //参数设置一页面起始地址
#define CHARGE_INFO_DATA_ADDR       0X04B5  //A枪充电信息页面
#define TIME_SET_ADDR               0X4E3   //时间设置页起始地址
//#define btn_AUTO_CRG_Addr           0X0016//智能充电
#define var_QR_Addr0                 0x0100  //主页二维码地址  A枪 
#define var_QR_Addr1                 0x0101  //主页二维码地址  B枪
#define var_MacSerialNum_Addr    0x0102  //机器串号地址
#define CHARGE_ICO_A_ADDR        0x04B1   //A枪的ICO地址

#define var_Caution_Addr    0x0032   //提醒、告警信息提示的起始信息
#define var_Countdown_Addr  0x0063   //倒计时显示地址
#define var_Internet_Onling_Addr  0x0062   //网络图标显示地址

#define btn_TIME_CRG_Addr           0X0010//按时间充电
#define btn_ENERGY_CRG_Addr         0X0011//按电量充电
#define btn_MONEY_CRG_Addr          0X0012//按金额充电
#define btn_Login_Check_Addr        0x0013 //密码登入

typedef enum
{
	TOUCH_CMD_NONE            =0,
	TOUCH_CMD_WRITE_REGISTER  =0X80,
	TOUCH_CMD_READ_REGISTER   =0X81,
	TOUCH_CMD_WRITE_VARIABLE  =0X82,
	TOUCH_CMD_READ_VARIABLE   =0X83,
	TOUCH_CMD_DRAW_CURVE      =0X84,
}TOUCH_CMD;

typedef enum
{
	CRG_MODE_AUTO				,		//自动充电
	CRG_MODE_MONEY				,		//金额模式
	CRG_MODE_TIME				,		//时间模式
	CRG_MODE_ENERGE				,		//电能模式
}CRG_MODE;

typedef enum
{
    PAGE_ID_HOME                        = 0,                             //开机首页
    PAGE_ID_MAIN_0                      = 1,                             //空闲窗口a
    PAGE_ID_MAIN_1                      = 2,                             //空闲窗口b
    PAGE_ID_CRGING_MSG_0                = 3,                             //充电信息a
    PAGE_ID_CRGING_MSG_1                = 4,                             //充电信息b
    PAGE_ID_START_ING                   = 5,                             //启动充电-ING  
    PAGE_ID_SELECT_MODE                 = 6,                             //选择充电模式
    
    PAGE_ID_SET_MODE_SUCCESS            = 12,                            //模式设置完成
    PAGE_ID_ADMIN_LOGIN                 = 13,                            //管理员登陆入口
    PAGE_ID_SYSTEM_MANAGE_HOME          = 15,                            //厂方系统管理主页
    PAGE_ID_CURRENT_FAULT               = 16,                            //当前故障
    PAGE_ID_HISTORY_FAULT               = 17,                            //历史故障
    PAGE_ID_CRG_HISTORY_0               = 18,                            //充电记录0
    PAGE_ID_CRG_HISTORY_1               = 19,                            //充电记录1
    PAGE_ID_DEVICE_MSG_PG_INPUT         = 20,                            //设备信息录入
    PAGE_ID_METER_INFO                  = 21,                            //电表信息
    PAGE_ID_OPT_RECORD               	= 22,                            //操作记录
    PAGE_ID_NORMAL_PASSWORD_SET         = 23,                            //用户密码设置
    PAGE_ID_NET_SET                     = 24,                            //网络设置
    PAGE_ID_FEE_SET_1                   = 25,                            //费率设置1
    PAGE_ID_PARA_SET_1                  = 26,                            //参数设置1
    PAGE_ID_PARA_SET_2                  = 27,                            //参数设置2   
    PAGE_ID_FEE_SET_2                   = 28,                            //费率设置2
    
    PAGE_ID_BOOK_SET                    = 31,                            //预约设置
    PAGE_ID_FAULT_INFO                  = 32,                            //故障信息提醒
    PAGE_ID_CONSUME_MSG                 = 35,                            //消费信息
    PAGE_ID_INSERT_GUN                  = 36,                            //插枪提醒
    PAGE_ID_NORMAL_SYSTEM_MANAGE_HOME   = 37,                            //普通管理员系统管理主页
    PAGE_ID_PASSWORD_SET                = 38,                            //厂方对用户密码进行设置
    PAGE_ID_SYSTEM_TIME_SET             = 39,                            //时间设置
    PAGE_ID_NORMAL_DEVICE_INPUT         = 42,                            //普通用户设备信息输入
    PAGE_ID_CONSUME_MSG_APP             = 48,                            //消费信息 APP
    
}PAGE_ID;


typedef struct
{
    uint8_t   firstFlag;        //第一次进入标志  
    uint8_t   startStatus;      //开启延时标志   1为开启延时   0为关闭
    uint32_t  timeStamp;     //延时时间戳
}PAGE_DELAY;

typedef struct 
{
    uint8_t   commFlag;           // 通讯状态        
    uint8_t	  commErrThreshold;   // 通讯失败阈值       
    uint32_t  commErrStartTime;   // 通讯失败计时开始时刻   
    uint32_t  commErrCount;
	uint32_t  startTime;          // 数据接收
	uint16_t  deviceStatus;
    uint32_t  enterPageTime;
    uint16_t  setPageId;
    uint8_t   curCrgPort;   //当前操作的充电口
    uint8_t   curSetChannal;   //当前设置的充电通道
    uint8_t   testSerNo[16];
}TouchData;

typedef enum
{
	e_IDLE,		    //空闲
    e_DATA_DEAL,    //数据处理中
	e_CHARGE,	    //充电中
	e_BOOK,		    //预约	
	e_CRG_STOP, 	//充电停止
    e_CRG_STOP_EXCEP ,  //异常停止
    e_CONSUME_OVER, //消费结算成功
	e_CHARGE_ONLINE,//在线充电  显示状态   
    e_START_CHARG_ING,//启动充电中
    e_GIVE_UP_CHARGE,//放弃充电
    e_UPDATA,//升级中
}E_ChargeStatus;

typedef struct
{
    uint8_t ModeSetFlag;    //模式设置Flag   0xff表示被设置   0则没有
	uint8_t SelectMode; 	//用户选择模式 ， e_AUTO自动充电，1自定义充电
    uint8_t BookStatus;    //预约状态
	uint8_t ChargeStatus;  //充电机状态 
//	uint8_t ChargeMode;	   //充电模式 自动模式 电量模式 时间模式
	uint32_t Para;		   //输入参数 电量值
//    uint8_t ChargeType;   //充电方式 1:立即 2:延时
//    uint8_t BookGunPos;   //预约枪号
    uint8_t Booktime[2];  //预约时间值  [0] 为时   [1]为分
//    uint32_t OptTime;		//操作时间
//    uint8_t SecCountDown;  //s秒倒计时
//    uint32_t PageSec;
  
}CHARGE_MODE_INFO;

typedef struct
{
    uint8_t LoginMode;   //1超级管理员    2为普通管理员
    uint8_t  FirstEnterPageFlag;//用于标记登陆管理系统是否是第一次登陆该页
    uint8_t  ChargeFirstFlag;//用于充电流程第一次进入的标志
}SYSTEM_SET_PARA;


typedef enum
{
    INSERT_GUN,     //提示插入充电枪
    SWIP_CARD,      //提示刷卡
    READY_CHARGE,   //提示充电启动中
    CHARGEING,      //启动充电
    CHARGE_STOP,    //正常停止充电充电完成
    CHARGE_STOP_EXCEP,//异常停止充电
    END_CARD_NOT_MATCH,//卡不匹配
    SWIP_END_CARD,  //刷卡结算
    CONSUM_INFO,    //消费信息
    TAKE_BACK_GUN,  //收枪
    AUTH_CARD,      //正在验证卡片
    BOOK_SUCCES,    //预约刷卡成功
    BOOK_CANCEL,    //用户取消预约
    CARD_NOT_ACCOUNT,//未结算
    CARD_ACCOUNT_LOCK, //账户冻结
    CARD_NO_MONEY,  //余额不足
    CARD_ONLINE_ERR,  //未知错误
    OFF_LINE,       //断网
    TRIAL_TIME_END,//试用期结束
    RERTURN_IDLE,//回到初始状态
    SET_TIME,      //设定触摸屏时间
    UPDATAING,      //升级中
    CHARGE_FAIL,    //启动失败
    CTRL_BAKE_LIGHT,//控制背光
}LCD_STATE;


typedef enum
{
	OTHER_FUN_BUTTON			,		//其他方式按钮
    LOGIN_ADMIN_BUTTON			,		//管理员登陆按钮
    SHITFT_A_B_DISPLAY  		,		//切换AB口的显示   预留
}MAIN_BUTTON_NO;

typedef enum
{
	MODE_RETURN                 ,  		//模式选择页返回按钮
	APPOINTMENT_BTN    			,		//预约充
	TIME_BTN            		,		//时间充
    ENERGE_BTN                  ,       //电量充
    MONEY_BTN                   ,       //金额充
}MODE_BUTTON_NO;

typedef enum
{
	LOGIN_RETURN                ,  		//返回按钮
	INPUT_PASSWORD    			,		//输入密码
}ADMIN_LOGIN_NO;

typedef enum
{
	SYSTEM_RETURN               ,  	//返回按钮
	RETURN_HOME         		,	//输入密码
    CUR_FAULT                   ,   //当前故障
    HISTORY_RECORD              ,   //历史记录
    CRG_RECORD                  ,   //充电记录
    DEVICE_INFO                 ,   //设备信息
    METER_INFO                  ,   //电表信息
    CONFIG_SET                  ,   //配置设置
    PASSWORD_SET                ,   //密码设置
    OPT_RECORD                  ,   //操作记录
}SYSTEM_MANAGE_NO;
typedef enum
{
	HISTORY_RETURN              ,  	//返回按钮
	HISTORY_UP           		,	//上翻历史记录
    HISTORY_DOWN                ,   //下翻历史记录
    SHIFT_BTN                   ,   //左右切换按钮
}PAGE_HISTORY_FAULT_NO;

typedef enum
{
	FEE_RETURN                  ,  	//返回按钮
	PRE_PAGE            		,	//前一页
    NEXT_PAGE                   ,   //下一页
    RESTORE_FACTORY_BUTTON      ,   //恢复出厂设置
    DELETE_RECORD_DATA          ,   //删除记录数据        
}PAGE_FEE_NO;
typedef enum
{
	PARA_SET_RETURN                  ,  	//返回按钮
    TIME_CONFIRM_BUTTON  =  3             ,      //时间确认按钮
	
}PAGE_PARA_SET_NO;

void SetupTouchApp(osPriority ThreadPriority,uint8_t StackNum);
extern CHARGE_MODE_INFO Get_Charge_Mode_Info( uint8_t Ch);
osThreadId Get_Touch_Task_Id(void);
uint32_t Get_Page_ID(uint8_t Ch);




#endif
