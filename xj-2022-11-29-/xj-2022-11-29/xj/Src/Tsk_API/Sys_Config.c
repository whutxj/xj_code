

/******************************************************************************
* Includes
******************************************************************************/
#include "Sys_Config.h"
#include "Utility.h"
#include "DRV_SPIFLASH.h"
#include "string.h"
#include "CRC16.h"
#include "Task_SpiFlash.h"
#include "Task_API_SpiFlash.h"
#include "Tsk_Charge_Ctrl.h"
#include "stdio.h"
#include "math.h"
#include "stdlib.h"
#include "time.h"
#include "Update.h"
#include "lwip/sockets.h"
#include "Tsk_Measure.h"
#include "eth_comm.h"
#include "drv_time.h"
#include "Tsk_TouchDW.h"
#include "Tsk_Modbus.h"

#include "lwip.h"
#include "Tsk_API_Test.h"

#include "BSP_LED_Function.h"
#include "Drv_RN8029.h"

#include "Tsk_JWT.h"
//µ•œ‡µƒ»Ìº˛∞Ê±æ
const char SoftVer_Phase1[20] = "V1.30";

const char SoftVer_Phase3[20] = "V1.50";

char SoftVer[20];
char CompileTime[40] = {0};

/*****************************************************************************
* Macros Definitions (constant/Macros)
******************************************************************************/
#define SYS_PARAM_ADDR           0x0000
#define SYS_PARAM_ADDR_BACK_UP   0x1000
#define INIT_UDP_PORT            9000//≥ı ºªØ∂Àø⁄
/******************************************************************************
* Data Type Definitions
******************************************************************************/

/******************************************************************************
* Global Variable Definitions
******************************************************************************/
SYS_CONFIG_PARAM g_Sys_Config_Param;      //œµÕ≥≈‰÷√»´æ÷±‰¡ø
static SYS_CONFIG_PARAM g_Tmp_Config_Param; //œµÕ≥≈‰÷√æ≤Ã¨¡Ÿ ±±‰¡ø

static SYS_CONFIG_PARAM g_Sys_Config_Param_bk;   //œµÕ≥≈‰÷√æ≤Ã¨±∏∑›±‰¡ø
static int g_fd = 0;
static struct sockaddr_in server;
static struct sockaddr_in st_Sin;

static int g_dbg_port;


const static PARAM_INFO g_ParmaTbl[] =
{
    {"IP" ,           sizeof(g_Sys_Config_Param.LocalIp),   g_Sys_Config_Param.LocalIp,                                "ip"},
    {"Server" ,      sizeof(g_Sys_Config_Param.SerIp),       g_Sys_Config_Param.SerIp,                                   "str"},
    {"Port" ,      sizeof(g_Sys_Config_Param.Port),       &g_Sys_Config_Param.Port,                                     "int"},
    {"NetMask" ,      sizeof(g_Sys_Config_Param.NetMask),  g_Sys_Config_Param.NetMask,                                  "ip"},
    {"GetAway" ,      sizeof(g_Sys_Config_Param.GetAway),  g_Sys_Config_Param.GetAway,                                  "ip"},
    {"MAC" ,        sizeof(g_Sys_Config_Param.MAC),         g_Sys_Config_Param.MAC,                                     "hex"},
    {"DnsSer" ,        sizeof(g_Sys_Config_Param.DomainSer),         g_Sys_Config_Param.DomainSer,                      "ip"},
    {"OverVolt" ,   sizeof(g_Sys_Config_Param.OverVolt),  &g_Sys_Config_Param.OverVolt,                                "int"},
    {"Undervoltage" ,   sizeof(g_Sys_Config_Param.Undervoltage),  &g_Sys_Config_Param.Undervoltage,                     "int"},
    {"Price01" ,   2,                  &g_Sys_Config_Param.Price[0],                           "int"},
    {"Price02" ,   2,                  &g_Sys_Config_Param.Price[1],                           "int"},
    {"Price03" ,  2,                  &g_Sys_Config_Param.Price[2],                           "int"},
    {"Price04" ,   2,                  &g_Sys_Config_Param.Price[3],                           "int"},
    {"Price05" ,   2,                  &g_Sys_Config_Param.Price[4],                           "int"},
    {"Price06" ,   2,                  &g_Sys_Config_Param.Price[5],                           "int"},
    {"Price07" ,   2,                  &g_Sys_Config_Param.Price[6],                           "int"},
    {"Price08" ,   2,                  &g_Sys_Config_Param.Price[7],                           "int"},
    {"Price09" ,   2,                  &g_Sys_Config_Param.Price[8],                           "int"},
    {"Price10" ,   2,                  &g_Sys_Config_Param.Price[9],                           "int"},
    {"Price11" ,   2,                  &g_Sys_Config_Param.Price[10],                           "int"},
    {"Price12" ,   2,                  &g_Sys_Config_Param.Price[11],                           "int"},
    {"Price13" ,  2,                  &g_Sys_Config_Param.Price[12],                           "int"},
    {"Price14" ,   2,                  &g_Sys_Config_Param.Price[13],                           "int"},
    {"Price15" ,   2,                  &g_Sys_Config_Param.Price[14],                           "int"},
    {"Price16" ,   2,                  &g_Sys_Config_Param.Price[15],                           "int"},
    {"Price17" ,   2,                  &g_Sys_Config_Param.Price[16],                           "int"},
    {"Price18" ,   2,                  &g_Sys_Config_Param.Price[17],                           "int"},
    {"Price19" ,   2,                  &g_Sys_Config_Param.Price[18],                           "int"},
    {"Price20" ,   2,                  &g_Sys_Config_Param.Price[19],                           "int"},
    {"Price21" ,   2,                  &g_Sys_Config_Param.Price[20],                           "int"},
    {"Price22" ,   2,                  &g_Sys_Config_Param.Price[21],                           "int"},
    {"Price23" ,  2,                  &g_Sys_Config_Param.Price[22],                           "int"},
    {"Price24" ,   2,                  &g_Sys_Config_Param.Price[23],                           "int"},
    {"Price25" ,   2,                  &g_Sys_Config_Param.Price[24],                           "int"},
    {"Price26" ,   2,                  &g_Sys_Config_Param.Price[25],                           "int"},
    {"Price27" ,   2,                  &g_Sys_Config_Param.Price[26],                           "int"},
    {"Price28" ,   2,                  &g_Sys_Config_Param.Price[27],                           "int"},
    {"Price29" ,   2,                  &g_Sys_Config_Param.Price[28],                           "int"},
    {"Price30" ,   2,                  &g_Sys_Config_Param.Price[29],                           "int"},
    {"Price31" ,   2,                  &g_Sys_Config_Param.Price[30],                           "int"},
    {"Price32" ,   2,                  &g_Sys_Config_Param.Price[31],                           "int"},
    {"Price33" ,  2,                  &g_Sys_Config_Param.Price[32],                           "int"},
    {"Price34" ,   2,                  &g_Sys_Config_Param.Price[33],                           "int"},
    {"Price35" ,   2,                  &g_Sys_Config_Param.Price[34],                           "int"},
    {"Price36" ,   2,                  &g_Sys_Config_Param.Price[35],                           "int"},
    {"Price37" ,   2,                  &g_Sys_Config_Param.Price[36],                           "int"},
    {"Price38" ,   2,                  &g_Sys_Config_Param.Price[37],                           "int"},
    {"Price39" ,   2,                  &g_Sys_Config_Param.Price[38],                           "int"},
    {"Price40" ,   2,                  &g_Sys_Config_Param.Price[39],                           "int"},
    {"Price41" ,   2,                  &g_Sys_Config_Param.Price[40],                           "int"},
    {"Price42" ,   2,                  &g_Sys_Config_Param.Price[41],                           "int"},
    {"Price43" ,  2,                  &g_Sys_Config_Param.Price[42],                           "int"},
    {"Price44" ,   2,                  &g_Sys_Config_Param.Price[43],                           "int"},
    {"Price45" ,   2,                  &g_Sys_Config_Param.Price[44],                           "int"},
    {"Price46" ,   2,                  &g_Sys_Config_Param.Price[45],                           "int"},
    {"Price47" ,   2,                  &g_Sys_Config_Param.Price[46],                           "int"},
    {"Price48" ,   2,                  &g_Sys_Config_Param.Price[47],                           "int"},
    {"CrgServFee" ,   sizeof(g_Sys_Config_Param.CrgServFee),        &g_Sys_Config_Param.CrgServFee,                      "int"},
    {"FeeVersion" ,   sizeof(g_Sys_Config_Param.FeeVersion),        g_Sys_Config_Param.FeeVersion,                        "str"},
    {"GunLockEn" ,   sizeof(g_Sys_Config_Param.GunLockEn),            &g_Sys_Config_Param.GunLockEn,                           "int"},//?1°¡®¥®∫?°§????°ß
    {"phaseType" ,   sizeof(g_Sys_Config_Param.phaseType),            &g_Sys_Config_Param.phaseType,                           "int"},//¶Ã£§?®§/®®y?®§
    {"ScreenEn" ,   sizeof(g_Sys_Config_Param.ScreenEn),        &g_Sys_Config_Param.ScreenEn,                                "int"},//?®¢??®∫1?®π;
    {"DoubelGunEn" ,   sizeof(g_Sys_Config_Param.DoubelGunEn),        &g_Sys_Config_Param.DoubelGunEn,                        "int"},//0¶Ã£§?11???1
    {"AutoIp" ,   sizeof(g_Sys_Config_Param.AutoIP),        &g_Sys_Config_Param.AutoIP,                               "int"},//°¡??°•IP
    {"CurrentType" ,   sizeof(g_Sys_Config_Param.CurrentType),        &g_Sys_Config_Param.CurrentType,                        "int"},//°¡??°•IP
    {"WorkMod" ,   sizeof(g_Sys_Config_Param.WorkMod),        &g_Sys_Config_Param.WorkMod,                                  "int"},//¶Ã?®¢°¬®∫?3?®§®§D®™
    {"DeviceType" ,   sizeof(g_Sys_Config_Param.DeviceType),        &g_Sys_Config_Param.DeviceType,                        "int"},//°¡???°¡®¶®™?®§®§D®™ 0 °‰®Æ?®≤  1?°¬?®≤
    {"BiasEn" ,   sizeof(g_Sys_Config_Param.BiasEn),        &g_Sys_Config_Param.BiasEn,                                     "int"},//??D°¿®∫1?®π

    {"CommType" ,   sizeof(g_Sys_Config_Param.CommType),            &g_Sys_Config_Param.CommType,                           "int"},
    {"MeterType" ,   sizeof(g_Sys_Config_Param.MeterType),          &g_Sys_Config_Param.MeterType,                                "int"},
    {"DtuType" ,   sizeof(g_Sys_Config_Param.DtuType),              &g_Sys_Config_Param.DtuType,                                "int"},
    {"HardVer" ,   sizeof(g_Sys_Config_Param.HardVer),              g_Sys_Config_Param.HardVer,                                "str"},
    {"SoftVer" ,   20,                                              (void*)SoftVer,                                            "str"},
    {"DevName" ,   sizeof(g_Sys_Config_Param.DevName),              g_Sys_Config_Param.DevName,                                "str"},
    {"FeeVersion" ,   sizeof(g_Sys_Config_Param.FeeVersion),        g_Sys_Config_Param.FeeVersion,                       "str"},

    {"QREN" ,   sizeof(g_Sys_Config_Param.QRCodeEN),        &g_Sys_Config_Param.QRCodeEN,                                "int"},
    {"QRCode" ,   sizeof(g_Sys_Config_Param.QRCode),        g_Sys_Config_Param.QRCode,                                      "str"},

    {"Serial" ,   sizeof(g_Sys_Config_Param.Serial),                g_Sys_Config_Param.Serial,                                   "str"},
    {"SlaveNum" ,   sizeof(g_Sys_Config_Param.SlaveNum),            &g_Sys_Config_Param.SlaveNum,                                   "int"},
    {"SlaveAddr" ,   sizeof(g_Sys_Config_Param.SlaveAddr),          &g_Sys_Config_Param.SlaveAddr,                                   "int"},
    {"CardReaderType" ,   sizeof(g_Sys_Config_Param.CardReaderType),  &g_Sys_Config_Param.CardReaderType,                     "int"},
    {"IcCardKey" ,   sizeof(g_Sys_Config_Param.IcCardKey),          g_Sys_Config_Param.IcCardKey,                     "str"},
    {"TrialTime" ,   sizeof(g_Sys_Config_Param.TrialTime),          g_Sys_Config_Param.TrialTime,                     "hex"},
    {"Date" ,       20,                                             NULL,                                               "time"},

    {"CompileTime" , sizeof(CompileTime),                                             (void*)CompileTime,                            "str"},
    {"TCP_ud_flag" ,   sizeof(g_Sys_Config_Param.TCP_ud_flag),            &g_Sys_Config_Param.TCP_ud_flag,                     "int"},
    {"TCP_adr" ,   sizeof(g_Sys_Config_Param.TCP_ud_stop_adr),            &g_Sys_Config_Param.TCP_ud_stop_adr,                 "int"},
    {"TCP_number" ,   sizeof(g_Sys_Config_Param.TCP_ud_stop_serial_number),            &g_Sys_Config_Param.TCP_ud_stop_serial_number,       "int"},
    {"restart_num" ,   sizeof(g_Sys_Config_Param.restart_num),            &g_Sys_Config_Param.restart_num,                     "int"},
    {"restart_time" ,   sizeof(g_Sys_Config_Param.restart_time),            &g_Sys_Config_Param.restart_time,                     "int"},

};


