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

#define NOADMIN      0    //�޹���Ա��½
#define SUPERADMIN   1
#define NORMALADMIN  2

#define METER_ADDR  0x500

#define MIAN_SOFT_VERSION_ADDR		0x0320	//����������汾ҳ��ʼ��ַ 
#define NET_SET_ADDR                0X0600  //��������ҳ�����ʼ��ַ
#define FEE_ONE_SET_ADDR            0X03B7  //����һ����ҳ����ʼ��ַ
#define FEE_TWO_SET_ADDR            0X03D1  //���ʶ�����ҳ����ʼ��ַ
#define PARA_ONE_SET_ADDR           0X03FD  //��������һҳ����ʼ��ַ
#define PARA_TWO_SET_ADDR           0X0411  //��������һҳ����ʼ��ַ
#define CHARGE_INFO_DATA_ADDR       0X04B5  //Aǹ�����Ϣҳ��
#define TIME_SET_ADDR               0X4E3   //ʱ������ҳ��ʼ��ַ
//#define btn_AUTO_CRG_Addr           0X0016//���ܳ��
#define var_QR_Addr0                 0x0100  //��ҳ��ά���ַ  Aǹ 
#define var_QR_Addr1                 0x0101  //��ҳ��ά���ַ  Bǹ
#define var_MacSerialNum_Addr    0x0102  //�������ŵ�ַ
#define CHARGE_ICO_A_ADDR        0x04B1   //Aǹ��ICO��ַ

#define var_Caution_Addr    0x0032   //���ѡ��澯��Ϣ��ʾ����ʼ��Ϣ
#define var_Countdown_Addr  0x0063   //����ʱ��ʾ��ַ
#define var_Internet_Onling_Addr  0x0062   //����ͼ����ʾ��ַ

#define btn_TIME_CRG_Addr           0X0010//��ʱ����
#define btn_ENERGY_CRG_Addr         0X0011//���������
#define btn_MONEY_CRG_Addr          0X0012//�������
#define btn_Login_Check_Addr        0x0013 //�������

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
	CRG_MODE_AUTO				,		//�Զ����
	CRG_MODE_MONEY				,		//���ģʽ
	CRG_MODE_TIME				,		//ʱ��ģʽ
	CRG_MODE_ENERGE				,		//����ģʽ
}CRG_MODE;

typedef enum
{
    PAGE_ID_HOME                        = 0,                             //������ҳ
    PAGE_ID_MAIN_0                      = 1,                             //���д���a
    PAGE_ID_MAIN_1                      = 2,                             //���д���b
    PAGE_ID_CRGING_MSG_0                = 3,                             //�����Ϣa
    PAGE_ID_CRGING_MSG_1                = 4,                             //�����Ϣb
    PAGE_ID_START_ING                   = 5,                             //�������-ING  
    PAGE_ID_SELECT_MODE                 = 6,                             //ѡ����ģʽ
    
    PAGE_ID_SET_MODE_SUCCESS            = 12,                            //ģʽ�������
    PAGE_ID_ADMIN_LOGIN                 = 13,                            //����Ա��½���
    PAGE_ID_SYSTEM_MANAGE_HOME          = 15,                            //����ϵͳ������ҳ
    PAGE_ID_CURRENT_FAULT               = 16,                            //��ǰ����
    PAGE_ID_HISTORY_FAULT               = 17,                            //��ʷ����
    PAGE_ID_CRG_HISTORY_0               = 18,                            //����¼0
    PAGE_ID_CRG_HISTORY_1               = 19,                            //����¼1
    PAGE_ID_DEVICE_MSG_PG_INPUT         = 20,                            //�豸��Ϣ¼��
    PAGE_ID_METER_INFO                  = 21,                            //�����Ϣ
    PAGE_ID_OPT_RECORD               	= 22,                            //������¼
    PAGE_ID_NORMAL_PASSWORD_SET         = 23,                            //�û���������
    PAGE_ID_NET_SET                     = 24,                            //��������
    PAGE_ID_FEE_SET_1                   = 25,                            //��������1
    PAGE_ID_PARA_SET_1                  = 26,                            //��������1
    PAGE_ID_PARA_SET_2                  = 27,                            //��������2   
    PAGE_ID_FEE_SET_2                   = 28,                            //��������2
    
    PAGE_ID_BOOK_SET                    = 31,                            //ԤԼ����
    PAGE_ID_FAULT_INFO                  = 32,                            //������Ϣ����
    PAGE_ID_CONSUME_MSG                 = 35,                            //������Ϣ
    PAGE_ID_INSERT_GUN                  = 36,                            //��ǹ����
    PAGE_ID_NORMAL_SYSTEM_MANAGE_HOME   = 37,                            //��ͨ����Աϵͳ������ҳ
    PAGE_ID_PASSWORD_SET                = 38,                            //�������û������������
    PAGE_ID_SYSTEM_TIME_SET             = 39,                            //ʱ������
    PAGE_ID_NORMAL_DEVICE_INPUT         = 42,                            //��ͨ�û��豸��Ϣ����
    PAGE_ID_CONSUME_MSG_APP             = 48,                            //������Ϣ APP
    
}PAGE_ID;


