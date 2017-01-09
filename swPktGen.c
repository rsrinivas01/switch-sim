#include "swLib.h"

extern swSimIntf *swSimIfList[10];

int appendPktDataAndSend(char *ifName, unsigned char *srcMac, unsigned char *dstMac, int numPkts)
{
	mqd_t msgQId;
	int ret;
	char *ptr;
	int i, j;
	unsigned short int type = htons(0x0800);
	char mqName[10];
	unsigned char pktData[82];
	unsigned char defData[68] = {0x45, 0x00, 0x00, 0x44, 0xad, 0x0b, 0x00, 0x00, 0x40, 0x11, 0x72, 0x72, 0xac, 0x14, 0x02, 0xfd, 
		                         0xac, 0x14, 0x00, 0x06, 0xe5, 0x87, 0x00, 0x35, 0x00, 0x30, 0x5b, 0x6d, 0xab, 0xc9, 0x01, 0x00, 
								 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x6d, 0x63, 0x63, 0x6c, 0x65, 0x6c, 0x6c, 
								 0x61, 0x6e, 0x02, 0x63, 0x73, 0x05, 0x6d, 0x69, 0x61, 0x6d, 0x69, 0x03, 0x65, 0x64, 0x75, 0x00,
								 0x00, 0x01, 0x00, 0x01
	                             };  


	memset(&pktData, 0, sizeof(pktData));

	memcpy((char *) &pktData[0], dstMac, 6);
	memcpy((char *) &pktData[6], srcMac, 6);
	memcpy((char *) &pktData[12], (char *) &type, 2); 
	memcpy((char *) &pktData[14], (char *) defData, sizeof(defData)); 

	for(i=0; i < numPkts; i++)
	{
		printf("Packet - %d: \n", i);
		for(j=0; j < sizeof(pktData); j++)
		{
			printf("%02x",pktData[j]);
		}
		printf("\n");
	}
 
	sprintf(mqName, "/%s%s", ifName, "ingMsgQ");

	msgQId = mq_open(mqName, O_WRONLY);

	if (msgQId < 0)
	{
		perror("mq_open() failed with error - - ");
		exit(0);
	}

	for(i=0; i < numPkts; i++)
	{
		ret = mq_send(msgQId, pktData, sizeof(pktData), 0);	
		if (ret < 0)
		{
			perror("Failed to send packet");
			return -1;
		}
	}

	return ret;
}
int main(int argc, char *argv[])
{
	int i;
	unsigned short vid;
	unsigned short ethType, ethLen;
	char choice;
	int ret;
	int userInput;
	int numPorts = 2;
	int numPkts = 2;
	char pktIfName[10];
	unsigned char pktSrcMac[6];
	unsigned char pktDstMac[6];
	unsigned char pktSrcMacString[15];

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
					printf("Enter Packet Source MAC in hex format (xx:xx:yy:yy:zz:zz):");
					scanf("%s", pktSrcMacString);
					if(!strcmp(pktSrcMacString, "0"))
					{
						printf("Added Test Src Mac - 00:18:8b:75:1d:e0 \n");
						strcpy((char *) &pktSrcMacString, "00:18:8b:75:1d:e0");
					}
					sscanf((const char *) &pktSrcMacString, "%x:%x:%x:%x:%x:%x", 
							(unsigned int *) &pktSrcMac[0], 
							(unsigned int *) &pktSrcMac[1], 
							(unsigned int *) &pktSrcMac[2], 
							(unsigned int *) &pktSrcMac[3], 
							(unsigned int *) &pktSrcMac[4], 
							(unsigned int *)  &pktSrcMac[5]);
					printf("Enter Packet Dest MAC in hex format (xx:xx:yy:yy:zz:zz):");
					scanf("%s", pktSrcMacString);
					if(!strcmp(pktSrcMacString, "0"))
					{
						printf("Added Test Dst Mac - 00:1f:f3:d8:47:ab \n");
						strcpy((char *) &pktSrcMacString, "00:1f:f3:d8:47:ab");
					}
					sscanf((const char *) &pktSrcMacString, "%x:%x:%x:%x:%x:%x", 
							(unsigned int *) &pktDstMac[0], 
							(unsigned int *) &pktDstMac[1], 
							(unsigned int *) &pktDstMac[2], 
							(unsigned int *) &pktDstMac[3], 
							(unsigned int *) &pktDstMac[4], 
							(unsigned int *) &pktDstMac[5]);
					printf("Enter number of packets to send :");
					scanf("%d", &numPkts);
					printf("entered source mac -   %.02x%02x.%02x%02x.%02x%02x  \
                            dest mac -  %.02x%02x.%02x%02x.%02x%02x  \
                            vid - %d \n",
							pktSrcMac[0], pktSrcMac[1], pktSrcMac[2], 
							pktSrcMac[3], pktSrcMac[4], pktSrcMac[5], 
							pktDstMac[0], pktDstMac[1], pktDstMac[2],
							pktDstMac[3], pktDstMac[4], pktDstMac[5],
                            vid);

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