const static opt_record st_opt_record[]=
{
     {OPT_POWER_ON,"int",NULL,0}
};
const static opt_record st_opt_record_bak[] =
{
	   {OPT_POWER_ON,"int",NULL,0}
};

static int getAllparam(char *str,void *p);
static int  do_help(char *str,void *p);
void eth_dbg_init(uint32 dbg_port);

int eth_udp_out_put(int fd,int port,uint8 *buf,int len);
int eth_udp_dbg(uint8 *buf,int len);
void check_power_on_record(void);

/*------------------------------------------------------------
≤Ÿ◊˜ƒøµƒ£∫ µ˜ ‘ ‰≥ˆ≥ı ºªØ
◊˜    ’ﬂ£∫
∫Ø ˝≤Œ ˝£∫dbg_port  ‰≥ˆ π”√µƒ∂Àø⁄∫≈
∑µªÿ÷µ£∫
            Œﬁ
∞Ê   ±æ:    V1.0
º«   ¬º:
------------------------------------------------------------*/

void eth_dbg_init(uint32 dbg_port)
{
    g_dbg_port = dbg_port;
}

/*------------------------------------------------------------
≤Ÿ◊˜ƒøµƒ£∫  ‰≥ˆ
◊˜    ’ﬂ£∫
∫Ø ˝≤Œ ˝£∫fd Œƒº˛√Ë ˆ
         port  ‰≥ˆµΩ÷∆∂®∂Àø⁄

         buf  ‰≥ˆ–≈œ¢
         len  buf µƒ≥§∂»
∑µªÿ÷µ£∫
            Œﬁ
∞Ê   ±æ:    V1.0
º«   ¬º:
------------------------------------------------------------*/

/*  JWT  */

int eth_udp_out_put(int fd,int port,uint8 *buf,int len)
{
    struct sockaddr_in s_add;
    memset(&s_add,0,sizeof(struct sockaddr_in));
    s_add.sin_family=AF_INET;
    s_add.sin_addr.s_addr= inet_addr("255.255.255.255");
    s_add.sin_port=htons(port);
    sendto(fd,buf,len,SO_BROADCAST,(struct sockaddr *)&s_add,sizeof(s_add));

    return 0;
}

/*------------------------------------------------------------
≤Ÿ◊˜ƒøµƒ£∫ eth_udp_dbg “‘Ã´Õ¯ ‰≥ˆ
◊˜    ’ﬂ£∫
∫Ø ˝≤Œ ˝£∫fd Œƒº˛√Ë ˆ
         port  ‰≥ˆµΩ÷∆∂®∂Àø⁄

         buf  ‰≥ˆ–≈œ¢
         len  buf µƒ≥§∂»
∑µªÿ÷µ£∫
            Œﬁ
∞Ê   ±æ:    V1.0
º«   ¬º:
------------------------------------------------------------*/

int eth_udp_dbg(uint8 *buf,int len)
{
    eth_udp_out_put(g_fd,g_dbg_port,buf,len);
    return len;
}

/*------------------------------------------------------------
≤Ÿ◊˜ƒøµƒ£∫ ≥ı ºªØ ≤Œ ˝≈‰÷√ π”√µƒÕ¯¬ÁΩ”ø⁄
◊˜    ’ﬂ£∫
∫Ø ˝≤Œ ˝£∫Œﬁ
            Œﬁ
∞Ê   ±æ:    V1.0
º«   ¬º:
------------------------------------------------------------*/

uint8 param_config_init(void)
{
    uint8 tmpBuf[128] = {0};
    g_fd = eth_create_socket(1);
    memset(&st_Sin,0,sizeof(st_Sin));

    st_Sin.sin_family=AF_INET;
    st_Sin.sin_addr.s_addr=htonl(INADDR_ANY);
    st_Sin.sin_port=htons(INIT_UDP_PORT);


    if(g_fd < 0)
    {
        dbg_printf(1,"creat socket err");
        return 0;
    }

    bind(g_fd,(struct sockaddr *)&st_Sin,sizeof(st_Sin));

	sprintf((void*)tmpBuf,"My Default ip = %d.%d.%d.%d;\n",g_Sys_Config_Param.LocalIp[0],g_Sys_Config_Param.LocalIp[1],g_Sys_Config_Param.LocalIp[2],g_Sys_Config_Param.LocalIp[3]);

    eth_udp_out_put(g_fd,INIT_UDP_PORT,tmpBuf,strlen((char*)tmpBuf));

    return 1;
}


/*

	ÂÅ•ÁΩëÁßëÊäÄ -- JWT

					*/

uint32 param_config_rcv(char *buf)
{

    int len = sizeof(server);
    int rcvLen = 0;
    rcvLen = recvfrom(g_fd,buf,128,0,(struct sockaddr*)&server,(socklen_t*)&len);
    if (rcvLen > 0)
    {
        return rcvLen;
    }
    return 0;

}

uint32 param_config_send(char *buf,int lenth)
{
    int len = sizeof(server);
    int rcvLen = 0;

    sendto(g_fd,buf,lenth,0,(struct sockaddr*)&server,len);
    if (rcvLen > 0)
    {
        dbg_printf(1,"send %d ok",rcvLen);
        return rcvLen;
    }
    return 0;
}

static int resonse_send(char *buf,int len,void *p)
{
    COMM_TYPE *stCom = p;

    if(stCom->link_type == 0)
    {
        //param_config_send(buf,len);

        dbg_printf(1,"%s",buf);
    }
    else
    {
        //usart_write(COM2,buf,len,100);
    }

    return 0;

}

static int reboot_System(char *str,void *p)
{
	dbg_printf(1,"reboot...\n");

    HAL_NVIC_SystemReset();

    return 0;
}


//void Updata_Probe(void)
//{
//    if(updata)
//    {
//        Indication_Updata();
//        dbg_printf(1,"reboot...\n");
//        NVIC_SystemReset();
//    }
//
//}

static int getAlarmRecord(char *str,void *p)
{
   uint8 tmp[64] = {0};
   int index = 0;

   int Getindex = 0;

   if(str)
   {
       Getindex = atoi(str);
   }

   ALARM_DATA stAlarmData;//æ… ˝æ›

   index = SPIFLASH_Method4_Get_RecordCnt(TYPE_ALARMLOG);

   if(Getindex > index)
   {
       Getindex = index;
   }

   if(index == 0)
   {
       dbg_printf(1,"getAlarmRecord no data %d\n");
       return 0;
   }

   if(Getindex > 0)
   {
       Getindex -= 1;
   }

   dbg_printf(1,"getAlarmRecord index %d\n",Getindex);

   if(Getindex >= 0)
   {
       SPIFLASH_Method4_Get_Record(TYPE_ALARMLOG,Getindex,(uint8*)&stAlarmData);
       shift_time_data(tmp,stAlarmData.StartTime);
       dbg_printf(1,"Alarm start time %s",tmp);
       dbg_printf(1,"Alarm code %d",stAlarmData.AlarmCode) ;
       shift_time_data(tmp,stAlarmData.EndTime);
       dbg_printf(1,"Alarm EndTime  %s",tmp);
       dbg_printf(1,"Alarm Recover  %d",stAlarmData.Recover);
   }

   return 1;

}

