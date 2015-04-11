#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ue_types.h"

key_t ue_key[TOT_MQ];
int ue_msqid[TOT_MQ];

message_buf sbuf[TOT_MQ];


int read_init_settings(void)
{
	FILE *fin;
	int fsize;
	int lc = 0;


	printf ("read_init_settings++\n");


	//Modem can now access JSON file data and complete initialization

//TODO:For now, read back the ini-setting json data to be passed through AM
	while (1) {	
#if 0
		//Open received JSON file
		fin = fopen("ue_config.cfg", "rb");
		fseek(fin, 0, SEEK_END);
		fsize = ftell(fin);
		//Send this file content back as AM data
		fseek(fin, 0, SEEK_SET);

		fread(sbuf[AM_TX].mtext, 1, fsize, fin);
		//printf ("%s", sbuf[AM_TX].mtext);

		sbuf[AM_TX].mtype = 1;


		//sbuf now has the content of JSON file. Write it back to the AM MQ
		if(msgsnd(ue_msqid[AM_TX], &sbuf[AM_TX], fsize, 0) < 0) {
			perror("msgsnd");
			exit(1);
		}
		else {
			printf ("\nMessage[%d]: \"%s\" Sent back to GUI using "
				"ue_msqid = 0x%x\n", AM_TX, sbuf[AM_TX], ue_msqid[AM_TX]);
		}

		sleep(5);
		lc++;
		printf("Off sleep, lc = %d\n", lc);
		fclose(fin);
		//break;
#endif

		lc++;

		sbuf[MQ_AMM].mtype = 1;
		//sbuf now has the content of JSON file. Write it back to the AM MQ
		strcpy (sbuf[MQ_AMM].mtext, "ue_app response");
		if(msgsnd(ue_msqid[MQ_AMM], &sbuf[MQ_AMM], 5,  0) < 0) {
			perror("msgsnd");
			exit(1);
		}
		else {
			printf ("\nMessage[%d]: \"%s\" Sent back to GUI using "
				"ue_msqid = 0x%x for %d times\n", MQ_AMM, sbuf[MQ_AMM].mtext, ue_msqid[MQ_AMM], lc);
		}

		sleep(60);
	}



	return 0;
}

int main(void)
{
	int i;

		//Connect to tx msgqueues
	for (i = 0; i < TOT_MQ; i++) {

		ue_key[i] = ftok ("ue_sockclient.c", i);
		if ((ue_msqid[i] = msgget(ue_key[i], CONNECT_MSGF)) < 0) {
			perror("ue_app: msgget");
			exit(1);
		}
		else
			(void) fprintf(stderr, "ue_app: msgget: msgget \
				succeeded: ue_msqid = 0x%x\n", ue_msqid[i]);
	}

	read_init_settings();

	

	return 0;
}