typedef struct
{
    uint8_t   firstFlag;        //��һ�ν����־  
    uint8_t   startStatus;      //������ʱ��־   1Ϊ������ʱ   0Ϊ�ر�
    uint32_t  timeStamp;     //��ʱʱ���
}PAGE_DELAY;

typedef struct 
{
    uint8_t   commFlag;           // ͨѶ״̬        
    uint8_t	  commErrThreshold;   // ͨѶʧ����ֵ       
    uint32_t  commErrStartTime;   // ͨѶʧ�ܼ�ʱ��ʼʱ��   
    uint32_t  commErrCount;
	uint32_t  startTime;          // ���ݽ���
	uint16_t  deviceStatus;
    uint32_t  enterPageTime;
    uint16_t  setPageId;
    uint8_t   curCrgPort;   //��ǰ�����ĳ���
    uint8_t   curSetChannal;   //��ǰ���õĳ��ͨ��
    uint8_t   testSerNo[16];
}TouchData;

typedef enum
{
	e_IDLE,		    //����
    e_DATA_DEAL,    //���ݴ�����
	e_CHARGE,	    //�����
	e_BOOK,		    //ԤԼ	
	e_CRG_STOP, 	//���ֹͣ
    e_CRG_STOP_EXCEP ,  //�쳣ֹͣ
    e_CONSUME_OVER, //���ѽ���ɹ�
	e_CHARGE_ONLINE,//���߳��  ��ʾ״̬   
    e_START_CHARG_ING,//���������
    e_GIVE_UP_CHARGE,//�������
    e_UPDATA,//������
}E_ChargeStatus;

typedef struct
{
    uint8_t ModeSetFlag;    //ģʽ����Flag   0xff��ʾ������   0��û��
	uint8_t SelectMode; 	//�û�ѡ��ģʽ �� e_AUTO�Զ���磬1�Զ�����
    uint8_t BookStatus;    //ԤԼ״̬
	uint8_t ChargeStatus;  //����״̬ 
//	uint8_t ChargeMode;	   //���ģʽ �Զ�ģʽ ����ģʽ ʱ��ģʽ
	uint32_t Para;		   //������� ����ֵ
//    uint8_t ChargeType;   //��緽ʽ 1:���� 2:��ʱ
//    uint8_t BookGunPos;   //ԤԼǹ��
    uint8_t Booktime[2];  //ԤԼʱ��ֵ  [0] Ϊʱ   [1]Ϊ��
//    uint32_t OptTime;		//����ʱ��
//    uint8_t SecCountDown;  //s�뵹��ʱ
//    uint32_t PageSec;
  
}CHARGE_MODE_INFO;

typedef struct
{
    uint8_t LoginMode;   //1��������Ա    2Ϊ��ͨ����Ա
    uint8_t  FirstEnterPageFlag;//���ڱ�ǵ�½����ϵͳ�Ƿ��ǵ�һ�ε�½��ҳ
    uint8_t  ChargeFirstFlag;//���ڳ�����̵�һ�ν���ı�־
}SYSTEM_SET_PARA;


