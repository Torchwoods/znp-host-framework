/**************************************************************************************************
 * Filename:       cmdLine.c
 * Description:    This file contains cmdLine application.
 *
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*********************************************************************
 * INCLUDES
 */
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "rpc.h"
#include "mtSys.h"
#include "mtZdo.h"
#include "mtAf.h"
#include "mtParser.h"
#include "mtSapi.h"
#include "rpcTransport.h"
#include "dbgPrint.h"
#include "hostConsole.h"

/*********************************************************************
 * MACROS
 */
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#define SET_RSP_COLOR(); consolePrint("%s", KYEL);
#define SET_HELP_COLOR(); consolePrint("%s", KGRN);
#define SET_PARAM_COLOR(); consolePrint("%s", KCYN);
#define SET_NRM_COLOR(); consolePrint("%s", KNRM);

/*********************************************************************
 * TYPES
 */

/*********************************************************************
 * LOCAL VARIABLE
 */

//init ZDO device state
devStates_t devState = DEV_HOLD;
uint8_t gSrcEndPoint = 1;
uint8_t gDstEndPoint = 1;

/***********************************************************************/

void usage(char* exeName)
{
	consolePrint("Usage: ./%s <port>\n", exeName);
	consolePrint("Eample: ./%s /dev/ttyACM0\n", exeName);
}

/*********************************************************************
 * LOCAL FUNCTIONS
 */
//ZDO Callbacks
static uint8_t mtZdoStateChangeIndCb(uint8_t newDevState);
static uint8_t mtZdoGetLinkKeyCb(GetLinkKeySrspFormat_t *msg);
static uint8_t mtZdoNwkAddrRspCb(NwkAddrRspFormat_t *msg);
static uint8_t mtZdoIeeeAddrRspCb(IeeeAddrRspFormat_t *msg);
static uint8_t mtZdoNodeDescRspCb(NodeDescRspFormat_t *msg);
static uint8_t mtZdoPowerDescRspCb(PowerDescRspFormat_t *msg);
static uint8_t mtZdoSimpleDescRspCb(SimpleDescRspFormat_t *msg);
static uint8_t mtZdoActiveEpRspCb(ActiveEpRspFormat_t *msg);
static uint8_t mtZdoMatchDescRspCb(MatchDescRspFormat_t *msg);
static uint8_t mtZdoComplexDescRspCb(ComplexDescRspFormat_t *msg);
static uint8_t mtZdoUserDescRspCb(UserDescRspFormat_t *msg);
static uint8_t mtZdoUserDescConfCb(UserDescConfFormat_t *msg);
static uint8_t mtZdoServerDiscRspCb(ServerDiscRspFormat_t *msg);
static uint8_t mtZdoEndDeviceBindRspCb(EndDeviceBindRspFormat_t *msg);
static uint8_t mtZdoBindRspCb(BindRspFormat_t *msg);
static uint8_t mtZdoUnbindRspCb(UnbindRspFormat_t *msg);
static uint8_t mtZdoMgmtNwkDiscRspCb(MgmtNwkDiscRspFormat_t *msg);
static uint8_t mtZdoMgmtLqiRspCb(MgmtLqiRspFormat_t *msg);
static uint8_t mtZdoMgmtRtgRspCb(MgmtRtgRspFormat_t *msg);
static uint8_t mtZdoMgmtBindRspCb(MgmtBindRspFormat_t *msg);
static uint8_t mtZdoMgmtLeaveRspCb(MgmtLeaveRspFormat_t *msg);
static uint8_t mtZdoMgmtDirectJoinRspCb(MgmtDirectJoinRspFormat_t *msg);
static uint8_t mtZdoMgmtPermitJoinRspCb(MgmtPermitJoinRspFormat_t *msg);
static uint8_t mtZdoEndDeviceAnnceIndCb(EndDeviceAnnceIndFormat_t *msg);
static uint8_t mtZdoMatchDescRspSentCb(MatchDescRspSentFormat_t *msg);
static uint8_t mtZdoStatusErrorRspCb(StatusErrorRspFormat_t *msg);
static uint8_t mtZdoSrcRtgIndCb(SrcRtgIndFormat_t *msg);
static uint8_t mtZdoBeaconNotifyIndCb(BeaconNotifyIndFormat_t *msg);
static uint8_t mtZdoJoinCnfCb(JoinCnfFormat_t *msg);
static uint8_t mtZdoNwkDiscoveryCnfCb(NwkDiscoveryCnfFormat_t *msg);
static uint8_t mtZdoLeaveIndCb(LeaveIndFormat_t *msg);
static uint8_t mtZdoMsgCbIncomingCb(MsgCbIncomingFormat_t *msg);

//SYS Callbacks
//static uint8_t mtSysResetInd(uint8_t resetReason, uint8_t version[5]);
static uint8_t mtSysPingSrspCb(PingSrspFormat_t *msg);
static uint8_t mtSysGetExtAddrSrspCb(GetExtAddrSrspFormat_t *msg);
static uint8_t mtSysRamReadSrspCb(RamReadSrspFormat_t *msg);
static uint8_t mtSysResetIndCb(ResetIndFormat_t *msg);
static uint8_t mtSysVersionSrspCb(VersionSrspFormat_t *msg);
static uint8_t mtSysOsalNvReadSrspCb(OsalNvReadSrspFormat_t *msg);
static uint8_t mtSysOsalNvLengthSrspCb(OsalNvLengthSrspFormat_t *msg);
static uint8_t mtSysOsalTimerExpiredCb(OsalTimerExpiredFormat_t *msg);
static uint8_t mtSysStackTuneSrspCb(StackTuneSrspFormat_t *msg);
static uint8_t mtSysAdcReadSrspCb(AdcReadSrspFormat_t *msg);
static uint8_t mtSysGpioSrspCb(GpioSrspFormat_t *msg);
static uint8_t mtSysRandomSrspCb(RandomSrspFormat_t *msg);
static uint8_t mtSysGetTimeSrspCb(GetTimeSrspFormat_t *msg);
static uint8_t mtSysSetTxPowerSrspCb(SetTxPowerSrspFormat_t *msg);

//AF callbacks
static uint8_t mtAfDataConfirmCb(DataConfirmFormat_t *msg);
static uint8_t mtAfIncomingMsgCb(IncomingMsgFormat_t *msg);
static uint8_t mtAfIncomingMsgExt(IncomingMsgExtFormat_t *msg);
static uint8_t mtAfDataRetrieveSrspCb(DataRetrieveSrspFormat_t *msg);
static uint8_t mtAfReflectErrorCb(ReflectErrorFormat_t *msg);

//SAPI Callbacks
static uint8_t mtSapiReadConfigurationSrspCb(ReadConfigurationSrspFormat_t *msg);
static uint8_t mtSapiGetDeviceInfoSrspCb(GetDeviceInfoSrspFormat_t *msg);
static uint8_t mtSapiFindDeviceCnfCb(FindDeviceCnfFormat_t *msg);
static uint8_t mtSapiSendDataCnfCb(SendDataCnfFormat_t *msg);
static uint8_t mtSapiReceiveDataIndCb(ReceiveDataIndFormat_t *msg);
static uint8_t mtSapiAllowBindCnfCb(AllowBindCnfFormat_t *msg);
static uint8_t mtSapiBindCnfCb(BindCnfFormat_t *msg);
static uint8_t mtSapiStartCnfCb(StartCnfFormat_t *msg);

//helper functions
static uint8_t setNVStartup(uint8_t startupOption);
static uint8_t setNVChanList(uint32_t chanList);
static uint8_t setNVPanID(uint32_t panId);
static uint8_t setNVDevType(uint8_t devType);
static int32_t startNetwork(void);
static int32_t registerAf(void);

typedef struct
{
	char *name;//参数名称
	uint8_t size;
	uint8_t isList;
} cmdAtt_t;

typedef struct
{
	char *cmdName;//命令名称
	char *cmdDesc;//命令描述
	uint8_t attNum;//命令参数个数
	cmdAtt_t atts[20]; //参数内容
} cmd_t;
#define COMMANDS_SIZE  72

cmd_t commands[COMMANDS_SIZE];

/*********************************************************************
 * CALLBACK FUNCTIONS
 */

// SYS callbacks
static mtSysCb_t mtSysCb =
{ 
	mtSysPingSrspCb, 
	mtSysGetExtAddrSrspCb, 
	mtSysRamReadSrspCb,
	mtSysResetIndCb, 
	mtSysVersionSrspCb,
	mtSysOsalNvReadSrspCb,
	mtSysOsalNvLengthSrspCb,
	mtSysOsalTimerExpiredCb,
	mtSysStackTuneSrspCb,
	mtSysAdcReadSrspCb,
	mtSysGpioSrspCb,
	mtSysRandomSrspCb, 
	mtSysGetTimeSrspCb, 
	mtSysSetTxPowerSrspCb 
};

static mtZdoCb_t mtZdoCb =
{ 
	mtZdoNwkAddrRspCb,       // MT_ZDO_NWK_ADDR_RSP
	mtZdoIeeeAddrRspCb,      // MT_ZDO_IEEE_ADDR_RSP
	mtZdoNodeDescRspCb,      // MT_ZDO_NODE_DESC_RSP
	mtZdoPowerDescRspCb,     // MT_ZDO_POWER_DESC_RSP
	mtZdoSimpleDescRspCb,    // MT_ZDO_SIMPLE_DESC_RSP
	mtZdoActiveEpRspCb,      // MT_ZDO_ACTIVE_EP_RSP
	mtZdoMatchDescRspCb,     // MT_ZDO_MATCH_DESC_RSP
	mtZdoComplexDescRspCb,   // MT_ZDO_COMPLEX_DESC_RSP
	mtZdoUserDescRspCb,      // MT_ZDO_USER_DESC_RSP
	mtZdoUserDescConfCb,     // MT_ZDO_USER_DESC_CONF
	mtZdoServerDiscRspCb,    // MT_ZDO_SERVER_DISC_RSP
	mtZdoEndDeviceBindRspCb, // MT_ZDO_END_DEVICE_BIND_RSP
	mtZdoBindRspCb,          // MT_ZDO_BIND_RSP
	mtZdoUnbindRspCb,        // MT_ZDO_UNBIND_RSP
	mtZdoMgmtNwkDiscRspCb,   // MT_ZDO_MGMT_NWK_DISC_RSP
	mtZdoMgmtLqiRspCb,       // MT_ZDO_MGMT_LQI_RSP
	mtZdoMgmtRtgRspCb,       // MT_ZDO_MGMT_RTG_RSP
	mtZdoMgmtBindRspCb,      // MT_ZDO_MGMT_BIND_RSP
	mtZdoMgmtLeaveRspCb,     // MT_ZDO_MGMT_LEAVE_RSP
	mtZdoMgmtDirectJoinRspCb,     // MT_ZDO_MGMT_DIRECT_JOIN_RSP
	mtZdoMgmtPermitJoinRspCb,     // MT_ZDO_MGMT_PERMIT_JOIN_RSP
	mtZdoStateChangeIndCb,   // MT_ZDO_STATE_CHANGE_IND
	mtZdoEndDeviceAnnceIndCb,   // MT_ZDO_END_DEVICE_ANNCE_IND
	mtZdoSrcRtgIndCb,        // MT_ZDO_SRC_RTG_IND
	mtZdoBeaconNotifyIndCb,	 //MT_ZDO_BEACON_NOTIFY_IND
	mtZdoJoinCnfCb,			 //MT_ZDO_JOIN_CNF
	mtZdoNwkDiscoveryCnfCb,	 //MT_ZDO_NWK_DISCOVERY_CNF
	NULL,                    // MT_ZDO_CONCENTRATOR_IND_CB
	mtZdoLeaveIndCb,         // MT_ZDO_LEAVE_IND
	mtZdoStatusErrorRspCb,   //MT_ZDO_STATUS_ERROR_RSP
	mtZdoMatchDescRspSentCb,  //MT_ZDO_MATCH_DESC_RSP_SENT
	mtZdoMsgCbIncomingCb, mtZdoGetLinkKeyCb 
};

static mtAfCb_t mtAfCb =
{ 
	mtAfDataConfirmCb,				//MT_AF_DATA_CONFIRM
	mtAfIncomingMsgCb,				//MT_AF_INCOMING_MSG
	mtAfIncomingMsgExt,				//MT_AF_INCOMING_MSG_EXT
	mtAfDataRetrieveSrspCb,			//MT_AF_DATA_RETRIEVE
	mtAfReflectErrorCb,			    //MT_AF_REFLECT_ERROR
};

// SAPI callbacks
static mtSapiCb_t mtSapiCb =
{ 
	mtSapiReadConfigurationSrspCb,				//MT_SAPI_READ_CONFIGURATION
	mtSapiGetDeviceInfoSrspCb,				//MT_SAPI_GET_DEVICE_INFO
	mtSapiFindDeviceCnfCb,				//MT_SAPI_FIND_DEVICE_CNF
	mtSapiSendDataCnfCb,				//MT_SAPI_SEND_DATA_CNF
	mtSapiReceiveDataIndCb,				//MT_SAPI_RECEIVE_DATA_IND
	mtSapiAllowBindCnfCb,				//MT_SAPI_ALLOW_BIND_CNF
	mtSapiBindCnfCb,				//MT_SAPI_BIND_CNF
	mtSapiStartCnfCb,				//MT_SAPI_START_CNF
};