static int getChargeRecord(char *str,void *p)
{
   uint8 tmp[64] = {0};
   int index = 0;

   int Getindex = 0;

   if(str)
   {
       Getindex = atoi(str);
   }

   ChargeRecord ChargeRecordDataOld;//æ… ˝æ›

   index = SPIFLASH_Method4_Get_RecordCnt(TYPE_CHARGELOG);

   if(Getindex > index)
   {
       Getindex = index;
   }
   if(index == 0)
   {
       dbg_printf(1,"getChargeRecord no data %d\n");

       return 0;
   }
   if(Getindex > 0)
   {
       Getindex -= 1;
   }

   dbg_printf(1,"getChargeRecord index %d\n",Getindex);

   if(Getindex >= 0)
   {
       SPIFLASH_Method4_Get_Record(TYPE_CHARGELOG,Getindex,(uint8*)&ChargeRecordDataOld);
       shift_time_data(tmp,ChargeRecordDataOld.StartTime);
       dbg_printf(1,"Record start time %s",tmp);
       shift_hex_data(tmp,ChargeRecordDataOld.CardNo,16);
       dbg_printf(1,"Record start CardNo %s",tmp) ;
       dbg_printf(1,"Record ChargeType %d",ChargeRecordDataOld.ChargeType);
       dbg_printf(1,"Record ChargeMode %d",ChargeRecordDataOld.ChargeMode);
       dbg_printf(1,"Record StartCardType %d",ChargeRecordDataOld.StartCardType);
       dbg_printf(1,"Record StartEnerge %d",ChargeRecordDataOld.StartEnerge);
       dbg_printf(1,"Record EndEnerge %d",ChargeRecordDataOld.EndEnerge);
       dbg_printf(1,"Record MaxVolt %d",ChargeRecordDataOld.MaxVolt);
       dbg_printf(1,"Record ChargeMoney %d",ChargeRecordDataOld.totalFee);
       dbg_printf(1,"Record StopReason %d",ChargeRecordDataOld.StopReason);
       shift_time_data(tmp,ChargeRecordDataOld.EndTime);
       dbg_printf(1,"Record EndTime  %s",tmp);
       dbg_printf(1,"Record PayFlag %d",ChargeRecordDataOld.PayFlag);
       dbg_printf(1,"Record Report %d",ChargeRecordDataOld.Report);
   }
   return 1;
}


/*------------------------------------------------------------
≤Ÿ◊˜ƒøµƒ£∫ ªÒ»°≤Œ ˝”¶¥
◊˜    ’ﬂ£∫
∫Ø ˝≤Œ ˝£∫name ≤Œ ˝√˚◊÷
        val ”¶¥Ω·π˚◊÷∑˚¥Æ
        *p ø…±‰≤Œ ˝
∑µªÿ÷µ£∫
            Œﬁ
∞Ê   ±æ:    V1.0
º«   ¬º:
------------------------------------------------------------*/

static uint8 getParamResponse(char *name,char *val,void *p)
{
    char send[64] = {0};

    sprintf(send,"get_param %s=%s\n",name,val);
    resonse_send(send,strlen(send),p);

    return 0;

}
static int getParam(char *str,void *p)
{
    char name[12] = {0};
    char val[200] = {0};

    uint8 ucVal[4] = {0};
    int i = 0;
    int tmpVal = 0;
    sscanf (str, "%[^\r]\\r%[^\n]", name,val);

    for(i = 0; i <sizeof(g_ParmaTbl)/sizeof(g_ParmaTbl[0]);i++)
    {
        if(strstr(name,g_ParmaTbl[i].name))
        {
            if(strstr(g_ParmaTbl[i].fmt ,"ip"))
            {
                memcpy((uint8*)ucVal,(uint8*)g_ParmaTbl[i].val,g_ParmaTbl[i].lenth);
                sprintf(val,"%d.%d.%d.%d",ucVal[0],ucVal[1],ucVal[2],ucVal[3]);
                getParamResponse(name,val,p);
            }
            else if(strstr(g_ParmaTbl[i].fmt ,"int"))
            {
                memcpy((uint8*)&tmpVal,(uint8*)g_ParmaTbl[i].val,g_ParmaTbl[i].lenth);
                sprintf(val,"%d",tmpVal);
                getParamResponse(name,val,p);
            }
            else if(strstr(g_ParmaTbl[i].fmt ,"str"))
            {

                memcpy((uint8*)val,(uint8*)g_ParmaTbl[i].val,g_ParmaTbl[i].lenth);
                getParamResponse(name,val,p);
            }
            else if(strstr(g_ParmaTbl[i].fmt ,"hex"))
            {
                int j = 0;
                uint8 buf[100] = {0};
                int index = 0;
                memcpy((uint8*)buf,(uint8*)g_ParmaTbl[i].val,g_ParmaTbl[i].lenth);
                for(j = 0; j < g_ParmaTbl[i].lenth;j++)
                {
                    sprintf(val+index,"%.2X",(uint8*)buf[j]);
                    index +=2;
                }
                getParamResponse(name,val,p);
            }
            else if(strstr(g_ParmaTbl[i].fmt ,"time"))
            {
                struct tm st_time;;
                struct tm *st_tmp;
                time_t  tim = time(NULL);
                st_tmp = gmtime((const time_t *)&tim);
                st_time = *st_tmp;
                sprintf((char *)val,"%d-%.2d-%.2d %.2d:%.2d:%.2d",st_time.tm_year + 1900,st_time.tm_mon + 1,st_time.tm_mday,
                    st_time.tm_hour,st_time.tm_min,st_time.tm_sec);
                getParamResponse(name,val,p);
            }
            return 1;
        }

    }

    getParamResponse(name,0,p);

    return 0;

}


void eth_param_cfg_proc(void)
{
    char rcv_buf[128] = {0};
    COMM_TYPE comm;
    int len = 0;

    comm.fd = g_fd;
    comm.link_type = LAN;
    len = param_config_rcv(rcv_buf);

    if(len)
    {
        Parse_Line(&comm,rcv_buf,len);
    }
}

static int testFun(char *str,void *p)
{

    dbg_printf(1,"start test\r\n",sizeof("start test\r\n"),p);


    osThreadTerminate(Get_Charge_Ctrl_Task_Id());
//    osThreadTerminate(Get_Touch_Task_Id());
//    osThreadTerminate(Get_Modbus_Task_Id());
//    osThreadTerminate(Get_Measrue_Task_ID());
////    osThreadTerminate(Get_Clod_Task_Id());
//    BSP_Buzzer_off();

    LED_Function(YALLOW_LED1,LED_Off);
    LED_Function(RED_LED1,LED_Off);
    LED_Function(GRERN_LED1,LED_Off);
//    LED_Function(RED_LED2,LED_Off);
//    LED_Function(GRERN_LED2,LED_Off);
//    LED_Function(YALLOW_LED2,LED_Off);
    test_help();

    while(1)
    {
        char rcv_buf[128] = {0};
        COMM_TYPE comm;
        int len = 0;

        comm.fd = g_fd;
        comm.link_type = LAN;
        len = param_config_rcv(rcv_buf);

        if(len)
        {
            testFunProc(rcv_buf,(void*)&comm);
        }
        osDelay(100);

        BSP_WatchDog_Refresh();//Œππ∑
    }
}


static uint8 setParamResponse(int val,void *p)
{
    char send[32] = {0};
    if(val  > 0)
        val = 1;
    sprintf(send,"set_param %d",val);

    resonse_send(send,strlen(send),p);
    return 0;
}
/*------------------------------------------------------------
≤Ÿ◊˜ƒøµƒ£∫ …Ë÷√≤Œ ˝ ¥¶¿Ì √¸¡Ó
◊˜    ’ﬂ£∫
∫Ø ˝≤Œ ˝£∫
        str ∂¡»Îµƒ◊÷∑˚¥Æ
        *p ø…±‰≤Œ ˝
∑µªÿ÷µ£∫
            Œﬁ
∞Ê   ±æ:    V1.0
º«   ¬º:
------------------------------------------------------------*/

static int setParam(char *src,void *p)
{

    char name[20] ={0};
    char val[100] = {0};
    char tmp[6][10] = {0};
    uint8 ChVal[6] = {0};

    int i = 0;

    sscanf (src, "%[^ ] %[^\n]", name,val);

    for(i = 0; i <sizeof(g_ParmaTbl)/sizeof(g_ParmaTbl[0]);i++)
    {
        if(strcmp(name,g_ParmaTbl[i].name) == 0)
        {
            if(strstr(g_ParmaTbl[i].fmt ,"ip"))
            {
                sscanf(val,"%[^.].%[^.].%[^.].%[^.]",tmp[0],tmp[1],tmp[2],tmp[3]);

                ChVal[0] = atoi(tmp[0]);
                ChVal[1] = atoi(tmp[1]);
                ChVal[2] = atoi(tmp[2]);
                ChVal[3] = atoi(tmp[3]);
                memcpy(g_ParmaTbl[i].val,(uint8*)ChVal,g_ParmaTbl[i].lenth);

                setParamResponse(1,p);
            }
            else if(strstr(g_ParmaTbl[i].fmt ,"int"))
            {
                int intVal[1] = {0};
                intVal[0] = atoi(val);
                memcpy(g_ParmaTbl[i].val,(uint8*)intVal,g_ParmaTbl[i].lenth);
                setParamResponse(1,p);
            }
            else if(strstr(g_ParmaTbl[i].fmt ,"str"))
            {

                memcpy((uint8*)g_ParmaTbl[i].val,val,g_ParmaTbl[i].lenth);
                setParamResponse(1,p);
            }
            else if(strstr(g_ParmaTbl[i].fmt ,"hex"))
            {
                char buf[100] = {0};
                uint8 ret = asc2hex((uint8*)buf,(uint8*)val,g_ParmaTbl[i].lenth * 2);
                if(ret)
                {
                    memcpy((uint8*)g_ParmaTbl[i].val,buf,g_ParmaTbl[i].lenth);
                }

                setParamResponse(ret,p);
            }
            else if(strstr(g_ParmaTbl[i].fmt ,"time"))
            {//2016-02-25 12:12:12:12

                uint8 ret = 0;
                struct tm TimeData;
                ret = sscanf(val,"%[^-]-%[^-]-%[^ ] %[^:]:%[^:]:%[^\\n]",tmp[0],tmp[1],tmp[2],tmp[3],tmp[4],tmp[5]);
                if(ret == 6)
                {
                    uint32 sec = 0;
                    struct tm *stTime;
                    TimeData.tm_year = atoi(tmp[0]) - 1900;
                    TimeData.tm_mon = atoi(tmp[1]) - 1;
                    TimeData.tm_mday = atoi(tmp[2]);
                    TimeData.tm_hour = atoi(tmp[3]);
                    TimeData.tm_min = atoi(tmp[4]);
                    TimeData.tm_sec = atoi(tmp[5]);
                    //
                    sec = mktime(&TimeData);
                    sec -= 8 * 3600;
                    stTime = localtime(&sec);
                    drv_set_sys_time(*stTime);
                }
                else
                {
                    ret = 0;
                }

                setParamResponse(ret,p);
            }
            else if(strstr(g_ParmaTbl[i].fmt ,"bcd"))
            {//2016-02-25 12:12:12:12
                uint8 buf[53] = {0};
                uint8 bcd[20] = {0};
                uint8 ret = asc2hex((uint8*)buf,(uint8*)val,g_ParmaTbl[i].lenth * 2);
                if(ret)
                {
                    int i = 0;
                    for(i = 0;i < g_ParmaTbl[i].lenth;i++)
                    {
                        bcd[i] = HexToBcd(buf[i]);

                    }
                    memcpy((uint8*)g_ParmaTbl[i].val,bcd,g_ParmaTbl[i].lenth);
                }

                setParamResponse(ret,p);
            }

            API_Save_Sys_Param_Direct();

            return 1;
        }

    }

    setParamResponse(0,p);

    return 0;
}

