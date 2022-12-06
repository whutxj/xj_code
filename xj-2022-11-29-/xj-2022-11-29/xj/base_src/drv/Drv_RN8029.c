
/******************************************************************************
* Include Files
******************************************************************************/

#include "Drv_RN8029.h"
#include "BSP_Spi.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "Task_SpiFlash.h"
//#include "Drv_ATT7053A.h"
/******************************************************************************
* inition Variable Define  //寄存器操作都是3个字节  初始化中间和最后一个字节
******************************************************************************/
#define EMUCFG_Mid_Byte     0x20    // 能量设置为读后清零操作  
#define EMUCFG_End_Byte     0x04//0x00  选择第二通道作为计量通道

#define ModuleEn_Mid_Byte   0x01
#define ModuleEn_End_Byte   0xff

#define ANAEN_End_Byte      0x3f    //0x3f

#define STATUSCFG_Mid_Byte  0x00
#define STATUSCFG_End_Byte  0x02

#define IOCFG_Mid_Byte      0x01
#define IOCFG_End_Byte      0x24

#define ADCCON_Mid_Byte     0x00
#define ADCCON_End_Byte     0x0c

#define DEFAULT_MIN_CUR 64     //额定电流的0.2%    32A为例  32*2/1000   *    1000变比  =  64
/******************************************************************************
* Static Variable Definitions
******************************************************************************/
static EMU_DATA Emu_Data;
static SAVEPARA saveParaData;
static SAVEPARA getPataData;
static UIPPARA paraUIP;


/******************************************************************************
* Static Function Prototype Definitions
******************************************************************************/
/**
  * @brief  Initializes the att7053 SPI and put it into StandBy State (Ready for 
  *         data transfer).
  * @retval None
  */
HAL_StatusTypeDef RN8209_SPI_IO_Init(void)
{
    HAL_StatusTypeDef Status = HAL_OK;
//    GPIO_InitTypeDef  gpioinitstruct = {0};

//    A7053_SPI_CS_GPIO_CLK_ENABLE();
//		
//    __HAL_RCC_AFIO_CLK_ENABLE();
//        
////    __HAL_AFIO_REMAP_SWJ_DISABLE();
//    __HAL_AFIO_REMAP_SWJ_NOJTAG();

//		//__HAL_AFIO_REMAP_SWJ_NOJTAG();
//    gpioinitstruct.Pin    = A7053_SPI_CS_PIN;
//    gpioinitstruct.Mode   = GPIO_MODE_OUTPUT_PP;
//    gpioinitstruct.Pull   = GPIO_PULLUP;
//    gpioinitstruct.Speed  = GPIO_SPEED_FREQ_HIGH;
//    HAL_GPIO_Init(A7053_SPI_CS_GPIO_PORT, &gpioinitstruct);

//    /* SPI FLASH Config */
//    Status = BSP_SPIx_Init(SPI_A7053_PORT);

//    A7053_SPI_CS_HIGH();
    return Status;
}

/**
  * @brief  Write a byte on the RN8209 SPI.
  * @param  Data: MUTI-byteS to send.
  * @retval None
  */
static void RN8209_SPI_IO_Write(uint8_t *Data, uint8_t len)
{
    int i = 0;
    A7053_SPI_CS_LOW();
    *Data |= 0x80;
    for(i = 0; i < len; i++)
    {
        /* Send the byte */
        SPI2_Write(*Data);
        Data++;
    }
    A7053_SPI_CS_HIGH();
}

/**
  * @brief  Read bytes from the RN8209 SPI.
  * @retval uint8_t (The received bytes).
  */
static void FLASH_SPI_IO_Read(uint8_t *data,uint8_t len)
{
    uint8_t i = 0;
    A7053_SPI_CS_LOW();
    *data &= 0x7f;
    SPI2_Write(*data);   //写寄存器地址
    data ++;
    for(i = 1; i < len; i++)
    {
        /* Get the received data */
        *data = SPI2_Read();
        data ++;
    }
    A7053_SPI_CS_HIGH();
}

/**
  * @brief  Initializes peripherals used by the Serial att7053 device.
  * @retval A7053_OK (0x00) if operation is correctly performed, else 
  *         return A7053_ERROR (0x01).
  */