static void InitCmds(void)
{

	commands[0].cmdName = "SYS_PING";
	commands[0].cmdDesc =
	        " This command issues PING requests to verify if a device is active and check the\n"
			        " capability of the device.\n";
	commands[0].attNum = 0;
	commands[1].cmdName = "SYS_SET_EXTADDR";
	commands[1].cmdDesc =
	        " This command is used to set the extended address of the device.\n";
	commands[1].attNum = 1;
	commands[1].atts[0].name = "ExtAddr";
	commands[1].atts[0].size = 8;
	commands[1].atts[0].isList = 0;
	commands[2].cmdName = "SYS_GET_EXTADDR";
	commands[2].cmdDesc =
	        " This command is used to get the extended address of the device.\n";
	commands[2].attNum = 0;
	commands[3].cmdName = "SYS_RAM_READ";
	commands[3].cmdDesc =
	        " This command is used by the tester to read a single memory location in the target\n"
			        " RAM. The command accepts an address value and returns the memory value present in\n"
			        " the target RAM at that address.\n";
	commands[3].attNum = 2;
	commands[3].atts[0].name = "Address";
	commands[3].atts[0].size = 2;
	commands[3].atts[0].isList = 0;
	commands[3].atts[1].name = "Len";
	commands[3].atts[1].size = 1;
	commands[3].atts[1].isList = 0;
	commands[4].cmdName = "SYS_RAM_WRITE";
	commands[4].cmdDesc =
	        " This command is used by the tester to write to a particular location in the target\n"
			        " RAM. The command accepts an address location and a memory value. The memory value\n"
			        " is written to the address location in the target RAM.\n";
	commands[4].attNum = 3;
	commands[4].atts[0].name = "Address";
	commands[4].atts[0].size = 2;
	commands[4].atts[0].isList = 0;
	commands[4].atts[1].name = "Len";
	commands[4].atts[1].size = 1;
	commands[4].atts[1].isList = 0;
	commands[4].atts[2].name = "Value";
	commands[4].atts[2].size = 1;
	commands[4].atts[2].isList = 128;
	commands[5].cmdName = "SYS_RESET_REQ";
	commands[5].cmdDesc =
	        " This command is sent by the tester to reset the target device.\n";
	commands[5].attNum = 1;
	commands[5].atts[0].name = "Type";
	commands[5].atts[0].size = 1;
	commands[5].atts[0].isList = 0;
	commands[6].cmdName = "SYS_VERSION";
	commands[6].cmdDesc =
	        " This command is used to request for the device\'s version string.\n";
	commands[6].attNum = 0;
	commands[7].cmdName = "SYS_OSAL_NV_READ";
	commands[7].cmdDesc =
	        " This command is used by the tester to read a single memory item from the target\n"
			        " non-volatile memory. The command accepts an attribute Id value and data offset and\n"
			        " returns the memory value present in the target for the specified attribute Id.\n";
	commands[7].attNum = 2;
	commands[7].atts[0].name = "Id";
	commands[7].atts[0].size = 2;
	commands[7].atts[0].isList = 0;
	commands[7].atts[1].name = "Offset";
	commands[7].atts[1].size = 1;
	commands[7].atts[1].isList = 0;
	commands[8].cmdName = "SYS_OSAL_NV_WRITE";
	commands[8].cmdDesc =
	        " This command is used by the tester to write to a particular item in non-volatile\n"
			        " memory. The command accepts an attribute Id, data offset, data length, and attribute\n"
			        " value. The attribute value is written to the location specified for the attribute\n"
			        " Id in the target.\n";
	commands[8].attNum = 4;
	commands[8].atts[0].name = "Id";
	commands[8].atts[0].size = 2;
	commands[8].atts[0].isList = 0;
	commands[8].atts[1].name = "Offset";
	commands[8].atts[1].size = 1;
	commands[8].atts[1].isList = 0;
	commands[8].atts[2].name = "Len";
	commands[8].atts[2].size = 1;
	commands[8].atts[2].isList = 0;
	commands[8].atts[3].name = "Value";
	commands[8].atts[3].size = 1;
	commands[8].atts[3].isList = 246;
	commands[9].cmdName = "SYS_OSAL_NV_ITEM_INIT";
	commands[9].cmdDesc =
	        " This command is used by the tester to write to a particular item in non-volatile\n"
			        " memory. The command accepts an attribute Id, data offset, data length, and attribute\n"
			        " value. The attribute value is written to the location specified for the attribute\n"
			        " Id in the target.\n";
	commands[9].attNum = 4;
	commands[9].atts[0].name = "Id";
	commands[9].atts[0].size = 2;
	commands[9].atts[0].isList = 0;
	commands[9].atts[1].name = "ItemLen";
	commands[9].atts[1].size = 2;
	commands[9].atts[1].isList = 0;
	commands[9].atts[2].name = "InitLen";
	commands[9].atts[2].size = 1;
	commands[9].atts[2].isList = 0;
	commands[9].atts[3].name = "InitData";
	commands[9].atts[3].size = 1;
	commands[9].atts[3].isList = 245;
	commands[10].cmdName = "SYS_OSAL_NV_DELETE";
	commands[10].cmdDesc =
	        " This command is used by the tester to delete an item from the non-volatile memory.\n"
			        " The ItemLen parameter must match the length of the NV item or the command will fail.\n"
			        " Use this command with caution â€“ deleted items cannot be recovered.\n";
	commands[10].attNum = 2;
	commands[10].atts[0].name = "Id";
	commands[10].atts[0].size = 2;
	commands[10].atts[0].isList = 0;
	commands[10].atts[1].name = "ItemLen";
	commands[10].atts[1].size = 2;
	commands[10].atts[1].isList = 0;
	commands[11].cmdName = "SYS_OSAL_NV_LENGTH";
	commands[11].cmdDesc =
	        " This command is used by the tester to get the length of an item in non-volatile\n"
			        " memory. A returned length of zero indicates that the NV item does not exist.\n";
	commands[11].attNum = 1;
	commands[11].atts[0].name = "Id";
	commands[11].atts[0].size = 2;
	commands[11].atts[0].isList = 0;
	commands[12].cmdName = "SYS_OSAL_START_TIMER";
	commands[12].cmdDesc =
	        " This command is used by the tester to start a timer event. The event will expired\n"
			        " after the indicated amount of time and a notification will be sent back to the tester.\n";
	commands[12].attNum = 2;
	commands[12].atts[0].name = "Id";
	commands[12].atts[0].size = 1;
	commands[12].atts[0].isList = 0;
	commands[12].atts[1].name = "Timeout";
	commands[12].atts[1].size = 2;
	commands[12].atts[1].isList = 0;
	commands[13].cmdName = "SYS_OSAL_STOP_TIMER";
	commands[13].cmdDesc =
	        " This command is used by the tester to stop a timer event.\n";
	commands[13].attNum = 1;
	commands[13].atts[0].name = "Id";
	commands[13].atts[0].size = 1;
	commands[13].atts[0].isList = 0;
	commands[14].cmdName = "SYS_STACK_TUNE";
	commands[14].cmdDesc =
	        " This command is used by the tester to tune intricate or arcane settings at runtime.\n";
	commands[14].attNum = 2;
	commands[14].atts[0].name = "Operation";
	commands[14].atts[0].size = 1;
	commands[14].atts[0].isList = 0;
	commands[14].atts[1].name = "Value";
	commands[14].atts[1].size = 1;
	commands[14].atts[1].isList = 0;
	commands[15].cmdName = "SYS_ADC_READ";
	commands[15].cmdDesc =
	        " This command reads a value from the ADC based on specified channel and resolution.\n";
	commands[15].attNum = 2;
	commands[15].atts[0].name = "Channel";
	commands[15].atts[0].size = 1;
	commands[15].atts[0].isList = 0;
	commands[15].atts[1].name = "Resolution";
	commands[15].atts[1].size = 1;
	commands[15].atts[1].isList = 0;
	commands[16].cmdName = "SYS_GPIO";
	commands[16].cmdDesc =
	        " This command is used by the tester to control the GPIO pins.\n";
	commands[16].attNum = 2;
	commands[16].atts[0].name = "Operation";
	commands[16].atts[0].size = 1;
	commands[16].atts[0].isList = 0;
	commands[16].atts[1].name = "Value";
	commands[16].atts[1].size = 1;
	commands[16].atts[1].isList = 0;
	commands[17].cmdName = "SYS_RANDOM";
	commands[17].cmdDesc =
	        " This command is used by the tester to get a random 16-bit number.\n";
	commands[17].attNum = 0;
	commands[18].cmdName = "SYS_SET_TIME";
	commands[18].cmdDesc =
	        " This command is used by the tester to set the target system date and time. The time\n"
			        " can be specified in seconds since 00:00:00 on January 1, 2000 or in parsed\n"
			        " date/time components.\n";
	commands[18].attNum = 7;
	commands[18].atts[0].name = "UTCTime";
	commands[18].atts[0].size = 4;
	commands[18].atts[0].isList = 0;
	commands[18].atts[1].name = "Hour";
	commands[18].atts[1].size = 1;
	commands[18].atts[1].isList = 0;
	commands[18].atts[2].name = "Minute";
	commands[18].atts[2].size = 1;
	commands[18].atts[2].isList = 0;
	commands[18].atts[3].name = "Second";
	commands[18].atts[3].size = 1;
	commands[18].atts[3].isList = 0;
	commands[18].atts[4].name = "Month";
	commands[18].atts[4].size = 1;
	commands[18].atts[4].isList = 0;
	commands[18].atts[5].name = "Day";
	commands[18].atts[5].size = 1;
	commands[18].atts[5].isList = 0;
	commands[18].atts[6].name = "Year";
	commands[18].atts[6].size = 2;
	commands[18].atts[6].isList = 0;
	commands[19].cmdName = "SYS_GET_TIME";
	commands[19].cmdDesc =
	        " This command is used by the tester to get the target system date and time. The time\n"
			        " is returned in seconds since 00:00:00 on January 1, 2000 and parsed date/time\n"
			        " components.\n";
	commands[19].attNum = 0;
	commands[20].cmdName = "SYS_SET_TX_POWER";
	commands[20].cmdDesc =
	        " This command is used by the tester to set the target system radio transmit power.\n"
			        " The returned TX power is the actual setting applied to the radio â€“ nearest characterized\n"
			        " value for the specific radio.\n";
	commands[20].attNum = 1;
	commands[20].atts[0].name = "TxPower";
	commands[20].atts[0].size = 1;
	commands[20].atts[0].isList = 0;
	commands[21].cmdName = "AF_REGISTER";
	commands[21].cmdDesc =
	        " This command enables the tester to register an application's endpoint description.\n";
	commands[21].attNum = 9;
	commands[21].atts[0].name = "EndPoint";
	commands[21].atts[0].size = 1;
	commands[21].atts[0].isList = 0;
	commands[21].atts[1].name = "AppProfId";
	commands[21].atts[1].size = 2;
	commands[21].atts[1].isList = 0;
	commands[21].atts[2].name = "AppDeviceId";
	commands[21].atts[2].size = 2;
	commands[21].atts[2].isList = 0;
	commands[21].atts[3].name = "AppDevVer";
	commands[21].atts[3].size = 1;
	commands[21].atts[3].isList = 0;
	commands[21].atts[4].name = "LatencyReq";
	commands[21].atts[4].size = 1;
	commands[21].atts[4].isList = 0;
	commands[21].atts[5].name = "AppNumInClusters";
	commands[21].atts[5].size = 1;
	commands[21].atts[5].isList = 0;
	commands[21].atts[6].name = "AppInClusterList";
	commands[21].atts[6].size = 2;
	commands[21].atts[6].isList = 16;
	commands[21].atts[7].name = "AppNumOutClusters";
	commands[21].atts[7].size = 1;
	commands[21].atts[7].isList = 0;
	commands[21].atts[8].name = "AppOutClusterList";
	commands[21].atts[8].size = 2;
	commands[21].atts[8].isList = 16;
	commands[22].cmdName = "AF_DATA_REQUEST";
	commands[22].cmdDesc =
	        " This command is used by the tester to build and send a message through AF layer.\n";
	commands[22].attNum = 9;
	commands[22].atts[0].name = "DstAddr";
	commands[22].atts[0].size = 2;
	commands[22].atts[0].isList = 0;
	commands[22].atts[1].name = "DstEndpoint";
	commands[22].atts[1].size = 1;
	commands[22].atts[1].isList = 0;
	commands[22].atts[2].name = "SrcEndpoint";
	commands[22].atts[2].size = 1;
	commands[22].atts[2].isList = 0;
	commands[22].atts[3].name = "ClusterID";
	commands[22].atts[3].size = 2;
	commands[22].atts[3].isList = 0;
	commands[22].atts[4].name = "TransID";
	commands[22].atts[4].size = 1;
	commands[22].atts[4].isList = 0;
	commands[22].atts[5].name = "Options";
	commands[22].atts[5].size = 1;
	commands[22].atts[5].isList = 0;
	commands[22].atts[6].name = "Radius";
	commands[22].atts[6].size = 1;
	commands[22].atts[6].isList = 0;
	commands[22].atts[7].name = "Len";
	commands[22].atts[7].size = 1;
	commands[22].atts[7].isList = 0;
	commands[22].atts[8].name = "Data";
	commands[22].atts[8].size = 1;
	commands[22].atts[8].isList = 128;
	commands[23].cmdName = "AF_DATA_REQUEST_EXT";
	commands[23].cmdDesc =
	        " This extended form of the AF_DATA_REQUEST must be used to send an inter-pan message\n"
			        " (note that the target code must be compiled with the INTER_PAN flag defined.) This\n"
			        " extended data request must also be used when making a request with a huge data byte\n"
			        " count which is defined to be a size that would cause the RPC request to exceed the\n"
			        " maximum allowed size:\n \tMT_RPC_DATA_MAX â€“ sizeof(AF_DATA_REQUEST_EXT)\n Where"
			        " sizeof(AF_DATA_REQUEST_EXT) counts everything but the data bytes and now stands\n"
			        " at 20. When making an AF_DATA_REQUEST_EXT with a huge data byte count, the request\n"
			        " shall not contain any data bytes. The huge data buffer is sent over separately as\n"
			        " a sequence of one or more AF_DATA_STORE requests. Note that the outgoing huge message\n"
			        " is timed-out in 15 seconds; thus all AF_DATA_STORE requests must be completed within\n"
			        " 15 seconds of an AF_DATA_REQUEST_EXT with a huge data byte count. And any AF_DATA_REQUEST_EXT\n"
			        " with a huge data byte count must be completed (or timed-out) before another will\n"
			        " be started. The default timeout can be changed by defining the following to other\n"
			        " values:\n \t#if !defined MT_AF_EXEC_CNT\n \t#define MT_AF_EXEC_CNT 15\n \t#endif\n"
			        " \t#if !defined MT_AF_EXEC_DLY\n \t#define MT_AF_EXEC_DLY 1000\n \t#endif\n";
	commands[23].attNum = 11;
	commands[23].atts[0].name = "DstAddrMode";
	commands[23].atts[0].size = 1;
	commands[23].atts[0].isList = 0;
	commands[23].atts[1].name = "DstAddr";
	commands[23].atts[1].size = 8;
	commands[23].atts[1].isList = 0;
	commands[23].atts[2].name = "DstEndpoint";
	commands[23].atts[2].size = 1;
	commands[23].atts[2].isList = 0;
	commands[23].atts[3].name = "DstPanID";
	commands[23].atts[3].size = 2;
	commands[23].atts[3].isList = 0;
	commands[23].atts[4].name = "SrcEndpoint";
	commands[23].atts[4].size = 1;
	commands[23].atts[4].isList = 0;
	commands[23].atts[5].name = "ClusterId";
	commands[23].atts[5].size = 2;
	commands[23].atts[5].isList = 0;
	commands[23].atts[6].name = "TransId";
	commands[23].atts[6].size = 1;
	commands[23].atts[6].isList = 0;
	commands[23].atts[7].name = "Options";
	commands[23].atts[7].size = 1;
	commands[23].atts[7].isList = 0;
	commands[23].atts[8].name = "Radius";
	commands[23].atts[8].size = 1;
	commands[23].atts[8].isList = 0;
	commands[23].atts[9].name = "Len";
	commands[23].atts[9].size = 2;
	commands[23].atts[9].isList = 0;
	commands[23].atts[10].name = "Data";
	commands[23].atts[10].size = 1;
	commands[23].atts[10].isList = 230;
	commands[24].cmdName = "AF_DATA_REQUEST_SRC_RTG";
	commands[24].cmdDesc =
	        " This command is used by the tester to build and send a message through AF layer\n"
			        " using source routing.\n";
	commands[24].attNum = 11;
	commands[24].atts[0].name = "DstAddr";
	commands[24].atts[0].size = 2;
	commands[24].atts[0].isList = 0;
	commands[24].atts[1].name = "DstEndpoint";
	commands[24].atts[1].size = 1;
	commands[24].atts[1].isList = 0;
	commands[24].atts[2].name = "SrcEndpoint";
	commands[24].atts[2].size = 1;
	commands[24].atts[2].isList = 0;
	commands[24].atts[3].name = "ClusterID";
	commands[24].atts[3].size = 2;
	commands[24].atts[3].isList = 0;
	commands[24].atts[4].name = "TransID";
	commands[24].atts[4].size = 1;
	commands[24].atts[4].isList = 0;
	commands[24].atts[5].name = "Options";
	commands[24].atts[5].size = 1;
	commands[24].atts[5].isList = 0;
	commands[24].atts[6].name = "Radius";
	commands[24].atts[6].size = 1;
	commands[24].atts[6].isList = 0;
	commands[24].atts[7].name = "RelayCount";
	commands[24].atts[7].size = 1;
	commands[24].atts[7].isList = 0;
	commands[24].atts[8].name = "RelayList";
	commands[24].atts[8].size = 2;
	commands[24].atts[8].isList = 255;
	commands[24].atts[9].name = "Len";
	commands[24].atts[9].size = 1;
	commands[24].atts[9].isList = 0;
	commands[24].atts[10].name = "Data";
	commands[24].atts[10].size = 1;
	commands[24].atts[10].isList = 128;
	commands[25].cmdName = "AF_INTER_PAN_CTL";
	commands[25].cmdDesc =
	        " Inter-Pan control command and data. The data content depends upon the command and\n"
			        " the available commands are enumerated as InterPanCtl_t.\n";
	commands[25].attNum = 2;
	commands[25].atts[0].name = "Command";
	commands[25].atts[0].size = 1;
	commands[25].atts[0].isList = 0;
	commands[25].atts[1].name = "Data";
	commands[25].atts[1].size = 1;
	commands[25].atts[1].isList = 3;
	commands[26].cmdName = "AF_DATA_STORE";
	commands[26].cmdDesc =
	        " Huge AF data request data buffer store command and data.\n";
	commands[26].attNum = 3;
	commands[26].atts[0].name = "Index";
	commands[26].atts[0].size = 2;
	commands[26].atts[0].isList = 0;
	commands[26].atts[1].name = "Length";
	commands[26].atts[1].size = 1;
	commands[26].atts[1].isList = 0;
	commands[26].atts[2].name = "Data";
	commands[26].atts[2].size = 1;
	commands[26].atts[2].isList = 247;
	commands[27].cmdName = "AF_DATA_RETRIEVE";
	commands[27].cmdDesc =
	        " Huge AF incoming message data buffer retrieve command.\n";
	commands[27].attNum = 3;
	commands[27].atts[0].name = "TimeStamp";
	commands[27].atts[0].size = 4;
	commands[27].atts[0].isList = 0;
	commands[27].atts[1].name = "Index";
	commands[27].atts[1].size = 2;
	commands[27].atts[1].isList = 0;
	commands[27].atts[2].name = "Length";
	commands[27].atts[2].size = 1;
	commands[27].atts[2].isList = 0;
	commands[28].cmdName = "AF_APSF_CONFIG_SET";
	commands[28].cmdDesc = " MT proxy for afAPSF_ConfigSet().\n";
	commands[28].attNum = 3;
	commands[28].atts[0].name = "Endpoint";
	commands[28].atts[0].size = 1;
	commands[28].atts[0].isList = 0;
	commands[28].atts[1].name = "FrameDelay";
	commands[28].atts[1].size = 1;
	commands[28].atts[1].isList = 0;
	commands[28].atts[2].name = "WindowSize";
	commands[28].atts[2].size = 1;
	commands[28].atts[2].isList = 0;
	commands[29].cmdName = "ZDO_NWK_ADDR_REQ";
	commands[29].cmdDesc =
	        " This message will request the device to send a Network Address Request.\n"
			        " This message sends a broadcast message looking for a 16 bit address with a known\n"
			        " 64 bit IEEE address.\n";
	commands[29].attNum = 3;
	commands[29].atts[0].name = "IEEEAddress";
	commands[29].atts[0].size = 8;
	commands[29].atts[0].isList = 0;
	commands[29].atts[1].name = "ReqType";
	commands[29].atts[1].size = 1;
	commands[29].atts[1].isList = 0;
	commands[29].atts[2].name = "StartIndex";
	commands[29].atts[2].size = 1;
	commands[29].atts[2].isList = 0;
	commands[30].cmdName = "ZDO_IEEE_ADDR_REQ";
	commands[30].cmdDesc =
	        " This command will request a device's IEEE 64-bit address.\n";
	commands[30].attNum = 3;
	commands[30].atts[0].name = "ShortAddr";
	commands[30].atts[0].size = 2;
	commands[30].atts[0].isList = 0;
	commands[30].atts[1].name = "ReqType";
	commands[30].atts[1].size = 1;
	commands[30].atts[1].isList = 0;
	commands[30].atts[2].name = "StartIndex";
	commands[30].atts[2].size = 1;
	commands[30].atts[2].isList = 0;
	commands[31].cmdName = "ZDO_NODE_DESC_REQ";
	commands[31].cmdDesc =
	        " This command is generated to inquire about the Node Descriptor information of the\n"
			        " destination device\n";
	commands[31].attNum = 2;
	commands[31].atts[0].name = "DstAddr";
	commands[31].atts[0].size = 2;
	commands[31].atts[0].isList = 0;
	commands[31].atts[1].name = "NwkAddrOfInterest";
	commands[31].atts[1].size = 2;
	commands[31].atts[1].isList = 0;
	commands[32].cmdName = "ZDO_POWER_DESC_REQ";
	commands[32].cmdDesc =
	        " This command is generated to inquire about the Power Descriptor information of the\n"
			        " destination device.\n";
	commands[32].attNum = 2;
	commands[32].atts[0].name = "DstAddr";
	commands[32].atts[0].size = 2;
	commands[32].atts[0].isList = 0;
	commands[32].atts[1].name = "NwkAddrOfInterest";
	commands[32].atts[1].size = 2;
	commands[32].atts[1].isList = 0;
	commands[33].cmdName = "ZDO_SIMPLE_DESC_REQ";
	commands[33].cmdDesc =
	        " This command is generated to inquire as to the Simple Descriptor of the destination\n"
			        " devices Endpoint.\n";
	commands[33].attNum = 3;
	commands[33].atts[0].name = "DstAddr";
	commands[33].atts[0].size = 2;
	commands[33].atts[0].isList = 0;
	commands[33].atts[1].name = "NwkAddrOfInterest";
	commands[33].atts[1].size = 2;
	commands[33].atts[1].isList = 0;
	commands[33].atts[2].name = "Endpoint";
	commands[33].atts[2].size = 1;
	commands[33].atts[2].isList = 0;
	commands[34].cmdName = "ZDO_ACTIVE_EP_REQ";
	commands[34].cmdDesc =
	        " This command is generated to request a list of active endpoint from the destination\n"
			        " device.\n";
	commands[34].attNum = 2;
	commands[34].atts[0].name = "DstAddr";
	commands[34].atts[0].size = 2;
	commands[34].atts[0].isList = 0;
	commands[34].atts[1].name = "NwkAddrOfInterest";
	commands[34].atts[1].size = 2;
	commands[34].atts[1].isList = 0;
	commands[35].cmdName = "ZDO_MATCH_DESC_REQ";
	commands[35].cmdDesc =
	        " This command is generated to request the device match descriptor.\n";
	commands[35].attNum = 7;
	commands[35].atts[0].name = "DstAddr";
	commands[35].atts[0].size = 2;
	commands[35].atts[0].isList = 0;
	commands[35].atts[1].name = "NwkAddrOfInterest";
	commands[35].atts[1].size = 2;
	commands[35].atts[1].isList = 0;
	commands[35].atts[2].name = "ProfileID";
	commands[35].atts[2].size = 2;
	commands[35].atts[2].isList = 0;
	commands[35].atts[3].name = "NumInClusters";
	commands[35].atts[3].size = 1;
	commands[35].atts[3].isList = 0;
	commands[35].atts[4].name = "InClusterList";
	commands[35].atts[4].size = 2;
	commands[35].atts[4].isList = 16;
	commands[35].atts[5].name = "NumOutClusters";
	commands[35].atts[5].size = 1;
	commands[35].atts[5].isList = 0;
	commands[35].atts[6].name = "OutClusterList";
	commands[35].atts[6].size = 2;
	commands[35].atts[6].isList = 16;
	commands[36].cmdName = "ZDO_COMPLEX_DESC_REQ";
	commands[36].cmdDesc =
	        " This command is generated to request for the destination deviceâ€™s complex descriptor.\n";
	commands[36].attNum = 2;
	commands[36].atts[0].name = "DstAddr";
	commands[36].atts[0].size = 2;
	commands[36].atts[0].isList = 0;
	commands[36].atts[1].name = "NwkAddrOfInterest";
	commands[36].atts[1].size = 2;
	commands[36].atts[1].isList = 0;
	commands[37].cmdName = "ZDO_USER_DESC_REQ";
	commands[37].cmdDesc =
	        " This command is generated to request for the destination deviceâ€™s user descriptor.\n";
	commands[37].attNum = 2;
	commands[37].atts[0].name = "DstAddr";
	commands[37].atts[0].size = 2;
	commands[37].atts[0].isList = 0;
	commands[37].atts[1].name = "NwkAddrOfInterest";
	commands[37].atts[1].size = 2;
	commands[37].atts[1].isList = 0;
	commands[38].cmdName = "ZDO_DEVICE_ANNCE";
	commands[38].cmdDesc =
	        " This command will cause the CC2480 device to issue an â€œEnd device announceâ€�\n"
			        " broadcast packet to the network. This is typically used by an end-device to announce\n"
			        " itself to the network.\n";
	commands[38].attNum = 3;
	commands[38].atts[0].name = "NWKAddr";
	commands[38].atts[0].size = 2;
	commands[38].atts[0].isList = 0;
	commands[38].atts[1].name = "IEEEAddr";
	commands[38].atts[1].size = 8;
	commands[38].atts[1].isList = 0;
	commands[38].atts[2].name = "Capabilities";
	commands[38].atts[2].size = 1;
	commands[38].atts[2].isList = 0;
	commands[39].cmdName = "ZDO_USER_DESC_SET";
	commands[39].cmdDesc =
	        " This command is generated to write a User Descriptor value to the targeted device\n";
	commands[39].attNum = 4;
	commands[39].atts[0].name = "DstAddr";
	commands[39].atts[0].size = 2;
	commands[39].atts[0].isList = 0;
	commands[39].atts[1].name = "NwkAddrOfInterest";
	commands[39].atts[1].size = 2;
	commands[39].atts[1].isList = 0;
	commands[39].atts[2].name = "Len";
	commands[39].atts[2].size = 1;
	commands[39].atts[2].isList = 0;
	commands[39].atts[3].name = "UserDescriptor";
	commands[39].atts[3].size = 1;
	commands[39].atts[3].isList = 16;
	commands[40].cmdName = "ZDO_SERVER_DISC_REQ";
	commands[40].cmdDesc =
	        " The command is used for local device to discover the location of a particular system\n"
			        " server or servers as indicated by the ServerMask parameter. The destination addressing\n"
			        " on this request is broadcast to all RxOnWhenIdle devices.\n";
	commands[40].attNum = 1;
	commands[40].atts[0].name = "ServerMask";
	commands[40].atts[0].size = 2;
	commands[40].atts[0].isList = 0;
	commands[41].cmdName = "ZDO_END_DEVICE_BIND_REQ";
	commands[41].cmdDesc =
	        " This command is generated to request an End Device Bind with the destination device.\n";
	commands[41].attNum = 9;
	commands[41].atts[0].name = "DstAddr";
	commands[41].atts[0].size = 2;
	commands[41].atts[0].isList = 0;
	commands[41].atts[1].name = "LocalCoordinator";
	commands[41].atts[1].size = 2;
	commands[41].atts[1].isList = 0;
	commands[41].atts[2].name = "CoordinatorIEEE";
	commands[41].atts[2].size = 8;
	commands[41].atts[2].isList = 0;
	commands[41].atts[3].name = "EndPoint";
	commands[41].atts[3].size = 1;
	commands[41].atts[3].isList = 0;
	commands[41].atts[4].name = "ProfileID";
	commands[41].atts[4].size = 2;
	commands[41].atts[4].isList = 0;
	commands[41].atts[5].name = "NumInClusters";
	commands[41].atts[5].size = 1;
	commands[41].atts[5].isList = 0;
	commands[41].atts[6].name = "InClusterList";
	commands[41].atts[6].size = 2;
	commands[41].atts[6].isList = 16;
	commands[41].atts[7].name = "NumOutClusters";
	commands[41].atts[7].size = 1;
	commands[41].atts[7].isList = 0;
	commands[41].atts[8].name = "OutClusterList";
	commands[41].atts[8].size = 2;
	commands[41].atts[8].isList = 16;
	commands[42].cmdName = "ZDO_BIND_REQ";
	commands[42].cmdDesc = " This command is generated to request a Bind.\n";
	commands[42].attNum = 7;
	commands[42].atts[0].name = "DstAddr";
	commands[42].atts[0].size = 2;
	commands[42].atts[0].isList = 0;
	commands[42].atts[1].name = "SrcAddress";
	commands[42].atts[1].size = 8;
	commands[42].atts[1].isList = 0;
	commands[42].atts[2].name = "SrcEndpoint";
	commands[42].atts[2].size = 1;
	commands[42].atts[2].isList = 0;
	commands[42].atts[3].name = "ClusterID";
	commands[42].atts[3].size = 2;
	commands[42].atts[3].isList = 0;
	commands[42].atts[4].name = "DstAddrMode";
	commands[42].atts[4].size = 1;
	commands[42].atts[4].isList = 0;
	commands[42].atts[5].name = "DstAddress";
	commands[42].atts[5].size = 8;
	commands[42].atts[5].isList = 0;
	commands[42].atts[6].name = "DstEndpoint";
	commands[42].atts[6].size = 1;
	commands[42].atts[6].isList = 0;
	commands[43].cmdName = "ZDO_UNBIND_REQ";
	commands[43].cmdDesc =
	        " This command is generated to request an un-bind.\n";
	commands[43].attNum = 7;
	commands[43].atts[0].name = "DstAddr";
	commands[43].atts[0].size = 2;
	commands[43].atts[0].isList = 0;
	commands[43].atts[1].name = "SrcAddress";
	commands[43].atts[1].size = 8;
	commands[43].atts[1].isList = 0;
	commands[43].atts[2].name = "SrcEndpoint";
	commands[43].atts[2].size = 1;
	commands[43].atts[2].isList = 0;
	commands[43].atts[3].name = "ClusterID";
	commands[43].atts[3].size = 2;
	commands[43].atts[3].isList = 0;
	commands[43].atts[4].name = "DstAddrMode";
	commands[43].atts[4].size = 1;
	commands[43].atts[4].isList = 0;
	commands[43].atts[5].name = "DstAddress";
	commands[43].atts[5].size = 8;
	commands[43].atts[5].isList = 0;
	commands[43].atts[6].name = "DstEndpoint";
	commands[43].atts[6].size = 1;
	commands[43].atts[6].isList = 0;
	commands[44].cmdName = "ZDO_MGMT_NWK_DISC_REQ";
	commands[44].cmdDesc =
	        " This command is generated to request the destination device to perform a network\n"
			        " discovery.\n";
	commands[44].attNum = 4;
	commands[44].atts[0].name = "DstAddr";
	commands[44].atts[0].size = 2;
	commands[44].atts[0].isList = 0;
	commands[44].atts[1].name = "ScanChannels";
	commands[44].atts[1].size = 4;
	commands[44].atts[1].isList = 0;
	commands[44].atts[2].name = "ScanDuration";
	commands[44].atts[2].size = 1;
	commands[44].atts[2].isList = 0;
	commands[44].atts[3].name = "StartIndex";
	commands[44].atts[3].size = 1;
	commands[44].atts[3].isList = 0;
	commands[45].cmdName = "ZDO_MGMT_LQI_REQ";
	commands[45].cmdDesc =
	        " This command is generated to request the destination device to perform a LQI query\n"
			        " of other devices in the network.\n";
	commands[45].attNum = 2;
	commands[45].atts[0].name = "DstAddr";
	commands[45].atts[0].size = 2;
	commands[45].atts[0].isList = 0;
	commands[45].atts[1].name = "StartIndex";
	commands[45].atts[1].size = 1;
	commands[45].atts[1].isList = 0;
	commands[46].cmdName = "ZDO_MGMT_RTG_REQ";
	commands[46].cmdDesc =
	        " This command is generated to request the Routing Table of the destination device\n";
	commands[46].attNum = 2;
	commands[46].atts[0].name = "DstAddr";
	commands[46].atts[0].size = 2;
	commands[46].atts[0].isList = 0;
	commands[46].atts[1].name = "StartIndex";
	commands[46].atts[1].size = 1;
	commands[46].atts[1].isList = 0;
	commands[47].cmdName = "ZDO_MGMT_BIND_REQ";
	commands[47].cmdDesc =
	        " This command is generated to request the Binding Table of the destination device.\n";
	commands[47].attNum = 2;
	commands[47].atts[0].name = "DstAddr";
	commands[47].atts[0].size = 2;
	commands[47].atts[0].isList = 0;
	commands[47].atts[1].name = "StartIndex";
	commands[47].atts[1].size = 1;
	commands[47].atts[1].isList = 0;
	commands[48].cmdName = "ZDO_MGMT_LEAVE_REQ";
	commands[48].cmdDesc =
	        " This command is generated to request a Management Leave Request for the target device\n";
	commands[48].attNum = 3;
	commands[48].atts[0].name = "DstAddr";
	commands[48].atts[0].size = 2;
	commands[48].atts[0].isList = 0;
	commands[48].atts[1].name = "DeviceAddr";
	commands[48].atts[1].size = 8;
	commands[48].atts[1].isList = 0;
	commands[48].atts[2].name = "RemoveChildre_Rejoin";
	commands[48].atts[2].size = 1;
	commands[48].atts[2].isList = 0;
	commands[49].cmdName = "ZDO_MGMT_DIRECT_JOIN_REQ";
	commands[49].cmdDesc =
	        " This command is generated to request the Management Direct Join Request of a designated\n"
			        " device.\n";
	commands[49].attNum = 3;
	commands[49].atts[0].name = "DstAddr";
	commands[49].atts[0].size = 2;
	commands[49].atts[0].isList = 0;
	commands[49].atts[1].name = "DeviceAddr";
	commands[49].atts[1].size = 8;
	commands[49].atts[1].isList = 0;
	commands[49].atts[2].name = "CapInfo";
	commands[49].atts[2].size = 1;
	commands[49].atts[2].isList = 0;
	commands[50].cmdName = "ZDO_MGMT_PERMIT_JOIN_REQ";
	commands[50].cmdDesc =
	        " This command is generated to set the Permit Join for the destination device\n";
	commands[50].attNum = 4;
	commands[50].atts[0].name = "AddrMode";
	commands[50].atts[0].size = 1;
	commands[50].atts[0].isList = 0;
	commands[50].atts[1].name = "DstAddr";
	commands[50].atts[1].size = 2;
	commands[50].atts[1].isList = 0;
	commands[50].atts[2].name = "Duration";
	commands[50].atts[2].size = 1;
	commands[50].atts[2].isList = 0;
	commands[50].atts[3].name = "TCSignificance";
	commands[50].atts[3].size = 1;
	commands[50].atts[3].isList = 0;
	commands[51].cmdName = "ZDO_MGMT_NWK_UPDATE_REQ";
	commands[51].cmdDesc =
	        " This command is provided to allow updating of network configuration parameters or\n"
			        " to request information from devices on network conditions in the local operating\n"
			        " environment.\n";
	commands[51].attNum = 6;
	commands[51].atts[0].name = "DstAddr";
	commands[51].atts[0].size = 2;
	commands[51].atts[0].isList = 0;
	commands[51].atts[1].name = "DstAddrMode";
	commands[51].atts[1].size = 1;
	commands[51].atts[1].isList = 0;
	commands[51].atts[2].name = "ChannelMask";
	commands[51].atts[2].size = 4;
	commands[51].atts[2].isList = 0;
	commands[51].atts[3].name = "ScanDuration";
	commands[51].atts[3].size = 1;
	commands[51].atts[3].isList = 0;
	commands[51].atts[4].name = "ScanCount";
	commands[51].atts[4].size = 1;
	commands[51].atts[4].isList = 0;
	commands[51].atts[5].name = "NwkManagerAddr";
	commands[51].atts[5].size = 2;
	commands[51].atts[5].isList = 0;
	commands[52].cmdName = "ZDO_STARTUP_FROM_APP";
	commands[52].cmdDesc = " This command starts the device in the network.\n";
	commands[52].attNum = 1;
	commands[52].atts[0].name = "StartDelay";
	commands[52].atts[0].size = 2;
	commands[52].atts[0].isList = 0;
	commands[53].cmdName = "ZDO_AUTO_FIND_DESTINATION";
	commands[53].cmdDesc =
	        " This function will issue a Match Description Request for the requested endpoint\n"
			        " outputs. This message will generate a broadcast message.\n";
	commands[53].attNum = 1;
	commands[53].atts[0].name = "Endpoint";
	commands[53].atts[0].size = 1;
	commands[53].atts[0].isList = 0;
	commands[54].cmdName = "ZDO_SET_LINK_KEY";
	commands[54].cmdDesc =
	        " This command sets the application link key for a given device.\n";
	commands[54].attNum = 3;
	commands[54].atts[0].name = "ShortAddr";
	commands[54].atts[0].size = 2;
	commands[54].atts[0].isList = 0;
	commands[54].atts[1].name = "IEEEaddr";
	commands[54].atts[1].size = 8;
	commands[54].atts[1].isList = 0;
	commands[54].atts[2].name = "LinkKeyData";
	commands[54].atts[2].size = 16;
	commands[54].atts[2].isList = 0;
	commands[55].cmdName = "ZDO_REMOVE_LINK_KEY";
	commands[55].cmdDesc =
	        " This command removes the application link key of a given device.\n";
	commands[55].attNum = 1;
	commands[55].atts[0].name = "IEEEaddr";
	commands[55].atts[0].size = 8;
	commands[55].atts[0].isList = 0;
	commands[56].cmdName = "ZDO_GET_LINK_KEY";
	commands[56].cmdDesc =
	        " This command retrieves the application link key of a given device.\n";
	commands[56].attNum = 1;
	commands[56].atts[0].name = "IEEEaddr";
	commands[56].atts[0].size = 8;
	commands[56].atts[0].isList = 0;
	commands[57].cmdName = "ZDO_NWK_DISCOVERY_REQ";
	commands[57].cmdDesc =
	        " This command is used to initiate a network discovery (active scan).\n";
	commands[57].attNum = 2;
	commands[57].atts[0].name = "ScanChannels";
	commands[57].atts[0].size = 4;
	commands[57].atts[0].isList = 0;
	commands[57].atts[1].name = "ScanDuration";
	commands[57].atts[1].size = 1;
	commands[57].atts[1].isList = 0;
	commands[58].cmdName = "ZDO_JOIN_REQ";
	commands[58].cmdDesc =
	        " This command is used to request the device to join itself to a parent device on\n"
			        " a network.\n";
	commands[58].attNum = 6;
	commands[58].atts[0].name = "LogicalChannel";
	commands[58].atts[0].size = 1;
	commands[58].atts[0].isList = 0;
	commands[58].atts[1].name = "PanID";
	commands[58].atts[1].size = 2;
	commands[58].atts[1].isList = 0;
	commands[58].atts[2].name = "ExtendedPanID";
	commands[58].atts[2].size = 8;
	commands[58].atts[2].isList = 0;
	commands[58].atts[3].name = "ChosenParent";
	commands[58].atts[3].size = 2;
	commands[58].atts[3].isList = 0;
	commands[58].atts[4].name = "ParentDepth";
	commands[58].atts[4].size = 1;
	commands[58].atts[4].isList = 0;
	commands[58].atts[5].name = "StackProfile";
	commands[58].atts[5].size = 1;
	commands[58].atts[5].isList = 0;
	commands[59].cmdName = "ZDO_MSG_CB_REGISTER";
	commands[59].cmdDesc = " This command registers for a ZDO callback.\n";
	commands[59].attNum = 1;
	commands[59].atts[0].name = "ClusterID";
	commands[59].atts[0].size = 2;
	commands[59].atts[0].isList = 0;
	commands[60].cmdName = "ZDO_MSG_CB_REMOVE";
	commands[60].cmdDesc =
	        " This command removes a registration for a ZDO callback\n";
	commands[60].attNum = 1;
	commands[60].atts[0].name = "ClusterID";
	commands[60].atts[0].size = 2;
	commands[60].atts[0].isList = 0;
	commands[61].cmdName = "ZB_SYSTEM_RESET";
	commands[61].cmdDesc =
	        " This command will reset the device by using a soft reset (i.e. a jump to the reset\n"
			        " vector) vice a hardware reset (i.e. watchdog reset.) This is especially useful in\n"
			        " the CC2531, for instance, so that the USB host does not have to contend with the\n"
			        " USB H/W resetting (and thus causing the USB host to re-enumerate the device which\n"
			        " can cause an open virtual serial port to hang.)\n";
	commands[61].attNum = 0;
	commands[62].cmdName = "ZB_APP_REGISTER_REQ";
	commands[62].cmdDesc =
	        " This command enables the application processor to register its application with\n"
			        " a ZNP device.\n";
	commands[62].attNum = 9;
	commands[62].atts[0].name = "AppEndpoint";
	commands[62].atts[0].size = 1;
	commands[62].atts[0].isList = 0;
	commands[62].atts[1].name = "AppProfileId";
	commands[62].atts[1].size = 2;
	commands[62].atts[1].isList = 0;
	commands[62].atts[2].name = "DeviceId";
	commands[62].atts[2].size = 2;
	commands[62].atts[2].isList = 0;
	commands[62].atts[3].name = "DeviceVersion";
	commands[62].atts[3].size = 1;
	commands[62].atts[3].isList = 0;
	commands[62].atts[4].name = "Unused";
	commands[62].atts[4].size = 1;
	commands[62].atts[4].isList = 0;
	commands[62].atts[5].name = "InputCommandsNum";
	commands[62].atts[5].size = 1;
	commands[62].atts[5].isList = 0;
	commands[62].atts[6].name = "InputCommandsList";
	commands[62].atts[6].size = 2;
	commands[62].atts[6].isList = 255;
	commands[62].atts[7].name = "OutputCommandsNum";
	commands[62].atts[7].size = 1;
	commands[62].atts[7].isList = 0;
	commands[62].atts[8].name = "OutputCommandsList";
	commands[62].atts[8].size = 2;
	commands[62].atts[8].isList = 255;
	commands[63].cmdName = "ZB_START_REQ";
	commands[63].cmdDesc =
	        " This command starts the ZigBee stack. When the ZigBee stack starts, the device reads\n"
			        " configuration parameters from nonvolatile memory and the device joins its network.\n"
			        " The ZigBee stack calls the zb_StartConfirm callback function when the startup process\n"
			        " completes. After the start request process completes, the device is ready to send,\n"
			        " receive, and route network traffic.\n";
	commands[63].attNum = 0;
	commands[64].cmdName = "ZB_PERMIT_JOINING_REQ";
	commands[64].cmdDesc =
	        " This command is used to control the joining permissions and thus allows or disallows\n"
			        " new devices from joining the network.\n";
	commands[64].attNum = 2;
	commands[64].atts[0].name = "Destination";
	commands[64].atts[0].size = 2;
	commands[64].atts[0].isList = 0;
	commands[64].atts[1].name = "Timeout";
	commands[64].atts[1].size = 1;
	commands[64].atts[1].isList = 0;
	commands[65].cmdName = "ZB_BIND_DEVICE";
	commands[65].cmdDesc =
	        " This command establishes or removes a â€˜bindingâ€™ between two devices. Once bound,\n"
			        " an application can send messages to a device by referencing the commandId for the\n"
			        " binding.\n";
	commands[65].attNum = 3;
	commands[65].atts[0].name = "Create";
	commands[65].atts[0].size = 1;
	commands[65].atts[0].isList = 0;
	commands[65].atts[1].name = "CommandId";
	commands[65].atts[1].size = 2;
	commands[65].atts[1].isList = 0;
	commands[65].atts[2].name = "DstIeee";
	commands[65].atts[2].size = 8;
	commands[65].atts[2].isList = 0;
	commands[66].cmdName = "ZB_ALLOW_BIND";
	commands[66].cmdDesc =
	        " This command puts the device into the Allow Binding Mode for a given period of time.\n"
			        " A peer device can establish a binding to a device in the Allow Binding Mode by calling\n"
			        " zb_BindDevice with a destination address of NULL.\n";
	commands[66].attNum = 1;
	commands[66].atts[0].name = "Timeout";
	commands[66].atts[0].size = 1;
	commands[66].atts[0].isList = 0;
	commands[67].cmdName = "ZB_SEND_DATA_REQ";
	commands[67].cmdDesc =
	        " This command initiates transmission of data to a peer device.\n";
	commands[67].attNum = 7;
	commands[67].atts[0].name = "Destination";
	commands[67].atts[0].size = 2;
	commands[67].atts[0].isList = 0;
	commands[67].atts[1].name = "CommandId";
	commands[67].atts[1].size = 2;
	commands[67].atts[1].isList = 0;
	commands[67].atts[2].name = "Handle";
	commands[67].atts[2].size = 1;
	commands[67].atts[2].isList = 0;
	commands[67].atts[3].name = "Ack";
	commands[67].atts[3].size = 1;
	commands[67].atts[3].isList = 0;
	commands[67].atts[4].name = "Radius";
	commands[67].atts[4].size = 1;
	commands[67].atts[4].isList = 0;
	commands[67].atts[5].name = "Len";
	commands[67].atts[5].size = 1;
	commands[67].atts[5].isList = 0;
	commands[67].atts[6].name = "Data";
	commands[67].atts[6].size = 1;
	commands[67].atts[6].isList = 99;
	commands[68].cmdName = "ZB_FIND_DEVICE_REQ";
	commands[68].cmdDesc =
	        " This command is used to determine the short address for a device in the network.\n"
			        " The device initiating a call to zbFindDeviceRequest and the device being discovered\n"
			        " must both be a member of the same network. When the search is complete, the zbFindDeviceConfirm\n"
			        " callback function is called.\n";
	commands[68].attNum = 1;
	commands[68].atts[0].name = "SearchKey";
	commands[68].atts[0].size = 8;
	commands[68].atts[0].isList = 0;
	commands[69].cmdName = "ZB_WRITE_CONFIGURATION";
	commands[69].cmdDesc =
	        " This command is used to write a Configuration Property to nonvolatile memory.\n";
	commands[69].attNum = 3;
	commands[69].atts[0].name = "ConfigId";
	commands[69].atts[0].size = 1;
	commands[69].atts[0].isList = 0;
	commands[69].atts[1].name = "Len";
	commands[69].atts[1].size = 1;
	commands[69].atts[1].isList = 0;
	commands[69].atts[2].name = "Value";
	commands[69].atts[2].size = 1;
	commands[69].atts[2].isList = 128;
	commands[70].cmdName = "ZB_GET_DEVICE_INFO";
	commands[70].cmdDesc =
	        " This command retrieves a Device Information Property.\n";
	commands[70].attNum = 1;
	commands[70].atts[0].name = "Param";
	commands[70].atts[0].size = 1;
	commands[70].atts[0].isList = 0;
	commands[71].cmdName = "ZB_READ_CONFIGURATION";
	commands[71].cmdDesc =
	        " This command is used to get a configuration property from nonvolatile memory.\n";
	commands[71].attNum = 1;
	commands[71].atts[0].name = "ConfigId";
	commands[71].atts[0].size = 1;
	commands[71].atts[0].isList = 0;
}

