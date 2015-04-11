#include <stdio.h>
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/socket.h>

#include "ue_types.h"

char kval[MSGSZ], fval[MSGSZ], ttool_type[MSGSZ], ipreq[MSGSZ], respBuff[MSGSZ], putip[MSGSZ], rsockBuff[SKBUFF_SZ], portn[SKBUFF_SZ];
int rx_msqid[TOT_MQ];
int sockfd, ue_triggered, key_type = -1;
struct sockaddr_in serv_addr;
key_t rx_key[TOT_MQ];
message_buf rgui, rbuf[TOT_MQ];



int extract_fields(char *msg)
{
	/* Msgs arrive in the format {"K_Field":"k1","F_Field":"f1"} */
	int i = 0, j = 0;

	i = i + 12; //Skip 12 bytes i.e. skip -> `{"K_Field":"`

	while (msg[i] != '"') {
		kval[j++] = msg[i++];
		//debug_print("kval[%d] = %c\n", j-1, kval[j-1]);
	}

	if(msg[i+1] != ',' || msg[i+2] != '"') {
		printf("Msg is invalid as k1 contains \" delimiter\n");
		debug_print("msg[i+1] = %c\n", msg[i+1]);
		debug_print("msg[i+2] = %c\n", msg[i+2]);

		return 1;
	}

	kval[j] = '\0';
	j = 0;
	i = i + 13; //Skip 13 bytes i.e. skip -> `,"F_Field:"`
	while (msg[i] != '"') {
		fval[j++] = msg[i++];
		//debug_print("fval[%d] = %c\n", j-1, fval[j-1]);

	}

	if(msg[i+1] != '}') {
		printf("Msg is invalid as f1 does not contain } delimiter\n");
		//debug_print("msg[i+1] = %c\n", msg[i+1]);

		return 1;
	}

	fval[j] = '\0';

	debug_print ("Extracted kval = %s\n", kval);
	debug_print ("Extracted fval = %s\n", fval);

	return 0;

}

int create_cfg_file(char * msg, int paramcnt)
{
	FILE *f;
	int i = 0, j = 0;
	char eachmsg[MSGSZ], rxmsg[MSGSZ];

	/* Msgs arrive in the format [{"K_Field:"k1","F_Field:"f1"}, \
         *         				{"K_Field:"k2","F_Field:"f2"}]
         */


	strncpy(rxmsg, msg, strlen(msg));

	f = fopen("ue_config.cfg", "w");
	debug_print ("checking for %s\n", rxmsg);
	debug_print ("loop = %d\n", paramcnt);


	//Next, trim the rx message (get rid of [] brackets)
	memmove (rxmsg, &(rxmsg[1]), strlen(rxmsg));//including NULL char
	i = strlen(rxmsg); //This gives the index to ']' character
	rxmsg[i - 1] = '\0'; //Now rxmsg is devoid of ] and ends at that point

	/* From here on, the rxmsg is of type:
	 * {},{},{},.....,{}
         */

	i = 0;

	/* Next, for each param, extract its kval and fval*/
	for (j = 0; j < paramcnt; j++) {
		while (rxmsg[i] != '}')
			i++;

		//debug_print("==== rxmsg[i = %d] = %c\n",i, rxmsg[i]);

		/* Now i contains the length of each msg */
		strncpy(eachmsg, rxmsg, i + 1);
		i++;

		eachmsg[i] = '\0';
		//debug_print("==== eachmsg[i = %d] = %d\n",i, eachmsg[i]);
		//debug_print("eachmsg[i - 1] = %c\n", eachmsg[i - 1]);
		//debug_print("eachmsg[len = %d] = %s\n", i, eachmsg);
 
		memmove (rxmsg, rxmsg + strlen(eachmsg) + 1/*Comma*/, strlen(rxmsg) - (strlen(eachmsg) - 2));

		//debug_print("Updated rxmsg = %s\n", rxmsg);


		debug_print("Extract fields for: %s\n", eachmsg);
			
		extract_fields(eachmsg); //This updates kval and fval

		fwrite(kval, strlen(kval), 1, f);
		fwrite("=", 1, 1, f);
		fwrite(fval, strlen(fval), 1, f);
		fwrite("\n", 1, 1, f);

		memset (eachmsg, '0', sizeof(eachmsg));
		i = 0;
	}
	fclose(f);

	return 0;
}


