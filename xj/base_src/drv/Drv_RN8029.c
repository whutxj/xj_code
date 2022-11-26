
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
* inition Variable Define  //�Ĵ�����������3���ֽ�  ��ʼ���м�����һ���ֽ�
******************************************************************************/
#define EMUCFG_Mid_Byte     0x20    // ��������Ϊ�����������  
#define EMUCFG_End_Byte     0x04//0x00  ѡ��ڶ�ͨ����Ϊ����ͨ��

#define ModuleEn_Mid_Byte   0x01
#define ModuleEn_End_Byte   0xff

#define ANAEN_End_Byte      0x3f    //0x3f

#define STATUSCFG_Mid_Byte  0x00
#define STATUSCFG_End_Byte  0x02

#define IOCFG_Mid_Byte      0x01
#define IOCFG_End_Byte      0x24

#define ADCCON_Mid_Byte     0x00
#define ADCCON_End_Byte     0x0c

#define DEFAULT_MIN_CUR 64     //�������0.2%    32AΪ��  32*2/1000   *    1000���  =  64
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
    SPI2_Write(*data);   //д�Ĵ�����ַ
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
  * @brief  д����,ֻ�ܲ���40H-45H
  * @retval
  */
static void RN8209_WR_En_4045(void)
{
    uint8_t RN8209Reg[4] = {0};
    RN8209Reg[0] = 0XEA;    //
    //�������ֽ�����  
    RN8209Reg[1] = 0xE5;   //0
    RN8209_SPI_IO_Write(RN8209Reg , 2);
}

/**
  * @brief  ʧ�ܶ�У�����ݵĶ�д��д���0xa6��0xbcֵ����
  * @retval
  */
static void RN8209_WR_Dis(void)
{
    uint8_t RN8209Reg[4] = {0};
    RN8209Reg[0] = 0XEA;    //д���� �Ĵ�����ַ
    //�������ֽ�����  
    RN8209Reg[1] = 0xdc;   //00��ʾ������У��Ĵ������ܽ��в���
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
    if(Emu_Data.K_U > 0.0009 || getPataData.rev != 0xa5a5 || Emu_Data.K_I2 ==0)//���ں�������
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

    RN8209_WR_En_4045();//ʹ��д
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
    
    //ʧ�ܼĴ����Ķ�д
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
  * @brief  4��8Bit --to-- 32Bit
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
  * @brief  ��ȡ��ѹ������������
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
//    P_Flag = RN8209Reg[3];  //0000 0100 NoPLd �������й�Ǳ��ʱ��NoPLd����Ϊ1������������ʱNoPLd��Ϊ0��
    //------����ͨ��1
    RN8209Reg[0] = ADIARMS;
    FLASH_SPI_IO_Read(RN8209Reg,4);
    RN8209Reg[0] = 0;
    Emu_Data.Effective_I1 = (Shift_2_32Bit(RN8209Reg) * Emu_Data.K_I1) * 100;  //����1��Чֵ
    
    //adc
    
    //------����ͨ��1
    RN8209Reg[0] = ADIARMS;
    FLASH_SPI_IO_Read(RN8209Reg,4);
    RN8209Reg[0] = 0;
    Emu_Data.Effective_I2 = (Shift_2_32Bit(RN8209Reg) * Emu_Data.K_I2) * 100;  //����1��Чֵ
    paraUIP.I = Emu_Data.Effective_I2;
    //------��ѹͨ��
    RN8209Reg[0] = ADURMS;
    FLASH_SPI_IO_Read(RN8209Reg,4);
    //У���
    Check[0] = ADRData;
    FLASH_SPI_IO_Read(Check,5);
    if(memcmp(Check + 2,RN8209Reg + 1,3) == 0)
    {   
        RN8209Reg[0] = 0;
        Emu_Data.Effective_U = (Shift_2_32Bit(RN8209Reg) * Emu_Data.K_U) * 100;  //����1��Чֵ
        paraUIP.U = Emu_Data.Effective_U;
    }    

    
    //�й�����
    RN8209Reg[0] = ADPowerPA;
    FLASH_SPI_IO_Read(RN8209Reg,5);
    //У���
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
    //-----���������ж�
//    if(P_Flag & 0x04)
//    {
//        Emu_Data.Effective_I1 = 0;
//        Emu_Data.Effective_P1 = 0;
//        Emu_Data.Effective_P2 = 0;
//        Emu_Data.Power_Factor = 0x1000;
//    }
//    
//    //----���ε���0.2%
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
* @brief  ��ȡ���
  * @retval
  */
//float Get_Err_Data(uint8_t *buf)   //����׼���ʴ�����
//{
//    uint8_t RN8209Reg[4] = {0};
//    uint32_t truePower = 0;
//    float errData = 0;
//    truePower = buf[8] + (buf[9] << 8)+ (buf[10] << 16)+ (buf[11] << 24);//Shift_2_32Bit(&buf[8]);    
//    
//    //���ڹ���
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
//    errData = 1 - (float)((float)Emu_Data.Effective_P2 / truePower);    ///�������   ע��ע��errData��Effective_P1����Чֵλ������
//    return errData;
//}

/**
* @brief  �����ѹ������������ϵ����U:220V   I:XX.XX��
  * @retval
  */
uint8_t RN8209_Cal_UIP_K(uint8_t *buf)
{
    float Rated_I = 0,Rated_U = 0, Rated_P = 0;
    uint8_t RN8209Reg[4] = {0};
    uint32_t dataBuf[3];
    memcpy(dataBuf,buf,12);
   
    {
        Rated_U = (float)dataBuf[0]/ 100; //��ѹ220V
        Rated_I = (float)dataBuf[1] / 100;     //�����  5A
        Rated_P = (float)dataBuf[2] / 100; //���� 1kw.h
        
        RN8209Reg[0] = ADIARMS;
        FLASH_SPI_IO_Read(RN8209Reg,4);
        RN8209Reg[0] = 0;
        Emu_Data.Effective_I2 = Shift_2_32Bit(RN8209Reg);
    //    Emu_Data.K_I1 = (float)(Rated_I / Emu_Data.Effective_I1) * 1000;  //����ϵ��
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
        Emu_Data.K_P2 = Rated_P / p2;  //����ϵ��    �й��޹����ڹ���ϵ������һ����
        saveParaData.saveK_P2 = Emu_Data.K_P2;

        /*
        
            ���棬����ϵ���������洢����
        */
        SPIFLASH_Method1_Save_Record(TYPE_METERPARA,(uint8_t *)&saveParaData);
    }

    return 1;
}


/**
* @brief  ���ô˺�����ȡU  I   P ֵ   ����ȡ����ֵ    �ڵ��ú�ʹ��ʱ����100�ɵ�ʵ��ֵ����ȷ��0.01
  * @retval  ָ������˳��Ϊ  U  I   P      ����󷵻�����ֵ   ������3200��
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
    {//У��
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


