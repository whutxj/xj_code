
#ifndef __DRV_MT_CARD_H
#define __DRV_MT_CARD_H

/*****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#ifndef __DRV_MT_CARD_C
#define DRV_MT_CARD_EXTERN  extern
#else
#define DRV_MT_CARD_EXTERN
#endif 

/******************************************************************************
* Includes
******************************************************************************/
#include "drv_usart.h"
#include "bsp_driver_uart.h"
#include <string.h>
#include "Utility.h"
#include "sys_Config.h"
#include <time.h>

/******************************************************************************
* Macros Definitions (constant/Macros)
******************************************************************************/
#define MT_STX   				0x02		/**> 命令头字节 */ 
#define MT_ETX   				0x03		/**> 命令结束字节 */ 
#define MT_RX_TIMEOUT			300		    /**> 读卡超时时间 */ 

#define MT_COM            COM5
/******************************************************************************
* Data Type Definitions
******************************************************************************/
/**< 操作指令类别定义 */
typedef enum
{
    CMD_FIND_CARD_MT			=0x37,	/**> 寻卡 */
    CMD_PRE_PROCESS     		=0x36,	/**> 预处理 */
    CMD_START_CHARGE           	=0x30,	/**> 开始加电 */
    CMD_STOP_CHARGE           	=0x34,	/**> 结束加电 */
    CMD_SUPPLY_TRADE           	=0x35,	/**> 补充交易 */
    CMD_CHECK_VALIDE           	=0x56,	/**> 验证卡片合法性 */
    CMD_SUCCESS                	=0x59,	/**> 成功 */
    CMD_PROCESS_FAIL           	=0x4E,	/**> 加电失败   结束加电失败  预处理过程失败 */
    CMD_PSAM_RECOVER_FAIL      	=0x47,	/**> PSAM复位失败 */
    CMD_USER_CARD_RECOVER_FAIL 	=0x48,	/**> 用户卡复位失败 */
    CMD_OTHER_USER_CHARGING    	=0x44,	/**> 其他用户正在充电 */
	CMD_MAC3_CHECK_FAIL        	=0x43,	/**> MAC3校验失败 */	
}DRV_MTCmdDef;

/**< 状态类别定义 */
typedef enum
{
    TYPE_51			=0x51,	/**> 0x51状态 */
    TYPE_32     	=0x32,	/**> 0x32状态 */
}DRV_MTCmdTypeDef;
/**< 状态类别定义 */
typedef enum
{
    SYSTEM_CARD			=0x30,	/**> 系统卡 */
    NOT_SYSTEM_CARD	    =0x31,	/**> 非系统卡 */
}DRV_MTCardTypeDef;
/**< 状态类别定义 */
typedef enum
{
	MT_DRV_OK 			= 0x00,
	MT_DRV_ERR 		    = 0x01,
	MT_DRV_TIMEOUT      = 0x02,
	MT_DRV_MISMATCH 	= 0x03,
    MT_DRV_CARD_NOT_MATCH    = 0x04,
    MT_MAC3_CHECK_FAIL  = 0x05,
    MT_SUPPLY_FAIL      = 0x06,
}DRV_MTStatusTypeDef;
/******************************************************************************
* Static Function Prototype Definitions
******************************************************************************/
DRV_MT_CARD_EXTERN DRV_MTStatusTypeDef Drv_MT_IC_FindCard(void);
DRV_MT_CARD_EXTERN DRV_MTStatusTypeDef Drv_MT_IC_Preprocess( uint8_t *pPin, uint8_t *buf);
DRV_MT_CARD_EXTERN DRV_MTStatusTypeDef Drv_MT_IC_StartCharge(uint32_t *pCardBalance);
DRV_MT_CARD_EXTERN DRV_MTStatusTypeDef Drv_MT_IC_StopCharge(uint32_t sumConsumption , uint32_t *pCardBalance);
DRV_MT_CARD_EXTERN DRV_MTStatusTypeDef Drv_MT_IC_SupplyTrade(uint8_t *buf);
#ifdef __cplusplus
}
#endif /* extern "C" */

/*****************************************************************************/
#endif /* __DRV_MT_CARD_H */

/******************************************************************************
*                            end of file
******************************************************************************/