/********************************************************************
 * START OF SYS CALL BACK FUNCTIONS
 */

static uint8_t mtSysPingSrspCb(PingSrspFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtSysPingSrspCb\n");
	consolePrint("Capabilities: 0x%04X\n", msg->Capabilities);
	SET_NRM_COLOR();
	return 0;
}
static uint8_t mtSysGetExtAddrSrspCb(GetExtAddrSrspFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtSysGetExtAddrSrspCb\n");
	consolePrint("ExtAddr: 0x%016llX\n", (long long unsigned int) msg->ExtAddr);
	SET_NRM_COLOR();
	return 0;
}
static uint8_t mtSysRamReadSrspCb(RamReadSrspFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtSysRamReadSrspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("Len: 0x%02X\n", msg->Len);
		uint32_t i;
		for (i = 0; i < msg->Len; i++)
		{
			consolePrint("Value[%d]: 0x%02X\n", i, msg->Value[i]);
		}
	}
	else
	{
		consolePrint("RamReadSrsp Status: FAIL 0x%02X\n", msg->Status);
	}
	SET_NRM_COLOR();
	return msg->Status;
}
static uint8_t mtSysResetIndCb(ResetIndFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("ZNP Version: %d.%d.%d\n", msg->MajorRel, msg->MinorRel,
	        msg->HwRev);
	SET_NRM_COLOR();
	return 0;
}
static uint8_t mtSysVersionSrspCb(VersionSrspFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtSysVersionSrspCb\n");
	consolePrint("TransportRev: 0x%02X\n", msg->TransportRev);
	consolePrint("Product: 0x%02X\n", msg->Product);
	consolePrint("MajorRel: 0x%02X\n", msg->MajorRel);
	consolePrint("MinorRel: 0x%02X\n", msg->MinorRel);
	consolePrint("MaintRel: 0x%02X\n", msg->MaintRel);
	SET_NRM_COLOR();
	return 0;
}
static uint8_t mtSysOsalNvReadSrspCb(OsalNvReadSrspFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtSysOsalNvReadSrspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("Len: 0x%02X\n", msg->Len);
		uint32_t i;
		for (i = 0; i < msg->Len; i++)
		{
			consolePrint("Value[%d]: 0x%02X\n", i, msg->Value[i]);
		}
	}
	else
	{
		consolePrint("OsalNvReadSrsp Status: FAIL 0x%02X\n", msg->Status);
	}
	SET_NRM_COLOR();
	return msg->Status;
}
static uint8_t mtSysOsalNvLengthSrspCb(OsalNvLengthSrspFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtSysOsalNvLengthSrspCb\n");
	consolePrint("ItemLen: 0x%04X\n", msg->ItemLen);
	SET_NRM_COLOR();
	return 0;
}
static uint8_t mtSysOsalTimerExpiredCb(OsalTimerExpiredFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtSysOsalTimerExpiredCb\n");
	consolePrint("Id: 0x%02X\n", msg->Id);
	SET_NRM_COLOR();
	return 0;
}
static uint8_t mtSysStackTuneSrspCb(StackTuneSrspFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtSysStackTuneSrspCb\n");
	consolePrint("Value: 0x%02X\n", msg->Value);
	SET_NRM_COLOR();
	return 0;
}
static uint8_t mtSysAdcReadSrspCb(AdcReadSrspFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtSysAdcReadSrspCb\n");
	consolePrint("Value: 0x%04X\n", msg->Value);
	SET_NRM_COLOR();
	return 0;
}
static uint8_t mtSysGpioSrspCb(GpioSrspFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtSysGpioSrspCb\n");
	consolePrint("Value: 0x%02X\n", msg->Value);
	SET_NRM_COLOR();
	return 0;
}
static uint8_t mtSysRandomSrspCb(RandomSrspFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtSysRandomSrspCb\n");
	consolePrint("Value: 0x%04X\n", msg->Value);
	SET_NRM_COLOR();
	return 0;
}
static uint8_t mtSysGetTimeSrspCb(GetTimeSrspFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtSysGetTimeSrspCb\n");
	consolePrint("UTCTime: 0x%08X\n", msg->UTCTime);
	consolePrint("Hour: 0x%02X\n", msg->Hour);
	consolePrint("Minute: 0x%02X\n", msg->Minute);
	consolePrint("Second: 0x%02X\n", msg->Second);
	consolePrint("Month: 0x%02X\n", msg->Month);
	consolePrint("Day: 0x%02X\n", msg->Day);
	consolePrint("Year: 0x%04X\n", msg->Year);
	SET_NRM_COLOR();
	return 0;
}
static uint8_t mtSysSetTxPowerSrspCb(SetTxPowerSrspFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtSysSetTxPowerSrspCb\n");
	consolePrint("TxPower: 0x%02X\n", msg->TxPower);
	SET_NRM_COLOR();
	return 0;
}
/********************************************************************
 * END OF SYS CALL BACK FUNCTIONS
 */

