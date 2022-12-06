/******************************************************************************
*  Copyright (C), 1988-2014, Kehua Tech. Co., Ltd.
*  File name:     mbFunction.c
*  Author:        Liu Longjie
*  Version:       V1.00
*  Created on:    2015-1-26
*  Description:   
*  History:
*      Date        Author        Modification
*    1.2015-1-26 
*    2. ...
******************************************************************************/
#ifndef __MBFUNCTION_C
#define __MBFUNCTION_C

/******************************************************************************
* Include Files
******************************************************************************/
#include "mb.h"
#include "mbdatatype.h"
#include "mbconfig.h"
#include "mbproto.h"
#include "mbfunc.h"
#include "mbfunction.h"
#include "ModbusCOM_App.h"
#include "GUI_User_Public.h"

/******************************************************************************
* Macros Definitions (constant/Macros)
******************************************************************************/   
extern UCHAR xMBUtilGetBits( UCHAR * ucByteBuf, USHORT usBitOffset, UCHAR ucNBits );
extern void xMBUtilSetBits( UCHAR * ucByteBuf, USHORT usBitOffset, UCHAR ucNBits,UCHAR ucValue );

/******************************************************************************
* Static Variable Definitions
******************************************************************************/


/* Modbus functions handlers */
xMBFunctionHandler xFuncHandlers[MB_FUNC_HANDLERS_MAX] = 
{
#if MB_FUNC_OTHER_REP_SLAVEID_ENABLED > 0
    {MB_FUNC_OTHER_REPORT_SLAVEID, eMBFuncReportSlaveID},
#endif
#if MB_FUNC_READ_INPUT_ENABLED > 0
    {MB_FUNC_READ_INPUT_REGISTER, eMBFuncReadInputRegister},
#endif
#if MB_FUNC_READ_HOLDING_ENABLED > 0
    {MB_FUNC_READ_HOLDING_REGISTER, eMBFuncReadHoldingRegister},
#endif
#if MB_FUNC_WRITE_MULTIPLE_HOLDING_ENABLED > 0
    {MB_FUNC_WRITE_MULTIPLE_REGISTERS, eMBFuncWriteMultipleHoldingRegister},
#endif
#if MB_FUNC_WRITE_HOLDING_ENABLED > 0
    {MB_FUNC_WRITE_REGISTER, eMBFuncWriteHoldingRegister},
#endif
#if MB_FUNC_READ_COILS_ENABLED > 0                
    {MB_FUNC_READ_COILS, eMBFuncReadCoils},   
#endif
#if MB_FUNC_WRITE_COIL_ENABLED > 0
    {MB_FUNC_WRITE_SINGLE_COIL, eMBFuncWriteCoil},
#endif
#if MB_FUNC_WRITE_MULTIPLE_COILS_ENABLED > 0
    {MB_FUNC_WRITE_MULTIPLE_COILS, eMBFuncWriteMultipleCoils},
#endif
#if MB_FUNC_READ_DISCRETE_INPUTS_ENABLED > 0
    {MB_FUNC_READ_DISCRETE_INPUTS, eMBFuncReadDiscreteInputs},
#endif
};