uint8_t Drv_RN8209_SPI_Init(void)
{
  //HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET);
    
  if(RN8209_SPI_IO_Init() != HAL_OK)
  {
    return A7053_ERROR;
  }
  else
  {
    return A7053_OK;
  }
}

/**
  * @brief  写保护,只能操作40H-45H
  * @retval
  */
static void RN8209_WR_En_4045(void)
{
    uint8_t RN8209Reg[4] = {0};
    RN8209Reg[0] = 0XEA;    //
    //跟着三字节数据  
    RN8209Reg[1] = 0xE5;   //0
    RN8209_SPI_IO_Write(RN8209Reg , 2);
}

/**
  * @brief  失能对校表数据的读写，写入非0xa6和0xbc值即可
  * @retval
  */
static void RN8209_WR_Dis(void)
{
    uint8_t RN8209Reg[4] = {0};
    RN8209Reg[0] = 0XEA;    //写保护 寄存器地址
    //跟着三字节数据  
    RN8209Reg[1] = 0xdc;   //00表示对所有校表寄存器不能进行操作
    RN8209_SPI_IO_Write(RN8209Reg , 2);
}
/**
  * @brief  Initializes att7053 Init para
  * @retval A7053_OK (0x00) if operation is correctly performed, else 
  *         return A7053_ERROR (0x01).
  */
void RN8209_Init_Para(void)
{
    uint8_t RN8209Reg[4] = {0};

    SPIFLASH_Method1_Get_Record(TYPE_METERPARA,(uint8_t *)&getPataData);
    Emu_Data.K_I2 = getPataData.saveK_I2;
    Emu_Data.K_P2 = getPataData.saveK_P2;
    Emu_Data.K_U  = getPataData.saveK_U;    
    if(Emu_Data.K_U > 0.0009 || getPataData.rev != 0xa5a5 || Emu_Data.K_I2 ==0)//不在合理区间
    {
        Emu_Data.K_U = 0.00013869726;
        Emu_Data.K_I2 = 0.000006211694653;
        Emu_Data.K_P2 = 0.00726545;
        saveParaData.saveK_I2 = Emu_Data.K_I2;
        saveParaData.saveK_P2 = Emu_Data.K_P2;
        saveParaData.saveK_U = Emu_Data.K_U;
        saveParaData.rev = 0xa5a5; 
        SPIFLASH_Method1_Save_Record(TYPE_METERPARA,(uint8_t *)&saveParaData);
        // clear meter val
        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR12,0);
        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR13,0);       
        
    }
    //dbg_printf(1,"KU %d,KI2 %d",Emu_Data.K_U * 1000000,Emu_Data.K_I2*10000000);

    RN8209_WR_En_4045();//使能写
    //test
    RN8209Reg[0] = ADDeviceID;
    FLASH_SPI_IO_Read(RN8209Reg,4); 

    RN8209Reg[0] = ADSYSCON;//
    RN8209Reg[1] = 0x00;
    RN8209Reg[2] = 0x00;
    RN8209_SPI_IO_Write(RN8209Reg, 3);    
        

    RN8209Reg[0] = ADHFConst;// 8236
    RN8209Reg[1] = 0x20;
    RN8209Reg[2] = 0x2C;
    RN8209_SPI_IO_Write(RN8209Reg, 3);
    
    RN8209Reg[0] = ADPStart;
    RN8209Reg[1] = 0x00;
    RN8209Reg[2] = 0x60;
    RN8209_SPI_IO_Write(RN8209Reg, 3);
    
    RN8209Reg[0] = ADDStart;
    RN8209Reg[1] = 0x01;
    RN8209Reg[2] = 0x20;
    RN8209_SPI_IO_Write(RN8209Reg, 3);


    RN8209Reg[0] = ADGPQA;
    RN8209Reg[1] = 0x00;
    RN8209Reg[2] = 0x00;
    RN8209_SPI_IO_Write(RN8209Reg, 3);

    RN8209Reg[0] = ADGPQA;
    RN8209Reg[1] = 0x00;
    RN8209Reg[2] = 0x00;
    RN8209_SPI_IO_Write(RN8209Reg, 3);
    
    RN8209Reg[0] = ADGPQB;
    RN8209Reg[1] = 0x00;
    RN8209Reg[2] = 0x00;
    RN8209_SPI_IO_Write(RN8209Reg, 3);    

    RN8209Reg[0] = ADPhsA;
    RN8209Reg[1] = 0x00;
    RN8209_SPI_IO_Write(RN8209Reg, 2);  
    
    RN8209Reg[0] = ADPhsB;
    RN8209Reg[1] = 0x00;
    RN8209_SPI_IO_Write(RN8209Reg, 2);
         
    RN8209Reg[0] = ADQPHSCAL;
    RN8209Reg[1] = 0x00;
    RN8209Reg[2] = 0x00;
    RN8209_SPI_IO_Write(RN8209Reg, 3);  

    RN8209Reg[0] = ADAPOSA;
    RN8209Reg[1] = 0x00;
    RN8209Reg[2] = 0x00;
    RN8209_SPI_IO_Write(RN8209Reg, 3);   

    RN8209Reg[0] = ADAPOSB;
    RN8209Reg[1] = 0x00;
    RN8209Reg[2] = 0x00;
    RN8209_SPI_IO_Write(RN8209Reg, 3);  
    
    RN8209Reg[0] = ADRPOSA;
    RN8209Reg[1] = 0x00;
    RN8209Reg[2] = 0x00;
    RN8209_SPI_IO_Write(RN8209Reg, 3); 
    
    RN8209Reg[0] = ADRPOSB;
    RN8209Reg[1] = 0x00;
    RN8209Reg[2] = 0x00;
    RN8209_SPI_IO_Write(RN8209Reg, 3);  

    RN8209Reg[0] = ADIARMSOS;
    RN8209Reg[1] = 0x00;
    RN8209Reg[2] = 0x00;
    RN8209_SPI_IO_Write(RN8209Reg, 3);  

    RN8209Reg[0] = ADIBRMSOS;
    RN8209Reg[1] = 0x00;
    RN8209Reg[2] = 0x00;
    RN8209_SPI_IO_Write(RN8209Reg, 3); 

    RN8209Reg[0] = ADIBGain;
    RN8209Reg[1] = 0x00;
    RN8209Reg[2] = 0x00;
    RN8209_SPI_IO_Write(RN8209Reg, 3); 

    RN8209Reg[0] = ADEMUCON2;
    RN8209Reg[1] = 0x00;
    RN8209Reg[2] = 0x00;
    RN8209_SPI_IO_Write(RN8209Reg, 3); 

    RN8209Reg[0] = ADEMUCON;
    RN8209Reg[1] = 0x80;
    RN8209Reg[2] = 0x03;
    RN8209_SPI_IO_Write(RN8209Reg, 3);
    
    //失能寄存器的读写
    RN8209_WR_Dis();
    
}