/********************************************************************
 * START OF ZDO CALL BACK FUNCTIONS
 */

/********************************************************************
 * @fn     Callback function for ZDO State Change Indication

 * @brief  receives the AREQ status and specifies the change ZDO state
 *
 * @param  uint8 zdoState
 *
 * @return SUCCESS or FAILURE
 */
static uint8_t mtZdoStateChangeIndCb(uint8_t newDevState)
{
	SET_RSP_COLOR();

	switch ((devStates_t) newDevState)
	{
	case DEV_HOLD:
		dbg_print(PRINT_LEVEL_INFO,
		        "mtZdoStateChangeIndCb: Initialized - not started automatically\n");
		break;
	case DEV_INIT:
		dbg_print(PRINT_LEVEL_INFO,
		        "mtZdoStateChangeIndCb: Initialized - not connected to anything\n");
		break;
	case DEV_NWK_DISC:
		dbg_print(PRINT_LEVEL_INFO,
		        "mtZdoStateChangeIndCb: Discovering PAN's to join\n");
		consolePrint("Network Discovering\n");
		break;
	case DEV_NWK_JOINING:
		dbg_print(PRINT_LEVEL_INFO, "mtZdoStateChangeIndCb: Joining a PAN\n");
		consolePrint("Network Joining\n");
		break;
	case DEV_NWK_REJOIN:
		dbg_print(PRINT_LEVEL_INFO,
		        "mtZdoStateChangeIndCb: ReJoining a PAN, only for end devices\n");
		consolePrint("Network Rejoining\n");
		break;
	case DEV_END_DEVICE_UNAUTH:
		consolePrint("Network Authenticating\n");
		dbg_print(PRINT_LEVEL_INFO,
		        "mtZdoStateChangeIndCb: Joined but not yet authenticated by trust center\n");
		break;
	case DEV_END_DEVICE:
		consolePrint("Network Joined\n");
		dbg_print(PRINT_LEVEL_INFO,
		        "mtZdoStateChangeIndCb: Started as device after authentication\n");
		break;
	case DEV_ROUTER:
		consolePrint("Network Joined\n");
		dbg_print(PRINT_LEVEL_INFO,
		        "mtZdoStateChangeIndCb: Device joined, authenticated and is a router\n");
		break;
	case DEV_COORD_STARTING:
		consolePrint("Network Starting\n");
		dbg_print(PRINT_LEVEL_INFO,
		        "mtZdoStateChangeIndCb: Started as Zigbee Coordinator\n");
		break;
	case DEV_ZB_COORD:
		consolePrint("Network Started\n");
		dbg_print(PRINT_LEVEL_INFO,
		        "mtZdoStateChangeIndCb: Started as Zigbee Coordinator\n");
		break;
	case DEV_NWK_ORPHAN:
		consolePrint("Network Orphaned\n");
		dbg_print(PRINT_LEVEL_INFO,
		        "mtZdoStateChangeIndCb: Device has lost information about its parent\n");
		break;
	default:
		dbg_print(PRINT_LEVEL_INFO, "mtZdoStateChangeIndCb: unknown state");
		break;
	}

	devState = (devStates_t) newDevState;

	SET_NRM_COLOR();

	return SUCCESS;
}