int json_to_c(void)
{
			//Convert JSON string to C data

		if (!strcmp ("CF_UL", kval))
			key_type = CF_UL;
		if (!strcmp ("CF_DL", kval))
			key_type = CF_DL;
		if (!strcmp ("Bandwidth", kval))
			key_type = Bandwidth;
		if (!strcmp ("IMSI", kval))
			key_type = IMSI;
		if (!strcmp ("IMEI", kval))
			key_type = IMEI;
		if (!strcmp ("Sver", kval))
			key_type = Sver;
		if (!strcmp ("Koper", kval))
			key_type = Koper;
		if (!strcmp ("Auth_Mil_OC", kval))
			key_type = Auth_Mil_OC;
		if (!strcmp ("Auth_Mil_OP_OPC", kval))
			key_type = Auth_Mil_OP_OPC;
		if (!strcmp ("Test_Tool_Act", kval)) {
			printf ("Successful match found\n");
			key_type = Test_Tool_Act;
		}
		if (!strcmp ("Test_Tool", kval)) {
			key_type = Test_Tool;
			strcpy(ttool_type, fval);
		}
		if (!strcmp ("CF_St", kval))
			key_type = CF_St;
		if (!strcmp ("ULEARFCN", kval))
			key_type = ULEARFCN;
		if (!strcmp ("DLEARFCN", kval))
			key_type = DLEARFCN;
		if (!strcmp ("Gain_St", kval))
			key_type = Gain_St;
		if (!strcmp ("Gain_Tx", kval))
			key_type = Gain_Tx;
		if (!strcmp ("Gain_Rx", kval))
			key_type = Gain_Rx;
		if (!strcmp ("Dup_Mode", kval))
			key_type = Dup_Mode;
		if (!strcmp ("Log_File_Storage", kval))
			key_type = Log_File_Storage;
		if (!strcmp ("Log_File_Offline", kval))
			key_type = Log_File_Offline;
		if (!strcmp ("Log_File_Online", kval))
			key_type = Log_File_Online;
		if (!strcmp ("UE_Category", kval))
			key_type = UE_Category;
		if (!strcmp ("USIM_ST", kval))
			key_type = USIM_ST;
		if (!strcmp ("Auth_Alg_St", kval))
			key_type = Auth_Alg_St;
		if (!strcmp ("Modem_Status", kval)) {
			if (!strcmp ("ON", fval))
				key_type = ON;
			else if (!strcmp ("OFF", fval))
				key_type = OFF;
		}
		if (!strcmp ("IP_Address", kval)) {
			debug_print ("fval in ip addr = %s\n", fval);
			{
				int len = strlen(fval);
				strcpy(putip, fval);
				
	debug_print ("fval transformed to putip as = %s\n", putip);
			}
			if (!strcmp ("GET", fval)) {
				key_type = GETIP;
				strcpy(ipreq, fval);
			}

			else {
				key_type = SETIP;
				strcpy(ipreq, fval);
			}

		}
		if (!strcmp ("Port_No", kval))
			key_type = Port_No;

}