/*------------------------------------------------------------
≤Ÿ◊˜ƒøµƒ£∫  …˝º∂ ¥¶¿Ì √¸¡Ó
◊˜    ’ﬂ£∫
∫Ø ˝≤Œ ˝£∫
        str ∂¡»Îµƒ◊÷∑˚¥Æ
        *p ø…±‰≤Œ ˝
∑µªÿ÷µ£∫
            Œﬁ
∞Ê   ±æ:    V1.0
º«   ¬º:
------------------------------------------------------------*/

static int updataFun(char *str,void *p)
{
    int ret = 0;
    UpdateInfo stUpdateInfo = {0};
    char USR[12] = {0};
    char PASS[12] = {0};
    char val[20] = {0};
    char file[32] = {0};
    char *fileStr;
    sscanf (str, "%[^ ] %[^ ] %[^ ]",USR,PASS,val);

    fileStr = strstr(str,val);

    if(fileStr)
    {
        sscanf (fileStr, "%[^ ] %[^ ]",val,file);
    }

//    memcpy(stUpdateInfo.ftpFile,file,strlen(file));

//    sscanf (str, "%[^ ] %[^ ] %[^\n]", USR,PASS,val);

//    memcpy(stUpdateInfo.ftpFile,"/program/EVD_app.bin",strlen("/program/EVD_app.bin"));

    memcpy(stUpdateInfo.ftpPass,PASS,strlen(PASS));
    memcpy(stUpdateInfo.ftpSer,val,strlen(val));
    stUpdateInfo.ftpPort = 21;
    memcpy(stUpdateInfo.ftpUsr,USR,strlen(USR));
    memcpy(stUpdateInfo.ftpFile,file,strlen(file));

    stUpdateInfo.UpdateMode = FTP_MODE;

    Notice_Updata_Info(&stUpdateInfo);

    dbg_printf(1,"rcv updata\r");

    return ret;
}
/*------------------------------------------------------------
≤Ÿ◊˜ƒøµƒ£∫  start_Charge √¸¡Ó
◊˜    ’ﬂ£∫
∫Ø ˝≤Œ ˝£∫
        str ∂¡»Îµƒ◊÷∑˚¥Æ
        *p ø…±‰≤Œ ˝
∑µªÿ÷µ£∫
            Œﬁ
∞Ê   ±æ:    V1.0
º«   ¬º:
------------------------------------------------------------*/
static int start_Charge(char *str,void *p)
{
//    int flag = 0;
    UsrAccount stUsrAccount = {0};
    stUsrAccount.Money = 9999;//for test
    extern RecordData g_RecordData;
//    if(str)
//    {
//        flag = atoi(str);
//    }
//    if(flag == 0)
//    {
        Ctrl_power_on(0,&stUsrAccount);

        //2020.12.28
        //g_RecordData.crgType = 0x01;
        //g_RecordData.payMoney = 1;
//    }
//    else
//    {
//       //Ctrl_Continu_Charge(0);
//    }

    dbg_printf(1,"start charge ok");

    return 0;
}

/*------------------------------------------------------------
≤Ÿ◊˜ƒøµƒ£∫  stop_Charge √¸¡Ó
◊˜    ’ﬂ£∫
∫Ø ˝≤Œ ˝£∫
        str ∂¡»Îµƒ◊÷∑˚¥Æ
        *p ø…±‰≤Œ ˝
∑µªÿ÷µ£∫
            Œﬁ
∞Ê   ±æ:    V1.0
º«   ¬º:
------------------------------------------------------------*/
static int stop_Charge(char *str,void *p)
{

    Ctrl_power_off(STOP_REASON_MANNAL_STOP,0);

    dbg_printf(1,"stop charge ok");

    return 0;
}

/*------------------------------------------------------------
≤Ÿ◊˜ƒøµƒ£∫  erase_flash √¸¡Ó  ≤¡≥˝flash ’˚øÈ
◊˜    ’ﬂ£∫
∫Ø ˝≤Œ ˝£∫
        str ∂¡»Îµƒ◊÷∑˚¥Æ
        *p ø…±‰≤Œ ˝
∑µªÿ÷µ£∫
            Œﬁ
∞Ê   ±æ:    V1.0
º«   ¬º:
------------------------------------------------------------*/
static int erase_flash(char *str,void *p)
{
    int addr = 0x00;

    osThreadTerminate(Get_Charge_Ctrl_Task_Id());
//    if(Get_Protocol_Type() == HP_P)
//    {
//        osThreadTerminate(Get_Touch_Task_Id());
//        osThreadTerminate(Get_Clod_Task_Id());
//    }
//
//    osThreadTerminate(Get_Modbus_Task_Id());
//    osThreadTerminate(Get_Measrue_Task_ID());

    for(addr = 0; addr < 1024 * 1024;addr += 4096)
    {
        BSP_SPIFLASH_SectorErase(addr);
    }

    dbg_printf(1,"erase_flash ok");

    NVIC_SystemReset();

   	return 0;
}


/*------------------------------------------------------------
≤Ÿ◊˜ƒøµƒ£∫  erase_flash √¸¡Ó  ≤¡≥˝flash ’˚øÈ
◊˜    ’ﬂ£∫
∫Ø ˝≤Œ ˝£∫
        str ∂¡»Îµƒ◊÷∑˚¥Æ
        *p ø…±‰≤Œ ˝
∑µªÿ÷µ£∫
            Œﬁ
∞Ê   ±æ:    V1.0
º«   ¬º:
------------------------------------------------------------*/
static int save_to_factory_param(char *str,void *p)
{

    Save_To_Factory_Param();
    dbg_printf(1,"save_to_factory ok");

   	return 0;
}
/*------------------------------------------------------------
≤Ÿ◊˜ƒøµƒ£∫  erase_flash √¸¡Ó  ≤¡≥˝flash ’˚øÈ
◊˜    ’ﬂ£∫
∫Ø ˝≤Œ ˝£∫
        str ∂¡»Îµƒ◊÷∑˚¥Æ
        *p ø…±‰≤Œ ˝
∑µªÿ÷µ£∫
            Œﬁ
∞Ê   ±æ:    V1.0
º«   ¬º:
------------------------------------------------------------*/
static int restory_factory_param(char *str,void *p)
{

    Restore_Factory_Param();

    dbg_printf(1,"restory_factory seting ok");

   	return 0;
}

static int Adjust_Meter_Param(char *str,void *p)
{

    char U[12] = {0};
    char I[20] = {0};
    char P[32] = {0};
    uint32 buf[3] = {0};
    uint8 type = Get_MeterType();
    //type = TYPE_RN8209;

    sscanf (str, "%[^ ] %[^ ] %[^ ]",U,I,P);

    buf[0] = atoi(U);
    buf[1] = atoi(I);
    buf[2] = (buf[0] * buf[1])/100;

    {
        RN8209_Measure_Adj((uint8*)buf);
    }

    dbg_printf(1,"Adjust_Meter_Param OK");
    return 0;
}


/*------------------------------------------------------------
≤Ÿ◊˜ƒøµƒ£∫  Delet_Alarm_Data √¸¡Ó  …æ≥˝∏ÊæØº«¬º
◊˜    ’ﬂ£∫
∫Ø ˝≤Œ ˝£∫
        str ∂¡»Îµƒ◊÷∑˚¥Æ
        *p ø…±‰≤Œ ˝
∑µªÿ÷µ£∫
            Œﬁ
∞Ê   ±æ:    V1.0
º«   ¬º:
------------------------------------------------------------*/
static int Delet_Alarm_Data(char *str,void *p)
{

    SPIFLASH_Data_Erase(SPIFLASH_METHOD_MULTIRECORD_OVERWRITE,TYPE_ALARMLOG);
    dbg_printf(1,"del_alarm_data ok");

    NVIC_SystemReset();

    return 0;
}


static int Reset_sys_Param(char *str,void *p)
{

    SPIFLASH_Data_Erase(SPIFLASH_METHOD_SINGLERECORD,TYPE_SYSPARA);

    dbg_printf(1,"del_alarm_data ok");

    NVIC_SystemReset();

    return 0;

}


/*------------------------------------------------------------
≤Ÿ◊˜ƒøµƒ£∫  Delet_Charge_Data √¸¡Ó  …æ≥˝≥‰µÁº«¬º
◊˜    ’ﬂ£∫
∫Ø ˝≤Œ ˝£∫
        str ∂¡»Îµƒ◊÷∑˚¥Æ
        *p ø…±‰≤Œ ˝
∑µªÿ÷µ£∫
            Œﬁ
∞Ê   ±æ:    V1.0
º«   ¬º:
------------------------------------------------------------*/
static int Delet_Charge_Data(char *str,void *p)
{
    SPIFLASH_Data_Erase(SPIFLASH_METHOD_MULTIRECORD_OVERWRITE,TYPE_CHARGELOG);

    dbg_printf(1,"del_Charge_data ok");

    NVIC_SystemReset();

    return 0;

}