typedef enum
{
    INSERT_GUN,     //��ʾ������ǹ
    SWIP_CARD,      //��ʾˢ��
    READY_CHARGE,   //��ʾ���������
    CHARGEING,      //�������
    CHARGE_STOP,    //����ֹͣ��������
    CHARGE_STOP_EXCEP,//�쳣ֹͣ���
    END_CARD_NOT_MATCH,//����ƥ��
    SWIP_END_CARD,  //ˢ������
    CONSUM_INFO,    //������Ϣ
    TAKE_BACK_GUN,  //��ǹ
    AUTH_CARD,      //������֤��Ƭ
    BOOK_SUCCES,    //ԤԼˢ���ɹ�
    BOOK_CANCEL,    //�û�ȡ��ԤԼ
    CARD_NOT_ACCOUNT,//δ����
    CARD_ACCOUNT_LOCK, //�˻�����
    CARD_NO_MONEY,  //����
    CARD_ONLINE_ERR,  //δ֪����
    OFF_LINE,       //����
    TRIAL_TIME_END,//�����ڽ���
    RERTURN_IDLE,//�ص���ʼ״̬
    SET_TIME,      //�趨������ʱ��
    UPDATAING,      //������
    CHARGE_FAIL,    //����ʧ��
    CTRL_BAKE_LIGHT,//���Ʊ���
}LCD_STATE;


typedef enum
{
	OTHER_FUN_BUTTON			,		//������ʽ��ť
    LOGIN_ADMIN_BUTTON			,		//����Ա��½��ť
    SHITFT_A_B_DISPLAY  		,		//�л�AB�ڵ���ʾ   Ԥ��
}MAIN_BUTTON_NO;

typedef enum
{
	MODE_RETURN                 ,  		//ģʽѡ��ҳ���ذ�ť
	APPOINTMENT_BTN    			,		//ԤԼ��
	TIME_BTN            		,		//ʱ���
    ENERGE_BTN                  ,       //������
    MONEY_BTN                   ,       //����
}MODE_BUTTON_NO;

typedef enum
{
	LOGIN_RETURN                ,  		//���ذ�ť
	INPUT_PASSWORD    			,		//��������
}ADMIN_LOGIN_NO;

typedef enum
{
	SYSTEM_RETURN               ,  	//���ذ�ť
	RETURN_HOME         		,	//��������
    CUR_FAULT                   ,   //��ǰ����
    HISTORY_RECORD              ,   //��ʷ��¼
    CRG_RECORD                  ,   //����¼
    DEVICE_INFO                 ,   //�豸��Ϣ
    METER_INFO                  ,   //�����Ϣ
    CONFIG_SET                  ,   //��������
    PASSWORD_SET                ,   //��������
    OPT_RECORD                  ,   //������¼
}SYSTEM_MANAGE_NO;
typedef enum
{
	HISTORY_RETURN              ,  	//���ذ�ť
	HISTORY_UP           		,	//�Ϸ���ʷ��¼
    HISTORY_DOWN                ,   //�·���ʷ��¼
    SHIFT_BTN                   ,   //�����л���ť
}PAGE_HISTORY_FAULT_NO;

typedef enum
{
	FEE_RETURN                  ,  	//���ذ�ť
	PRE_PAGE            		,	//ǰһҳ
    NEXT_PAGE                   ,   //��һҳ
    RESTORE_FACTORY_BUTTON      ,   //�ָ���������
    DELETE_RECORD_DATA          ,   //ɾ����¼����        
}PAGE_FEE_NO;
typedef enum
{
	PARA_SET_RETURN                  ,  	//���ذ�ť
    TIME_CONFIRM_BUTTON  =  3             ,      //ʱ��ȷ�ϰ�ť
	
}PAGE_PARA_SET_NO;

void SetupTouchApp(osPriority ThreadPriority,uint8_t StackNum);
extern CHARGE_MODE_INFO Get_Charge_Mode_Info( uint8_t Ch);
osThreadId Get_Touch_Task_Id(void);
uint32_t Get_Page_ID(uint8_t Ch);




#endif
