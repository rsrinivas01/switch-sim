#include "swLib.h"

extern swSimIntf *swSimIfList[10];

int appendPktDataAndSend(char *ifName, unsigned short int *srcMac, unsigned short int *dstMac, int numPkts)
{
	mqd_t msgQId;
	int ret;
	char *ptr;
	int i;
	char mqName[10];
	char dummyData = 0x56;
	unsigned char pktData[PACKTET_LENGTH];
	ethHdr ethHeader;
	
	sprintf(mqName, "/%s%s", ifName, "ingMsgQ");

	msgQId = mq_open(mqName, O_WRONLY);

	if (msgQId < 0)
	{
		perror("mq_open() failed with error - - ");
		exit(0);
	}

	//memset(pktData, 0x99, PACKTET_LENGTH);
	

	for(i=0; i < numPkts; i++)
	{
		ret = mq_send(msgQId, pktData, sizeof(pktData), 0);	
		if (ret < 0)
		{
			perror("Failed to send packet");
			return -1;
		}
	}
	//printf("\n");
	//printf("%x",pktData[i]);
	return ret;
}
int main(int argc, char *argv[])
{
	int i;
	int ret;
	int userInput;
	int numPorts = 2;
	int numPkts = 2;
	char pktIfName[10];
	unsigned char pktSrcMac[6];
	unsigned char pktDstMac[6];

	while(1)
	{
		printf("---- Packet Generator Menu ----\n");
		printf("1. Send Packet \n");
		printf("2. Exit Packet Generator \n");
		scanf("%d", &userInput);

		switch(userInput)
		{
			case 1:
					printf("Enter Interface Name:");
					scanf("%s", pktIfName);
					printf("Enter Packet Source MAC \
							in hex format (xxxx.yyyy.zzzz):");
					sscanf("%4hx.%4hx.%4hx", (unsigned short *) &pktSrcMac[0], 
											 (unsigned short *) &pktSrcMac[2], 
											 (unsigned short *)  &pktSrcMac[4]);
					printf("Enter Packet Dest MAC \
							in hex format (xxxx.yyyy.zzzz):");
					sscanf("%4hx.%4hx.%4hx", (unsigned short *) &pktDstMac[0], 
											 (unsigned short *) &pktDstMac[2], 
											 (unsigned short *)  &pktDstMac[4]);
					printf("Enter number of packets to send :");
					sscanf("%d", &numPkts);
					appendPktDataAndSend(pktIfName, pktSrcMac, pktDstMac, numPkts);
					break;
			case 2:
					printf("Exiting Packet Generator..\n");
					exit(0);
			default:
					printf("Invalid Input ...\n");
					break;
		}
	}

}