static uint8_t mtZdoGetLinkKeyCb(GetLinkKeySrspFormat_t *msg)
{
	SET_RSP_COLOR();

	consolePrint("mtZdoGetLinkKeyCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("IEEEAddr: 0x%016llX\n",
		        (long long unsigned int) msg->IEEEAddr);
	}
	else
	{
		consolePrint("GetLinkKey Status: FAIL 0x%02X\n", msg->Status);
	}
	SET_NRM_COLOR();

	return msg->Status;
}
static uint8_t mtZdoNwkAddrRspCb(NwkAddrRspFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtZdoNwkAddrRspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("IEEEAddr: 0x%016llX\n",
		        (long long unsigned int) msg->IEEEAddr);
		consolePrint("NwkAddr: 0x%04X\n", msg->NwkAddr);
		consolePrint("StartIndex: 0x%02X\n", msg->StartIndex);
		consolePrint("NumAssocDev: 0x%02X\n", msg->NumAssocDev);
		uint32_t i;
		for (i = 0; i < msg->NumAssocDev; i++)
		{
			consolePrint("AssocDevList[%d]: 0x%04X\n", i, msg->AssocDevList[i]);
		}
	}
	else
	{
		consolePrint("NwkAddrRsp Status: FAIL 0x%02X\n", msg->Status);
	}
	SET_NRM_COLOR();

	return msg->Status;
}
static uint8_t mtZdoIeeeAddrRspCb(IeeeAddrRspFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtZdoIeeeAddrRspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("IEEEAddr: 0x%016llX\n",
		        (long long unsigned int) msg->IEEEAddr);
		//consolePrint("%08X\n", msg -> IEEEAddr);
		consolePrint("NwkAddr: 0x%04X\n", msg->NwkAddr);
		consolePrint("StartIndex: 0x%02X\n", msg->StartIndex);
		consolePrint("NumAssocDev: 0x%02X\n", msg->NumAssocDev);
		uint32_t i;
		for (i = 0; i < msg->NumAssocDev; i++)
		{
			consolePrint("AssocDevList[%d]: 0x%04X\n", i, msg->AssocDevList[i]);
		}
	}
	else
	{
		consolePrint("IeeeAddrRsp Status: FAIL 0x%02X\n", msg->Status);
	}
	SET_NRM_COLOR();

	return msg->Status;
}
static uint8_t mtZdoNodeDescRspCb(NodeDescRspFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtZdoNodeDescRspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("NwkAddr: 0x%04X\n", msg->NwkAddr);
		consolePrint("LoTy_ComDescAv_UsrDesAv: 0x%02X\n",
		        msg->LoTy_ComDescAv_UsrDesAv);
		consolePrint("APSFlg_FrqBnd: 0x%02X\n", msg->APSFlg_FrqBnd);
		consolePrint("MACCapFlg: 0x%02X\n", msg->MACCapFlg);
		consolePrint("ManufacturerCode: 0x%04X\n", msg->ManufacturerCode);
		consolePrint("MaxBufferSize: 0x%02X\n", msg->MaxBufferSize);
		consolePrint("MaxTransferSize: 0x%04X\n", msg->MaxTransferSize);
		consolePrint("ServerMask: 0x%04X\n", msg->ServerMask);
		consolePrint("MaxOutTransferSize: 0x%04X\n", msg->MaxOutTransferSize);
		consolePrint("DescriptorCapabilities: 0x%02X\n",
		        msg->DescriptorCapabilities);
	}
	else
	{
		consolePrint("NodeDescRsp Status: FAIL 0x%02X\n", msg->Status);
	}
	SET_NRM_COLOR();

	return msg->Status;
}
static uint8_t mtZdoPowerDescRspCb(PowerDescRspFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtZdoPowerDescRspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("NwkAddr: 0x%04X\n", msg->NwkAddr);
		consolePrint("CurrntPwrMode_AvalPwrSrcs: 0x%02X\n",
		        msg->CurrntPwrMode_AvalPwrSrcs);
		consolePrint("CurrntPwrSrc_CurrntPwrSrcLvl: 0x%02X\n",
		        msg->CurrntPwrSrc_CurrntPwrSrcLvl);
	}
	else
	{
		consolePrint("PowerDescRsp Status: FAIL 0x%02X\n", msg->Status);
	}
	SET_NRM_COLOR();

	return msg->Status;
}
static uint8_t mtZdoSimpleDescRspCb(SimpleDescRspFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtZdoSimpleDescRspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("NwkAddr: 0x%04X\n", msg->NwkAddr);
		consolePrint("Len: 0x%02X\n", msg->Len);
		consolePrint("Endpoint: 0x%02X\n", msg->Endpoint);
		consolePrint("ProfileID: 0x%04X\n", msg->ProfileID);
		consolePrint("DeviceID: 0x%04X\n", msg->DeviceID);
		consolePrint("DeviceVersion: 0x%02X\n", msg->DeviceVersion);
		consolePrint("NumInClusters: 0x%02X\n", msg->NumInClusters);
		uint32_t i;
		for (i = 0; i < msg->NumInClusters; i++)
		{
			consolePrint("InClusterList[%d]: 0x%04X\n", i,
			        msg->InClusterList[i]);
		}
		consolePrint("NumOutClusters: 0x%02X\n", msg->NumOutClusters);
		for (i = 0; i < msg->NumOutClusters; i++)
		{
			consolePrint("OutClusterList[%d]: 0x%04X\n", i,
			        msg->OutClusterList[i]);
		}
	}
	else
	{
		consolePrint("SimpleDescRsp Status: FAIL 0x%02X\n", msg->Status);
	}
	SET_NRM_COLOR();

	return msg->Status;
}
static uint8_t mtZdoActiveEpRspCb(ActiveEpRspFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtZdoActiveEpRspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("NwkAddr: 0x%04X\n", msg->NwkAddr);
		consolePrint("ActiveEPCount: 0x%02X\n", msg->ActiveEPCount);
		uint32_t i;
		for (i = 0; i < msg->ActiveEPCount; i++)
		{
			consolePrint("ActiveEPList[%d]: 0x%02X\n", i, msg->ActiveEPList[i]);
		}
	}
	else
	{
		consolePrint("ActiveEpRsp Status: FAIL 0x%02X\n", msg->Status);
	}
	SET_NRM_COLOR();

	return msg->Status;
}
static uint8_t mtZdoMatchDescRspCb(MatchDescRspFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtZdoMatchDescRspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("NwkAddr: 0x%04X\n", msg->NwkAddr);
		consolePrint("MatchLength: 0x%02X\n", msg->MatchLength);
		uint32_t i;
		for (i = 0; i < msg->MatchLength; i++)
		{
			consolePrint("MatchList[%d]: 0x%02X\n", i, msg->MatchList[i]);
		}
	}
	else
	{
		consolePrint("MatchDescRsp Status: FAIL 0x%02X\n", msg->Status);
	}
	SET_NRM_COLOR();

	return msg->Status;
}
static uint8_t mtZdoComplexDescRspCb(ComplexDescRspFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtZdoComplexDescRspCb\n");

	consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
	consolePrint("Status: 0x%02X\n", msg->Status);
	consolePrint("NwkAddr: 0x%04X\n", msg->NwkAddr);
	consolePrint("ComplexLength: 0x%02X\n", msg->ComplexLength);
	uint32_t i;
	for (i = 0; i < msg->ComplexLength; i++)
	{
		consolePrint("ComplexList[%d]: 0x%02X\n", i, msg->ComplexList[i]);
	}
	SET_NRM_COLOR();

	return msg->Status;
}
static uint8_t mtZdoUserDescRspCb(UserDescRspFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtZdoUserDescRspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("NwkAddr: 0x%04X\n", msg->NwkAddr);
		consolePrint("Len: 0x%02X\n", msg->Len);
		uint32_t i;
		for (i = 0; i < msg->Len; i++)
		{
			consolePrint("CUserDescriptor[%d]: 0x%02X\n", i,
			        msg->CUserDescriptor[i]);
		}
	}
	else
	{
		consolePrint("UserDescRsp Status: FAIL 0x%02X\n", msg->Status);
	}
	SET_NRM_COLOR();

	return msg->Status;
}
static uint8_t mtZdoUserDescConfCb(UserDescConfFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtZdoUserDescConfCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("NwkAddr: 0x%04X\n", msg->NwkAddr);
	}
	else
	{
		consolePrint("UserDescConf Status: FAIL 0x%02X\n", msg->Status);
	}
	SET_NRM_COLOR();

	return msg->Status;
}
static uint8_t mtZdoServerDiscRspCb(ServerDiscRspFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtZdoServerDiscRspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("ServerMask: 0x%04X\n", msg->ServerMask);
	}
	else
	{
		consolePrint("ServerDiscRsp Status: FAIL 0x%02X\n", msg->Status);
	}
	SET_NRM_COLOR();

	return msg->Status;
}
static uint8_t mtZdoEndDeviceBindRspCb(EndDeviceBindRspFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtZdoEndDeviceBindRspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
		consolePrint("Status: 0x%02X\n", msg->Status);
	}
	else
	{
		consolePrint("EndDeviceBindRsp Status: FAIL 0x%02X\n", msg->Status);
	}
	SET_NRM_COLOR();

	return msg->Status;
}
static uint8_t mtZdoBindRspCb(BindRspFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtZdoBindRspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
		consolePrint("Status: 0x%02X\n", msg->Status);
	}
	else
	{
		consolePrint("BindRsp Status: FAIL 0x%02X\n", msg->Status);
	}
	SET_NRM_COLOR();

	return msg->Status;
}
static uint8_t mtZdoUnbindRspCb(UnbindRspFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtZdoUnbindRspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
		consolePrint("Status: 0x%02X\n", msg->Status);
	}
	else
	{
		consolePrint("UnbindRsp Status: FAIL 0x%02X\n", msg->Status);
	}
	SET_NRM_COLOR();

	return msg->Status;
}
static uint8_t mtZdoMgmtNwkDiscRspCb(MgmtNwkDiscRspFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtZdoMgmtNwkDiscRspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("NetworkCount: 0x%02X\n", msg->NetworkCount);
		consolePrint("StartIndex: 0x%02X\n", msg->StartIndex);
		consolePrint("NetworkListCount: 0x%02X\n", msg->NetworkListCount);

		uint32_t i;
		for (i = 0; i < msg->NetworkListCount; i++)
		{
			consolePrint("mtZdoNetworkListItems[%d]:\n", i);
			consolePrint("\tPanID: 0x%016llX\n",
			        (long long unsigned int) msg->NetworkList[i].PanID);
			consolePrint("\tLogicalChannel: 0x%02X\n",
			        msg->NetworkList[i].LogicalChannel);
			consolePrint("\tStackProf_ZigVer: 0x%02X\n",
			        msg->NetworkList[i].StackProf_ZigVer);
			consolePrint("\tBeacOrd_SupFramOrd: 0x%02X\n",
			        msg->NetworkList[i].BeacOrd_SupFramOrd);
			consolePrint("\tPermitJoin: 0x%02X\n\n",
			        msg->NetworkList[i].PermitJoin);
		}
	}
	else
	{
		consolePrint("MgmtNwkDiscRsp Status: FAIL 0x%02X\n", msg->Status);
	}
	SET_NRM_COLOR();

	return msg->Status;
}
static uint8_t mtZdoMgmtLqiRspCb(MgmtLqiRspFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtZdoMgmtLqiRspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("NeighborTableEntries: 0x%02X\n",
		        msg->NeighborTableEntries);
		consolePrint("StartIndex: 0x%02X\n", msg->StartIndex);
		consolePrint("NeighborLqiListCount: 0x%02X\n",
		        msg->NeighborLqiListCount);
		uint32_t i;
		for (i = 0; i < msg->NeighborLqiListCount; i++)
		{

			consolePrint("mtZdoNeighborLqiListItem[%d]:\n", i);

			consolePrint("\tExtendedPanID: 0x%016llX\n",
			        (long long unsigned int) msg->NeighborLqiList[i].ExtendedPanID);
			consolePrint("\tExtendedAddress: 0x%016llX\n",
			        (long long unsigned int) msg->NeighborLqiList[i].ExtendedAddress);
			consolePrint("\tNetworkAddress: 0x%04X\n",
			        msg->NeighborLqiList[i].NetworkAddress);
			consolePrint("\tDevTyp_RxOnWhenIdle_Relat: 0x%02X\n",
			        msg->NeighborLqiList[i].DevTyp_RxOnWhenIdle_Relat);
			consolePrint("\tPermitJoining: 0x%02X\n",
			        msg->NeighborLqiList[i].PermitJoining);
			consolePrint("\tDepth: 0x%02X\n", msg->NeighborLqiList[i].Depth);
			consolePrint("\tLQI: 0x%02X\n", msg->NeighborLqiList[i].LQI);
		}
	}
	else
	{
		consolePrint("MgmtLqiRsp Status: FAIL 0x%02X\n", msg->Status);
	}
	SET_NRM_COLOR();

	return msg->Status;
}
static uint8_t mtZdoMgmtRtgRspCb(MgmtRtgRspFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtZdoMgmtRtgRspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("RoutingTableEntries: 0x%02X\n", msg->RoutingTableEntries);
		consolePrint("StartIndex: 0x%02X\n", msg->StartIndex);
		consolePrint("RoutingTableListCount: 0x%02X\n",
		        msg->RoutingTableListCount);
		uint32_t i;
		for (i = 0; i < msg->RoutingTableListCount; i++)
		{
			consolePrint("RoutingTableListItem[%d]:\n", i);
			consolePrint("\tDstAddr: 0x%04X\n",
			        msg->RoutingTableList[i].DstAddr);
			consolePrint("\tStatus: 0x%02X\n", msg->RoutingTableList[i].Status);
			consolePrint("\tNextHop: 0x%04X\n",
			        msg->RoutingTableList[i].NextHop);
		}
	}
	else
	{
		consolePrint("MgmtRtgRsp Status: FAIL 0x%02X\n", msg->Status);
	}
	SET_NRM_COLOR();

	return msg->Status;
}
static uint8_t mtZdoMgmtBindRspCb(MgmtBindRspFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtZdoMgmtBindRspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("BindingTableEntries: 0x%02X\n", msg->BindingTableEntries);
		consolePrint("StartIndex: 0x%02X\n", msg->StartIndex);
		consolePrint("BindingTableListCount: 0x%02X\n",
		        msg->BindingTableListCount);
		uint32_t i;
		for (i = 0; i < msg->BindingTableListCount; i++)
		{
			consolePrint("BindingTableList[%d]:\n", i);
			consolePrint("SrcIEEEAddr: 0x%016llX\n",
			        (long long unsigned int) msg->BindingTableList[i].SrcIEEEAddr);
			consolePrint("\tSrcEndpoint: 0x%02X\n",
			        msg->BindingTableList[i].SrcEndpoint);
			consolePrint("\tClusterID: 0x%02X\n",
			        msg->BindingTableList[i].ClusterID);
			consolePrint("\tDstAddrMode: 0x%02X\n",
			        msg->BindingTableList[i].DstAddrMode);
			consolePrint("DstIEEEAddr: 0x%016llX\n",
			        (long long unsigned int) msg->BindingTableList[i].DstIEEEAddr);
			consolePrint("\tDstEndpoint: 0x%02X\n",
			        msg->BindingTableList[i].DstEndpoint);
		}
	}
	else
	{
		consolePrint("MgmtBindRsp Status: FAIL 0x%02X\n", msg->Status);
	}
	SET_NRM_COLOR();

	return msg->Status;
}
static uint8_t mtZdoMgmtLeaveRspCb(MgmtLeaveRspFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtZdoMgmtLeaveRspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
		consolePrint("Status: 0x%02X\n", msg->Status);
	}
	else
	{
		consolePrint("MgmtLeaveRsp Status: FAIL 0x%02X\n", msg->Status);
	}
	SET_NRM_COLOR();

	return msg->Status;
}
static uint8_t mtZdoMgmtDirectJoinRspCb(MgmtDirectJoinRspFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtZdoMgmtDirectJoinRspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
		consolePrint("Status: 0x%02X\n", msg->Status);
	}
	else
	{
		consolePrint("MgmtDirectJoinRsp Status: FAIL 0x%02X\n", msg->Status);
	}
	SET_NRM_COLOR();

	return msg->Status;
}
static uint8_t mtZdoMgmtPermitJoinRspCb(MgmtPermitJoinRspFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtZdoMgmtPermitJoinRspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
		consolePrint("Status: 0x%02X\n", msg->Status);
	}
	else
	{
		consolePrint("MgmtPermitJoinRsp Status: FAIL 0x%02X\n", msg->Status);
	}
	SET_NRM_COLOR();

	return msg->Status;
}
static uint8_t mtZdoEndDeviceAnnceIndCb(EndDeviceAnnceIndFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtZdoEndDeviceAnnceIndCb\n");
	consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
	consolePrint("NwkAddr: 0x%04X\n", msg->NwkAddr);
	consolePrint("IEEEAddr: 0x%016llX\n",
	        (long long unsigned int) msg->IEEEAddr);
	consolePrint("Capabilities: 0x%02X\n", msg->Capabilities);
	SET_NRM_COLOR();
	return 0;
}
static uint8_t mtZdoMatchDescRspSentCb(MatchDescRspSentFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtZdoMatchDescRspSentCb\n");
	consolePrint("NwkAddr: 0x%04X\n", msg->NwkAddr);
	consolePrint("NumInClusters: 0x%02X\n", msg->NumInClusters);
	uint32_t i;
	for (i = 0; i < msg->NumInClusters; i++)
	{
		consolePrint("InClusterList[%d]: 0x%04X\n", i, msg->InClusterList[i]);
	}
	consolePrint("NumOutClusters: 0x%02X\n", msg->NumOutClusters);
	for (i = 0; i < msg->NumOutClusters; i++)
	{
		consolePrint("OutClusterList[%d]: 0x%04X\n", i, msg->OutClusterList[i]);
	}
	SET_NRM_COLOR();

	return 0;
}
static uint8_t mtZdoStatusErrorRspCb(StatusErrorRspFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtZdoStatusErrorRspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
		consolePrint("Status: 0x%02X\n", msg->Status);
	}
	else
	{
		consolePrint("StatusErrorRsp Status: FAIL 0x%02X\n", msg->Status);
	}
	SET_NRM_COLOR();

	return msg->Status;
}
static uint8_t mtZdoSrcRtgIndCb(SrcRtgIndFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtZdoSrcRtgIndCb\n");
	consolePrint("DstAddr: 0x%04X\n", msg->DstAddr);
	consolePrint("RelayCount: 0x%02X\n", msg->RelayCount);
	uint32_t i;
	for (i = 0; i < msg->RelayCount; i++)
	{
		consolePrint("RelayList[%d]: 0x%04X\n", i, msg->RelayList[i]);
	}
	SET_NRM_COLOR();

	return 0;
}
static uint8_t mtZdoBeaconNotifyIndCb(BeaconNotifyIndFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtZdoBeaconNotifyIndCb\n");
	consolePrint("BeaconCount: 0x%02X\n", msg->BeaconCount);
	uint32_t i;
	for (i = 0; i < msg->BeaconCount; i++)
	{
		consolePrint("BeaconListItem[%d]:\n", i);

		consolePrint("\tSrcAddr: 0x%04X\n", msg->BeaconList[i].SrcAddr);
		consolePrint("\tPanId: 0x%04X\n", msg->BeaconList[i].PanId);
		consolePrint("\tLogicalChannel: 0x%02X\n",
		        msg->BeaconList[i].LogicalChannel);
		consolePrint("\tPermitJoining: 0x%02X\n",
		        msg->BeaconList[i].PermitJoining);
		consolePrint("\tRouterCap: 0x%02X\n", msg->BeaconList[i].RouterCap);
		consolePrint("\tPDevCap: 0x%02X\n", msg->BeaconList[i].DevCap);
		consolePrint("\tProtocolVer: 0x%02X\n", msg->BeaconList[i].ProtocolVer);
		consolePrint("\tStackProf: 0x%02X\n", msg->BeaconList[i].StackProf);
		consolePrint("\tLQI: 0x%02X\n", msg->BeaconList[i].Lqi);
		consolePrint("\tDepth: 0x%02X\n", msg->BeaconList[i].Depth);
		consolePrint("\tUpdateId: 0x%02X\n", msg->BeaconList[i].UpdateId);
		consolePrint("ExtendedPanID: 0x%016llX\n",
		        (long long unsigned int) msg->BeaconList[i].ExtendedPanId);
	}
	SET_NRM_COLOR();

	return 0;
}
static uint8_t mtZdoJoinCnfCb(JoinCnfFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtZdoJoinCnfCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("DevAddr: 0x%04X\n", msg->DevAddr);
		consolePrint("ParentAddr: 0x%04X\n", msg->ParentAddr);
	}
	else
	{
		consolePrint("JoinCnf Status: FAIL 0x%02X\n", msg->Status);
	}
	SET_NRM_COLOR();

	return msg->Status;
}
static uint8_t mtZdoNwkDiscoveryCnfCb(NwkDiscoveryCnfFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtZdoNwkDiscoveryCnfCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("Status: 0x%02X\n", msg->Status);
	}
	else
	{
		consolePrint("NwkDiscoveryCnf Status: FAIL 0x%02X\n", msg->Status);
	}
	SET_NRM_COLOR();

	return msg->Status;
}
static uint8_t mtZdoLeaveIndCb(LeaveIndFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtZdoLeaveIndCb\n");
	consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
	consolePrint("ExtAddr: 0x%016llX\n", (long long unsigned int) msg->ExtAddr);
	consolePrint("Request: 0x%02X\n", msg->Request);
	consolePrint("Remove: 0x%02X\n", msg->Remove);
	consolePrint("Rejoin: 0x%02X\n", msg->Rejoin);
	return 0;
}
static uint8_t mtZdoMsgCbIncomingCb(MsgCbIncomingFormat_t *msg)
{
	SET_RSP_COLOR();

	consolePrint("mtZdoMsgCbIncomingCb\n");
	consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
	consolePrint("WasBroadcast: 0x%02X\n", msg->WasBroadcast);
	consolePrint("ClusterID: 0x%04X\n", msg->ClusterID);
	consolePrint("SecurityUse: 0x%02X\n", msg->SecurityUse);
	consolePrint("SeqNum: 0x%02X\n", msg->SeqNum);
	consolePrint("MacDstAddr: 0x%04X\n", msg->MacDstAddr);
	consolePrint("Status: 0x%02X\n", msg->Status);
	consolePrint("ExtAddr: 0x%016llX\n", (long long unsigned int) msg->ExtAddr);
	consolePrint("NwkAddr: 0x%04X\n", msg->NwkAddr);

	SET_NRM_COLOR();
	return 0;
}