uint32_t RN8209_Read_ID(void)
{
    uint8_t RN8209Reg[4] = {0};
    uint32_t ID = 0;
    RN8209Reg[0] = ADDeviceID;
    FLASH_SPI_IO_Read(RN8209Reg,4);
    
    ID = (RN8209Reg[1]<<16)+ (RN8209Reg[2]<<8) + RN8209Reg[3];
    return ID;
    
}
/**
  * @brief  4个8Bit --to-- 32Bit
  * @retval
  */
static uint32_t Shift_2_32Bit(uint8_t *data)
{
    uint32_t ret = 0;
    ret = data[3];
    ret += data[2] << 8;
    ret += data[1] << 16;
    ret += data[0] << 24;
    return ret;
}

/**
  * @brief  读取电压、电流、功率
  * @retval
  */
void RN802_Read_UIP(void)
{
//    Emu_Data.K_I1 = 1;
//    Emu_Data.K_I2 = 1;

//    Emu_Data.K_U = 1;
    uint8_t Check[5] = {0};
    uint8_t RN8209Reg[5] = {0};
    static uint16_t emu_sum = 0;
//    RN8209Reg[0] = EMUCFG;
//    FLASH_SPI_IO_Read(RN8209Reg,4);
//    
//    P_Flag = RN8209Reg[3];  //0000 0100 NoPLd 当发生有功潜动时，NoPLd被置为1；当处于启动时NoPLd清为0。
    //------电流通道1
    RN8209Reg[0] = ADIARMS;
    FLASH_SPI_IO_Read(RN8209Reg,4);
    RN8209Reg[0] = 0;
    Emu_Data.Effective_I1 = (Shift_2_32Bit(RN8209Reg) * Emu_Data.K_I1) * 100;  //电流1有效值
    
    //adc
    
    //------电流通道1
    RN8209Reg[0] = ADIARMS;
    FLASH_SPI_IO_Read(RN8209Reg,4);
    RN8209Reg[0] = 0;
    Emu_Data.Effective_I2 = (Shift_2_32Bit(RN8209Reg) * Emu_Data.K_I2) * 100;  //电流1有效值
    paraUIP.I = Emu_Data.Effective_I2;
    //------电压通道
    RN8209Reg[0] = ADURMS;
    FLASH_SPI_IO_Read(RN8209Reg,4);
    //校验和
    Check[0] = ADRData;
    FLASH_SPI_IO_Read(Check,5);
    if(memcmp(Check + 2,RN8209Reg + 1,3) == 0)
    {   
        RN8209Reg[0] = 0;
        Emu_Data.Effective_U = (Shift_2_32Bit(RN8209Reg) * Emu_Data.K_U) * 100;  //电流1有效值
        paraUIP.U = Emu_Data.Effective_U;
    }    

    
    //有功功率
    RN8209Reg[0] = ADPowerPA;
    FLASH_SPI_IO_Read(RN8209Reg,5);
    //校验和
    Check[0] = ADRData;
    FLASH_SPI_IO_Read(Check,5);
    if(memcmp(Check + 1,RN8209Reg + 1,4) == 0)
    { 
        RN8209Reg[0] = 0;
        Emu_Data.Effective_P2 = Shift_2_32Bit(RN8209Reg + 1);
        float p2 = Emu_Data.Effective_P2;
        if(RN8209Reg[1] & 0x80)
        {
            p2 = Emu_Data.Effective_P2 - pow(2,24);
        }
        
        Emu_Data.Effective_P2 = p2 * Emu_Data.K_P2 * 100;
        paraUIP.P = Emu_Data.Effective_P2;
    }

    Check[0] = ADEMUStatus;
    FLASH_SPI_IO_Read(Check,4);
    if(emu_sum == 0)
    {
        emu_sum = (Check[1] << 8) + Check[2];
    }
    else
    {
        uint16_t sum = (Check[2] << 8) + Check[3];
        if(sum != emu_sum) 
        {
            emu_sum = sum;
            RN8209_Init_Para();
            dbg_printf(1,"emu_sum err~~~~~~~~~~~~~~~~~ reset param");
        }            
    }
    
    Check[0] = ADHFConst;//
    FLASH_SPI_IO_Read(Check,3);
    if(Check[1] != 0x20 && Check[2] != 0x2c)
    {
        RN8209_Init_Para();
        dbg_printf(1,"ADHFConst err~~~~~~~~~~~~~~~~~ reset param");
    }
    //-----启动功率判断
//    if(P_Flag & 0x04)
//    {
//        Emu_Data.Effective_I1 = 0;
//        Emu_Data.Effective_P1 = 0;
//        Emu_Data.Effective_P2 = 0;
//        Emu_Data.Power_Factor = 0x1000;
//    }
//    
//    //----屏蔽电流0.2%
//    if(Emu_Data.Effective_I2 < (DEFAULT_MIN_CUR))
//    {
//        Emu_Data.Effective_I2 = 0;
//        Emu_Data.Effective_P2 = 0;
//    }
//    if(Emu_Data.Effective_I1 < (DEFAULT_MIN_CUR))
//    {
//        Emu_Data.Effective_I1 = 0;
//    }
    
}
/**
* @brief  获取误差
  * @retval
  */