/*------------------------------------------------------------
≤Ÿ◊˜ƒøµƒ£∫  Get_Run_Info √¸¡Ó
◊˜    ’ﬂ£∫
∫Ø ˝≤Œ ˝£∫
        str ∂¡»Îµƒ◊÷∑˚¥Æ
        *p ø…±‰≤Œ ˝
∑µªÿ÷µ£∫
            Œﬁ
∞Ê   ±æ:    V1.0
º«   ¬º:
------------------------------------------------------------*/
static int Get_Run_Info(char *str,void *p)
{

    char Str[48] = {0};
    int iVal = 0;
    MEASURE_DATA  Meter = Get_Measrue_Data(0);
    uint8 sys = Get_Charge_Ctrl_State(0);
    Get_Auto_Ip(Str);
    iVal = Get_CP_Sample(0);

    dbg_printf(1,"get_run_info IP = %s",Str);

    dbg_printf(1,"get_run_info CP = %d.%2.2d",iVal/100,iVal%100);

    iVal = Meter.VolageA;

    dbg_printf(1,"get_run_info V = %d.%d",iVal/10,iVal%10);

    iVal = Meter.CurrentA;

    dbg_printf(1,"get_run_info A = %d.%2.2d",iVal/100,iVal%100);

    iVal = Meter.Totol_Energy;

    dbg_printf(1,"get_run_info E = %d.%2.2d",iVal/100,iVal%100);

    if(Get_FaultBit(0))
    {
        dbg_printf(1,"get_run_info SYS = warning");
    }
    if(sys == STATE_CHARGEING)
    {
        dbg_printf(1,"get_run_info SYS = charging");
    }
    else if(sys == STATE_END_CHARGE)
    {
        dbg_printf(1,"get_run_info SYS = finished");
    }
    else if(sys == STATE_BOOK)
    {
        dbg_printf(1,"get_run_info SYS = book");
    }
    else
    {
        dbg_printf(1,"get_run_info SYS = idle");
    }
    In_put_test(p);

    return 0;
}

/*------------------------------------------------------------
≤Ÿ◊˜ƒøµƒ£∫  Export_price √¸¡Ó
◊˜    ’ﬂ£∫
∫Ø ˝≤Œ ˝£∫
        str ∂¡»Îµƒ◊÷∑˚¥Æ
        *p ø…±‰≤Œ ˝
∑µªÿ÷µ£∫
            Œﬁ
∞Ê   ±æ:    V1.0
º«   ¬º:
------------------------------------------------------------*/
static int Export_price(char *str,void *p)
{
    uint8 i =0;

   for(i=0;i<48;i++)
  {
    dbg_printf(1,"price[%d]=%d",i,g_Sys_Config_Param.Price[i]);
    dbg_printf(1,"feetime[%d]=%d",i,g_Sys_Config_Param.FeeTime[i]);
  }
  dbg_printf(1,"Serverprice=%d",g_Sys_Config_Param.CrgServFee);
    return 1;
}

/*------------------------------------------------------------
≤Ÿ◊˜ƒøµƒ£∫  Export_Record √¸¡Ó
◊˜    ’ﬂ£∫
∫Ø ˝≤Œ ˝£∫
        str ∂¡»Îµƒ◊÷∑˚¥Æ
        *p ø…±‰≤Œ ˝
∑µªÿ÷µ£∫
            Œﬁ
∞Ê   ±æ:    V1.0
º«   ¬º:
------------------------------------------------------------*/
static int Export_Record(char *str,void *p)
{
    char OutStr[150] = {0};
    uint8 tmp[50] = {0};
    int index = 0;
    int i = 0;
    int offset = 0;

    osDelay(200);
    ChargeRecord ChargeRecordDataOld;//æ… ˝æ›

    index = SPIFLASH_Method4_Get_RecordCnt(TYPE_CHARGELOG);

    if(index > 0)
    {

       for(i = index - 1; i >= 0;i--)
       {

          if(SPIFLASH_Method4_Get_Record(TYPE_CHARGELOG,i,(uint8*)&ChargeRecordDataOld) == SPIFLASH_OK)
          {
              memset(OutStr,0,sizeof(OutStr));
              shift_time_data(tmp,ChargeRecordDataOld.StartTime);
              offset = sprintf(OutStr,"export_record %s,",tmp);
              memset(tmp,0,sizeof(tmp));
              shift_time_data(tmp,ChargeRecordDataOld.EndTime);
              offset += sprintf(OutStr + offset ,"%s,",tmp);
              offset += sprintf(OutStr + offset ,"%d,",ChargeRecordDataOld.ChargeTime/60);
              memset(tmp,0,sizeof(tmp));
              if(ChargeRecordDataOld.CardNo[0] == 0)
              {
                  memset(tmp,30,16);
              }
              else
              {
                  memcpy(tmp,ChargeRecordDataOld.CardNo,16);
              }
              //sprintf(OutStr + offset ,"%s,",tmp);
              //shift_hex_data(tmp,ChargeRecordDataOld.CardNo,16);
              offset += sprintf(OutStr + offset ,"%s,",tmp);
              offset += sprintf(OutStr + offset ,"%d,",ChargeRecordDataOld.StartCardType);
              offset += sprintf(OutStr + offset ,"%d,",ChargeRecordDataOld.EndEnerge - ChargeRecordDataOld.StartEnerge);//≥‰µÁµÁ¡ø
              offset += sprintf(OutStr + offset ,"%d,",ChargeRecordDataOld.StopReason);
              offset += sprintf(OutStr + offset ,"%d,",ChargeRecordDataOld.Report);
              offset += sprintf(OutStr + offset ,"%d,",ChargeRecordDataOld.EndFlag);
              offset += sprintf(OutStr + offset ,"%d,",ChargeRecordDataOld.ChargeMode);

              dbg_printf(1,"%s",OutStr);

              osDelay(20);
          }
       }
    }
    return 1;
}

/*------------------------------------------------------------
≤Ÿ◊˜ƒøµƒ£∫  Export_Alarm √¸¡Ó
◊˜    ’ﬂ£∫
∫Ø ˝≤Œ ˝£∫
        str ∂¡»Îµƒ◊÷∑˚¥Æ
        *p ø…±‰≤Œ ˝
∑µªÿ÷µ£∫
            Œﬁ
∞Ê   ±æ:    V1.0
º«   ¬º:
------------------------------------------------------------*/
static int Export_Alarm(char *str,void *p)
{

    char OutStr[128] = {0};
    uint8 tmp[33] = {0};
    int index = 0;
    int offset = 0;


    ALARM_DATA stAlarmData;//æ… ˝æ›

    index = SPIFLASH_Method4_Get_RecordCnt(TYPE_ALARMLOG);

    osDelay(200);

    if(index > 0)
    {
        int i = 0;

        for(i = index - 1; i >= 0;i--)
        {
            SPIFLASH_Method4_Get_Record(TYPE_ALARMLOG,i - 1,(uint8*)&stAlarmData);

            memset(tmp,0,sizeof(tmp));

            offset = sprintf(OutStr,"export_alarm %d,",stAlarmData.AlarmCode);

            shift_time_data(tmp,stAlarmData.StartTime);
            offset += sprintf(OutStr + offset,"%s,",tmp);
            memset(tmp,0,sizeof(tmp));
            shift_time_data(tmp,stAlarmData.EndTime);
            offset += sprintf(OutStr + offset,"%s,",tmp);
            offset += sprintf(OutStr + offset ,"%d,",stAlarmData.Recover);
            offset += sprintf(OutStr + offset ,"%d,",stAlarmData.ReportF);

            dbg_printf(1,"%s",OutStr);

            osDelay(10);
        }
    }

    return 1;
}

static int set_charger_NO(char *str,void *p)
{
  SYS_CONFIG_PARAM Sys_Config_Param = {0};

  memcpy(g_Sys_Config_Param.Serial, str, 16);
  SPIFLASH_Data_Erase(SPIFLASH_METHOD_SINGLERECORD,TYPE_SYSPARA);
  SPIFLASH_Data_Erase(SPIFLASH_METHOD_SINGLERECORD,TYPE_FACTORY_PARAM);
  g_Sys_Config_Param.CrcVal = CRC16((uint16_t * )&g_Sys_Config_Param,sizeof(Sys_Config_Param)/2 - 1);
  SPIFLASH_Method1_Save_Record(TYPE_SYSPARA,(uint8_t *)&g_Sys_Config_Param);
  SPIFLASH_Method1_Save_Record(TYPE_FACTORY_PARAM,(uint8_t *)&g_Sys_Config_Param);

  dbg_printf(1,"set_charger_NO OK");
	return 0;
}

static int set_price(char *str,void *p)
{
  uint8  i;
	uint16 price;
	SYS_CONFIG_PARAM Sys_Config_Param = {0};


	price = (str[0] & 0x0F)*100 + (str[1] & 0x0F)*10 + (str[2] & 0x0F);
  for(i = 0; i < 48 ;i++)
  {
    g_Sys_Config_Param.Price[i] = price;
  }
  SPIFLASH_Data_Erase(SPIFLASH_METHOD_SINGLERECORD,TYPE_SYSPARA);
  SPIFLASH_Data_Erase(SPIFLASH_METHOD_SINGLERECORD,TYPE_FACTORY_PARAM);
  g_Sys_Config_Param.CrcVal = CRC16((uint16_t * )&g_Sys_Config_Param,sizeof(Sys_Config_Param)/2 - 1);
  SPIFLASH_Method1_Save_Record(TYPE_SYSPARA,(uint8_t *)&g_Sys_Config_Param);
  SPIFLASH_Method1_Save_Record(TYPE_FACTORY_PARAM,(uint8_t *)&g_Sys_Config_Param);
  dbg_printf(1,"set_price OK");

	return 0;
}