/********************************************************************
 * END OF ZDO CALL BACK FUNCTIONS
 */

/********************************************************************
 * START OF AF CALL BACK FUNCTIONS
 */

static uint8_t mtAfDataConfirmCb(DataConfirmFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtAfDataConfirmCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("Endpoint: 0x%02X\n", msg->Endpoint);
		consolePrint("TransId: 0x%02X\n", msg->TransId);
	}
	else
	{
		consolePrint("DataConfirm Status: FAIL 0x%02X\n", msg->Status);
	}
	SET_NRM_COLOR();

	return msg->Status;
}
static uint8_t mtAfIncomingMsgCb(IncomingMsgFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtAfIncomingMsgCb\n");
	consolePrint("GroupId: 0x%04X\n", msg->GroupId);
	consolePrint("ClusterId: 0x%04X\n", msg->ClusterId);
	consolePrint("SrcAddr: 0x%04X\n", msg->SrcAddr);
	consolePrint("SrcEndpoint: 0x%02X\n", msg->SrcEndpoint);
	consolePrint("DstEndpoint: 0x%02X\n", msg->DstEndpoint);
	consolePrint("WasVroadcast: 0x%02X\n", msg->WasVroadcast);
	consolePrint("LinkQuality: 0x%02X\n", msg->LinkQuality);
	consolePrint("SecurityUse: 0x%02X\n", msg->SecurityUse);
	consolePrint("TimeStamp: 0x%08X\n", msg->TimeStamp);
	consolePrint("TransSeqNum: 0x%02X\n", msg->TransSeqNum);
	consolePrint("Len: 0x%02X\n", msg->Len);
	uint32_t i;
	for (i = 0; i < msg->Len; i++)
	{
		consolePrint("Data[%d]: 0x%02X\n", i, msg->Data[i]);
	}
	SET_NRM_COLOR();

	return 0;
}
static uint8_t mtAfIncomingMsgExt(IncomingMsgExtFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtAfIncomingMsgExt\n");
	consolePrint("GroupId: 0x%04X\n", msg->GroupId);
	consolePrint("ClusterId: 0x%04X\n", msg->ClusterId);
	consolePrint("SrcAddrMode: 0x%02X\n", msg->SrcAddrMode);
	consolePrint("SrcAddr: 0x%016llX\n", (long long unsigned int) msg->SrcAddr);
	consolePrint("SrcEndpoint: 0x%02X\n", msg->SrcEndpoint);
	consolePrint("SrcPanId: 0x%04X\n", msg->SrcPanId);
	consolePrint("DstEndpoint: 0x%02X\n", msg->DstEndpoint);
	consolePrint("WasVroadcast: 0x%02X\n", msg->WasVroadcast);
	consolePrint("LinkQuality: 0x%02X\n", msg->LinkQuality);
	consolePrint("SecurityUse: 0x%02X\n", msg->SecurityUse);
	consolePrint("TimeStamp: 0x%08X\n", msg->TimeStamp);
	consolePrint("TransSeqNum: 0x%02X\n", msg->TransSeqNum);
	consolePrint("Len: 0x%02X\n", msg->Len);
	uint32_t i;
	for (i = 0; i < msg->Len; i++)
	{
		consolePrint("Data[%d]: 0x%02X\n", i, msg->Data[i]);
	}
	SET_NRM_COLOR();

	return 0;
}
static uint8_t mtAfDataRetrieveSrspCb(DataRetrieveSrspFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtAfDataRetrieveSrspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("Length: 0x%02X\n", msg->Length);
		uint32_t i;
		for (i = 0; i < msg->Length; i++)
		{
			consolePrint("Data[%d]: 0x%02X\n", i, msg->Data[i]);
		}
	}
	else
	{
		consolePrint("DataRetrieveSrsp Status: FAIL 0x%02X\n", msg->Status);
	}
	SET_NRM_COLOR();

	return msg->Status;
}
static uint8_t mtAfReflectErrorCb(ReflectErrorFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtAfReflectErrorCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("Endpoint: 0x%02X\n", msg->Endpoint);
		consolePrint("TransId: 0x%02X\n", msg->TransId);
		consolePrint("DstAddrMode: 0x%02X\n", msg->DstAddrMode);
		consolePrint("DstAddr: 0x%04X\n", msg->DstAddr);
	}
	else
	{
		consolePrint("ReflectError Status: FAIL 0x%02X\n", msg->Status);
	}

	SET_NRM_COLOR();
	return msg->Status;
}

/********************************************************************
 * END OF AF CALL BACK FUNCTIONS
 */

/********************************************************************
 * START OF SAPI CALL BACK FUNCTIONS
 */