int check_msg_validity(char *rxmsg, int msglen)
{
	/* Msgs arrive in the format [{"K_Field:"k1","F_Field:"f1"}, \
         *         				{"K_Field:"k2","F_Field:"f2"}]
         */

	int i = 0, j = 0, paramcnt = 0;
	char eachmsg[MSGSZ], getIPAddr[MSGSZ], getPortNo[MSGSZ], cmd[MSGSZ];

	/* First, check the no. of params. arrived */
	while (i < msglen) {
		if (rxmsg[i] == '{')
			paramcnt++;
		i++;
	}

	debug_print ("paramcnt = %d\n", paramcnt);

	//Next, trim the rx message (get rid of [] brackets)
	memmove (rxmsg, &(rxmsg[1]), strlen(rxmsg));//including NULL char
	i = strlen(rxmsg); //This gives the index to ']' character
	rxmsg[i - 1] = '\0'; //Now rxmsg is devoid of ] and ends at that point

	/* From here on, the rxmsg is of type:
	 * {},{},{},.....,{}
         */

	debug_print ("Trimmed msg = %s\n", rxmsg);

	i = 0;

	/* Next, extract individual param strings */
	for (j = 0; j < paramcnt; j++) {
		while (rxmsg[i] != '}')
			i++;

		//debug_print("==== rxmsg[i = %d] = %c\n",i, rxmsg[i]);

		/* Now i contains the length of each msg */
		strncpy(eachmsg, rxmsg, i + 1);
		i++;

		eachmsg[i] = '\0';
		//debug_print("==== eachmsg[i = %d] = %d\n",i, eachmsg[i]);
		//debug_print("eachmsg[i - 1] = %c\n", eachmsg[i - 1]);
		//debug_print("eachmsg[len = %d] = %s\n", i, eachmsg);
 
		memmove (rxmsg, rxmsg + strlen(eachmsg) + 1/*Comma*/, strlen(rxmsg) - (strlen(eachmsg) - 2));

		//debug_print("Updated rxmsg = %s\n", rxmsg);


		debug_print("Extract fields for: %s\n", eachmsg);
			 
		extract_fields(eachmsg); //This updates kval and fval

		json_to_c(); //This updates the key_type & depends on kval and fval

		switch (key_type) {
			case CF_St:
			case CF_UL:
			case CF_DL:
			case ULEARFCN:
			case DLEARFCN:
			case Gain_St:
			case Gain_Tx:
			case Gain_Rx:
			case Bandwidth:
			case Dup_Mode:
			case Log_File_Storage:
			case Log_File_Offline:
			case Log_File_Online:
			case UE_Category:
			case Bit_Ind:
			case USIM_ST:
			case IMSI:
			case IMEI:
			case Sver:
			case Koper:
			case Auth_Alg_St:
			case Auth_Mil_OC:
				break;
			case Auth_Mil_OP_OPC:	//Settings
				if(!ue_triggered) {
					char cfgmsg[MSGSZ];
					strncpy(cfgmsg, rsockBuff, msglen);
					create_cfg_file(cfgmsg, paramcnt);
					printf ("***** Calling mymo_app *****\n");
					(void)system("./ue_app&");
					ue_triggered = 1;
				}
				break;

			case Port_No:
				debug_print("**** Port_No should already be handled along with IPAddress ****\n");
				break;
			case GETIP:
				debug_print("ipreq = %s\n", ipreq);
				if(!strcmp(ipreq, "GET")){
					debug_print("Requesting IP Config.: %s\n", fval);
					FILE *fip;
					fip = popen("ifconfig eth1 | grep 'inet addr:' | cut -d: -f2 | awk '{print $1}'", "r");

					while(fgets(getIPAddr, sizeof(getIPAddr)-1, fip) != NULL);

					i = strlen(getIPAddr);
					getIPAddr[i-1] = '\0'; //Get rid of '\n' char

					memset(respBuff, '0', sizeof(respBuff));

					fip = popen("cat /proc/sys/net/ipv4/ip_local_reserved_ports", "r");
					while (fgets(getPortNo, sizeof(getPortNo)-1, fip) != NULL);

					i = strlen(getPortNo);
					getPortNo[i-1] = '\0'; //Get rid of '\n' char

					printf("IP ADDRESS>>>>: %s\n", getIPAddr);
					printf("Port No>>>>: %s\n", getPortNo);

					snprintf (respBuff, sizeof(respBuff), "[{\"K_Field\":\"IP_Address\",\"F_Field\":\"%s\"},{\"K_Field\":\"Port_No\",\"F_Field\":\"%s\"}]", getIPAddr, getPortNo);
				
					printf ("Sending JSON data %s\n", respBuff);
					i = write(sockfd, respBuff, strlen(respBuff));


				}
				else{
					printf("STATUS: IP config: %s\n", putip);
					snprintf (cmd, 128, "ifconfig eth1 %s", putip);
					printf("cmd = %s\n", cmd);
					(void)system(cmd);

					printf("STATUS: Port config: %s\n", portn);
					snprintf (cmd, 128, "echo %s > /proc/sys/net/ipv4/ip_local_reserved_ports", portn);
					printf ("Port set cmd = %s\n", cmd);
					(void)system(cmd);
				}
				break;

				case L1_Run:
					printf("STATUS: Starting L1_Run\n");
				break;

				case L2_Run:
					printf("STATUS: Starting L2_Run\n");
				break;

				case AT_CMD:
					printf("STATUS: Starting AT_CMD\n");
				break;

				case ON:
					/*Call Modem ON script*/
					(void)system("./ue_modem.sh&");
					printf("STATUS: Modem start\n");
				break;

				case OFF:
					/*Kill Modem Binary*/
					(void)system("kill ./ue_modem&");
					ue_triggered = 0;
					printf("STATUS: Modem stop\n");
				break;

				case Test_Tool:
					printf("Starting Test_Tool: %s\n", fval);
					strcpy(ttool_type, fval);
				break;

				case Test_Tool_Act:
					printf("STATUS: Acting on Test_Tool: %s\n", ttool_type);
					printf("Action type: %s\n", fval);
					if (!strcmp(ttool_type, "\"Anite 9000\"")) {
						printf("In Anite 9000\n");
					
						if (!strcmp("\"Start\"", fval)) {
							printf("Starting Anite 9000\n");
						}
						else if (!strcmp("\"Stop\"", fval)) {
							printf("Stopping Anite 9000\n");
						}
					}
					if (!strcmp(ttool_type, "\"R & S CMW 500\"")) {
						printf("In R & S CMW 500\n");
					
						if (!strcmp("\"Start\"", fval)) {
							printf("Starting R & S CMW 500\n");
						}
						else if (!strcmp("\"Stop\"", fval)) {
							printf("Stopping R & S CMW 500\n");
						}
					}


				break;

				case RTT:
					printf("STATUS: Starting RTT\n");
				break;
				case AMM:
					printf("STATUS: Starting AMM\n");
				break;
				case HLT:
					printf("STATUS: Starting HLT\n");
				break;
				case Req_Modem_SL:
					printf("STATUS: Starting Req_Modem_SL\n");
				break;
				default:
					printf("STATUS: Invalid request received from GUI, key = %d\n", key_type);
				break;
		}

		memset (eachmsg, '0', sizeof(eachmsg));
		i = 0;

	}

	return 0;
}