/******************************************************************************
* Function: eMBErrorCode eMBRegInputCB()
* Parameter:
* Return:   eStatus
* Description:
******************************************************************************/
eMBErrorCode eMBRegInputCB(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs)
{
    eMBErrorCode    eStatus = MB_ENOERR;
    int             iRegIndex;

    if( ( usAddress >= REG_INPUT_START )      //from 1000 to 1004 .lj
        && ( usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS ) )
    {
        iRegIndex = ( int )( usAddress - usRegInputStart );
        while( usNRegs > 0 )
        {
            *pucRegBuffer++ = ( unsigned char )( *usRegInputBuf[iRegIndex] >> 8 );
            *pucRegBuffer++ = ( unsigned char )( *usRegInputBuf[iRegIndex] & 0xFF );
            iRegIndex++;
            usNRegs--;
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}

/******************************************************************************
* Function: eMBErrorCode eMBRegHoldingCB()
* Parameter:
* Return:   eStatus
* Description:
******************************************************************************/
eMBErrorCode eMBRegHoldingCB(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode)
{
    eMBErrorCode    eStatus = MB_ENOERR;
    int             iRegIndex;
    unsigned int    eFunctionCode=0;
//	UCHAR			TestCondition;
	UCHAR			MonthMax;
	void 			*p = NULL;
    if(usNRegs>1)
    {
        eFunctionCode=MB_FUNC_WRITE_MULTIPLE_REGISTERS;     
    }
    else
    {
        eFunctionCode=MB_FUNC_WRITE_REGISTER;
    }
    if( ( usAddress >= REG_HOLDING_START ) &&
        ( usAddress + usNRegs <= REG_HOLDING_START + REG_HOLDING_NREGS ) )
    {
        iRegIndex = ( int )( usAddress - usRegHoldingStart );
        switch ( eMode )
        {
            /* Pass current register values to the protocol stack. */
        case MB_REG_READ:
			/*上位机时间更新*/
			if(usNRegs>0)
			{
				Date_TypeDef  DateNow;
				Time_TypeDef  TimeNow;
				DateNow = BSP_RTC_GetDate();
				TimeNow = BSP_RTC_GetTime();
				MBDataSet.Year = DateNow.Year+2000;
				MBDataSet.Month = DateNow.Month;
				MBDataSet.Day = DateNow.Date;
				MBDataSet.Hour = TimeNow.Hours;
				MBDataSet.Min = TimeNow.Minutes;
				MBDataSet.Sec = TimeNow.Seconds;
			}
            while( usNRegs > 0 )
            {
                *pucRegBuffer++ = ( UCHAR ) ( *usRegHoldingBuf[iRegIndex] >> 8 );
                *pucRegBuffer++ = ( UCHAR ) ( *usRegHoldingBuf[iRegIndex] & 0xFF );
                iRegIndex++;
                usNRegs--;
            }
            break;

            /* Update current register values with new values from the
             * protocol stack. */
        case MB_REG_WRITE:
            while( usNRegs > 0 )
            {
                *usRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
                *usRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
                usNRegs--;
                switch(eFunctionCode)
                {
                    case MB_FUNC_WRITE_REGISTER:
                        {
                            switch(iRegIndex)
                            {
                                case MBBUZZERCTRLREG:
                                    {
										if((*usRegHoldingBuf[iRegIndex])==1)
										{
											if((Inv_RunState.InvRunStatus.Bit.BuzzerDisable==1)&&(SettingCtrl==0))			//蜂鸣器原为禁音则响应
											{
												SettingCtrl=1;
												p = osPoolAlloc(DSPSetMsg_Pool);
												if(p!=NULL)
												{
													MBMessage = (ParaSetMsg*)p;
													MBMessage->Index=SETINDEX_BROAD_BUZZERCTRL;
													MBMessage->CMD_Data[0]=0;		
													MBMessage->CMD_Data[1]=0;
													osMessagePut(osQueue_DSPSetControl,(uint32_t)MBMessage,0);
												}
												SettingCtrl=0;
											}
										} 
										else if((*usRegHoldingBuf[iRegIndex])==2)
										{
											if((Inv_RunState.InvRunStatus.Bit.BuzzerDisable==0)&&(Inv_RunState.InvRunStatus.Bit.BuzzerStatus==1))		//原无禁音且有鸣叫
											{
												if(SettingCtrl==0)
												{
													SettingCtrl=1;
													p = osPoolAlloc(DSPSetMsg_Pool);
													if(p!=NULL)
													{
														MBMessage = (ParaSetMsg*)p;
														MBMessage->Index=SETINDEX_BROAD_BUZZERCTRL;
														MBMessage->CMD_Data[0]=0;		
														MBMessage->CMD_Data[1]=1;
														osMessagePut(osQueue_DSPSetControl,(uint32_t)MBMessage,0);
													}
													SettingCtrl=0;
												}
											}
										}
                                    }
                                    break;
									
								case MBBATTTESTREG:
//                                    {
//                                        if((*usRegHoldingBuf[iRegIndex])>0 && (*usRegHoldingBuf[iRegIndex])<4)
//                                        {
//											if((*usRegHoldingBuf[iRegIndex])==3)
//											{
//												if((Rec_Info.BattTestState&0x0001)!=0) 	//电池测试中才响应取消测试命令
//												{
//													p = osPoolAlloc(DSPSetMsg_Pool);
//													if(p!=NULL)
//													{
//														MBMessage = (ParaSetMsg*)p;
//														MBMessage->Index=SETINDEX_REC_BATTTEST;
//														MBMessage->CMD_Data[0]=(uint8_t)(((*usRegHoldingBuf[iRegIndex])>>8)&0xFF);	
//														MBMessage->CMD_Data[1]=0;
//														osMessagePut(osQueue_DSPSetControl,(uint32_t)MBMessage,0);
//													}
//												}
//											}
//											else
//											{
//												TestCondition=GetTestCondition();
//												if(TestCondition==1)             		//满足条件才测试
//												{
//													p = osPoolAlloc(DSPSetMsg_Pool);
//													if(p!=NULL)
//													{
//														MBMessage = (ParaSetMsg*)p;
//														MBMessage->Index=SETINDEX_REC_BATTTEST;
//														MBMessage->CMD_Data[0]=(uint8_t)(((*usRegHoldingBuf[iRegIndex])>>8)&0xFF);	
//														MBMessage->CMD_Data[1]=(uint8_t)((*usRegHoldingBuf[iRegIndex])&0xFF);
//														osMessagePut(osQueue_DSPSetControl,(uint32_t)MBMessage,0);
//													}
//												}
//											}
//                                        }      
//										MBDataSet.BattTest = 0;				//触发类参数，下发后立即置0
//                                    }
                                    break;
									
								case MBEQUALCHARGEVOLTREG:
//									{
//                                        if((*usRegHoldingBuf[iRegIndex])>=12000 && (*usRegHoldingBuf[iRegIndex])<=15500)
//                                        {
//                                            MBMessage = (ParaSetMsg*) osPoolAlloc(DSPSetMsg_Pool);
//                                            MBMessage->Index=SETINDEX_REC_EQUALVOLT;
//                                            MBMessage->CMD_Data[1]=(uint8_t)((*usRegHoldingBuf[iRegIndex])&0xFF); 
//											  MBMessage->CMD_Data[0]=(uint8_t)(((*usRegHoldingBuf[iRegIndex])>>8)&0xFF);											
//                                            osMessagePut(osQueue_DSPSetControl,(uint32_t)MBMessage,0);
//                                        }   
//										else
//										{
//											MBDataSet.EqualChargeVoltCell = Rec_Setting_Data.EqualChargeVolt.Para.Para16b;
//										}
//                                    }
                                    break;
									
								case MBFLOATCHARGEVOLTREG:
//									{
//                                        if((*usRegHoldingBuf[iRegIndex])>=12000 && (*usRegHoldingBuf[iRegIndex])<=14500)
//                                        {
//                                            MBMessage = (ParaSetMsg*) osPoolAlloc(DSPSetMsg_Pool);
//                                            MBMessage->Index=SETINDEX_REC_FLOATVOLT;
//                                            MBMessage->CMD_Data[1]=(uint8_t)((*usRegHoldingBuf[iRegIndex])&0xFF); 
//											MBMessage->CMD_Data[0]=(uint8_t)(((*usRegHoldingBuf[iRegIndex])>>8)&0xFF);											
//                                            osMessagePut(osQueue_DSPSetControl,(uint32_t)MBMessage,0);
//                                        }      
//										else
//										{
//											MBDataSet.FloatChargeVoltCell = Rec_Setting_Data.FloatingChargeVolt.Para.Para16b;
//										}
//                                    }
                                    break;
                                    
								case MBTEMPCOMPENREG:
//                                    {
//                                        if((*usRegHoldingBuf[iRegIndex])>=2 && (*usRegHoldingBuf[iRegIndex])<=4)
//                                        {
//                                            MBMessage = (ParaSetMsg*) osPoolAlloc(DSPSetMsg_Pool);
//                                            MBMessage->Index=SETINDEX_REC_TEMPCOMPENSATION;
//                                            MBMessage->CMD_Data[1]=(uint8_t)((*usRegHoldingBuf[iRegIndex]-2)&0xFF); 
//											MBMessage->CMD_Data[0]=(uint8_t)(((*usRegHoldingBuf[iRegIndex]-2)>>8)&0xFF);											
//                                            osMessagePut(osQueue_DSPSetControl,(uint32_t)MBMessage,0);
//                                        }       
//										else
//										{
//											MBDataSet.TempCompensation = Rec_Setting_Data.TempCompensation.Para.Para16b+2;
//										}										
//                                    }
									
                                    break;
                                    
									
								 default:
									 break;
                            }
                        }
                        break;
                    case MB_FUNC_WRITE_MULTIPLE_REGISTERS:
                        {
                            switch(iRegIndex)
                            {
                                case MBTIMEREG:
                                    {
                                        Date_TypeDef  MBDateSet;
                                        Time_TypeDef  MBTimeSet;
 
                                        if(*usRegHoldingBuf[iRegIndex-5]<2000)
										{
											MBDateSet.Year=(UCHAR)2000;
										}
										else if(*usRegHoldingBuf[iRegIndex-5]>2099)
										{
											MBDateSet.Year=(UCHAR)2099;
										}
										else
										{
											MBDateSet.Year=(UCHAR)(*usRegHoldingBuf[iRegIndex-5]-2000);
										}
                                        MBDateSet.Month=(UCHAR)(*usRegHoldingBuf[iRegIndex-4]);
                                        MBDateSet.Date=(UCHAR)(*usRegHoldingBuf[iRegIndex-3]);
                                        MBTimeSet.Hours=(UCHAR)(*usRegHoldingBuf[iRegIndex-2]);
                                        MBTimeSet.Minutes=(UCHAR)(*usRegHoldingBuf[iRegIndex-1]);
                                        MBTimeSet.Seconds=(UCHAR)(*usRegHoldingBuf[iRegIndex]);
                                                
                                        if(MBTimeSet.Seconds<60 
                                           && MBTimeSet.Minutes<60 
                                           && MBTimeSet.Hours<24                                          
                                           && MBDateSet.Month<13 
                                           && MBDateSet.Month>0 
                                           && MBDateSet.Date>0 
                                           && MBDateSet.Year<=99)
                                        {
											//日期合法判断
											if(MBDateSet.Month == 2)
											{
												if ((((MBDateSet.Year%4)==0) && ((MBDateSet.Year%100)!=0)) || ((MBDateSet.Year%400)==0 ))
												{
													MonthMax = 29;
												}
												else
												{
													MonthMax = 28;
												}
											}
											else if((MBDateSet.Month == 4) || (MBDateSet.Month == 6) || (MBDateSet.Month == 9) || (MBDateSet.Month == 11))
											{
												MonthMax = 30;
											}
											else
											{
												MonthMax = 31;
											} 
											if(MBDateSet.Date<=MonthMax)
											{
												BSP_RTC_CalendarConfig(MBDateSet,MBTimeSet);
                                            }
                                        }
                                    }
                                    break;
                            }
                        }
                        break;
                    default:
                        break;
                }
				iRegIndex++;
            }
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}

/******************************************************************************
* Function: eMBErrorCode eMBRegCoilsCB()
* Parameter:Void
* Return:   MB_ENOREG
* Description:
******************************************************************************/
eMBErrorCode eMBRegCoilsCB(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils, eMBRegisterMode eMode)
{       
	eMBErrorCode    eStatus = MB_ENOERR;
    int             iNCoils = ( int )usNCoils;
    unsigned short  usBitOffset;
	unsigned short  uFuctionCode;
	void 			*p = NULL;
    if(usNCoils>1)
    {
        uFuctionCode=MB_FUNC_WRITE_MULTIPLE_COILS;
    }
    else
    {
        uFuctionCode=MB_FUNC_WRITE_SINGLE_COIL;
    }
    
    /* Check if we have registers mapped at this block. */
    if( ( usAddress >= REG_COIL_START ) &&
        ( usAddress + usNCoils <= REG_COIL_START + REG_COIL_NREGS ) )
    {
        usBitOffset = ( unsigned short )( usAddress - REG_COIL_START );
        switch ( eMode )
        {
                /* Read current values and pass to protocol stack. */
            case MB_REG_READ:
                while( iNCoils > 0 )
                {
                    *pucRegBuffer++ =
                        xMBUtilGetBits( usRegCoilBuf[0], usBitOffset,
                                        ( unsigned char )( iNCoils >
                                                           8 ? 8 :
                                                           iNCoils ) );
                    iNCoils -= 8;
                    usBitOffset += 8;
										
                }
                break;

                /* Update current register values. */
            case MB_REG_WRITE:
                while( iNCoils > 0 )
                {
                    xMBUtilSetBits( usRegCoilBuf[0], usBitOffset, 
                                    ( unsigned char )( iNCoils > 8 ? 8 : iNCoils ),
                                    *pucRegBuffer++ );
                    switch(uFuctionCode)
                    {
                        case MB_FUNC_WRITE_SINGLE_COIL:
                            {
                                switch(usBitOffset)
                                {
                                    case 0:
                                    {
										p = osPoolAlloc(DSPSetMsg_Pool);
										if(p!=NULL)
										{
											MBMessage = (ParaSetMsg*)p;
											MBMessage->Index=SETINDEX_INV_POWERONOFF;
											if(((*(usRegCoilBuf[0])) & 0x01)==1)
											{
												MBMessage->CMD_Data[0]=0;   
												MBMessage->CMD_Data[1]=1;   
											}
											else
											{
												MBMessage->CMD_Data[0]=0;
												MBMessage->CMD_Data[1]=0;   
											}
											osMessagePut(osQueue_DSPSetControl,(uint32_t)MBMessage,0);
										}
                                    }
                                    break;
                                }
                            }
                            break;
                        case MB_FUNC_WRITE_MULTIPLE_COILS:
                            break;
                    }
                    iNCoils -= 8;
                    usBitOffset += 8;
									
                }
                break;
        }

    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus; 
}


/******************************************************************************
* Function: eMBErrorCode eMBRegDiscreteCB()
* Parameter:Void
* Return:   MB_ENOREG
* Description:
******************************************************************************/
eMBErrorCode eMBRegDiscreteCB(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete)
{
	  eMBErrorCode    eStatus = MB_ENOERR;
    short           iNDiscrete = ( short )usNDiscrete;
    unsigned short  usBitOffset;

    /* Check if we have registers mapped at this block. */
    if( ( usAddress >= REG_DISC_START_STANDARD ) &&
        ( usAddress + usNDiscrete <= REG_DISC_START_STANDARD + REG_DISC_NREGS_STANDARD ))
    {
        usBitOffset = ( unsigned short )( usAddress - REG_DISC_START_STANDARD );
        while( iNDiscrete > 0 )
        {
            *pucRegBuffer++ =
                xMBUtilGetBits( usRegDiscBufStandard[0], usBitOffset,
                                ( unsigned char )( iNDiscrete >
                                                   8 ? 8 : iNDiscrete ) );
            iNDiscrete -= 8;
            usBitOffset += 8;
        }
    }
	else if( ( usAddress >= REG_DISC_START_EXPAND ) &&
        ( usAddress + usNDiscrete <= REG_DISC_START_EXPAND + REG_DISC_NREGS_EXPAND ))
    {
        usBitOffset = ( unsigned short )( usAddress - REG_DISC_START_EXPAND );
        while( iNDiscrete > 0 )
        {
            *pucRegBuffer++ =
                xMBUtilGetBits( usRegDiscBufExpand[0], usBitOffset,
                                ( unsigned char )( iNDiscrete >
                                                   8 ? 8 : iNDiscrete ) );
            iNDiscrete -= 8;
            usBitOffset += 8;
        }
    }
	else if( ( usAddress >= REG_DISC_START_INNER ) &&
        ( usAddress + usNDiscrete <= REG_DISC_START_INNER + REG_DISC_NREGS_INNER ))
    {
        usBitOffset = ( unsigned short )( usAddress - REG_DISC_START_INNER );
        while( iNDiscrete > 0 )
        {
            *pucRegBuffer++ =
                xMBUtilGetBits( usRegDiscBufInner[0], usBitOffset,
                                ( unsigned char )( iNDiscrete >
                                                   8 ? 8 : iNDiscrete ) );
            iNDiscrete -= 8;
            usBitOffset += 8;
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus; 
}

UCHAR eMBGetCoilAddrSector(USHORT usAddress)
{
	UCHAR temp;
	if((usAddress>=REG_COIL_START) && usAddress<(REG_COIL_START+16))
	{
		temp=0;
	}
  if((usAddress>=REG_COIL_START+16) && usAddress<(REG_COIL_START+32))
	{
		temp=1;
	}
	if((usAddress>=REG_COIL_START+32) && usAddress<(REG_COIL_START+48))
	{
		temp=2;
	}
	if((usAddress>=REG_COIL_START+48) && usAddress<(REG_COIL_START+64))
	{
		temp=3;
	}
	if((usAddress>=REG_COIL_START+64) && usAddress<(REG_COIL_START+80))
	{
		temp=4;
	}
	if((usAddress>=REG_COIL_START+80) && usAddress<(REG_COIL_START+96))
	{
		temp=5;
	}
	if((usAddress>=REG_COIL_START+96) && usAddress<(REG_COIL_START+112))
	{
		temp=6;
	}
	if((usAddress>=REG_COIL_START+112) && usAddress<(REG_COIL_START+128))
	{
		temp=7;
	}
	if((usAddress>=REG_COIL_START+128) && usAddress<(REG_COIL_START+144))
	{
		temp=8;
	}
	return temp;
}

#endif /*__MBFUNCTION_C */
/******************************************************************************
*                            end of file
******************************************************************************/