static int set_port_NO(char *str,void *p)
{
  uint8  i;
	uint16 price;
	SYS_CONFIG_PARAM Sys_Config_Param = {0};

	g_Sys_Config_Param.AutoIP = 0;
  g_Sys_Config_Param.Port = (str[0] & 0x0F)*1000 + (str[1] & 0x0F)*100 + (str[2] & 0x0F)*10 + (str[3] & 0x0F);

  SPIFLASH_Data_Erase(SPIFLASH_METHOD_SINGLERECORD,TYPE_SYSPARA);
  SPIFLASH_Data_Erase(SPIFLASH_METHOD_SINGLERECORD,TYPE_FACTORY_PARAM);
  g_Sys_Config_Param.CrcVal = CRC16((uint16_t * )&g_Sys_Config_Param,sizeof(Sys_Config_Param)/2 - 1);
  SPIFLASH_Method1_Save_Record(TYPE_SYSPARA,(uint8_t *)&g_Sys_Config_Param);
  SPIFLASH_Method1_Save_Record(TYPE_FACTORY_PARAM,(uint8_t *)&g_Sys_Config_Param);
  dbg_printf(1," set_port_NO OK");

	return 0;
}

static int set_AutoIP(char *str,void *p)
{
  uint8  i;
	uint16 price;
	SYS_CONFIG_PARAM Sys_Config_Param = {0};

	g_Sys_Config_Param.AutoIP = (str[0] == 0x31) ? 1 : 0;
  SPIFLASH_Data_Erase(SPIFLASH_METHOD_SINGLERECORD,TYPE_SYSPARA);
  SPIFLASH_Data_Erase(SPIFLASH_METHOD_SINGLERECORD,TYPE_FACTORY_PARAM);
  g_Sys_Config_Param.CrcVal = CRC16((uint16_t * )&g_Sys_Config_Param,sizeof(Sys_Config_Param)/2 - 1);
  SPIFLASH_Method1_Save_Record(TYPE_SYSPARA,(uint8_t *)&g_Sys_Config_Param);
  SPIFLASH_Method1_Save_Record(TYPE_FACTORY_PARAM,(uint8_t *)&g_Sys_Config_Param);
  dbg_printf(1," set_AutoIP OK");

	return 0;
}

static 	 CMD_INFO st_cmd_Tbl[]=
{   //√¸¡Ó√˚≥∆£¨        ∫Ø ˝,          ∞Ô÷˙–≈œ¢
	{"reboot",			reboot_System,"reboot system"}, //∏¥Œª
	{"get_alarm_record",getAlarmRecord,"get alarm record"},//ªÒ»°∏ÊæØ
	{"get_charge_record",getChargeRecord,"get charge record"},//ªÒ»°¿˙ ∑
	{"get_param",		getParam,"get_param"},          //≤Œ ˝
  {"get_run_info",      Get_Run_Info,"Get_Run_Info"},//‘À–– ˝æ›
  {"test_mode",            testFun,"test function"},       //µ•∞Â≤‚ ‘”√
  {"set_param",       setParam,"set system param"},   //…Ë÷√≤Œ ˝
  {"updata",          updataFun,"updata system"},     //∏¸–¬ πÃº˛
  {"start_charge",    start_Charge,"start charge cmd"},       //ø™ º≥‰µÁ
  {"stop_charge",     stop_Charge,"stop charge"},             //Õ£÷π≥‰µÁ
  {"get_all_param",     getAllparam,"print all param"},       //ªÒ»°À˘”–≤Œ ˝
  {"del_alarm_data",    Delet_Alarm_Data,"Delet_Alarm_Data"},     //≤Œ ˝À˘”–∏ÊæØ ˝æ›
  {"del_charge_data",   Delet_Charge_Data,"Delet_Charge_Data"},// «…æ≥˝
  {"reset_sys_param",   Reset_sys_Param,"Reset_sys_Param"},
  {"export_record",       Export_Record,"Export_Record"},
  {"export_alarm",        Export_Alarm,"Export_Alarm"},
  {"erase_flash",       erase_flash,"erase_flash all"},
  {"save_factory_param", save_to_factory_param,"save_factory_param"},
  {"restore_param", restory_factory_param,"save_factory_param"},
  {"adjust_meter", Adjust_Meter_Param,"adjust_meter"},
  {"set_charger_NO", set_charger_NO, "set_charger_NO"},
  {"set_price", set_price, "set_price"},
  {"set_AutoIP", set_AutoIP, "set_AutoIP"},
	{"set_port_NO", set_port_NO, "set_port_NO"},
  {"help",             do_help,"printf all cmd"},
  {"get_ip",     Get_ip,"printf all cmd"},
  {"export_price",Export_price,"Export_price"},
};

#define MAX_TBL_LENTH   (sizeof(st_cmd_Tbl)/sizeof(st_cmd_Tbl[0]))
/*------------------------------------------------------------
≤Ÿ◊˜ƒøµƒ£∫  getAllparam √¸¡Ó ªÒ»°À˘”–µƒ≤Œ ˝
◊˜    ’ﬂ£∫
∫Ø ˝≤Œ ˝£∫
        str ∂¡»Îµƒ◊÷∑˚¥Æ
        *p ø…±‰≤Œ ˝
∑µªÿ÷µ£∫
            Œﬁ
∞Ê   ±æ:    V1.0
º«   ¬º:
------------------------------------------------------------*/
static int getAllparam(char *str,void *p)
{

    int i = 0;
    char name[20] = {0};
    char val[40] = {0};

    uint8 intVal[4] = {0};
    int tmpVal = 0;

    for(i = 0; i < MAX_TBL_LENTH;i++)
    {
        memset(name,0,sizeof(name));
        memcpy(name,g_ParmaTbl[i].name,sizeof(name));
        if(strstr(g_ParmaTbl[i].fmt ,"ip"))
        {
            memcpy((uint8*)intVal,(uint8*)g_ParmaTbl[i].val,g_ParmaTbl[i].lenth);
            sprintf(val,"%d.%d.%d.%d",intVal[0],intVal[1],intVal[2],intVal[3]);
            getParamResponse(name,val,p);
        }
        else if(strstr(g_ParmaTbl[i].fmt ,"int"))
        {
            memcpy((uint8*)&tmpVal,(uint8*)g_ParmaTbl[i].val,g_ParmaTbl[i].lenth);
            sprintf(val,"%d",tmpVal);
            getParamResponse(name,val,p);
        }
        else if(strstr(g_ParmaTbl[i].fmt ,"str"))
        {

            memcpy((uint8*)val,(uint8*)g_ParmaTbl[i].val,g_ParmaTbl[i].lenth);
            getParamResponse(name,val,p);
        }
        else if(strstr(g_ParmaTbl[i].fmt ,"hex"))
        {
            int j = 0;
            uint8 buf[64] = {0};
            int index = 0;
            memcpy((uint8*)buf,(uint8*)g_ParmaTbl[i].val,g_ParmaTbl[i].lenth);
            for(j = 0; j < g_ParmaTbl[i].lenth;j++)
            {
                sprintf(val+index,"%.2X",(uint8*)buf[j]);
                index +=2;
            }
            getParamResponse(name,val,p);
        }
        osDelay(20);
    }
    return 0;
}

/*------------------------------------------------------------
≤Ÿ◊˜ƒøµƒ£∫  do_help help √¸¡Ó
◊˜    ’ﬂ£∫
∫Ø ˝≤Œ ˝£∫
        str ∂¡»Îµƒ◊÷∑˚¥Æ
        *p ø…±‰≤Œ ˝
∑µªÿ÷µ£∫
            Œﬁ
∞Ê   ±æ:    V1.0
º«   ¬º:
------------------------------------------------------------*/
static int  do_help(char *str,void *p)
{
    char buf[64] = {0};
    int i = 0;

    for(i = 0; i <MAX_TBL_LENTH;i++)
    {
        memset(buf,0,sizeof(buf));
        sprintf(buf,"%s\n",st_cmd_Tbl[i].cmd);
        resonse_send(buf,strlen(buf),p);

    }
    return 0;
}
/*------------------------------------------------------------
≤Ÿ◊˜ƒøµƒ£∫  read_line “‘ªª––∑÷ø™√¸¡Ó
◊˜    ’ﬂ£∫
∫Ø ˝≤Œ ˝£∫
        str ∂¡»Îµƒ◊÷∑˚¥Æ
        len ◊÷∑˚¥Æ≥§∂»
∑µªÿ÷µ£∫
            Œﬁ
∞Ê   ±æ:    V1.0
º«   ¬º:
------------------------------------------------------------*/
static uint32 read_line(char *str,uint32 len)
{
    uint32 tem = 0;
    while(*str != 0x0d && *str != 0)
    {
        tem++;
        str++;
    }
    return tem;
}
/*------------------------------------------------------------
≤Ÿ◊˜ƒøµƒ£∫  Ω‚Œˆ√¸¡Ó
◊˜    ’ﬂ£∫
∫Ø ˝≤Œ ˝£∫comm Ω·ππÃÂ
        line ∂¡»Îµƒ◊÷∑˚¥Æ
        len ◊÷∑˚¥Æ≥§∂»
∑µªÿ÷µ£∫
            Œﬁ
∞Ê   ±æ:    V1.0
º«   ¬º:
------------------------------------------------------------*/
int Parse_Line(COMM_TYPE *comm,char *line,int len)
{
    int i = 0;
    char cmd[128];

    char val[128];

    int readlen = 0;

    if(comm->link_type != LAN)
    {
        readlen =  read_line(line,len);

        if(readlen == len )
        {
            return 0;
        }
    }
    else
    {
        line[len] = '\n';
    }

    for(i = 0; i < MAX_TBL_LENTH;i++)
    {
        if(strstr(line,st_cmd_Tbl[i].cmd))
        {
            if(comm->link_type == LAN)
            {
                sscanf (line, "%[^ ] %[^\n]", cmd,val);
            }
            else
            {
                sscanf (line, "%[^ ] %[^\n]", cmd,val);
            }

            if(st_cmd_Tbl[i].fun_Proc)
            {
                st_cmd_Tbl[i].fun_Proc(val,comm);
                break;
            }
        }
    }

    return readlen;
}