static uint8_t mtSapiReadConfigurationSrspCb(ReadConfigurationSrspFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtSapiReadConfigurationSrspCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("Status: 0x%02X\n", msg->Status);
		consolePrint("ConfigId: 0x%02X\n", msg->ConfigId);
		consolePrint("Len: 0x%02X\n", msg->Len);
		uint32_t i;
		for (i = 0; i < msg->Len; i++)
		{
			consolePrint("Value[%d]: 0x%02X\n", i, msg->Value[i]);
		}
	}
	else
	{
		consolePrint("ReadConfigurationSrsp Status: FAIL 0x%02X\n",
		        msg->Status);
	}
	SET_NRM_COLOR();
	return msg->Status;
}
static uint8_t mtSapiGetDeviceInfoSrspCb(GetDeviceInfoSrspFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtSapiGetDeviceInfoSrspCb\n");

	switch (msg->Param)
	{
	case 0:
		consolePrint("Param: (0x%02X) State\n", msg->Param);
		consolePrint("Value: 0x%01X\n", msg->Value[0]);
		break;
	case 1:
		consolePrint("Param: (0x%02X) IEEE Address\n", msg->Param);
		consolePrint(
		        "Value: 0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X\n",
		        (unsigned char) msg->Value[0], (unsigned char) msg->Value[1],
		        (unsigned char) msg->Value[2], (unsigned char) msg->Value[3],
		        (unsigned char) msg->Value[4], (unsigned char) msg->Value[5],
		        (unsigned char) msg->Value[6], (unsigned char) msg->Value[7]);
		break;
	case 2:
		consolePrint("Param: (0x%02X) Short Address\n", msg->Param);
		consolePrint("Value: 0x%04X\n",
		        BUILD_UINT16(msg->Value[0], msg->Value[1]));
		break;
	case 3:
		consolePrint("Param: (0x%02X) Parent Short Address\n", msg->Param);
		consolePrint("Value: 0x%04X\n",
		        BUILD_UINT16(msg->Value[0], msg->Value[1]));
		break;
	case 4:
		consolePrint("Param: (0x%02X) Parent IEEE Address\n", msg->Param);
		consolePrint(
		        "Value: 0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X\n",
		        (unsigned char) msg->Value[0], (unsigned char) msg->Value[1],
		        (unsigned char) msg->Value[2], (unsigned char) msg->Value[3],
		        (unsigned char) msg->Value[4], (unsigned char) msg->Value[5],
		        (unsigned char) msg->Value[6], (unsigned char) msg->Value[7]);
		break;
	case 5:
		consolePrint("Param: (0x%02X) Channel\n", msg->Param);
		consolePrint("Value: 0x%01X\n", msg->Value[0]);
		break;
	case 6:
		consolePrint("Param: (0x%02X) PAN ID\n", msg->Param);
		consolePrint("Value: 0x%04X\n",
		        BUILD_UINT16(msg->Value[0], msg->Value[1]));
		break;
	case 7:
		consolePrint("Param: (0x%02X) Extended PAN ID\n", msg->Param);
		consolePrint(
		        "Value: 0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X\n",
		        (unsigned char) msg->Value[0], (unsigned char) msg->Value[1],
		        (unsigned char) msg->Value[2], (unsigned char) msg->Value[3],
		        (unsigned char) msg->Value[4], (unsigned char) msg->Value[5],
		        (unsigned char) msg->Value[6], (unsigned char) msg->Value[7]);
		break;

	}

	SET_NRM_COLOR();

	return 0;
}
static uint8_t mtSapiFindDeviceCnfCb(FindDeviceCnfFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtSapiFindDeviceCnfCb\n");
	consolePrint("SearchKey: 0x%04X\n", msg->SearchKey);
	consolePrint("Result: 0x%016llX\n", (long long unsigned int) msg->Result);
	SET_NRM_COLOR();
	return 0;
}
static uint8_t mtSapiSendDataCnfCb(SendDataCnfFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtSapiSendDataCnfCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("Handle: 0x%02X\n", msg->Handle);
		consolePrint("Status: 0x%02X\n", msg->Status);
	}
	else
	{
		consolePrint("SendDataCnf Status: FAIL 0x%02X\n", msg->Status);
	}
	SET_NRM_COLOR();
	return msg->Status;
}
static uint8_t mtSapiReceiveDataIndCb(ReceiveDataIndFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtSapiReceiveDataIndCb\n");
	consolePrint("Source: 0x%04X\n", msg->Source);
	consolePrint("Command: 0x%04X\n", msg->Command);
	consolePrint("Len: 0x%04X\n", msg->Len);
	uint32_t i;
	for (i = 0; i < msg->Len; i++)
	{
		consolePrint("Data[%d]: 0x%02X\n", i, msg->Data[i]);
	}
	SET_NRM_COLOR();
	return 0;
}
static uint8_t mtSapiAllowBindCnfCb(AllowBindCnfFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtSapiAllowBindCnfCb\n");
	consolePrint("Source: 0x%04X\n", msg->Source);
	SET_NRM_COLOR();
	return 0;
}
static uint8_t mtSapiBindCnfCb(BindCnfFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtSapiBindCnfCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("CommandId: 0x%04X\n", msg->CommandId);
		consolePrint("Status: 0x%02X\n", msg->Status);
	}
	else
	{
		consolePrint("BindCnf Status: FAIL 0x%02X\n", msg->Status);
	}
	SET_NRM_COLOR();
	return msg->Status;
}
static uint8_t mtSapiStartCnfCb(StartCnfFormat_t *msg)
{
	SET_RSP_COLOR();
	consolePrint("mtSapiStartCnfCb\n");
	if (msg->Status == MT_RPC_SUCCESS)
	{
		consolePrint("Status: 0x%02X\n", msg->Status);
	}
	else
	{
		consolePrint("StartCnf Status: FAIL 0x%02X\n", msg->Status);
	}
	SET_NRM_COLOR();
	return msg->Status;
}

/********************************************************************
 * END OF SAPI CALL BACK FUNCTIONS
 */

// helper functions for building and sending the NV messages
static uint8_t setNVStartup(uint8_t startupOption)
{
	uint8_t status;
	OsalNvWriteFormat_t nvWrite;

	// sending startup option
	nvWrite.Id = ZCD_NV_STARTUP_OPTION;
	nvWrite.Offset = 0;
	nvWrite.Len = 1;
	nvWrite.Value[0] = startupOption;
	status = sysOsalNvWrite(&nvWrite);

	dbg_print(PRINT_LEVEL_INFO, "\n");

	dbg_print(PRINT_LEVEL_INFO, "NV Write Startup Option cmd sent[%d]...\n",
	        status);

	return status;
}

static uint8_t setNVDevType(uint8_t devType)
{
	uint8_t status;
	OsalNvWriteFormat_t nvWrite;

	nvWrite.Id = ZCD_NV_LOGICAL_TYPE;
	nvWrite.Offset = 0;
	nvWrite.Len = 1;
	nvWrite.Value[0] = devType;
	status = sysOsalNvWrite(&nvWrite);

	dbg_print(PRINT_LEVEL_INFO, "\n");
	dbg_print(PRINT_LEVEL_INFO, "NV Write Device Type cmd sent... [%d]\n",
	        status);

	return status;
}

static uint8_t setNVPanID(uint32_t panId)
{
	uint8_t status;
	OsalNvWriteFormat_t nvWrite;

	dbg_print(PRINT_LEVEL_INFO, "\n");
	dbg_print(PRINT_LEVEL_INFO, "NV Write PAN ID cmd sending...\n");

	nvWrite.Id = ZCD_NV_PANID;
	nvWrite.Offset = 0;
	nvWrite.Len = 2;
	nvWrite.Value[0] = LO_UINT16(panId);
	nvWrite.Value[1] = HI_UINT16(panId);
	status = sysOsalNvWrite(&nvWrite);

	dbg_print(PRINT_LEVEL_INFO, "\n");
	dbg_print(PRINT_LEVEL_INFO, "NV Write PAN ID cmd sent...[%d]\n", status);

	return status;
}

static uint8_t setNVChanList(uint32_t chanList)
{
	OsalNvWriteFormat_t nvWrite;
	uint8_t status;

	// setting chanList
	nvWrite.Id = ZCD_NV_CHANLIST;
	nvWrite.Offset = 0;
	nvWrite.Len = 4;
	nvWrite.Value[0] = BREAK_UINT32(chanList, 0);
	nvWrite.Value[1] = BREAK_UINT32(chanList, 1);
	nvWrite.Value[2] = BREAK_UINT32(chanList, 2);
	nvWrite.Value[3] = BREAK_UINT32(chanList, 3);
	status = sysOsalNvWrite(&nvWrite);

	dbg_print(PRINT_LEVEL_INFO, "\n");
	dbg_print(PRINT_LEVEL_INFO, "NV Write Channel List cmd sent...[%d]\n",
	        status);

	return status;
}

static int32_t startNetwork(void)
{
	char cDevType;
	uint8_t devType;
	int32_t status;
	uint8_t newNwk = 0;
	char sCh[128];

	do
	{
		consolePrint("Do you wish to start/join a new network? (y/n)\n");
		consoleGetLine(sCh, 128);
		if (sCh[0] == 'n' || sCh[0] == 'N')
		{
			status = setNVStartup(0);
		}
		else if (sCh[0] == 'y' || sCh[0] == 'Y')
		{
			status = setNVStartup(
			ZCD_STARTOPT_CLEAR_STATE | ZCD_STARTOPT_CLEAR_CONFIG);
			newNwk = 1;

		}
		else
		{
			consolePrint("Incorrect input please type y or n\n");
		}
	} while (sCh[0] != 'y' && sCh[0] != 'Y' && sCh[0] != 'n' && sCh[0] != 'N');

	if (status != MT_RPC_SUCCESS)
	{
		dbg_print(PRINT_LEVEL_WARNING, "network start failed\n");
		return -1;
	}
	consolePrint("Resetting ZNP\n");
	ResetReqFormat_t resReq;
	resReq.Type = 1;
	sysResetReq(&resReq);
	//flush the rsp
	rpcWaitMqClientMsg(5000);

	if (newNwk)
	{
#ifndef CC26xx
		consolePrint(
		        "Enter device type c: Coordinator, r: Router, e: End Device:\n");
		consoleGetLine(sCh, 128);
		cDevType = sCh[0];

		switch (cDevType)
		{
		case 'c':
		case 'C':
			devType = DEVICETYPE_COORDINATOR;
			break;
		case 'r':
		case 'R':
			devType = DEVICETYPE_ROUTER;
			break;
		case 'e':
		case 'E':
		default:
			devType = DEVICETYPE_ENDDEVICE;
			break;
		}
		
		status = setNVDevType(devType);

		if (status != MT_RPC_SUCCESS)
		{
			dbg_print(PRINT_LEVEL_WARNING, "setNVDevType failed\n");
			return 0;
		}
#endif // CC26xx


		//Select random PAN ID for Coord and join any PAN for RTR/ED
		status = setNVPanID(0xFFFF);
		if (status != MT_RPC_SUCCESS)
		{
			dbg_print(PRINT_LEVEL_WARNING, "setNVPanID failed\n");
			return -1;
		}
		consolePrint("Enter channel 11-26:\n");
		consoleGetLine(sCh, 128);

		status = setNVChanList(1 << atoi(sCh));
		if (status != MT_RPC_SUCCESS)
		{
			dbg_print(PRINT_LEVEL_INFO, "setNVPanID failed\n");
			return -1;
		}

	}

	//注册一个节点
	registerAf();
	consolePrint("EndPoint: 1\n");

	status = zdoInit();
	if (status == NEW_NETWORK)
	{
		dbg_print(PRINT_LEVEL_INFO, "zdoInit NEW_NETWORK\n");
		status = MT_RPC_SUCCESS;
	}
	else if (status == RESTORED_NETWORK)
	{
		dbg_print(PRINT_LEVEL_INFO, "zdoInit RESTORED_NETWORK\n");
		status = MT_RPC_SUCCESS;
	}
	else
	{
		dbg_print(PRINT_LEVEL_INFO, "zdoInit failed\n");
		status = -1;
	}

	dbg_print(PRINT_LEVEL_INFO, "process zdoStatechange callbacks\n");

	//flush AREQ ZDO State Change messages
	while (status != -1)
	{
		status = rpcWaitMqClientMsg(5000);

		if (((devType == DEVICETYPE_COORDINATOR) && (devState == DEV_ZB_COORD))
		        || ((devType == DEVICETYPE_ROUTER) && (devState == DEV_ROUTER))
		        || ((devType == DEVICETYPE_ENDDEVICE)
		                && (devState == DEV_END_DEVICE)))
		{
			break;
		}
	}
	//set startup option back to keep configuration in case of reset
	status = setNVStartup(0);
	if (devState < DEV_END_DEVICE)
	{
		//start network failed
		return -1;
	}

	return 0;
}

static int32_t registerAf(void)
{
	int32_t status = 0;
	RegisterFormat_t reg;

	reg.EndPoint = 1;
	reg.AppProfId = 0x0104;
	reg.AppDeviceId = 0x0100;
	reg.AppDevVer = 1;
	reg.LatencyReq = 0;
	reg.AppNumInClusters = 1;
	reg.AppInClusterList[0] = 0x0006;
	reg.AppNumOutClusters = 0;

	status = afRegister(&reg);
	return status;
}

/*********************************************************************
 * INTERFACE FUNCTIONS
 */
uint32_t appInit(void)
{
	int32_t status = 0;
	uint32_t msgCnt = 0;

	//Flush all messages from the que
	//清空消息队列	
	while (status != -1)
	{
		
		status = rpcWaitMqClientMsg(10);
		//如果还有消息存在，则一直读取消息，直到没有消息位置
		if (status != -1)
		{
			msgCnt++;
		}
	}


	dbg_print(PRINT_LEVEL_INFO, "flushed %d message from msg queue\n", msgCnt);

	//Register Callbacks MT system callbacks
	//注册MT_SYS层的CallBack处理函数
	sysRegisterCallbacks(mtSysCb);
	
	zdoRegisterCallbacks(mtZdoCb);
	afRegisterCallbacks(mtAfCb);
	sapiRegisterCallbacks(mtSapiCb);

	return 0;
}
/*********************************************************************
 * START OF COMMANDS PROCESSING
 */