int act_on_msg(char *msg, int msglen)
{
	int ret;
	char rxmsg[MSGSZ];

	debug_print ("Analysing msg of len %d\n", msglen);

	strncpy(rxmsg, msg, msglen);
	ret = check_msg_validity(rxmsg, msglen);

	if (!ret) {
		debug_print ("Msg is valid\n");

	}
	return ret;
}

int sendto_gui(char *data, int size)
{
	int n = 0;

	n = write(sockfd, data, size);
	printf ("Wrote %d bytes of (%d) GUI data successfully\n", n, size);
	printf ("The GUI data wrote is %s\n", data);

	return 0;
}

int tx_guidata(char *txdata)
{
	int i;

	debug_print ("tx_guidata++\n");
	//Setup AMM MQ only for now
	rbuf[MQ_AMM].mtype = 1;

	while (1) {
		printf ("[ctl thread]: Reading msgq data on rx_msgqid = 0x%x\n", rx_msqid[MQ_AMM]);
		if((i = msgrcv(rx_msqid[MQ_AMM], &rbuf[MQ_AMM], MSGSZ, 1, 0)) < 0) {
			perror("msgrcv on ue_ctl_txdata");
			continue;
		}
		else {
			rbuf[MQ_AMM].mtext[i] = '\0';
			printf ("Rxd. [MQ_AMM] Message on: 0x%x\n", rx_msqid[MQ_AMM]);
			printf ("MSG: %s\n", rbuf[MQ_AMM].mtext);
			snprintf(rgui.mtext, sizeof(rgui.mtext), "[{\"K_Field\":\"AMM\",\"F_Field\":\"%s\"}]", rbuf[MQ_AMM].mtext);

			sendto_gui(rgui.mtext, strlen(rgui.mtext));
		}
	}		


	return 0;
}