//∞Ê±æ◊‘∂Ø ∂±
static void Version_Detect(void)
{
//    char bit = 0;

}
/*Á®ãÂ∫èËÆæÁΩÆÊúçÂä°Ë¥π*/
void Set_Fee(void)
{
	uint8 i = 0;
	
	g_Sys_Config_Param.CrgServFee = 50;
	for(i = 0;i < 16;i++)
	{
		g_Sys_Config_Param.Price[i] = 25;
		g_Sys_Config_Param.FeeTime[i] = 4;
	}
	for(i = 16;i < 20;i++)
	{
		g_Sys_Config_Param.Price[i] = 66;
		g_Sys_Config_Param.FeeTime[i] = 3;
	}
	for(i = 20;i < 22;i++)
	{
		g_Sys_Config_Param.Price[i] = 112;
		g_Sys_Config_Param.FeeTime[i] = 2;
	}
	for(i = 22;i < 24;i++)
	{
		g_Sys_Config_Param.Price[i] = 140;
		g_Sys_Config_Param.FeeTime[i] = 1;
	}
	for(i = 24;i < 28;i++)
	{
		g_Sys_Config_Param.Price[i] = 66;
		g_Sys_Config_Param.FeeTime[i] = 3;
	}
	for(i = 28;i < 30;i++)
	{
		g_Sys_Config_Param.Price[i] = 112;
		g_Sys_Config_Param.FeeTime[i] = 2;
	}
	for(i = 30;i < 34;i++)
	{
		g_Sys_Config_Param.Price[i] = 140;
		g_Sys_Config_Param.FeeTime[i] = 1;
	}
	for(i = 34;i < 38;i++)
	{
		g_Sys_Config_Param.Price[i] = 112;
		g_Sys_Config_Param.FeeTime[i] = 2;
	}
	for(i = 38;i < 48;i++)
	{
		g_Sys_Config_Param.Price[i] = 66;
		g_Sys_Config_Param.FeeTime[i] = 3;
	}
	g_Sys_Config_Param.CrcVal = CRC16((uint16_t * )&g_Sys_Config_Param,sizeof(g_Sys_Config_Param)/2 - 1);
    SPIFLASH_Method1_Save_Record(TYPE_SYSPARA,(uint8_t *)&g_Sys_Config_Param);
    SPIFLASH_Method1_Save_Record(TYPE_FACTORY_PARAM,(uint8_t *)&g_Sys_Config_Param);
}

///
/*------------------------------------------------------------
≤Ÿ◊˜ƒøµƒ£∫   ∏¥ŒªÀ˘”–≤Œ ˝
◊˜    ’ﬂ£∫
≤Ÿ◊˜Ω·π˚£∫
∫Ø ˝≤Œ ˝£∫type º«¬º≤Œ ˝¿‡±

∑µªÿ÷µ£∫
            Œﬁ

∞Ê   ±æ:    V1.0

º«   ¬º:

------------------------------------------------------------*/

static void Sys_Config_Para_Reset(void)
{
    int i = 0;

    uint8 Tmp[32] = {0};

    memset(&g_Sys_Config_Param,0,sizeof(g_Sys_Config_Param));

    g_Sys_Config_Param.LocalIp[0] = 169;
    g_Sys_Config_Param.LocalIp[1] = 254;
    g_Sys_Config_Param.LocalIp[2] = 195;
    g_Sys_Config_Param.LocalIp[3] = 132;

    memcpy(g_Sys_Config_Param.SerIp, "47.112.139.118",sizeof( "47.112.139.118"));

    g_Sys_Config_Param.Port = 8612;

    g_Sys_Config_Param.NetMask[0] = 255;
    g_Sys_Config_Param.NetMask[1] = 255;
    g_Sys_Config_Param.NetMask[2] = 255;
    g_Sys_Config_Param.NetMask[3] = 0;

    g_Sys_Config_Param.GetAway[0] = 192;
    g_Sys_Config_Param.GetAway[1] = 168;
    g_Sys_Config_Param.GetAway[2] = 1;
    g_Sys_Config_Param.GetAway[3] = 1;

    g_Sys_Config_Param.DomainSer[0] = 8;
    g_Sys_Config_Param.DomainSer[1] = 8;
    g_Sys_Config_Param.DomainSer[2] = 8;
    g_Sys_Config_Param.DomainSer[3] = 8;
    g_Sys_Config_Param.MAC[0] = 0xb8;
    Get_Chip_ID(Tmp);
    memcpy(&g_Sys_Config_Param.MAC[1],Tmp,6);

    g_Sys_Config_Param.OverVolt = 2640;
    g_Sys_Config_Param.Undervoltage = 1700;//1760;//1870

    for(i = 0; i < 48 ;i++)
    {
        g_Sys_Config_Param.Price[i] = 100;
        g_Sys_Config_Param.FeeTime[i] = 1;

    }

    //g_Sys_Config_Param.FeeTime[0] = 0;
    //g_Sys_Config_Param.FeeTime[1] = ((23<<8) + 59);

    g_Sys_Config_Param.CrgServFee = 50;
    g_Sys_Config_Param.FeeVersion[0] = 0;   //≥ı º◊¥Ã¨∑—¬ ∞Ê±æŒ™ø’”…∫ÛÃ®œ¬∑¢
    g_Sys_Config_Param.GunLockEn = 0;
    g_Sys_Config_Param.phaseType = PHASE_1;
    g_Sys_Config_Param.WorkMod = 0;
    g_Sys_Config_Param.ScreenEn = 0;
    g_Sys_Config_Param.DeviceType = AS_7K;
    g_Sys_Config_Param.AutoIP = 1;
    g_Sys_Config_Param.BiasEn = 0;
    g_Sys_Config_Param.CurrentType = CUR_32;
    g_Sys_Config_Param.DoubelGunEn = 0;

    g_Sys_Config_Param.CommType = KH_P;
    g_Sys_Config_Param.MeterType = TYPE_RN8209;
    g_Sys_Config_Param.DtuType = 0;
    g_Sys_Config_Param.PassWord = 9999;
    memcpy(g_Sys_Config_Param.HardVer,"V1.00",sizeof("V1.00"));
    memcpy(g_Sys_Config_Param.Serial,"6720201229000001",16);
    g_Sys_Config_Param.SlaveNum = 0;
    g_Sys_Config_Param.SlaveAddr = 1;
    g_Sys_Config_Param.CardReaderType = CARD_MT;
    g_Sys_Config_Param.TrialTime[0] = 29;
    g_Sys_Config_Param.TrialTime[1] = 12;
    g_Sys_Config_Param.TrialTime[2] = 30;
    memcpy(g_Sys_Config_Param.IcCardKey,"123456",6);
    memcpy(g_Sys_Config_Param.DevName,"CDZ  ",sizeof(g_Sys_Config_Param.DevName));
    g_Sys_Config_Param.QRCodeEN = 0;
    strcpy((char*)g_Sys_Config_Param.QRCode,"https://nesp.jwt520.com/?serialnum=");


	//add
    g_Sys_Config_Param.interval = 60;
    g_Sys_Config_Param.FeeSectionNum = 1;
    g_Sys_Config_Param.DbgMod = 0;
	memcpy(SoftVer,SoftVer_Phase1,sizeof(SoftVer_Phase1));

	g_Sys_Config_Param.TCP_ud_flag = 0;
	g_Sys_Config_Param.TCP_ud_stop_adr = 0;
	g_Sys_Config_Param.TCP_ud_stop_serial_number = 0;

  g_Sys_Config_Param.restart_num = 0;
  g_Sys_Config_Param.restart_time = 0;
  Set_Fee();

}

/*------------------------------------------------------------
≤Ÿ◊˜ƒøµƒ£∫   ∏¥ŒªÀ˘”–≤Œ ˝
◊˜    ’ﬂ£∫
∫Ø ˝≤Œ ˝£∫Œﬁ

∑µªÿ÷µ£∫
            Œﬁ

∞Ê   ±æ:    V1.0

º«   ¬º:

------------------------------------------------------------*/
void Sys_Config_Para_Load(void)
{

    SYS_CONFIG_PARAM Sys_Config_Param = {0};
    U16 crc = 0;

    SPIFLASH_Method1_Get_Record(TYPE_SYSPARA,(uint8_t *)&Sys_Config_Param);
    crc = CRC16((uint16_t * )&Sys_Config_Param,sizeof(Sys_Config_Param)/2 - 1);
    if(crc == Sys_Config_Param.CrcVal)
    {
        memcpy(&g_Sys_Config_Param,&Sys_Config_Param,sizeof(Sys_Config_Param));
    }
    else
    {
        Sys_Config_Para_Reset();
        SPIFLASH_Data_Erase(SPIFLASH_METHOD_SINGLERECORD,TYPE_SYSPARA);
        SPIFLASH_Data_Erase(SPIFLASH_METHOD_SINGLERECORD,TYPE_FACTORY_PARAM);
        g_Sys_Config_Param.CrcVal = CRC16((uint16_t * )&g_Sys_Config_Param,sizeof(Sys_Config_Param)/2 - 1);
        SPIFLASH_Method1_Save_Record(TYPE_SYSPARA,(uint8_t *)&g_Sys_Config_Param);
        SPIFLASH_Method1_Save_Record(TYPE_FACTORY_PARAM,(uint8_t *)&g_Sys_Config_Param);
    }
    if(g_Sys_Config_Param.CommType == KH_P
        || g_Sys_Config_Param.CommType == KH_V4)
    {
        Version_Detect();
    }


    strcpy(CompileTime,__DATE__);
    strcat(CompileTime," ");
    strcat(CompileTime,__TIME__);
    //»Ìº˛∞Ê±æ

    memcpy(SoftVer,SoftVer_Phase1,sizeof(SoftVer_Phase1));

    g_Sys_Config_Param_bk = g_Sys_Config_Param;
    check_power_on_record();
    g_Tmp_Config_Param = g_Sys_Config_Param;
}

/*------------------------------------------------------------
≤Ÿ◊˜ƒøµƒ£∫   ªÒ»°œµÕ≥≈‰÷√ ˝æ›£¨∑µªÿµÿ÷∑
◊˜    ’ﬂ£∫
∫Ø ˝≤Œ ˝£∫Œﬁ

∑µªÿ÷µ£∫
            Œﬁ
∞Ê   ±æ:    V1.0

º«   ¬º:

------------------------------------------------------------*/
SYS_CONFIG_PARAM *GetSys_Config_Param(void)
{
    return &g_Sys_Config_Param;
}


/*------------------------------------------------------------
≤Ÿ◊˜ƒøµƒ£∫   ªÒ»°œµÕ≥≈‰÷√ ˝æ›£¨∑µªÿµÿ÷∑
◊˜    ’ﬂ£∫
∫Ø ˝≤Œ ˝£∫Œﬁ

∑µªÿ÷µ£∫
            Œﬁ
∞Ê   ±æ:    V1.0

º«   ¬º:

------------------------------------------------------------*/
SYS_CONFIG_PARAM *GetTmp_Config_Param(void)
{
    return &g_Tmp_Config_Param;
}



