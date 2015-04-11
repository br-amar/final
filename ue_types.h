#ifndef __UE_TYPES_H
#define __UE_TYPES_H

#ifdef DEBUG
#define DEBUG_PRINT 1
#else
#define DEBUG_PRINT 0
#endif

#define debug_print(fmt, args...) \
	do {if (DEBUG_PRINT) fprintf(stderr, "%s:%d:%s(): "  fmt, __FILE__, __LINE__, __FUNCTION__, ##args);} while (0)


#define CREATE_MSGF	IPC_CREAT | 0666
#define CONNECT_MSGF	0666

#define SKBUFF_SZ	1024
#define MSGSZ		1024

enum UE_MQ {
	MQ_AMM,
	TOT_MQ
} ue_mq_t;

typedef struct msgbuf {
	long mtype;
	char mtext[MSGSZ];
}message_buf;


typedef enum MSG_TYPE {
	CF_St,
	CF_UL,
	CF_DL,
	CF_ULEARFCN,
	CF_DLEARFCN,//5
	Gain_St, 
	Gain_Tx,
	Gain_Rx,
	Bandwidth,
	Dup_Mode,//10
	Log_File_Storage,
	Log_File_Offline,
	Log_File_Online,
	UE_Category,
	Bit_Ind,//15
	IMSI,
	IMEI,
	Sver,
	Koper,
	Auth_Mil_OC,//20
	Auth_Mil_OP_OPC,	//Settings
	SETIP,
	Port_No,	//IP Settings
	GETIP,
	Test_Tool,//25
	Test_Tool_Act, 
	ULEARFCN,
	DLEARFCN,
	USIM_ST,
	Auth_Alg_St, //30
	L1_Run,
	L2_Run, 
	AT_CMD,
	ON,
	OFF, //35
	RTT,
	AMM,
	HLT,
	Req_Modem_SL
} msg_type_t;








#endif //__UE_TYPES_H