int main(int argc, char *argv[])
{
	int *params, ret, i = 0;
	char errBuff[SKBUFF_SZ];
	pthread_attr_t attr;
	pthread_t tx_guithread;


	if(argc != 2) {
		printf ("\nUsage: %s <IP Addr. of the server>\n\n", argv[0]);
		return 1;
	}

	/* Create MsgQueues */

	for (i = 0; i < TOT_MQ; i++) {
		debug_print ("Creating MsgQueue[%d]\n", i);
		rx_key[i] = ftok ("ue_sockclient.c", i);

		if ((rx_msqid[i] = msgget(rx_key[i], CREATE_MSGF)) < 0) {
			perror("msgget");
			exit(1);
		}
		else {
			(void) fprintf(stderr, "ue_sock: msgget: ue_rxapp" \
					" msgget succeeded: rx_msqid[%d] = 0x%x\n", i, rx_msqid[i]);
		}
	}

	/* Create a seperate thread for data tx from modem */

	debug_print ("Creating tx_guithread..\n");
	pthread_attr_init(&attr);
	i = pthread_create(&tx_guithread, 0, tx_guidata, params);

	if (i < 0) {
		printf ("Error creating thread, bailing out!\n");
		exit(1);
	}
	/* With reqd MsgQs and TxGUI thread in place, initiate SK connect */

	memset (rsockBuff, '0', sizeof(rsockBuff));

	debug_print ("Creating socket interface..\n");

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		printf ("\n Error: Could not create socket\n");
		return 1;
	}

	memset (&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(5000);

	if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <=0) {
		printf ("\n inet_pton error occured\n");
		return 1;
	}

	debug_print ("Connecting to socket..\n");

    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
       printf("\n Error : Connect Failed with errno: %d\n", errno);
       return 1;
    } 

	debug_print ("Socket connected successfully.\nEntering SKREAD mode...\n");

	/* With all the required prerequisites done and a seperate thread for tx 
	   to GUI created, we now enter into an infinite loop, always waiting to 
	   read data from the server side. 
    */

	while (1) {
		i = read(sockfd, rsockBuff, sizeof(rsockBuff));

		if (i < 0) {
			int mqcnt = TOT_MQ;

			printf ("Error reading data from server, errno = %d\n", errno);
			printf("Bailing out from the application\n");

			while (mqcnt--)
				msgctl(rx_msqid[mqcnt], IPC_RMID, 0); //Delete created MQs
			close(sockfd);

			return 1;
		}

		debug_print ("Received %d bytes of data\n", i);

		if (i < SKBUFF_SZ)
			rsockBuff[i] = '\0';

//TODO: Remove below debug line later
		debug_print ("Socket received msg[%d]: %s\n", i, rsockBuff);

		ret = act_on_msg(rsockBuff, i);

		if (ret) {
			strcpy (errBuff, "Invalid inputs detected\n");
			write(sockfd, errBuff, strlen(errBuff));
		}

		
	}

	return 0;
}