//float Get_Err_Data(uint8_t *buf)   //将标准功率传进来
//{
//    uint8_t RN8209Reg[4] = {0};
//    uint32_t truePower = 0;
//    float errData = 0;
//    truePower = buf[8] + (buf[9] << 8)+ (buf[10] << 16)+ (buf[11] << 24);//Shift_2_32Bit(&buf[8]);    
//    
//    //视在功率
//    RN8209Reg[0] = PowerP2;
//    FLASH_SPI_IO_Read(RN8209Reg,4);
//    RN8209Reg[0] = 0;
//    Emu_Data.Effective_P2 = Shift_2_32Bit(RN8209Reg);
//    float p2 = Emu_Data.Effective_P2;
//    if(RN8209Reg[1] & 0x80)
//    {
//        p2 = Emu_Data.Effective_P2 -  pow(2,24);
//    }
//    Emu_Data.Effective_P2 = p2 * Emu_Data.K_P2 * 100;
//    
//    errData = 1 - (float)((float)Emu_Data.Effective_P2 / truePower);    ///计算误差   注：注意errData和Effective_P1的有效值位数？？
//    return errData;
//}

/**
* @brief  计算电压、电流、功率系数（U:220V   I:XX.XX）
  * @retval
  */
uint8_t RN8209_Cal_UIP_K(uint8_t *buf)
{
    float Rated_I = 0,Rated_U = 0, Rated_P = 0;
    uint8_t RN8209Reg[4] = {0};
    uint32_t dataBuf[3];
    memcpy(dataBuf,buf,12);
   
    {
        Rated_U = (float)dataBuf[0]/ 100; //电压220V
        Rated_I = (float)dataBuf[1] / 100;     //额定电流  5A
        Rated_P = (float)dataBuf[2] / 100; //功率 1kw.h
        
        RN8209Reg[0] = ADIARMS;
        FLASH_SPI_IO_Read(RN8209Reg,4);
        RN8209Reg[0] = 0;
        Emu_Data.Effective_I2 = Shift_2_32Bit(RN8209Reg);
    //    Emu_Data.K_I1 = (float)(Rated_I / Emu_Data.Effective_I1) * 1000;  //电流系数
        Emu_Data.K_I2 = Rated_I / Emu_Data.Effective_I2;
        saveParaData.saveK_I2 = Emu_Data.K_I2;
        
        
        RN8209Reg[0] = ADURMS;
        FLASH_SPI_IO_Read(RN8209Reg,4);
        RN8209Reg[0] = 0;
        Emu_Data.Effective_U = Shift_2_32Bit(RN8209Reg);

        Emu_Data.K_U = (Rated_U / Emu_Data.Effective_U);
        saveParaData.saveK_U = Emu_Data.K_U;
        
        RN8209Reg[0] = ADPowerPA;
        FLASH_SPI_IO_Read(RN8209Reg,4);
        RN8209Reg[0] = 0;
        Emu_Data.Effective_P2 = Shift_2_32Bit(RN8209Reg);
        float p2 = Emu_Data.Effective_P2;
        if(RN8209Reg[1] & 0x80)
        {
            p2 = Emu_Data.Effective_P2 - pow(2,24);
        }
        Emu_Data.K_P2 = Rated_P / p2;  //功率系数    有功无功视在功率系数都是一样的
        saveParaData.saveK_P2 = Emu_Data.K_P2;

        /*
        
            保存，将各系数保存至存储区域。
        */
        SPIFLASH_Method1_Save_Record(TYPE_METERPARA,(uint8_t *)&saveParaData);
    }

    return 1;
}


/**
* @brief  调用此函数读取U  I   P 值   ，读取能量值    在调用后使用时除以100可得实际值，精确到0.01
  * @retval  指针数据顺序为  U  I   P      换算后返回能量值   电表表常数3200？
  */
float RN8209_Read_Val(uint32_t *buf)
{
    float energyP = 0;
    
    uint32_t readRegP;
    uint8_t RN8209Reg[5] = {0};
    uint8_t Check[5] = {0};
    RN802_Read_UIP();

    *buf = paraUIP.U;
    *(buf+1) = paraUIP.I;
    *(buf+2) = paraUIP.P;

    RN8209Reg[0] = ADEnergyP;
    FLASH_SPI_IO_Read(RN8209Reg,4);
    
    Check[0] = ADRData;
    FLASH_SPI_IO_Read(Check,5);
    if(memcmp(Check + 2,RN8209Reg + 1,3) == 0)
    {//校验
        RN8209Reg[0] = 0;
        readRegP = Shift_2_32Bit(RN8209Reg);
        energyP = (float)readRegP / 3200;        
    }
    return energyP;
}

uint8_t RN8209_Measure_Adj(uint8_t *buf)
{   
    RN8209_Cal_UIP_K(buf);
    return 0;
}