/*------------------------------------------------------------
≤Ÿ◊˜ƒøµƒ£∫   ªÒ»°µ•œ‡ªÚ’ﬂ»˝œ‡¿‡–Õ
◊˜    ’ﬂ£∫
∫Ø ˝≤Œ ˝£∫Œﬁ

∑µªÿ÷µ£∫   µ•œ‡ªÚ’ﬂ»˝œ‡
            Œﬁ
∞Ê   ±æ:    V1.0

º«   ¬º:

------------------------------------------------------------*/
U8 Get_phaseType(void)
{
    return g_Sys_Config_Param.phaseType;
}
/*------------------------------------------------------------
≤Ÿ◊˜ƒøµƒ£∫   ªÒ»°À´«π «∑Ò πƒ‹
◊˜    ’ﬂ£∫
∫Ø ˝≤Œ ˝£∫Œﬁ

∑µªÿ÷µ£∫   µ•ªÚ’ﬂÀ´«π
            Œﬁ
∞Ê   ±æ:    V1.0

º«   ¬º:

------------------------------------------------------------*/
U8 Get_DoubelGunEn(void)
{
    return g_Sys_Config_Param.DoubelGunEn;
}
/*------------------------------------------------------------
≤Ÿ◊˜ƒøµƒ£∫   ªÒ»°∑˛ŒÒ∆˜µƒIPµÿ÷∑
◊˜    ’ﬂ£∫
∫Ø ˝≤Œ ˝£∫ ‰≥ˆIPµÿ÷∑

∑µªÿ÷µ£∫   Œﬁ
            Œﬁ
∞Ê   ±æ:    V1.0

º«   ¬º:

------------------------------------------------------------*/
U8 Get_Server_Ip(U8 *ip)
{
    memcpy(ip,g_Sys_Config_Param.SerIp,sizeof(g_Sys_Config_Param.SerIp));
    return 0;
}
/*------------------------------------------------------------
≤Ÿ◊˜ƒøµƒ£∫   ªÒ»°∑˛ŒÒ∆˜µƒ∂Àø⁄∫≈
◊˜    ’ﬂ£∫
∫Ø ˝≤Œ ˝£∫Œﬁ

∑µªÿ÷µ£∫  ∂Àø⁄
            Œﬁ
∞Ê   ±æ:    V1.0

º«   ¬º:

------------------------------------------------------------*/
U16 Get_Server_Port(void)
{
   return g_Sys_Config_Param.Port;
}

void Get_Card_Key(uint8 *Key)
{
    uint8 null[6] = {0};
    if(memcmp(null,g_Sys_Config_Param.IcCardKey,6) == 0)
    {
        memset(Key,0xff,6);
    }
    else
    {
        memcpy(Key,g_Sys_Config_Param.IcCardKey,6);
    }

}

U8 Get_Local_Ip(U8 *ip)
{
    memcpy(ip,g_Tmp_Config_Param.LocalIp,4);
    return 0;
}

U8 Get_NetMask(U8 *Mask)
{
    memcpy(Mask,g_Sys_Config_Param.NetMask,4);
    return 0;
}

U8 Get_GetAway(U8 *GetAway)
{
    memcpy(GetAway,g_Sys_Config_Param.GetAway,4);
    return 0;
}

U8 Get_NetMAC(U8 *MAC)
{
    memcpy(MAC,g_Sys_Config_Param.MAC,6);
    return 0;
}


U8 Get_DomainSer(U8 *Ser)
{
    memcpy(Ser,g_Sys_Config_Param.DomainSer,4);
    return 0;
}



U8 Get_Cur_Out_Type(void)
{
    return g_Sys_Config_Param.CurrentType;
}


U8 Get_LocKEn(void)
{
    return g_Sys_Config_Param.GunLockEn;
}


U16 Get_OverVolt(void)
{
    return g_Sys_Config_Param.OverVolt;
}
U16 Get_Undervoltage(void)
{
    return g_Sys_Config_Param.Undervoltage;
}

U8 Get_WorkMod(void)
{
    return 0;//g_Sys_Config_Param.WorkMod;
}

U8 Get_SlaverAddr(void)
{
    return g_Sys_Config_Param.SlaveAddr;
}

U8 Get_BiasEn(void)
{
    return g_Sys_Config_Param.BiasEn;
}

U8 Get_MeterType(void)
{
    return g_Sys_Config_Param.MeterType;

}

U16 Get_Price(uint8 index)
{
    return g_Sys_Config_Param.Price[index];

}
U8 Get_Card_Read_Type(void)
{
    return g_Sys_Config_Param.CardReaderType;
}

U8 Get_Slaver_Num(void)
{
    return g_Sys_Config_Param.SlaveNum;
}

U8 Get_Dev_Name(uint8 *dev)
{
    memcpy(dev,g_Sys_Config_Param.DevName,sizeof(g_Sys_Config_Param.DevName));

    return 0;
}

U8 Get_Protocol_Type(void)
{

    return g_Sys_Config_Param.CommType;

}

char *Get_ComlieTime(void)
{
    return CompileTime;
}
//0  «◊‘∂Ø 1  ÷∂ØªÒ»°
char Get_AutoIP_En(void)
{
  return g_Sys_Config_Param.AutoIP;
}

char Get_Screen_En(void)
{
    return g_Sys_Config_Param.ScreenEn;
}

//uint8_t Get_Card_Read_Type(void)
//{
//    return g_Sys_Config_Param.CardReaderType;
//}

U8 Get_ip()
{
    dbg_printf(1,"IP %d.%d.%d.%d",g_Tmp_Config_Param.LocalIp[0] ,g_Tmp_Config_Param.LocalIp[1] , g_Tmp_Config_Param.LocalIp[2] ,g_Tmp_Config_Param.LocalIp[3] );
    return 0;
}

void check_opt_record(void)
{
	int i  = 0;
    int index = 0;
	opt_record_info st_opt_record_info = {0};
	struct tm;
    uint8 tmp[48] = {0};
    memset(&st_opt_record_info,0,sizeof(st_opt_record_info));
	for(i = 0;i < sizeof(st_opt_record)/sizeof(st_opt_record[0]);i++)
	{
		if(memcmp(st_opt_record[i].val_name,st_opt_record_bak[i].val_name,st_opt_record[i].val_len))
		{
			st_opt_record_info.opt_type = st_opt_record[i].opt_type;

            if(strstr(st_opt_record[i].fmt,"int"))
            {
                int tmpVal = 0;
                memcpy(&tmpVal,(uint8*)st_opt_record[i].val_name,st_opt_record[i].val_len);
                if(st_opt_record[i].retio == 0)
                {
                    sprintf((char*)st_opt_record_info.LogVal,"%d",tmpVal);
                }
                else if(st_opt_record[i].retio == 1)
                {
                    sprintf((char*)st_opt_record_info.LogVal,"%d.%d",tmpVal/10,tmpVal%10);
                }
                else if(st_opt_record[i].retio == 2)
                {
                    sprintf((char*)st_opt_record_info.LogVal,"%d.%2.2d",tmpVal/100,tmpVal%100);
                }
                else if(st_opt_record[i].retio == 3)
                {
                    sprintf((char*)st_opt_record_info.LogVal,"%d.%3.3d",tmpVal/1000,tmpVal%1000);
                }
            }
            else if(strstr(st_opt_record[i].fmt,"hex"))
            {
                int k = 0;
                memcpy(tmp,(uint8*)st_opt_record[i].val_name,st_opt_record[i].val_len);
                for(k = 0; k < st_opt_record[i].val_len;k++)
                {
                    sprintf((char*)&st_opt_record_info.LogVal[index],"%.2X",tmp[k]);
                    index += 2;
                }
            }
            else if(strstr(st_opt_record[i].fmt,"ip"))
            {
                memcpy(tmp,(uint8*)st_opt_record[i].val_name,st_opt_record[i].val_len);
                sprintf((char*)st_opt_record_info.LogVal,"%d.%d.%d.%d",tmp[0],
                tmp[1],tmp[2],tmp[3]);

            }
            else if(strstr(st_opt_record[i].fmt,"str"))
            {
                memcpy(st_opt_record_info.LogVal,st_opt_record[i].val_name,st_opt_record[i].val_len);
            }
            memcpy(st_opt_record_bak[i].val_name,st_opt_record[i].val_name,st_opt_record[i].val_len);
			st_opt_record_info.opt_time =  time(NULL);

            SPIFLASH_Method2_Save_Record(RECORD_USERLOG,(uint8_t*)&st_opt_record_info);
		}
	}

}

void check_power_on_record(void)
{
	opt_record_info st_opt_record_info = {0};

	st_opt_record_info.opt_type = OPT_POWER_ON;//ø™ª˙ ¬º˛

	st_opt_record_info.opt_time =  time(NULL);

    SPIFLASH_Method2_Save_Record(RECORD_USERLOG,(uint8_t*)&st_opt_record_info);

}
void save_updata_record(uint8 res)
{
	opt_record_info st_opt_record_info = {0};

	st_opt_record_info.opt_type = OPT_UPDATA;//…˝º∂
    if(res)
    {
         sprintf((char*)st_opt_record_info.LogVal,"≥…π¶");
    }
    else
    {
        sprintf((char*)st_opt_record_info.LogVal," ß∞‹");
    }

	st_opt_record_info.opt_time =  time(NULL);

    SPIFLASH_Method2_Save_Record(RECORD_USERLOG,(uint8_t*)&st_opt_record_info);
}

void set_sys_time_record(struct tm set_time)
{
	opt_record_info st_opt_record_info = {0};

	st_opt_record_info.opt_type = OPT_SET_TIME;//…Ë÷√ ±º‰
    sprintf((char*)st_opt_record_info.LogVal,"%d-%.2d-%.2d %.2d:%.2d:%.2d",set_time.tm_year + 1900,set_time.tm_mon +1,
    set_time.tm_mday,set_time.tm_hour,set_time.tm_min,set_time.tm_sec);

	st_opt_record_info.opt_time =  time(NULL);

    SPIFLASH_Method2_Save_Record(RECORD_USERLOG,(uint8_t*)&st_opt_record_info);

}

uint8 Trial_Time_Check(void)
{
    //int time_sec = 0;

    return FALSE;

}