static void sendCmd(uint8_t* req, uint8_t index)
{
	switch (index)
	{
	case 0:
		sysPing();
		break;
	case 1:
		sysSetExtAddr((SetExtAddrFormat_t*) req);
		break;
	case 2:
		sysGetExtAddr();
		break;
	case 3:
		sysRamRead((RamReadFormat_t*) req);
		break;
	case 4:
		sysRamWrite((RamWriteFormat_t*) req);
		break;
	case 5:
		sysResetReq((ResetReqFormat_t*) req);
		break;
	case 6:
		sysVersion();
		break;
	case 7:
		sysOsalNvRead((OsalNvReadFormat_t*) req);
		break;
	case 8:
		sysOsalNvWrite((OsalNvWriteFormat_t*) req);
		break;
	case 9:
		sysOsalNvItemInit((OsalNvItemInitFormat_t*) req);
		break;
	case 10:
		sysOsalNvDelete((OsalNvDeleteFormat_t*) req);
		break;
	case 11:
		sysOsalNvLength((OsalNvLengthFormat_t*) req);
		break;
	case 12:
		sysOsalStartTimer((OsalStartTimerFormat_t*) req);
		break;
	case 13:
		sysOsalStopTimer((OsalStopTimerFormat_t*) req);
		break;
	case 14:
		sysStackTune((StackTuneFormat_t*) req);
		break;
	case 15:
		sysAdcRead((AdcReadFormat_t*) req);
		break;
	case 16:
		sysGpio((GpioFormat_t*) req);
		break;
	case 17:
		sysRandom();
		break;
	case 18:
		sysSetTime((SetTimeFormat_t*) req);
		break;
	case 19:
		sysGetTime();
		break;
	case 20:
		sysSetTxPower((SetTxPowerFormat_t*) req);
		break;
	case 21:
		afRegister((RegisterFormat_t*) req);
		break;
	case 22:
		afDataRequest((DataRequestFormat_t*) req);
		break;
	case 23:
		afDataRequestExt((DataRequestExtFormat_t*) req);
		break;
	case 24:
		afDataRequestSrcRtg((DataRequestSrcRtgFormat_t*) req);
		break;
	case 25:
		afInterPanCtl((InterPanCtlFormat_t*) req);
		break;
	case 26:
		afDataStore((DataStoreFormat_t*) req);
		break;
	case 27:
		afDataRetrieve((DataRetrieveFormat_t*) req);
		break;
	case 28:
		afApsfConfigSet((ApsfConfigSetFormat_t*) req);
		break;
	case 29:
		zdoNwkAddrReq((NwkAddrReqFormat_t*) req);
		break;
	case 30:
		zdoIeeeAddrReq((IeeeAddrReqFormat_t*) req);
		break;
	case 31:
		zdoNodeDescReq((NodeDescReqFormat_t*) req);
		break;
	case 32:
		zdoPowerDescReq((PowerDescReqFormat_t*) req);
		break;
	case 33:
		zdoSimpleDescReq((SimpleDescReqFormat_t*) req);
		break;
	case 34:
		zdoActiveEpReq((ActiveEpReqFormat_t*) req);
		break;
	case 35:
		zdoMatchDescReq((MatchDescReqFormat_t*) req);
		break;
	case 36:
		zdoComplexDescReq((ComplexDescReqFormat_t*) req);
		break;
	case 37:
		zdoUserDescReq((UserDescReqFormat_t*) req);
		break;
	case 38:
		zdoDeviceAnnce((DeviceAnnceFormat_t*) req);
		break;
	case 39:
		zdoUserDescSet((UserDescSetFormat_t*) req);
		break;
	case 40:
		zdoServerDiscReq((ServerDiscReqFormat_t*) req);
		break;
	case 41:
		zdoEndDeviceBindReq((EndDeviceBindReqFormat_t*) req);
		break;
	case 42:
		zdoBindReq((BindReqFormat_t*) req);
		break;
	case 43:
		zdoUnbindReq((UnbindReqFormat_t*) req);
		break;
	case 44:
		zdoMgmtNwkDiscReq((MgmtNwkDiscReqFormat_t*) req);
		break;
	case 45:
		zdoMgmtLqiReq((MgmtLqiReqFormat_t*) req);
		break;
	case 46:
		zdoMgmtRtgReq((MgmtRtgReqFormat_t*) req);
		break;
	case 47:
		zdoMgmtBindReq((MgmtBindReqFormat_t*) req);
		break;
	case 48:
		zdoMgmtLeaveReq((MgmtLeaveReqFormat_t*) req);
		break;
	case 49:
		zdoMgmtDirectJoinReq((MgmtDirectJoinReqFormat_t*) req);
		break;
	case 50:
		zdoMgmtPermitJoinReq((MgmtPermitJoinReqFormat_t*) req);
		break;
	case 51:
		zdoMgmtNwkUpdateReq((MgmtNwkUpdateReqFormat_t*) req);
		break;
	case 52:
		zdoStartupFromApp((StartupFromAppFormat_t*) req);
		break;
	case 53:
		zdoAutoFindDestination((AutoFindDestinationFormat_t*) req);
		break;
	case 54:
		zdoSetLinkKey((SetLinkKeyFormat_t*) req);
		break;
	case 55:
		zdoRemoveLinkKey((RemoveLinkKeyFormat_t*) req);
		break;
	case 56:
		zdoGetLinkKey((GetLinkKeyFormat_t*) req);
		break;
	case 57:
		zdoNwkDiscoveryReq((NwkDiscoveryReqFormat_t*) req);
		break;
	case 58:
		zdoJoinReq((JoinReqFormat_t*) req);
		break;
	case 59:
		zdoMsgCbRegister((MsgCbRegisterFormat_t*) req);
		break;
	case 60:
		zdoMsgCbRemove((MsgCbRemoveFormat_t*) req);
		break;
	case 61:
		zbSystemReset();
		break;
	case 62:
		zbAppRegisterReq((AppRegisterReqFormat_t*) req);
		break;
	case 63:
		zbStartReq();
		break;
	case 64:
		zbPermitJoiningReq((PermitJoiningReqFormat_t*) req);
		break;
	case 65:
		zbBindDevice((BindDeviceFormat_t*) req);
		break;
	case 66:
		zbAllowBind((AllowBindFormat_t*) req);
		break;
	case 67:
		zbSendDataReq((SendDataReqFormat_t*) req);
		break;
	case 68:
		zbFindDeviceReq((FindDeviceReqFormat_t*) req);
		break;
	case 69:
		zbWriteConfiguration((WriteConfigurationFormat_t*) req);
		break;
	case 70:
		zbGetDeviceInfo((GetDeviceInfoFormat_t*) req);
		break;
	case 71:
		zbReadConfiguration((ReadConfigurationFormat_t*) req);
		break;

	}

}
uint8_t matchedCmds[250];
uint8_t matchedLength;
static void inputCmd(uint16_t index)
{
	char value[128];
	char* strPos = value;
	uint8_t commandSize = 0;
	uint8_t attSize = 0;
	uint16_t currentPos = 0;

	uint8_t x;
	for (x = 0; x < commands[index].attNum; x++)
	{
		commandSize += commands[index].atts[x].size
		        * (commands[index].atts[x].isList == 0 ?
		                1 : commands[index].atts[x].isList);
		commandSize += (
		        (commands[index].atts[x].size == 2 && (commandSize % 2) != 0) ?
		                1 : 0);
	}

	uint8_t *input = malloc(commandSize);
	int tem;

	SET_PARAM_COLOR();
	consolePrint("Command: %s\n", commands[index].cmdName);
	SET_NRM_COLOR();
	for (x = 0; x < commands[index].attNum; x++)
	{
		attSize = commands[index].atts[x].size;
		if (commands[index].atts[x].isList == 0)
		{
			SET_PARAM_COLOR();
			consolePrint("Enter %s: (%dB)\n", commands[index].atts[x].name,
			        attSize);
			SET_NRM_COLOR();
			consoleGetLine(value, 128);

			if (attSize == 2)
			{
				sscanf(value, "%x", &tem);
				sprintf(value, "%04X", tem);
				//strPos = value;
				if (currentPos % 2 != 0)
				{
					currentPos++;
				}
			}
			else if (attSize == 4)
			{
				sscanf(value, "%x", &tem);
				sprintf(value, "%08X", tem);
				//strPos = value;
			}
			else if (attSize == 1)
			{
				sscanf(value, "%x", &tem);
				sprintf(value, "%02X", tem);
				//strPos = value;
			}
			strPos = value;
			uint8_t idx;
			for (idx = 0; idx < attSize; idx++)
			{
				if (strlen(strPos) > 0)
				{

					sscanf(strPos, "%2hhx",
					        &input[currentPos + attSize - 1 - idx]);
					strPos += (strPos[2] == ':' ? 3 : 2);
				}
				else
				{
					input[currentPos + attSize - 1 - idx] = 0;
				}

			}
			currentPos += attSize;
		}
		else
		{
			uint16_t tempPos = currentPos;
			uint8_t prevAttSize = commands[index].atts[x - 1].size;
			uint16_t maxLen = commands[index].atts[x].isList;
			uint16_t listLen = (prevAttSize == 1) ?
                                    ((uint16_t)input[currentPos - 1]) :
                                    BUILD_UINT16(input[currentPos - 2], input[currentPos - 1]);
			uint16_t listIdx;

			if(listLen >= maxLen)
            {
                consolePrint("\nPlease enter a length less than 0x%02X\n\n", maxLen);
                x-=2;
                currentPos -= prevAttSize;
            }
			else
            {
                for (listIdx = 0; listIdx < listLen; listIdx++)
                {
                    SET_PARAM_COLOR();
                    consolePrint("Enter %s[%d]:\n", commands[index].atts[x].name,
                            listIdx);
                    SET_NRM_COLOR();
                    consoleGetLine(value, 128);
                    if (attSize == 2)
                    {
                        sscanf(value, "%x", &tem);
                        sprintf(value, "%04X", tem);
                        //strPos = value;
                        if (tempPos % 2 != 0)
                        {
                            tempPos++;
                            currentPos++;
                        }
                    }
                    else if (attSize == 1)
                    {
                        sscanf(value, "%x", &tem);
                        sprintf(value, "%02X", tem);
                        //strPos = value;
                    }
                    strPos = value;
                    uint8_t idx;
                    for (idx = 0; idx < attSize; idx++)
                    {
                        if (strlen(strPos) > 0)
                        {
                            sscanf(strPos, "%2hhx",
                                    &input[tempPos + attSize - 1 - idx]);
                            strPos += (strPos[2] == ':' ? 3 : 2);
                        }
                        else
                        {
                            input[currentPos + attSize - 1 - idx] = 0;
                        }
                    }
                    tempPos += attSize;
                }
                currentPos += (attSize * commands[index].atts[x].isList);
            }
		}
	}

	sendCmd(input, index);
	free(input);

}
//比较输入的命令和commands中的命令
static uint8_t getMatched(char* cmd)
{
	uint8_t cmdLength = strlen(cmd);
	uint8_t tempLength = 0;
	uint16_t cmdMatches = 0;
	uint8_t matchedTemp[250];
	char tempcmd[200];

	uint16_t Idx;
	for (Idx = 0; Idx < COMMANDS_SIZE; Idx++)
	{
		tempLength = strlen(commands[Idx].cmdName);
		//比较命令名字长度
		if (tempLength >= cmdLength)
		{
			strcpy(tempcmd, commands[Idx].cmdName);
			tempcmd[cmdLength] = '\0';
			//比较名称，如果一样
			if (strcmp(cmd, tempcmd) == 0)
			{
				//名称长度一样
				if (tempLength == cmdLength)
				{
					//将命令所在的idx存放到matchedTemp[0]中
					matchedTemp[0] = Idx;
					//匹配到一个
					cmdMatches = 1;
					
					matchedLength = 0;
					//直接跳到最后一个
					Idx = COMMANDS_SIZE;
				}
				else
				{
					//名称长度不一样，则进行查找
					matchedTemp[cmdMatches] = Idx;
					cmdMatches++;
				}
			}
		}
	}
	uint8_t index = strlen(cmd);
	
	if (cmdMatches > 1 && (matchedLength == cmdMatches || matchedLength == 0))
	{

		matchedLength = cmdMatches;
		cmd[index] = commands[matchedTemp[0]].cmdName[index];
		cmd[index + 1] = '\0';
		getMatched(cmd);
		matchedLength = cmdMatches;
		memcpy(matchedCmds, matchedTemp, cmdMatches);
		return 0;
	}
	memcpy(matchedCmds, matchedTemp, cmdMatches);
	if (matchedLength != 0)
		cmd[index - 1] = '\0';
	matchedLength = cmdMatches;
	return 0;

}

uint8_t histIdx = 0;
uint8_t isHistRollOver = 0;
uint32_t maxHist = 256;
char hist[256][256];
static uint8_t clGetCmd(void)
{
	char ch;
	char tempStr[256];
	char tempHist[256];
	uint8_t cPos = 0;
	char cmdStr[256] =
		{ 0 };
	char lastChar;
	uint32_t cmdStrIdx = 0;
	uint32_t currHist = 0;
	uint8_t cmdComplete = 0;
	uint8_t isDir;
	isDir = 0;
	cmdStr[0] = '\0';
	consolePrint("Enter CMD\n");

	while (cmdComplete == 0)
	{
		//获取一个字符
		ch = consoleGetCh();
		switch (ch)
		{
		case '\t'://制表符
			consoleClearLn();
			
			consolePrint("\r%s", cmdStr);

			getMatched(cmdStr);

			if (matchedLength == 1) // there was only 1 match
			{
				if (lastChar == '\t') //2 tabs pressed then print command description
				{
					SET_HELP_COLOR();
					consolePrint("\nDescription:\n%s",
					        commands[matchedCmds[0]].cmdDesc);
					SET_NRM_COLOR();
					consolePrint("\nEnter CMD\n");
					ch = ' ';
				}
				else
				{
					strcpy(cmdStr, commands[matchedCmds[0]].cmdName);
				}
			}
			else if (matchedLength > 1) // there was more than 1 match
			{
				if (lastChar == '\t') //2 tabs pressed then print all commands that start with
				{
					consolePrint("\n\n");
					uint8_t mIdx;
					SET_HELP_COLOR();
					for (mIdx = 0; mIdx < matchedLength; mIdx++)
					{
						consolePrint("%s\n",
						        commands[matchedCmds[mIdx]].cmdName);
					}
					SET_NRM_COLOR();
					consolePrint("\n\nEnter CMD\n");
					ch = ' ';
				}
			}
			cmdStrIdx = strlen(cmdStr);
			consoleClearLn();
			consolePrint("\r%s", cmdStr);
			matchedLength = 0;
			consolePrint("%s", KNRM);
			break;
		case '\n'://换行符号
			//enter was press
			getMatched(cmdStr);
			if ((histIdx == 0 && isHistRollOver == 0)
			        || strcmp(hist[(maxHist + histIdx - 1) % maxHist], cmdStr)
			                != 0)
			{
				strcpy(hist[histIdx], cmdStr);
				histIdx = ((histIdx + 1) % maxHist);
			}
			if (histIdx == 0)
			{
				isHistRollOver = 1;
			}
			if (matchedLength == 1)
			{
				if (strcmp(cmdStr, commands[matchedCmds[0]].cmdName) == 0)
				{
					consolePrint("\n");
					inputCmd(matchedCmds[0]);

				}

			}
			matchedLength = 0;
			cmdComplete = 1;
			break;
		case 127:
			//backspace was press
			//clear line
			if (cmdStrIdx > 0)
			{
				if (cPos < cmdStrIdx)
				{
					uint8_t x;
					for (x = 0; x < cPos; x++)
					{
						cmdStr[cmdStrIdx - cPos + x - 1] =
						        tempStr[cPos - x - 1];
					}
					cmdStr[--cmdStrIdx] = '\0';
				}

				consoleClearLn();
				consolePrint("\r%s", cmdStr);

			}
			else
			{
				consoleClearLn();
				consolePrint("\r");
			}
			break;
		case 27:
			isDir = 1;
			if (cmdStrIdx > 0)
			{
				consoleClearLn();
				consolePrint("\r%s", cmdStr);

			}
			else
			{
				consoleClearLn();
				consolePrint("\r");
			}

			break;
		default:
			if (isDir)
			{
				if (ch == '[')
				{
					isDir = 1;
				}
				else if (ch == 'A')
				{
					if (currHist < histIdx)
					{
						cPos = 0;
						if (currHist)
						{
							strcpy(hist[histIdx - currHist], cmdStr);
						}
						if (currHist == 0)
							strcpy(tempHist, cmdStr);
						currHist++;
						strcpy(cmdStr, hist[histIdx - currHist]);
						cmdStrIdx = strlen(cmdStr);

					}

					isDir = 0;
				}
				else if (ch == 'B')
				{
					if (currHist)
					{
						strcpy(hist[histIdx - currHist], cmdStr);
						currHist--;
						if (currHist)
						{
							strcpy(cmdStr, hist[histIdx - currHist]);
						}
						else
						{
							strcpy(cmdStr, tempHist);
						}
						cmdStrIdx = strlen(cmdStr);

					}
					isDir = 0;
				}
				else if (ch == 'C')
				{
					if (cPos > 0)
					{
						cPos--;
					}

					isDir = 0;
				}
				else if (ch == 'D')
				{
					if (cPos < cmdStrIdx)
					{
						tempStr[cPos] = cmdStr[cmdStrIdx - cPos - 1];
						cPos++;
					}
					isDir = 0;
				}
				else
				{
					isDir = 0;
				}
			}
			else
			{
				if (cPos != 0)
				{
					cmdStr[cmdStrIdx - cPos] = ch;
					uint8_t x;
					for (x = 1; x <= cPos; x++)
					{
						cmdStr[cmdStrIdx - cPos + x] = tempStr[cPos - x];
					}
					cmdStr[++cmdStrIdx] = '\0';
				}
				else
				{
					cmdStr[cmdStrIdx++] = ch;
					cmdStr[cmdStrIdx] = '\0';
				}

			}
			consoleClearLn()
			;
			consolePrint("\r%s", cmdStr);
			break;
		}

		uint8_t x;
		for (x = 1; x <= cPos; x++)
		{
			consolePrint("\b");
		}
		consoleFlush();
		lastChar = ch;
	}

	return 0;
}

/*********************************************************************
 * END OF COMMANDS PROCESSING
 *********************************************************************/

void appProcess(void *argument)
{
	int32_t status;
	InitCmds();

	//Flush all messages from the que
	//清空队列
	do
	{
		status = rpcWaitMqClientMsg(50);
	} while (status != -1);

	//init variable
	devState = DEV_HOLD;
	gSrcEndPoint = 1;
	gDstEndPoint = 1;

	status = startNetwork();
	if (status != -1)
	{
		consolePrint("Network up\n\n");
	}
	else
	{
		consolePrint("Network Error\n\n");
	}

	sysGetExtAddr();

	OsalNvWriteFormat_t nvWrite;
	nvWrite.Id = ZCD_NV_ZDO_DIRECT_CB;
	nvWrite.Offset = 0;
	nvWrite.Len = 1;
	nvWrite.Value[0] = 1;
	status = sysOsalNvWrite(&nvWrite);

	while (1)
	{
		status = clGetCmd();
		while (status != -1)
		{
			status = rpcWaitMqClientMsg(1000);
			consolePrint("\n");
		}
	}
}

